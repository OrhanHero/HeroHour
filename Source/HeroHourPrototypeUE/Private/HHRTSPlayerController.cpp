#include "HHRTSPlayerController.h"

#include "Components/PrimitiveComponent.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/Pawn.h"
#include "InputCoreTypes.h"
#include "HHPrototypeCombatActor.h"
#include "HHPrototypeGameMode.h"

AHHRTSPlayerController::AHHRTSPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    DefaultMouseCursor = EMouseCursor::Default;
}

void AHHRTSPlayerController::BeginPlay()
{
    Super::BeginPlay();
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
}

void AHHRTSPlayerController::PlayerTick(const float DeltaSeconds)
{
    Super::PlayerTick(DeltaSeconds);

    if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
    {
        BeginSelection();
    }
    if (WasInputKeyJustReleased(EKeys::LeftMouseButton))
    {
        FinishSelection();
    }
    if (WasInputKeyJustPressed(EKeys::RightMouseButton))
    {
        bAttackMoveArmed = false;
        IssueContextOrder();
    }
    if (WasInputKeyJustPressed(EKeys::R))
    {
        ConsoleCommand(TEXT("RestartLevel"));
    }
    if (WasInputKeyJustPressed(EKeys::A)) bAttackMoveArmed = true;
    if (WasInputKeyJustPressed(EKeys::S)) StopSelectedUnits();
    if (AHHPrototypeGameMode* GameMode = GetWorld()->GetAuthGameMode<AHHPrototypeGameMode>())
    {
        if (WasInputKeyJustPressed(EKeys::F1)) GameMode->RequestHeroProduction(EHHPrototypeUnitRole::Infantry);
        if (WasInputKeyJustPressed(EKeys::F2)) GameMode->RequestHeroProduction(EHHPrototypeUnitRole::Vehicle);
        if (WasInputKeyJustPressed(EKeys::F3)) GameMode->RequestHeroProduction(EHHPrototypeUnitRole::Air);
        if (WasInputKeyJustPressed(EKeys::C)) GameMode->CancelHeroProduction();
    }

    const bool bControlDown = IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl);
    const FKey GroupKeys[3] = { EKeys::One, EKeys::Two, EKeys::Three };
    for (int32 GroupIndex = 0; GroupIndex < 3; ++GroupIndex)
    {
        if (WasInputKeyJustPressed(GroupKeys[GroupIndex]))
        {
            if (bControlDown) StoreControlGroup(GroupIndex);
            else RecallControlGroup(GroupIndex);
        }
    }
    if (WasInputKeyJustPressed(EKeys::Escape))
    {
        bAttackMoveArmed = false;
        ClearSelection();
    }

}

TArray<AActor*> AHHRTSPlayerController::GetSelectedActors() const
{
    TArray<AActor*> Result;
    for (const TWeakObjectPtr<AActor>& Actor : SelectedActors)
    {
        if (Actor.IsValid()) Result.Add(Actor.Get());
    }
    return Result;
}

void AHHRTSPlayerController::BeginSelection()
{
    bSelectionInProgress = GetMousePosition(SelectionStart.X, SelectionStart.Y);
}

void AHHRTSPlayerController::FinishSelection()
{
    if (!bSelectionInProgress) return;
    bSelectionInProgress = false;
    FVector2D End;
    if (!GetMousePosition(End.X, End.Y)) return;

    if (bAttackMoveArmed)
    {
        bAttackMoveArmed = false;
        IssueAttackMoveOrder();
        return;
    }

    const bool bShift = IsInputKeyDown(EKeys::LeftShift) || IsInputKeyDown(EKeys::RightShift);
    if (FVector2D::Distance(SelectionStart, End) >= 8.0f)
    {
        SelectInScreenBox(SelectionStart, End, bShift);
    }
    else
    {
        SelectUnderCursor(bShift);
    }
}

bool AHHRTSPlayerController::IsSelectableHeroUnit(const AActor* Actor) const
{
    const AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Actor);
    return Unit && Unit->IsAlive() && !Unit->IsHeadquarters()
        && Unit->GetFaction() == EHHPrototypeFaction::Hero;
}

void AHHRTSPlayerController::SelectUnderCursor(const bool bToggleSelection)
{
    FHitResult Hit;
    AActor* Candidate = nullptr;
    if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        AHHPrototypeCombatActor* Combatant = Cast<AHHPrototypeCombatActor>(Hit.GetActor());
        if (IsSelectableHeroUnit(Combatant))
        {
            Candidate = Combatant;
        }
    }
    if (!bToggleSelection)
    {
        ClearSelection();
        if (Candidate) AddSelectedActor(Candidate);
    }
    else if (Candidate)
    {
        if (SelectedActors.Contains(Candidate)) RemoveSelectedActor(Candidate);
        else AddSelectedActor(Candidate);
    }
}

void AHHRTSPlayerController::SelectInScreenBox(const FVector2D& BoxStart, const FVector2D& BoxEnd, const bool bAddToSelection)
{
    if (!bAddToSelection) ClearSelection();
    const FVector2D Min(FMath::Min(BoxStart.X, BoxEnd.X), FMath::Min(BoxStart.Y, BoxEnd.Y));
    const FVector2D Max(FMath::Max(BoxStart.X, BoxEnd.X), FMath::Max(BoxStart.Y, BoxEnd.Y));
    for (TActorIterator<AHHPrototypeCombatActor> It(GetWorld()); It; ++It)
    {
        FVector2D Screen;
        if (IsSelectableHeroUnit(*It) && ProjectWorldLocationToScreen(It->GetActorLocation(), Screen)
            && Screen.X >= Min.X && Screen.X <= Max.X && Screen.Y >= Min.Y && Screen.Y <= Max.Y)
        {
            AddSelectedActor(*It);
        }
    }
}

void AHHRTSPlayerController::IssueContextOrder()
{
    if (SelectedActors.IsEmpty()) return;
    FHitResult Hit;
    if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        AHHPrototypeCombatActor* Target = Cast<AHHPrototypeCombatActor>(Hit.GetActor());
        int32 UnitIndex = 0;
        for (const TWeakObjectPtr<AActor>& Actor : SelectedActors)
        {
            if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Actor.Get()))
            {
                if (Target && Target->IsAlive() && Target->GetFaction() != Unit->GetFaction())
                    Unit->SetAttackTarget(Target);
                else
                    Unit->SetMoveTarget(Hit.Location + GetFormationOffset(UnitIndex++, SelectedActors.Num()));
            }
        }
        DrawOrderMarker(Hit.Location, Target ? FColor::Red : FColor::Cyan);
    }
}

void AHHRTSPlayerController::IssueAttackMoveOrder()
{
    FHitResult Hit;
    if (!GetHitResultUnderCursor(ECC_Visibility, false, Hit)) return;
    AHHPrototypeCombatActor* Target = Cast<AHHPrototypeCombatActor>(Hit.GetActor());
    int32 Index = 0;
    for (const TWeakObjectPtr<AActor>& Actor : SelectedActors)
    {
        if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Actor.Get()))
        {
            if (Target && Target->GetFaction() != Unit->GetFaction()) Unit->SetAttackTarget(Target);
            else Unit->SetMoveTarget(Hit.Location + GetFormationOffset(Index++, SelectedActors.Num()));
        }
    }
    DrawOrderMarker(Hit.Location, FColor::Orange);
}

void AHHRTSPlayerController::StopSelectedUnits()
{
    for (const TWeakObjectPtr<AActor>& Actor : SelectedActors)
        if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Actor.Get()))
            Unit->SetMoveTarget(Unit->GetActorLocation());
}

void AHHRTSPlayerController::StoreControlGroup(const int32 GroupIndex)
{
    ControlGroups[GroupIndex] = SelectedActors;
}

void AHHRTSPlayerController::RecallControlGroup(const int32 GroupIndex)
{
    ClearSelection();
    for (const TWeakObjectPtr<AActor>& Actor : ControlGroups[GroupIndex])
        if (IsSelectableHeroUnit(Actor.Get())) AddSelectedActor(Actor.Get());
}

FVector AHHRTSPlayerController::GetFormationOffset(const int32 UnitIndex, const int32 UnitCount) const
{
    if (UnitCount <= 1) return FVector::ZeroVector;
    constexpr float Spacing = 135.0f;
    const int32 Columns = FMath::CeilToInt(FMath::Sqrt(static_cast<float>(UnitCount)));
    const int32 Row = UnitIndex / Columns;
    const int32 Column = UnitIndex % Columns;
    return FVector((Row - (Columns - 1) * 0.5f) * Spacing, (Column - (Columns - 1) * 0.5f) * Spacing, 0.0f);
}

void AHHRTSPlayerController::DrawOrderMarker(const FVector& Location, const FColor& Color) const
{
    DrawDebugCircle(GetWorld(), Location + FVector(0, 0, 8), 52.0f, 24, Color, false, 0.85f, 0, 4.0f,
        FVector(1, 0, 0), FVector(0, 1, 0), false);
}

void AHHRTSPlayerController::ClearSelection()
{
    for (const TWeakObjectPtr<AActor>& Actor : SelectedActors)
        if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Actor.Get())) Unit->SetSelected(false);
    SelectedActors.Reset();
    SelectedActor.Reset();
}

void AHHRTSPlayerController::AddSelectedActor(AActor* NewSelection)
{
    if (!IsSelectableHeroUnit(NewSelection) || SelectedActors.Contains(NewSelection)) return;
    SelectedActors.Add(NewSelection);
    CastChecked<AHHPrototypeCombatActor>(NewSelection)->SetSelected(true);
    RefreshPrimarySelection();
}

void AHHRTSPlayerController::RemoveSelectedActor(AActor* Selection)
{
    if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Selection)) Unit->SetSelected(false);
    SelectedActors.Remove(Selection);
    RefreshPrimarySelection();
}

void AHHRTSPlayerController::RefreshPrimarySelection()
{
    SelectedActors.RemoveAll([](const TWeakObjectPtr<AActor>& Actor) { return !Actor.IsValid(); });
    SelectedActor = SelectedActors.IsEmpty() ? nullptr : SelectedActors[0];
}

void AHHRTSPlayerController::SetSelectedActor(AActor* NewSelection)
{
    ClearSelection();
    AddSelectedActor(NewSelection);
}
