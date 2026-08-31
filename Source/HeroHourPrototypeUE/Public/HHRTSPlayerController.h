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

private:
    void SelectUnderCursor();
    void IssueMoveOrder();
    void SetSelectedActor(AActor* NewSelection);

    UPROPERTY(Transient)
    TWeakObjectPtr<AActor> SelectedActor;

};
