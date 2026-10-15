#include "HRCombat.h"
#include "HRRTSDefinitions.h"
#include "HRRTSTags.h"
#include "../PrototypeGameMode.h"
#include "Engine/World.h"
#include "Misc/ScopeExit.h"
#include <limits>

bool RunHRCombatSelfTests(UWorld* World)
{
    bool Passed = true;
    int32 Count = 0;
    auto Check = [&](bool Condition, const TCHAR* Label)
    {
        ++Count;
        Passed &= Condition;
        UE_LOG(LogTemp, Display, TEXT("TACTICAL_TEST %s: %s"), Label, Condition ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto Close = [](float Actual, float Expected) { return FMath::IsNearlyEqual(Actual, Expected, .001f); };

    Check(Close(FHRDamageTable::Resolve(20.f,EHRDamageType::Kinetic,EHRArmorClass::Structure,1.f,false),10.f)
        && Close(FHRDamageTable::Resolve(20.f,EHRDamageType::Explosive,EHRArmorClass::Structure,1.f,false),28.f)
        && Close(FHRDamageTable::Resolve(20.f,EHRDamageType::Tempest,EHRArmorClass::Vehicle,1.f,false),24.f),
        TEXT("damage matrix differentiates kinetic, explosive and Tempest against armor classes"));
    Check(Close(FHRDamageTable::Resolve(20.f,EHRDamageType::Kinetic,EHRArmorClass::Infantry,1.1f,true),14.f)
        && Close(FHRDamageTable::Resolve(20.f,EHRDamageType::Kinetic,EHRArmorClass::Vehicle,1.f,true),20.f),
        TEXT("cover reduces infantry damage by 30 percent and does not protect vehicles"));
    const float NaN = std::numeric_limits<float>::quiet_NaN();
    Check(FHRDamageTable::Resolve(NaN,EHRDamageType::Kinetic,EHRArmorClass::Vehicle,1.f,false) == 0.f
        && FHRDamageTable::Resolve(20.f,EHRDamageType::Kinetic,EHRArmorClass::Vehicle,0.f,false) == 0.f
        && FHRDamageTable::Resolve(20.f,static_cast<EHRDamageType>(99),EHRArmorClass::Vehicle,1.f,false) == 0.f
        && FMath::IsFinite(FHRDamageTable::Resolve(TNumericLimits<float>::Max(),EHRDamageType::Explosive,
            EHRArmorClass::Infantry,.1f,false)), TEXT("malformed damage and armor are rejected; extreme finite damage remains finite"));
    UHRDamageMatrix* Matrix = NewObject<UHRDamageMatrix>(GetTransientPackage());
    FHRDamageRule Immune;
    Immune.DamageType = HRRTSTags::Damage_Tempest;
    Immune.ArmorType = HRRTSTags::Armor_Vehicle;
    Immune.Multiplier = 0.f;
    Matrix->Rules.Add(Immune);
    Check(FHRDamageTable::Resolve(100.f,EHRDamageType::Tempest,EHRArmorClass::Vehicle,1.f,false,Matrix) == 0.f,
        TEXT("authored damage matrix immunity is consumed by the same damage resolver"));

    FHRCombatState Veteran;
    Veteran.AddExperience(99.f,false);
    Check(Veteran.Rank == 0 && Veteran.Experience == 99.f, TEXT("99 cumulative XP has no rank"));
    Veteran.AddExperience(1.f,false);
    Check(Veteran.Rank == 1 && Close(Veteran.GetVeterancyModifiers().Damage,1.15f),
        TEXT("100 XP grants rank one and its authored fallback damage bonus"));
    Veteran.AddExperience(150.f,false);
    Check(Veteran.Rank == 2 && Close(Veteran.GetVeterancyModifiers().Range,1.16f)
        && Close(Veteran.GetVeterancyModifiers().Armor,1.2f), TEXT("250 XP selects rank two range and armor bonuses"));
    Veteran.AddExperience(200.f,true);
    Check(Veteran.Rank == 3 && Veteran.Experience == 500.f && Close(Veteran.GetVeterancyModifiers().Damage,1.5f),
        TEXT("kill bonus is 50 XP; rank three starts at exactly 500 XP"));
    Veteran.AddExperience(NaN,true);
    Veteran.AddExperience(-100.f,true);
    Check(Veteran.Experience == 500.f, TEXT("invalid and zero damage cannot generate kill XP"));
    UHRUnitDefinition* AuthoredVeteran = NewObject<UHRUnitDefinition>(GetTransientPackage());
    FHRVeterancyLevel First; First.XPThreshold=100; First.DamageMultiplier=1.1f;
    FHRVeterancyLevel Second; Second.XPThreshold=250; Second.DamageMultiplier=1.4f; Second.RangeMultiplier=1.2f;
    AuthoredVeteran->VeterancyLevels = {Second,First};
    Check(Close(Veteran.GetVeterancyModifiers(AuthoredVeteran).Damage,1.4f)
        && Close(Veteran.GetVeterancyModifiers(AuthoredVeteran).Range,1.2f),
        TEXT("saved level data uses highest reached threshold without stacking or relying on order"));

    FHRCombatState Suppressed;
    Suppressed.AddSuppression(20.f,100.f);
    Check(Suppressed.IsSuppressed() && Suppressed.Suppression == 40.f, TEXT("20 percent health damage reaches infantry suppression threshold"));
    Suppressed.Advance(1.f);
    Suppressed.Advance(1.f);
    Check(Suppressed.Suppression == 40.f, TEXT("suppression does not recover during two second hit grace period"));
    Suppressed.Advance(.5f);
    Check(Close(Suppressed.Suppression,31.f) && !Suppressed.IsSuppressed(), TEXT("suppression recovers at 18 points per simulation second"));
    const float BeforeSuppression = Suppressed.Suppression;
    Suppressed.Advance(0.f); Suppressed.Advance(-1.f); Suppressed.Advance(NaN);
    Check(Suppressed.Suppression == BeforeSuppression, TEXT("pause and malformed delta do not advance combat clocks"));

    FHRCombatState Abilities;
    Check(!Abilities.CommitAbility(EHRAbility::Grenade,false,true,true)
        && !Abilities.CommitAbility(EHRAbility::Grenade,true,false,true)
        && !Abilities.CommitAbility(EHRAbility::Grenade,true,true,false) && Abilities.GrenadeCooldown == 0.f,
        TEXT("unsupported, unavailable and targetless abilities leave cooldown untouched"));
    Check(Abilities.CommitAbility(EHRAbility::Grenade,true,true,true) && Abilities.GrenadeCooldown == 12.f
        && !Abilities.CommitAbility(EHRAbility::Grenade,true,true,true), TEXT("grenade cooldown commits once and rejects repeat activation"));
    Abilities.Advance(0.f);
    Check(Abilities.GrenadeCooldown == 12.f, TEXT("paused grenade cooldown does not elapse"));
    Abilities.Advance(12.f);
    Check(Abilities.GrenadeCooldown == 0.f && Abilities.CommitAbility(EHRAbility::Grenade,true,true,true),
        TEXT("grenade becomes available exactly at its 12 second cooldown"));
    Check(Abilities.CommitAbility(EHRAbility::Deploy,true,true,true) && Abilities.bDeployed
        && !Abilities.CommitAbility(EHRAbility::Deploy,true,true,true), TEXT("deployment toggle has a one second commitment cooldown"));
    Abilities.Advance(1.f);
    Check(Abilities.CommitAbility(EHRAbility::Deploy,true,true,true) && !Abilities.bDeployed,
        TEXT("redeployment reverses the mode after cooldown without stacking bonuses"));

    FHRCommanderState Commander;
    Commander.Earn(200.f);
    Commander.Earn(NaN);
    Check(Commander.Points == 100.f && !Commander.TryActivate(EHRDoctrine::Scorch,0,true)
        && !Commander.TryActivate(EHRDoctrine::PrecisionStrike,0,false) && Commander.Points == 100.f
        && Commander.GlobalCooldown == 0.f, TEXT("commander cap and faction/target checks preserve points on rejected powers"));
    Check(Commander.TryActivate(EHRDoctrine::PrecisionStrike,0,true) && Commander.Points == 40.f
        && Commander.GlobalCooldown == 20.f && !Commander.TryActivate(EHRDoctrine::RepairPulse,0,true),
        TEXT("precision strike charges 60 points and blocks other powers with a global cooldown"));
    Commander.Advance(0.f);
    Check(Commander.Points == 40.f && Commander.GlobalCooldown == 20.f, TEXT("pause freezes commander points and global cooldown"));
    Commander.Advance(20.f);
    Check(Commander.Points == 45.f && Commander.GlobalCooldown == 0.f
        && Commander.TryActivate(EHRDoctrine::RepairPulse,0,true) && Commander.Points == 10.f,
        TEXT("commander earns 0.25 points per second and can commit repair pulse after global cooldown"));
    FHRCommanderState Ember;
    Ember.Points = 90.f;
    Check(Ember.TryActivate(EHRDoctrine::Scorch,1,true) && Ember.Points == 40.f
        && !Ember.TryActivate(EHRDoctrine::PrecisionStrike,1,true), TEXT("Ember doctrine wallet supports its own powers and rejects Aegis powers"));

    APrototypeGameMode* Mode = World ? World->GetAuthGameMode<APrototypeGameMode>() : nullptr;
    TArray<APrototypeUnit*> Fixtures;
    ON_SCOPE_EXIT
    {
        for (APrototypeUnit* Actor : Fixtures)
            if (IsValid(Actor) && !Actor->IsActorBeingDestroyed()) Actor->Destroy();
    };
    auto Spawn = [&](int32 Team, EPrototypeRole Role, const FVector& Position) -> APrototypeUnit*
    {
        if (!Mode) return nullptr;
        APrototypeUnit* Unit = Mode->SpawnUnit(Team,Position);
        if (!Unit) return nullptr;
        Unit->ConfigureRole(Role);
        Unit->SetActorTickEnabled(false);
        Fixtures.Add(Unit);
        return Unit;
    };
    APrototypeUnit* A = Spawn(0,EPrototypeRole::Combat,FVector(-11000,-8000,35));
    APrototypeUnit* B = Spawn(1,EPrototypeRole::Combat,FVector(-10700,-8000,35));
    if (!A || !B)
    {
        Check(false, TEXT("live tactical fixtures spawn in the game world"));
    }
    else
    {
        B->Health=7.f;
        const float Damage = B->ReceiveCombatHit(200.f,EHRDamageType::Kinetic,A);
        Check(Damage == 7.f && A->CombatState.Experience == 57.f
            && B->ReceiveCombatHit(200.f,EHRDamageType::Kinetic,A) == 0.f && A->CombatState.Experience == 57.f,
            TEXT("live damage awards actual health debit plus one kill bonus; overkill/replay earn no extra XP"));
        APrototypeUnit* C = Spawn(1,EPrototypeRole::Combat,FVector(-10700,-8000,35));
        if (C)
        {
            const float Initial = C->Health;
            const float Expected = FHRDamageTable::Resolve(10.f,EHRDamageType::Tempest,EHRArmorClass::Vehicle,
                C->GetEffectiveArmor(),false,Mode->GetDamageMatrix());
            const float Given = C->ReceiveCombatHit(10.f,EHRDamageType::Tempest,A);
            Check(Close(Given,Expected) && Close(Initial-C->Health,Given),
                TEXT("live unit damage consumes saved matrix and faction armor while conserving health loss"));
            const float HealthBeforeInvalid = C->Health;
            C->ReceiveCombatHit(NaN,EHRDamageType::Tempest,A);
            C->ReceiveHit(NaN);
            Check(C->Health == HealthBeforeInvalid, TEXT("live direct and typed damage reject NaN without corrupting health"));
            C->Destroy();
        }
        else Check(false,TEXT("live typed damage victim spawns"));
        A->Destroy();
    }

    APrototypeUnit* Cover = Spawn(0,EPrototypeRole::Depot,FVector(-10800,-6000,35));
    APrototypeUnit* Infantry = Spawn(0,EPrototypeRole::Infantry,FVector(-10800,-6600,35));
    if (Cover && Infantry)
    {
        Check(Infantry->IsInCover(), TEXT("live infantry detects completed friendly building boundary as cover"));
        Cover->ConstructionProgress=.5f;
        Check(!Infantry->IsInCover(), TEXT("unfinished building does not supply infantry cover"));
        Cover->ConstructionProgress=1.f;
        Cover->Team=1;
        Check(!Infantry->IsInCover(), TEXT("enemy building does not supply friendly infantry cover"));
        Cover->Team=0;
        const float Before = Infantry->Health;
        const float Expected = FHRDamageTable::Resolve(10.f,EHRDamageType::Kinetic,EHRArmorClass::Infantry,
            Infantry->GetEffectiveArmor(),true,Mode->GetDamageMatrix());
        const float Given = Infantry->ReceiveCombatHit(10.f,EHRDamageType::Kinetic,nullptr);
        Check(Close(Given,Expected) && Close(Before-Infantry->Health,Given) && Infantry->CombatState.Suppression > 0.f,
            TEXT("live covered infantry takes reduced damage and accumulates suppression"));
        const float NormalSpeed = Infantry->GetEffectiveSpeed();
        Infantry->CombatState.Suppression=100.f;
        Check(Close(Infantry->GetEffectiveSpeed(),NormalSpeed*.45f), TEXT("live suppressed infantry movement is reduced to 45 percent"));
        Infantry->CombatState.AddExperience(500.f,false);
        Infantry->CombatState.LastDamageAgo=3.9f;
        Infantry->CombatState.Suppression=0.f;
        Infantry->Health=Infantry->MaxHealth-10.f;
        Infantry->Tick(.2f);
        Check(Close(Infantry->Health,Infantry->MaxHealth-9.8f), TEXT("elite recovery counts only the portion of a tick after four combat-free seconds"));
        Cover->Destroy(); Infantry->Destroy();
    }
    else Check(false,TEXT("live infantry cover fixtures spawn"));

    APrototypeUnit* Siege = Spawn(0,EPrototypeRole::Siege,FVector(-11000,-7600,35));
    if (Siege)
    {
        const float NormalRange=Siege->GetEffectiveRange(), NormalArmor=Siege->GetEffectiveArmor();
        Check(Siege->TryActivateAbility(EHRAbility::Deploy) && Siege->CombatState.bDeployed
            && Close(Siege->GetEffectiveRange(),NormalRange*1.5f)
            && Close(Siege->GetEffectiveArmor(),NormalArmor*1.5f) && Siege->GetEffectiveSpeed()==0.f,
            TEXT("live siege deployment increases range and armor while immobilizing the unit"));
        const FVector Initial=Siege->GetActorLocation();
        Check(!Siege->OrderManualMove(Initial+FVector(100,0,0)), TEXT("deployed siege unit rejects movement without clearing deployment"));
        Siege->CombatState.Advance(1.f);
        Check(Siege->TryActivateAbility(EHRAbility::Deploy) && !Siege->CombatState.bDeployed
            && Close(Siege->GetEffectiveRange(),NormalRange) && Siege->GetEffectiveSpeed()>0.f,
            TEXT("live siege undeployment restores movement and base range"));
        Siege->Destroy();
    }
    else Check(false,TEXT("live siege fixture spawns"));

    APrototypeUnit* Grenadier = Spawn(0,EPrototypeRole::Infantry,FVector(-11200,-7000,35));
    if (Grenadier)
    {
        Mode->UpdateVisibility();
        Check(!Grenadier->TryActivateAbility(EHRAbility::Grenade) && Grenadier->CombatState.GrenadeCooldown==0.f,
            TEXT("live targetless grenade does not consume its cooldown"));
        APrototypeUnit* VictimOne = Spawn(1,EPrototypeRole::Combat,FVector(-10800,-7000,35));
        APrototypeUnit* VictimTwo = Spawn(1,EPrototypeRole::Combat,FVector(-10700,-7050,35));
        APrototypeUnit* Friendly = Spawn(0,EPrototypeRole::Combat,FVector(-10900,-7000,35));
        if (VictimOne && VictimTwo && Friendly)
        {
            VictimOne->Health=VictimOne->MaxHealth=1000.f;
            VictimTwo->Health=VictimTwo->MaxHealth=1000.f;
            Friendly->Health=Friendly->MaxHealth=1000.f;
            Mode->UpdateVisibility();
            const float ExpectedOne=FHRDamageTable::Resolve(55.f,EHRDamageType::Explosive,EHRArmorClass::Vehicle,
                VictimOne->GetEffectiveArmor(),false,Mode->GetDamageMatrix());
            const float ExpectedTwo=FHRDamageTable::Resolve(55.f,EHRDamageType::Explosive,EHRArmorClass::Vehicle,
                VictimTwo->GetEffectiveArmor(),false,Mode->GetDamageMatrix());
            Check(Grenadier->TryActivateAbility(EHRAbility::Grenade)
                && Close(VictimOne->Health,1000.f-ExpectedOne) && Close(VictimTwo->Health,1000.f-ExpectedTwo)
                && Friendly->Health==1000.f && Grenadier->CombatState.GrenadeCooldown==12.f,
                TEXT("live grenade applies explosive area damage to two enemies and preserves friendly health"));
            const float After=VictimOne->Health;
            Check(!Grenadier->TryActivateAbility(EHRAbility::Grenade) && VictimOne->Health==After,
                TEXT("live repeated grenade activation cannot deal damage during cooldown"));
            VictimOne->Destroy(); VictimTwo->Destroy(); Friendly->Destroy();
        }
        else Check(false,TEXT("live grenade area fixtures spawn"));
        Grenadier->Destroy();
    }
    else Check(false,TEXT("live grenadier fixture spawns"));

    APrototypeUnit* Smoker = Spawn(0,EPrototypeRole::Combat,FVector(-11000,-7400,35));
    APrototypeUnit* Observer = Spawn(1,EPrototypeRole::Combat,FVector(-10450,-7400,35));
    if (Smoker && Observer)
    {
        Mode->UpdateVisibility();
        Check(Mode->CanTeamSee(1,Smoker),TEXT("nearby enemy is visible before smoke activation"));
        const float HealthBeforeSmoke=Smoker->Health;
        Observer->OrderAttack(Smoker);
        Check(Smoker->TryActivateAbility(EHRAbility::Smoke) && Smoker->CombatState.SmokeCooldown==18.f,
            TEXT("live Aegis smoke creates a six second concealment volume and commits cooldown"));
        Mode->UpdateVisibility();
        Observer->Tick(.1f);
        Check(!Mode->CanTeamSee(1,Smoker) && Smoker->Health==HealthBeforeSmoke && !Observer->AttackTarget,
            TEXT("live smoke cancels a hidden attack target and prevents shooting through concealment"));
        Mode->Visibility.Advance(6.f);
        Mode->UpdateVisibility();
        Check(Mode->CanTeamSee(1,Smoker),TEXT("enemy visibility returns after six simulation seconds of smoke"));
        Observer->SetActorLocation(Smoker->GetActorLocation()+FVector(300,0,0));
        Mode->UpdateVisibility();
        Observer->OrderAttack(Smoker);
        Observer->Tick(1.1f);
        Check(Smoker->Health<HealthBeforeSmoke,TEXT("live weapon attack resumes after smoke expires"));
        Check(!Observer->TryActivateAbility(EHRAbility::Smoke) && Observer->CombatState.SmokeCooldown==0.f,
            TEXT("Ember cannot activate the Aegis-only smoke ability"));
        Smoker->Destroy(); Observer->Destroy();
    }
    else Check(false,TEXT("live smoke fixtures spawn"));

    UE_LOG(LogTemp, Display, TEXT("TACTICAL_TEST_COUNT=%d"), Count);
    UE_LOG(LogTemp, Display, TEXT("TACTICAL_TEST_RESULT=%s"), Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}
