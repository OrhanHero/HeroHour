#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HHRTSCameraPawn.generated.h"

class UCameraComponent;
class USceneComponent;
class USpringArmComponent;

/** Isometric RTS camera used by the vertical slice. */
UCLASS()
class HEROHOURPROTOTYPEUE_API AHHRTSCameraPawn : public APawn
{
    GENERATED_BODY()

public:
    AHHRTSCameraPawn();
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "RTS Camera")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "RTS Camera")
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY(VisibleAnywhere, Category = "RTS Camera")
    TObjectPtr<UCameraComponent> Camera;

    UPROPERTY(EditAnywhere, Category = "RTS Camera|Movement")
    float MoveSpeed = 8000.0f;

    UPROPERTY(EditAnywhere, Category = "RTS Camera|Movement")
    bool bEnableEdgeScroll = false;

    UPROPERTY(EditAnywhere, Category = "RTS Camera|Movement", meta = (ClampMin = "1.0", ClampMax = "100.0"))
    float EdgeScrollPixels = 16.0f;

    UPROPERTY(EditAnywhere, Category = "RTS Camera|Zoom")
    float ZoomStep = 1500.0f;

    UPROPERTY(EditAnywhere, Category = "RTS Camera|Zoom")
    float MinZoom = 8000.0f;

    UPROPERTY(EditAnywhere, Category = "RTS Camera|Zoom")
    float MaxZoom = 32000.0f;

    UPROPERTY(EditAnywhere, Category = "RTS Camera|Movement")
    FVector2D CameraBounds = FVector2D(60000.0f, 60000.0f);
};
