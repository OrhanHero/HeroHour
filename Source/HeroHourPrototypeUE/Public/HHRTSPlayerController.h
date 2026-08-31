#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HHRTSPlayerController.generated.h"

/** Mouse selection and simple prototype move orders. */
UCLASS()
class HEROHOURPROTOTYPEUE_API AHHRTSPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHHRTSPlayerController();
    virtual void BeginPlay() override;
    virtual void PlayerTick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category = "RTS")
    AActor* GetSelectedActor() const { return SelectedActor.Get(); }

    UFUNCTION(BlueprintPure, Category = "RTS")
    TArray<AActor*> GetSelectedActors() const;

private:
    void BeginSelection();
    void FinishSelection();
    void SelectUnderCursor(bool bToggleSelection);
    void SelectInScreenBox(const FVector2D& BoxStart, const FVector2D& BoxEnd, bool bAddToSelection);
    void IssueContextOrder();
    void IssueAttackMoveOrder();
    void StopSelectedUnits();
    void StoreControlGroup(int32 GroupIndex);
    void RecallControlGroup(int32 GroupIndex);
    void AddSelectedActor(AActor* NewSelection);
    void RemoveSelectedActor(AActor* Selection);
    void ClearSelection();
    void RefreshPrimarySelection();
    void SetSelectedActor(AActor* NewSelection);
    bool IsSelectableHeroUnit(const AActor* Actor) const;
    FVector GetFormationOffset(int32 UnitIndex, int32 UnitCount) const;
    void DrawOrderMarker(const FVector& Location, const FColor& Color) const;

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> SelectedActor;

    UPROPERTY(Transient)
    TArray<TWeakObjectPtr<AActor>> SelectedActors;

    TArray<TWeakObjectPtr<AActor>> ControlGroups[3];
    FVector2D SelectionStart = FVector2D::ZeroVector;
    bool bSelectionInProgress = false;
    bool bAttackMoveArmed = false;

};
