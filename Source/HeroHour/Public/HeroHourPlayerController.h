// HeroHourPlayerController.h
// RTS Camera and Input Controller

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HeroHourPlayerController.generated.h"

class AHeroHourUnit;
class AHeroHourBuilding;
class UHeroHourHUDWidget;
class UHeroHourCommandMenuWidget;
class UHeroHourMinimapWidget;

UENUM(BlueprintType, Category = "HeroHour|Camera")
enum class EHeroHourCameraMode : uint8
{
    Normal,
    Following,
    Cinematic,
    Free,
    Tactical
};

UCLASS(Blueprintable)
class HEROHOUR_API AHeroHourPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AHeroHourPlayerController();

    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaTime) override;
    virtual void PlayerTick(float DeltaTime) override;

    // Input handling
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Input")
    void HandleCommand(const FString& CommandData);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Selection")
    void SelectUnits(const TArray<AActor*>& Units);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Selection")
    void AddToSelection(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Selection")
    void RemoveFromSelection(AActor* Unit);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Selection")
    void ClearSelection();

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Selection")
    TArray<AActor*> GetSelectedUnits() const;

    // Camera control
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Camera")
    void MoveCamera(FVector2D Direction);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Camera")
    void ZoomCamera(float ZoomAmount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Camera")
    void RotateCamera(float RotationAmount);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Camera")
    void CenterCameraOnSelection();

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Camera")
    void SetCameraMode(EHeroHourCameraMode Mode);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|Camera")
    void SetCameraTarget(AActor* Target);

    // Rally points
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Building")
    void SetRallyPoint(FVector Location);

    // UI
    UFUNCTION(BlueprintCallable, Category = "HeroHour|UI")
    void ShowCommandMenu(UClass* MenuClass);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|UI")
    void HideCommandMenu();

    UFUNCTION(BlueprintCallable, Category = "HeroHour|UI")
    void UpdateResourceDisplay(int32 Credits, int32 Energy, int32 TechPoints);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|UI")
    void UpdatePopulationDisplay(int32 Current, int32 Max);

    // Ability usage
    UFUNCTION(BlueprintCallable, Category = "HeroHour|Ability")
    void UseAbility(int32 UnitID, FString AbilityID, FVector TargetLocation);

    // Control groups
    UFUNCTION(BlueprintCallable, Category = "HeroHour|ControlGroups")
    void SetControlGroup(int32 GroupIndex);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|ControlGroups")
    void RecallControlGroup(int32 GroupIndex);

    UFUNCTION(BlueprintCallable, Category = "HeroHour|ControlGroups")
    void AddToControlGroup(int32 GroupIndex);

protected:
    // Input actions
    void OnLeftClickPressed();
    void OnLeftClickReleased();
    void OnRightClickPressed();
    void OnRightClickReleased();
    void OnMiddleClickPressed();
    void OnMiddleClickReleased();
    void OnMouseWheel(float Value);
    void OnMoveForward(float Value);
    void OnMoveRight(float Value);
    void OnZoom(float Value);
    void OnRotate(float Value);
    void OnSelectAll();
    void OnDeselectAll();
    void OnControlGroup(int32 GroupIndex);
    void OnRecallControlGroup(int32 GroupIndex);
    void OnAbilityKey(int32 AbilityIndex);
    void OnPause();

    // Camera
    void UpdateCameraPosition();
    void HandleEdgeScrolling();
    void SmoothMoveCamera(FVector TargetLocation);
    void SmoothZoomCamera(float TargetZoom);
    void ClampCameraToMapBounds();

    // Selection
    void UpdateSelectionBox();
    void PerformBoxSelection();
    void PerformClickSelection();
    bool IsActorSelectable(AActor* Actor) const;

    // UI Widgets
    UPROPERTY()
    UHeroHourHUDWidget* HUDWidget;

    UPROPERTY()
    UHeroHourCommandMenuWidget* CommandMenuWidget;

    UPROPERTY()
    UHeroHourMinimapWidget* MinimapWidget;

    // Selection state
    UPROPERTY()
    TArray<AActor*> SelectedUnits;

    UPROPERTY()
    FVector2D SelectionStart;

    UPROPERTY()
    FVector2D SelectionEnd;

    UPROPERTY()
    bool bIsBoxSelecting = false;

    // Camera settings
    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|Camera")
    float CameraMoveSpeed = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|Camera")
    float CameraZoomSpeed = 500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|Camera")
    float CameraRotationSpeed = 90.0f;

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|Camera")
    FVector2D CameraZoomLimits = FVector2D(500.0f, 5000.0f);

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|Camera")
    float EdgeScrollMargin = 20.0f;

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|Camera")
    float EdgeScrollSpeed = 1500.0f;

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|Camera")
    float CameraSmoothSpeed = 10.0f;

    // Camera state
    UPROPERTY()
    EHeroHourCameraMode CurrentCameraMode = EHeroHourCameraMode::Normal;

    UPROPERTY()
    AActor* CameraTarget = nullptr;

    UPROPERTY()
    FVector TargetCameraLocation;

    UPROPERTY()
    float TargetCameraZoom = 2000.0f;

    // F# Bridge
    UPROPERTY()
    class UHeroHourFSharpBridge* FSharpBridge;

    // HUD Widget Class
    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|UI")
    TSubclassOf<UHeroHourHUDWidget> HUDWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|UI")
    TSubclassOf<UHeroHourCommandMenuWidget> CommandMenuWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "HeroHour|UI")
    TSubclassOf<UHeroHourMinimapWidget> MinimapWidgetClass;
};