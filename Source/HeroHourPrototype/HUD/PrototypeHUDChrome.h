#pragma once

#include "CoreMinimal.h"
#include "../PrototypeHUDLayout.h"

class AHUD;
class UCanvas;

// Original vector chrome. Coordinates use the same reference space as the HUD
// layout; only the helper applies the viewport scale. No game art is sampled.
namespace HeroHUDChrome
{
    HEROHOURPROTOTYPE_API void DrawSteelFrame(AHUD* HUD, const FPrototypeHUDRect& Rect,
        float Scale, FLinearColor Accent, bool bHeavy = false, bool bFill = true);

    HEROHOURPROTOTYPE_API void DrawTabFill(AHUD* HUD, const FPrototypeHUDRect& Rect,
        float Scale, FLinearColor Accent, bool bSelected, bool bEnabled = true);

    // UCanvas is explicit because AHUD::Canvas is protected. Call from DrawHUD
    // with this HUD's Canvas. The low-level transform is always popped here.
    HEROHOURPROTOTYPE_API void DrawVerticalLabel(AHUD* HUD, UCanvas* Canvas,
        const FPrototypeHUDRect& Rect, float Scale, const FString& Label,
        FLinearColor Color, float ReferenceFontScale = .85f);
}
