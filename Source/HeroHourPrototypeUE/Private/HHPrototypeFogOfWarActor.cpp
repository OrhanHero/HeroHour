#include "HHPrototypeFogOfWarActor.h"

#include "HHPrototypeCombatActor.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"

AHHPrototypeFogOfWarActor::AHHPrototypeFogOfWarActor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;
    SetActorEnableCollision(false);
}

void AHHPrototypeFogOfWarActor::BeginPlay()
{
    Super::BeginPlay();
    InitializeGrid();
    RefreshVisibility();
}

void AHHPrototypeFogOfWarActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TimeUntilRefresh -= DeltaSeconds;
    if (TimeUntilRefresh <= 0.0f)
    {
        RefreshVisibility();
        TimeUntilRefresh = FMath::Max(0.05f, UpdateInterval);
    }
    if (bDrawDebugGrid) DrawGridDebug();
}

void AHHPrototypeFogOfWarActor::InitializeGrid()
{
    GridSize.X = FMath::Clamp(GridSize.X, 4, 256);
    GridSize.Y = FMath::Clamp(GridSize.Y, 4, 256);
    BattlefieldExtent.X = FMath::Max(100.0f, BattlefieldExtent.X);
    BattlefieldExtent.Y = FMath::Max(100.0f, BattlefieldExtent.Y);
    VisibilityCells.Init(static_cast<uint8>(EHHFogVisibility::Unseen), GridSize.X * GridSize.Y);
}

void AHHPrototypeFogOfWarActor::RefreshVisibility()
{
    if (VisibilityCells.Num() != GridSize.X * GridSize.Y) InitializeGrid();

    for (uint8& Cell : VisibilityCells)
    {
        if (Cell == static_cast<uint8>(EHHFogVisibility::Visible))
            Cell = static_cast<uint8>(EHHFogVisibility::Explored);
    }

    // Prototype ownership hook: HERO is the local player's faction. Replace this iterator
    // with replicated team vision providers when multiplayer ownership is introduced.
    for (TActorIterator<AHHPrototypeCombatActor> It(GetWorld()); It; ++It)
    {
        const AHHPrototypeCombatActor* Source = *It;
        if (Source->IsAlive() && Source->GetFaction() == EHHPrototypeFaction::Hero)
            RevealCircle(Source->GetActorLocation(), Source->IsHeadquarters() ? HeadquartersSightRadius : UnitSightRadius);
    }
}

EHHFogVisibility AHHPrototypeFogOfWarActor::GetVisibilityAtWorldLocation(const FVector& WorldLocation) const
{
    const int32 Index = WorldToCellIndex(WorldLocation);
    return VisibilityCells.IsValidIndex(Index)
        ? static_cast<EHHFogVisibility>(VisibilityCells[Index]) : EHHFogVisibility::Unseen;
}

bool AHHPrototypeFogOfWarActor::IsWorldLocationVisible(const FVector& WorldLocation) const
{
    return GetVisibilityAtWorldLocation(WorldLocation) == EHHFogVisibility::Visible;
}

bool AHHPrototypeFogOfWarActor::IsWorldLocationExplored(const FVector& WorldLocation) const
{
    return GetVisibilityAtWorldLocation(WorldLocation) != EHHFogVisibility::Unseen;
}

bool AHHPrototypeFogOfWarActor::WorldToRadarUV(const FVector& WorldLocation, FVector2D& OutRadarUV) const
{
    const FVector2D Min = BattlefieldCenter - BattlefieldExtent;
    const FVector2D Size = BattlefieldExtent * 2.0f;
    OutRadarUV = FVector2D((WorldLocation.X - Min.X) / Size.X, (WorldLocation.Y - Min.Y) / Size.Y);
    const bool bInside = OutRadarUV.X >= 0.0f && OutRadarUV.X <= 1.0f && OutRadarUV.Y >= 0.0f && OutRadarUV.Y <= 1.0f;
    OutRadarUV.X = FMath::Clamp(OutRadarUV.X, 0.0f, 1.0f);
    OutRadarUV.Y = FMath::Clamp(OutRadarUV.Y, 0.0f, 1.0f);
    return bInside;
}

FVector AHHPrototypeFogOfWarActor::RadarUVToWorld(const FVector2D& RadarUV, const float WorldZ) const
{
    const FVector2D Min = BattlefieldCenter - BattlefieldExtent;
    const FVector2D UV(FMath::Clamp(RadarUV.X, 0.0f, 1.0f), FMath::Clamp(RadarUV.Y, 0.0f, 1.0f));
    return FVector(Min.X + UV.X * BattlefieldExtent.X * 2.0f,
        Min.Y + UV.Y * BattlefieldExtent.Y * 2.0f, WorldZ);
}

int32 AHHPrototypeFogOfWarActor::WorldToCellIndex(const FVector& WorldLocation) const
{
    FVector2D UV;
    if (!WorldToRadarUV(WorldLocation, UV)) return INDEX_NONE;
    const int32 X = FMath::Min(FMath::FloorToInt(UV.X * GridSize.X), GridSize.X - 1);
    const int32 Y = FMath::Min(FMath::FloorToInt(UV.Y * GridSize.Y), GridSize.Y - 1);
    return Y * GridSize.X + X;
}

FVector AHHPrototypeFogOfWarActor::CellCenterToWorld(const int32 X, const int32 Y) const
{
    return RadarUVToWorld(FVector2D((X + 0.5f) / GridSize.X, (Y + 0.5f) / GridSize.Y), 18.0f);
}

void AHHPrototypeFogOfWarActor::RevealCircle(const FVector& WorldLocation, const float Radius)
{
    FVector2D CenterUV;
    WorldToRadarUV(WorldLocation, CenterUV);
    const int32 CenterX = FMath::FloorToInt(CenterUV.X * GridSize.X);
    const int32 CenterY = FMath::FloorToInt(CenterUV.Y * GridSize.Y);
    const FVector2D CellSize(BattlefieldExtent.X * 2.0f / GridSize.X, BattlefieldExtent.Y * 2.0f / GridSize.Y);
    const int32 CellsX = FMath::CeilToInt(Radius / CellSize.X);
    const int32 CellsY = FMath::CeilToInt(Radius / CellSize.Y);
    const float RadiusSquared = FMath::Square(Radius + 0.5f * CellSize.Size());

    for (int32 Y = FMath::Max(0, CenterY - CellsY); Y <= FMath::Min(GridSize.Y - 1, CenterY + CellsY); ++Y)
    for (int32 X = FMath::Max(0, CenterX - CellsX); X <= FMath::Min(GridSize.X - 1, CenterX + CellsX); ++X)
    {
        if (FVector::DistSquared2D(CellCenterToWorld(X, Y), WorldLocation) <= RadiusSquared)
            VisibilityCells[Y * GridSize.X + X] = static_cast<uint8>(EHHFogVisibility::Visible);
    }
}

void AHHPrototypeFogOfWarActor::DrawGridDebug() const
{
    if (!GetWorld()) return;
    const FVector2D CellSize(BattlefieldExtent.X * 2.0f / GridSize.X, BattlefieldExtent.Y * 2.0f / GridSize.Y);
    for (int32 Y = 0; Y < GridSize.Y; ++Y)
    for (int32 X = 0; X < GridSize.X; ++X)
    {
        const EHHFogVisibility State = static_cast<EHHFogVisibility>(VisibilityCells[Y * GridSize.X + X]);
        if (State == EHHFogVisibility::Visible) continue;
        const FColor Color = State == EHHFogVisibility::Explored ? FColor(35, 45, 55) : FColor::Black;
        DrawDebugBox(GetWorld(), CellCenterToWorld(X, Y), FVector(CellSize.X, CellSize.Y, 4.0f) * 0.48f,
            Color, false, 0.0f, 0, State == EHHFogVisibility::Unseen ? 2.0f : 1.0f);
    }
}
