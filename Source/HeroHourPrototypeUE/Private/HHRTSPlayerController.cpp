#include "HHRTSPlayerController.h"

#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "InputCoreTypes.h"
#include "HHPrototypeCombatActor.h"

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
        SelectUnderCursor();
    }
    if (WasInputKeyJustPressed(EKeys::RightMouseButton))
    {
        IssueMoveOrder();
    }
    if (WasInputKeyJustPressed(EKeys::R))
    {
        ConsoleCommand(TEXT("RestartLevel"));
    }

}

void AHHRTSPlayerController::SelectUnderCursor()
{
    FHitResult Hit;
    AActor* Candidate = nullptr;
    if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        AHHPrototypeCombatActor* Combatant = Cast<AHHPrototypeCombatActor>(Hit.GetActor());
        if (Combatant && Combatant->GetFaction() == EHHPrototypeFaction::Hero && !Combatant->IsHeadquarters())
        {
            Candidate = Combatant;
        }
    }
    SetSelectedActor(Candidate);
}

void AHHRTSPlayerController::IssueMoveOrder()
{
    if (!SelectedActor.IsValid())
    {
        return;
    }

    FHitResult Hit;
    if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
    {
        if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(SelectedActor.Get()))
        {
            if (AHHPrototypeCombatActor* Target = Cast<AHHPrototypeCombatActor>(Hit.GetActor());
                Target && Target->GetFaction() != Unit->GetFaction())
            {
                Unit->SetAttackTarget(Target);
            }
            else
            {
                Unit->SetMoveTarget(Hit.Location);
                UE_LOG(LogTemp, Display, TEXT("HERO move order: %s -> %s"), *Unit->GetName(), *Hit.Location.ToCompactString());
            }
        }
    }
}

void AHHRTSPlayerController::SetSelectedActor(AActor* NewSelection)
{
    if (SelectedActor.IsValid())
    {
        if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(SelectedActor.Get()))
        {
            Unit->SetSelected(false);
        }
    }

    SelectedActor = NewSelection;
    if (SelectedActor.IsValid())
    {
        if (AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(SelectedActor.Get()))
        {
            Unit->SetSelected(true);
            UE_LOG(LogTemp, Display, TEXT("HERO selected: %s"), *Unit->GetName());
        }
    }
}
