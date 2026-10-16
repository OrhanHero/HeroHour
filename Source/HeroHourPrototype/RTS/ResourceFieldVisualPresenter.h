#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "ResourceFieldVisualPresenter.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

// Presentation snapshot only: no resource-field, wallet, reservation or collector references.
struct FResourceFieldVisualSample
{
    int32 CellId=INDEX_NONE;
    FVector Position=FVector::ZeroVector;
    float Activity=0.f;
    bool bCurrentlyVisible=false;
    bool bExplored=false;
    bool bFiniteDepleted=false;
};

// The same fog memory policy used by the renderer; independent of world/timers/resources.
class HEROHOURPROTOTYPE_API FResourceFieldVisualMemory
{
public:
    TArray<FResourceFieldVisualSample> Observe(const TArray<FResourceFieldVisualSample>& Samples);
    void Reset() { LastSeen.Reset(); }
private:
    TMap<int32,FResourceFieldVisualSample> LastSeen;
};

// Pairs index the supplied display array, using spatial neighbours rather than cell IDs.
HEROHOURPROTOTYPE_API TArray<FIntPoint> BuildResourceFieldVisualEdges(const TArray<FResourceFieldVisualSample>& Display);
HEROHOURPROTOTYPE_API bool RunHRResourceFieldVisualSelfTests(UWorld* TestWorld);

UCLASS()
class HEROHOURPROTOTYPE_API AResourceFieldVisualPresenter : public AActor
{
    GENERATED_BODY()
public:
    AResourceFieldVisualPresenter();
    // Caller owns all simulation. Meshes are original organic segments/pods, not game extracts.
    void Configure(UStaticMesh* RootMesh,UStaticMesh* PodMesh,UMaterialInterface* RootMaterial,UMaterialInterface* PodMaterial);
    void SubmitSnapshot(const TArray<FResourceFieldVisualSample>& Samples);
    // Reset when reusing a presenter for a different field/team; cached fog is team-local.
    void ResetPresentation();
    bool HasUsableAssets() const;
    int32 GetEdgeCount() const { return EdgeCount; }
    int32 GetPodCount() const;
    int32 GetRootSegmentCount() const;
    static constexpr int32 MaxCells=16;
    static constexpr int32 MaxEdges=24;
    static constexpr int32 MaxPods=64;
    static constexpr int32 SegmentsPerEdge=3;
private:
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Roots;
    UPROPERTY() TObjectPtr<UInstancedStaticMeshComponent> Pods;
    TArray<FResourceFieldVisualSample> Pending;
    FResourceFieldVisualMemory Memory;
    uint32 LastSignature=0;
    bool bHasSignature=false;
    bool bConfiguredAssets=false;
    FTimerHandle PresentationTimer;
    int32 EdgeCount=0;
    void PresentPending();
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
