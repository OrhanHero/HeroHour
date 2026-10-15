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
FVector2D StructureHalfExtents(const APrototypeUnit* Structure)
{
    if (!IsValid(Structure)) return FVector2D::ZeroVector;
    if (const auto* Mode = Structure->GetWorld() ? Structure->GetWorld()->GetAuthGameMode<APrototypeGameMode>() : nullptr)
    {
        const FIntPoint Cells = Mode->GetBuildDimensions(Structure->Role, Structure->BuildRotation, Structure->Team);
        return FVector2D(Cells.X * BattlefieldLayout::CellSize * .5f, Cells.Y * BattlefieldLayout::CellSize * .5f);
    }
    const float Radius = APrototypeGameMode::Footprint(Structure->Role);
    return FVector2D(Radius, Radius);
}
bool ObstacleContains(const FPrototypeNavigationObstacle& Obstacle, const FVector& Point)
{
    if (Obstacle.HalfExtents.X > 0 && Obstacle.HalfExtents.Y > 0)
    {
        const FVector2D Delta(Point.X-Obstacle.Position.X, Point.Y-Obstacle.Position.Y);
        const FVector2D Outside(FMath::Max(0., FMath::Abs(Delta.X)-Obstacle.HalfExtents.X),
            FMath::Max(0., FMath::Abs(Delta.Y)-Obstacle.HalfExtents.Y));
        return Outside.SizeSquared() < FMath::Square(BattlefieldLayout::VehicleRadius);
    }
    return FVector::DistSquared2D(Point, Obstacle.Position) < FMath::Square(Obstacle.Radius+BattlefieldLayout::VehicleRadius);
}
double SegmentPointDistanceSquared(const FVector2D& A,const FVector2D& B,const FVector2D& Point)
{
    const FVector2D Delta=B-A;
    const double LengthSquared=Delta.SizeSquared();
    const double T=LengthSquared>0. ? FMath::Clamp(FVector2D::DotProduct(Point-A,Delta)/LengthSquared,0.,1.) : 0.;
    return (Point-(A+Delta*T)).SizeSquared();
}
double SegmentRectangleDistanceSquared(const FVector2D& A,const FVector2D& B,const FVector2D& Half)
{
    const FVector2D Delta=B-A;
    double First=0.,Last=1.;
    bool Intersects=true;
    for(int32 Axis=0;Axis<2;++Axis)
    {
        const double Origin=Axis==0?A.X:A.Y,Direction=Axis==0?Delta.X:Delta.Y,Extent=Axis==0?Half.X:Half.Y;
        if(FMath::Abs(Direction)<1.e-12)
        {
            if(Origin<-Extent || Origin>Extent) Intersects=false;
            continue;
        }
        double Enter=(-Extent-Origin)/Direction,Exit=(Extent-Origin)/Direction;
        if(Enter>Exit) Swap(Enter,Exit);
        First=FMath::Max(First,Enter);Last=FMath::Min(Last,Exit);
        if(First>Last) Intersects=false;
    }
    if(Intersects) return 0.;
    auto PointDistance=[&Half](const FVector2D& Point)
    {
        return FVector2D(FMath::Max(0.,FMath::Abs(Point.X)-Half.X),FMath::Max(0.,FMath::Abs(Point.Y)-Half.Y)).SizeSquared();
    };
    double Best=FMath::Min(PointDistance(A),PointDistance(B));
    for(int32 X:{-1,1}) for(int32 Y:{-1,1})
        Best=FMath::Min(Best,SegmentPointDistanceSquared(A,B,FVector2D(X*Half.X,Y*Half.Y)));
    return Best;
}
bool SegmentHitsObstacle(const FVector& Start,const FVector& End,const FPrototypeNavigationObstacle& Obstacle)
{
    const FVector2D A(Start.X-Obstacle.Position.X,Start.Y-Obstacle.Position.Y);
    const FVector2D B(End.X-Obstacle.Position.X,End.Y-Obstacle.Position.Y);
    if(Obstacle.HalfExtents.X>0 && Obstacle.HalfExtents.Y>0)
        return SegmentRectangleDistanceSquared(A,B,Obstacle.HalfExtents)<FMath::Square(BattlefieldLayout::VehicleRadius);
    return SegmentPointDistanceSquared(A,B,FVector2D::ZeroVector)<FMath::Square(Obstacle.Radius+BattlefieldLayout::VehicleRadius);
}
}

float FPrototypeNavigation::StructureApproachRadius(const APrototypeUnit* Structure, const FVector& Direction, float Clearance)
{
    if (!IsValid(Structure)) return 0.f;
    FVector Axis = Direction.GetSafeNormal2D();
    if (Axis.IsNearlyZero()) Axis = FVector(0,1,0);
    const FVector2D Half = StructureHalfExtents(Structure) + FVector2D(FMath::Max(0.f,Clearance));
    const double X = FMath::Abs(Axis.X)>UE_SMALL_NUMBER ? Half.X/FMath::Abs(Axis.X) : TNumericLimits<double>::Max();
    const double Y = FMath::Abs(Axis.Y)>UE_SMALL_NUMBER ? Half.Y/FMath::Abs(Axis.Y) : TNumericLimits<double>::Max();
    return static_cast<float>(FMath::Min(X,Y));
}

FVector FPrototypeNavigation::ProjectOutsideStructure(const APrototypeUnit* Structure, const FVector& Point,
    const FVector& FallbackDirection, float Clearance)
{
    if (!IsValid(Structure)) return Point;
    FVector Direction = Point-Structure->GetActorLocation();
    if (Direction.SizeSquared2D()<1.f) Direction=FallbackDirection;
    if (Direction.SizeSquared2D()<1.f) Direction=FVector(0,1,0);
    Direction=Direction.GetSafeNormal2D();
    const float Radius=StructureApproachRadius(Structure,Direction,Clearance);
    if (FVector::DistSquared2D(Point,Structure->GetActorLocation())>=FMath::Square(Radius)) return Point;
    FVector Result=Structure->GetActorLocation()+Direction*Radius;
    Result.Z=Point.Z;
    return Result;
}

bool FPrototypeNavigation::FindStructureApproach(UWorld* World, const APrototypeUnit* Target,
    const FVector& Start, float WeaponRange, FVector& OutPoint)
{
    if (!World || !IsValid(Target) || !Target->IsStructure() || Target->Health<=0.f
        || Target->IsActorBeingDestroyed() || Start.ContainsNaN() || !FMath::IsFinite(WeaponRange)) return false;
    const auto Obstacles=GetWorldObstacles(World);
    if (!IsWalkable(Start,Obstacles)) return false;
    FVector Preferred=(Start-Target->GetActorLocation()).GetSafeNormal2D();
    if (Preferred.IsNearlyZero()) Preferred=FVector(0,1,0);
    const float PreferredAngle=FMath::Atan2(Preferred.Y,Preferred.X);
    // Try the facing side first, then alternate sides in increasing angular order.
    // At most32 finite A* searches, each bounded by the navigation grid's closed set.
    for(int32 Index=0;Index<32;++Index)
    {
        const int32 Step=Index==0?0:((Index+1)/2)*(Index%2==1?1:-1);
        const float Angle=PreferredAngle+Step*(2.f*PI/32.f);
        const FVector Direction(FMath::Cos(Angle),FMath::Sin(Angle),0);
        const float OutsideRadius=StructureApproachRadius(Target,Direction,90.f);
        const float Range=FMath::Max(FMath::Max(0.f,WeaponRange),OutsideRadius+170.f);
        const float Radius=FMath::Max(Range*.88f,OutsideRadius+5.f);
        FVector Candidate=Target->GetActorLocation()+Direction*Radius;
        Candidate.Z=Start.Z;
        if(!IsWalkable(Candidate,Obstacles)) continue;
        TArray<FVector> Route;
        if(!FindPath(Start,Candidate,Route,Obstacles)) continue;
        OutPoint=Candidate;
        return true;
    }
    return false;
}

bool FPrototypeNavigation::IsWalkable(const FVector& Point, const TArray<FPrototypeNavigationObstacle>& Obstacles)
{
    if (!BattlefieldLayout::IsLand(Point)) return false;
    if (!BattlefieldLayout::IsStaticClear(Point)) return false;
    for (const FPrototypeNavigationObstacle& Obstacle : Obstacles)
        if (ObstacleContains(Obstacle, Point)) return false;
    return true;
}

bool FPrototypeNavigation::SegmentWalkable(const FVector& Start, const FVector& End, const TArray<FPrototypeNavigationObstacle>& Obstacles)
{
    // IsLand is piecewise constant between river-bank and bridge-edge crossings.
    // Test every interval, including arbitrarily short clips missed by fixed sampling.
    if (!BattlefieldLayout::IsLand(Start) || !BattlefieldLayout::IsLand(End)) return false;
    TArray<double, TInlineAllocator<8>> LandCuts{0.,1.};
    const FVector LandDelta=End-Start;
    auto AddLandCut=[&](double Origin,double Delta,double Boundary)
    {
        if(FMath::Abs(Delta)<1.e-12) return;
        const double T=(Boundary-Origin)/Delta;
        if(T>0. && T<1.) LandCuts.Add(T);
    };
    const double RiverEdge=BattlefieldLayout::RiverHalfWidth+BattlefieldLayout::VehicleRadius;
    for(int32 Sign:{-1,1})
        AddLandCut(Start.X,LandDelta.X,BattlefieldLayout::RiverX+Sign*RiverEdge);
    const double BridgeEdge=BattlefieldLayout::BridgeHalfWidth-BattlefieldLayout::VehicleRadius;
    for(int32 BridgeSign:{-1,1}) for(int32 EdgeSign:{-1,1})
        AddLandCut(Start.Y,LandDelta.Y,BridgeSign*BattlefieldLayout::BridgeOffsetY+EdgeSign*BridgeEdge);
    LandCuts.Sort();
    for(int32 Index=0;Index<LandCuts.Num();++Index)
    {
        if(!BattlefieldLayout::IsLand(Start+LandDelta*LandCuts[Index])) return false;
        if(Index>0 && !BattlefieldLayout::IsLand(Start+LandDelta*((LandCuts[Index-1]+LandCuts[Index])*.5))) return false;
    }
    // Sampling a long tangent can miss the few centimetres cutting a rounded corner.
    // Validate its entire chord analytically, exactly as short movement steps do.
    for(const auto& Obstacle:Obstacles) if(SegmentHitsObstacle(Start,End,Obstacle)) return false;
    auto HitsCircle=[&](float X,float Y,float Radius)
    {
        FPrototypeNavigationObstacle Circle;Circle.Position=FVector(X,Y,0);Circle.Radius=Radius;
        return SegmentHitsObstacle(Start,End,Circle);
    };
    for(float X:BattlefieldLayout::CityColumns) for(float Y:BattlefieldLayout::CityRows)
        if(HitsCircle(X,Y,BattlefieldLayout::CityObstacleRadius)) return false;
    for(float X:BattlefieldLayout::WestHouseColumns) for(float Y:BattlefieldLayout::WestHouseRows)
        if(HitsCircle(X,Y,BattlefieldLayout::WestHouseRadius)) return false;
    for(const auto& Landmark:BattlefieldLayout::LandmarkFootprints)
        if(HitsCircle(Landmark.X,Landmark.Y,Landmark.Radius)) return false;
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
        Obstacle.HalfExtents = StructureHalfExtents(*It);
        Obstacles.Add(Obstacle);
    }
    // Resource plants are traversable harvesting terrain. Their individual
    // pickups must stay reachable; construction excludes cells separately.
    // The former single-cache obstacle must not survive the multi-cell layout.
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
    FPrototypeNavigationObstacle Rectangle;
    Rectangle.HalfExtents=FVector2D(200,600);
    Rectangle.Radius=180.f; // A smaller legacy circle must not override grid geometry.
    Check(ObstacleContains(Rectangle,FVector(190,590,35)),TEXT("rectangular grid corner blocks outside legacy circle"));
    Check(!ObstacleContains(Rectangle,FVector(290,0,35)) && ObstacleContains(Rectangle,FVector(270,0,35)),
        TEXT("short rectangle edge uses vehicle clearance without circumscribed-circle overblocking"));
    Check(ObstacleContains(Rectangle,FVector(250,650,35)) && !ObstacleContains(Rectangle,FVector(270,670,35)),
        TEXT("rectangular corners retain rounded vehicle clearance"));
    Swap(Rectangle.HalfExtents.X,Rectangle.HalfExtents.Y);
    Check(ObstacleContains(Rectangle,FVector(590,190,35)) && !ObstacleContains(Rectangle,FVector(0,290,35)),
        TEXT("quarter-turn footprint swaps long and short navigation edges"));
    Rectangle.HalfExtents=FVector2D::ZeroVector;
    Check(ObstacleContains(Rectangle,FVector(250,0,35)) && !ObstacleContains(Rectangle,FVector(270,0,35)),
        TEXT("zero rectangle extents preserve radius-only obstacle compatibility"));
    Rectangle.Position=FVector(-8000,0,35);Rectangle.HalfExtents=FVector2D(600,600);
    const FVector Tangent=FVector(1,-1,0).GetSafeNormal2D(),Normal=FVector(1,1,0).GetSafeNormal2D();
    const FVector Corner=Rectangle.Position+FVector(600,600,0);
    const FVector GrazingCenter=Corner+Normal*79.9f;
    const FVector GrazingStart=GrazingCenter-Tangent*180.f,GrazingEnd=GrazingCenter+Tangent*220.f;
    Obstacles={Rectangle};
    bool SampledClear=true;
    for(int32 Sample=0;Sample<=10;++Sample)
        SampledClear&=IsWalkable(FMath::Lerp(GrazingStart,GrazingEnd,Sample/10.f),Obstacles);
    Check(SampledClear && !SegmentWalkable(GrazingStart,GrazingEnd,Obstacles),
        TEXT("analytic rounded-corner chord rejects grazing collision missed by forty-centimetre samples"));
    const FVector SafeCenter=Corner+Normal*80.1f;
    Check(SegmentWalkable(SafeCenter-Tangent*180.f,SafeCenter+Tangent*220.f,Obstacles),
        TEXT("analytic rounded-corner chord preserves genuinely clear tangent without oversized blocker"));
    const FVector BeforeBend=Rectangle.Position+FVector(630,675,0),Bend=Rectangle.Position+FVector(650,690,0),AfterBend=Rectangle.Position+FVector(-700,690,0);
    Check(FVector::Dist2D(BeforeBend,Bend)<30.f && SegmentWalkable(BeforeBend,Bend,Obstacles)
        && SegmentWalkable(Bend,AfterBend,Obstacles) && !SegmentWalkable(BeforeBend,AfterBend,Obstacles),
        TEXT("near intermediate bend cannot be skipped when its replacement chord cuts building clearance"));
    const FVector WaterClipStart(-2040,2399.8,35),WaterClipEnd(-2440,2799.8,35);
    bool SampledLand=true;
    for(int32 Sample=0;Sample<=15;++Sample)
        SampledLand&=BattlefieldLayout::IsLand(FMath::Lerp(WaterClipStart,WaterClipEnd,Sample/15.));
    Check(SampledLand && !SegmentWalkable(WaterClipStart,WaterClipEnd),
        TEXT("analytic bridge intervals reject sub-centimetre water clip missed by forty-centimetre samples"));
    Check(SegmentWalkable(FVector(-2040,2400.2,35),FVector(-2440,2800.2,35)),
        TEXT("analytic bridge intervals preserve clear bank-to-bridge corner chord"));
    UE_LOG(LogTemp, Display, TEXT("NAVIGATION_TEST_RESULT=%s"), Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}
