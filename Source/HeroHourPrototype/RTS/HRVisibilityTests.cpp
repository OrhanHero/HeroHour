#include "HRVisibility.h"
#include <limits>

bool RunHRVisibilitySelfTests()
{
    bool Passed = true;
    int32 Count = 0;
    auto Check = [&](bool Condition, const TCHAR* Label)
    {
        ++Count;
        Passed &= Condition;
        UE_LOG(LogTemp, Display, TEXT("VISIBILITY_TEST %s: %s"), Label,
            Condition ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto Source = [](int32 Team, double X, double Y, float Radius)
    {
        FHRVisionSource Result;
        Result.Team = Team;
        Result.Position = FVector(X, Y, 0.0);
        Result.SightRadius = Radius;
        return Result;
    };
    const FVector2D Min(-12000.0, -9000.0);
    const FVector2D Max(12000.0, 9000.0);
    const float NaN = std::numeric_limits<float>::quiet_NaN();
    const float Infinity = std::numeric_limits<float>::infinity();
    FHRVisibilityGrid Grid;
    Check(Grid.Reset(Min, Max, 400.f, 2) && Grid.GetWidth() == 60
        && Grid.GetHeight() == 45 && Grid.GetTeamCount() == 2,
        TEXT("24000x18000 map creates independent 60x45 team grids"));
    Check(!Grid.Reset(Max, Min, 400.f, 2) && !Grid.Reset(Min, Max, 0.f, 2)
        && !Grid.Reset(Min, Max, NaN, 2) && !Grid.Reset(Min, Max, 1.f, 2)
        && !Grid.Reset(Min, Max, 400.f, 0) && Grid.GetWidth() == 60,
        TEXT("invalid resets reject malformed or oversized grids and preserve state"));
    TArray<FColor> Pixels;
    Grid.GetFogPixels(0, Pixels);
    Check(Pixels.Num() == 2700 && Pixels[0] == FColor(0, 0, 0, 235)
        && !Grid.IsVisible(0, FVector::ZeroVector) && !Grid.IsExplored(0, FVector::ZeroVector),
        TEXT("a new team knows no terrain or enemies and mask begins opaque"));

    TArray<FHRVisionSource> Sources;
    Sources.Add(Source(0, 0.0, 0.0, 1000.f));
    Sources.Add(Source(1, 6000.0, 0.0, 700.f));
    Sources.Add(Source(99, 0.0, 0.0, 20000.f));
    Sources.Add(Source(0, 0.0, 0.0, NaN));
    Sources.Add(Source(0, Infinity, 0.0, 20000.f));
    Sources.Add(Source(0, 13000.0, 0.0, 20000.f));
    Grid.RebuildVision(Sources);
    Check(Grid.IsVisible(0, FVector::ZeroVector) && Grid.IsExplored(0, FVector::ZeroVector)
        && !Grid.IsVisible(1, FVector::ZeroVector) && !Grid.IsExplored(1, FVector::ZeroVector)
        && Grid.IsVisible(1, FVector(6000.0, 0.0, 0.0))
        && !Grid.IsVisible(0, FVector(6000.0, 0.0, 0.0)),
        TEXT("teams reveal only their own source circles; malformed sources disclose nothing"));
    Check(Grid.IsVisible(0, FVector(1000.0, 0.0, 0.0))
        && !Grid.IsVisible(0, FVector(1001.0, 0.0, 0.0))
        && !Grid.IsVisible(0, FVector(950.0, 950.0, 0.0)),
        TEXT("target queries use exact circular sight instead of leaking across raster cells"));
    Check(Grid.IsAnyFootprintVisible(0, FVector(1500.0, 0.0, 0.0), 501.f)
        && !Grid.IsAnyFootprintVisible(0, FVector(1500.0, 0.0, 0.0), 499.f)
        && Grid.IsAnyFootprintVisible(0, FVector::ZeroVector, 0.f)
        && !Grid.IsAnyFootprintVisible(0, FVector::ZeroVector, -1.f),
        TEXT("a structure edge can reveal its footprint without revealing an out of range centre"));
    Check(!Grid.IsVisible(-1, FVector::ZeroVector) && !Grid.IsVisible(2, FVector::ZeroVector)
        && !Grid.IsExplored(99, FVector::ZeroVector)
        && !Grid.IsVisible(0, FVector(12000.0, 0.0, 0.0))
        && !Grid.IsVisible(0, FVector(0.0, 9000.0, 0.0))
        && !Grid.IsVisible(0, FVector(NaN, 0.0, 0.0)),
        TEXT("invalid teams, nonfinite positions and offmap targets are never visible"));
    Grid.GetFogPixels(0, Pixels);
    Check(Grid.GetFogAlpha(0, FVector::ZeroVector) == 0.f
        && Grid.GetFogAlpha(0, FVector(10000.0, 8000.0, 0.0)) == .92f
        && Pixels[22 * 60 + 30].A == 0 && Pixels.Last().A == 235,
        TEXT("texture mask matches visible and unknown world regions in row-major order"));

    Grid.RebuildVision(TArray<FHRVisionSource>());
    Check(!Grid.IsVisible(0, FVector::ZeroVector) && Grid.IsExplored(0, FVector::ZeroVector)
        && Grid.GetFogAlpha(0, FVector::ZeroVector) == .55f
        && !Grid.IsExplored(1, FVector::ZeroVector),
        TEXT("removing a scout restores fog while preserving only that team's exploration"));
    Grid.GetFogPixels(0, Pixels);
    Check(Pixels[22 * 60 + 30].A == 140 && Pixels.Last().A == 235,
        TEXT("explored texture pixels remain distinct from never visited terrain"));

    Sources.Reset();
    Sources.Add(Source(0, -12000.0, -9000.0, 600.f));
    Grid.RebuildVision(Sources);
    Check(Grid.IsVisible(0, FVector(-12000.0, -9000.0, 0.0))
        && !Grid.IsVisible(0, FVector(-12000.1, -9000.0, 0.0))
        && Grid.IsAnyFootprintVisible(0, FVector(-12100.0, -9000.0, 0.0), 200.f)
        && !Grid.IsAnyFootprintVisible(0, FVector(-13000.0, -9000.0, 0.0), 200.f),
        TEXT("map minimum is included and border footprints reveal only their in-map part"));

    Sources.Reset();
    Sources.Add(Source(0, -2000.0, 0.0, 5000.f));
    Sources.Add(Source(1, 0.0, 0.0, 2200.f));
    Grid.RebuildVision(Sources);
    const int32 SmokeId = Grid.AddSmoke(1, FVector::ZeroVector, 500.f, 6.f);
    Check(SmokeId != INDEX_NONE && Grid.GetSmokeVolumes().Num() == 1
        && !Grid.IsVisible(0, FVector::ZeroVector) && Grid.IsVisible(1, FVector::ZeroVector),
        TEXT("hostile smoke immediately conceals targets while friendly smoke preserves vision"));
    Check(Grid.IsAnyFootprintVisible(0, FVector::ZeroVector, 900.f)
        && !Grid.IsAnyFootprintVisible(0, FVector::ZeroVector, 100.f),
        TEXT("a large visible structure edge survives smoke hiding its centre while a contained target stays hidden"));
    Check(Grid.IsSmokeConcealed(0, FVector(-2000.0, 0.0, 0.0), FVector(1500.0, 0.0, 0.0))
        && !Grid.IsVisible(0, FVector(1500.0, 0.0, 0.0))
        && Grid.IsVisible(0, FVector(0.0, 1600.0, 0.0)),
        TEXT("smoke blocks intersecting sight lines but leaves a clear adjacent sight line"));
    Check(!Grid.IsSmokeConcealed(0, FVector(-400.0, 0.0, 0.0), FVector::ZeroVector)
        && Grid.IsSmokeConcealed(0, FVector(-400.1, 0.0, 0.0), FVector::ZeroVector)
        && !Grid.IsSmokeConcealed(1, FVector(-2000.0, 0.0, 0.0), FVector::ZeroVector),
        TEXT("400 cm proximity overrides smoke exactly at the boundary and its owner sees through"));
    Sources.Add(Source(0, -200.0, 0.0, 500.f));
    Grid.RebuildVision(Sources);
    Check(Grid.IsVisible(0, FVector::ZeroVector),
        TEXT("a second close observer reveals a target hidden from the distant scout"));
    Sources.RemoveAt(Sources.Num() - 1);
    Grid.RebuildVision(Sources);
    Check(Grid.Advance(0.f) && Grid.GetSmokeVolumes().Num() == 1
        && Grid.GetSmokeVolumes()[0].RemainingSeconds == 6.0
        && !Grid.IsVisible(0, FVector::ZeroVector),
        TEXT("zero simulation delta freezes smoke and visibility for pause or match end"));
    Check(!Grid.Advance(-1.f) && !Grid.Advance(NaN) && !Grid.Advance(Infinity)
        && Grid.GetSimulationSeconds() == 0.0 && Grid.GetSmokeVolumes()[0].RemainingSeconds == 6.0,
        TEXT("invalid deltas cannot advance the smoke clock"));
    Check(Grid.Advance(5.5f) && Grid.GetSmokeVolumes().Num() == 1
        && !Grid.IsVisible(0, FVector::ZeroVector),
        TEXT("smoke continues to conceal until its full authored lifetime"));
    Check(Grid.Advance(.5f) && Grid.GetSmokeVolumes().IsEmpty()
        && Grid.IsVisible(0, FVector::ZeroVector) && Grid.GetSimulationSeconds() == 6.0,
        TEXT("expiry rebuilds visibility at six simulation seconds without stale hidden cells"));
    const int32 ClampedSmoke = Grid.AddSmoke(1, FVector::ZeroVector, 900.f, 20.f);
    Check(ClampedSmoke != INDEX_NONE && ClampedSmoke != SmokeId
        && Grid.GetSmokeVolumes()[0].Radius == 600.f
        && Grid.GetSmokeVolumes()[0].RemainingSeconds == 6.0
        && Grid.RemoveSmoke(ClampedSmoke) && !Grid.RemoveSmoke(ClampedSmoke)
        && Grid.IsVisible(0, FVector::ZeroVector),
        TEXT("smoke bounds are enforced and removing its stable ID immediately restores sight"));
    Check(Grid.AddSmoke(9, FVector::ZeroVector) == INDEX_NONE
        && Grid.AddSmoke(0, FVector(NaN, 0.0, 0.0)) == INDEX_NONE
        && Grid.AddSmoke(0, FVector::ZeroVector, -1.f) == INDEX_NONE
        && Grid.AddSmoke(0, FVector::ZeroVector, 600.f, Infinity) == INDEX_NONE,
        TEXT("malformed smoke requests cannot create areas"));
    Grid.AddSmoke(1, FVector::ZeroVector);
    Grid.ClearSmoke();
    Check(Grid.GetSmokeVolumes().IsEmpty() && Grid.IsVisible(0, FVector::ZeroVector),
        TEXT("clearing smoke preserves sight sources and refreshes the mask"));
    Grid.GetFogPixels(99, Pixels);
    Check(Pixels.Num() == 2700 && Pixels[0].A == 235 && Pixels.Last().A == 235,
        TEXT("an invalid team's mask returns unknown terrain rather than another team's knowledge"));
    Check(Grid.Reset(FVector2D(-500.0, -500.0), FVector2D(500.0, 500.0), 400.f, 2)
        && Grid.GetWidth() == 3 && Grid.GetHeight() == 3
        && Grid.GetCellCenter(2, 2).Equals(FVector(400.0, 400.0, 0.0))
        && Grid.GetSimulationSeconds() == 0.0 && !Grid.IsExplored(0, FVector::ZeroVector),
        TEXT("reset clears match knowledge and partial edge cells have centres within their bounds"));
    UE_LOG(LogTemp, Display, TEXT("VISIBILITY_TEST_COUNT=%d"), Count);
    UE_LOG(LogTemp, Display, TEXT("VISIBILITY_TEST_RESULT=%s"), Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}
