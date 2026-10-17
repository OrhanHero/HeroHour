#include "HRProductionQueue.h"
#include "HRRTSDefinitions.h"
#include "HRRTSTags.h"
#include "../PrototypeGameMode.h"
#include "Engine/StaticMesh.h"
#include "Engine/Texture2D.h"
#include "Misc/ScopeExit.h"

bool RunHRRTSArchitectureSelfTests()
{
    bool Passed=true;
    int32 Count=0;
    auto Check=[&](bool Condition,const TCHAR* Label)
    {
        ++Count;Passed&=Condition;
        UE_LOG(LogTemp,Display,TEXT("ARCHITECTURE_TEST %s: %s"),Label,Condition?TEXT("PASS"):TEXT("FAIL"));
    };

    // These load the real saved packages, including in the packaged suite.
    // They exercise the live role lookup, not only a transient schema example.
    struct FRoleAssetFixture
    {
        EPrototypeRole Role;
        const TCHAR* Name;
        const TCHAR* RoleName;
        FGameplayTag UnitId;
        bool bStructure;
        int32 Cost;
        float Seconds;
        float Health;
        const TCHAR* MeshPath;
        const TCHAR* IconPath;
    };
    const FRoleAssetFixture RoleFixtures[] = {
        {EPrototypeRole::Combat,TEXT("DA_HR_Striker"),TEXT("Combat"),HRRTSTags::Unit_Striker,false,100,5.f,100.f,
            TEXT("/Game/HERO/ReferenceV7/Meshes/SM_HR_Aegis_APC8.SM_HR_Aegis_APC8"),
            TEXT("/Game/HERO/PresentationV5/Portraits/T_HR_Aegis_Combat.T_HR_Aegis_Combat")},
        {EPrototypeRole::Collector,TEXT("DA_HR_Harvester"),TEXT("Collector"),HRRTSTags::Unit_Harvester,false,150,6.f,100.f,
            TEXT("/Game/HERO/ConceptV2/Meshes/SM_HERO_Harvester.SM_HERO_Harvester"),
            TEXT("/Game/HERO/PresentationV5/Portraits/T_HR_Aegis_Collector.T_HR_Aegis_Collector")},
        {EPrototypeRole::Headquarters,TEXT("DA_HR_CommandHub"),TEXT("HQ"),HRRTSTags::Building_CommandHub,true,0,1.f,800.f,
            TEXT("/Game/HERO/TrellisHQV9/Meshes/SM_HQ9_AegisHQ.SM_HQ9_AegisHQ"),
            TEXT("/Game/HERO/PresentationV5/Portraits/T_HR_Aegis_HQ.T_HR_Aegis_HQ")},
        {EPrototypeRole::Depot,TEXT("DA_HR_Refinery"),TEXT("Depot"),HRRTSTags::Building_Refinery,true,200,8.f,400.f,
            TEXT("/Game/HERO/MilitaryDetailV6/Meshes/SM_MDV6_Refinery.SM_MDV6_Refinery"),
            TEXT("/Game/HERO/PresentationV5/Portraits/T_HR_Aegis_Depot.T_HR_Aegis_Depot")},
        {EPrototypeRole::Factory,TEXT("DA_HR_VehiclePlant"),TEXT("Factory"),HRRTSTags::Building_VehiclePlant,true,350,12.f,600.f,
            TEXT("/Game/HERO/TrellisFactoryV10/Meshes/SM_FV10_AegisFactory.SM_FV10_AegisFactory"),
            TEXT("/Game/HERO/PresentationV5/Portraits/T_HR_Aegis_Factory.T_HR_Aegis_Factory")},
    };
    TArray<const UHRUnitDefinition*> SavedRoles;
    for (const FRoleAssetFixture& Fixture : RoleFixtures)
    {
        const FString Path=FString::Printf(TEXT("/Game/HERO/DataV4/D_Aegis_%s.D_Aegis_%s"),Fixture.RoleName,Fixture.RoleName);
        const UHRUnitDefinition* Definition=LoadObject<UHRUnitDefinition>(nullptr,*Path,nullptr,LOAD_NoWarn);
        SavedRoles.Add(Definition);
        const UClass* ExpectedClass=Fixture.bStructure ? UHRBuildingDefinition::StaticClass() : UHRUnitDefinition::StaticClass();
        Check(Definition && Definition->GetClass()==ExpectedClass
            && APrototypeGameMode::GetRoleDefinition(Fixture.Role)==Definition
            && Definition->UnitId==Fixture.UnitId && Definition->FactionId==HRRTSTags::Faction_Aegis,
            *FString::Printf(TEXT("saved %s has exact class, native identity and live role binding"),Fixture.Name));
        Check(Definition && Definition->CreditCost==Fixture.Cost
            && FMath::IsNearlyEqual(Definition->ProductionSeconds,Fixture.Seconds)
            && FMath::IsNearlyEqual(Definition->Stats.Health,Fixture.Health)
            && FMath::IsNearlyEqual(APrototypeGameMode::RoleCost(Fixture.Role),static_cast<float>(Fixture.Cost))
            && FMath::IsNearlyEqual(APrototypeGameMode::RoleDuration(Fixture.Role),Fixture.Seconds),
            *FString::Printf(TEXT("saved %s preserves live costs, durations and health"),Fixture.Name));
        UStaticMesh* Mesh=Definition ? Definition->StaticMesh.LoadSynchronous() : nullptr;
        UTexture2D* Icon=Definition ? Definition->Icon.LoadSynchronous() : nullptr;
        Check(Mesh && Icon && Mesh->GetPathName()==Fixture.MeshPath && Icon->GetPathName()==Fixture.IconPath,
            *FString::Printf(TEXT("saved %s resolves its authored mesh and portrait soft references"),Fixture.Name));
    }

    const UHRFactionDefinition* SavedFaction=LoadObject<UHRFactionDefinition>(nullptr,
        TEXT("/Game/HERO/DataV4/D_Faction_Aegis.D_Faction_Aegis"),nullptr,LOAD_NoWarn);
    Check(SavedFaction && SavedFaction->GetClass()==UHRFactionDefinition::StaticClass()
        && SavedFaction->FactionId==HRRTSTags::Faction_Aegis
        && SavedFaction->EconomyStyle==EHREconomyStyle::RegrowingField
        && SavedFaction->BuildStyle==EHRBuildStyle::SidebarPlacement,
        TEXT("saved Aegis faction identifies current finite economy and sidebar construction metadata"));
    bool bRosterMatches=SavedFaction && SavedFaction->Units.Num()==4 && SavedFaction->Buildings.Num()==7;
    if (SavedFaction)
    {
        for(const auto& Ref:SavedFaction->Units) bRosterMatches &= Ref.LoadSynchronous()!=nullptr;
        for(const auto& Ref:SavedFaction->Buildings) bRosterMatches &= Ref.LoadSynchronous()!=nullptr;
    }
    Check(bRosterMatches,TEXT("saved faction resolves four mobile roles and seven structure roles"));
    const TCHAR* RoleNames[]={TEXT("Combat"),TEXT("Collector"),TEXT("HQ"),TEXT("Depot"),TEXT("Factory"),TEXT("PowerPlant"),TEXT("Barracks"),TEXT("Radar"),TEXT("Defense"),TEXT("Infantry"),TEXT("Siege")};
    for(int32 Team=0;Team<2;++Team)
    for(int32 Index=0;Index<11;++Index)
    {
        const UHRUnitDefinition* Def=APrototypeGameMode::GetRoleDefinition(static_cast<EPrototypeRole>(Index),Team);
        Check(Def && Def->GetOutermost()->GetName().StartsWith(TEXT("/Game/HERO/DataV4/"))
            && Def->FactionId==(Team==0?HRRTSTags::Faction_Aegis:HRRTSTags::Faction_Ember)
            && Def->StaticMesh.LoadSynchronous() && Def->Icon.LoadSynchronous(),
            *FString::Printf(TEXT("live team %d %s has saved faction mesh and icon"),Team,RoleNames[Index]));
    }

    const UHRTempestFieldDefinitionAsset* SavedField=LoadObject<UHRTempestFieldDefinitionAsset>(nullptr,
        TEXT("/Game/HERO/DataV3/DA_HR_TempestField.DA_HR_TempestField"),nullptr,LOAD_NoWarn);
    Check(SavedField && SavedField->GetClass()==UHRTempestFieldDefinitionAsset::StaticClass()
        && APrototypeGameMode::GetFieldDefinition()==SavedField,
        TEXT("saved resource settings load and bind to the live field getter"));
    Check(SavedField && SavedField->Settings.Capacity==4000.f && SavedField->Settings.InitialAmount==4000.f
        && SavedField->Settings.HarvestRate==20.f && SavedField->Settings.RegrowthPerSecond==2.f
        && SavedField->Settings.RegrowthDelay==10.f && SavedField->Settings.SeedFloor==0.f,
        TEXT("saved resource settings preserve baseline stock and explicitly authored optional growth"));

    const UHRDamageMatrix* SavedMatrix=LoadObject<UHRDamageMatrix>(nullptr,
        TEXT("/Game/HERO/DataV4/D_DamageMatrix.D_DamageMatrix"),nullptr,LOAD_NoWarn);
    Check(SavedMatrix && SavedMatrix->GetClass()==UHRDamageMatrix::StaticClass() && SavedMatrix->Rules.Num()==9,
        TEXT("saved damage matrix loads nine authored damage and armour pairs"));
    const float ExpectedPairs[3][3]={{1.1f,1.f,.5f},{1.5f,.85f,1.4f},{1.f,1.2f,.9f}};
    bool bNeutralPairs=SavedMatrix && SavedMatrix->Rules.Num()==9;
    const FGameplayTag DamageTags[]={HRRTSTags::Damage_Kinetic,HRRTSTags::Damage_Explosive,HRRTSTags::Damage_Tempest};
    const FGameplayTag ArmorTags[]={HRRTSTags::Armor_Infantry,HRRTSTags::Armor_Vehicle,HRRTSTags::Armor_Structure};
    if (SavedMatrix)
        for (int32 D=0;D<3;++D)
        for (int32 A=0;A<3;++A)
        {
            const FGameplayTag Damage=DamageTags[D],Armor=ArmorTags[A];
            int32 MatchingRules=0;
            for (const FHRDamageRule& SavedRule : SavedMatrix->Rules)
                if (SavedRule.DamageType==Damage && SavedRule.ArmorType==Armor)
                { ++MatchingRules;bNeutralPairs &= FMath::IsNearlyEqual(SavedRule.Multiplier,ExpectedPairs[D][A]); }
            bNeutralPairs &= MatchingRules==1 && FMath::IsNearlyEqual(SavedMatrix->GetDamageMultiplier(Damage,Armor),ExpectedPairs[D][A]);
        }
    Check(bNeutralPairs,TEXT("saved matrix has each native pair exactly once with the own three-type combat matrix"));

    // Direct field changes are deliberately transient. The scoped restore runs
    // before the remainder of this suite and never writes or dirties a package.
    UHRUnitDefinition* LiveStriker=const_cast<UHRUnitDefinition*>(APrototypeGameMode::GetRoleDefinition(EPrototypeRole::Combat));
    if (LiveStriker)
    {
        const int32 OriginalCost=LiveStriker->CreditCost;
        const float OriginalSeconds=LiveStriker->ProductionSeconds;
        {
            ON_SCOPE_EXIT
            {
                LiveStriker->CreditCost=OriginalCost;
                LiveStriker->ProductionSeconds=OriginalSeconds;
            };
            LiveStriker->CreditCost=OriginalCost+37;
            LiveStriker->ProductionSeconds=OriginalSeconds+.75f;
            Check(APrototypeGameMode::RoleCost(EPrototypeRole::Combat)==static_cast<float>(OriginalCost+37)
                && FMath::IsNearlyEqual(APrototypeGameMode::RoleDuration(EPrototypeRole::Combat),OriginalSeconds+.75f),
                TEXT("live role cost and duration respond to data changes instead of retained hardcoded values"));
        }
        Check(LiveStriker->CreditCost==OriginalCost && LiveStriker->ProductionSeconds==OriginalSeconds
            && APrototypeGameMode::RoleCost(EPrototypeRole::Combat)==static_cast<float>(OriginalCost)
            && FMath::IsNearlyEqual(APrototypeGameMode::RoleDuration(EPrototypeRole::Combat),OriginalSeconds),
            TEXT("transient live definition probe restores authored price and duration before other tests"));
    }
    else
    {
        Check(false,TEXT("live role cost and duration respond to data changes instead of retained hardcoded values"));
        Check(false,TEXT("transient live definition probe restores authored price and duration before other tests"));
    }

    UHRUnitDefinition* Unit=NewObject<UHRUnitDefinition>();
    Unit->UnitId=HRRTSTags::Unit_Striker;Unit->FactionId=HRRTSTags::Faction_Aegis;
    Unit->ProductionType=HRRTSTags::Production_Vehicles;Unit->CreditCost=100;Unit->ProductionSeconds=5;
    Unit->Prerequisites.AddTag(HRRTSTags::Tech_Factory);
    FHRProductionLane Vehicles,Infantry;
    Check(Vehicles.Initialize(HRRTSTags::Faction_Aegis,HRRTSTags::Production_Vehicles,2)
        && Infantry.Initialize(HRRTSTags::Faction_Aegis,HRRTSTags::Production_Infantry,2),TEXT("registered tags initialize independent category lanes"));
    int64 Credits=500;FGuid First,Second,Rejected;
    FGameplayTagContainer Tech;
    Check(!Vehicles.TryEnqueue(Unit,Tech,Credits,Rejected)&&Credits==500&&Vehicles.Num()==0&&!Rejected.IsValid(),
        TEXT("missing prerequisite rejects without credit or queue mutation"));
    Tech.AddTag(HRRTSTags::Tech_Factory);
    Check(!Infantry.TryEnqueue(Unit,Tech,Credits,Rejected)&&Credits==500&&Infantry.Num()==0,
        TEXT("vehicle cannot enter infantry lane"));
    Unit->FactionId=HRRTSTags::Faction_Ember;
    Check(!Vehicles.TryEnqueue(Unit,Tech,Credits,Rejected)&&Credits==500,TEXT("foreign faction definition rejects"));
    Unit->FactionId=HRRTSTags::Faction_Aegis;
    Credits=99;
    Check(!Vehicles.TryEnqueue(Unit,Tech,Credits,Rejected)&&Credits==99,TEXT("insufficient credits cannot enqueue"));
    Credits=500;
    Check(Vehicles.TryEnqueue(Unit,Tech,Credits,First)&&Vehicles.TryEnqueue(Unit,Tech,Credits,Second)
        &&Credits==300&&First.IsValid()&&Second.IsValid()&&First!=Second,TEXT("accepted orders charge exactly once and have unique identities"));
    Check(!Vehicles.TryEnqueue(Unit,Tech,Credits,Rejected)&&Credits==300&&Vehicles.Num()==2,
        TEXT("full category lane cannot charge for rejected order"));
    Unit->ProductionSeconds=900;
    FHRPowerGridState Power;Power.CurrentPower=50;Power.RequiredPower=100;
    Check(Power.GetState()==EHRPowerState::LowPower&&!Power.CanRunRadarAndDefense(),TEXT("insufficient power disables radar and defense"));
    Vehicles.Advance(4,Power.GetProductionRate());
    Check(FMath::IsNearlyEqual(Vehicles.GetOrders()[0].WorkSeconds,2)&&Vehicles.GetOrders()[1].WorkSeconds==0&&!Vehicles.PeekReady(),
        TEXT("low power halves work rate and queued item does not advance"));
    Power.CurrentPower=0;Vehicles.Advance(20,Power.GetProductionRate());
    Check(Power.GetState()==EHRPowerState::Offline&&Vehicles.GetOrders()[0].WorkSeconds==2,
        TEXT("zero power freezes paid active work without cancellation"));
    Power.CurrentPower=100;Vehicles.Advance(3,Power.GetProductionRate());
    Check(Power.CanRunRadarAndDefense()&&Vehicles.PeekReady()&&Vehicles.PeekReady()->OrderId==First,
        TEXT("restoring power resumes saved work and completes using original paid duration"));
    Vehicles.Advance(30,Power.GetProductionRate());
    Check(Vehicles.Num()==2&&Vehicles.PeekReady()&&Vehicles.PeekReady()->OrderId==First&&Credits==300
        &&Vehicles.GetOrders()[1].WorkSeconds==0,TEXT("blocked output keeps paid ready order and blocks subsequent work"));
    Check(!Vehicles.CommitSpawn(Second)&&Vehicles.Num()==2,TEXT("wrong completion identity cannot consume another order"));
    Check(Vehicles.CommitSpawn(First)&&!Vehicles.CommitSpawn(First)&&Vehicles.Num()==1&&Credits==300,
        TEXT("successful output commits exactly once without extra charge"));
    Unit->CreditCost=900;
    Check(Vehicles.Cancel(Second,Credits)&&Credits==400&&Vehicles.Num()==0&&!Vehicles.Cancel(Second,Credits)&&Credits==400,
        TEXT("cancel refunds original paid price once after definition price changes"));
    UHRUnitDefinition* InfantryUnit=NewObject<UHRUnitDefinition>();
    InfantryUnit->UnitId=HRRTSTags::Unit_Harvester;InfantryUnit->FactionId=HRRTSTags::Faction_Aegis;
    InfantryUnit->ProductionType=HRRTSTags::Production_Infantry;InfantryUnit->ProductionSeconds=4;
    Unit->CreditCost=100;Unit->ProductionSeconds=5;
    FGuid InfantryOrder,VehicleOrder;
    const bool Parallel=Infantry.TryEnqueue(InfantryUnit,Tech,Credits,InfantryOrder)
        &&Vehicles.TryEnqueue(Unit,Tech,Credits,VehicleOrder);
    Infantry.Advance(4,1);Vehicles.Advance(4,1);
    Check(Parallel&&Credits==200&&Infantry.PeekReady()&&!Vehicles.PeekReady()
        &&Vehicles.GetOrders()[0].WorkSeconds==4,TEXT("two production categories advance independently with one shared wallet"));
    Check(Infantry.Cancel(InfantryOrder,Credits)&&Vehicles.Cancel(VehicleOrder,Credits)&&Credits==400,
        TEXT("independent category cancellations restore both distinct payments"));
    UHRDamageMatrix* Matrix=NewObject<UHRDamageMatrix>();
    FHRDamageRule Rule;Rule.DamageType=HRRTSTags::Damage_Kinetic;Rule.ArmorType=HRRTSTags::Armor_Vehicle;Rule.Multiplier=.5f;
    Matrix->Rules.Add(Rule);
    Check(Matrix->GetDamageMultiplier(HRRTSTags::Damage_Kinetic,HRRTSTags::Armor_Vehicle)==.5f
        &&Matrix->GetDamageMultiplier(HRRTSTags::Damage_Explosive,HRRTSTags::Armor_Vehicle)==1.f,
        TEXT("damage matrix matches exact pair with neutral fallback"));
    UE_LOG(LogTemp,Display,TEXT("ARCHITECTURE_TEST_COUNT=%d"),Count);
    UE_LOG(LogTemp,Display,TEXT("ARCHITECTURE_TEST_RESULT=%s"),Passed?TEXT("PASS"):TEXT("FAIL"));
    return Passed;
}
