#pragma once
#include "CoreMinimal.h"
#include "PrototypeEconomyTypes.h"

class APrototypeHUD;
enum class EHRDoctrine : uint8;
// Shared by sidebar buttons and controller hotkeys; the next battlefield click
// confirms the target through the authoritative game-mode command.
HEROHOURPROTOTYPE_API void BeginPrototypeDoctrineTarget(APrototypeHUD* HUD,EHRDoctrine Doctrine);
HEROHOURPROTOTYPE_API void CancelPrototypeDoctrineTarget(APrototypeHUD* HUD);
HEROHOURPROTOTYPE_API bool HasPrototypeDoctrineTarget(const APrototypeHUD* HUD);

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
    static constexpr float MinimumTextLinePixels=12.f;
    float TextScale(float ReferenceScale,float UnscaledFontHeight) const
    {
        return FMath::Max(ReferenceScale*S,MinimumTextLinePixels/FMath::Max(1.f,UnscaledFontHeight));
    }
    static constexpr float SidebarWidth=264.f;
    FPrototypeHUDRect Resource() const { return {W-SidebarWidth-454,12,430,46}; }
    FPrototypeHUDRect Army() const { return {12,12,260,46}; }
    FPrototypeHUDRect Mission() const { return {W*.5f-62,4,124,46}; }
    FPrototypeHUDRect MiniMap() const { return {W-SidebarWidth-12,12,SidebarWidth,320}; }
    FPrototypeHUDRect Selection() const { return {330,H-296,570,274}; }
    FPrototypeHUDRect Production() const { return {W-SidebarWidth-12,344,SidebarWidth,H-366}; }
    FPrototypeHUDRect Events() const { return {82,H-210,236,104}; }
    FPrototypeHUDRect Placement() const { return {W*.5f-260,118,520,58}; }
    FPrototypeHUDRect Menu() const { return {W*.5f-240,H*.5f-124,480,248}; }
    FPrototypeHUDRect MenuAction(int32 Index) const { const auto P=Menu();return {P.X+30,P.Y+128+Index*50,P.W-60,40}; }
    FPrototypeHUDRect ProductionTile(int32 Column, int32 Row) const
    {
        const auto P=Production();return {P.X+12+Column*80,P.Y+202+Row*100,74,94};
    }
    static constexpr int32 ProductionColumns=3;
    FPrototypeHUDRect ProductionCard(int32 Card) const { return ProductionTile(Card%ProductionColumns,Card/ProductionColumns); }
    FPrototypeHUDRect DoctrineCard(int32 Card) const { return {8,H*.38f+Card*66.f,58,58}; }
    FPrototypeHUDRect PrimaryTab(int32 Index) const { const auto P=Production();return {P.X+8+Index*124,P.Y+8,120,40}; }
    FPrototypeHUDRect ProductionTab(int32 Category) const
    {
        const auto P=Production();return {P.X+12+Category*48,P.Y+60,46,108};
    }
    FPrototypeHUDRect QueueSlot(int32 Slot) const { const auto P=Production();return {P.X+12+Slot*48,P.Y+P.H-92,44,40}; }
    FPrototypeHUDRect Cancel() const { const auto P=Production();return {P.X+12,P.Y+P.H-43,P.W-24,28}; }
    FPrototypeHUDRect ControlGroup(int32 Slot) const { return {12+Slot*29.f,H-92,27,70}; }
    FPrototypeHUDRect Focus() const { const auto P=Selection();return {P.X+429,P.Y+168,122,40}; }
    FPrototypeHUDRect SelectionMember(int32 Index) const { const auto P=Selection();return {P.X+18+(Index%3)*50,P.Y+43+(Index/3)*50,48,48}; }
    FPrototypeHUDRect Ability(int32 Index) const { const auto P=Selection();return {P.X+182+Index*123,P.Y+218,114,40}; }
    FPrototypeHUDRect PowerToggle() const { return Ability(0); }
    FPrototypeHUDRect Sell() const { return Ability(1); }
    static int32 CategoryCardCount(int32 Category)
    {
        return Category==0 ? 5 : Category==1 || Category==2 ? 1 : Category==3 ? 3 : Category==4 ? 2 : 0;
    }
    static bool CategoryCardRole(int32 Category,int32 Card,EPrototypeRole& Role)
    {
        if(Card<0||Card>=CategoryCardCount(Category)||Category==4)return false;
        static constexpr EPrototypeRole Buildings[]={EPrototypeRole::Depot,EPrototypeRole::Factory,EPrototypeRole::PowerPlant,EPrototypeRole::Barracks,EPrototypeRole::Radar};
        static constexpr EPrototypeRole Vehicles[]={EPrototypeRole::Combat,EPrototypeRole::Collector,EPrototypeRole::Siege};
        Role=Category==0 ? Buildings[Card] : Category==1 ? EPrototypeRole::Defense : Category==2 ? EPrototypeRole::Infantry : Vehicles[Card];
        return true;
    }
    bool IsOverPanel(const FVector2D& Screen) const
    {
        const FVector2D P=Screen/S;
        return Resource().Contains(P)||Army().Contains(P)||Mission().Contains(P)||MiniMap().Contains(P)
            ||Selection().Contains(P)||Production().Contains(P)||Events().Contains(P)
            ||DoctrineCard(0).Contains(P)||DoctrineCard(1).Contains(P)
            ||FPrototypeHUDRect{12,H-92,290,70}.Contains(P);
    }
};
