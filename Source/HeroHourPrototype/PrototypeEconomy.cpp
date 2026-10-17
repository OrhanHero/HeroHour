#include "PrototypeGameMode.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "BattlefieldLayout.h"
#include "RTS/HRRTSDefinitions.h"
#include "RTS/HRRTSTags.h"
#include "RTS/ResourceFieldVisualPresenter.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include <cmath>
#include <limits>

namespace
{
bool FindDeliveryPoint(APrototypeGameMode* Mode,APrototypeUnit* Collector,APrototypeUnit* Depot,FVector& Out)
{
    if(!Mode->IsUnitOperational(Depot)) return false;
    const float Radius=FPrototypeNavigation::StructureApproachRadius(Depot,Collector->Destination-Depot->GetActorLocation(),90.f);
    const auto Obstacles=FPrototypeNavigation::GetWorldObstacles(Mode->GetWorld());
    const float GoalDistance=FVector::DistSquared2D(Collector->Destination,Depot->GetActorLocation());
    // Preserve a valid chosen dock approach across movement ticks.
    if (GoalDistance>=FMath::Square(Radius-5.f) && GoalDistance<=FMath::Square(Radius+5.f)
        && FPrototypeNavigation::IsWalkable(Collector->Destination,Obstacles)
        && FPrototypeNavigation::SegmentWorldWalkable(Mode->GetWorld(),Collector->Destination,Depot->GetActorLocation(),Depot))
    {
        TArray<FVector> Route;
        if (FPrototypeNavigation::FindWorldPath(Mode->GetWorld(),Collector->GetActorLocation(),Collector->Destination,Route))
        { Out=Collector->Destination;return true; }
    }
    const float Preferred=(Collector->GetActorLocation()-Depot->GetActorLocation()).Rotation().Yaw*PI/180.f;
    float Nearest=TNumericLimits<float>::Max();bool Found=false;
    for (int32 Index=0;Index<16;++Index)
    {
        const float Angle=Preferred+Index*2.f*PI/16.f;
        const FVector Direction(FMath::Cos(Angle),FMath::Sin(Angle),0);
        FVector Point=Depot->GetActorLocation()+Direction*FPrototypeNavigation::StructureApproachRadius(Depot,Direction,90.f);
        Point.Z=35.f;
        const float Distance=FVector::DistSquared2D(Collector->GetActorLocation(),Point);
        TArray<FVector> Route;
        if (Distance<Nearest
            && FPrototypeNavigation::SegmentWorldWalkable(Mode->GetWorld(),Point,Depot->GetActorLocation(),Depot)
            && FPrototypeNavigation::FindWorldPath(Mode->GetWorld(),Collector->GetActorLocation(),Point,Route))
        { Nearest=Distance;Out=Point;Found=true; }
    }
    return Found;
}
bool Alive(const APrototypeUnit* Unit)
{
    return IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health > 0.f;
}

double& Wallet(APrototypeGameMode* Mode, int32 Team)
{
    return Team == 0 ? Mode->Credits : Mode->EnemyCredits;
}

bool FindSpawnPoint(APrototypeGameMode* Mode, APrototypeUnit* Producer, FVector& Out)
{
    for (int32 Ring = 0; Ring < 3; ++Ring)
    {
        for (int32 Index = 0; Index < 16; ++Index)
        {
            const float Angle = Index * 2.f * PI / 16.f;
            const FVector Direction(FMath::Cos(Angle),FMath::Sin(Angle),0);
            const float Radius = FPrototypeNavigation::StructureApproachRadius(Producer,Direction,90.f) + 40.f + Ring * 100.f;
            FVector Point = Producer->GetActorLocation() + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
            Point.Z = 35.f;
            if (!FPrototypeNavigation::IsWalkable(Point, FPrototypeNavigation::GetWorldObstacles(Mode->GetWorld()))) continue;
            bool bFree = true;
            for (TActorIterator<APrototypeUnit> It(Mode->GetWorld()); It; ++It)
            {
                if (!Alive(*It)) continue;
                const float Minimum = It->IsStructure() ? APrototypeGameMode::Footprint(It->Role) + 95.f : 170.f;
                if (FVector::DistSquared2D(Point, It->GetActorLocation()) < FMath::Square(Minimum)) { bFree = false; break; }
            }
            if (bFree) { Out = Point; return true; }
        }
    }
    return false;
}
}

float APrototypeGameMode::Footprint(EPrototypeRole InRole)
{
    switch (InRole)
    {
    case EPrototypeRole::Headquarters: return 600.f;
    case EPrototypeRole::Depot: return 400.f;
    case EPrototypeRole::Factory: return 540.f;
    case EPrototypeRole::PowerPlant: return 300.f;
    case EPrototypeRole::Barracks: return 300.f;
    case EPrototypeRole::Radar: return 300.f;
    case EPrototypeRole::Defense: return 180.f;
    default: return 80.f;
    }
}

float APrototypeGameMode::RoleCost(EPrototypeRole InRole, int32 Team)
{
    if (const UHRUnitDefinition* Definition = GetRoleDefinition(InRole, Team))
        return FMath::Max(0, Definition->CreditCost);
    switch (InRole)
    {
    case EPrototypeRole::Depot: return 200.f;
    case EPrototypeRole::Factory: return 350.f;
    case EPrototypeRole::Collector: return 150.f;
    case EPrototypeRole::Combat: return 100.f;
    default: return 0.f;
    }
}

float APrototypeGameMode::RoleDuration(EPrototypeRole InRole, int32 Team)
{
    if (const UHRUnitDefinition* Definition = GetRoleDefinition(InRole, Team))
        return FMath::Max(.01f, Definition->ProductionSeconds);
    switch (InRole)
    {
    case EPrototypeRole::Depot: return 8.f;
    case EPrototypeRole::Factory: return 12.f;
    case EPrototypeRole::Collector: return 6.f;
    default: return 5.f;
    }
}

APrototypeUnit* APrototypeGameMode::FindCompletedStructure(int32 Team, EPrototypeRole InRole, const FVector& Near) const
{
    APrototypeUnit* Result = nullptr;
    float Nearest = TNumericLimits<float>::Max();
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Alive(*It) || It->Team != Team || It->Role != InRole || !It->IsCompleted()) continue;
        const float Distance = FVector::DistSquared2D(Near, It->GetActorLocation());
        if (Distance < Nearest) { Nearest = Distance; Result = *It; }
    }
    return Result;
}

bool APrototypeGameMode::CanPlaceBuilding(EPrototypeRole InRole, const FVector& Position, int32 Team) const
{
    if (Outcome != 0 || GetWorld()->IsPaused() || Position.ContainsNaN() || (Team != 0 && Team != 1)
        || static_cast<int32>(InRole)<static_cast<int32>(EPrototypeRole::Depot)
        || static_cast<int32>(InRole)>static_cast<int32>(EPrototypeRole::Defense)) return false;
    const FVector Site=SnapBuildPosition(Position);
    const APrototypeUnit* HQ = Team == 0 ? PlayerHQ : EnemyHQ;
    if (!Alive(HQ) || !HQ->IsCompleted() || !BattlefieldLayout::IsLand(Site, Footprint(InRole) + BattlefieldLayout::VehicleRadius)
        || !FPrototypeNavigation::IsWalkable(Site, FPrototypeNavigation::GetWorldObstacles(GetWorld()))) return false;
    TArray<const APrototypeUnit*> Connected;Connected.Add(HQ);
    for (int32 Index=0;Index<Connected.Num();++Index)
    {
        const APrototypeUnit* Source=Connected[Index];
        const auto* Def=Cast<UHRBuildingDefinition>(Source->Definition);
        const float Radius=Def?Def->ConnectedBuildRadiusCm:2300.f;
        for (TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
            if (Alive(*It) && It->Team==Team && It->IsStructure() && It->IsCompleted() && !Connected.Contains(*It)
                && FVector::DistSquared2D(Source->GetActorLocation(),It->GetActorLocation())<=FMath::Square(Radius)) Connected.Add(*It);
    }
    bool bConnected=false;
    for (const APrototypeUnit* Source:Connected)
    {
        const auto* Def=Cast<UHRBuildingDefinition>(Source->Definition);
        bConnected |= FVector::DistSquared2D(Site,Source->GetActorLocation())<=FMath::Square(Def?Def->ConnectedBuildRadiusCm:2300.f);
    }
    if (!bConnected || !BattlefieldLayout::IsStaticClear(Site,Footprint(InRole)+30.f)) return false;
    const FIntPoint Dimensions=GetBuildDimensions(InRole,BuildingQuarterTurns,Team);
    const FVector Half(Dimensions.X*100.f,Dimensions.Y*100.f,0);
    // Reserve the entire grid rectangle immediately, including construction sites.
    for (float X=-Half.X;X<=Half.X;X+=200.f)
    for (float Y=-Half.Y;Y<=Half.Y;Y+=200.f)
        if (!BattlefieldLayout::IsLand(Site+FVector(X,Y,0),80.f)
            || !BattlefieldLayout::IsStaticClear(Site+FVector(X,Y,0),30.f)) return false;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Alive(*It)) continue;
        const float Radius = Footprint(InRole) + Footprint(It->Role) + 30.f;
        if (FVector::DistSquared2D(Site, It->GetActorLocation()) < FMath::Square(Radius)) return false;
        const FVector UnitDelta=Site-It->GetActorLocation();
        if(!It->IsStructure() && FMath::Abs(UnitDelta.X)<Half.X+BattlefieldLayout::VehicleRadius+30.f
            && FMath::Abs(UnitDelta.Y)<Half.Y+BattlefieldLayout::VehicleRadius+30.f) return false;
        if (It->IsStructure())
        {
            const FIntPoint Other=GetBuildDimensions(It->Role,It->BuildRotation,It->Team);
            const FVector Delta=Site-It->GetActorLocation();
            if (FMath::Abs(Delta.X)<Half.X+Other.X*100.f+30.f && FMath::Abs(Delta.Y)<Half.Y+Other.Y*100.f+30.f) return false;
        }
    }
    return CanPlaceOverFields(InRole,Site,Team);
}

bool APrototypeGameMode::BeginBuilding(EPrototypeRole InRole, const FVector& Position, int32 Team)
{
    if (!CanPlaceBuilding(InRole, Position, Team))
    {
        if (Team == 0)
        {
            if (Outcome != 0) Feedback = TEXT("Construction unavailable after the match ends.");
            else if (!Alive(PlayerHQ)) Feedback = TEXT("Construction requires a surviving headquarters.");
            else if (!BattlefieldLayout::IsLand(Position, Footprint(InRole) + BattlefieldLayout::VehicleRadius)) Feedback = TEXT("Place structures on dry land inside the battlefield.");
            else if (FVector::DistSquared2D(Position, PlayerHQ->GetActorLocation()) > FMath::Square(2300.f)) Feedback = TEXT("Site too far away: build within 2300 units of your headquarters.");
            else Feedback = TEXT("Site obstructed: leave room around vehicles, structures and salvage.");
        }
        return false;
    }
    const float Cost = RoleCost(InRole,Team);
    if (Wallet(this, Team) < Cost)
    {
        if (Team == 0) Feedback = TEXT("Not enough salvage credits for this structure.");
        return false;
    }
    APrototypeUnit* Building = SpawnUnit(Team, SnapBuildPosition(Position));
    if (!Building) return false;
    Building->ConfigureRole(InRole);
    Building->ConstructionProgress = 0.f;
    Building->BuildRotation=BuildingQuarterTurns%4;
    Building->SetActorRotation(FRotator(0,Building->BuildRotation*90.f,0));
    Wallet(this, Team) -= Cost;
    if (Team == 0) Feedback = FString::Printf(TEXT("%s: Baustelle in Bauwarteschlange (%.0fs)."),*Building->Definition->DisplayName.ToString(),RoleDuration(InRole,Team));
    return true;
}

int32 APrototypeGameMode::GetQueueCount(const APrototypeUnit* Producer, int32 Team) const
{
    int32 Count = 0;
    for (const FPrototypeProduction& Item : ProductionQueue)
        if (Alive(Item.Producer.Get()) && (Producer ? Item.Producer.Get() == Producer : Item.Producer->Team == Team)) ++Count;
    return Count;
}

float APrototypeGameMode::GetProductionProgress(const APrototypeUnit* Producer) const
{
    for (const FPrototypeProduction& Item : ProductionQueue)
        if (Item.Producer.Get() == Producer) return FMath::Clamp(Item.Progress / Item.Duration, 0.f, 1.f);
    return 0.f;
}

bool APrototypeGameMode::RequestProduction(EPrototypeRole InRole, int32 Team)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || (InRole != EPrototypeRole::Combat && InRole != EPrototypeRole::Collector && InRole!=EPrototypeRole::Infantry && InRole!=EPrototypeRole::Siege) || (Team != 0 && Team != 1)) return false;
    const EPrototypeRole ProducerRole = InRole == EPrototypeRole::Collector ? EPrototypeRole::Headquarters : InRole==EPrototypeRole::Infantry ? EPrototypeRole::Barracks:EPrototypeRole::Factory;
    APrototypeUnit* Producer = nullptr;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == Team && It->Role == ProducerRole && It->IsCompleted() && GetQueueCount(*It, Team) < 5)
        { Producer = *It; break; }
    if (!Producer)
    {
        if (Team == 0) Feedback = InRole == EPrototypeRole::Combat ? TEXT("Build a completed factory first; each factory queue holds five.") : TEXT("HQ queue full or unavailable.");
        return false;
    }
    return RequestProductionAt(Producer, InRole);
}

bool APrototypeGameMode::RequestProductionAt(APrototypeUnit* Producer, EPrototypeRole InRole)
{
    const bool bSupported = ((InRole == EPrototypeRole::Combat || InRole==EPrototypeRole::Siege) && Alive(Producer) && Producer->Role == EPrototypeRole::Factory)
        || (InRole == EPrototypeRole::Collector && Alive(Producer) && Producer->Role == EPrototypeRole::Headquarters)
        || (InRole == EPrototypeRole::Infantry && Alive(Producer) && Producer->Role == EPrototypeRole::Barracks);
    if (Outcome != 0 || GetWorld()->IsPaused() || !bSupported || !Producer->IsCompleted() || (Producer->Team != 0 && Producer->Team != 1))
    {
        Feedback = TEXT("Selected building cannot produce this vehicle.");
        return false;
    }
    const int32 Team = Producer->Team;
    const UHRUnitDefinition* Definition=GetRoleDefinition(InRole,Team);
    if (Definition)
        for (const FGameplayTag& Required:Definition->Prerequisites)
        {
            const bool Available=(Required==HRRTSTags::Tech_Factory && FindCompletedStructure(Team,EPrototypeRole::Factory))
                || (Required==HRRTSTags::Tech_Barracks && FindCompletedStructure(Team,EPrototypeRole::Barracks));
            if (!Available) { if(Team==0) Feedback=TEXT("Voraussetzung fehlt: Produktionsgebäude fertigstellen.");return false; }
        }
    if (GetQueueCount(Producer, Team) >= 5)
    {
        if (Team == 0) Feedback = TEXT("Selected building queue is full (five vehicles).");
        return false;
    }
    int32 ArmyCount = 0;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == Team && !It->IsStructure()) ++ArmyCount;
    if (ArmyCount + GetQueueCount(nullptr, Team) >= 40)
    {
        if (Team == 0) Feedback = TEXT("Army capacity reached (40, including queued vehicles).");
        return false;
    }
    if (Wallet(this, Team) < RoleCost(InRole,Team))
    {
        if (Team == 0) Feedback = TEXT("Insufficient credits: deliver more salvage.");
        return false;
    }
    FPrototypeProduction Item;
    Item.Producer = Producer;
    Item.Role = InRole;
    Item.Duration = RoleDuration(InRole,Team);
    Item.PaidCost = RoleCost(InRole,Team);
    ProductionQueue.Add(Item);
    Wallet(this, Team) -= Item.PaidCost;
    if (Team == 0) Feedback = InRole == EPrototypeRole::Combat ? TEXT("Combat pickup queued (5 seconds).") : TEXT("Salvage collector queued (6 seconds).");
    return true;
}

bool APrototypeGameMode::CancelProduction(APrototypeUnit* Producer)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || !Alive(Producer) || (Producer->Team != 0 && Producer->Team != 1)) return false;
    // Own prototype rule: remove the latest unfinished order and refund its exact paid cost.
    for (int32 Index = ProductionQueue.Num() - 1; Index >= 0; --Index)
    {
        const FPrototypeProduction& Item = ProductionQueue[Index];
        if (Item.Producer.Get() != Producer) continue;
        Wallet(this, Producer->Team) += Item.PaidCost;
        ProductionQueue.RemoveAt(Index);
        if (Producer->Team == 0) Feedback = TEXT("Latest production order cancelled; credits refunded.");
        return true;
    }
    if (Producer->Team == 0) Feedback = TEXT("Selected building has no production order to cancel.");
    return false;
}

bool APrototypeGameMode::SetRallyPoint(APrototypeUnit* Producer, const FVector& Position)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || !Alive(Producer) || !Producer->IsCompleted()
        || (Producer->Role != EPrototypeRole::Factory && Producer->Role != EPrototypeRole::Headquarters && Producer->Role!=EPrototypeRole::Barracks)
        || (Producer->Team != 0 && Producer->Team != 1) || Position.ContainsNaN()) return false;
    const FVector Goal(Position.X, Position.Y, 35.f);
    const auto Obstacles = FPrototypeNavigation::GetWorldObstacles(GetWorld());
    if (!FPrototypeNavigation::IsWalkable(Goal, Obstacles))
    {
        if (Producer->Team == 0) Feedback = TEXT("Rally point rejected: destination blocked or on water.");
        return false;
    }
    // Validate from a navigable exit without making current parked vehicles a permanent restriction.
    bool bReachable = false;
    for (int32 Index = 0; Index < 16 && !bReachable; ++Index)
    {
        const float Angle = Index * 2.f * PI / 16.f;
        const float Radius = FPrototypeNavigation::StructureApproachRadius(Producer,FVector(FMath::Cos(Angle),FMath::Sin(Angle),0),90.f)+40.f;
        FVector Exit = Producer->GetActorLocation() + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0);
        Exit.Z = 35.f;
        if (!FPrototypeNavigation::IsWalkable(Exit, Obstacles)) continue;
        TArray<FVector> Route;
        bReachable = FPrototypeNavigation::FindWorldPath(GetWorld(), Exit, Goal, Route);
    }
    if (!bReachable)
    {
        if (Producer->Team == 0) Feedback = TEXT("Rally point rejected: no route from this building's exit.");
        return false;
    }
    Producer->RallyPoint = Goal;
    Producer->bHasRallyPoint = true;
    if (Producer->Team == 0) Feedback = TEXT("Rally point set: new vehicles move here automatically.");
    return true;
}

void APrototypeGameMode::ResetEconomy()
{
    RefineryDocks.Empty();
    ProductionQueue.Empty();
    for (FPrototypeSalvage& OldField : SalvageNodes)
    {
        if(OldField.FieldPresenter.IsValid())OldField.FieldPresenter->Destroy();
        for (const TWeakObjectPtr<AActor>& Visual : OldField.CellVisuals)
            if (Visual.IsValid()) Visual->Destroy();
    }
    SalvageNodes.Empty();
    UStaticMesh* PatchMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/DistrictV3/Meshes/SM_DV3_TempestPatch.SM_DV3_TempestPatch"));
    UStaticMesh* RootMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/SM_TFV10_OrganicRootSegment.SM_TFV10_OrganicRootSegment"));
    UStaticMesh* PodMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/SM_TFV10_SeedPod.SM_TFV10_SeedPod"));
    UMaterialInterface* RootMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/M_TFV10_OrganicRoot.M_TFV10_OrganicRoot"));
    UMaterialInterface* PodMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/M_TFV10_SeedPod.M_TFV10_SeedPod"));
    // Authored map caches are replaced once by the actual 48 spatial cells.
    for (TActorIterator<AActor> It(GetWorld());It;++It)
        for (int32 Index=0;Index<3;++Index)
            if (It->ActorHasTag(FName(*FString::Printf(TEXT("HERO_Salvage_%d"),Index))))
            { It->SetActorHiddenInGame(true);It->SetActorEnableCollision(false); }
    constexpr int32 CellCount=16;
    constexpr float Offsets[]={0.f,-260.f,260.f,520.f};
    for (int32 Index=0;Index<3;++Index)
    {
        FPrototypeSalvage Node;
        Node.Position = FVector(BattlefieldLayout::SalvageColumns[Index],BattlefieldLayout::SalvageRows[Index],35.f);
        // Keep the eastern field clear of the refinery's southern dock exit.
        if (Index==1) { Node.Position.X=6880.f;Node.Position.Y=550.f; }
        const float TotalCapacity=Index==2 ? 6000.f : 4000.f;
        FHRTempestFieldDefinition FieldSettings;
        if (const UHRTempestFieldDefinitionAsset* Definition = GetFieldDefinition()) FieldSettings = Definition->Settings;
        FieldSettings.Capacity=FieldSettings.InitialAmount=TotalCapacity/CellCount;
        // The saved rate is a field-wide budget, not sixteen copies of it.
        if (!GetFieldDefinition()) { FieldSettings.HarvestRate=20.f; FieldSettings.RegrowthPerSecond=2.f; }
        FieldSettings.RegrowthPerSecond/=CellCount;
        FieldSettings.SeedFloor/=CellCount;
        Node.Field.Reset(FieldSettings,FParse::Param(FCommandLine::Get(),TEXT("PrototypeFiniteFields"))
            ? EHREconomyStyle::FiniteSupply : EHREconomyStyle::RegrowingField,CellCount);
        Node.CellDepleted.Init(false,CellCount);
        for (int32 CellIndex=0;CellIndex<CellCount;++CellIndex)
        {
            const float EastOffsets[]={0.f,-200.f,200.f,400.f};
            Node.CellOffsets.Add(FVector(Index==1?EastOffsets[CellIndex%4]:Offsets[CellIndex%4],Offsets[CellIndex/4],0.f));
            FActorSpawnParameters Parameters;Parameters.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
            AStaticMeshActor* Visual=GetWorld()->SpawnActor<AStaticMeshActor>(Node.CellPosition(CellIndex)-FVector(0,0,35),FRotator(0,CellIndex*37.f,0),Parameters);
            if (Visual)
            {
                Visual->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
                Visual->GetStaticMeshComponent()->SetStaticMesh(PatchMesh);
                Visual->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                Visual->GetStaticMeshComponent()->SetCanEverAffectNavigation(false);
                Visual->SetActorScale3D(FVector(.4f,.4f,.55f));
                Visual->Tags.Add(FName(*FString::Printf(TEXT("HERO_Field_%d_Cell_%d"),Index,CellIndex)));
            }
            Node.CellVisuals.Add(Visual);
        }
        Node.VisualActor=Node.CellVisuals[0];
        if(RootMesh && PodMesh && RootMaterial && PodMaterial)
        {
            auto* Presenter=GetWorld()->SpawnActor<AResourceFieldVisualPresenter>();
            if(Presenter)
            {
                Presenter->Configure(RootMesh,PodMesh,RootMaterial,PodMaterial);
                if(Presenter->HasUsableAssets())Node.FieldPresenter=Presenter;
                else Presenter->Destroy();
            }
        }
        Node.RefreshAmount();
        SalvageNodes.Add(MoveTemp(Node));
    }
    Credits = EnemyCredits = 800.f;
    BotDecisionTime = 4.f;
    PlayerHQ = SpawnUnit(0, FVector(BattlefieldLayout::PlayerHQX, 0, 110), true);
    EnemyHQ = SpawnUnit(1, FVector(BattlefieldLayout::EnemyHQX, 0, 110), true);
    PlayerHQ->ConfigureRole(EPrototypeRole::Headquarters);
    EnemyHQ->ConfigureRole(EPrototypeRole::Headquarters);
    auto SpawnRole = [this](int32 Team, EPrototypeRole InRole, const FVector& Position)
    {
        APrototypeUnit* Unit = SpawnUnit(Team, Position);
        if (Unit) Unit->ConfigureRole(InRole);
        return Unit;
    };
    SpawnRole(0, EPrototypeRole::Depot, FVector(-7500, 1600, 35));
    SpawnRole(1, EPrototypeRole::Depot, FVector(7500, 1600, 35));
    // Both sides buy their first factory from the same 800-credit budget.
    // A free enemy factory previously imposed a hidden 350-credit/12s handicap.
    SpawnRole(0, EPrototypeRole::PowerPlant, FVector(-9000,-1600,35));
    SpawnRole(1, EPrototypeRole::PowerPlant, FVector(9000,-1600,35));
    for (int32 Index = 0; Index < 3; ++Index) SpawnRole(0, EPrototypeRole::Combat, FVector(-6800, (Index - 1) * 300.f, 35));
    for (int32 Index = 0; Index < 3; ++Index) SpawnRole(1, EPrototypeRole::Combat, FVector(6800, (Index - 1.f) * 300.f, 35));
    SpawnRole(0, EPrototypeRole::Collector, SalvageNodes[0].Position);
    SpawnRole(1, EPrototypeRole::Collector, SalvageNodes[1].Position);
    UpdateResourceVisuals();
}

bool APrototypeGameMode::CanPlaceOverFields(EPrototypeRole InRole,const FVector& Position,int32 Team) const
{
    const FIntPoint Cells=GetBuildDimensions(InRole,BuildingQuarterTurns,Team);
    for (const FPrototypeSalvage& Node : SalvageNodes)
        for (int32 CellIndex=0;CellIndex<Node.Field.Num();++CellIndex)
            if ((Node.Field.GetAmount(CellIndex)>0.f || Node.Field.GetStyle()==EHREconomyStyle::RegrowingField)
                && FMath::Abs(Position.X-Node.CellPosition(CellIndex).X)<Cells.X*100.f+95.f
                && FMath::Abs(Position.Y-Node.CellPosition(CellIndex).Y)<Cells.Y*100.f+95.f) return false;
    return true;
}

int32 APrototypeGameMode::GetFieldCellCount(int32 FieldIndex) const
{
    return SalvageNodes.IsValidIndex(FieldIndex) ? SalvageNodes[FieldIndex].Field.Num() : 0;
}

FVector APrototypeGameMode::GetFieldCellPosition(int32 FieldIndex,int32 CellIndex) const
{
    return SalvageNodes.IsValidIndex(FieldIndex) && SalvageNodes[FieldIndex].Field.GetCell(CellIndex)
        ? SalvageNodes[FieldIndex].CellPosition(CellIndex) : FVector::ZeroVector;
}

double APrototypeGameMode::GetFieldAmount(int32 FieldIndex) const
{
    return SalvageNodes.IsValidIndex(FieldIndex) ? SalvageNodes[FieldIndex].Field.GetTotalAmount() : 0.;
}

bool APrototypeGameMode::ResetFieldAmount(int32 FieldIndex,float TotalAmount)
{
    if (!SalvageNodes.IsValidIndex(FieldIndex) || !FMath::IsFinite(TotalAmount) || TotalAmount<0.f) return false;
    FPrototypeSalvage& Node=SalvageNodes[FieldIndex];
    double Left=TotalAmount;
    for (int32 CellIndex=0;CellIndex<Node.Field.Num();++CellIndex)
    {
        const float Amount=static_cast<float>(FMath::Min(Left,static_cast<double>(Node.Field.GetDefinition().Capacity)));
        Node.Field.SetAmount(CellIndex,Amount);Left-=Amount;
    }
    Node.RefreshAmount();
    return true;
}

void APrototypeGameMode::ReleaseSalvageReservation(APrototypeUnit* Unit, bool bReleaseDock)
{
    if (!Unit) return;
    if (bReleaseDock)
    {
        for (auto& Pair : RefineryDocks) Pair.Value.Release(Unit);
        Unit->DeliveryDepot.Reset();
        Unit->CollectorState = EPrototypeCollectorState::SeekingField;
    }
    for (FPrototypeSalvage& Node : SalvageNodes)
        Node.Field.Release(Unit);
    Unit->HarvestNode = INDEX_NONE;
    Unit->HarvestCell = INDEX_NONE;
    Unit->HarvestTimer = 0.f;
    Unit->HarvestRouteCheck = 0.f;
}

void APrototypeGameMode::TickCollector(APrototypeUnit* Unit, float DeltaSeconds)
{
    if (!Alive(Unit) || Unit->Role != EPrototypeRole::Collector) { ReleaseSalvageReservation(Unit); return; }
    if (Outcome != 0 || GetWorld()->IsPaused() || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds<=0.f || UGameplayStatics::IsGamePaused(this)) return;
    for (int32 NodeIndex=0;NodeIndex<SalvageNodes.Num();++NodeIndex)
    {
        FPrototypeSalvage& Node=SalvageNodes[NodeIndex];
        Node.Field.ReleaseInvalidReservations([&Node,NodeIndex](const AActor* Actor)
        {
            const APrototypeUnit* Owner=Cast<APrototypeUnit>(Actor);
            return Alive(Owner) && Owner->Role==EPrototypeRole::Collector && !Owner->bCollectorManualOrder
                && Owner->HarvestNode==NodeIndex && Node.Field.GetReservedCell(Owner)==Owner->HarvestCell;
        });
    }
    Unit->AttackTarget = nullptr;
    Unit->StrategicTarget = nullptr;
    if (Unit->bCollectorManualOrder)
    {
        ReleaseSalvageReservation(Unit);
        if (FVector::DistSquared2D(Unit->GetActorLocation(),Unit->Destination) <= FMath::Square(5.f))
            Unit->bCollectorManualOrder = false;
        return;
    }
    const bool bReturning = Unit->Cargo >= 100. || (Unit->Cargo > 0. && !Unit->bRetainedUnloadRemainder
        && (Unit->DeliveryDepot.IsValid() || !SalvageNodes.IsValidIndex(Unit->HarvestNode)
            || SalvageNodes[Unit->HarvestNode].Field.GetHarvestableAmount(Unit->HarvestCell) <= 0.f));
    if (bReturning)
    {
        ReleaseSalvageReservation(Unit, false);
        APrototypeUnit* Depot = Unit->DeliveryDepot.Get();
        FVector DeliveryPoint = Unit->DockApproach;
        TArray<FVector> Route;
        const auto DockReachable = [&](APrototypeUnit* Candidate, const FVector& Point)
        {
            return Alive(Candidate) && Candidate->Team == Unit->Team && Candidate->Role == EPrototypeRole::Depot
                && Candidate->IsCompleted() && IsUnitOperational(Candidate)
                && FPrototypeNavigation::SegmentWorldWalkable(GetWorld(), Point, Candidate->GetActorLocation(), Candidate)
                && FPrototypeNavigation::FindWorldPath(GetWorld(), Unit->GetActorLocation(), Point, Route);
        };
        if (!DockReachable(Depot, DeliveryPoint))
        {
            for (auto& Pair : RefineryDocks) Pair.Value.Release(Unit);
            Unit->DeliveryDepot.Reset(); Depot = nullptr;
            float Nearest = TNumericLimits<float>::Max();
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            {
                if (!Alive(*It) || It->Team != Unit->Team || It->Role != EPrototypeRole::Depot || !It->IsCompleted()
                    || !IsUnitOperational(*It)) continue;
                const float Distance = FVector::DistSquared2D(Unit->GetActorLocation(), It->GetActorLocation());
                FVector Candidate = FVector::ZeroVector;
                const bool AlreadyDocked = Distance <= FMath::Square(Footprint(It->Role) + 100.f)
                    && FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Unit->GetActorLocation(),It->GetActorLocation(),*It);
                if (Distance < Nearest && (AlreadyDocked || FindDeliveryPoint(this, Unit, *It, Candidate)))
                { Depot = *It; DeliveryPoint = AlreadyDocked ? Unit->GetActorLocation() : Candidate; Nearest = Distance; }
            }
            if (Depot) { Unit->DeliveryDepot = Depot; Unit->DockApproach = DeliveryPoint; }
        }
        if (!Depot) { Unit->CollectorState = EPrototypeCollectorState::MovingToRefinery; Unit->Destination = Unit->GetActorLocation(); return; }
        FHRRefineryDocking* Dock = RefineryDocks.Find(Depot);
        if (!Dock) { FHRRefineryDocking NewDock; NewDock.Reset(Depot, 1, 40.); Dock = &RefineryDocks.Add(Depot, MoveTemp(NewDock)); }
        Dock->ReleaseInvalid([&](AActor* Actor)
        {
            const APrototypeUnit* Owner = Cast<APrototypeUnit>(Actor);
            return Alive(Owner) && Owner->Role == EPrototypeRole::Collector && !Owner->bCollectorManualOrder
                && Owner->DeliveryDepot.Get() == Depot && Owner->Cargo > 0.;
        });
        Dock->Submit(Unit);
        if (Dock->IsDocked(Unit) || (Dock->GetNextWaiting() == Unit && Dock->NumDocked() == 0))
        {
            const bool bAtDock = FVector::DistSquared2D(Unit->GetActorLocation(), DeliveryPoint) <= FMath::Square(5.f)
                && FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Unit->GetActorLocation(),Depot->GetActorLocation(),Depot);
            if (Dock->IsDocked(Unit) && !bAtDock) { Dock->Release(Unit); Dock->Submit(Unit); }
            if ((Dock->IsDocked(Unit) && bAtDock) || Dock->TryAcquireNext(Unit, bAtDock))
            {
                Unit->CollectorState = EPrototypeCollectorState::Unloading;
                Unit->Destination = Unit->GetActorLocation(); Unit->bMoveOrdered = false;
                // The authored rear coupling faces the refinery while unloading.
                const FVector AwayFromDepot=Unit->GetActorLocation()-Depot->GetActorLocation();
                if (!AwayFromDepot.IsNearlyZero()) Unit->SetActorRotation(FRotator(0.f,AwayFromDepot.Rotation().Yaw,0.f));
                const double Delivered = Dock->Unload(Unit, DeltaSeconds, Unit->Cargo, Wallet(this, Unit->Team), ++Unit->UnloadOperation);
                // A sub-ULP residual is retained in cargo, never rounded into credits or discarded.
                // Free the scarce dock and merge that residual into the next harvested load.
                if (Delivered == 0. && Unit->Cargo > 0.
                    && Unit->Cargo < std::nextafter(Wallet(this,Unit->Team),std::numeric_limits<double>::infinity())-Wallet(this,Unit->Team))
                {
                    Dock->Release(Unit); Unit->DeliveryDepot.Reset();
                    Unit->bRetainedUnloadRemainder=true; Unit->CollectorState=EPrototypeCollectorState::SeekingField;
                    return;
                }
                if (Unit->Cargo <= 0.)
                { Dock->Release(Unit); Unit->DeliveryDepot.Reset(); Unit->CollectorState = EPrototypeCollectorState::SeekingField; }
                return;
            }
            Unit->CollectorState = EPrototypeCollectorState::MovingToRefinery;
            Unit->OrderMove(DeliveryPoint); return;
        }
        Unit->CollectorState = EPrototypeCollectorState::WaitingForDock;
        // Distinct, reachable staging points remain outside the refinery footprint and its unload lane.
        FVector WaitingPoint = Unit->GetActorLocation(); bool bFoundWaiting = false;

        // Presentation objects must not alter collector routes through UObject IDs.
        // Prefer the nearest spatial staging sector, then validate all sixteen slots.
        const FVector ToCollector=Unit->GetActorLocation()-Depot->GetActorLocation();
        const float Sector=2.f*PI/16.f;
        const float StartAngle=FMath::RoundToFloat(FMath::Atan2(ToCollector.Y,ToCollector.X)/Sector)*Sector;
        for (int32 Index = 0; Index < 16 && !bFoundWaiting; ++Index)
        {
            const float Angle = StartAngle + Index * 2.f * PI / 16.f;
            const FVector Direction(FMath::Cos(Angle),FMath::Sin(Angle),0);
            const float Radius=FPrototypeNavigation::StructureApproachRadius(Depot,Direction,90.f)+290.f;
            FVector Candidate = Depot->GetActorLocation() + Direction*Radius;
            Candidate.Z = 35.f; bool bOccupied = false;
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
                if (*It != Unit && Alive(*It) && !It->IsStructure()
                    && (FVector::DistSquared2D(It->GetActorLocation(),Candidate)<FMath::Square(180.f)
                        || (It->DeliveryDepot.Get()==Depot && FVector::DistSquared2D(It->Destination,Candidate)<FMath::Square(180.f)))) bOccupied=true;
            if (!bOccupied && FPrototypeNavigation::FindWorldPath(GetWorld(),Unit->GetActorLocation(),Candidate,Route))
            { WaitingPoint=Candidate; bFoundWaiting=true; }
        }
        if (bFoundWaiting) Unit->OrderMove(WaitingPoint);
        else { Unit->Destination=Unit->GetActorLocation(); Unit->bMoveOrdered=false; }
        return;
    }
    Unit->CollectorState = EPrototypeCollectorState::SeekingField;
    Unit->HarvestRouteCheck -= DeltaSeconds;
    if (SalvageNodes.IsValidIndex(Unit->HarvestNode) && Unit->HarvestRouteCheck <= 0.f)
    {
        TArray<FVector> Route;
        const FPrototypeSalvage& Assigned = SalvageNodes[Unit->HarvestNode];
        if (!Assigned.Field.HasValidReservation(Unit->HarvestCell,Unit) || Assigned.Field.GetHarvestableAmount(Unit->HarvestCell) <= 0.f
            || !FPrototypeNavigation::FindWorldPath(GetWorld(), Unit->GetActorLocation(), Assigned.CellPosition(Unit->HarvestCell), Route))
            ReleaseSalvageReservation(Unit, false);
        Unit->HarvestRouteCheck = 0.8f;
    }
    if (!SalvageNodes.IsValidIndex(Unit->HarvestNode) || !SalvageNodes[Unit->HarvestNode].Field.HasValidReservation(Unit->HarvestCell,Unit)
        || SalvageNodes[Unit->HarvestNode].Field.GetHarvestableAmount(Unit->HarvestCell) <= 0.f)
    {
        ReleaseSalvageReservation(Unit, false);
        float Nearest = TNumericLimits<float>::Max();
        for (int32 Index = 0; Index < SalvageNodes.Num(); ++Index)
        {
            FPrototypeSalvage& Candidate = SalvageNodes[Index];
            for (int32 CellIndex=0;CellIndex<Candidate.Field.Num();++CellIndex)
            {
                const FHRTempestCellState* Cell=Candidate.Field.GetCell(CellIndex);
                if (!Cell || Cell->ReservedCollector.IsValid() || Candidate.Field.GetHarvestableAmount(CellIndex)<=0.f) continue;
                const FVector Pickup=Candidate.CellPosition(CellIndex);
                const float Distance=FVector::DistSquared2D(Unit->GetActorLocation(),Pickup);
                TArray<FVector> Route;
                if (Distance<Nearest && FPrototypeNavigation::FindWorldPath(GetWorld(),Unit->GetActorLocation(),Pickup,Route))
                { Nearest=Distance;Unit->HarvestNode=Index;Unit->HarvestCell=CellIndex; }
            }
        }
        if (SalvageNodes.IsValidIndex(Unit->HarvestNode))
        {
            if(!SalvageNodes[Unit->HarvestNode].Field.TryReserve(Unit->HarvestCell,Unit))
            { Unit->HarvestNode=Unit->HarvestCell=INDEX_NONE;return; }
            Unit->HarvestRouteCheck = 0.8f;
        }
    }
    if (!SalvageNodes.IsValidIndex(Unit->HarvestNode)) { Unit->Destination = Unit->GetActorLocation(); return; }
    FPrototypeSalvage& Node = SalvageNodes[Unit->HarvestNode];
    const FVector Pickup=Node.CellPosition(Unit->HarvestCell);
    if (FVector::DistSquared2D(Unit->GetActorLocation(), Pickup) > FMath::Square(105.f))
    {
        Unit->OrderMove(Pickup);
        return;
    }
    Unit->Destination = Unit->GetActorLocation();
    Unit->CollectorState = EPrototypeCollectorState::Harvesting;
    Unit->HarvestTimer += DeltaSeconds;
    if (Unit->HarvestTimer >= 0.5f)
    {
        const int32 Pulses = FMath::FloorToInt(Unit->HarvestTimer / 0.5f);
        Unit->HarvestTimer -= Pulses * 0.5f;
        const double FreeCargo=FMath::Max(0.,100.-Unit->Cargo);
        float RepresentableFreeCargo=static_cast<float>(FreeCargo);
        if(static_cast<double>(RepresentableFreeCargo)>FreeCargo)
            RepresentableFreeCargo=std::nextafter(RepresentableFreeCargo,0.f);
        const float Amount = Node.Field.Harvest(Unit->HarvestCell,Unit,RepresentableFreeCargo,Pulses*.5f);
        Unit->Cargo += Amount;
        if (Amount > 0.f) Unit->bRetainedUnloadRemainder=false;
        Node.RefreshAmount();
        const float CellAmount=Node.Field.GetAmount(Unit->HarvestCell);
        const double DebitQuantum=static_cast<double>(CellAmount)-std::nextafter(CellAmount,0.f);
        const bool bCargoCannotFitDebit=Amount==0.f && Unit->Cargo>0. && FreeCargo<DebitQuantum;
        if (Unit->Cargo >= 100.f || bCargoCannotFitDebit || Node.Field.GetHarvestableAmount(Unit->HarvestCell) <= 0.f)
            ReleaseSalvageReservation(Unit, false);
        UpdateResourceVisuals();
    }
}

void APrototypeGameMode::UpdateResourceVisuals()
{
    for (int32 Index=0; Index<SalvageNodes.Num(); ++Index)
    {
        FPrototypeSalvage& Node=SalvageNodes[Index];
        TArray<FResourceFieldVisualSample> PresentationSamples;
        const bool bPresenterReady=Node.FieldPresenter.IsValid() && Node.FieldPresenter->HasUsableAssets();
        bool bDepleted=true;
        for (int32 CellIndex=0;CellIndex<Node.Field.Num();++CellIndex)
            bDepleted &= Node.Field.GetHarvestableAmount(CellIndex)<=0.f;
        if (Node.bVisualDepleted!=bDepleted)
        {
            Node.bVisualDepleted=bDepleted;
            OnResourceDepletionChanged.Broadcast(Index,bDepleted);
        }
        for (int32 CellIndex=Node.Field.Num();CellIndex<Node.CellVisuals.Num();++CellIndex)
            if (Node.CellVisuals[CellIndex].IsValid()) Node.CellVisuals[CellIndex]->SetActorHiddenInGame(true);
        for (int32 CellIndex=0;CellIndex<Node.Field.Num();++CellIndex)
        {
            const bool bCellDepleted=Node.Field.GetHarvestableAmount(CellIndex)<=0.f;
            if (Node.CellDepleted.IsValidIndex(CellIndex)) Node.CellDepleted[CellIndex]=bCellDepleted;
            AActor* Visual=Node.CellVisuals.IsValidIndex(CellIndex) ? Node.CellVisuals[CellIndex].Get() : nullptr;
            if (!Visual) continue;
            Visual->SetActorLocation(Node.CellPosition(CellIndex)-FVector(0,0,35));
            Visual->SetActorHiddenInGame(bCellDepleted && Node.Field.GetStyle()==EHREconomyStyle::FiniteSupply);
            if (AStaticMeshActor* Patch=Cast<AStaticMeshActor>(Visual))
            {
                const FHRTempestFieldDefinition& Settings=Node.Field.GetDefinition();
                const float ProtectedSeed=Node.Field.GetStyle()==EHREconomyStyle::RegrowingField ? Settings.SeedFloor : 0.f;
                const float UsableCapacity=FMath::Max(0.f,Settings.Capacity-ProtectedSeed);
                const float Activity=UsableCapacity>0.f ? FMath::Clamp(Node.Field.GetHarvestableAmount(CellIndex)/UsableCapacity,0.f,1.f) : 0.f;
                // Preserve legacy actor depletion flags/scales and pickup paths;
                // only the optional old patch renderer is replaced.
                Patch->GetStaticMeshComponent()->SetVisibility(!bPresenterReady);
                if(bPresenterReady)
                {
                    const FVector Position=Node.CellPosition(CellIndex);
                    const bool bArtReview=FParse::Param(FCommandLine::Get(),TEXT("PrototypeArtReview"));
                    FResourceFieldVisualSample Sample;
                    Sample.CellId=CellIndex;Sample.Position=Position;Sample.Activity=Activity;
                    Sample.bCurrentlyVisible=bArtReview || Visibility.IsVisible(0,Position);
                    Sample.bExplored=bArtReview || Visibility.IsExplored(0,Position);
                    Sample.bFiniteDepleted=bCellDepleted && Node.Field.GetStyle()==EHREconomyStyle::FiniteSupply;
                    PresentationSamples.Add(Sample);
                }
                Patch->GetStaticMeshComponent()->SetCustomPrimitiveDataFloat(0,Activity);
                // Actual geometry changes in three readable growth stages while
                // exhausted regrowing cells retain their subdued root bed.
                const float Height=Activity<=0.f ? .22f : Activity<.34f ? .32f : Activity<.67f ? .43f : .55f;
                Patch->SetActorScale3D(FVector(.4f,.4f,Height));
            }
        }
        if(bPresenterReady)Node.FieldPresenter->SubmitSnapshot(PresentationSamples);
    }
}

void APrototypeGameMode::TickEconomy(float DeltaSeconds)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.f || UGameplayStatics::IsGamePaused(this)) return;
    for (auto It = RefineryDocks.CreateIterator(); It; ++It)
    {
        APrototypeUnit* Depot = It.Key().Get();
        It.Value().ReleaseInvalid([Depot](AActor* Actor)
        {
            const APrototypeUnit* Owner = Cast<APrototypeUnit>(Actor);
            return Alive(Depot) && Depot->Role==EPrototypeRole::Depot && Depot->IsCompleted() && !Depot->bManuallyPoweredDown
                && Alive(Owner) && Owner->Role==EPrototypeRole::Collector && !Owner->bCollectorManualOrder
                && Owner->DeliveryDepot.Get()==Depot && Owner->Team==Depot->Team && Owner->Cargo>0.;
        });
        if (!Alive(Depot) || Depot->Role!=EPrototypeRole::Depot) It.RemoveCurrent();
    }
    RefreshPowerGrid();
    MatchTime += DeltaSeconds;
    for(FPrototypeSalvage& Node:SalvageNodes)
    {
        Node.Field.Advance(DeltaSeconds);
        Node.RefreshAmount();
    }
    UpdateResourceVisuals();
    TSet<int32> ConstructionLanes;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->IsStructure() && !It->IsCompleted())
        {
            const int32 Lane=It->Team*3+(It->Role==EPrototypeRole::PowerPlant?2:It->Role==EPrototypeRole::Defense?1:0);
            if (ConstructionLanes.Contains(Lane)) continue;
            ConstructionLanes.Add(Lane);
            const auto* Definition=Cast<UHRBuildingDefinition>(It->Definition);
            const float Rate=(Definition && Definition->PowerSupply>0)?1.f:GetProductionRate(It->Team);
            It->ConstructionProgress = FMath::Min(1.f, It->ConstructionProgress + DeltaSeconds*Rate / RoleDuration(It->Role,It->Team));
        }
    TSet<APrototypeUnit*> Advanced;
    for (int32 Index = 0; Index < ProductionQueue.Num();)
    {
        FPrototypeProduction& Item = ProductionQueue[Index];
        APrototypeUnit* Producer = Item.Producer.Get();
        if (!Alive(Producer)) { ProductionQueue.RemoveAt(Index); continue; }
        if (!Producer->IsCompleted() || Advanced.Contains(Producer)) { ++Index; continue; }
        Advanced.Add(Producer);
        Item.Progress += DeltaSeconds*(Producer->bManuallyPoweredDown?0.f:GetProductionRate(Producer->Team));
        if (Item.Progress >= Item.Duration)
        {
            FVector Position;
            if (FindSpawnPoint(this, Producer, Position))
            {
                APrototypeUnit* Produced = SpawnUnit(Producer->Team, Position);
                if (Produced)
                {
                    Produced->ConfigureRole(Item.Role);
                    if (Producer->bHasRallyPoint && !Produced->OrderManualMove(Producer->RallyPoint) && Producer->Team == 0)
                        Feedback = TEXT("Rally point blocked: vehicle waiting at production exit.");
                    if (Produced->Team == 1 && Item.Role == EPrototypeRole::Combat && MatchTime >= 60.f) Produced->OrderAttack(PlayerHQ);
                    ProductionQueue.RemoveAt(Index);
                    continue;
                }
            }
            if (Producer->Team == 0) Feedback = TEXT("Exit obstructed: move vehicles away from the production building.");
        }
        ++Index;
    }
    TickBot(DeltaSeconds);
}

void APrototypeGameMode::RunEconomyTest()
{
    bool bPass = true;
    auto Check = [&bPass](bool Condition, const TCHAR* Label)
    {
        bPass &= Condition;
        UE_LOG(LogTemp, Display, TEXT("ECONOMY_TEST %s: %s"), Label, Condition ? TEXT("PASS") : TEXT("FAIL"));
    };
    const auto IsReachableDockPosition = [this](const APrototypeUnit* Depot, const FVector& Position)
    {
        if (!Alive(Depot)) return false;
        const FVector Offset=Position-Depot->GetActorLocation();
        const float Radius=FPrototypeNavigation::StructureApproachRadius(Depot,Offset,90.f);
        return FMath::Abs(Offset.Size2D()-Radius)<=5.f
            && FPrototypeNavigation::IsWalkable(Position,FPrototypeNavigation::GetWorldObstacles(GetWorld()))
            && FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Position,Depot->GetActorLocation(),Depot);
    };
    BotDecisionTime = 10000.f;
    const float Initial = Credits;
    TickEconomy(1.f);
    Check(Credits == Initial, TEXT("no passive income without delivery"));
    APrototypeUnit* Collector = nullptr;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (It->Team == 0 && It->Role == EPrototypeRole::Collector) Collector = *It;
    if (Collector)
    {
        Collector->SetActorTickEnabled(false);
        Collector->SetActorLocation(SalvageNodes[0].Position);
        const float Before = SalvageNodes[0].Remaining;
        TickCollector(Collector, 5.f);
        Check(Collector->Cargo == 100.f && SalvageNodes[0].Remaining == Before - 100.f && Credits == Initial,
            TEXT("timed harvesting depletes salvage and fills cargo without instant credit"));
        APrototypeUnit* Depot = FindCompletedStructure(0, EPrototypeRole::Depot);
        const FVector ManualGoal = Collector->GetActorLocation()+FVector(0,-500,0);
        Check(Collector->OrderManualMove(ManualGoal),TEXT("loaded collector accepts reachable manual order"));
        for (int32 Step=0; Step<10; ++Step) Collector->Tick(.05f);
        Check(Collector->bCollectorManualOrder && FVector::DistSquared2D(Collector->Destination,ManualGoal)<1.f
            && Collector->Cargo==100.f && Credits==Initial,
            TEXT("manual collector order survives automatic logistics ticks without delivery"));
        for (int32 Step=0; Step<300 && Collector->bCollectorManualOrder; ++Step) Collector->Tick(.05f);
        Check(!Collector->bCollectorManualOrder && FVector::DistSquared2D(Collector->GetActorLocation(),ManualGoal)<=FMath::Square(5.f),
            TEXT("collector reaches manual goal and releases manual control"));
        const FVector BeforeInvalid=Collector->Destination;
        Check(!Collector->OrderManualMove(FVector(BattlefieldLayout::RiverX,0,35)) && !Collector->bCollectorManualOrder
            && FVector::DistSquared2D(Collector->Destination,BeforeInvalid)<1.f,
            TEXT("unreachable water order rejects without disabling automatic logistics"));
        Check(!Collector->OrderManualMove(Depot->GetActorLocation()) && !Collector->bCollectorManualOrder,
            TEXT("blocked structure-center order rejects without claiming manual control"));
        const FVector LoadedPosition = Collector->GetActorLocation();
        int32 TravelTicks = 0;
        for (; TravelTicks < 300 && Collector->Cargo > 0.f; ++TravelTicks) Collector->Tick(0.05f);
        Check(Collector->Cargo == 0.f && Credits == Initial + 100.f && TravelTicks > 0 && TravelTicks < 300
            && FVector::DistSquared2D(Collector->GetActorLocation(), LoadedPosition) > FMath::Square(10.f)
            && IsReachableDockPosition(Depot,Collector->GetActorLocation()),
            TEXT("loaded collector drives real movement ticks to depot and delivers without hanging"));
        TickCollector(Collector, 0.1f);
        Check(Credits == Initial + 100.f, TEXT("delivery cannot pay duplicate credits"));
    }
    else Check(false, TEXT("starting collector exists"));
    Check(!RequestProduction(EPrototypeRole::Combat), TEXT("combat production rejected without factory"));
    const FVector FactorySite(-7400, -1600, 35);
    const float BeforeBuild = Credits;
    Check(!BeginBuilding(EPrototypeRole::Factory, PlayerHQ->GetActorLocation()) && Credits == BeforeBuild,
        TEXT("overlapping building placement rejects without charging"));
    Check(BeginBuilding(EPrototypeRole::Factory, FactorySite) && Credits == BeforeBuild - 350.f,
        TEXT("valid factory construction charges 350"));
    Check(!RequestProduction(EPrototypeRole::Combat), TEXT("unfinished factory cannot produce"));
    TickEconomy(11.f);
    Check(!FindCompletedStructure(0, EPrototypeRole::Factory), TEXT("factory unfinished before twelve seconds"));
    TickEconomy(1.f);
    APrototypeUnit* Factory = FindCompletedStructure(0, EPrototypeRole::Factory);
    Check(Factory != nullptr, TEXT("factory completes at twelve seconds"));
    Credits = 99.f;
    Check(!RequestProduction(EPrototypeRole::Combat) && Credits == 99.f, TEXT("insufficient production credits reject without charge"));
    Credits = 1000.f;
    APrototypeUnit* SecondFactory = SpawnUnit(0, FVector(-6000, -1600, 35));
    if (SecondFactory) SecondFactory->ConfigureRole(EPrototypeRole::Factory);
    Check(RequestProductionAt(SecondFactory, EPrototypeRole::Combat) && GetQueueCount(SecondFactory) == 1
        && GetQueueCount(Factory) == 0 && Credits == 900.f,
        TEXT("explicit producer queues only at selected factory"));
    Check(!RequestProductionAt(SecondFactory, EPrototypeRole::Collector) && Credits == 900.f
        && GetQueueCount(SecondFactory) == 1,
        TEXT("unsupported selected producer rejects without charge or queue mutation"));
    Check(RequestProductionAt(SecondFactory, EPrototypeRole::Combat) && Credits == 800.f,
        TEXT("second selected producer order charges exact cost"));
    TickEconomy(2.f);
    Check(CancelProduction(SecondFactory) && Credits == 900.f && GetQueueCount(SecondFactory) == 1
        && GetProductionProgress(SecondFactory) > .39f,
        TEXT("cancelling newest queued item refunds cost and preserves active progress"));
    Check(CancelProduction(SecondFactory) && Credits == 1000.f && GetQueueCount(SecondFactory) == 0
        && !CancelProduction(SecondFactory) && Credits == 1000.f,
        TEXT("active cancellation refunds once and empty cancellation cannot duplicate credits"));
    // Six-cell factory extends to Y=-2200; the rally needs another80 cm of vehicle clearance.
    const FVector RallyGoal(-5800, -2400, 35);
    Check(SetRallyPoint(SecondFactory, RallyGoal), TEXT("selected factory accepts reachable rally point"));
    Check(!SetRallyPoint(SecondFactory, FVector(BattlefieldLayout::RiverX, 0, 35))
        && SecondFactory->bHasRallyPoint && SecondFactory->RallyPoint.Equals(RallyGoal, 1.f),
        TEXT("invalid water rally rejects while preserving previous rally point"));
    Check(!SetRallyPoint(SecondFactory, SecondFactory->GetActorLocation())
        && SecondFactory->RallyPoint.Equals(RallyGoal, 1.f),
        TEXT("rally inside producing structure rejects without replacing valid point"));
    TSet<APrototypeUnit*> ExistingUnits;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) ExistingUnits.Add(*It);
    const bool bRallyQueued = RequestProductionAt(SecondFactory, EPrototypeRole::Combat);
    TickEconomy(5.f);
    APrototypeUnit* RallyVehicle = nullptr;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == 0 && It->Role == EPrototypeRole::Combat && !ExistingUnits.Contains(*It)) RallyVehicle = *It;
    Check(bRallyQueued && RallyVehicle && RallyVehicle->Destination.Equals(RallyGoal, 1.f)
        && GetQueueCount(SecondFactory) == 0,
        TEXT("completed selected factory vehicle receives its rally move order"));
    if (RallyVehicle)
    {
        const FVector SpawnPosition = RallyVehicle->GetActorLocation();
        for (int32 Step = 0; Step < 250; ++Step) RallyVehicle->Tick(.05f);
        Check(FVector::DistSquared2D(RallyVehicle->GetActorLocation(), RallyGoal) <= FMath::Square(5.f)
            && FVector::DistSquared2D(RallyVehicle->GetActorLocation(), SpawnPosition) > FMath::Square(20.f),
            TEXT("produced vehicle follows real movement ticks to rally destination"));
        RallyVehicle->Destroy();
    }
    else Check(false, TEXT("produced vehicle follows real movement ticks to rally destination"));
    APrototypeUnit* RallyBlocker = SpawnUnit(0, RallyGoal);
    if (RallyBlocker) RallyBlocker->ConfigureRole(EPrototypeRole::PowerPlant);
    ExistingUnits.Empty();
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) ExistingUnits.Add(*It);
    const float CreditsBeforeBlockedRally = Credits;
    const bool bBlockedRallyQueued = RequestProductionAt(SecondFactory, EPrototypeRole::Combat);
    TickEconomy(5.f);
    APrototypeUnit* WaitingVehicle = nullptr;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == 0 && It->Role == EPrototypeRole::Combat && !ExistingUnits.Contains(*It)) WaitingVehicle = *It;
    Check(bBlockedRallyQueued && WaitingVehicle && !WaitingVehicle->bMoveOrdered
        && !WaitingVehicle->bCollectorManualOrder && GetQueueCount(SecondFactory) == 0
        && Credits == CreditsBeforeBlockedRally - RoleCost(EPrototypeRole::Combat)
        && Feedback == TEXT("Rally point blocked: vehicle waiting at production exit.")
        && SecondFactory->bHasRallyPoint && SecondFactory->RallyPoint.Equals(RallyGoal, 1.f),
        TEXT("newly blocked rally produces paid vehicle at exit and reports failure without extra charge or stale movement"));
    if (WaitingVehicle) WaitingVehicle->Destroy();
    if (RallyBlocker) RallyBlocker->Destroy();
    const FVector CollectorRally(-7000, -900, 35);
    Check(SetRallyPoint(PlayerHQ, CollectorRally) && RequestProductionAt(PlayerHQ, EPrototypeRole::Collector),
        TEXT("HQ accepts collector rally and selected collector production"));
    ExistingUnits.Empty();
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) ExistingUnits.Add(*It);
    TickEconomy(6.f);
    APrototypeUnit* RallyCollector = nullptr;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == 0 && It->Role == EPrototypeRole::Collector && !ExistingUnits.Contains(*It)) RallyCollector = *It;
    if (RallyCollector)
    {
        RallyCollector->Tick(.05f);
        Check(RallyCollector->bCollectorManualOrder && RallyCollector->Destination.Equals(CollectorRally, 1.f),
            TEXT("new collector retains rally order through automatic logistics tick"));
        RallyCollector->Destroy();
    }
    else Check(false, TEXT("new collector retains rally order through automatic logistics tick"));
    if (SecondFactory) SecondFactory->Destroy();
    Credits = 1000.f;
    for (int32 Index = 0; Index < 5; ++Index) Check(RequestProduction(EPrototypeRole::Combat), TEXT("paid production queue accepts vehicle"));
    Check(Credits == 500.f && GetQueueCount(Factory) == 5 && !RequestProduction(EPrototypeRole::Combat) && Credits == 500.f,
        TEXT("five-item queue cap prevents overflow and extra charges"));
    int32 BeforeArmy = 0;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) if (Alive(*It) && It->Team == 0 && It->Role == EPrototypeRole::Combat) ++BeforeArmy;
    TickEconomy(4.f);
    Check(GetQueueCount(Factory) == 5 && GetProductionProgress(Factory) > 0.79f, TEXT("vehicle requires five seconds and exposes production progress"));
    TickEconomy(1.f);
    int32 AfterArmy = 0;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) if (Alive(*It) && It->Team == 0 && It->Role == EPrototypeRole::Combat) ++AfterArmy;
    Check(AfterArmy == BeforeArmy + 1 && GetQueueCount(Factory) == 4, TEXT("production completes one paid vehicle per factory"));
    if (Factory) Factory->Destroy();
    TickEconomy(0.1f);
    Check(ProductionQueue.IsEmpty() && Credits == 500.f, TEXT("destroyed factory cancels its paid pending production without spawning or refund duplication"));
    ResetMatch();
    Check(ProductionQueue.IsEmpty() && Credits == 800.f && EnemyCredits == 800.f && SalvageNodes.Num() == 3
        && SalvageNodes[0].Remaining == 4000.f, TEXT("restart restores economy and clears production"));
    for (int32 Step = 0; Step < 44; ++Step) TickEconomy(1.f);
    int32 DefensiveCombat = 0;
    bool bPrematureAttack = false;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Alive(*It) || It->Team != 1 || It->Role != EPrototypeRole::Combat) continue;
        ++DefensiveCombat;
        bPrematureAttack |= It->StrategicTarget == PlayerHQ;
    }
    for (const FPrototypeProduction& Item : ProductionQueue)
        if (Alive(Item.Producer.Get()) && Item.Producer->Team == 1 && Item.Role == EPrototypeRole::Combat) ++DefensiveCombat;
    Check(!bPrematureAttack && DefensiveCombat <= 8 && DefensiveCombat > 3,
        TEXT("bot builds paid defenses capped at eight and grants sixty-second setup grace"));
    TickEconomy(66.f);
    bool bAttackAfterGrace = false;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == 1 && It->Role == EPrototypeRole::Combat && It->StrategicTarget == PlayerHQ) bAttackAfterGrace = true;
    Check(bAttackAfterGrace, TEXT("bot begins HQ attacks when setup grace expires"));
    ResetMatch();
    BotDecisionTime = 10000.f;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (It->Role == EPrototypeRole::Collector) { ReleaseSalvageReservation(*It); It->Destroy(); }
    // This fixture deliberately isolates finite depletion and a single stocked
    // spatial cell. Normal matches keep the default sixteen regrowing cells.
    for (int32 Index=0;Index<SalvageNodes.Num();++Index)
    {
        auto Settings=SalvageNodes[Index].Field.GetDefinition();
        Settings.InitialAmount=0.f;
        SalvageNodes[Index].Field.Reset(Settings,EHREconomyStyle::FiniteSupply,16);
        ResetFieldAmount(Index,Index==0 ? 150.f : 0.f);
    }
    const FVector Pickup = SalvageNodes[0].Position;
    APrototypeUnit* First = SpawnUnit(0, Pickup + FVector(0, -220, 0));
    APrototypeUnit* Second = SpawnUnit(0, Pickup + FVector(0, -240, 0));
    First->ConfigureRole(EPrototypeRole::Collector);
    Second->ConfigureRole(EPrototypeRole::Collector);
    First->SetActorTickEnabled(false);
    Second->SetActorTickEnabled(false);
    TickCollector(First, .1f);
    TickCollector(Second, .1f);
    Check(First->HarvestNode == 0 && Second->HarvestNode == INDEX_NONE && Second->Cargo == 0.f,
        TEXT("two collectors cannot occupy the same reserved pickup"));
    Check(First->OrderManualMove(Pickup + FVector(0, -500, 0)), TEXT("reserved collector accepts manual departure"));
    TickCollector(Second, .1f);
    Check(Second->HarvestNode == 0 && First->HarvestNode == INDEX_NONE,
        TEXT("manual departure releases pickup to waiting collector"));
    Second->ReceiveHit(Second->MaxHealth);
    First->bCollectorManualOrder = false;
    TickCollector(First, .1f);
    Check(First->HarvestNode == 0, TEXT("destroyed collector releases pickup immediately"));
    const float BeforeReservedDelivery = Credits;
    First->SetActorLocation(Pickup);
    TickCollector(First, 5.f);
    APrototypeUnit* Third = SpawnUnit(0, Pickup);
    Third->ConfigureRole(EPrototypeRole::Collector);
    Third->SetActorTickEnabled(false);
    TickCollector(Third, 5.f);
    Check(First->Cargo == 100.f && Third->Cargo == 50.f && SalvageNodes[0].Remaining == 0.f,
        TEXT("successive collectors conserve finite salvage without doubled harvest"));
    APrototypeUnit* DeliveryDepot = FindCompletedStructure(0, EPrototypeRole::Depot);
    First->SetActorLocation(DeliveryDepot->GetActorLocation()+FVector(-490,0,0));
    Third->SetActorLocation(DeliveryDepot->GetActorLocation()+FVector(0,-490,0));
    for (int32 Step=0;Step<100 && (First->Cargo>0. || Third->Cargo>0.);++Step)
    { TickCollector(First,.1f); TickCollector(Third,.1f); }
    Check(Credits == BeforeReservedDelivery + 150.f && First->Cargo == 0.f && Third->Cargo == 0.f,
        TEXT("two collector deliveries pay exactly the depleted salvage once"));
    SalvageNodes[0].Position = FVector(BattlefieldLayout::RiverX, 0, 35);
    ResetFieldAmount(0,50.f);
    SalvageNodes[1].Position = Pickup;
    ResetFieldAmount(1,50.f);
    First->SetActorLocation(Pickup + FVector(0, -220, 0));
    TickCollector(First, .1f);
    Check(First->HarvestNode == 1 && SalvageNodes[0].Remaining == 50.f,
        TEXT("collector skips unreachable water resource and reserves reachable alternative"));
    ResetMatch();
    AActor* Cache=SalvageNodes[0].VisualActor.Get();
    Check(Cache && FPrototypeNavigation::IsWalkable(Cache->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld()))
        && FPrototypeNavigation::IsWalkable(SalvageNodes[0].Position,FPrototypeNavigation::GetWorldObstacles(GetWorld())),
        TEXT("spatial resource plants and all pickup approaches remain traversable"));
    auto EmptyCacheSettings=SalvageNodes[0].Field.GetDefinition();EmptyCacheSettings.InitialAmount=0.f;
    SalvageNodes[0].Field.Reset(EmptyCacheSettings,EHREconomyStyle::FiniteSupply,16);ResetFieldAmount(0,0.f);
    TickEconomy(.1f);
    Check(Cache && Cache->IsHidden() && FPrototypeNavigation::IsWalkable(Cache->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld())),
        TEXT("finite depleted spatial cell disappears without leaving an obsolete cache blocker"));
    ResetMatch();
    BotDecisionTime=10000.f;
    APrototypeUnit* EnemyCollector=nullptr;
    for (TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if (Alive(*It) && It->Team==1 && It->Role==EPrototypeRole::Collector) EnemyCollector=*It;
    const float EnemyCreditsBefore=EnemyCredits;
    int32 EnemyDeliveryTicks=0;
    if (EnemyCollector)
    {
        EnemyCollector->SetActorTickEnabled(false);
        for (;EnemyDeliveryTicks<500 && (EnemyCredits==EnemyCreditsBefore || EnemyCollector->Cargo>0.);++EnemyDeliveryTicks) EnemyCollector->Tick(.05f);
    }
    Check(EnemyCollector && EnemyDeliveryTicks<500 && FMath::Abs(EnemyCredits-EnemyCreditsBefore-100.)<1.e-8
        && EnemyCollector->Cargo==0.f && SalvageNodes[1].Remaining==3900.f,
        TEXT("opponent collector reaches local field and delivers real income outside enlarged city footprint"));
    ResetMatch();BotDecisionTime=10000.f;
    APrototypeUnit* FieldCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)FieldCollector=*It;
    if(FieldCollector)
    {
        FieldCollector->SetActorTickEnabled(false);
        FHRTempestFieldDefinition Settings;
        Settings.Capacity=100;Settings.InitialAmount=50;Settings.HarvestRate=20;
        Settings.SeedFloor=20;Settings.RegrowthDelay=10;Settings.RegrowthPerSecond=2;
        SalvageNodes[0].Field.Reset(Settings,EHREconomyStyle::RegrowingField,1);
        ResetFieldAmount(0,50);ResetFieldAmount(1,0);ResetFieldAmount(2,0);
        const float BeforeSeedDelivery=Credits;
        TickCollector(FieldCollector,5);
        Check(FieldCollector->Cargo==30 && SalvageNodes[0].Remaining==20 && FieldCollector->HarvestNode==INDEX_NONE,
            TEXT("live regrowing field preserves seed while depleted harvestable stock releases claim"));
        for(int32 Step=0;Step<350 && FieldCollector->Cargo>0.;++Step)FieldCollector->Tick(.05f);
        Check(Credits==BeforeSeedDelivery+30 && FieldCollector->Cargo==0,
            TEXT("seed floor triggers partial cargo return and exact delivery instead of stranded collector"));
        TickEconomy(9);
        Check(SalvageNodes[0].Remaining==20,TEXT("live field cannot regrow before authored harvest delay"));
        TickEconomy(6);
        Check(SalvageNodes[0].Remaining==30 && SalvageNodes[0].Field.GetAmount(0)==30,
            TEXT("live field grows only five seconds beyond delay without double advancement"));
    }
    else Check(false,TEXT("live regrowing field collector fixture exists"));
    ResetMatch();BotDecisionTime=10000.f;
    FieldCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)FieldCollector=*It;
    if(FieldCollector)
    {
        FieldCollector->SetActorTickEnabled(false);TickCollector(FieldCollector,.1f);
        FieldCollector->ConfigureRole(EPrototypeRole::Combat);
        APrototypeUnit* Replacement=SpawnUnit(0,SalvageNodes[0].Position+FVector(0,-220,0));
        Replacement->ConfigureRole(EPrototypeRole::Collector);Replacement->SetActorTickEnabled(false);
        TickCollector(Replacement,.1f);
        Check(Replacement->HarvestNode==0 && Replacement->HarvestCell!=INDEX_NONE
            &&SalvageNodes[0].Field.HasValidReservation(Replacement->HarvestCell,Replacement),
            TEXT("role change clears stale cell ownership for a replacement collector"));
    }
    else Check(false,TEXT("role change collector fixture exists"));
    ResetMatch();BotDecisionTime=10000.f;
    FieldCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)FieldCollector=*It;
    if(FieldCollector)
    {
        FieldCollector->SetActorTickEnabled(false);FieldCollector->Cargo=100;
        FieldCollector->SetActorLocation(FVector(-4500,0,35));
        APrototypeUnit* BlockedDepot=SpawnUnit(0,FVector(BattlefieldLayout::RiverX,0,35));
        BlockedDepot->ConfigureRole(EPrototypeRole::Depot);
        APrototypeUnit* ReachableDepot=FindCompletedStructure(0,EPrototypeRole::Depot,FVector(-7500,1600,35));
        const float BeforeFallback=Credits;
        TickCollector(FieldCollector,.05f);
        Check(FieldCollector->DeliveryDepot.Get()==ReachableDepot
            && IsReachableDockPosition(ReachableDepot,FieldCollector->Destination),
            TEXT("collector bypasses nearer water-blocked refinery for reachable delivery approach"));
        for(int32 Step=0;Step<500 && FieldCollector->Cargo>0.;++Step)FieldCollector->Tick(.05f);
        Check(Credits==BeforeFallback+100 && FieldCollector->Cargo==0,
            TEXT("alternate refinery delivery uses real movement and pays loaded cargo once"));
    }
    else Check(false,TEXT("alternate refinery collector fixture exists"));
    ResetMatch();BotDecisionTime=10000.f;
    FieldCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)FieldCollector=*It;
    APrototypeUnit* ScreenedDepot=FindCompletedStructure(0,EPrototypeRole::Depot);
    if(FieldCollector&&ScreenedDepot)
    {
        FieldCollector->SetActorTickEnabled(false);FieldCollector->Cargo=100;
        const FVector ScreenedStart=ScreenedDepot->GetActorLocation()+FVector(490,0,0);
        FieldCollector->SetActorLocation(ScreenedStart);
        // The defense blocks the final unloading segment but leaves the actual
        // collector start outside its expanded rectangle (sqrt(50²+70²)>80).
        APrototypeUnit* CacheBlocker=SpawnUnit(0,ScreenedDepot->GetActorLocation()+FVector(240,270,0));
        if(CacheBlocker)
        {
            CacheBlocker->ConfigureRole(EPrototypeRole::Defense);CacheBlocker->SetActorTickEnabled(false);
            const bool bDirectBlocked=!FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),ScreenedStart,ScreenedDepot->GetActorLocation(),ScreenedDepot);
            const bool bScreenedStartFree=FPrototypeNavigation::IsWalkable(ScreenedStart,FPrototypeNavigation::GetWorldObstacles(GetWorld()));
            const float BeforeScreenedDelivery=Credits;
            TickCollector(FieldCollector,.05f);
            TArray<FVector> Detour;
            Check(bScreenedStartFree && bDirectBlocked && Credits==BeforeScreenedDelivery && FieldCollector->Cargo==100
                && !FieldCollector->Destination.Equals(ScreenedStart,1.f)
                && FPrototypeNavigation::FindWorldPath(GetWorld(),ScreenedStart,FieldCollector->Destination,Detour),
                TEXT("blocked direct dock segment inside delivery radius preserves cargo and orders reachable detour"));
            int32 DetourTicks=0;
            for(;DetourTicks<350 && FieldCollector->Cargo>0.;++DetourTicks)FieldCollector->Tick(.05f);
            Check(DetourTicks>0 && DetourTicks<350 && Credits==BeforeScreenedDelivery+100 && FieldCollector->Cargo==0
                && FVector::DistSquared2D(ScreenedStart,FieldCollector->GetActorLocation())>FMath::Square(10.f),
                TEXT("screened refinery pays once only after real movement reaches an unobstructed unloading side"));
        }
        else Check(false,TEXT("screened refinery blocker fixture exists"));
    }
    else Check(false,TEXT("screened refinery collector fixture exists"));
    ResetMatch();BotDecisionTime=10000.f;
    FieldCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)FieldCollector=*It;
    if(FieldCollector)
    {
        FieldCollector->SetActorTickEnabled(false);FieldCollector->Cargo=100;
        const FVector CachedStart(-4500,0,35);
        FieldCollector->SetActorLocation(CachedStart);
        APrototypeUnit* FarDepot=FindCompletedStructure(0,EPrototypeRole::Depot);
        APrototypeUnit* NearDepot=SpawnUnit(0,FVector(-1700,0,35));
        APrototypeUnit* NorthBridgeBlocker=SpawnUnit(0,FVector(BattlefieldLayout::RiverX,BattlefieldLayout::BridgeOffsetY,35));
        APrototypeUnit* SouthBridgeBlocker=SpawnUnit(0,FVector(BattlefieldLayout::RiverX,-BattlefieldLayout::BridgeOffsetY,35));
        if(FarDepot&&NearDepot&&NorthBridgeBlocker&&SouthBridgeBlocker)
        {
            NearDepot->ConfigureRole(EPrototypeRole::Depot);
            NorthBridgeBlocker->ConfigureRole(EPrototypeRole::Factory);
            SouthBridgeBlocker->ConfigureRole(EPrototypeRole::Factory);
            const FVector CachedDock=NearDepot->GetActorLocation()+FVector(-490,0,0);
            FieldCollector->Destination=CachedDock;
            TArray<FVector> StaleRoute;
            const bool bCachedPointFree=FPrototypeNavigation::IsWalkable(CachedDock,FPrototypeNavigation::GetWorldObstacles(GetWorld()));
            const bool bCachedRouteGone=!FPrototypeNavigation::FindWorldPath(GetWorld(),CachedStart,CachedDock,StaleRoute);
            const float BeforeCachedFallback=Credits;
            TickCollector(FieldCollector,.05f);
            Check(bCachedPointFree && bCachedRouteGone && Credits==BeforeCachedFallback && FieldCollector->Cargo==100
                && FieldCollector->DeliveryDepot.Get()==FarDepot
                && IsReachableDockPosition(FarDepot,FieldCollector->Destination),
                TEXT("walkable cached dock with lost route cannot hide a reachable alternative refinery"));
            int32 FallbackTicks=0;
            for(;FallbackTicks<500 && FieldCollector->Cargo>0.;++FallbackTicks)FieldCollector->Tick(.05f);
            Check(FallbackTicks>0 && FallbackTicks<500 && Credits==BeforeCachedFallback+100 && FieldCollector->Cargo==0,
                TEXT("cached dock route invalidation recovers through real alternate refinery delivery"));
        }
        else Check(false,TEXT("cached dock and bridge blocker fixtures exist"));
    }
    else Check(false,TEXT("cached dock collector fixture exists"));
    ResetMatch();BotDecisionTime=10000.f;
    FieldCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)FieldCollector=*It;
    if(FieldCollector)
    {
        FieldCollector->SetActorTickEnabled(false);
        FHRTempestFieldDefinition FractionalSettings;
        FractionalSettings.Capacity=2.f;FractionalSettings.InitialAmount=1.123456f;FractionalSettings.HarvestRate=20.f;
        SalvageNodes[0].Field.Reset(FractionalSettings,EHREconomyStyle::FiniteSupply,1);
        ResetFieldAmount(0,FractionalSettings.InitialAmount);
        ResetFieldAmount(1,0.f);ResetFieldAmount(2,0.f);
        FieldCollector->SetActorLocation(SalvageNodes[0].Position);
        TickCollector(FieldCollector,.1f);
        FieldCollector->Cargo=80.0;
        const double FractionalWalletBefore=Credits;
        const double FractionalBalanceBefore=SalvageNodes[0].Field.GetTotalAmount()+FieldCollector->Cargo;
        TickCollector(FieldCollector,.5f);
        const auto FractionalBalance=[&]()
        {
            return SalvageNodes[0].Field.GetTotalAmount()+FieldCollector->Cargo+(Credits-FractionalWalletBefore);
        };
        bool bFractionalBalanceHeld=FMath::Abs(FractionalBalance()-FractionalBalanceBefore)<=1.e-8;
        Check(SalvageNodes[0].Remaining==0.f && FieldCollector->Cargo>80.0 && FieldCollector->HarvestNode==INDEX_NONE
            && Credits==FractionalWalletBefore && bFractionalBalanceHeld,
            TEXT("fractional live field debit preserves existing cargo without rounded resource gain"));
        const FVector FractionalPickup=FieldCollector->GetActorLocation();
        int32 FractionalTravelTicks=0;
        for(;FractionalTravelTicks<350 && FieldCollector->Cargo>0.;++FractionalTravelTicks)
        {
            FieldCollector->Tick(.05f);
            bFractionalBalanceHeld&=FMath::Abs(FractionalBalance()-FractionalBalanceBefore)<=1.e-8;
        }
        Check(FractionalTravelTicks>0 && FractionalTravelTicks<350 && bFractionalBalanceHeld && FieldCollector->Cargo==0.0
            && FMath::Abs((Credits-FractionalWalletBefore)-FractionalBalanceBefore)<=1.e-8
            && FVector::DistSquared2D(FractionalPickup,FieldCollector->GetActorLocation())>FMath::Square(10.f),
            TEXT("fractional field plus cargo plus wallet remains conserved through real return and unloading"));
    }
    else Check(false,TEXT("fractional field collector fixture exists"));
    ResetMatch();BotDecisionTime=10000.f;
    FieldCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)FieldCollector=*It;
    APrototypeUnit* FractionalDepot=FindCompletedStructure(0,EPrototypeRole::Depot);
    if(FieldCollector&&FractionalDepot)
    {
        FieldCollector->SetActorTickEnabled(false);
        FieldCollector->SetActorLocation(FractionalDepot->GetActorLocation()+FVector(-490,0,0));
        const double FractionalCargo=static_cast<double>(.1f);
        FieldCollector->Cargo=FractionalCargo;
        const double BeforeSmallDelivery=Credits;
        TickCollector(FieldCollector,.05f);
        Check(FieldCollector->Cargo==0.0 && FMath::Abs((Credits-BeforeSmallDelivery)-FractionalCargo)<=1.e-8,
            TEXT("fractional cargo at a valid dock credits the actual amount without float wallet loss"));
        const double AfterSmallDelivery=Credits;
        for(int32 Step=0;Step<4;++Step)TickCollector(FieldCollector,.05f);
        Check(Credits==AfterSmallDelivery && FieldCollector->Cargo==0.0,
            TEXT("repeated collector ticks cannot duplicate a fractional dock payment"));
    }
    else Check(false,TEXT("fractional dock collector fixture exists"));
    ResetMatch(); BotDecisionTime=10000.f;
    APrototypeUnit* DockDepot=FindCompletedStructure(0,EPrototypeRole::Depot);
    APrototypeUnit* DockFirst=SpawnUnit(0,DockDepot->GetActorLocation()+FVector(-490,0,0));
    APrototypeUnit* DockSecond=SpawnUnit(0,DockDepot->GetActorLocation()+FVector(0,-800,0));
    DockFirst->ConfigureRole(EPrototypeRole::Collector); DockSecond->ConfigureRole(EPrototypeRole::Collector);
    DockFirst->SetActorTickEnabled(false); DockSecond->SetActorTickEnabled(false);
    DockFirst->Cargo=100.; DockSecond->Cargo=50.;
    const double DockWalletBefore=Credits;
    TickCollector(DockFirst,.5f); TickCollector(DockSecond,.5f);
    Check(DockFirst->CollectorState==EPrototypeCollectorState::Unloading && DockFirst->Cargo==80.
        && Credits==DockWalletBefore+20.,TEXT("live dock transfers only authored 40 credits per second"));
    Check(DockSecond->CollectorState==EPrototypeCollectorState::WaitingForDock && DockSecond->Cargo==50.
        && FVector::DistSquared2D(DockSecond->Destination,DockDepot->GetActorLocation())>FMath::Square(Footprint(EPrototypeRole::Depot)+100.f),
        TEXT("second collector waits outside refinery and cannot bypass exclusive FIFO slot"));
    const double InterruptedCargo=DockFirst->Cargo; const double InterruptedWallet=Credits;
    Check(DockFirst->OrderManualMove(DockFirst->GetActorLocation()+FVector(-400,0,0)),TEXT("unloading collector accepts reachable manual departure"));
    TickCollector(DockFirst,.5f);
    Check(DockFirst->Cargo==InterruptedCargo && Credits==InterruptedWallet && !RefineryDocks.FindChecked(DockDepot).IsDocked(DockFirst),
        TEXT("manual departure releases dock without discarding cargo or paying further credits"));
    TickCollector(DockSecond,.1f);
    DockSecond->SetActorLocation(DockSecond->DockApproach);
    TickCollector(DockSecond,.5f);
    Check(DockSecond->CollectorState==EPrototypeCollectorState::Unloading && DockSecond->Cargo==30.,
        TEXT("FIFO successor acquires reachable freed dock and unloads partial cargo"));
    DockDepot->ReceiveHit(DockDepot->MaxHealth);
    const double LostDepotCargo=DockSecond->Cargo; const double LostDepotWallet=Credits;
    TickCollector(DockSecond,.5f);
    Check(DockSecond->Cargo==LostDepotCargo && Credits==LostDepotWallet && !DockSecond->DeliveryDepot.IsValid(),
        TEXT("destroyed refinery stops unload while preserving remaining cargo for rerouting"));
    APrototypeUnit* RecoveryDepot=SpawnUnit(0,FVector(-8200,1600,35)); RecoveryDepot->ConfigureRole(EPrototypeRole::Depot);
    TickCollector(DockSecond,.1f);
    Check(DockSecond->DeliveryDepot.Get()==RecoveryDepot,TEXT("collector reroutes retained cargo to replacement reachable refinery"));
    DockSecond->SetActorLocation(DockSecond->DockApproach); TickCollector(DockSecond,1.f);
    Check(DockSecond->Cargo==0. && FMath::Abs(Credits-(DockWalletBefore+70.))<1.e-8,
        TEXT("interrupted and recovered unload conserves total delivered credits exactly once"));
    APrototypeUnit* ResidualCollector=SpawnUnit(0,RecoveryDepot->GetActorLocation()+FVector(-490,0,0));
    ResidualCollector->ConfigureRole(EPrototypeRole::Collector); ResidualCollector->SetActorTickEnabled(false);
    const double ResidualWallet=Credits;
    const double ResidualCargo=(std::nextafter(Credits,std::numeric_limits<double>::infinity())-Credits)*.5;
    ResidualCollector->Cargo=ResidualCargo; TickCollector(ResidualCollector,.5f);
    Check(ResidualCollector->Cargo==ResidualCargo && Credits==ResidualWallet && ResidualCollector->bRetainedUnloadRemainder
        && !RefineryDocks.FindChecked(RecoveryDepot).IsDocked(ResidualCollector),
        TEXT("sub-wallet-ULP remainder preserves cargo and frees dock without creating unrepresentable credits"));
    TickCollector(ResidualCollector,.1f);
    Check(ResidualCollector->HarvestNode!=INDEX_NONE && ResidualCollector->Cargo==ResidualCargo && Credits==ResidualWallet,
        TEXT("retained fractional remainder resumes reachable harvesting instead of repeatedly blocking refinery"));
    ResetMatch();BotDecisionTime=10000.f;
    if(SalvageNodes.IsValidIndex(0))
    {
        FPrototypeSalvage& VisualNode=SalvageNodes[0];
        AStaticMeshActor* ActualPatch=Cast<AStaticMeshActor>(VisualNode.VisualActor.Get());
        if(ActualPatch)
        {
            FHRTempestFieldDefinition VisualSettings;
            VisualSettings.Capacity=50.f;VisualSettings.InitialAmount=5.f;VisualSettings.SeedFloor=5.f;
            VisualSettings.RegrowthPerSecond=2.f;VisualSettings.RegrowthDelay=0.f;
            VisualNode.Field.Reset(VisualSettings,EHREconomyStyle::RegrowingField,1);
            ResetFieldAmount(0,5.f);VisualNode.bVisualDepleted=false;
            int32 DepletedEvents=0;int32 RestoredEvents=0;
            const FDelegateHandle VisualEvents=OnResourceDepletionChanged.AddLambda([&](int32 Index,bool bDepleted)
            {
                if(Index!=0)return;
                if(bDepleted)++DepletedEvents;else ++RestoredEvents;
            });
            UpdateResourceVisuals();UpdateResourceVisuals();
            const TArray<float>& DepletedData=ActualPatch->GetStaticMeshComponent()->GetCustomPrimitiveData().Data;
            Check(DepletedEvents==1 && RestoredEvents==0 && VisualNode.bVisualDepleted && !ActualPatch->IsHidden()
                && DepletedData.IsValidIndex(0) && DepletedData[0]==0.f,
                TEXT("harvestable depletion emits one event across repeated visual updates and sets actual patch activity to zero"));
            // Empty regrowth terrain stays reserved even though finite supply
            // would permit this exact footprint once its collector is removed.
            for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
                if(Alive(*It)&&It->Team==0&&It->Role==EPrototypeRole::Collector)It->Destroy();
            ResetFieldAmount(0,0.f);
            UpdateResourceVisuals();UpdateResourceVisuals();
            Check(DepletedEvents==1 && !ActualPatch->IsHidden()
                && FPrototypeNavigation::IsWalkable(ActualPatch->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld()))
                && !CanPlaceBuilding(EPrototypeRole::Factory,VisualNode.Position),
                TEXT("zero-stock regrowing cell keeps its root bed and construction exclusion while pickup stays reachable"));
            TickEconomy(6.f);UpdateResourceVisuals();UpdateResourceVisuals();
            const TArray<float>& RestoredData=ActualPatch->GetStaticMeshComponent()->GetCustomPrimitiveData().Data;
            Check(DepletedEvents==1 && RestoredEvents==1 && !VisualNode.bVisualDepleted && !ActualPatch->IsHidden()
                && VisualNode.Remaining==12.f && RestoredData.IsValidIndex(0) && FMath::IsNearlyEqual(RestoredData[0],7.f/45.f),
                TEXT("regrowth restores actual patch activity and emits exactly one nondepleted transition"));
            OnResourceDepletionChanged.Remove(VisualEvents);
            VisualSettings.InitialAmount=0.f;VisualSettings.SeedFloor=0.f;
            VisualNode.Field.Reset(VisualSettings,EHREconomyStyle::FiniteSupply,1);ResetFieldAmount(0,0.f);
            UpdateResourceVisuals();
            Check(ActualPatch->IsHidden()
                && FPrototypeNavigation::IsWalkable(ActualPatch->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld()))
                && CanPlaceBuilding(EPrototypeRole::Factory,VisualNode.Position),
                TEXT("finite depletion hides actual patch and releases the same navigation and construction footprint"));
        }
        else Check(false,TEXT("resource visual fixture has an actual static mesh actor"));
    }
    else Check(false,TEXT("resource visual fixture field exists"));
    ResetMatch();BotDecisionTime=10000.f;
    Check(SalvageNodes.Num()==3 && GetFieldCellCount(0)==16 && GetFieldCellCount(1)==16 && GetFieldCellCount(2)==16
        && GetFieldAmount(0)==4000. && GetFieldAmount(1)==4000. && GetFieldAmount(2)==6000.,
        TEXT("normal restart creates three sixteen-cell fields with unchanged 4000 4000 6000 total capacity"));
    bool bAllPickupsReachable=true;bool bAllVisualsReal=true;TSet<AActor*> UniqueCellVisuals;
    for(int32 FieldIndex=0;FieldIndex<SalvageNodes.Num();++FieldIndex)
    {
        const FPrototypeSalvage& Field=SalvageNodes[FieldIndex];
        const FVector Approach=FieldIndex==1 ? FVector(7500,1100,35) : FVector(-7500,1100,35);
        for(int32 CellIndex=0;CellIndex<Field.Field.Num();++CellIndex)
        {
            TArray<FVector> Route;
            const FVector PickupPosition=GetFieldCellPosition(FieldIndex,CellIndex);
            const bool bReachable=FPrototypeNavigation::IsWalkable(PickupPosition,FPrototypeNavigation::GetWorldObstacles(GetWorld()))
                && FPrototypeNavigation::FindWorldPath(GetWorld(),Approach,PickupPosition,Route);
            if(!bReachable) UE_LOG(LogTemp,Display,TEXT("ECONOMY_CELL_ROUTE_FAILED field=%d cell=%d pickup=%s approach=%s"),FieldIndex,CellIndex,*PickupPosition.ToString(),*Approach.ToString());
            bAllPickupsReachable &= bReachable;
            AStaticMeshActor* Visual=Cast<AStaticMeshActor>(Field.CellVisuals.IsValidIndex(CellIndex) ? Field.CellVisuals[CellIndex].Get() : nullptr);
            bAllVisualsReal&=Visual && Visual->GetStaticMeshComponent()->GetStaticMesh()
                && Visual->GetStaticMeshComponent()->GetCollisionEnabled()==ECollisionEnabled::NoCollision;
            if(Visual)UniqueCellVisuals.Add(Visual);
        }
    }
    Check(bAllPickupsReachable,TEXT("every one of forty-eight spatial pickup cells has a real unobstructed refinery approach route"));
    Check(bAllVisualsReal && UniqueCellVisuals.Num()==48,
        TEXT("forty-eight distinct mesh actors visualize the authoritative cells without blocking collector navigation"));
    Check(SalvageNodes[0].Field.GetStyle()==EHREconomyStyle::RegrowingField
        && FMath::IsNearlyEqual(SalvageNodes[0].Field.GetDefinition().RegrowthPerSecond*16.f,2.f),
        TEXT("normal gameplay enables regrowth with two credits per second per whole field instead of sixteen-fold inflation"));
    const double BeforeInvalidStock=GetFieldAmount(0);
    Check(!ResetFieldAmount(0,-1.f) && GetFieldAmount(0)==BeforeInvalidStock
        && !ResetFieldAmount(-1,10.f) && GetFieldCellCount(99)==0,
        TEXT("explicit stock restore rejects invalid inputs without introducing another resource authority"));
    for(int32 FieldIndex=0;FieldIndex<3;++FieldIndex)ResetFieldAmount(FieldIndex,0.f);
    TickEconomy(8.f);
    Check(GetFieldAmount(0)==16. && GetFieldAmount(1)==16. && GetFieldAmount(2)==16.,
        TEXT("forty-eight independently depleted cells regrow at sixteen credits per field over eight simulation seconds"));
    ResetMatch();BotDecisionTime=10000.f;
    APrototypeUnit* EarlyCellCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(It->Team==0 && It->Role==EPrototypeRole::Collector)EarlyCellCollector=*It;
    APrototypeUnit* LaterCellCollector=SpawnUnit(0,GetFieldCellPosition(0,2));
    if(EarlyCellCollector && LaterCellCollector)
    {
        EarlyCellCollector->SetActorTickEnabled(false);
        LaterCellCollector->ConfigureRole(EPrototypeRole::Collector);LaterCellCollector->SetActorTickEnabled(false);
        TickCollector(EarlyCellCollector,.5f);TickEconomy(5.f);TickCollector(LaterCellCollector,.5f);TickEconomy(7.f);
        Check(EarlyCellCollector->HarvestCell==0 && LaterCellCollector->HarvestCell==2
            && SalvageNodes[0].Field.GetAmount(0)==240.25f && SalvageNodes[0].Field.GetAmount(2)==240.f
            && GetFieldAmount(0)==3980.25,
            TEXT("separately harvested live cells keep independent regrowth delays and advance the field clock exactly once"));
    }
    else Check(false,TEXT("independent regrowth delay collectors exist"));
    ResetMatch();BotDecisionTime=10000.f;
    TArray<APrototypeUnit*> StartingCollectors;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        if(It->Role==EPrototypeRole::Collector)StartingCollectors.Add(*It);
    for(APrototypeUnit* ExistingCollector:StartingCollectors){ReleaseSalvageReservation(ExistingCollector);ExistingCollector->Destroy();}
    TArray<APrototypeUnit*> Fleet;
    for(int32 Index=0;Index<4;++Index)
    {
        APrototypeUnit* Vehicle=SpawnUnit(0,GetFieldCellPosition(0,Index));
        if(Vehicle){Vehicle->ConfigureRole(EPrototypeRole::Collector);Vehicle->SetActorTickEnabled(false);Fleet.Add(Vehicle);}
    }
    TSet<int32> ClaimedCells;
    for(APrototypeUnit* Vehicle:Fleet)
    {
        TickCollector(Vehicle,.1f);
        if(Vehicle->HarvestNode==0 && SalvageNodes[0].Field.HasValidReservation(Vehicle->HarvestCell,Vehicle))ClaimedCells.Add(Vehicle->HarvestCell);
    }
    Check(Fleet.Num()==4 && ClaimedCells.Num()==4,
        TEXT("four collectors reserve four distinct cells of the same spatial field concurrently"));
    bool bOneGlobalClaim=true;
    for(APrototypeUnit* Vehicle:Fleet)
    {
        int32 Claims=0;
        for(const FPrototypeSalvage& Field:SalvageNodes)
            for(const FHRTempestCellState& Cell:Field.Field.GetCells())if(Cell.ReservedCollector.Get()==Vehicle)++Claims;
        bOneGlobalClaim&=Claims==1;
    }
    Check(bOneGlobalClaim,TEXT("each active collector owns exactly one cell across all three fields"));
    const double FleetWalletBefore=Credits;
    const double FleetStockBefore=GetFieldAmount(0)+GetFieldAmount(1)+GetFieldAmount(2);
    TArray<FVector> FleetStarts;for(APrototypeUnit* Vehicle:Fleet)FleetStarts.Add(Vehicle->GetActorLocation());
    bool bFleetConserved=true;bool bAllVehiclesDelivered=false;int32 FleetTicks=0;
    TSet<APrototypeUnit*> DeliveredVehicles;
    for(;FleetTicks<1800 && !bAllVehiclesDelivered;++FleetTicks)
    {
        for(APrototypeUnit* Vehicle:Fleet)
        {
            const double CargoBefore=Vehicle->Cargo;
            Vehicle->Tick(.05f);
            if(CargoBefore>0. && Vehicle->Cargo<CargoBefore)DeliveredVehicles.Add(Vehicle);
        }
        double TotalCargo=0.;for(APrototypeUnit* Vehicle:Fleet)TotalCargo+=Vehicle->Cargo;
        const double Total=GetFieldAmount(0)+GetFieldAmount(1)+GetFieldAmount(2)+TotalCargo+(Credits-FleetWalletBefore);
        bFleetConserved&=FMath::Abs(Total-FleetStockBefore)<1.e-6;
        bAllVehiclesDelivered=DeliveredVehicles.Num()==4;
    }
    bool bEveryVehicleMoved=Fleet.Num()==4;
    for(int32 Index=0;Index<Fleet.Num();++Index)
        bEveryVehicleMoved&=FVector::DistSquared2D(Fleet[Index]->GetActorLocation(),FleetStarts[Index])>FMath::Square(10.f);
    Check(bAllVehiclesDelivered && FleetTicks>0 && FleetTicks<1800 && bEveryVehicleMoved,
        TEXT("all four spatial collectors drive real routes and enter the refinery FIFO to deliver income"));
    Check(bFleetConserved && Credits>FleetWalletBefore,
        TEXT("concurrent spatial harvesting plus collector cargo plus dock payouts preserves the complete financial balance every tick"));
    if(Fleet.Num()==4)
    {
        APrototypeUnit* ManualCollector=Fleet[0];
        ManualCollector->Cargo=0.;ReleaseSalvageReservation(ManualCollector);
        ManualCollector->SetActorLocation(GetFieldCellPosition(0,8));TickCollector(ManualCollector,.1f);
        const int32 FormerCell=ManualCollector->HarvestCell;
        Check(FormerCell!=INDEX_NONE && ManualCollector->OrderManualMove(GetFieldCellPosition(0,8)+FVector(-400,0,0))
            && ManualCollector->HarvestNode==INDEX_NONE && ManualCollector->HarvestCell==INDEX_NONE
            && SalvageNodes[0].Field.GetReservedCell(ManualCollector)==INDEX_NONE,
            TEXT("manual collector movement atomically releases its spatial field and cell assignment"));
        APrototypeUnit* CrossFieldCollector=Fleet[1];ReleaseSalvageReservation(CrossFieldCollector);CrossFieldCollector->Cargo=0.;
        CrossFieldCollector->SetActorLocation(GetFieldCellPosition(2,0));TickCollector(CrossFieldCollector,.1f);
        Check(CrossFieldCollector->HarvestNode==2 && SalvageNodes[0].Field.GetReservedCell(CrossFieldCollector)==INDEX_NONE
            && SalvageNodes[2].Field.HasValidReservation(CrossFieldCollector->HarvestCell,CrossFieldCollector),
            TEXT("cross-field relocation leaves no stale claim in the old spatial field"));
    }
    ResetMatch();BotDecisionTime=10000.f;
    APrototypeUnit* PauseCollector=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)if(It->Team==0 && It->Role==EPrototypeRole::Collector)PauseCollector=*It;
    ResetFieldAmount(0,0.f);
    const double PauseStock=GetFieldAmount(0);const double PauseClock=SalvageNodes[0].Field.GetSimulationSeconds();const float PauseMatch=MatchTime;
    if(APlayerController* Controller=GetWorld()->GetFirstPlayerController())
    {
        Controller->SetPause(true);TickEconomy(10.f);if(PauseCollector)TickCollector(PauseCollector,10.f);
        Check(GetFieldAmount(0)==PauseStock && SalvageNodes[0].Field.GetSimulationSeconds()==PauseClock && MatchTime==PauseMatch
            && PauseCollector && PauseCollector->Cargo==0.,
            TEXT("paused simulation neither regrows cells nor harvests cargo nor advances the field clock"));
        Controller->SetPause(false);
    }
    else Check(false,TEXT("spatial resource pause fixture has player controller"));
    ResetMatch(); BotDecisionTime=10000.f;
    APrototypeUnit* ActiveDepot=FindCompletedStructure(0,EPrototypeRole::Depot);
    APrototypeUnit* ActiveUnload=SpawnUnit(0,FVector(-7000,-1000,35));
    APlayerController* UnloadPauseController=GetWorld()->GetFirstPlayerController();
    if(ActiveDepot && ActiveUnload && UnloadPauseController)
    {
        ActiveUnload->ConfigureRole(EPrototypeRole::Collector);
        ActiveUnload->SetActorTickEnabled(false);
        // Explicit initial cargo is a transfer fixture, not harvested income.
        ActiveUnload->Cargo=100.;
        FVector ActualDock;
        const bool Reachable=FindDeliveryPoint(this,ActiveUnload,ActiveDepot,ActualDock);
        Check(Reachable,TEXT("active unload pause fixture finds a genuinely reachable refinery approach"));
        if(Reachable)
        {
            ActiveUnload->SetActorLocation(ActualDock);
            ActiveUnload->Destination=ActualDock;
            ActiveUnload->DeliveryDepot=ActiveDepot;
            ActiveUnload->DockApproach=ActualDock;
            const double WalletBeforeLoad=Credits;
            TickCollector(ActiveUnload,.5f);
            FHRRefineryDocking* ActiveDock=RefineryDocks.Find(ActiveDepot);
            Check(ActiveUnload->Cargo==80. && Credits==WalletBeforeLoad+20.
                && ActiveUnload->CollectorState==EPrototypeCollectorState::Unloading
                && ActiveDock && ActiveDock->IsDocked(ActiveUnload),
                TEXT("active unload commits exactly twenty credits from its real occupied dock before pause"));
            const uint64 OperationBeforePause=ActiveUnload->UnloadOperation;
            const float ClockBeforePause=MatchTime;
            const bool EnteredPause=UnloadPauseController->SetPause(true);
            Check(EnteredPause && GetWorld()->IsPaused(),TEXT("active unloading enters real world pause"));
            TickCollector(ActiveUnload,1.f); TickEconomy(1.f); ActiveUnload->Tick(1.f);
            ActiveDock=RefineryDocks.Find(ActiveDepot);
            Check(ActiveUnload->Cargo==80. && Credits==WalletBeforeLoad+20.
                && ActiveUnload->UnloadOperation==OperationBeforePause && MatchTime==ClockBeforePause
                && ActiveUnload->DeliveryDepot.Get()==ActiveDepot
                && ActiveDock && ActiveDock->IsDocked(ActiveUnload) && ActiveDock->NumDocked()==1,
                TEXT("pause during active transfer freezes cargo, wallet, operation, time and exclusive dock ownership"));
            UnloadPauseController->SetPause(false);
            TickCollector(ActiveUnload,.5f);
            Check(ActiveUnload->Cargo==60. && Credits==WalletBeforeLoad+40.,
                TEXT("unpaused active transfer resumes at forty credits per second without catch-up or duplication"));
            const bool Disabled=TogglePower(ActiveDepot);
            TickCollector(ActiveUnload,1.f); TickEconomy(1.f);
            ActiveDock=RefineryDocks.Find(ActiveDepot);
            Check(Disabled && ActiveDepot->bManuallyPoweredDown && ActiveUnload->Cargo==60.
                && Credits==WalletBeforeLoad+40.
                && (!ActiveDock || (!ActiveDock->IsDocked(ActiveUnload) && !ActiveDock->IsQueued(ActiveUnload))),
                TEXT("manually disabled refinery retains cargo and credits and releases its active docking claim"));
            const bool Enabled=TogglePower(ActiveDepot);
            TickCollector(ActiveUnload,1.5f);
            Check(Enabled && !ActiveDepot->bManuallyPoweredDown && ActiveUnload->Cargo==0.
                && Credits==WalletBeforeLoad+100.,
                TEXT("re-enabled refinery reacquires the dock and finishes exactly the original hundred-unit load"));
            TickCollector(ActiveUnload,.1f);
            Check(Credits==WalletBeforeLoad+100.,TEXT("pause and manual power recovery cannot pay the completed load twice"));
        }
        UnloadPauseController->SetPause(false);
    }
    else Check(false,TEXT("active unload pause and power fixtures exist"));
    if(Alive(ActiveUnload)) {ReleaseSalvageReservation(ActiveUnload);ActiveUnload->Destroy();}
    ResetMatch();
    UE_LOG(LogTemp, Display, TEXT("ECONOMY_TEST_RESULT=%s"), bPass ? TEXT("PASS") : TEXT("FAIL"));
    if (auto* Controller = GetWorld()->GetFirstPlayerController()) Controller->ConsoleCommand(TEXT("quit"));
}
