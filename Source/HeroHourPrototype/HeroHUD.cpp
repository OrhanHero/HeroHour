#include "PrototypeGameMode.h"
#include "PrototypeHUDLayout.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "BattlefieldLayout.h"
#include "RTS/HRRTSDefinitions.h"

namespace
{
FPrototypeHUDRect MapContent(const FPrototypeHUDLayout& L)
{
    const auto R=L.MiniMap(); return {R.X+20,R.Y+43,R.W-40,(R.W-40)*BattlefieldLayout::ExtentY/BattlefieldLayout::ExtentX};
}
FString RoleName(EPrototypeRole R)
{
    if (const UHRUnitDefinition* Definition=APrototypeGameMode::GetRoleDefinition(R))
        return Definition->DisplayName.ToString();
    switch(R) {
    case EPrototypeRole::Headquarters:return TEXT("KOMMANDO-HUB");
    case EPrototypeRole::Factory:return TEXT("FAHRZEUGFABRIK");
    case EPrototypeRole::Depot:return TEXT("RAFFINERIE");
    case EPrototypeRole::Collector:return TEXT("SAMMLER");
    default:return TEXT("STRIKER / KAMPFFAHRZEUG");
    }
}
UTexture2D* Portrait(EPrototypeRole R)
{
    if (const UHRUnitDefinition* Definition=APrototypeGameMode::GetRoleDefinition(R))
        if (UTexture2D* Icon=Definition->Icon.LoadSynchronous()) return Icon;
    // LoadObject resolves the same imported package texture on subsequent draws.
    const TCHAR* Path=R==EPrototypeRole::Headquarters?TEXT("/Game/HERO/UI/T_HERO_CommandHub.T_HERO_CommandHub"):
        R==EPrototypeRole::Factory?TEXT("/Game/HERO/UI/T_HERO_VehiclePlant.T_HERO_VehiclePlant"):
        R==EPrototypeRole::Depot?TEXT("/Game/HERO/UI/T_HERO_Refinery.T_HERO_Refinery"):
        R==EPrototypeRole::Collector?TEXT("/Game/HERO/UI/T_HERO_Salvager.T_HERO_Salvager"):TEXT("/Game/HERO/UI/T_HERO_Striker.T_HERO_Striker");
    return LoadObject<UTexture2D>(nullptr,Path);
}
UTexture2D* InterfaceTexture(const TCHAR* Name)
{
    static TMap<FName,TWeakObjectPtr<UTexture2D>> Loaded;
    const FName Key(Name);
    if(const auto* Cached=Loaded.Find(Key);Cached&&Cached->IsValid())return Cached->Get();
    const FString Path=FString::Printf(TEXT("/Game/HERO/UI/ConceptV3/%s.%s"),Name,Name);
    UTexture2D* Texture=LoadObject<UTexture2D>(nullptr,*Path,nullptr,LOAD_NoWarn);
    Loaded.Add(Key,Texture);
    return Texture;
}
int32 VisibleCategory(const APrototypeHUD* HUD,const APrototypeUnit* Producer)
{
    return HUD->ProductionCategory>=0?HUD->ProductionCategory:
        Producer&&Producer->Role==EPrototypeRole::Factory?3:0;
}
bool CategoryHasContent(int32 Category) { return Category==0||Category==3; }
bool CategoryRole(int32 Category,int32 Card,EPrototypeRole& Role)
{
    if(Card<0||Card>1||!CategoryHasContent(Category))return false;
    Role=Category==0?(Card==0?EPrototypeRole::Depot:EPrototypeRole::Factory):
        (Card==0?EPrototypeRole::Combat:EPrototypeRole::Collector);
    return true;
}
FString ConstructionNotice(const FString& Message)
{
    if(Message.Contains(TEXT("Construction unavailable"))) return TEXT("Bau gesperrt: Einsatz beendet.");
    if(Message.Contains(TEXT("Construction requires"))) return TEXT("Bau gesperrt: Eigener Hub erforderlich.");
    if(Message.Contains(TEXT("Place structures on"))) return TEXT("Gebäude auf freier Landfläche platzieren.");
    if(Message.Contains(TEXT("Site too far"))) return TEXT("Außer Reichweite: Näher am Hub bauen.");
    if(Message.Contains(TEXT("Site obstructed"))) return TEXT("Bauplatz blockiert: Abstand halten.");
    return FString();
}
FString FeedbackNotice(APrototypeGameMode* Mode)
{
    FString Notice=ConstructionNotice(Mode->Feedback);
    if(Notice.IsEmpty()) Notice=Mode->Feedback;
    if(Notice.Contains(TEXT("Not enough credits."))) Notice=TEXT("NACHSCHUB: Mindestens 100 Kredits erforderlich.");
    else if(Notice.Contains(TEXT("Army limit"))) Notice=TEXT("NACHSCHUB: Armeelimit von 40 Fahrzeugen erreicht.");
    else if(Notice.Contains(TEXT("Reinforcement"))) Notice=TEXT("NACHSCHUB: Panzer am eigenen Hub eingetroffen.");
    else if(Notice.Contains(TEXT("Invalid site"))) Notice=TEXT("BAUPLATZ UNGÜLTIG: Nahe am Hub, auf freiem Gelände und frei von Fahrzeugen oder Ressourcenfeldern bauen.");
    else if(Notice.Contains(TEXT("Not enough salvage"))) Notice=TEXT("BAU: Zu wenig Kredits. Sammler müssen Tempest abliefern.");
    else if(Notice.Contains(TEXT("Vehicle factory under"))) Notice=TEXT("BAU: Werkstatt wird errichtet / 12 Sekunden.");
    else if(Notice.Contains(TEXT("Salvage depot under"))) Notice=TEXT("BAU: Raffinerie wird errichtet / 8 Sekunden.");
    else if(Notice.Contains(TEXT("Build a completed"))) Notice=TEXT("PRODUKTION: Fertige Werkstatt erforderlich. Jede Werkstatt nimmt 5 Aufträge an.");
    else if(Notice.Contains(TEXT("HQ queue"))) Notice=TEXT("PRODUKTION: Hub nicht verfügbar oder Warteschlange voll.");
    else if(Notice.Contains(TEXT("Army capacity"))) Notice=TEXT("PRODUKTION: Fahrzeuglimit 40 erreicht, einschließlich wartender Aufträge.");
    else if(Notice.Contains(TEXT("Insufficient credits"))) Notice=TEXT("PRODUKTION: Zu wenig Kredits. Tempest abliefern.");
    else if(Notice.Contains(TEXT("Combat pickup queued"))) Notice=TEXT("PRODUKTION: Panzer bestellt / 5 Sekunden.");
    else if(Notice.Contains(TEXT("Salvage collector queued"))) Notice=TEXT("PRODUKTION: Sammler bestellt / 6 Sekunden.");
    else if(Notice.Contains(TEXT("Exit obstructed"))) Notice=TEXT("AUSFAHRT BLOCKIERT: Fahrzeuge vom Produktionsgebäude wegbewegen.");
    else if(Notice.Contains(TEXT("Selected building cannot"))) Notice=TEXT("PRODUKTION: Werkstatt für Panzer, Hub für Sammler auswählen.");
    else if(Notice.Contains(TEXT("Selected building queue is full"))) Notice=TEXT("PRODUKTION: Warteschlange dieses Gebäudes voll / höchstens 5 Aufträge.");
    else if(Notice.Contains(TEXT("Latest production order cancelled"))) Notice=TEXT("AUFTRAG STORNIERT: Kredits zurückerstattet.");
    else if(Notice.Contains(TEXT("Selected building has no production"))) Notice=TEXT("STORNIERUNG: Dieses Gebäude hat keine wartenden Aufträge.");
    else if(Notice.Contains(TEXT("Rally point rejected"))) Notice=TEXT("SAMMELPUNKT UNGÜLTIG: Freies, erreichbares Gelände auswählen.");
    else if(Notice.Contains(TEXT("Rally point set"))) Notice=TEXT("SAMMELPUNKT GESETZT: Neue Fahrzeuge fahren automatisch dorthin.");
    else if(Notice.Contains(TEXT("Move rejected"))) Notice=TEXT("BEWEGUNG NICHT MÖGLICH: Ziel blockiert oder unerreichbar.");
    return Notice;
}
}

void APrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    auto* Mode=GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    auto* Controller=Cast<APrototypeController>(PlayerOwner);
    if(!Mode||!Controller||!Canvas)return;
    const FPrototypeHUDLayout L(Canvas->SizeX,Canvas->SizeY);
    const float S=L.S,W=L.W,H=L.H;
    const FLinearColor Ink(.004f,.007f,.01f,.97f),Metal(.35f,.42f,.46f,.96f),Dark(.019f,.029f,.036f,.98f);
    const FLinearColor White(.9f,.94f,.96f),Muted(.52f,.61f,.64f),Mint(.39f,.92f,.57f),Cyan(.35f,.85f,.94f),Gold(.98f,.77f,.3f),Red(.96f,.30f,.21f);
    auto Rect=[&](FLinearColor C,float X,float Y,float A,float B){DrawRect(C,X*S,Y*S,A*S,B*S);};
    auto Line=[&](float X,float Y,float A,float B,FLinearColor C,float T=1.f){DrawLine(X*S,Y*S,A*S,B*S,C,T*S);};
    auto Text=[&](const FString& T,float X,float Y,FLinearColor C,float Z=1.f){DrawText(T,FLinearColor(0,0,0,.9f),(X+.7f)*S,(Y+.9f)*S,nullptr,Z*S);DrawText(T,C,X*S,Y*S,nullptr,Z*S);};
    auto Texture=[&](UTexture2D* T,FPrototypeHUDRect R,FLinearColor Tint=FLinearColor::White,float U=0,float V=0,float UW=1,float VH=1){
        if(T)DrawTexture(T,R.X*S,R.Y*S,R.W*S,R.H*S,U,V,UW,VH,Tint,BLEND_Translucent);
    };
    auto Glyph=[&](int32 Index,float X,float Y,float Size,FLinearColor Tint){
        Texture(InterfaceTexture(TEXT("T_UI_GlyphAtlas_V3")),{X,Y,Size,Size},Tint,(Index%4)*.25f,(Index/4)*.25f,.25f,.25f);
    };
    auto NineSlice=[&](UTexture2D* T,FPrototypeHUDRect R,float Corner){
        if(!T)return;
        // Atlas corners stay fixed; only the steel rails stretch with the panel.
        const float X[4]={R.X,R.X+Corner,R.X+R.W-Corner,R.X+R.W};
        const float Y[4]={R.Y,R.Y+Corner,R.Y+R.H-Corner,R.Y+R.H};
        const float UV[4]={0,.125f,.875f,1};
        for(int32 Row=0;Row<3;++Row)for(int32 Column=0;Column<3;++Column)
            Texture(T,{X[Column],Y[Row],X[Column+1]-X[Column],Y[Row+1]-Y[Row]},FLinearColor::White,
                UV[Column],UV[Row],UV[Column+1]-UV[Column],UV[Row+1]-UV[Row]);
    };
    auto Frame=[&](FPrototypeHUDRect R,FLinearColor Accent){
        const float K=12;
        Rect(FLinearColor(0,0,0,.32f),R.X-4,R.Y+7,R.W+8,R.H);
        Rect(Ink,R.X+K,R.Y,R.W-2*K,R.H);
        Rect(Ink,R.X,R.Y+K,R.W,R.H-2*K);
        Texture(InterfaceTexture(TEXT("T_UI_PanelGlass_V3")),{R.X+10,R.Y+10,R.W-20,R.H-20},FLinearColor(1,1,1,.88f));
        for(int I=0;I<5;++I){
            const float X=R.X+I,Y=R.Y+I,A=R.W-2*I,B=R.H-2*I;
            const FLinearColor C=I==0?FLinearColor(.10f,.14f,.17f):I==1?Metal:I==2?FLinearColor(.66f,.71f,.72f,.8f):I==3?Dark:Metal;
            Line(X+K,Y,X+A-K,Y,C);Line(X+A-K,Y,X+A,Y+K,C);
            Line(X+A,Y+K,X+A,Y+B-K,C);Line(X+A,Y+B-K,X+A-K,Y+B,C);
            Line(X+A-K,Y+B,X+K,Y+B,C);Line(X+K,Y+B,X,Y+B-K,C);
            Line(X,Y+B-K,X,Y+K,C);Line(X,Y+K,X+K,Y,C);
        }
        const bool IsMap=FMath::IsNearlyEqual(R.W,L.MiniMap().W)&&FMath::IsNearlyEqual(R.H,L.MiniMap().H);
        NineSlice(InterfaceTexture(IsMap?TEXT("T_UI_MinimapFrame_V3"):TEXT("T_UI_Frame_V3")),R,IsMap?25.f:20.f);
        Texture(InterfaceTexture(TEXT("T_UI_TitlePlate_V3")),{R.X+11,R.Y+8,R.W-22,25},FLinearColor(1,1,1,.55f));
        Line(R.X+14,R.Y+34,R.X+R.W-14,R.Y+34,FLinearColor(.20f,.27f,.30f,.8f),.6f);
        Line(R.X+11,R.Y+39,R.X+11,R.Y+R.H-17,Accent,.6f);
        Rect(Accent,R.X+15,R.Y+11,3,16);
        for(float X:{R.X+7,R.X+R.W-10})for(float Y:{R.Y+9,R.Y+R.H-12}){
            Rect(FLinearColor(0,0,0,.8f),X-1,Y,5,5);Rect(Metal,X,Y,3,3);Line(X,Y+1,X+3,Y+1,Ink,.6f);
        }
    };
    auto Bar=[&](float X,float Y,float A,float Ratio,FLinearColor C){Rect(Ink,X-1,Y-1,A+2,7);Rect(Dark,X,Y,A,5);Rect(C,X,Y,A*FMath::Clamp(Ratio,0.f,1.f),5);Line(X,Y,X+A*FMath::Clamp(Ratio,0.f,1.f),Y,FLinearColor(.85f,.95f,1,.5f),.6f);Line(X,Y+6,X+A,Y+6,Metal,.5f);};
    auto Image=[&](EPrototypeRole R,float X,float Y,float A,float B,float Alpha=1.f){
        if(UTexture2D* T=Portrait(R))DrawTexture(T,X*S,Y*S,A*S,B*S,0,0,1,1,FLinearColor(1,1,1,Alpha),BLEND_Translucent);
    };
    int32 Allies=0,Enemies=0,Collectors=0,Chosen=0;float Health=0,Maximum=0,ArmyHealth=0,ArmyMaximum=0;
    APrototypeUnit* Single=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It){
        if(It->Health<=0)continue;
        if(!It->IsStructure()){if(It->Team==0){++Allies;ArmyHealth+=It->Health;ArmyMaximum+=It->MaxHealth;}else ++Enemies;}
        if(It->Team==0&&It->Role==EPrototypeRole::Collector)++Collectors;
        if(It->bSelected){++Chosen;Single=*It;Health+=It->Health;Maximum+=It->MaxHealth;}
    }
    const auto Resource=L.Resource();Frame(Resource,Cyan);
    Text(TEXT("HERO HOUR"),Resource.X+25,Resource.Y+11,White,1.5f);
    Glyph(12,Resource.X+20,Resource.Y+39,20,Gold);
    Text(TEXT("KREDITS"),Resource.X+47,Resource.Y+45,Muted,.8f);
    Text(FText::AsNumber(FMath::FloorToInt(Mode->Credits)).ToString(),Resource.X+117,Resource.Y+42,Gold,1.12f);
    Line(Resource.X+211,Resource.Y+40,Resource.X+211,Resource.Y+59,Metal);
    Text(FString::Printf(TEXT("ARMEE  %02d / 40"),Allies),Resource.X+226,Resource.Y+44,Cyan,.9f);
    Text(FString::Printf(TEXT("SAMMLER  %02d"),Collectors),Resource.X+378,Resource.Y+44,Mint,.82f);
    const auto Army=L.Army();Frame(Army,Mint);Text(TEXT("ARMEE / HERO"),Army.X+25,Army.Y+12,White,1.f);
    Glyph(3,Army.X+21,Army.Y+39,30,White);Glyph(12,Army.X+131,Army.Y+39,30,White);
    Text(FString::Printf(TEXT("%02d"),Allies-Collectors),Army.X+66,Army.Y+43,White,1.3f);
    Text(FString::Printf(TEXT("%02d"),Collectors),Army.X+176,Army.Y+43,White,1.3f);
    Text(TEXT("ZUSTAND"),Army.X+246,Army.Y+40,Muted,.69f);
    Text(FString::Printf(TEXT("%03d%%"),ArmyMaximum>0?FMath::RoundToInt(100*ArmyHealth/ArmyMaximum):0),Army.X+246,Army.Y+54,Mint,.92f);
    Bar(Army.X+20,Army.Y+73,95,Allies>0?1:0,Mint);Bar(Army.X+131,Army.Y+73,95,Collectors>0?1:0,Cyan);Bar(Army.X+246,Army.Y+73,83,ArmyMaximum>0?ArmyHealth/ArmyMaximum:0,Mint);
    const auto Mission=L.Mission();Frame(Mission,Red);Text(TEXT("EINSATZ / CITY HERO"),Mission.X+25,Mission.Y+12,White,1.06f);
    const int32 Seconds=FMath::FloorToInt(Mode->MatchTime);
    Text(FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60),Mission.X+22,Mission.Y+41,Cyan,1.85f);
    Text(FString::Printf(TEXT("%02d FEINDFAHRZEUGE"),Enemies),Mission.X+121,Mission.Y+51,Muted,.8f);
    Text(GetWorld()->IsPaused()?TEXT("PAUSIERT / F10 FORTSETZEN"):Mode->MatchTime<60&&Mode->Outcome==0?FString::Printf(TEXT("ANGRIFF IN %02d SEK"),FMath::CeilToInt(60-Mode->MatchTime)):TEXT("FEINDLICHEN HUB ZERSTÖREN"),Mission.X+22,Mission.Y+87,GetWorld()->IsPaused()?Cyan:Mode->MatchTime<60?Gold:Red,.85f);

    const auto Mini=L.MiniMap();Frame(Mini,Cyan);Text(TEXT("TAKTISCHE KARTE"),Mini.X+25,Mini.Y+12,White,1.0f);
    const auto MR=MapContent(L);
    auto MapX=[&](float X){return MR.X+FMath::Clamp((X+BattlefieldLayout::ExtentX)/(2*BattlefieldLayout::ExtentX),0.f,1.f)*MR.W;};
    auto MapY=[&](float Y){return MR.Y+FMath::Clamp((Y+BattlefieldLayout::ExtentY)/(2*BattlefieldLayout::ExtentY),0.f,1.f)*MR.H;};
    auto Region=[&](float X,float Y,float A,float B,FLinearColor C){Rect(C,MapX(X),MapY(Y),MapX(A)-MapX(X),MapY(B)-MapY(Y));};
    if(UTexture2D* MapBackground=InterfaceTexture(TEXT("T_UI_CityHeroMap_V3")))Texture(MapBackground,MR);
    else
    {
        Rect(FLinearColor(.21f,.18f,.12f,1),MR.X,MR.Y,MR.W,MR.H);
        Region(1000,-4500,6500,4500,FLinearColor(.18f,.19f,.18f,1));Region(-10000,1000,-5000,6000,FLinearColor(.20f,.26f,.16f,1));Region(7000,2500,10000,5500,FLinearColor(.28f,.20f,.13f,1));
        Region(BattlefieldLayout::RiverX-BattlefieldLayout::RiverHalfWidth,-BattlefieldLayout::ExtentY,BattlefieldLayout::RiverX+BattlefieldLayout::RiverHalfWidth,BattlefieldLayout::ExtentY,FLinearColor(.055f,.24f,.29f,1));
        for(float Y:{-BattlefieldLayout::BridgeOffsetY,BattlefieldLayout::BridgeOffsetY})Region(BattlefieldLayout::RiverX-BattlefieldLayout::RiverHalfWidth,Y-BattlefieldLayout::BridgeHalfWidth,BattlefieldLayout::RiverX+BattlefieldLayout::RiverHalfWidth,Y+BattlefieldLayout::BridgeHalfWidth,FLinearColor(.48f,.46f,.36f,1));
    }
    for(int I=0;I<8;++I){Line(MR.X+MR.W*I/8,MR.Y,MR.X+MR.W*I/8,MR.Y+MR.H,FLinearColor(.5f,.55f,.4f,.12f),.5f);Line(MR.X,MR.Y+MR.H*I/8,MR.X+MR.W,MR.Y+MR.H*I/8,FLinearColor(.5f,.55f,.4f,.12f),.5f);}
    for(const auto& Node:Mode->SalvageNodes)if(Node.Remaining>0)Rect(Gold,MapX(Node.Position.X)-2,MapY(Node.Position.Y)-2,4,4);
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It){if(It->Health<=0)continue;const FVector P=It->GetActorLocation();const float D=It->IsStructure()?7:4;
        if(It->bSelected)Rect(White,MapX(P.X)-D/2-1,MapY(P.Y)-D/2-1,D+2,D+2);Rect(It->Team==0?Mint:Red,MapX(P.X)-D/2,MapY(P.Y)-D/2,D,D);}
    // Ground intersection of the actual four viewport rays forms the camera footprint.
    FVector2D Corners[4];bool ValidFootprint=true;
    for(int I=0;I<4;++I){FVector Origin,Direction;const FVector2D Screen(I==1||I==2?Canvas->SizeX:0,I>=2?Canvas->SizeY:0);
        if(!Controller->DeprojectScreenPositionToWorld(Screen.X,Screen.Y,Origin,Direction)||Direction.Z>=-.001f){ValidFootprint=false;break;}
        const FVector P=Origin+Direction*((35-Origin.Z)/Direction.Z);Corners[I]=FVector2D(MapX(P.X),MapY(P.Y));}
    if(ValidFootprint)for(int I=0;I<4;++I)Line(Corners[I].X,Corners[I].Y,Corners[(I+1)%4].X,Corners[(I+1)%4].Y,Cyan,1.2f);
    Text(TEXT("KLICK: KAMERA VERSCHIEBEN"),Mini.X+16,Mini.Y+261,Cyan,.75f);
    Text(TEXT("HERO"),Mini.X+17,Mini.Y+287,Mint,.8f);Text(TEXT("FEIND"),Mini.X+101,Mini.Y+287,Red,.8f);Text(TEXT("TEMPEST"),Mini.X+195,Mini.Y+287,Gold,.8f);
    const float HQRatio=IsValid(Mode->PlayerHQ)?Mode->PlayerHQ->Health/Mode->PlayerHQ->MaxHealth:0;
    Bar(Mini.X+17,Mini.Y+311,276,HQRatio,Mint);

    auto* Producer=Controller->GetSelectedProducer();
    if(Producer&&Producer->bHasRallyPoint){const float X=MapX(Producer->RallyPoint.X),Y=MapY(Producer->RallyPoint.Y);Line(X-5,Y,X+5,Y,Cyan,2);Line(X,Y-5,X,Y+5,Cyan,2);
        FVector2D P;if(Controller->ProjectWorldLocationToScreen(Producer->RallyPoint+FVector(0,0,100),P)&&!IsOverInterface(P)){Line(P.X/S,P.Y/S-20,P.X/S,P.Y/S+4,Cyan,2);Rect(Cyan,P.X/S+2,P.Y/S-20,15,9);}}
    const auto Selection=L.Selection();Frame(Selection,Cyan);Text(TEXT("AUSGEWÄHLTES OBJEKT"),Selection.X+25,Selection.Y+12,White,1.04f);
    const FPrototypeHUDRect PortraitRect{Selection.X+18,Selection.Y+43,150,163};
    Rect(FLinearColor(.03f,.065f,.08f),PortraitRect.X,PortraitRect.Y,PortraitRect.W,PortraitRect.H);
    for(int32 I=0;I<6;++I)Line(PortraitRect.X,PortraitRect.Y+I*28,PortraitRect.X+PortraitRect.W,PortraitRect.Y+I*28,FLinearColor(.18f,.4f,.46f,.16f),.5f);
    if(Chosen>0&&Single)Image(Single->Role,Selection.X+17,Selection.Y+44,154,154);
    else {Glyph(0,Selection.X+48,Selection.Y+77,88,Cyan);Text(TEXT("HERO KOMMANDO"),Selection.X+30,Selection.Y+181,Muted,.77f);}
    NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),PortraitRect,10);
    const FString Title=Chosen==1&&Single?RoleName(Single->Role):(Chosen>1?FString::Printf(TEXT("EINSATZGRUPPE / %02d"),Chosen):TEXT("KEINE AUSWAHL"));
    Text(Title,Selection.X+182,Selection.Y+47,Cyan,1.04f);
    Text(Chosen==1&&Single?(Single->IsStructure()?TEXT("HERO / BASISGEBÄUDE"):TEXT("HERO / FAHRZEUG")):TEXT("KLICKEN ODER AUSWAHLRAHMEN ZIEHEN"),Selection.X+182,Selection.Y+69,Muted,.8f);
    Text(FString::Printf(TEXT("ZUSTAND   %.0f / %.0f"),Health,Maximum),Selection.X+182,Selection.Y+96,White,.9f);
    Bar(Selection.X+182,Selection.Y+119,369,Maximum>0?Health/Maximum:0,Mint);
    FString Detail=TEXT("SHIFT: AUSWAHL ERWEITERN");
    if(Chosen==1&&Single){
        if(!Single->IsCompleted())Detail=FString::Printf(TEXT("BAU   %d%%"),FMath::RoundToInt(Single->ConstructionProgress*100));
        else if(Single->Role==EPrototypeRole::Collector)Detail=FString::Printf(TEXT("LADUNG   %.0f / 100 TEMPEST"),Single->Cargo);
        else if(Single->IsStructure())Detail=FString::Printf(TEXT("WARTESCHLANGE   %d / 5"),Mode->GetQueueCount(Single));
        else Detail=TEXT("SCHADEN 20   /   FEUERRATE 1 SEK");
    }
    Text(Detail,Selection.X+182,Selection.Y+135,Gold,.88f);
    if(Chosen==1&&Single&&Single->IsStructure())Bar(Selection.X+182,Selection.Y+155,369,Single->IsCompleted()?Mode->GetProductionProgress(Single):Single->ConstructionProgress,Gold);
    const auto Focus=L.Focus();Rect(Chosen>0?FLinearColor(.055f,.14f,.18f):Dark,Focus.X,Focus.Y,Focus.W,Focus.H);
    NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),Focus,8);Glyph(15,Focus.X+8,Focus.Y+7,26,Chosen>0?Cyan:Muted);
    Text(TEXT("F / FOKUS"),Focus.X+40,Focus.Y+13,Chosen>0?White:Muted,.84f);
    Glyph(Producer?9:5,Selection.X+184,Selection.Y+174,22,Cyan);
    Text(Producer?TEXT("RECHTSKLICK: SAMMELPUNKT"):TEXT("RECHTSKLICK: BEFEHL"),Selection.X+212,Selection.Y+177,Cyan,.73f);
    Text(TEXT("STRG + 0–9: GRUPPE"),Selection.X+183,Selection.Y+199,Muted,.73f);

    const auto Production=L.Production();Frame(Production,Cyan);Text(TEXT("BAU / PRODUKTION"),Production.X+25,Production.Y+12,White,1.1f);
    FVector2D Mouse;Controller->GetMousePosition(Mouse.X,Mouse.Y);Mouse/=S;
    const bool Active=Mode->Outcome==0&&IsValid(Mode->PlayerHQ);
    const bool Ready=!Producer||Producer->IsCompleted(),Queue=!Producer||Mode->GetQueueCount(Producer)<5;
    const int32 Category=VisibleCategory(this,Producer);
    const TCHAR* TabNames[5]={TEXT("BASIS"),TEXT("ABWEHR"),TEXT("INF."),TEXT("FAHRZ."),TEXT("SPEZIAL")};
    for(int32 Index=0;Index<5;++Index){const auto Tab=L.ProductionTab(Index);const bool Enabled=CategoryHasContent(Index),Selected=Index==Category;
        Rect(Selected?FLinearColor(.045f,.15f,.19f,.97f):Dark,Tab.X,Tab.Y,Tab.W,Tab.H);
        Glyph(Index,Tab.X+25,Tab.Y+2,25,Enabled?(Selected?Cyan:White):FLinearColor(.29f,.35f,.38f,.55f));
        Text(TabNames[Index],Tab.X+8,Tab.Y+28,Enabled?(Selected?Cyan:Muted):FLinearColor(.3f,.36f,.38f),.62f);
        Line(Tab.X,Tab.Y+Tab.H,Tab.X+Tab.W,Tab.Y+Tab.H,Selected?Cyan:Metal,Selected?2.f:.6f);
        if(!Enabled){Rect(Ink,Tab.X+59,Tab.Y+7,7,8);Line(Tab.X+60,Tab.Y+6,Tab.X+65,Tab.Y+6,Muted,.7f);}
    }
    FString Context=Producer?RoleName(Producer->Role):TEXT("BASISKOMMANDO");
    for(int32 Index=0;Index<5;++Index)if(!CategoryHasContent(Index)&&L.ProductionTab(Index).Contains(Mouse))Context=TEXT("KEINE VERFÜGBAREN EINHEITEN IN DIESER KATEGORIE");
    Text(Context,Production.X+19,Production.Y+88,Muted,.75f);
    const TCHAR* Keys[2]={Category==0?TEXT("V"):TEXT("P"),Category==0?TEXT("B"):TEXT("C")};
    for(int I=0;I<2;++I){const auto T=L.ProductionTile(I,0);EPrototypeRole R;if(!CategoryRole(Category,I,R))continue;
        bool Available=Active&&Mode->Credits>=APrototypeGameMode::RoleCost(R);
        if(Category==3)Available=Available&&Ready&&Queue&&(!Producer||Producer->Role==(R==EPrototypeRole::Combat?EPrototypeRole::Factory:EPrototypeRole::Headquarters))&&Mode->FindCompletedStructure(0,R==EPrototypeRole::Combat?EPrototypeRole::Factory:EPrototypeRole::Headquarters);
        const bool Hover=T.Contains(Mouse);Rect(Hover&&Available?FLinearColor(.055f,.14f,.18f,.97f):Dark,T.X,T.Y,T.W,T.H);
        for(int32 Stripe=0;Stripe<4;++Stripe)Line(T.X+5,T.Y+16+Stripe*16,T.X+T.W-5,T.Y+16+Stripe*16,FLinearColor(.22f,.35f,.40f,.12f),.5f);
        Image(R,T.X+45,T.Y-1,102,102,Available?1.f:.3f);
        Rect(Ink,T.X+4,T.Y+78,T.W-8,27);Line(T.X+6,T.Y+78,T.X+T.W-6,T.Y+78,Metal,.6f);
        NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),T,10);
        Rect(Ink,T.X+9,T.Y+9,22,20);Text(Keys[I],T.X+15,T.Y+11,Available?Cyan:Muted,.94f);
        const FString Label=R==EPrototypeRole::Depot?TEXT("RAFFINERIE"):R==EPrototypeRole::Factory?TEXT("FABRIK"):R==EPrototypeRole::Combat?TEXT("STRIKER"):TEXT("SAMMLER");
        Text(Label,T.X+11,T.Y+84,Available?White:Muted,.81f);
        Text(FString::Printf(TEXT("%.0f"),APrototypeGameMode::RoleCost(R)),T.X+135,T.Y+84,Available?Gold:Muted,.97f);
        Text(FString::Printf(TEXT("%.0f SEK"),APrototypeGameMode::RoleDuration(R)),T.X+125,T.Y+14,Muted,.70f);
        if(Hover&&Available)Line(T.X+10,T.Y+3,T.X+T.W-10,T.Y+3,Cyan,2.f);
    }
    const int32 QueueCount=Producer?Mode->GetQueueCount(Producer):Mode->GetQueueCount();
    Glyph(14,Production.X+19,Production.Y+230,22,Cyan);
    Text(FString::Printf(TEXT("WARTESCHLANGE  %02d%s"),QueueCount,Producer?TEXT(" / 5"):TEXT(" GESAMT")),Production.X+49,Production.Y+235,Cyan,.87f);
    int32 Slot=0;
    for(const FPrototypeProduction& Item:Mode->ProductionQueue){if(!Item.Producer.IsValid()||Item.Producer->Team!=0||(Producer&&Item.Producer.Get()!=Producer))continue;
        const FPrototypeHUDRect QueueSlot{Production.X+20+Slot*76,Production.Y+258,68,40};Rect(Dark,QueueSlot.X,QueueSlot.Y,QueueSlot.W,QueueSlot.H);
        Image(Item.Role,QueueSlot.X+13,QueueSlot.Y-1,42,42);NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),QueueSlot,7);
        if(Slot==0&&Producer)Bar(QueueSlot.X+5,QueueSlot.Y+33,58,Mode->GetProductionProgress(Producer),Gold);
        if(++Slot>=5)break;
    }
    for(;Slot<5;++Slot){const float X=Production.X+20+Slot*76;Rect(FLinearColor(.009f,.017f,.02f,.7f),X,Production.Y+258,68,40);Line(X,Production.Y+258,X+68,Production.Y+258,FLinearColor(.15f,.22f,.25f),.5f);}
    const auto Cancel=L.Cancel();Rect(QueueCount>0&&Producer?FLinearColor(.20f,.08f,.06f,.96f):Dark,Cancel.X,Cancel.Y,Cancel.W,Cancel.H);
    Glyph(10,Cancel.X+8,Cancel.Y+5,18,QueueCount>0&&Producer?Red:Muted);
    Text(TEXT("RÜCKTASTE / LETZTEN AUFTRAG STORNIEREN"),Cancel.X+32,Cancel.Y+8,QueueCount>0&&Producer?White:Muted,.75f);
    Text(Producer?TEXT("RECHTSKLICK IM GELÄNDE: SAMMELPUNKT"):Category==0?TEXT("BAUKARTE WÄHLEN, DANN BAUPLATZ SETZEN"):TEXT("HUB ODER FABRIK FÜR PRODUKTION WÄHLEN"),Production.X+20,Production.Y+350,Muted,.73f);
    const auto Events=L.Events();Frame(Events,Mint);Text(TEXT("EREIGNISSE"),Events.X+25,Events.Y+12,White,.9f);
    FString Notice=FeedbackNotice(Mode);
    // Wrap by measured font width; messages remain within the same panel that owns hit testing.
    if(Notice.IsEmpty())Notice=TEXT("Sammler versorgen die Basis. Fabrik errichten, Striker produzieren.");
    TArray<FString> Words;Notice.ParseIntoArray(Words,TEXT(" "),true);FString Current;int32 Row=0;
    for(const FString& Word:Words){FString Candidate=Current.IsEmpty()?Word:Current+TEXT(" ")+Word;float TW=0,TH=0;GetTextSize(Candidate,TW,TH,nullptr,.72f*S);
        if(TW>(Events.W-40)*S&&!Current.IsEmpty()){Text(Current,Events.X+20,Events.Y+40+Row*16,Gold,.72f);Current=Word;++Row;if(Row>=3)break;}else Current=Candidate;}
    if(Row<3&&!Current.IsEmpty())Text(Current,Events.X+20,Events.Y+40+Row*16,Gold,.72f);

    if(Controller->bPlacing&&Mode->Outcome==0){const auto R=L.Placement();Frame(R,Mint);Text(FString(RoleName(Controller->PlacementRole))+TEXT(" PLATZIEREN"),R.X+25,R.Y+12,Mint,1.f);Text(TEXT("GRÜN: GÜLTIGER BAUPLATZ / ESC: ABBRECHEN"),R.X+20,R.Y+39,White,.82f);}
    Text(TEXT("WASD KAMERA / MAUSRAD ZOOM / DOPPELKLICK TYP WÄHLEN / F10 PAUSE / R NEUSTART"),W*.5f-295,H-15,Muted,.66f);
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It){
        if(It->Health<=0||(!It->bHQ&&!It->bSelected&&It->Health>=It->MaxHealth&&It->IsCompleted()))continue;
        FVector Origin,Extent;It->GetActorBounds(false,Origin,Extent);
        FVector2D P;if(!Controller->ProjectWorldLocationToScreen(FVector(It->GetActorLocation().X,It->GetActorLocation().Y,Origin.Z+Extent.Z+35.f),P)||IsOverInterface(P)||P.Y<0||P.X<0||P.X>Canvas->SizeX||P.Y>Canvas->SizeY)continue;
        const float A=It->IsStructure()?108:46;Rect(Ink,P.X/S-A/2-2,P.Y/S-2,A+4,9);Bar(P.X/S-A/2,P.Y/S,A,It->Health/It->MaxHealth,It->Team==0?Mint:Red);
        if(It->IsStructure()){Text(FString(It->Team==1?TEXT("FEIND / "):TEXT(""))+RoleName(It->Role),P.X/S-A/2,P.Y/S-17,White,.65f);if(!It->IsCompleted())Bar(P.X/S-A/2,P.Y/S+8,A,It->ConstructionProgress,Gold);}
    }
    if(Controller->bDragging){FVector2D P;Controller->GetMousePosition(P.X,P.Y);const FVector2D A=Controller->DragStart;
        DrawRect(FLinearColor(.25f,.7f,.5f,.08f),FMath::Min(A.X,P.X),FMath::Min(A.Y,P.Y),FMath::Abs(A.X-P.X),FMath::Abs(A.Y-P.Y));
        DrawLine(A.X,A.Y,P.X,A.Y,Mint,S);DrawLine(P.X,A.Y,P.X,P.Y,Mint,S);DrawLine(P.X,P.Y,A.X,P.Y,Mint,S);DrawLine(A.X,P.Y,A.X,A.Y,Mint,S);}
    if(Mode->Outcome!=0){const FPrototypeHUDRect R{W*.5f-260,H*.5f-75,520,150};Frame(R,Mode->Outcome>0?Mint:Red);
        Text(Mode->Outcome>0?TEXT("SEKTOR GESICHERT"):TEXT("HUB VERLOREN"),R.X+26,R.Y+25,Mode->Outcome>0?Mint:Red,2.f);
        Text(Mode->Outcome>0?TEXT("Feindliches Kommando ausgeschaltet."):TEXT("Das HERO-Kommando wurde zerstört."),R.X+26,R.Y+76,White,1.f);Text(TEXT("[ R ] NEUEN EINSATZ STARTEN"),R.X+26,R.Y+115,Cyan,1.f);}
}

bool APrototypeHUD::IsOverInterface(const FVector2D& ScreenPosition) const
{
    auto* Controller=Cast<APrototypeController>(PlayerOwner);if(!Controller)return false;
    int32 X=0,Y=0;Controller->GetViewportSize(X,Y);if(X<=0||Y<=0)return false;
    const FPrototypeHUDLayout L(X,Y);
    return L.IsOverPanel(ScreenPosition)||(Controller->bPlacing&&L.Placement().Contains(ScreenPosition/L.S));
}

bool APrototypeHUD::HandleClick(const FVector2D& ScreenPosition)
{
    auto* Mode=GetWorld()->GetAuthGameMode<APrototypeGameMode>();auto* Controller=Cast<APrototypeController>(PlayerOwner);
    if(!Mode||!Controller)return false;int32 X=0,Y=0;Controller->GetViewportSize(X,Y);if(X<=0||Y<=0)return false;
    const FPrototypeHUDLayout L(X,Y);const FVector2D P=ScreenPosition/L.S;const auto Map=MapContent(L);
    if(Controller->bPlacing&&L.Placement().Contains(P))return true;
    if(Map.Contains(P)){Controller->PanToWorld(FVector((P.X-Map.X)/Map.W*2*BattlefieldLayout::ExtentX-BattlefieldLayout::ExtentX,(P.Y-Map.Y)/Map.H*2*BattlefieldLayout::ExtentY-BattlefieldLayout::ExtentY,35));return true;}
    if(L.Focus().Contains(P)){Controller->FocusSelection();return true;}
    for(int32 Category=0;Category<5;++Category)if(L.ProductionTab(Category).Contains(P)){
        if(CategoryHasContent(Category))ProductionCategory=Category;
        return true;
    }
    if(Mode->Outcome==0){
        const int32 Category=VisibleCategory(this,Controller->GetSelectedProducer());
        for(int Card=0;Card<2;++Card)if(L.ProductionTile(Card,0).Contains(P)){
            EPrototypeRole CardRole;if(!CategoryRole(Category,Card,CardRole))return true;
            if(Category==0)Controller->BeginPlacement(CardRole);
            else if(auto* Producer=Controller->GetSelectedProducer())Mode->RequestProductionAt(Producer,CardRole);
            else Mode->RequestProduction(CardRole);return true;}
        if(L.Cancel().Contains(P)){if(auto* Producer=Controller->GetSelectedProducer())Mode->CancelProduction(Producer);return true;}
    }
    return L.IsOverPanel(ScreenPosition);
}
