#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PrototypeCollectorVisual.generated.h"

class APrototypeUnit;
class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

/** Original articulated harvester presentation. Never writes simulation state. */
UCLASS()
class HEROHOURPROTOTYPE_API UPrototypeCollectorVisualComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPrototypeCollectorVisualComponent();
    // Call after ConfigureRole finishes choosing and colouring the original hull.
    // Missing/partial animation assets retain that complete original hull.
    static void SyncRole(APrototypeUnit* Unit, UStaticMeshComponent* Hull);
    // Call immediately after TickCollector, inside the actor's existing guards.
    static void TickVisual(APrototypeUnit* Unit, float DeltaSeconds);
    bool HasArticulatedAssets() const { return bReady; }
    virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
private:
    bool Initialize(UStaticMeshComponent* Hull);
    void Advance(float DeltaSeconds);
    void RemoveParts();
    UPROPERTY() TObjectPtr<UStaticMeshComponent> HullComponent;
    UPROPERTY() TObjectPtr<UStaticMesh> OriginalHull;
    UPROPERTY() TObjectPtr<UStaticMesh> BaseMesh;
    UPROPERTY() TArray<TObjectPtr<UMaterialInterface>> OriginalMaterials;
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;
    float ArmAngle = 10.f;
    float CouplerExtension = 0.f;
    float DisplayCargo = 0.f;
    float DrumAngle = 0.f;
    bool bReady = false;
};
