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
    FPrototypeHUDRect Resource() const { return {12,8,560,60}; }
    FPrototypeHUDRect Army() const { return {W*.5f-160,10,320,80}; }
    FPrototypeHUDRect Mission() const { return {W-340,12,328,168}; }
    FPrototypeHUDRect MiniMap() const { return {12,H-352,310,330}; }
    FPrototypeHUDRect Selection() const { return {W*.5f-285,H-296,570,274}; }
    FPrototypeHUDRect Production() const { return {W-438,H-362,426,340}; }
    FPrototypeHUDRect Events() const { return {12,H-460,310,94}; }
    FPrototypeHUDRect Placement() const { return {W*.5f-260,118,520,58}; }
    FPrototypeHUDRect ProductionTile(int32 Column, int32 Row) const
    {
        const auto P=Production();return {P.X+16+Column*80,P.Y+106+Row*100,74,94};
    }
    static constexpr int32 ProductionColumns=5;
    FPrototypeHUDRect ProductionCard(int32 Card) const { return ProductionTile(Card%ProductionColumns,Card/ProductionColumns); }
    FPrototypeHUDRect DoctrineCard(int32 Card) const { const auto P=Mission();return {P.X+15+Card*151,P.Y+54,145,78}; }
    FPrototypeHUDRect ProductionTab(int32 Category) const
    {
        const auto P=Production();return {P.X+18+Category*78,P.Y+39,74,40};
    }
    FPrototypeHUDRect QueueSlot(int32 Slot) const { const auto P=Production();return {P.X+20+Slot*76,P.Y+238,68,40}; }
    FPrototypeHUDRect Cancel() const { const auto P=Production();return {P.X+18,P.Y+286,390,28}; }
    FPrototypeHUDRect Focus() const { const auto P=Selection();return {P.X+429,P.Y+168,122,40}; }
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
            ||Selection().Contains(P)||Production().Contains(P)||Events().Contains(P);
    }
};
