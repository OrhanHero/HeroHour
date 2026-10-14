#include "BattlefieldLayout.h"
#include "PrototypeGameMode.h"
#include "EngineUtils.h"
#include "Engine/World.h"

namespace
{
constexpr int32 GridWidth = 120;
constexpr int32 GridHeight = 90;
constexpr int32 GridCellCount = GridWidth * GridHeight;
FVector CellPoint(int32 Index, float Z)
{
    return FVector(-BattlefieldLayout::ExtentX + (Index % GridWidth + .5f) * BattlefieldLayout::CellSize,
        -BattlefieldLayout::ExtentY + (Index / GridWidth + .5f) * BattlefieldLayout::CellSize, Z);
}
int32 PointCell(const FVector& Point)
{
    const int32 X = FMath::Clamp(FMath::FloorToInt((Point.X + BattlefieldLayout::ExtentX) / BattlefieldLayout::CellSize), 0, GridWidth - 1);
    const int32 Y = FMath::Clamp(FMath::FloorToInt((Point.Y + BattlefieldLayout::ExtentY) / BattlefieldLayout::CellSize), 0, GridHeight - 1);
    return Y * GridWidth + X;
}
}

bool FPrototypeNavigation::IsWalkable(const FVector& Point, const TArray<FPrototypeNavigationObstacle>& Obstacles)
{
    if (!BattlefieldLayout::IsLand(Point)) return false;
    if (!BattlefieldLayout::IsStaticClear(Point)) return false;
    for (const FPrototypeNavigationObstacle& Obstacle : Obstacles)
        if (FVector::DistSquared2D(Point, Obstacle.Position) < FMath::Square(Obstacle.Radius + BattlefieldLayout::VehicleRadius)) return false;
    return true;
}

bool FPrototypeNavigation::SegmentWalkable(const FVector& Start, const FVector& End, const TArray<FPrototypeNavigationObstacle>& Obstacles)
{
    const int32 Steps = FMath::Max(1, FMath::CeilToInt(FVector::Dist2D(Start, End) / 40.f));
    for (int32 Step = 0; Step <= Steps; ++Step)
        if (!IsWalkable(FMath::Lerp(Start, End, static_cast<float>(Step) / Steps), Obstacles)) return false;
    return true;
}

bool FPrototypeNavigation::FindPath(const FVector& Start, const FVector& Goal, TArray<FVector>& OutPath,
    const TArray<FPrototypeNavigationObstacle>& Obstacles)
{
    OutPath.Empty();
    if (!IsWalkable(Start, Obstacles) || !IsWalkable(Goal, Obstacles)) return false;
    if (SegmentWalkable(Start, Goal, Obstacles)) { OutPath.Add(Goal); return true; }
    TArray<uint8> Walkable;
    Walkable.SetNumUninitialized(GridCellCount);
    for (int32 Index = 0; Index < GridCellCount; ++Index) Walkable[Index] = IsWalkable(CellPoint(Index, Start.Z), Obstacles) ? 1 : 0;
    auto EndpointCell = [&Walkable, &Obstacles](const FVector& Endpoint)
    {
        const int32 Center = PointCell(Endpoint);
        int32 Best = INDEX_NONE;
        float Distance = TNumericLimits<float>::Max();
        for (int32 DY = -2; DY <= 2; ++DY)
        for (int32 DX = -2; DX <= 2; ++DX)
        {
            const int32 X = Center % GridWidth + DX, Y = Center / GridWidth + DY;
            if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight) continue;
            const int32 Index = Y * GridWidth + X;
            const FVector Point = CellPoint(Index, Endpoint.Z);
            const float Candidate = FVector::DistSquared2D(Endpoint, Point);
            if (Walkable[Index] && Candidate < Distance && SegmentWalkable(Endpoint, Point, Obstacles))
            { Best = Index; Distance = Candidate; }
        }
        return Best;
    };
    const int32 First = EndpointCell(Start), Last = EndpointCell(Goal);
    if (First == INDEX_NONE || Last == INDEX_NONE) return false;
    TArray<float> Cost;
    Cost.Init(TNumericLimits<float>::Max(), GridCellCount);
    TArray<int32> Parent;
    Parent.Init(INDEX_NONE, GridCellCount);
    TArray<uint8> Closed;
    Closed.Init(0, GridCellCount);
    TArray<int32> Open;
    Open.Add(First);
    Cost[First] = 0.f;
    auto Estimate = [Last](int32 Index)
    {
        const float DX = FMath::Abs(Index % GridWidth - Last % GridWidth), DY = FMath::Abs(Index / GridWidth - Last / GridWidth);
        return FMath::Max(DX, DY) + .41421356f * FMath::Min(DX, DY);
    };
    while (!Open.IsEmpty())
    {
        int32 BestSlot = 0;
        for (int32 Slot = 1; Slot < Open.Num(); ++Slot)
            if (Cost[Open[Slot]] + Estimate(Open[Slot]) < Cost[Open[BestSlot]] + Estimate(Open[BestSlot])) BestSlot = Slot;
        const int32 Current = Open[BestSlot];
        Open.RemoveAtSwap(BestSlot);
        if (Closed[Current]) continue;
        if (Current == Last)
        {
            TArray<FVector> Reverse;
            for (int32 Cursor = Last; Cursor != INDEX_NONE; Cursor = Parent[Cursor]) Reverse.Add(CellPoint(Cursor, Start.Z));
            TArray<FVector> Raw;
            for (int32 Index = Reverse.Num() - 1; Index >= 0; --Index) Raw.Add(Reverse[Index]);
            Raw.Add(FVector(Goal.X, Goal.Y, Start.Z));
            FVector Anchor = Start;
            for (int32 Index = 0; Index < Raw.Num();)
            {
                int32 Farthest = Index;
                for (int32 Candidate = Index + 1; Candidate < Raw.Num(); ++Candidate)
                    if (SegmentWalkable(Anchor, Raw[Candidate], Obstacles)) Farthest = Candidate;
                OutPath.Add(Raw[Farthest]);
                Anchor = Raw[Farthest];
                Index = Farthest + 1;
            }
            return true;
        }
        Closed[Current] = 1;
        for (int32 DY = -1; DY <= 1; ++DY)
        for (int32 DX = -1; DX <= 1; ++DX)
        {
            if (DX == 0 && DY == 0) continue;
            const int32 X = Current % GridWidth + DX, Y = Current / GridWidth + DY;
            if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight) continue;
            const int32 Next = Y * GridWidth + X;
            if (!Walkable[Next] || Closed[Next]) continue;
            if (DX != 0 && DY != 0 && (!Walkable[(Current / GridWidth) * GridWidth + X] || !Walkable[Y * GridWidth + Current % GridWidth])) continue;
            if (!SegmentWalkable(CellPoint(Current, Start.Z), CellPoint(Next, Start.Z), Obstacles)) continue;
            const float NextCost = Cost[Current] + (DX != 0 && DY != 0 ? 1.41421356f : 1.f);
            if (NextCost < Cost[Next])
            {
                Cost[Next] = NextCost;
                Parent[Next] = Current;
                Open.Add(Next);
            }
        }
    }
    return false;
}

bool FPrototypeNavigation::FindWorldPath(UWorld* World, const FVector& Start, const FVector& Goal,
    TArray<FVector>& OutPath, const APrototypeUnit* IgnoreStructure)
{
    if (!World) { OutPath.Empty(); return false; }
    return FindPath(Start, Goal, OutPath, GetWorldObstacles(World, IgnoreStructure));
}

TArray<FPrototypeNavigationObstacle> FPrototypeNavigation::GetWorldObstacles(UWorld* World,
    const APrototypeUnit* IgnoreStructure)
{
    TArray<FPrototypeNavigationObstacle> Obstacles;
    if (!World) return Obstacles;
    for (TActorIterator<APrototypeUnit> It(World); It; ++It)
    {
        if (*It == IgnoreStructure || !It->IsStructure() || It->Health <= 0.f || It->IsActorBeingDestroyed()) continue;
        FPrototypeNavigationObstacle Obstacle;
        Obstacle.Position = It->GetActorLocation();
        Obstacle.Radius = APrototypeGameMode::Footprint(It->Role);
        Obstacles.Add(Obstacle);
    }
    if (const auto* Mode=World->GetAuthGameMode<APrototypeGameMode>())
        for (const FPrototypeSalvage& Node : Mode->SalvageNodes)
            if ((Node.Remaining>0.f || Node.Field.GetStyle()==EHREconomyStyle::RegrowingField) && Node.VisualActor.IsValid())
                Obstacles.Add({Node.VisualActor->GetActorLocation(),250.f});
    return Obstacles;
}

bool FPrototypeNavigation::SegmentWorldWalkable(UWorld* World, const FVector& Start, const FVector& End,
    const APrototypeUnit* IgnoreStructure)
{
    return World && SegmentWalkable(Start, End, GetWorldObstacles(World, IgnoreStructure));
}

bool FPrototypeNavigation::RunSelfTests()
{
    bool Passed = true;
    auto Check = [&Passed](bool Condition, const TCHAR* Label)
    {
        Passed &= Condition;
        UE_LOG(LogTemp, Display, TEXT("NAVIGATION_TEST %s: %s"), Label, Condition ? TEXT("PASS") : TEXT("FAIL"));
    };
    TArray<FVector> Route;
    Check(!IsWalkable(FVector(1500, 0, 35)) && IsWalkable(FVector(1500, 750, 35)),
        TEXT("shared city footprints block buildings and leave streets traversable"));
    Check(FindPath(FVector(-8000, 0, 35), FVector(8000, 0, 35), Route) && Route.Num() > 1, TEXT("cross-river order routes through bridge"));
    FVector Previous(-8000, 0, 35);
    bool Safe = !Route.IsEmpty();
    bool CrossedBridge = false;
    for (const FVector& Point : Route)
    {
        Safe &= SegmentWalkable(Previous, Point);
        const int32 Samples = FMath::Max(1, FMath::CeilToInt(FVector::Dist2D(Previous, Point) / 40.f));
        for (int32 Sample = 0; Sample <= Samples; ++Sample)
        {
            const FVector P = FMath::Lerp(Previous, Point, static_cast<float>(Sample) / Samples);
            if (FMath::Abs(P.X - BattlefieldLayout::RiverX) < 600.f) CrossedBridge = true;
        }
        Previous = Point;
    }
    Check(Safe && CrossedBridge, TEXT("every route segment respects water and bridge clearance"));
    Check(!FindPath(FVector(-8000, 0, 35), FVector(-3000, 0, 35), Route) && Route.IsEmpty(), TEXT("water destination rejects without stale route"));
    TArray<FPrototypeNavigationObstacle> Obstacles;
    FPrototypeNavigationObstacle Block;
    Block.Position = FVector(3000, 0, 35); Block.Radius = 600.f; Obstacles.Add(Block);
    Check(FindPath(FVector(750, 0, 35), FVector(6750, 0, 35), Route, Obstacles) && Route.Num() > 1, TEXT("structure footprint detours on same riverbank"));
    Previous = FVector(750, 0, 35); Safe = !Route.IsEmpty();
    for (const FVector& Point : Route) { Safe &= SegmentWalkable(Previous, Point, Obstacles); Previous = Point; }
    Check(Safe, TEXT("smoothed route preserves structure clearance"));
    Block.Position = FVector(-3000, 3200, 35); Block.Radius = 1000.f; Obstacles.Empty(); Obstacles.Add(Block);
    Check(FindPath(FVector(-8000, 2000, 35), FVector(8000, 3200, 35), Route, Obstacles), TEXT("blocked north bridge reroutes to southern bridge"));
    Check(!IsWalkable(FVector(-9500,3000,35)) && !IsWalkable(FVector(8900,3900,35))
        && !IsWalkable(FVector(3000,7000,35)) && !IsWalkable(FVector(-9000,-6000,35)),
        TEXT("western houses and refinery stadium powerplant footprints block movement"));
    Check(!FindPath(FVector(13000, 0, 35), FVector(0, 0, 35), Route), TEXT("out-of-bounds start rejects"));
    UE_LOG(LogTemp, Display, TEXT("NAVIGATION_TEST_RESULT=%s"), Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}
