#include "PrototypeGameMode.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "BattlefieldLayout.h"
#include "RTS/HRRTSDefinitions.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"

namespace
{
bool FindDeliveryPoint(APrototypeGameMode* Mode,APrototypeUnit* Collector,APrototypeUnit* Depot,FVector& Out)
{
    const float Radius=APrototypeGameMode::Footprint(Depot->Role)+90.f;
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
        FVector Point=Depot->GetActorLocation()+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,0);
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
            const float Radius = APrototypeGameMode::Footprint(Producer->Role) + 130.f + Ring * 100.f;
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
    default: return 80.f;
    }
}

float APrototypeGameMode::RoleCost(EPrototypeRole InRole)
{
    if (const UHRUnitDefinition* Definition = GetRoleDefinition(InRole))
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

float APrototypeGameMode::RoleDuration(EPrototypeRole InRole)
{
    if (const UHRUnitDefinition* Definition = GetRoleDefinition(InRole))
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
    if (Outcome != 0 || (InRole != EPrototypeRole::Depot && InRole != EPrototypeRole::Factory) || (Team != 0 && Team != 1)) return false;
    const APrototypeUnit* HQ = Team == 0 ? PlayerHQ : EnemyHQ;
    if (!Alive(HQ) || !HQ->IsCompleted() || !BattlefieldLayout::IsLand(Position, Footprint(InRole) + BattlefieldLayout::VehicleRadius)
        || !FPrototypeNavigation::IsWalkable(Position, FPrototypeNavigation::GetWorldObstacles(GetWorld()))
        || FVector::DistSquared2D(Position, HQ->GetActorLocation()) > FMath::Square(2300.f)) return false;
    if (!BattlefieldLayout::IsStaticClear(Position,Footprint(InRole)+30.f)) return false;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Alive(*It)) continue;
        const float Radius = Footprint(InRole) + Footprint(It->Role) + 30.f;
        if (FVector::DistSquared2D(Position, It->GetActorLocation()) < FMath::Square(Radius)) return false;
    }
    for (const FPrototypeSalvage& Node : SalvageNodes)
        if (Node.Remaining > 0 || Node.Field.GetStyle()==EHREconomyStyle::RegrowingField)
        {
            if (FVector::DistSquared2D(Position,Node.Position)<FMath::Square(Footprint(InRole)+90.f)) return false;
            if (Node.VisualActor.IsValid() && FVector::DistSquared2D(Position,Node.VisualActor->GetActorLocation())<FMath::Square(Footprint(InRole)+280.f)) return false;
        }
    return true;
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
    const float Cost = RoleCost(InRole);
    if (Wallet(this, Team) < Cost)
    {
        if (Team == 0) Feedback = TEXT("Not enough salvage credits for this structure.");
        return false;
    }
    APrototypeUnit* Building = SpawnUnit(Team, FVector(Position.X, Position.Y, 35.f));
    if (!Building) return false;
    Building->ConfigureRole(InRole);
    Building->ConstructionProgress = 0.f;
    Wallet(this, Team) -= Cost;
    if (Team == 0) Feedback = InRole == EPrototypeRole::Factory ? TEXT("Vehicle factory under construction (12 seconds).") : TEXT("Salvage depot under construction (8 seconds).");
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
    if (Outcome != 0 || (InRole != EPrototypeRole::Combat && InRole != EPrototypeRole::Collector) || (Team != 0 && Team != 1)) return false;
    const EPrototypeRole ProducerRole = InRole == EPrototypeRole::Combat ? EPrototypeRole::Factory : EPrototypeRole::Headquarters;
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
    const bool bSupported = (InRole == EPrototypeRole::Combat && Alive(Producer) && Producer->Role == EPrototypeRole::Factory)
        || (InRole == EPrototypeRole::Collector && Alive(Producer) && Producer->Role == EPrototypeRole::Headquarters);
    if (Outcome != 0 || !bSupported || !Producer->IsCompleted() || (Producer->Team != 0 && Producer->Team != 1))
    {
        Feedback = TEXT("Selected building cannot produce this vehicle.");
        return false;
    }
    const int32 Team = Producer->Team;
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
    if (Wallet(this, Team) < RoleCost(InRole))
    {
        if (Team == 0) Feedback = TEXT("Insufficient credits: deliver more salvage.");
        return false;
    }
    FPrototypeProduction Item;
    Item.Producer = Producer;
    Item.Role = InRole;
    Item.Duration = RoleDuration(InRole);
    Item.PaidCost = RoleCost(InRole);
    ProductionQueue.Add(Item);
    Wallet(this, Team) -= RoleCost(InRole);
    if (Team == 0) Feedback = InRole == EPrototypeRole::Combat ? TEXT("Combat pickup queued (5 seconds).") : TEXT("Salvage collector queued (6 seconds).");
    return true;
}

bool APrototypeGameMode::CancelProduction(APrototypeUnit* Producer)
{
    if (Outcome != 0 || !Alive(Producer) || (Producer->Team != 0 && Producer->Team != 1)) return false;
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
    if (Outcome != 0 || !Alive(Producer) || !Producer->IsCompleted()
        || (Producer->Role != EPrototypeRole::Factory && Producer->Role != EPrototypeRole::Headquarters)
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
        const float Radius = Footprint(Producer->Role) + 130.f;
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
    ProductionQueue.Empty();
    SalvageNodes.Empty();
    for (int32 Index=0;Index<3;++Index)
    {
        FPrototypeSalvage Node;
        Node.Position = FVector(BattlefieldLayout::SalvageColumns[Index],BattlefieldLayout::SalvageRows[Index],35.f);
        Node.Remaining = Index == 2 ? 6000.f : 4000.f;
        FHRTempestFieldDefinition FieldSettings;
        if (const UHRTempestFieldDefinitionAsset* Definition = GetFieldDefinition()) FieldSettings = Definition->Settings;
        FieldSettings.Capacity=FieldSettings.InitialAmount=Node.Remaining;
        // Per-node stocks preserve the 4k/6k map balance; rates come from the saved asset.
        if (!GetFieldDefinition()) { FieldSettings.HarvestRate=20.f; FieldSettings.RegrowthPerSecond=2.f; }
        Node.Field.Reset(FieldSettings,FParse::Param(FCommandLine::Get(),TEXT("PrototypeRegrowingFields"))
            ? EHREconomyStyle::RegrowingField : EHREconomyStyle::FiniteSupply,1);
        const FName VisualTag(*FString::Printf(TEXT("HERO_Salvage_%d"),SalvageNodes.Num()));
        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
            if (It->ActorHasTag(VisualTag)) { Node.VisualActor=*It; It->SetActorHiddenInGame(false); break; }
        SalvageNodes.Add(Node);
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
    SpawnRole(1, EPrototypeRole::Factory, FVector(7600, -1600, 35));
    for (int32 Index = 0; Index < 3; ++Index) SpawnRole(0, EPrototypeRole::Combat, FVector(-6800, (Index - 1) * 300.f, 35));
    for (int32 Index = 0; Index < 3; ++Index) SpawnRole(1, EPrototypeRole::Combat, FVector(6800, (Index - 1.f) * 300.f, 35));
    SpawnRole(0, EPrototypeRole::Collector, SalvageNodes[0].Position);
    SpawnRole(1, EPrototypeRole::Collector, SalvageNodes[1].Position);
}

void APrototypeGameMode::ReleaseSalvageReservation(APrototypeUnit* Unit)
{
    if (!Unit) return;
    for (FPrototypeSalvage& Node : SalvageNodes)
    {
        Node.Field.Release(Unit);
        if (Node.Collector.Get() == Unit) Node.Collector.Reset();
    }
    Unit->HarvestNode = INDEX_NONE;
    Unit->HarvestTimer = 0.f;
    Unit->HarvestRouteCheck = 0.f;
}

void APrototypeGameMode::TickCollector(APrototypeUnit* Unit, float DeltaSeconds)
{
    if (Outcome != 0 || !Alive(Unit) || Unit->Role != EPrototypeRole::Collector) return;
    for (int32 NodeIndex=0;NodeIndex<SalvageNodes.Num();++NodeIndex)
    {
        FPrototypeSalvage& Node=SalvageNodes[NodeIndex];
        // Direct fixture/editor stock edits are validated before the shared debit.
        Node.Field.SetAmount(0,Node.Remaining);
        Node.Remaining=Node.Field.GetAmount(0);
        Node.Field.ReleaseInvalidReservations([&Node,NodeIndex](const AActor* Actor)
        {
            const APrototypeUnit* Owner=Cast<APrototypeUnit>(Actor);
            return Alive(Owner) && Owner->Role==EPrototypeRole::Collector && !Owner->bCollectorManualOrder
                && Owner->HarvestNode==NodeIndex && Node.Collector.Get()==Owner;
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
    const bool bReturning = Unit->Cargo >= 100.f || (Unit->Cargo > 0.f
        && (!SalvageNodes.IsValidIndex(Unit->HarvestNode) || SalvageNodes[Unit->HarvestNode].Field.GetHarvestableAmount(0) <= 0.f));
    if (bReturning)
    {
        ReleaseSalvageReservation(Unit);
        APrototypeUnit* Depot=nullptr;FVector DeliveryPoint;bool bSelectedAlreadyDocked=false;
        float Nearest=TNumericLimits<float>::Max();
        for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
        {
            if(!Alive(*It)||It->Team!=Unit->Team||It->Role!=EPrototypeRole::Depot||!It->IsCompleted())continue;
            const float Distance=FVector::DistSquared2D(Unit->GetActorLocation(),It->GetActorLocation());
            FVector Candidate=FVector::ZeroVector;
            const bool AlreadyDocked=Distance<=FMath::Square(Footprint(It->Role)+100.f)
                &&FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Unit->GetActorLocation(),It->GetActorLocation(),*It);
            if(Distance<Nearest&&(AlreadyDocked||FindDeliveryPoint(this,Unit,*It,Candidate)))
            {
                Depot=*It;DeliveryPoint=AlreadyDocked?Unit->GetActorLocation():Candidate;
                Nearest=Distance;bSelectedAlreadyDocked=AlreadyDocked;
            }
        }
        if (!Depot) { Unit->Destination = Unit->GetActorLocation(); return; }
        if (bSelectedAlreadyDocked)
        {
            Wallet(this, Unit->Team) += Unit->Cargo;
            Unit->Cargo = 0.f;
            Unit->HarvestNode = INDEX_NONE;
            Unit->HarvestTimer = 0.f;
            Unit->Destination = Unit->GetActorLocation();
            return;
        }
        Unit->OrderMove(DeliveryPoint);
        return;
    }
    Unit->HarvestRouteCheck -= DeltaSeconds;
    if (SalvageNodes.IsValidIndex(Unit->HarvestNode) && Unit->HarvestRouteCheck <= 0.f)
    {
        TArray<FVector> Route;
        const FPrototypeSalvage& Assigned = SalvageNodes[Unit->HarvestNode];
        if (Assigned.Collector.Get() != Unit || !Assigned.Field.HasValidReservation(0,Unit) || Assigned.Field.GetHarvestableAmount(0) <= 0.f
            || !FPrototypeNavigation::FindWorldPath(GetWorld(), Unit->GetActorLocation(), Assigned.Position, Route))
            ReleaseSalvageReservation(Unit);
        Unit->HarvestRouteCheck = 0.8f;
    }
    if (!SalvageNodes.IsValidIndex(Unit->HarvestNode) || SalvageNodes[Unit->HarvestNode].Field.GetHarvestableAmount(0) <= 0.f)
    {
        ReleaseSalvageReservation(Unit);
        float Nearest = TNumericLimits<float>::Max();
        for (int32 Index = 0; Index < SalvageNodes.Num(); ++Index)
        {
            FPrototypeSalvage& Candidate = SalvageNodes[Index];
            APrototypeUnit* ReservationOwner = Candidate.Collector.Get();
            if (!Alive(ReservationOwner) || ReservationOwner->Role!=EPrototypeRole::Collector
                || ReservationOwner->bCollectorManualOrder || ReservationOwner->HarvestNode != Index
                || !Candidate.Field.HasValidReservation(0,ReservationOwner))
            { Candidate.Field.Release(ReservationOwner);Candidate.Collector.Reset(); }
            if (Candidate.Field.GetHarvestableAmount(0) <= 0.f || Candidate.Collector.IsValid()) continue;
            const float Distance = FVector::DistSquared2D(Unit->GetActorLocation(), SalvageNodes[Index].Position);
            TArray<FVector> Route;
            if (Distance < Nearest && FPrototypeNavigation::FindWorldPath(GetWorld(), Unit->GetActorLocation(), Candidate.Position, Route))
            { Nearest = Distance; Unit->HarvestNode = Index; }
        }
        if (SalvageNodes.IsValidIndex(Unit->HarvestNode))
        {
            if(!SalvageNodes[Unit->HarvestNode].Field.TryReserve(0,Unit)){Unit->HarvestNode=INDEX_NONE;return;}
            SalvageNodes[Unit->HarvestNode].Collector = Unit;
            Unit->HarvestRouteCheck = 0.8f;
        }
    }
    if (!SalvageNodes.IsValidIndex(Unit->HarvestNode)) { Unit->Destination = Unit->GetActorLocation(); return; }
    FPrototypeSalvage& Node = SalvageNodes[Unit->HarvestNode];
    if (FVector::DistSquared2D(Unit->GetActorLocation(), Node.Position) > FMath::Square(130.f))
    {
        Unit->OrderMove(Node.Position);
        return;
    }
    Unit->Destination = Unit->GetActorLocation();
    Unit->HarvestTimer += DeltaSeconds;
    if (Unit->HarvestTimer >= 0.5f)
    {
        const int32 Pulses = FMath::FloorToInt(Unit->HarvestTimer / 0.5f);
        Unit->HarvestTimer -= Pulses * 0.5f;
        const float Amount = Node.Field.Harvest(0,Unit,100.f-Unit->Cargo,Pulses*.5f);
        Unit->Cargo += Amount;
        Node.Remaining=Node.Field.GetAmount(0);
        if (Unit->Cargo >= 100.f || Node.Field.GetHarvestableAmount(0) <= 0.f) ReleaseSalvageReservation(Unit);
        UpdateResourceVisuals();
    }
}

void APrototypeGameMode::UpdateResourceVisuals()
{
    for (int32 Index=0; Index<SalvageNodes.Num(); ++Index)
    {
        FPrototypeSalvage& Node=SalvageNodes[Index];
        const bool bDepleted=Node.Field.GetHarvestableAmount(0)<=0.f;
        if (Node.bVisualDepleted!=bDepleted)
        {
            Node.bVisualDepleted=bDepleted;
            OnResourceDepletionChanged.Broadcast(Index,bDepleted);
        }
        if (AActor* Visual=Node.VisualActor.Get())
        {
            Visual->SetActorHiddenInGame(bDepleted && Node.Field.GetStyle()==EHREconomyStyle::FiniteSupply);
            if (AStaticMeshActor* Patch=Cast<AStaticMeshActor>(Visual))
            {
                const FHRTempestFieldDefinition& Settings=Node.Field.GetDefinition();
                const float ProtectedSeed=Node.Field.GetStyle()==EHREconomyStyle::RegrowingField ? Settings.SeedFloor : 0.f;
                const float UsableCapacity=FMath::Max(0.f,Settings.Capacity-ProtectedSeed);
                const float Activity=UsableCapacity>0.f ? FMath::Clamp(Node.Field.GetHarvestableAmount(0)/UsableCapacity,0.f,1.f) : 0.f;
                Patch->GetStaticMeshComponent()->SetCustomPrimitiveDataFloat(0,Activity);
            }
        }
    }
}

void APrototypeGameMode::TickEconomy(float DeltaSeconds)
{
    if (Outcome != 0 || DeltaSeconds <= 0.f) return;
    MatchTime += DeltaSeconds;
    for(FPrototypeSalvage& Node:SalvageNodes)
    {
        Node.Field.SetAmount(0,Node.Remaining);
        Node.Field.Advance(DeltaSeconds);
        Node.Remaining=Node.Field.GetAmount(0);
    }
    UpdateResourceVisuals();
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->IsStructure() && !It->IsCompleted())
            It->ConstructionProgress = FMath::Min(1.f, It->ConstructionProgress + DeltaSeconds / RoleDuration(It->Role));
    TSet<APrototypeUnit*> Advanced;
    for (int32 Index = 0; Index < ProductionQueue.Num();)
    {
        FPrototypeProduction& Item = ProductionQueue[Index];
        APrototypeUnit* Producer = Item.Producer.Get();
        if (!Alive(Producer)) { ProductionQueue.RemoveAt(Index); continue; }
        if (!Producer->IsCompleted() || Advanced.Contains(Producer)) { ++Index; continue; }
        Advanced.Add(Producer);
        Item.Progress += DeltaSeconds;
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
    BotDecisionTime -= DeltaSeconds;
    if (BotDecisionTime > 0.f || !Alive(EnemyHQ)) return;
    BotDecisionTime = 4.f;
    int32 Collectors = 0;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == 1 && It->Role == EPrototypeRole::Collector) ++Collectors;
    if (Collectors < 2 && GetQueueCount(EnemyHQ, 1) == 0) RequestProduction(EPrototypeRole::Collector, 1);
    if (!FindCompletedStructure(1, EPrototypeRole::Depot)) BeginBuilding(EPrototypeRole::Depot, FVector(7500, 1600, 35), 1);
    if (!FindCompletedStructure(1, EPrototypeRole::Factory)) BeginBuilding(EPrototypeRole::Factory, FVector(7600, -1600, 35), 1);
    int32 EnemyCombatCount = 0;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == 1 && It->Role == EPrototypeRole::Combat) ++EnemyCombatCount;
    for (const FPrototypeProduction& Item : ProductionQueue)
        if (Alive(Item.Producer.Get()) && Item.Producer->Team == 1 && Item.Role == EPrototypeRole::Combat) ++EnemyCombatCount;
    if (EnemyCombatCount < 6) RequestProduction(EPrototypeRole::Combat, 1);
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (MatchTime >= 60.f && Alive(*It) && It->Team == 1 && It->Role == EPrototypeRole::Combat && !Alive(It->AttackTarget)) It->OrderAttack(PlayerHQ);
}

void APrototypeGameMode::RunEconomyTest()
{
    bool bPass = true;
    auto Check = [&bPass](bool Condition, const TCHAR* Label)
    {
        bPass &= Condition;
        UE_LOG(LogTemp, Display, TEXT("ECONOMY_TEST %s: %s"), Label, Condition ? TEXT("PASS") : TEXT("FAIL"));
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
            && Depot && FVector::DistSquared2D(Collector->GetActorLocation(), Depot->GetActorLocation()) <= FMath::Square(Footprint(Depot->Role) + 100.f),
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
    const FVector RallyGoal(-5800, -2200, 35);
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
    if (RallyBlocker) RallyBlocker->ConfigureRole(EPrototypeRole::Depot);
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
    Check(!bPrematureAttack && DefensiveCombat <= 6 && DefensiveCombat > 3,
        TEXT("bot builds paid defenses capped at six and grants sixty-second setup grace"));
    TickEconomy(16.f);
    bool bAttackAfterGrace = false;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (Alive(*It) && It->Team == 1 && It->Role == EPrototypeRole::Combat && It->StrategicTarget == PlayerHQ) bAttackAfterGrace = true;
    Check(bAttackAfterGrace, TEXT("bot begins HQ attacks when setup grace expires"));
    ResetMatch();
    BotDecisionTime = 10000.f;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (It->Role == EPrototypeRole::Collector) { ReleaseSalvageReservation(*It); It->Destroy(); }
    for (int32 Index = 1; Index < SalvageNodes.Num(); ++Index) SalvageNodes[Index].Remaining = 0.f;
    SalvageNodes[0].Remaining = 150.f;
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
    First->SetActorLocation(DeliveryDepot->GetActorLocation());
    Third->SetActorLocation(DeliveryDepot->GetActorLocation());
    TickCollector(First, .1f);
    TickCollector(Third, .1f);
    TickCollector(First, .1f);
    TickCollector(Third, .1f);
    Check(Credits == BeforeReservedDelivery + 150.f && First->Cargo == 0.f && Third->Cargo == 0.f,
        TEXT("two collector deliveries pay exactly the depleted salvage once"));
    SalvageNodes[0].Position = FVector(BattlefieldLayout::RiverX, 0, 35);
    SalvageNodes[0].Remaining = 50.f;
    SalvageNodes[1].Position = Pickup;
    SalvageNodes[1].Remaining = 50.f;
    First->SetActorLocation(Pickup + FVector(0, -220, 0));
    TickCollector(First, .1f);
    Check(First->HarvestNode == 1 && SalvageNodes[0].Remaining == 50.f,
        TEXT("collector skips unreachable water resource and reserves reachable alternative"));
    ResetMatch();
    AActor* Cache=SalvageNodes[0].VisualActor.Get();
    Check(Cache && !FPrototypeNavigation::IsWalkable(Cache->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld()))
        && FPrototypeNavigation::IsWalkable(SalvageNodes[0].Position,FPrototypeNavigation::GetWorldObstacles(GetWorld())),
        TEXT("visible supply crates block driving while their pickup approach stays open"));
    SalvageNodes[0].Remaining=0.f;
    TickEconomy(.1f);
    Check(Cache && Cache->IsHidden() && FPrototypeNavigation::IsWalkable(Cache->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld())),
        TEXT("depleted supply cache disappears and releases its navigation footprint"));
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
        for (;EnemyDeliveryTicks<500 && EnemyCredits==EnemyCreditsBefore;++EnemyDeliveryTicks) EnemyCollector->Tick(.05f);
    }
    Check(EnemyCollector && EnemyDeliveryTicks<500 && EnemyCredits==EnemyCreditsBefore+100.f
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
        SalvageNodes[0].Remaining=50;SalvageNodes[1].Remaining=SalvageNodes[2].Remaining=0;
        const float BeforeSeedDelivery=Credits;
        TickCollector(FieldCollector,5);
        Check(FieldCollector->Cargo==30 && SalvageNodes[0].Remaining==20 && FieldCollector->HarvestNode==INDEX_NONE,
            TEXT("live regrowing field preserves seed while depleted harvestable stock releases claim"));
        for(int32 Step=0;Step<350 && Credits==BeforeSeedDelivery;++Step)FieldCollector->Tick(.05f);
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
        Check(Replacement->HarvestNode==0 && SalvageNodes[0].Collector.Get()==Replacement
            &&SalvageNodes[0].Field.HasValidReservation(0,Replacement),
            TEXT("role change clears stale field and compatibility claims for a replacement collector"));
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
        Check(ReachableDepot && FVector::DistSquared2D(FieldCollector->Destination,ReachableDepot->GetActorLocation())
            <=FMath::Square(Footprint(ReachableDepot->Role)+100),
            TEXT("collector bypasses nearer water-blocked refinery for reachable delivery approach"));
        for(int32 Step=0;Step<500 && Credits==BeforeFallback;++Step)FieldCollector->Tick(.05f);
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
        // A cache blocks the direct unloading segment while another side of the
        // same refinery remains reachable. This exercises payout, not just pathing.
        APrototypeUnit* CacheBlocker=SpawnUnit(0,ScreenedDepot->GetActorLocation()+FVector(240,230,0));
        if(CacheBlocker)
        {
            CacheBlocker->ConfigureRole(EPrototypeRole::Combat);CacheBlocker->SetActorTickEnabled(false);
            SalvageNodes[0].VisualActor=CacheBlocker;
            const bool bDirectBlocked=!FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),ScreenedStart,ScreenedDepot->GetActorLocation(),ScreenedDepot);
            const float BeforeScreenedDelivery=Credits;
            TickCollector(FieldCollector,.05f);
            TArray<FVector> Detour;
            Check(bDirectBlocked && Credits==BeforeScreenedDelivery && FieldCollector->Cargo==100
                && !FieldCollector->Destination.Equals(ScreenedStart,1.f)
                && FPrototypeNavigation::FindWorldPath(GetWorld(),ScreenedStart,FieldCollector->Destination,Detour),
                TEXT("blocked direct dock segment inside delivery radius preserves cargo and orders reachable detour"));
            int32 DetourTicks=0;
            for(;DetourTicks<350 && Credits==BeforeScreenedDelivery;++DetourTicks)FieldCollector->Tick(.05f);
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
                && FVector::DistSquared2D(FieldCollector->Destination,FarDepot->GetActorLocation())<=FMath::Square(Footprint(FarDepot->Role)+100.f),
                TEXT("walkable cached dock with lost route cannot hide a reachable alternative refinery"));
            int32 FallbackTicks=0;
            for(;FallbackTicks<500 && Credits==BeforeCachedFallback;++FallbackTicks)FieldCollector->Tick(.05f);
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
        SalvageNodes[0].Remaining=FractionalSettings.InitialAmount;
        SalvageNodes[1].Remaining=SalvageNodes[2].Remaining=0.f;
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
        for(;FractionalTravelTicks<350 && Credits==FractionalWalletBefore;++FractionalTravelTicks)
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
            VisualNode.Remaining=5.f;VisualNode.bVisualDepleted=false;
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
            VisualNode.Field.SetAmount(0,0.f);VisualNode.Remaining=0.f;
            UpdateResourceVisuals();UpdateResourceVisuals();
            Check(DepletedEvents==1 && !ActualPatch->IsHidden()
                && !FPrototypeNavigation::IsWalkable(ActualPatch->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld()))
                && !CanPlaceBuilding(EPrototypeRole::Factory,VisualNode.Position),
                TEXT("zero-stock regrowing patch stays visible and preserves navigation and building exclusion"));
            TickEconomy(6.f);UpdateResourceVisuals();UpdateResourceVisuals();
            const TArray<float>& RestoredData=ActualPatch->GetStaticMeshComponent()->GetCustomPrimitiveData().Data;
            Check(DepletedEvents==1 && RestoredEvents==1 && !VisualNode.bVisualDepleted && !ActualPatch->IsHidden()
                && VisualNode.Remaining==12.f && RestoredData.IsValidIndex(0) && FMath::IsNearlyEqual(RestoredData[0],7.f/45.f),
                TEXT("regrowth restores actual patch activity and emits exactly one nondepleted transition"));
            OnResourceDepletionChanged.Remove(VisualEvents);
            VisualSettings.InitialAmount=0.f;VisualSettings.SeedFloor=0.f;
            VisualNode.Field.Reset(VisualSettings,EHREconomyStyle::FiniteSupply,1);VisualNode.Remaining=0.f;
            UpdateResourceVisuals();
            Check(ActualPatch->IsHidden()
                && FPrototypeNavigation::IsWalkable(ActualPatch->GetActorLocation(),FPrototypeNavigation::GetWorldObstacles(GetWorld()))
                && CanPlaceBuilding(EPrototypeRole::Factory,VisualNode.Position),
                TEXT("finite depletion hides actual patch and releases the same navigation and construction footprint"));
        }
        else Check(false,TEXT("resource visual fixture has an actual static mesh actor"));
    }
    else Check(false,TEXT("resource visual fixture field exists"));
    ResetMatch();
    UE_LOG(LogTemp, Display, TEXT("ECONOMY_TEST_RESULT=%s"), bPass ? TEXT("PASS") : TEXT("FAIL"));
    if (auto* Controller = GetWorld()->GetFirstPlayerController()) Controller->ConsoleCommand(TEXT("quit"));
}
