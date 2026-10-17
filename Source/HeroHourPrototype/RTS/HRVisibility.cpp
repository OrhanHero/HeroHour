#include "HRVisibility.h"

namespace
{
    constexpr float UnknownAlpha = .92f;
    constexpr float ExploredAlpha = .55f;
    constexpr double SmokeProximitySq = 400.0 * 400.0;
    constexpr int32 MaxGridCells = 1024 * 1024;
    constexpr int32 MaxTeams = 16;
    constexpr int32 MaxSmokeVolumes = 128;

    bool Finite2D(const FVector2D& Value)
    {
        return FMath::IsFinite(Value.X) && FMath::IsFinite(Value.Y);
    }
}

bool FHRVisibilityGrid::IsFinitePosition(const FVector& Position)
{
    return FMath::IsFinite(Position.X) && FMath::IsFinite(Position.Y)
        && FMath::IsFinite(Position.Z);
}

bool FHRVisibilityGrid::Reset(const FVector2D& InMin, const FVector2D& InMax,
    float InCellSize, int32 InTeamCount)
{
    if (!Finite2D(InMin) || !Finite2D(InMax) || !FMath::IsFinite(InCellSize)
        || InCellSize < 1.f || InMax.X <= InMin.X || InMax.Y <= InMin.Y
        || InTeamCount <= 0 || InTeamCount > MaxTeams) return false;
    const double Columns = FMath::CeilToDouble((InMax.X - InMin.X) / InCellSize);
    const double Rows = FMath::CeilToDouble((InMax.Y - InMin.Y) / InCellSize);
    if (!FMath::IsFinite(Columns) || !FMath::IsFinite(Rows)
        || Columns > MaxGridCells || Rows > MaxGridCells
        || Columns * Rows > MaxGridCells) return false;

    BoundsMin = InMin;
    BoundsMax = InMax;
    CellSize = InCellSize;
    Width = static_cast<int32>(Columns);
    Height = static_cast<int32>(Rows);
    TeamStates.SetNum(InTeamCount);
    for (FTeamState& State : TeamStates)
    {
        State.Visible.Init(0, Width * Height);
        State.Explored.Init(0, Width * Height);
        State.Sources.Reset();
    }
    CurrentSources.Reset();
    SmokeVolumes.Reset();
    NextSmokeId = 1;
    SimulationSeconds = 0.0;
    return true;
}

bool FHRVisibilityGrid::Advance(float DeltaSeconds)
{
    if (Width <= 0 || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds < 0.f) return false;
    SimulationSeconds += static_cast<double>(DeltaSeconds);
    bool bExpired = false;
    for (int32 Index = SmokeVolumes.Num() - 1; Index >= 0; --Index)
    {
        FHRSmokeVolume& Volume = SmokeVolumes[Index];
        Volume.RemainingSeconds -= static_cast<double>(DeltaSeconds);
        if (Volume.RemainingSeconds <= 0.0)
        {
            SmokeVolumes.RemoveAtSwap(Index);
            bExpired = true;
        }
    }
    if (bExpired) RebuildVision(CurrentSources);
    return true;
}

bool FHRVisibilityGrid::IsInside(const FVector& Position) const
{
    return Width > 0 && IsFinitePosition(Position)
        && Position.X >= BoundsMin.X && Position.X < BoundsMax.X
        && Position.Y >= BoundsMin.Y && Position.Y < BoundsMax.Y;
}

int32 FHRVisibilityGrid::GetIndex(const FVector& Position) const
{
    if (!IsInside(Position)) return INDEX_NONE;
    return GetIndex(FMath::FloorToInt((Position.X - BoundsMin.X) / CellSize),
        FMath::FloorToInt((Position.Y - BoundsMin.Y) / CellSize));
}

int32 FHRVisibilityGrid::GetIndex(int32 X, int32 Y) const
{
    return X >= 0 && X < Width && Y >= 0 && Y < Height ? Y * Width + X : INDEX_NONE;
}

FVector FHRVisibilityGrid::GetCellCenter(int32 X, int32 Y) const
{
    if (GetIndex(X, Y) == INDEX_NONE) return FVector::ZeroVector;
    const double Left = BoundsMin.X + X * static_cast<double>(CellSize);
    const double Top = BoundsMin.Y + Y * static_cast<double>(CellSize);
    return FVector((Left + FMath::Min(Left + CellSize, BoundsMax.X)) * .5,
        (Top + FMath::Min(Top + CellSize, BoundsMax.Y)) * .5, 0.0);
}

void FHRVisibilityGrid::RebuildVision(const TArray<FHRVisionSource>& Sources)
{
    // Copy first: smoke changes rebuild using CurrentSources itself.
    const TArray<FHRVisionSource> Input = Sources;
    CurrentSources.Reset();
    for (FTeamState& State : TeamStates)
    {
        State.Visible.Init(0, Width * Height);
        State.Sources.Reset();
    }
    for (const FHRVisionSource& Source : Input)
    {
        if (!IsValidTeam(Source.Team) || !IsFinitePosition(Source.Position)
            || !FMath::IsFinite(Source.SightRadius) || Source.SightRadius <= 0.f
            || !IsInside(Source.Position)) continue;
        CurrentSources.Add(Source);
        FTeamState& State = TeamStates[Source.Team];
        State.Sources.Add(Source);

        // Clamp in floating point before converting hostile/unbounded radii to indices.
        const double Left = FMath::Max(BoundsMin.X, Source.Position.X - Source.SightRadius);
        const double Right = FMath::Min(BoundsMax.X, Source.Position.X + Source.SightRadius);
        const double Top = FMath::Max(BoundsMin.Y, Source.Position.Y - Source.SightRadius);
        const double Bottom = FMath::Min(BoundsMax.Y, Source.Position.Y + Source.SightRadius);
        const int32 MinX = FMath::Clamp(FMath::FloorToInt((Left - BoundsMin.X) / CellSize), 0, Width - 1);
        const int32 MaxX = FMath::Clamp(FMath::FloorToInt((Right - BoundsMin.X) / CellSize), 0, Width - 1);
        const int32 MinY = FMath::Clamp(FMath::FloorToInt((Top - BoundsMin.Y) / CellSize), 0, Height - 1);
        const int32 MaxY = FMath::Clamp(FMath::FloorToInt((Bottom - BoundsMin.Y) / CellSize), 0, Height - 1);
        const double RadiusSq = FMath::Square(static_cast<double>(Source.SightRadius));
        for (int32 Y = MinY; Y <= MaxY; ++Y)
        {
            for (int32 X = MinX; X <= MaxX; ++X)
            {
                const int32 Index = Y * Width + X;
                if (State.Visible[Index]) continue;
                const double CellLeft = BoundsMin.X + X * static_cast<double>(CellSize);
                const double CellTop = BoundsMin.Y + Y * static_cast<double>(CellSize);
                const FVector Nearest(FMath::Clamp(Source.Position.X, CellLeft,
                        FMath::Min(CellLeft + CellSize, BoundsMax.X) - UE_SMALL_NUMBER),
                    FMath::Clamp(Source.Position.Y, CellTop,
                        FMath::Min(CellTop + CellSize, BoundsMax.Y) - UE_SMALL_NUMBER), 0.0);
                if (FVector::DistSquared2D(Source.Position, Nearest) > RadiusSq) continue;
                // Avoid rendering a clear cell through the centre of hostile smoke.
                if (IsSmokeConcealed(Source.Team, Source.Position, GetCellCenter(X, Y))) continue;
                State.Visible[Index] = 1;
                State.Explored[Index] = 1;
            }
        }
    }
}

bool FHRVisibilityGrid::IsVisible(int32 Team, const FVector& WorldPosition) const
{
    if (!IsValidTeam(Team) || !IsInside(WorldPosition)) return false;
    for (const FHRVisionSource& Source : TeamStates[Team].Sources)
    {
        if (FVector::DistSquared2D(Source.Position, WorldPosition)
                <= FMath::Square(static_cast<double>(Source.SightRadius))
            && !IsSmokeConcealed(Team, Source.Position, WorldPosition)) return true;
    }
    return false;
}

bool FHRVisibilityGrid::IsExplored(int32 Team, const FVector& WorldPosition) const
{
    const int32 Index = GetIndex(WorldPosition);
    return IsValidTeam(Team) && Index != INDEX_NONE && TeamStates[Team].Explored[Index] != 0;
}

bool FHRVisibilityGrid::IsAnyFootprintVisible(int32 Team, const FVector& Position, float Radius) const
{
    if (!IsValidTeam(Team) || !IsFinitePosition(Position)
        || !FMath::IsFinite(Radius) || Radius < 0.f) return false;
    if (IsVisible(Team, Position)) return true;
    if (Radius == 0.f) return false;
    const double RadiusSq = FMath::Square(static_cast<double>(Radius));
    for (const FHRVisionSource& Source : TeamStates[Team].Sources)
    {
        FVector Direction = Source.Position - Position;
        Direction.Z = 0.0;
        const double Distance = Direction.Size2D();
        if (Distance > static_cast<double>(Source.SightRadius) + Radius) continue;
        const FVector Nearest = Distance > UE_SMALL_NUMBER
            ? Position + Direction * (FMath::Min(static_cast<double>(Radius), Distance) / Distance)
            : Position;
        if (IsVisible(Team, Nearest)) return true;
        // A circle at a map edge may have its nearest point outside the playable bounds.
        const FVector Clipped(FMath::Clamp(Nearest.X, BoundsMin.X, BoundsMax.X - UE_SMALL_NUMBER),
            FMath::Clamp(Nearest.Y, BoundsMin.Y, BoundsMax.Y - UE_SMALL_NUMBER), Position.Z);
        if (FVector::DistSquared2D(Position, Clipped) <= RadiusSq && IsVisible(Team, Clipped)) return true;
        // If smoke conceals the nearest point, a different edge of a large building may show.
        for (int32 Sample = 0; Sample < 24; ++Sample)
        {
            const double Angle = Sample * (2.0 * UE_PI / 24.0);
            const FVector Edge = Position + FVector(FMath::Cos(Angle) * Radius,
                FMath::Sin(Angle) * Radius, 0.0);
            if (IsVisible(Team, Edge)) return true;
        }
    }
    return false;
}

bool FHRVisibilityGrid::IsSmokeConcealed(int32 ObserverTeam, const FVector& ObserverPosition,
    const FVector& TargetPosition) const
{
    if (!IsValidTeam(ObserverTeam) || !IsFinitePosition(ObserverPosition)
        || !IsFinitePosition(TargetPosition)) return true;
    const FVector2D Start(ObserverPosition.X, ObserverPosition.Y);
    const FVector2D End(TargetPosition.X, TargetPosition.Y);
    const FVector2D Segment = End - Start;
    const double LengthSq = Segment.SizeSquared();
    if (LengthSq <= SmokeProximitySq) return false;
    for (const FHRSmokeVolume& Smoke : SmokeVolumes)
    {
        if (Smoke.OwnerTeam == ObserverTeam) continue;
        const FVector2D Center(Smoke.Position.X, Smoke.Position.Y);
        const double Along = FMath::Clamp(FVector2D::DotProduct(Center - Start, Segment) / LengthSq, 0.0, 1.0);
        const FVector2D Nearest = Start + Segment * Along;
        if (FVector2D::DistSquared(Center, Nearest) <= FMath::Square(static_cast<double>(Smoke.Radius))) return true;
    }
    return false;
}

int32 FHRVisibilityGrid::AddSmoke(int32 OwnerTeam, const FVector& Position, float Radius, float Lifetime)
{
    if (!IsValidTeam(OwnerTeam) || !IsInside(Position) || !FMath::IsFinite(Radius)
        || !FMath::IsFinite(Lifetime) || Radius <= 0.f || Lifetime <= 0.f
        || SmokeVolumes.Num() >= MaxSmokeVolumes || NextSmokeId == MAX_int32) return INDEX_NONE;
    FHRSmokeVolume Smoke;
    Smoke.Id = NextSmokeId++;
    Smoke.OwnerTeam = OwnerTeam;
    Smoke.Position = Position;
    Smoke.Radius = FMath::Min(Radius, 600.f);
    Smoke.RemainingSeconds = FMath::Min(Lifetime, 6.f);
    SmokeVolumes.Add(Smoke);
    RebuildVision(CurrentSources);
    return Smoke.Id;
}

bool FHRVisibilityGrid::RemoveSmoke(int32 Id)
{
    const int32 Index = SmokeVolumes.IndexOfByPredicate([Id](const FHRSmokeVolume& Smoke) { return Smoke.Id == Id; });
    if (Index == INDEX_NONE) return false;
    SmokeVolumes.RemoveAtSwap(Index);
    RebuildVision(CurrentSources);
    return true;
}

void FHRVisibilityGrid::ClearSmoke()
{
    SmokeVolumes.Reset();
    RebuildVision(CurrentSources);
}

float FHRVisibilityGrid::AlphaForState(bool bVisible, bool bExplored)
{
    return bVisible ? 0.f : bExplored ? ExploredAlpha : UnknownAlpha;
}

float FHRVisibilityGrid::GetFogAlpha(int32 Team, const FVector& WorldPosition) const
{
    const int32 Index = GetIndex(WorldPosition);
    if (!IsValidTeam(Team) || Index == INDEX_NONE) return UnknownAlpha;
    const FTeamState& State = TeamStates[Team];
    return AlphaForState(State.Visible[Index] != 0, State.Explored[Index] != 0);
}

void FHRVisibilityGrid::GetFogPixels(int32 Team, TArray<FColor>& OutPixels) const
{
    OutPixels.SetNum(Width * Height);
    for (int32 Index = 0; Index < OutPixels.Num(); ++Index)
    {
        const bool bVisible = IsValidTeam(Team) && TeamStates[Team].Visible[Index] != 0;
        const bool bExplored = IsValidTeam(Team) && TeamStates[Team].Explored[Index] != 0;
        OutPixels[Index] = FColor(0, 0, 0,
            static_cast<uint8>(FMath::RoundToInt(255.f * AlphaForState(bVisible, bExplored))));
    }
}

bool FHRVisibilityGrid::IsCellVisible(int32 Team, int32 X, int32 Y) const
{
    const int32 Index = GetIndex(X, Y);
    return IsValidTeam(Team) && Index != INDEX_NONE && TeamStates[Team].Visible[Index] != 0;
}

bool FHRVisibilityGrid::IsCellExplored(int32 Team, int32 X, int32 Y) const
{
    const int32 Index = GetIndex(X, Y);
    return IsValidTeam(Team) && Index != INDEX_NONE && TeamStates[Team].Explored[Index] != 0;
}
