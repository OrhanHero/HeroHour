#pragma once

#include "CoreMinimal.h"

/** A live, completed unit or building contributes vision in centimetres. */
struct HEROHOURPROTOTYPE_API FHRVisionSource
{
    int32 Team = INDEX_NONE;
    FVector Position = FVector::ZeroVector;
    float SightRadius = 2600.f;
};

/** Smoke belongs to its casting team and ages only with simulation time. */
struct HEROHOURPROTOTYPE_API FHRSmokeVolume
{
    int32 Id = INDEX_NONE;
    int32 OwnerTeam = INDEX_NONE;
    FVector Position = FVector::ZeroVector;
    float Radius = 600.f;
    double RemainingSeconds = 6.0;
};

/**
 * Independent current visibility and persistent exploration for each team.
 * The mask is a conservative cell approximation for world/minimap rendering;
 * actor/command queries use exact source distances and smoke intersections so
 * a revealed cell cannot disclose a target outside actual sight range.
 * This game-thread value type owns no actors, materials or textures.
 */
class HEROHOURPROTOTYPE_API FHRVisibilityGrid
{
public:
    /** Invalid settings preserve the existing grid. A valid reset erases knowledge. */
    bool Reset(const FVector2D& InMin, const FVector2D& InMax,
        float InCellSize = 400.f, int32 InTeamCount = 2);

    /** Advance exactly once per simulation step; invalid/negative dt is rejected. */
    bool Advance(float DeltaSeconds);
    void RebuildVision(const TArray<FHRVisionSource>& Sources);

    bool IsVisible(int32 Team, const FVector& WorldPosition) const;
    bool IsExplored(int32 Team, const FVector& WorldPosition) const;
    bool IsAnyFootprintVisible(int32 Team, const FVector& Position, float Radius) const;

    /** Returns a stable ID, or INDEX_NONE. Radius/lifetime are capped at 600 cm/6 s. */
    int32 AddSmoke(int32 OwnerTeam, const FVector& Position,
        float Radius = 600.f, float Lifetime = 6.f);
    bool RemoveSmoke(int32 Id);
    void ClearSmoke();
    /** Friendly smoke is transparent. At <=400 cm any smoke can be seen through. */
    bool IsSmokeConcealed(int32 ObserverTeam, const FVector& ObserverPosition,
        const FVector& TargetPosition) const;
    const TArray<FHRSmokeVolume>& GetSmokeVolumes() const { return SmokeVolumes; }

    /** Unknown=.92, explored=.55, visible=0. Pixels are black with that alpha. */
    float GetFogAlpha(int32 Team, const FVector& WorldPosition) const;
    void GetFogPixels(int32 Team, TArray<FColor>& OutPixels) const;
    bool IsCellVisible(int32 Team, int32 X, int32 Y) const;
    bool IsCellExplored(int32 Team, int32 X, int32 Y) const;
    FVector GetCellCenter(int32 X, int32 Y) const;
    int32 GetWidth() const { return Width; }
    int32 GetHeight() const { return Height; }
    int32 GetTeamCount() const { return TeamStates.Num(); }
    float GetCellSize() const { return CellSize; }
    const FVector2D& GetBoundsMin() const { return BoundsMin; }
    const FVector2D& GetBoundsMax() const { return BoundsMax; }
    double GetSimulationSeconds() const { return SimulationSeconds; }

private:
    struct FTeamState
    {
        TArray<uint8> Visible;
        TArray<uint8> Explored;
        TArray<FHRVisionSource> Sources;
    };
    bool IsValidTeam(int32 Team) const { return TeamStates.IsValidIndex(Team); }
    bool IsInside(const FVector& Position) const;
    int32 GetIndex(const FVector& Position) const;
    int32 GetIndex(int32 X, int32 Y) const;
    static bool IsFinitePosition(const FVector& Position);
    static float AlphaForState(bool bVisible, bool bExplored);

    FVector2D BoundsMin = FVector2D::ZeroVector;
    FVector2D BoundsMax = FVector2D::ZeroVector;
    float CellSize = 400.f;
    int32 Width = 0;
    int32 Height = 0;
    int32 NextSmokeId = 1;
    double SimulationSeconds = 0.0;
    TArray<FTeamState> TeamStates;
    TArray<FHRVisionSource> CurrentSources;
    TArray<FHRSmokeVolume> SmokeVolumes;
};

HEROHOURPROTOTYPE_API bool RunHRVisibilitySelfTests();
