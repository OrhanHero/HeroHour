#include "PrototypeHUD.h"

#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "HHPrototypeCombatActor.h"
#include "HHPrototypeGameMode.h"
#include "HHPrototypeEconomyComponent.h"
#include "HHPrototypeFogOfWarActor.h"
#include "HHRTSPlayerController.h"
#include "EngineUtils.h"

void APrototypeHUD::DrawHUD()
{
    Super::DrawHUD();

    if (!Canvas || !GEngine)
    {
        return;
    }

    UFont* Font = GEngine->GetSmallFont();
    const float W = Canvas->ClipX;
    const float H = Canvas->ClipY;
    const float S = FMath::Clamp(FMath::Min(W / 1920.0f, H / 1080.0f), 0.72f, 1.35f);
    const FLinearColor Panel(0.018f, 0.029f, 0.038f, 0.94f);
    const FLinearColor Panel2(0.035f, 0.055f, 0.067f, 0.96f);
    const FLinearColor Cyan(0.05f, 0.78f, 0.95f, 1.0f);
    const FLinearColor Red(0.94f, 0.16f, 0.12f, 1.0f);
    const FLinearColor Gold(0.95f, 0.70f, 0.16f, 1.0f);
    const FLinearColor Muted(0.54f, 0.63f, 0.66f, 1.0f);
    auto Box = [this](float X, float Y, float BW, float BH, const FLinearColor& Fill, const FLinearColor& Edge)
    {
        DrawRect(Edge, X, Y, BW, BH);
        DrawRect(Fill, X + 2.0f, Y + 2.0f, BW - 4.0f, BH - 4.0f);
    };
    auto Label = [this, Font](const FString& Text, const FLinearColor& Color, float X, float Y, float Scale = 1.0f)
    {
        DrawText(Text, Color, X, Y, Font, Scale, false);
    };

    // Command-center top bar.
    Box(18*S, 16*S, W-36*S, 76*S, Panel, FLinearColor(0.11f, 0.18f, 0.21f, 1));
    DrawRect(Cyan, 20*S, 18*S, 6*S, 72*S);
    Label(TEXT("HERO HOUR"), FLinearColor::White, 42*S, 29*S, 1.35f*S);
    Label(TEXT("TACTICAL COMMAND"), Muted, 43*S, 57*S, 0.82f*S);

    FString MatchStatus = TEXT("COMBAT ACTIVE");
    FLinearColor StatusColor = Gold;
    const AHHPrototypeGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AHHPrototypeGameMode>() : nullptr;
    if (GameMode)
    {
        if (GameMode->GetMatchResult() == EHHPrototypeMatchResult::HeroVictory) { MatchStatus = TEXT("MISSION ACCOMPLISHED"); StatusColor = Cyan; }
        else if (GameMode->GetMatchResult() == EHHPrototypeMatchResult::USAVictory) { MatchStatus = TEXT("MISSION FAILED"); StatusColor = Red; }
        else if (!GameMode->HasUSAStarted()) { MatchStatus = FString::Printf(TEXT("HOSTILE MOBILIZATION  T-%.0f"), GameMode->GetUSAOrderTimeRemaining()); }
    }
    float TW=0, TH=0;
    GetTextSize(MatchStatus, TW, TH, Font, 1.05f*S);
    Label(MatchStatus, StatusColor, (W-TW)*0.5f, 28*S, 1.05f*S);
    const FString Objective = TEXT("PRIMARY OBJECTIVE  //  DESTROY USA HEADQUARTERS");
    GetTextSize(Objective, TW, TH, Font, 0.82f*S);
    Label(Objective, FLinearColor(0.82f,0.86f,0.84f,1), (W-TW)*0.5f, 57*S, 0.82f*S);
    Label(TEXT("OPERATION 01"), Gold, W-180*S, 31*S, 0.9f*S);
    Label(TEXT("SECTOR: RED VALLEY"), Muted, W-180*S, 57*S, 0.72f*S);
    if (GameMode)
    {
        const int32 HeroCredits = GameMode->GetHeroEconomy() ? GameMode->GetHeroEconomy()->GetCredits() : 0;
        const int32 USACredits = GameMode->GetUSAEconomy() ? GameMode->GetUSAEconomy()->GetCredits() : 0;
        Label(FString::Printf(TEXT("HERO  $%04d"), HeroCredits), Cyan, 300*S, 32*S, 0.92f*S);
        Label(FString::Printf(TEXT("USA  $%04d"), USACredits), Red, 300*S, 58*S, 0.72f*S);
    }

    // Left faction telemetry stack.
    const float SideY = 112*S;
    Box(18*S, SideY, 250*S, 226*S, Panel, FLinearColor(0.08f,0.16f,0.18f,1));
    Label(TEXT("FORCE STATUS"), Gold, 34*S, SideY+16*S, 0.85f*S);
    float HeroHQHealth = 0.0f;
    float USAHQHealth = 0.0f;
    for (TActorIterator<AHHPrototypeCombatActor> It(GetWorld()); It; ++It)
    {
        if (It->IsHeadquarters())
        {
            const float Ratio = It->GetHealthRatio();
            if (It->GetFaction() == EHHPrototypeFaction::Hero) HeroHQHealth = Ratio;
            else USAHQHealth = Ratio;
        }
    }
    auto DrawFaction = [&](const TCHAR* Name, const TCHAR* Subtitle, float Y, const FLinearColor& Color, float HealthRatio)
    {
        DrawRect(Panel2, 32*S, Y, 220*S, 75*S);
        DrawRect(Color, 32*S, Y, 5*S, 75*S);
        Label(Name, Color, 48*S, Y+9*S, 1.15f*S);
        Label(Subtitle, Muted, 48*S, Y+32*S, 0.72f*S);
        Label(TEXT("HQ"), FLinearColor::White, 48*S, Y+53*S, 0.72f*S);
        DrawRect(FLinearColor(0.08f,0.10f,0.10f,1), 82*S, Y+56*S, 154*S, 8*S);
        DrawRect(Color, 84*S, Y+58*S, 150*S*FMath::Clamp(HealthRatio, 0.f, 1.f), 4*S);
        Label(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(HealthRatio*100.f)), FLinearColor::White, 196*S, Y+42*S, 0.62f*S);
    };
    DrawFaction(TEXT("HERO"), TEXT("EXPEDITIONARY FORCE"), SideY+47*S, Cyan, HeroHQHealth);
    DrawFaction(TEXT("USA"), TEXT("HOSTILE COMMAND"), SideY+132*S, Red, USAHQHealth);

    // Tactical minimap: schematic by design and always useful in the asset-free fallback.
    const float MapW=238*S, MapH=206*S, MapX=18*S, MapY=H-MapH-18*S;
    Box(MapX, MapY, MapW, MapH, Panel, FLinearColor(0.10f,0.19f,0.20f,1));
    Label(TEXT("TACTICAL MAP"), Gold, MapX+15*S, MapY+13*S, 0.82f*S);
    DrawRect(FLinearColor(0.045f,0.075f,0.072f,1), MapX+14*S, MapY+40*S, MapW-28*S, MapH-55*S);
    for (int32 I=1; I<5; ++I)
    {
        const float GX=MapX+14*S+(MapW-28*S)*I/5.f;
        const float GY=MapY+40*S+(MapH-55*S)*I/5.f;
        DrawLine(GX, MapY+40*S, GX, MapY+MapH-15*S, FLinearColor(0.08f,0.15f,0.14f,1), 1.f);
        DrawLine(MapX+14*S, GY, MapX+MapW-14*S, GY, FLinearColor(0.08f,0.15f,0.14f,1), 1.f);
    }
    const float RadarX=MapX+14*S, RadarY=MapY+40*S, RadarW=MapW-28*S, RadarH=MapH-55*S;
    const AHHPrototypeFogOfWarActor* Fog = GameMode ? GameMode->GetFogOfWar() : nullptr;
    if (Fog)
    {
        const FIntPoint Grid = Fog->GetGridSize();
        const TArray<uint8>& Cells = Fog->GetVisibilityCells();
        const int32 Step = 4;
        for (int32 Y=0; Y<Grid.Y; Y+=Step) for (int32 X=0; X<Grid.X; X+=Step)
        {
            const int32 Index=Y*Grid.X+X;
            if (!Cells.IsValidIndex(Index)) continue;
            const EHHFogVisibility State=static_cast<EHHFogVisibility>(Cells[Index]);
            const FLinearColor Shade = State == EHHFogVisibility::Unseen
                ? FLinearColor(0.005f,0.008f,0.008f,0.93f)
                : State == EHHFogVisibility::Explored ? FLinearColor(0.025f,0.04f,0.04f,0.72f)
                : FLinearColor(0.0f,0.0f,0.0f,0.06f);
            DrawRect(Shade, RadarX+RadarW*X/Grid.X, RadarY+RadarH*Y/Grid.Y,
                RadarW*Step/Grid.X+1, RadarH*Step/Grid.Y+1);
        }
    }
    for (TActorIterator<AHHPrototypeCombatActor> It(GetWorld()); It; ++It)
    {
        if (!It->IsAlive()) continue;
        if (It->GetFaction()==EHHPrototypeFaction::USA && Fog && !Fog->IsWorldLocationVisible(It->GetActorLocation())) continue;
        FVector2D UV;
        if (Fog && Fog->WorldToRadarUV(It->GetActorLocation(), UV))
        {
            const float Size=It->IsHeadquarters()?10*S:5*S;
            DrawRect(It->GetFaction()==EHHPrototypeFaction::Hero?Cyan:Red,
                RadarX+UV.X*RadarW-Size*0.5f, RadarY+UV.Y*RadarH-Size*0.5f, Size, Size);
        }
    }
    Label(TEXT("N"), FLinearColor::White, MapX+MapW-27*S, MapY+44*S, 0.7f*S);

    // Bottom selection and command deck.
    const float BottomH=178*S, BottomY=H-BottomH-18*S, DeckOffset=252*S;
    Box(18*S+DeckOffset, BottomY, W-36*S-DeckOffset, BottomH, Panel, FLinearColor(0.10f,0.18f,0.20f,1));
    DrawRect(Gold, 20*S+DeckOffset, BottomY+2*S, W-40*S-DeckOffset, 3*S);
    FString RoleName = TEXT("NO UNIT SELECTED");
    FString UnitClass = TEXT("CLICK A HERO UNIT TO ASSIGN ORDERS");
    bool bHasSelection=false;
    float SelectionHealth=0.0f;
    int32 SelectionCount=0;
    if (const AHHRTSPlayerController* Controller = Cast<AHHRTSPlayerController>(GetOwningPlayerController()))
    {
        SelectionCount=Controller->GetSelectedActors().Num();
        if (const AHHPrototypeCombatActor* Unit = Cast<AHHPrototypeCombatActor>(Controller->GetSelectedActor()))
        {
            bHasSelection=true;
            SelectionHealth=Unit->GetHealthRatio();
            RoleName = Unit->GetRole() == EHHPrototypeUnitRole::Infantry ? TEXT("INFANTRY SQUAD")
                : Unit->GetRole() == EHHPrototypeUnitRole::Vehicle ? TEXT("ARMORED VEHICLE") : TEXT("AIR SUPPORT");
            UnitClass = Unit->GetFaction() == EHHPrototypeFaction::Hero ? TEXT("HERO // COMBAT READY") : TEXT("USA // HOSTILE CONTACT");
            if (SelectionCount > 1)
            {
                RoleName=FString::Printf(TEXT("%d UNITS SELECTED"), SelectionCount);
                UnitClass=TEXT("HERO // FORMATION READY");
            }
        }
    }
    DrawRect(Panel2, 34*S+DeckOffset, BottomY+24*S, 370*S, 132*S);
    DrawRect(bHasSelection?Cyan:Muted, 34*S+DeckOffset, BottomY+24*S, 6*S, 132*S);
    Label(TEXT("SELECTED UNIT"), Gold, 52*S+DeckOffset, BottomY+36*S, 0.73f*S);
    Label(RoleName, bHasSelection?FLinearColor::White:Muted, 52*S+DeckOffset, BottomY+61*S, 1.2f*S);
    Label(UnitClass, Muted, 52*S+DeckOffset, BottomY+89*S, 0.72f*S);
    Label(TEXT("INTEGRITY"), Muted, 52*S+DeckOffset, BottomY+117*S, 0.68f*S);
    DrawRect(FLinearColor(0.08f,0.1f,0.1f,1), 122*S+DeckOffset, BottomY+120*S, 252*S, 10*S);
    if (bHasSelection) DrawRect(Cyan, 124*S+DeckOffset, BottomY+122*S, 248*S*SelectionHealth, 6*S);
    if (bHasSelection) Label(FString::Printf(TEXT("%d%%"), FMath::RoundToInt(SelectionHealth*100.f)), FLinearColor::White, 330*S+DeckOffset, BottomY+103*S, 0.65f*S);

    const float CmdX=682*S;
    Label(TEXT("COMMAND INTERFACE"), Gold, CmdX, BottomY+27*S, 0.74f*S);
    const TCHAR* Commands[] = { TEXT("MOVE / ATTACK"), TEXT("ATTACK MOVE"), TEXT("STOP"), TEXT("BOX SELECT") };
    const TCHAR* Keys[] = { TEXT("RMB"), TEXT("A + LMB"), TEXT("S"), TEXT("DRAG LMB") };
    for (int32 I=0; I<4; ++I)
    {
        const float X=CmdX+I*132*S;
        DrawRect(Panel2, X, BottomY+55*S, 116*S, 82*S);
        DrawRect(I<2?Cyan:Gold, X, BottomY+55*S, 116*S, 3*S);
        Label(Keys[I], I<2?Cyan:Gold, X+12*S, BottomY+69*S, 0.75f*S);
        Label(Commands[I], FLinearColor::White, X+12*S, BottomY+101*S, 0.82f*S);
    }
    const UHHPrototypeEconomyComponent* Economy=GameMode?GameMode->GetHeroEconomy():nullptr;
    const float ProdX=CmdX+548*S;
    Label(TEXT("FIELD PRODUCTION"), Gold, ProdX, BottomY+27*S, 0.74f*S);
    Label(TEXT("F1 INF $300   F2 VEH $800   F3 AIR $1200"), FLinearColor::White, ProdX, BottomY+57*S, 0.68f*S);
    DrawRect(FLinearColor(0.08f,0.1f,0.1f,1), ProdX, BottomY+83*S, 270*S, 12*S);
    if (Economy && Economy->IsProducing())
    {
        DrawRect(Gold, ProdX+2*S, BottomY+85*S, 266*S*Economy->GetProductionProgress(), 8*S);
        const TCHAR* QueueName=Economy->GetQueuedRole()==EHHPrototypeUnitRole::Infantry?TEXT("INFANTRY"):
            Economy->GetQueuedRole()==EHHPrototypeUnitRole::Vehicle?TEXT("VEHICLE"):TEXT("AIR SUPPORT");
        Label(FString::Printf(TEXT("BUILDING %s  %d%%  // C CANCEL"), QueueName,
            FMath::RoundToInt(Economy->GetProductionProgress()*100)), Gold, ProdX, BottomY+104*S, 0.68f*S);
    }
    else Label(TEXT("QUEUE READY"), Cyan, ProdX, BottomY+104*S, 0.68f*S);
    Label(TEXT("GROUPS  CTRL+1/2/3 STORE  //  1/2/3 RECALL"), Muted, ProdX, BottomY+130*S, 0.62f*S);
    Label(TEXT("CAMERA  WASD / EDGE SCROLL     ZOOM  MOUSE WHEEL"), Muted, W-520*S, BottomY+145*S, 0.68f*S);
}
