#pragma once
#include "CoreMinimal.h"

namespace BattlefieldLayout
{
constexpr float ExtentX = 12000.f;
constexpr float ExtentY = 9000.f;
constexpr float RiverX = -3000.f;
constexpr float RiverHalfWidth = 700.f;
constexpr float BridgeOffsetY = 3200.f;
constexpr float BridgeHalfWidth = 700.f;
constexpr float CellSize = 200.f;
constexpr float VehicleRadius = 80.f;
constexpr float PlayerHQX = -8000.f;
constexpr float EnemyHQX = 8000.f;
constexpr float SalvageColumns[] = {-6500.f,7000.f,0.f};
constexpr float SalvageRows[] = {1600.f,750.f,-3200.f};
constexpr float CityObstacleRadius = 600.f;
constexpr float CityColumns[] = {1500.f, 3000.f, 4500.f, 6000.f};
constexpr float CityRows[] = {-4500.f, -3000.f, -1500.f, 0.f, 1500.f, 3000.f, 4500.f};
constexpr float WestHouseColumns[] = {-9500.f, -8250.f, -7000.f, -5750.f};
constexpr float WestHouseRows[] = {3000.f, 4250.f, 5500.f, 6750.f};
constexpr float WestHouseRadius = 430.f;
struct FStaticFootprint { float X; float Y; float Radius; };
constexpr FStaticFootprint LandmarkFootprints[] = {
    {8900.f,3900.f,700.f}, {10300.f,5600.f,700.f}, {8700.f,7000.f,700.f},
    {3000.f,7000.f,1400.f}, {-9000.f,-6000.f,900.f}
};

inline bool IsStaticClear(const FVector& Point, float Clearance = VehicleRadius)
{
    for (float X : CityColumns)
    for (float Y : CityRows)
        if (FVector::DistSquared2D(Point,FVector(X,Y,Point.Z)) < FMath::Square(CityObstacleRadius + Clearance)) return false;
    for (float X : WestHouseColumns)
    for (float Y : WestHouseRows)
        if (FVector::DistSquared2D(Point,FVector(X,Y,Point.Z)) < FMath::Square(WestHouseRadius + Clearance)) return false;
    for (const FStaticFootprint& Landmark : LandmarkFootprints)
        if (FVector::DistSquared2D(Point,FVector(Landmark.X,Landmark.Y,Point.Z)) < FMath::Square(Landmark.Radius + Clearance)) return false;
    return true;
}

inline bool InBounds(const FVector& Point, float Margin = 0.f)
{
    return FMath::Abs(Point.X) <= ExtentX - Margin && FMath::Abs(Point.Y) <= ExtentY - Margin;
}

inline bool IsLand(const FVector& Point, float Clearance = VehicleRadius)
{
    if (!InBounds(Point, Clearance)) return false;
    if (FMath::Abs(Point.X - RiverX) > RiverHalfWidth + Clearance) return true;
    return FMath::Abs(Point.Y - BridgeOffsetY) <= BridgeHalfWidth - Clearance
        || FMath::Abs(Point.Y + BridgeOffsetY) <= BridgeHalfWidth - Clearance;
}

inline FVector ClampPoint(const FVector& Point)
{
    return FVector(FMath::Clamp(Point.X, -ExtentX + VehicleRadius, ExtentX - VehicleRadius),
        FMath::Clamp(Point.Y, -ExtentY + VehicleRadius, ExtentY - VehicleRadius), Point.Z);
}
}

struct FPrototypeNavigationObstacle
{
    FVector Position = FVector::ZeroVector;
    float Radius = 0.f;
};

class HEROHOURPROTOTYPE_API FPrototypeNavigation
{
public:
    // Output excludes Start and includes the reachable goal. Water goals are rejected.
    static bool FindPath(const FVector& Start, const FVector& Goal, TArray<FVector>& OutPath,
        const TArray<FPrototypeNavigationObstacle>& Obstacles = {});
    static bool FindWorldPath(class UWorld* World, const FVector& Start, const FVector& Goal,
        TArray<FVector>& OutPath, const class APrototypeUnit* IgnoreStructure = nullptr);
    // Static city blocks are always included by IsWalkable; this returns dynamic building circles.
    static TArray<FPrototypeNavigationObstacle> GetWorldObstacles(class UWorld* World,
        const class APrototypeUnit* IgnoreStructure = nullptr);
    static bool SegmentWorldWalkable(class UWorld* World, const FVector& Start, const FVector& End,
        const class APrototypeUnit* IgnoreStructure = nullptr);
    static bool IsWalkable(const FVector& Point, const TArray<FPrototypeNavigationObstacle>& Obstacles = {});
    static bool SegmentWalkable(const FVector& Start, const FVector& End,
        const TArray<FPrototypeNavigationObstacle>& Obstacles = {});
    static bool RunSelfTests();
};
