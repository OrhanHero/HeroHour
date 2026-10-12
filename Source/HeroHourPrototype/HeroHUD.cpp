#include "PrototypeGameMode.h"
#include "Engine/Canvas.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "BattlefieldLayout.h"

namespace
{
FString ConstructionNotice(const FString& Message)
{
    if(Message.Contains(TEXT("Construction unavailable"))) return TEXT("Bau gesperrt: Der Einsatz ist beendet.");
    if(Message.Contains(TEXT("Construction requires"))) return TEXT("Bau gesperrt: Ein eigener Hub muss überleben.");
    if(Message.Contains(TEXT("Place structures on"))) return TEXT("Ungültig: Gebäude auf der befahrbaren Straße platzieren.");
    if(Message.Contains(TEXT("Site too far"))) return TEXT("Zu weit entfernt: Höchstens 2300 Einheiten vom Hub bauen.");
    if(Message.Contains(TEXT("Site obstructed"))) return TEXT("Blockiert: Abstand zu Gebäuden, Fahrzeugen und Schrott.");
    return FString();
}
}

void APrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    auto* Controller = Cast<APrototypeController>(PlayerOwner);
    if (!Mode || !Controller || !Canvas) return;
    const float S = FMath::Min(Canvas->SizeX / 1440.f, Canvas->SizeY / 900.f);
    const float W = Canvas->SizeX / S, H = Canvas->SizeY / S;
    const FLinearColor Ink(0.013f, 0.019f, 0.021f, 0.97f), Frame(0.22f, 0.32f, 0.32f, 0.9f);
    const FLinearColor Mint(0.46f, 0.9f, 0.68f), Cyan(0.38f, 0.8f, 0.84f), White(0.84f, 0.89f, 0.86f);
    const FLinearColor Muted(0.48f, 0.59f, 0.57f), Red(0.95f, 0.32f, 0.23f), Gold(0.96f, 0.74f, 0.34f);
    auto Rect = [&](FLinearColor C, float X, float Y, float A, float B) { DrawRect(C, X*S, Y*S, A*S, B*S); };
    auto Line = [&](float X, float Y, float A, float B, FLinearColor C, float Thick=1.f) { DrawLine(X*S,Y*S,A*S,B*S,C,Thick*S); };
    auto Text = [&](const FString& T, float X, float Y, FLinearColor C, float Scale=1.f) { DrawText(T,C,X*S,Y*S,nullptr,Scale*S); };
    auto Panel = [&](float X,float Y,float A,float B) {
        Rect(Ink,X,Y,A,B); Line(X,Y,X+A,Y,Frame); Line(X,Y+B,X+A,Y+B,Frame);
        Line(X,Y,X,Y+B,Frame); Line(X+A,Y,X+A,Y+B,Frame);
        Line(X,Y,X+18,Y,Mint,2); Line(X,Y,X,Y+10,Mint,2);
        Line(X+A-18,Y+B,X+A,Y+B,Cyan,2);
    };
    auto TankIcon = [&](float X,float Y,float Scale,FLinearColor C) {
        // Armoured tracked vehicle symbol for the actual combat unit.
        Rect(C,X+3*Scale,Y+21*Scale,46*Scale,13*Scale);
        Rect(Ink,X+7*Scale,Y+25*Scale,38*Scale,5*Scale);
        for(int Wheel=0;Wheel<5;++Wheel) Rect(C,X+(9+Wheel*7)*Scale,Y+25*Scale,3*Scale,5*Scale);
        Rect(C,X+8*Scale,Y+17*Scale,37*Scale,6*Scale);
        Rect(C,X+19*Scale,Y+10*Scale,17*Scale,8*Scale);
        Rect(C,X+34*Scale,Y+12*Scale,20*Scale,3*Scale);
    };
    int32 Allies=0, Enemies=0, Chosen=0, Collectors=0;
    APrototypeUnit* SingleSelected=nullptr;
    float SelectedHealth=0,SelectedMax=0;
    for (TActorIterator<APrototypeUnit> It(GetWorld());It;++It) {
        if(It->Health<=0) continue;
        if(!It->IsStructure()) { if(It->Team==0) ++Allies; else ++Enemies; }
        if(It->Team==0 && It->Role==EPrototypeRole::Collector) ++Collectors;
        if(It->bSelected) { ++Chosen; SingleSelected=*It; SelectedHealth+=It->Health; SelectedMax+=It->MaxHealth; }
    }
    Panel(18,18,244,60);
    Text(TEXT("HERO"),32,24,White,2.2f); Text(TEXT("SCHWARZMARKT-KOMMANDO"),33,58,Mint,.85f);
    Panel(274,18,410,60);
    Text(TEXT("KREDITS"),290,26,Muted,.8f);
    Text(FString::Printf(TEXT("%04d"),FMath::FloorToInt(Mode->Credits)),290,43,Gold,1.65f);
    Text(FString::Printf(TEXT("%02d SAMMLER / SCHROTTWIRTSCHAFT"),Collectors),405,48,Mint,.8f);
    Panel(W-396,18,378,60);
    Text(TEXT("CITY HERO / INDUSTRIEGRENZE"),W-380,27,Muted,.85f);
    Text(FString::Printf(TEXT("EINSATZZEIT %02d:%02d / FEIND %02d"),FMath::FloorToInt(Mode->MatchTime)/60,FMath::FloorToInt(Mode->MatchTime)%60,Enemies),W-380,46,Red,1.05f);
    const FString Mission=Mode->MatchTime<60.f && Mode->Outcome==0
        ? FString::Printf(TEXT("AUFBAUPHASE / Gegnerangriff in %02d s. Werkstatt bauen und Schrott sammeln."),FMath::CeilToInt(60.f-Mode->MatchTime))
        : TEXT("AUFTRAG / Feindlichen Hub zerstören. Eigenen Hub verteidigen.");
    Text(Mission,24,91,White,.9f);
    if(Controller->bPlacing && Mode->Outcome==0) {
        const bool Depot=Controller->PlacementRole==EPrototypeRole::Depot;
        Panel(W/2-240,140,480,53);
        Text(Depot?TEXT("SCHROTTDEPOT PLATZIEREN / 200 KREDITS"):TEXT("WERKSTATT PLATZIEREN / 350 KREDITS"),W/2-224,149,Mint,.95f);
        FString PlacementHint=TEXT("Grün: Straße, max. 2300 zum Hub / Abstand halten / ESC");
        FLinearColor HintColor=White;
        const FString SiteNotice=ConstructionNotice(Mode->Feedback);
        if(!SiteNotice.IsEmpty()) {
            PlacementHint=SiteNotice;
            HintColor=Red;
        } else if(Mode->Feedback.Contains(TEXT("Invalid site"))) {
            PlacementHint=TEXT("Ungültig: Straße, Hub-Nähe und Abstände prüfen.");
            HintColor=Red;
        } else if(Mode->Feedback.Contains(TEXT("Not enough")) || Mode->Feedback.Contains(TEXT("Insufficient credits"))) {
            PlacementHint=TEXT("Zu wenig Kredits: weiteren Schrott abliefern.");
            HintColor=Gold;
        }
        Text(PlacementHint,W/2-224,173,HintColor,.8f);
    }

    const float B=H-216;
    Panel(18,B,242,184);
    Text(TEXT("TAKTISCHE ÜBERSICHT"),30,B+10,Cyan,.85f);
    const float MX=30, MY=B+49, MW=146;
    const float MH=MW*BattlefieldLayout::ExtentY/BattlefieldLayout::ExtentX;
    auto MapX=[&](float WorldX) { return MX+FMath::Clamp((WorldX+BattlefieldLayout::ExtentX)/(2.f*BattlefieldLayout::ExtentX),0.f,1.f)*MW; };
    auto MapY=[&](float WorldY) { return MY+FMath::Clamp((WorldY+BattlefieldLayout::ExtentY)/(2.f*BattlefieldLayout::ExtentY),0.f,1.f)*MH; };
    auto MapRegion=[&](float X0,float Y0,float X1,float Y1,FLinearColor Color) { Rect(Color,MapX(X0),MapY(Y0),MapX(X1)-MapX(X0),MapY(Y1)-MapY(Y0)); };
    Rect(FLinearColor(.035f,.048f,.038f,1),MX,MY,MW,MH);
    // Geography uses the same world coordinates and bridge dimensions as navigation.
    MapRegion(1000,-4500,6500,4500,FLinearColor(.09f,.10f,.10f,1));
    MapRegion(-10000,1000,-5000,6000,FLinearColor(.055f,.10f,.065f,1));
    MapRegion(7000,2500,10000,5500,FLinearColor(.15f,.105f,.055f,1));
    MapRegion(BattlefieldLayout::RiverX-BattlefieldLayout::RiverHalfWidth,-BattlefieldLayout::ExtentY,
        BattlefieldLayout::RiverX+BattlefieldLayout::RiverHalfWidth,BattlefieldLayout::ExtentY,FLinearColor(.035f,.13f,.17f,1));
    for(float BridgeY:{-BattlefieldLayout::BridgeOffsetY,BattlefieldLayout::BridgeOffsetY}) {
        MapRegion(BattlefieldLayout::RiverX-BattlefieldLayout::RiverHalfWidth,BridgeY-BattlefieldLayout::BridgeHalfWidth,
            BattlefieldLayout::RiverX+BattlefieldLayout::RiverHalfWidth,BridgeY+BattlefieldLayout::BridgeHalfWidth,FLinearColor(.3f,.32f,.28f,1));
    }
    for(int I=0;I<=4;++I) { Line(MX+MW*I/4,MY,MX+MW*I/4,MY+MH,Frame,.5f); Line(MX,MY+MH*I/4,MX+MW,MY+MH*I/4,Frame,.5f); }
    // World X runs left-to-right; world Y runs top-to-bottom. Every live unit is represented.
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It) {
        if(It->Health<=0) continue;
        FVector P=It->GetActorLocation();
        float X=MapX(P.X), Y=MapY(P.Y);
        float D=It->IsStructure()?6.f:3.5f;
        if(It->bSelected) Rect(White,X-4,Y-4,8,8);
        Rect(It->Team==0?Mint:Red,X-D/2,Y-D/2,D,D);
    }
    for(const auto& Node:Mode->SalvageNodes) {
        if(Node.Remaining<=0) continue;
        float X=MapX(Node.Position.X), Y=MapY(Node.Position.Y);
        Rect(Gold,X-2,Y-2,4,4);
    }
    Text(TEXT("FLUSS / ZWEI BRÜCKEN"),MX,B+166,Muted,.62f);
    Rect(Mint,187,B+48,6,6); Text(TEXT("HERO"),199,B+46,Mint,.75f);
    Rect(Red,187,B+70,6,6); Text(TEXT("FEIND"),199,B+68,Red,.75f);
    Rect(Gold,187,B+92,6,6); Text(TEXT("SCHROTT"),199,B+90,Gold,.65f);
    Text(TEXT("HUB"),186,B+113,Muted,.75f);
    float HQRatio=IsValid(Mode->PlayerHQ)?Mode->PlayerHQ->Health/Mode->PlayerHQ->MaxHealth:0;
    Text(FString::Printf(TEXT("%d%%"),FMath::RoundToInt(HQRatio*100)),186,B+132,White,1.2f);
    Rect(Frame,186,B+152,61,5); Rect(Mint,186,B+152,61*HQRatio,5);

    const float CX=274, CW=W-640;
    Panel(CX,B,CW,184);
    Text(TEXT("EINSATZGRUPPE"),CX+16,B+10,Muted,.85f);
    if(Chosen==1 && SingleSelected && SingleSelected->IsStructure()) {
        const FLinearColor C=SingleSelected->IsCompleted()?Mint:Gold;
        Rect(C,CX+22,B+69,65,32); Rect(C,CX+31,B+56,47,15);
        Rect(Ink,CX+46,B+79,17,22); Rect(Ink,CX+32,B+75,8,7);
        Rect(Ink,CX+68,B+75,8,7); Line(CX+20,B+102,CX+90,B+102,C,2);
        Text(TEXT("HERO"),CX+36,B+59,Ink,.8f);
    } else TankIcon(CX+18,B+48,1.5f,Chosen>0?Mint:Muted);
    FString SelectedTitle=Chosen>0?FString::Printf(TEXT("%02d OBJEKTE AUSGEWÄHLT"),Chosen):TEXT("KEINE AUSWAHL");
    if(Chosen==1 && SingleSelected) {
        switch(SingleSelected->Role) {
        case EPrototypeRole::Headquarters: SelectedTitle=TEXT("SCHWARZMARKT-HUB"); break;
        case EPrototypeRole::Depot: SelectedTitle=TEXT("SCHROTTDEPOT"); break;
        case EPrototypeRole::Factory: SelectedTitle=TEXT("FAHRZEUGWERKSTATT"); break;
        case EPrototypeRole::Collector: SelectedTitle=TEXT("SCHROTTSAMMLER"); break;
        default: SelectedTitle=TEXT("PANZER"); break;
        }
    }
    Text(SelectedTitle,CX+108,B+38,White,1.1f);
    FString Details=FString::Printf(TEXT("FAHRZEUGE %02d / 40   ·   SAMMLER %02d"),Allies,Collectors);
    if(Chosen==1 && SingleSelected) {
        if(!SingleSelected->IsCompleted()) Details=FString::Printf(TEXT("BAUFORTSCHRITT %d%%"),FMath::RoundToInt(SingleSelected->ConstructionProgress*100));
        else if(SingleSelected->IsStructure()) Details=FString::Printf(TEXT("PRODUKTIONSWARTESCHLANGE: %d"),Mode->GetQueueCount(SingleSelected));
        else if(SingleSelected->Role==EPrototypeRole::Collector) Details=FString::Printf(TEXT("SCHROTTLADUNG: %.0f / 100"),SingleSelected->Cargo);
    }
    Text(Details,CX+108,B+65,Cyan,.85f);
    Text(TEXT("ZUSTAND / PRODUKTION"),CX+108,B+92,Muted,.75f);
    Rect(Frame,CX+108,B+112,CW-126,6);
    if(SelectedMax>0) Rect(Mint,CX+108,B+112,(CW-126)*SelectedHealth/SelectedMax,6);
    if(Chosen==1 && SingleSelected && SingleSelected->IsStructure()) {
        float Progress=SingleSelected->IsCompleted()?Mode->GetProductionProgress(SingleSelected):SingleSelected->ConstructionProgress;
        Rect(Frame,CX+108,B+122,CW-126,4); Rect(Gold,CX+108,B+122,(CW-126)*Progress,4);
    }
    Line(CX+16,B+132,CX+CW-16,B+132,Frame);
    Text(TEXT("RECHTSKLICK  Bewegen / angreifen"),CX+16,B+143,White,.85f);
    Text(TEXT("LEERTASTE  Armee wählen   ·   SHIFT  Auswahl erweitern"),CX+16,B+163,Muted,.78f);

    Panel(W-352,B,334,184);
    Text(TEXT("BAU / FAHRZEUGPRODUKTION"),W-336,B+10,Cyan,.85f);
    auto Tile=[&](float X,float Y,const TCHAR* Label,const TCHAR* Info,EPrototypeRole InRole,bool Available) {
        Rect(Available?FLinearColor(.07f,.14f,.11f,1):FLinearColor(.05f,.07f,.07f,1),X,Y,145,38);
        const FLinearColor Accent=InRole==EPrototypeRole::Collector?Gold:(InRole==EPrototypeRole::Combat?Mint:Cyan);
        Line(X,Y,X+145,Y,Available?Accent:Frame);
        Text(Label,X+7,Y+5,Available?White:Muted,.78f); Text(Info,X+7,Y+22,Gold,.67f);
    };
    bool Active=Mode->Outcome==0 && IsValid(Mode->PlayerHQ);
    Tile(W-336,B+34,TEXT("[V] SCHROTTDEPOT"),TEXT("200 KR / 8 SEK"),EPrototypeRole::Depot,Active&&Mode->Credits>=200);
    Tile(W-179,B+34,TEXT("[B] WERKSTATT"),TEXT("350 KR / 12 SEK"),EPrototypeRole::Factory,Active&&Mode->Credits>=350);
    Tile(W-336,B+78,TEXT("[P] PANZER"),TEXT("100 KR / 5 SEK"),EPrototypeRole::Combat,Active&&Mode->Credits>=100&&Mode->FindCompletedStructure(0,EPrototypeRole::Factory));
    Tile(W-179,B+78,TEXT("[C] SAMMLER"),TEXT("150 KR / 6 SEK"),EPrototypeRole::Collector,Active&&Mode->Credits>=150&&Mode->FindCompletedStructure(0,EPrototypeRole::Headquarters));
    Text(FString::Printf(TEXT("AUFTRÄGE: %d / BAU PER LINKSKLICK"),Mode->GetQueueCount()),W-336,B+129,Cyan,.76f);
    Text(TEXT("Depot: Schrott abliefern / Werkstatt: Panzer"),W-336,B+148,Muted,.72f);
    Text(TEXT("Sammler holen Schrott automatisch. ESC: Abbruch"),W-336,B+165,Muted,.68f);
    Text(TEXT("WASD  Kamera   ·   MAUSRAD  Zoom   ·   ZIEHEN  Gruppenauswahl   ·   R  Neustart"),24,H-22,Muted,.8f);

    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It) {
        if(It->Health<=0 || (!It->bHQ && !It->bSelected && It->Health>=It->MaxHealth && It->IsCompleted())) continue;
        FVector2D P;
        float LabelHeight=180.f;
        if(It->Role==EPrototypeRole::Headquarters) LabelHeight=570.f;
        else if(It->Role==EPrototypeRole::Factory) LabelHeight=350.f;
        else if(It->Role==EPrototypeRole::Depot) LabelHeight=260.f;
        if(!Controller->ProjectWorldLocationToScreen(It->GetActorLocation()+FVector(0,0,LabelHeight),P)) continue;
        if(P.Y<115*S || P.Y>B*S-12 || P.X<0 || P.X>Canvas->SizeX) continue;
        float Width=(It->IsStructure()?108.f:46.f)*S;
        DrawRect(Ink,P.X-Width/2-2*S,P.Y-2*S,Width+4*S,9*S);
        DrawRect(It->Team==0?Mint:Red,P.X-Width/2,P.Y,Width*It->Health/It->MaxHealth,4*S);
        if(It->IsStructure()) {
            FString Name=It->Role==EPrototypeRole::Headquarters?TEXT("SCHWARZMARKT-HUB"):(It->Role==EPrototypeRole::Factory?TEXT("WERKSTATT"):TEXT("SCHROTTDEPOT"));
            if(It->Team==1) Name=TEXT("FEIND / ")+Name;
            if(!It->IsCompleted()) {
                Name+=FString::Printf(TEXT(" / %d%%"),FMath::RoundToInt(It->ConstructionProgress*100));
                DrawRect(Frame,P.X-Width/2,P.Y+7*S,Width,3*S);
                DrawRect(Gold,P.X-Width/2,P.Y+7*S,Width*It->ConstructionProgress,3*S);
            }
            DrawText(Name,White,P.X-Width/2,P.Y-17*S,nullptr,.65f*S);
        }
    }
    if(Controller->bDragging) {
        FVector2D Mouse; Controller->GetMousePosition(Mouse.X,Mouse.Y);
        const auto Start=Controller->DragStart;
        DrawRect(FLinearColor(.25f,.7f,.5f,.09f),FMath::Min(Start.X,Mouse.X),FMath::Min(Start.Y,Mouse.Y),FMath::Abs(Start.X-Mouse.X),FMath::Abs(Start.Y-Mouse.Y));
        DrawLine(Start.X,Start.Y,Mouse.X,Start.Y,Mint,1.3f*S); DrawLine(Mouse.X,Start.Y,Mouse.X,Mouse.Y,Mint,1.3f*S);
        DrawLine(Mouse.X,Mouse.Y,Start.X,Mouse.Y,Mint,1.3f*S); DrawLine(Start.X,Mouse.Y,Start.X,Start.Y,Mint,1.3f*S);
    }
    if(Mode->Outcome!=0) {
        Panel(W/2-250,H/2-75,500,150);
        Text(Mode->Outcome>0?TEXT("SEKTOR GESICHERT"):TEXT("HUB VERLOREN"),W/2-218,H/2-50,Mode->Outcome>0?Mint:Red,2.f);
        Text(Mode->Outcome>0?TEXT("Der feindliche Hub wurde ausgeschaltet."):TEXT("Das Schwarzmarkt-Kommando wurde zerstört."),W/2-218,H/2-8,White,1.f);
        Text(TEXT("[ R ]  NEUEN EINSATZ STARTEN"),W/2-218,H/2+35,Cyan,1.f);
    } else if(!Mode->Feedback.IsEmpty()) {
        FString Notice=ConstructionNotice(Mode->Feedback);
        if(Notice.IsEmpty()) Notice=Mode->Feedback;
        if(Notice.Contains(TEXT("Not enough credits."))) Notice=TEXT("NACHSCHUB: Mindestens 100 Kredits erforderlich.");
        else if(Notice.Contains(TEXT("Army limit"))) Notice=TEXT("NACHSCHUB: Armeelimit von 40 Fahrzeugen erreicht.");
        else if(Notice.Contains(TEXT("Reinforcement"))) Notice=TEXT("NACHSCHUB: Panzer am eigenen Hub eingetroffen.");
        else if(Notice.Contains(TEXT("Invalid site"))) Notice=TEXT("BAUPLATZ UNGÜLTIG: Nahe am Hub, auf der Straße und frei von Fahrzeugen oder Schrott bauen.");
        else if(Notice.Contains(TEXT("Not enough salvage"))) Notice=TEXT("BAU: Zu wenig Kredits. Sammler müssen weiteren Schrott abliefern.");
        else if(Notice.Contains(TEXT("Vehicle factory under"))) Notice=TEXT("BAU: Werkstatt wird errichtet / 12 Sekunden.");
        else if(Notice.Contains(TEXT("Salvage depot under"))) Notice=TEXT("BAU: Schrottdepot wird errichtet / 8 Sekunden.");
        else if(Notice.Contains(TEXT("Build a completed"))) Notice=TEXT("PRODUKTION: Fertige Werkstatt erforderlich. Jede Werkstatt nimmt 5 Aufträge an.");
        else if(Notice.Contains(TEXT("HQ queue"))) Notice=TEXT("PRODUKTION: Hub nicht verfügbar oder Warteschlange voll.");
        else if(Notice.Contains(TEXT("Army capacity"))) Notice=TEXT("PRODUKTION: Fahrzeuglimit 40 erreicht, einschließlich wartender Aufträge.");
        else if(Notice.Contains(TEXT("Insufficient credits"))) Notice=TEXT("PRODUKTION: Zu wenig Kredits. Weiteren Schrott abliefern.");
        else if(Notice.Contains(TEXT("Combat pickup queued"))) Notice=TEXT("PRODUKTION: Panzer bestellt / 5 Sekunden.");
        else if(Notice.Contains(TEXT("Salvage collector queued"))) Notice=TEXT("PRODUKTION: Schrottsammler bestellt / 6 Sekunden.");
        else if(Notice.Contains(TEXT("Exit obstructed"))) Notice=TEXT("AUSFAHRT BLOCKIERT: Fahrzeuge vom Produktionsgebäude wegbewegen.");
        Text(Notice,24,114,Gold,.85f);
    }
}


bool APrototypeHUD::HandleClick(const FVector2D& ScreenPosition)
{
    auto* Mode=GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    auto* Controller=Cast<APrototypeController>(PlayerOwner);
    if(!Mode || !Controller) return false;
    int32 SizeX=0,SizeY=0; Controller->GetViewportSize(SizeX,SizeY);
    if(SizeX<=0 || SizeY<=0) return false;
    const float S=FMath::Min(SizeX/1440.f,SizeY/900.f);
    const float W=SizeX/S,H=SizeY/S,B=H-216;
    const FVector2D P=ScreenPosition/S;
    if(P.Y>=B && P.X>=18 && P.X<=W-18) {
        if(P.X>=W-336 && P.X<=W-34 && Mode->Outcome==0) {
            if(P.Y>=B+34 && P.Y<=B+72) {
                if(P.X<=W-191) Controller->BeginPlacement(EPrototypeRole::Depot);
                else if(P.X>=W-179) Controller->BeginPlacement(EPrototypeRole::Factory);
            } else if(P.Y>=B+78 && P.Y<=B+116) {
                if(P.X<=W-191) Mode->RequestProduction(EPrototypeRole::Combat);
                else if(P.X>=W-179) Mode->RequestProduction(EPrototypeRole::Collector);
            }
        }
        return true;
    }
    return P.Y<=110;
}
