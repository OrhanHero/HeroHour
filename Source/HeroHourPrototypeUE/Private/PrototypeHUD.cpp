#include "PrototypeHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "HHPrototypeCombatActor.h"
#include "HHPrototypeGameMode.h"
#include "HHRTSPlayerController.h"

void APrototypeHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !GEngine)
    {
        return;
    }

    UFont* Font = GEngine->GetSmallFont();
    DrawRect(FLinearColor(0.015f, 0.025f, 0.04f, 0.88f), 18.0f, 18.0f, 440.0f, 118.0f);
    DrawText(TEXT("HERO HOUR - PROTOTYPE"), FLinearColor(0.15f, 0.85f, 1.0f), 34.0f, 30.0f, Font, 1.25f);
    DrawText(TEXT("WASD: Camera   Wheel: Zoom"), FLinearColor::White, 34.0f, 61.0f, Font, 1.0f);
    DrawText(TEXT("Left: Select   Right: Move / Attack   R: Restart"), FLinearColor::White, 34.0f, 83.0f, Font, 1.0f);

    FString SelectionText = TEXT("Selected: none");
    if (const AHHRTSPlayerController* Controller = Cast<AHHRTSPlayerController>(GetOwningPlayerController()))
    {
        if (const AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Controller->GetSelectedActor()))
        {
            const TCHAR* RoleName = Unit->GetRole() == EHHPrototypeUnitRole::Infantry ? TEXT("Infantry")
                : Unit->GetRole() == EHHPrototypeUnitRole::Vehicle ? TEXT("Vehicle") : TEXT("Air");
            SelectionText = FString::Printf(TEXT("Selected: HERO %s"), RoleName);
        }
    }
    DrawText(SelectionText, FLinearColor(0.95f, 0.85f, 0.25f), 34.0f, 106.0f, Font, 1.0f);

    FString MatchStatus = TEXT("Combat active");
    if (const AHHPrototypeGameMode* GameMode = GetWorld()->GetAuthGameMode<AHHPrototypeGameMode>())
    {
        if (!GameMode->HasUSAStarted())
        {
            MatchStatus = FString::Printf(TEXT("USA attacks in %.0f seconds"), GameMode->GetUSAOrderTimeRemaining());
        }
    }
    DrawText(MatchStatus, FLinearColor(1.0f, 0.55f, 0.2f), 480.0f, 62.0f, Font, 1.1f);

    const FString Objective = TEXT("OBJECTIVE: Destroy the USA headquarters");
    float Width = 0.0f;
    float Height = 0.0f;
    GetTextSize(Objective, Width, Height, Font, 1.0f);
    DrawText(Objective, FLinearColor(1.0f, 0.82f, 0.18f), (Canvas->ClipX - Width) * 0.5f, 24.0f, Font, 1.0f);
}
