#include "HHRTSCameraPawn.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputCoreTypes.h"

AHHRTSCameraPawn::AHHRTSCameraPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(SceneRoot);
    SpringArm->SetUsingAbsoluteRotation(true);
    // CitySample is authored at real-world city scale. A higher, angled camera
    // reveals complete blocks and the skyline instead of a single road surface.
    SpringArm->SetRelativeRotation(FRotator(-58.0f, -45.0f, 0.0f));
    SpringArm->TargetArmLength = 18000.0f;
    SpringArm->bDoCollisionTest = false;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->FieldOfView = 50.0f;
}

void AHHRTSCameraPawn::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    APlayerController* PC = Cast<APlayerController>(GetController());
    if (!PC)
    {
        return;
    }

    FVector2D Direction = FVector2D::ZeroVector;
    Direction.Y += PC->IsInputKeyDown(EKeys::W) ? 1.0f : 0.0f;
    Direction.Y -= PC->IsInputKeyDown(EKeys::S) ? 1.0f : 0.0f;
    Direction.X += PC->IsInputKeyDown(EKeys::D) ? 1.0f : 0.0f;
    Direction.X -= PC->IsInputKeyDown(EKeys::A) ? 1.0f : 0.0f;

    if (bEnableEdgeScroll)
    {
        float MouseX = 0.0f;
        float MouseY = 0.0f;
        int32 ViewportX = 0;
        int32 ViewportY = 0;
        PC->GetViewportSize(ViewportX, ViewportY);
        if (PC->GetMousePosition(MouseX, MouseY) && ViewportX > 0 && ViewportY > 0)
        {
            Direction.X += MouseX <= EdgeScrollPixels ? -1.0f : 0.0f;
            Direction.X += MouseX >= ViewportX - EdgeScrollPixels ? 1.0f : 0.0f;
            Direction.Y += MouseY <= EdgeScrollPixels ? 1.0f : 0.0f;
            Direction.Y += MouseY >= ViewportY - EdgeScrollPixels ? -1.0f : 0.0f;
        }
    }

    if (!Direction.IsNearlyZero())
    {
        Direction.Normalize();
        AddActorWorldOffset(FVector(Direction.Y, Direction.X, 0.0f) * MoveSpeed * DeltaSeconds, true);
        FVector Location = GetActorLocation();
        Location.X = FMath::Clamp(Location.X, -CameraBounds.X, CameraBounds.X);
        Location.Y = FMath::Clamp(Location.Y, -CameraBounds.Y, CameraBounds.Y);
        SetActorLocation(Location);
    }

    const float Wheel = PC->GetInputAnalogKeyState(EKeys::MouseWheelAxis);
    if (!FMath::IsNearlyZero(Wheel))
    {
        SpringArm->TargetArmLength = FMath::Clamp(
            SpringArm->TargetArmLength - Wheel * ZoomStep,
            MinZoom,
            MaxZoom);
    }

    const float RotationInput = (PC->IsInputKeyDown(EKeys::E) ? 1.0f : 0.0f)
        - (PC->IsInputKeyDown(EKeys::Q) ? 1.0f : 0.0f);
    if (!FMath::IsNearlyZero(RotationInput))
    {
        FRotator Rotation = SpringArm->GetRelativeRotation();
        Rotation.Yaw += RotationInput * 55.0f * DeltaSeconds;
        SpringArm->SetRelativeRotation(Rotation);
    }
}
