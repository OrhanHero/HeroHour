#pragma once
#include "CoreMinimal.h"

struct FPrototypeHUDRect
{
    float X, Y, W, H;
    bool Contains(const FVector2D& P) const { return P.X >= X && P.X <= X+W && P.Y >= Y && P.Y <= Y+H; }
};

// Rendering and mouse routing share the same concept-derived panel geometry.
struct FPrototypeHUDLayout
{
    float S, W, H;
    FPrototypeHUDLayout(float Width, float Height)
        : S(FMath::Max(.01f,FMath::Min(Width/1672.f,Height/941.f))), W(Width/S), H(Height/S) {}
    FPrototypeHUDRect Resource() const { return {12,8,520,68}; }
    FPrototypeHUDRect Army() const { return {W*.5f-175,10,350,86}; }
    FPrototypeHUDRect Mission() const { return {W-340,12,328,114}; }
    FPrototypeHUDRect MiniMap() const { return {12,H-352,310,330}; }
    FPrototypeHUDRect Selection() const { return {W*.5f-285,H-252,570,230}; }
    FPrototypeHUDRect Production() const { return {W-438,H-402,426,380}; }
    FPrototypeHUDRect Events() const { return {12,H-460,310,94}; }
    FPrototypeHUDRect Placement() const { return {W*.5f-260,118,520,58}; }
    FPrototypeHUDRect ProductionTile(int32 Column, int32 Row) const
    {
        const auto P=Production();return {P.X+18+Column*198,P.Y+106+Row*118,192,110};
    }
    FPrototypeHUDRect ProductionTab(int32 Category) const
    {
        const auto P=Production();return {P.X+18+Category*78,P.Y+39,74,40};
    }
    FPrototypeHUDRect Cancel() const { const auto P=Production();return {P.X+18,P.Y+310,390,28}; }
    FPrototypeHUDRect Focus() const { const auto P=Selection();return {P.X+429,P.Y+168,122,40}; }
    bool IsOverPanel(const FVector2D& Screen) const
    {
        const FVector2D P=Screen/S;
        return Resource().Contains(P)||Army().Contains(P)||Mission().Contains(P)||MiniMap().Contains(P)
            ||Selection().Contains(P)||Production().Contains(P)||Events().Contains(P);
    }
};
