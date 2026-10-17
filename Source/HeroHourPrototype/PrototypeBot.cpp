#include "PrototypeGameMode.h"
#include "BattlefieldLayout.h"
#include "RTS/HRRTSDefinitions.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
bool BotLive(const APrototypeUnit* Unit)
{
    return IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health>0.f;
}

int32 RoleCount(APrototypeGameMode* Mode,int32 Team,EPrototypeRole Role,bool bIncludeQueue=true)
{
    int32 Count=0;
    for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        if (BotLive(*It) && It->Team==Team && It->Role==Role) ++Count;
    if (bIncludeQueue)
        for (const FPrototypeProduction& Order:Mode->ProductionQueue)
            if (BotLive(Order.Producer.Get()) && Order.Producer->Team==Team && Order.Role==Role) ++Count;
    return Count;
}

bool BeginBotBuilding(APrototypeGameMode* Mode,int32 Team,EPrototypeRole Role)
{
    const double Credits=Team==0 ? Mode->Credits : Mode->EnemyCredits;
    if (Credits<Mode->RoleCost(Role,Team)) return false;
    TArray<APrototypeUnit*> Anchors;
    APrototypeUnit* HQ=Team==0 ? Mode->PlayerHQ : Mode->EnemyHQ;
    if (BotLive(HQ) && HQ->IsCompleted()) Anchors.Add(HQ);
    for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        if (BotLive(*It) && It->Team==Team && It->IsStructure() && It->IsCompleted() && *It!=HQ)
            Anchors.Add(*It);
    const int32 SavedRotation=Mode->BuildingQuarterTurns;
    bool Built=false;
    // Bounded deterministic search. Start on the outside of the home base,
    // leaving its riverward side and the starting refinery route clear.
    const float Direction=Team==0 ? -1.f : 1.f;
    for (int32 AnchorIndex=0;AnchorIndex<FMath::Min(Anchors.Num(),6) && !Built;++AnchorIndex)
    for (float Radius:{1000.f,1400.f,1800.f,2200.f})
    {
        if (Built) break;
        for (int32 Step=0;Step<16 && !Built;++Step)
        {
            const float Angle=(-.6f+Step*2.f*PI/16.f);
            const FVector Site=Mode->SnapBuildPosition(Anchors[AnchorIndex]->GetActorLocation()
                +FVector(Direction*FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0));
            for (int32 Rotation=0;Rotation<2 && !Built;++Rotation)
            {
                Mode->BuildingQuarterTurns=Rotation;
                if (Mode->CanPlaceBuilding(Role,Site,Team)) Built=Mode->BeginBuilding(Role,Site,Team);
            }
        }
    }
    Mode->BuildingQuarterTurns=SavedRotation;
    return Built;
}

bool HasConstruction(APrototypeGameMode* Mode,int32 Team,bool bDefenseLane)
{
    for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        if (BotLive(*It) && It->Team==Team && It->IsStructure() && !It->IsCompleted()
            && (It->Role==EPrototypeRole::Defense)==bDefenseLane) return true;
    return false;
}
}

// Both the opponent and the soak-test player use the public match commands.
// No wallet grants, instant construction or free unit spawning are used here.
void RunPrototypeTeamBot(APrototypeGameMode* Mode,int32 Team)
{
    if (!Mode || Mode->Outcome!=0 || (Team!=0 && Team!=1) || Mode->BotDecisionTime>9000.f) return;
    APrototypeUnit* HQ=Team==0 ? Mode->PlayerHQ : Mode->EnemyHQ;
    APrototypeUnit* EnemyHQ=Team==0 ? Mode->EnemyHQ : Mode->PlayerHQ;
    if (!BotLive(HQ)) return;
    const bool bBaseLaneOccupied=HasConstruction(Mode,Team,false);
    const bool bDefenseLaneOccupied=HasConstruction(Mode,Team,true);
    // A small power reserve prevents a newly finished factory from freezing
    // construction before the first power plant can be completed.
    const bool bNeedPower=Mode->PowerGrid[Team].CurrentPower<Mode->PowerGrid[Team].RequiredPower+100;
    if (!bBaseLaneOccupied)
    {
        if (bNeedPower && RoleCount(Mode,Team,EPrototypeRole::PowerPlant,false)<4)
            BeginBotBuilding(Mode,Team,EPrototypeRole::PowerPlant);
        else if (RoleCount(Mode,Team,EPrototypeRole::Depot,false)<1)
            BeginBotBuilding(Mode,Team,EPrototypeRole::Depot);
        else if (RoleCount(Mode,Team,EPrototypeRole::Factory,false)<1)
            BeginBotBuilding(Mode,Team,EPrototypeRole::Factory);
        else if (RoleCount(Mode,Team,EPrototypeRole::Barracks,false)<1)
            BeginBotBuilding(Mode,Team,EPrototypeRole::Barracks);
        else if (RoleCount(Mode,Team,EPrototypeRole::Radar,false)<1 && RoleCount(Mode,Team,EPrototypeRole::Combat)>=4)
            BeginBotBuilding(Mode,Team,EPrototypeRole::Radar);
    }
    if (!bDefenseLaneOccupied && Mode->IsPowered(Team) && RoleCount(Mode,Team,EPrototypeRole::Defense,false)<1
        && RoleCount(Mode,Team,EPrototypeRole::Combat)>=5) BeginBotBuilding(Mode,Team,EPrototypeRole::Defense);

    if (RoleCount(Mode,Team,EPrototypeRole::Collector)<2) Mode->RequestProduction(EPrototypeRole::Collector,Team);
    if (RoleCount(Mode,Team,EPrototypeRole::Combat)<8) Mode->RequestProduction(EPrototypeRole::Combat,Team);
    if (RoleCount(Mode,Team,EPrototypeRole::Infantry)<3) Mode->RequestProduction(EPrototypeRole::Infantry,Team);
    if (RoleCount(Mode,Team,EPrototypeRole::Siege)<2 && RoleCount(Mode,Team,EPrototypeRole::Combat)>=5)
        Mode->RequestProduction(EPrototypeRole::Siege,Team);

    const int32 Combatants=RoleCount(Mode,Team,EPrototypeRole::Combat,false)
        +RoleCount(Mode,Team,EPrototypeRole::Infantry,false)+RoleCount(Mode,Team,EPrototypeRole::Siege,false);
    TArray<APrototypeUnit*> Army;
    for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        if (BotLive(*It) && It->Team==Team && It->IsCombatant()) Army.Add(*It);
    APrototypeUnit* FirstVisibleEnemy=nullptr;
    for (APrototypeUnit* Unit:Army)
    {
        APrototypeUnit* NearestEnemy=nullptr;
        float NearestDistance=TNumericLimits<float>::Max();
        for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        {
            if (!BotLive(*It) || It->Team==Team || !Mode->CanTeamSee(Team,*It)) continue;
            const float Distance=FVector::DistSquared2D(Unit->GetActorLocation(),It->GetActorLocation());
            if (Distance<NearestDistance) { NearestDistance=Distance; NearestEnemy=*It; }
        }
        if (NearestEnemy && !FirstVisibleEnemy) FirstVisibleEnemy=NearestEnemy;
        if (Unit->Role==EPrototypeRole::Infantry && NearestEnemy && NearestDistance<=FMath::Square(800.f))
            Unit->TryActivateAbility(EHRAbility::Grenade);
        if (Mode->GetFactionIndex(Team)==0 && NearestEnemy && Unit->Health<Unit->MaxHealth*.45f
            && NearestDistance>FMath::Square(400.f)) Unit->TryActivateAbility(EHRAbility::Smoke);
        if (Unit->Role==EPrototypeRole::Siege)
        {
            const bool bShouldDeploy=NearestEnemy && NearestDistance<=FMath::Square(Unit->GetEffectiveRange()*(Unit->CombatState.bDeployed?1.f:1.5f));
            if (bShouldDeploy!=Unit->CombatState.bDeployed) Unit->TryActivateAbility(EHRAbility::Deploy);
        }
        if (Unit->IsStructure()) continue;
        if (BotLive(EnemyHQ) && (Mode->MatchTime>=60.f && (Combatants>=7 || Mode->MatchTime>=110.f)) && !Unit->StrategicTarget)
            Unit->OrderAttack(EnemyHQ);
        // Move reinforcements away from a blocked production exit while keeping
        // their strategic target. This order is unnecessary once they march.
    }
    const int32 Faction=Mode->GetFactionIndex(Team);
    if (Faction==0)
    {
        APrototypeUnit* Damaged=nullptr;
        for (APrototypeUnit* Unit:Army)
            if (Unit->Health<Unit->MaxHealth*.55f) { Damaged=Unit; break; }
        if (Damaged) Mode->ActivateDoctrine(EHRDoctrine::RepairPulse,Team,Damaged->GetActorLocation());
        if (FirstVisibleEnemy) Mode->ActivateDoctrine(EHRDoctrine::PrecisionStrike,Team,FirstVisibleEnemy->GetActorLocation());
    }
    else
    {
        if (FirstVisibleEnemy) Mode->ActivateDoctrine(EHRDoctrine::Scorch,Team,FirstVisibleEnemy->GetActorLocation());
        if (FirstVisibleEnemy && !Army.IsEmpty()) Mode->ActivateDoctrine(EHRDoctrine::Overdrive,Team,Army[0]->GetActorLocation());
    }
}

void APrototypeGameMode::TickBot(float DeltaSeconds)
{
    if (Outcome!=0 || BotDecisionTime>9000.f || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds<=0.f
        || FParse::Param(FCommandLine::Get(),TEXT("PrototypeNoBot"))) return;
    BotDecisionTime-=DeltaSeconds;
    if (BotDecisionTime>0.f) return;
    BotDecisionTime=.75f;
    RunPrototypeTeamBot(this,1);
}
