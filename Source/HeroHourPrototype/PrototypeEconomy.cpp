#include "PrototypeGameMode.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "BattlefieldLayout.h"

namespace
{
bool Alive(const APrototypeUnit* Unit)
{
    return IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health > 0.f;
}

float& Wallet(APrototypeGameMode* Mode, int32 Team)
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
    case EPrototypeRole::Headquarters: return 360.f;
    case EPrototypeRole::Depot: return 180.f;
    case EPrototypeRole::Factory: return 200.f;
    default: return 80.f;
    }
}

float APrototypeGameMode::RoleCost(EPrototypeRole InRole)
{
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
        if (Node.Remaining > 0 && FVector::DistSquared2D(Position, Node.Position) < FMath::Square(Footprint(InRole) + 90.f)) return false;
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
    ProductionQueue.Add(Item);
    Wallet(this, Team) -= RoleCost(InRole);
    if (Team == 0) Feedback = InRole == EPrototypeRole::Combat ? TEXT("Combat pickup queued (5 seconds).") : TEXT("Salvage collector queued (6 seconds).");
    return true;
}

void APrototypeGameMode::ResetEconomy()
{
    ProductionQueue.Empty();
    SalvageNodes.Empty();
    for (float X : { -6500.f, 6500.f, 0.f })
    {
        FPrototypeSalvage Node;
        Node.Position = FVector(X, X == 0.f ? -3200.f : 1600.f, 35.f);
        Node.Remaining = X == 0.f ? 6000.f : 4000.f;
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
    SpawnRole(0, EPrototypeRole::Collector, FVector(-6500, 1600, 35));
    SpawnRole(1, EPrototypeRole::Collector, FVector(6500, 1600, 35));
}

void APrototypeGameMode::TickCollector(APrototypeUnit* Unit, float DeltaSeconds)
{
    if (Outcome != 0 || !Alive(Unit) || Unit->Role != EPrototypeRole::Collector) return;
    Unit->AttackTarget = nullptr;
    Unit->StrategicTarget = nullptr;
    const bool bReturning = Unit->Cargo >= 100.f || (Unit->Cargo > 0.f
        && (!SalvageNodes.IsValidIndex(Unit->HarvestNode) || SalvageNodes[Unit->HarvestNode].Remaining <= 0.f));
    if (bReturning)
    {
        APrototypeUnit* Depot = FindCompletedStructure(Unit->Team, EPrototypeRole::Depot, Unit->GetActorLocation());
        if (!Depot) { Unit->Destination = Unit->GetActorLocation(); return; }
        if (FVector::DistSquared2D(Unit->GetActorLocation(), Depot->GetActorLocation()) <= FMath::Square(Footprint(Depot->Role) + 100.f))
        {
            Wallet(this, Unit->Team) += Unit->Cargo;
            Unit->Cargo = 0.f;
            Unit->HarvestNode = INDEX_NONE;
            Unit->HarvestTimer = 0.f;
            Unit->Destination = Unit->GetActorLocation();
            return;
        }
        Unit->OrderMove(Depot->GetActorLocation());
        return;
    }
    if (!SalvageNodes.IsValidIndex(Unit->HarvestNode) || SalvageNodes[Unit->HarvestNode].Remaining <= 0.f)
    {
        Unit->HarvestNode = INDEX_NONE;
        float Nearest = TNumericLimits<float>::Max();
        for (int32 Index = 0; Index < SalvageNodes.Num(); ++Index)
        {
            if (SalvageNodes[Index].Remaining <= 0.f) continue;
            const float Distance = FVector::DistSquared2D(Unit->GetActorLocation(), SalvageNodes[Index].Position);
            if (Distance < Nearest) { Nearest = Distance; Unit->HarvestNode = Index; }
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
        const float Amount = FMath::Min(FMath::Min(Pulses * 10.f, 100.f - Unit->Cargo), Node.Remaining);
        Unit->Cargo += Amount;
        Node.Remaining -= Amount;
    }
}

void APrototypeGameMode::TickEconomy(float DeltaSeconds)
{
    if (Outcome != 0 || DeltaSeconds <= 0.f) return;
    MatchTime += DeltaSeconds;
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
    UE_LOG(LogTemp, Display, TEXT("ECONOMY_TEST_RESULT=%s"), bPass ? TEXT("PASS") : TEXT("FAIL"));
    if (auto* Controller = GetWorld()->GetFirstPlayerController()) Controller->ConsoleCommand(TEXT("quit"));
}
