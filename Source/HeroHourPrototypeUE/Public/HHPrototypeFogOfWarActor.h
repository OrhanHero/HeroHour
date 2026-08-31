#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HHPrototypeFogOfWarActor.generated.h"

UENUM(BlueprintType)
enum class EHHFogVisibility : uint8
{
    Unseen,
    Explored,
    Visible
};

/** Lightweight, server/local-prototype fog grid. It owns no textures and is safe to query from Canvas HUDs. */
UCLASS(Blueprintable)
class HEROHOURPROTOTYPEUE_API AHHPrototypeFogOfWarActor : public AActor
{
    GENERATED_BODY()

public:
    AHHPrototypeFogOfWarActor();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** Rebuilds the grid and clears exploration. Call after changing bounds or resolution at runtime. */
    UFUNCTION(BlueprintCallable, Category="HeroHour|Fog of War")
    void InitializeGrid();

    /** Performs one visibility pass immediately. Normally driven by UpdateInterval. */
    UFUNCTION(BlueprintCallable, Category="HeroHour|Fog of War")
    void RefreshVisibility();

    UFUNCTION(BlueprintPure, Category="HeroHour|Fog of War")
    EHHFogVisibility GetVisibilityAtWorldLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category="HeroHour|Fog of War")
    bool IsWorldLocationVisible(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintPure, Category="HeroHour|Fog of War")
    bool IsWorldLocationExplored(const FVector& WorldLocation) const;

    /** Converts world XY to normalized radar UV. Returns false outside the configured battlefield. */
    UFUNCTION(BlueprintPure, Category="HeroHour|Radar")
    bool WorldToRadarUV(const FVector& WorldLocation, FVector2D& OutRadarUV) const;

    UFUNCTION(BlueprintPure, Category="HeroHour|Radar")
    FVector RadarUVToWorld(const FVector2D& RadarUV, float WorldZ = 0.0f) const;

    UFUNCTION(BlueprintPure, Category="HeroHour|Fog of War")
    FIntPoint GetGridSize() const { return GridSize; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Fog of War")
    const TArray<uint8>& GetVisibilityCells() const { return VisibilityCells; }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Fog of War", meta=(ClampMin="4", ClampMax="256"))
    FIntPoint GridSize = FIntPoint(64, 64);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Fog of War")
    FVector2D BattlefieldCenter = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Fog of War", meta=(ClampMin="100.0"))
    FVector2D BattlefieldExtent = FVector2D(4200.0f, 3000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Fog of War", meta=(ClampMin="0.05"))
    float UpdateInterval = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Fog of War", meta=(ClampMin="50.0"))
    float UnitSightRadius = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Fog of War", meta=(ClampMin="50.0"))
    float HeadquartersSightRadius = 1250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Fog of War|Debug")
    bool bDrawDebugGrid = false;

private:
    int32 WorldToCellIndex(const FVector& WorldLocation) const;
    FVector CellCenterToWorld(int32 X, int32 Y) const;
    void RevealCircle(const FVector& WorldLocation, float Radius);
    void DrawGridDebug() const;

    UPROPERTY(Transient)
    TArray<uint8> VisibilityCells;

    float TimeUntilRefresh = 0.0f;
};
