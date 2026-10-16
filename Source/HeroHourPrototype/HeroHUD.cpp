#include "PrototypeGameMode.h"
#include "PrototypeHUDLayout.h"
#include "Engine/Canvas.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "BattlefieldLayout.h"
#include "RTS/HRRTSDefinitions.h"
#include "InputCoreTypes.h"
#include "UObject/StrongObjectPtr.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
FPrototypeHUDRect MapContent(const FPrototypeHUDLayout& L)
{
    const auto R=L.MiniMap(); return {R.X+20,R.Y+43,R.W-40,(R.W-40)*BattlefieldLayout::ExtentY/BattlefieldLayout::ExtentX};
}
FString RoleName(EPrototypeRole R,int32 Team=0)
{
    if (const UHRUnitDefinition* Definition=APrototypeGameMode::GetRoleDefinition(R,Team))
        return Definition->DisplayName.ToString();
    switch(R) {
    case EPrototypeRole::Headquarters:return TEXT("KOMMANDO-HUB");
    case EPrototypeRole::Factory:return TEXT("FAHRZEUGFABRIK");
    case EPrototypeRole::Depot:return TEXT("RAFFINERIE");
    case EPrototypeRole::Collector:return TEXT("SAMMLER");
    case EPrototypeRole::PowerPlant:return TEXT("KRAFTWERK");
    case EPrototypeRole::Barracks:return TEXT("KASERNE");
    case EPrototypeRole::Radar:return TEXT("RADARZENTRUM");
    case EPrototypeRole::Defense:return TEXT("WACHTURM");
    case EPrototypeRole::Infantry:return TEXT("RANGER");
    case EPrototypeRole::Siege:return TEXT("BELAGERER");
    default:return TEXT("STRIKER / KAMPFFAHRZEUG");
    }
}
UTexture2D* Portrait(EPrototypeRole R,int32 Team=0)
{
    if (const UHRUnitDefinition* Definition=APrototypeGameMode::GetRoleDefinition(R,Team))
        if (UTexture2D* Icon=Definition->Icon.LoadSynchronous()) return Icon;
    if (R>=EPrototypeRole::PowerPlant)return nullptr;
    // LoadObject resolves the same imported package texture on subsequent draws.
    const TCHAR* Path=R==EPrototypeRole::Headquarters?TEXT("/Game/HERO/UI/T_HERO_CommandHub.T_HERO_CommandHub"):
        R==EPrototypeRole::Factory?TEXT("/Game/HERO/UI/T_HERO_VehiclePlant.T_HERO_VehiclePlant"):
        R==EPrototypeRole::Depot?TEXT("/Game/HERO/UI/T_HERO_Refinery.T_HERO_Refinery"):
        R==EPrototypeRole::Collector?TEXT("/Game/HERO/UI/T_HERO_Salvager.T_HERO_Salvager"):TEXT("/Game/HERO/UI/T_HERO_Striker.T_HERO_Striker");
    return LoadObject<UTexture2D>(nullptr,Path);
}
UTexture2D* InterfaceTexture(const TCHAR* Name)
{
    static TMap<FName,TStrongObjectPtr<UTexture2D>> Loaded;
    const FName Key(Name);
    if(const auto* Cached=Loaded.Find(Key);Cached&&Cached->IsValid())return Cached->Get();
    const FString Path=FString::Printf(TEXT("/Game/HERO/UI/ConceptV3/%s.%s"),Name,Name);
    UTexture2D* Texture=LoadObject<UTexture2D>(nullptr,*Path,nullptr,LOAD_NoWarn);
    Loaded.Add(Key,TStrongObjectPtr<UTexture2D>(Texture));
    return Texture;
}
int32 VisibleCategory(const APrototypeHUD* HUD,const APrototypeUnit* Producer)
{
    return HUD->ProductionCategory>=0?HUD->ProductionCategory:
        Producer&&Producer->Role==EPrototypeRole::Barracks?2:Producer&&Producer->Role==EPrototypeRole::Factory?3:0;
}
bool CategoryHasContent(int32 Category) { return Category>=0&&Category<5; }
bool CategoryRole(int32 Category,int32 Card,EPrototypeRole& Role)
{
    return FPrototypeHUDLayout::CategoryCardRole(Category,Card,Role);
}
EPrototypeRole ProducerRole(EPrototypeRole Role)
{
    return Role==EPrototypeRole::Infantry?EPrototypeRole::Barracks:
        Role==EPrototypeRole::Collector?EPrototypeRole::Headquarters:EPrototypeRole::Factory;
}
bool SupportsAbility(const APrototypeUnit* Unit,EHRAbility Ability)
{
    if(!IsValid(Unit)||Unit->Health<=0.f||Unit->IsActorBeingDestroyed()||Unit->Team!=0)return false;
    return Ability==EHRAbility::Grenade ? Unit->Role==EPrototypeRole::Infantry:
        Ability==EHRAbility::Smoke ? APrototypeGameMode::GetFactionIndex(Unit->Team)==0&&Unit->IsCombatant()&&!Unit->IsStructure():
        Unit->Role==EPrototypeRole::Siege;
}
bool HasGrenadeTarget(const APrototypeUnit* Unit,const APrototypeGameMode* Mode)
{
    for(TActorIterator<APrototypeUnit> It(Unit->GetWorld());It;++It)
        if(It->Team!=Unit->Team&&It->Health>0.f&&!It->IsActorBeingDestroyed()&&Mode->CanTeamSee(Unit->Team,*It)
            &&FVector::DistSquared2D(It->GetActorLocation(),Unit->GetActorLocation())<=FMath::Square(800.f))return true;
    return false;
}
EHRDoctrine PlayerDoctrine(int32 Card)
{
    return APrototypeGameMode::GetFactionIndex(0)==0 ? (Card==0?EHRDoctrine::RepairPulse:EHRDoctrine::PrecisionStrike):
        (Card==0?EHRDoctrine::Scorch:EHRDoctrine::Overdrive);
}
FString DoctrineName(EHRDoctrine Doctrine)
{
    switch(Doctrine){case EHRDoctrine::RepairPulse:return TEXT("REPARATURIMPULS");case EHRDoctrine::PrecisionStrike:return TEXT("PRÄZISIONSSCHLAG");
        case EHRDoctrine::Scorch:return TEXT("FLAMMENZONE");default:return TEXT("ÜBERTAKTUNG");}
}
FString RoleDescription(EPrototypeRole Role)
{
    switch(Role)
    {
    case EPrototypeRole::Depot:return TEXT("Sammler entladen im exklusiven Dock. Tempest wird zu Kredits.");
    case EPrototypeRole::Factory:return TEXT("Produziert Kampffahrzeuge und Belagerer. Eigene FIFO mit fünf Plätzen.");
    case EPrototypeRole::PowerPlant:return TEXT("Versorgt das Stromnetz. Unterversorgung bremst Bau und Produktion.");
    case EPrototypeRole::Barracks:return TEXT("Produziert Infanterie mit Granaten, Deckung und Unterdrückung.");
    case EPrototypeRole::Radar:return TEXT("Aktiviert die taktische Karte bei ausreichender Energie.");
    case EPrototypeRole::Defense:return TEXT("Automatische Basisabwehr. Bei Unterversorgung deaktiviert.");
    case EPrototypeRole::Collector:return TEXT("Reserviert eine Tempest-Zelle. 100 Ladung, Raffinerie-FIFO und zeitliches Entladen.");
    case EPrototypeRole::Infantry:return TEXT("G: Granate auf sichtbare Gegner. Gebäudekanten bieten Deckung.");
    case EPrototypeRole::Siege:return TEXT("Y: Stationär entfalten. Größere Reichweite und Panzerung, keine Bewegung.");
    default:return TEXT("Mobiles Kampffahrzeug. T: Rauchwand für AEGIS. Kampferfahrung verbessert Werte.");
    }
}
struct FDoctrineAim { EHRDoctrine Doctrine=EHRDoctrine::RepairPulse;float StartedAt=0.f; };
TMap<TWeakObjectPtr<APrototypeHUD>,FDoctrineAim> DoctrineAims;
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

void CancelPrototypeDoctrineTarget(APrototypeHUD* HUD)
{
    DoctrineAims.Remove(HUD);
}

bool HasPrototypeDoctrineTarget(const APrototypeHUD* HUD)
{
    return DoctrineAims.Contains(const_cast<APrototypeHUD*>(HUD));
}

void BeginPrototypeDoctrineTarget(APrototypeHUD* HUD,EHRDoctrine Doctrine)
{
    if(!IsValid(HUD))return;
    APrototypeGameMode* Mode=HUD->GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if(!Mode||Mode->Outcome!=0||HUD->GetWorld()->IsPaused())return;
    if(!Mode->Commander[0].CanActivate(Doctrine,APrototypeGameMode::GetFactionIndex(0),true))
    {Mode->Feedback=TEXT("Doktrin nicht bereit: Befehlspunkte oder globaler Cooldown fehlen.");return;}
    if(APrototypeController* Controller=Cast<APrototypeController>(HUD->GetOwningPlayerController());Controller&&Controller->bPlacing)
        Controller->ClearSelection();
    FDoctrineAim Aim;Aim.Doctrine=Doctrine;Aim.StartedAt=Mode->MatchTime;
    DoctrineAims.Add(HUD,Aim);
    Mode->Feedback=DoctrineName(Doctrine)+TEXT(": Ziel im Gelände anklicken. ESC / Rechtsklick bricht ab.");
}

void APrototypeHUD::DrawHUD()
{
    Super::DrawHUD();
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeArtReview"))
        && (FParse::Param(FCommandLine::Get(),TEXT("PrototypeOverviewCapture"))
            || FParse::Param(FCommandLine::Get(),TEXT("PrototypeDistrictCapture")))) return;
    auto* Mode=GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    auto* Controller=Cast<APrototypeController>(PlayerOwner);
    if(!Mode||!Controller||!Canvas)return;
    const FPrototypeHUDLayout L(Canvas->SizeX,Canvas->SizeY);
    const float S=L.S,W=L.W,H=L.H;
    FVector2D Mouse;Controller->GetMousePosition(Mouse.X,Mouse.Y);Mouse/=S;
    FString TooltipTitle,TooltipBody,TooltipStats;
    for(auto It=DoctrineAims.CreateIterator();It;++It)if(!It.Key().IsValid())It.RemoveCurrent();
    if(const FDoctrineAim* Aim=DoctrineAims.Find(this);Aim&&(Mode->Outcome!=0||Mode->MatchTime<Aim->StartedAt||Controller->bPlacing))CancelPrototypeDoctrineTarget(this);
    const FLinearColor Ink(.006f,.009f,.009f,.95f),Metal(.24f,.29f,.30f,.92f),Dark(.018f,.025f,.026f,.97f);
    const FLinearColor White(.9f,.94f,.96f),Muted(.52f,.61f,.64f),Mint(.39f,.92f,.57f),Cyan(.35f,.85f,.94f),Gold(.98f,.77f,.3f),Red(.96f,.30f,.21f);
    auto Rect=[&](FLinearColor C,float X,float Y,float A,float B){DrawRect(C,X*S,Y*S,A*S,B*S);};
    auto Line=[&](float X,float Y,float A,float B,FLinearColor C,float T=1.f){DrawLine(X*S,Y*S,A*S,B*S,C,T*S);};
    float FontWidth=0.f,FontHeight=0.f;GetTextSize(TEXT("Ag"),FontWidth,FontHeight,nullptr,1.f);
    auto FontScale=[&](float Z){return L.TextScale(Z,FontHeight);};
    auto CompactFont=[&](float Z,float ReferenceHeight){return FMath::Min(FontScale(Z),FMath::Max(L.MinimumTextLinePixels,ReferenceHeight*S)/FMath::Max(1.f,FontHeight))/S;};
    auto Text=[&](const FString& T,float X,float Y,FLinearColor C,float Z=1.f){DrawText(T,FLinearColor(0,0,0,.9f),(X+.7f)*S,(Y+.9f)*S,nullptr,FontScale(Z));DrawText(T,C,X*S,Y*S,nullptr,FontScale(Z));};
    // Measure the same font and viewport scale used by DrawText. Compact cards
    // keep their full name in the tooltip rather than crossing a neighbour.
    auto FitText=[&](const FString& T,float X,float Y,float Width,FLinearColor C,float Z=1.f){
        const float Limit=FMath::Max(1.f,(Width-.7f)*S),MinimumScale=FontScale(0.f);
        float Scale=FontScale(Z),TW=0.f,TH=0.f;GetTextSize(T,TW,TH,nullptr,Scale);
        if(TW>Limit&&TW>0.f)Scale=FMath::Max(MinimumScale,Scale*Limit/TW);
        FString Visible=T;GetTextSize(Visible,TW,TH,nullptr,Scale);
        if(TW>Limit)
        {
            // Preserve the minimum font size. Full object names remain in the
            // hover tooltip; a narrow label is elided instead of becoming tiny.
            for(int32 End=T.Len()-1;End>=0;--End)
            {
                Visible=T.Left(End)+TEXT("...");GetTextSize(Visible,TW,TH,nullptr,Scale);
                if(TW<=Limit)break;
            }
        }
        Text(Visible,X,Y,C,Scale/S);
    };
    auto Texture=[&](UTexture2D* T,FPrototypeHUDRect R,FLinearColor Tint=FLinearColor::White,float U=0,float V=0,float UW=1,float VH=1){
        if(T)DrawTexture(T,R.X*S,R.Y*S,R.W*S,R.H*S,U,V,UW,VH,Tint,BLEND_Translucent);
    };
    auto Glyph=[&](int32 Index,float X,float Y,float Size,FLinearColor Tint){
        Texture(InterfaceTexture(TEXT("T_UI_GlyphAtlas_V3")),{X,Y,Size,Size},Tint,(Index%4)*.25f,(Index/4)*.25f,.25f,.25f);
    };
    // Fine chamfered steel edges follow the concept; no stretched ornamental frame atlas.
    auto NineSlice=[&](UTexture2D* T,FPrototypeHUDRect R,float Corner){
        (void)T;
        const float K=FMath::Min(4.f,Corner);
        const FLinearColor Edge(.24f,.29f,.30f,.9f),Shade(.025f,.032f,.035f,.95f);
        Line(R.X+K,R.Y,R.X+R.W-K,R.Y,Edge,.8f);
        Line(R.X+R.W-K,R.Y,R.X+R.W,R.Y+K,Edge,.8f);
        Line(R.X+R.W,R.Y+K,R.X+R.W,R.Y+R.H-K,Edge,.8f);
        Line(R.X+R.W,R.Y+R.H-K,R.X+R.W-K,R.Y+R.H,Shade,.8f);
        Line(R.X+R.W-K,R.Y+R.H,R.X+K,R.Y+R.H,Shade,.8f);
        Line(R.X+K,R.Y+R.H,R.X,R.Y+R.H-K,Edge,.8f);
        Line(R.X,R.Y+R.H-K,R.X,R.Y+K,Edge,.8f);
        Line(R.X,R.Y+K,R.X+K,R.Y,Edge,.8f);
    };
    auto Frame=[&](FPrototypeHUDRect R,FLinearColor Accent){
        Rect(FLinearColor(0,0,0,.24f),R.X+2,R.Y+4,R.W,R.H);
        Rect(Ink,R.X,R.Y,R.W,R.H);
        Rect(FLinearColor(.024f,.031f,.032f,.96f),R.X+1,R.Y+1,R.W-2,31);
        NineSlice(nullptr,R,4);
        Line(R.X+7,R.Y+34,R.X+R.W-7,R.Y+34,FLinearColor(.18f,.22f,.23f,.8f),.7f);
        // Small status marker keeps the chrome neutral and reserves color for state.
        Rect(Accent,R.X+12,R.Y+12,2,12);
    };
    auto Bar=[&](float X,float Y,float A,float Ratio,FLinearColor C){Rect(Ink,X-1,Y-1,A+2,7);Rect(Dark,X,Y,A,5);Rect(C,X,Y,A*FMath::Clamp(Ratio,0.f,1.f),5);Line(X,Y,X+A*FMath::Clamp(Ratio,0.f,1.f),Y,FLinearColor(.85f,.95f,1,.5f),.6f);Line(X,Y+6,X+A,Y+6,Metal,.5f);};
    auto Image=[&](EPrototypeRole R,float X,float Y,float A,float B,float Alpha=1.f,int32 Team=0){
        if(UTexture2D* T=Portrait(R,Team))DrawTexture(T,X*S,Y*S,A*S,B*S,0,0,1,1,FLinearColor(1,1,1,Alpha),BLEND_Translucent);
        else Glyph(static_cast<int32>(R)%16,X+A*.22f,Y+B*.12f,FMath::Min(A,B)*.6f,FLinearColor(.35f,.85f,.94f,Alpha));
    };
    int32 Allies=0,Enemies=0,Collectors=0,Infantry=0,Chosen=0;float Health=0,Maximum=0;
    APrototypeUnit* Single=nullptr;
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It){
        if(It->Health<=0||It->IsActorBeingDestroyed())continue;
        if(!It->IsStructure()){if(It->Team==0){++Allies;if(It->Role==EPrototypeRole::Infantry)++Infantry;}else if(Mode->CanTeamSee(0,*It))++Enemies;}
        if(It->Team==0&&It->Role==EPrototypeRole::Collector)++Collectors;
        if(It->bSelected&&It->Team==0){++Chosen;Single=*It;Health+=It->Health;Maximum+=It->MaxHealth;}
    }
    const auto Resource=L.Resource();Rect(Ink,Resource.X,Resource.Y,Resource.W,Resource.H);NineSlice(nullptr,Resource,4);
    Text(TEXT("HERO HOUR"),Resource.X+8,Resource.Y+3,White,1.4f);
    Line(Resource.X+7,Resource.Y+29,Resource.X+Resource.W-7,Resource.Y+29,Metal,.7f);
    const FHRPowerGridState& Power=Mode->PowerGrid[0];
    const FLinearColor PowerColor=Power.GetState()==EHRPowerState::Online?Mint:Power.GetState()==EHRPowerState::LowPower?Gold:Red;
    Glyph(13,Resource.X+8,Resource.Y+34,18,PowerColor);
    FitText(FString::Printf(TEXT("%d / %d"),Power.CurrentPower,Power.RequiredPower),Resource.X+30,Resource.Y+37,109,PowerColor,.95f);
    Text(TEXT("KREDITS"),Resource.X+151,Resource.Y+38,Muted,.75f);
    FitText(FText::AsNumber(FMath::FloorToInt(Mode->Credits)).ToString(),Resource.X+218,Resource.Y+36,105,Gold,1.05f);
    Text(FString::Printf(TEXT("ARMEE %02d / 40"),Allies),Resource.X+335,Resource.Y+37,Cyan,.86f);
    Text(Mode->HasRadar(0)?TEXT("RADAR AN"):TEXT("RADAR AUS"),Resource.X+468,Resource.Y+39,Mode->HasRadar(0)?Cyan:Muted,.64f);
    const UHRFactionDefinition* PlayerFaction=Mode->GetFactionDefinition(0);
    const auto Army=L.Army();Frame(Army,Mint);Text(TEXT("ARMEE / ")+(PlayerFaction?PlayerFaction->DisplayName.ToString():TEXT("HERO")),Army.X+25,Army.Y+12,White,.95f);
    const int32 ArmyCounts[]={Infantry,Allies-Collectors-Infantry,Collectors};
    const int32 ArmyGlyphs[]={2,3,12};
    for(int32 Index=0;Index<3;++Index){const float AX=Army.X+18+Index*101;
        Glyph(ArmyGlyphs[Index],AX,Army.Y+39,25,Muted);
        Text(FString::Printf(TEXT("%02d"),ArmyCounts[Index]),AX+35,Army.Y+41,White,1.12f);
        Bar(AX,Army.Y+69,83,ArmyCounts[Index]>0?1:0,Index==0?Mint:Index==1?Cyan:Gold);
    }
    const auto Mission=L.Mission();Frame(Mission,Cyan);
    const int32 Seconds=FMath::FloorToInt(Mode->MatchTime);
    Text(TEXT("COMMANDER"),Mission.X+25,Mission.Y+12,White,1.f);
    Text(FString::Printf(TEXT("%02d:%02d"),Seconds/60,Seconds%60),Mission.X+245,Mission.Y+12,Muted,.96f);
    Text(FString::Printf(TEXT("BEFEHLSPUNKTE %03d / 100"),FMath::FloorToInt(Mode->Commander[0].Points)),Mission.X+15,Mission.Y+35,Gold,.75f);
    for(int32 Index=0;Index<2;++Index){const auto D=L.DoctrineCard(Index);const EHRDoctrine Doctrine=PlayerDoctrine(Index);
        const bool Available=Mode->Outcome==0&&!GetWorld()->IsPaused()&&Mode->Commander[0].CanActivate(Doctrine,APrototypeGameMode::GetFactionIndex(0),true);
        Rect(Dark,D.X,D.Y,D.W,D.H);NineSlice(nullptr,D,4);
        Glyph(Index==0?7:11,D.X+8,D.Y+8,32,Available?Cyan:Muted);
        Text(Index==0?TEXT("Z"):TEXT("X"),D.X+118,D.Y+8,Available?White:Muted,.8f);
        const FString Name=DoctrineName(Doctrine);
        FitText(Name,D.X+7,D.Y+42,D.W-14,Available?White:Muted,.72f);
        Text(Mode->Commander[0].GlobalCooldown>0?FString::Printf(TEXT("%d SEK"),FMath::CeilToInt(Mode->Commander[0].GlobalCooldown)):Available?TEXT("BEREIT"):FString::Printf(TEXT("%.0f PUNKTE"),FHRCommanderState::Cost(Doctrine)),D.X+7,D.Y+61,Available?Mint:Gold,.72f);
        if(D.Contains(Mouse)){TooltipTitle=Name;TooltipStats=FString::Printf(TEXT("%.0f PUNKTE / GEMEINSAMER COOLDOWN %.0f SEK"),FHRCommanderState::Cost(Doctrine),Mode->Commander[0].GlobalCooldown);TooltipBody=TEXT("Klicken, danach ein gültiges Ziel im Gelände bestätigen. Punkte werden erst beim gültigen Ziel verbraucht.");}
    }
    FitText(GetWorld()->IsPaused()?TEXT("PAUSIERT / F10 FORTSETZEN"):Mode->MatchTime<60&&Mode->Outcome==0?FString::Printf(TEXT("ANGRIFF IN %02d SEK"),FMath::CeilToInt(60-Mode->MatchTime)):FString::Printf(TEXT("KONTAKTE %02d / HUB ZERSTÖREN"),Enemies),Mission.X+15,Mission.Y+143,Mission.W-30,GetWorld()->IsPaused()?Cyan:Gold,.76f);

    const auto Mini=L.MiniMap();Frame(Mini,Cyan);Text(TEXT("TAKTISCHE KARTE"),Mini.X+25,Mini.Y+12,White,1.0f);
    const auto MR=MapContent(L);
    auto MapX=[&](float X){return MR.X+FMath::Clamp((X+BattlefieldLayout::ExtentX)/(2*BattlefieldLayout::ExtentX),0.f,1.f)*MR.W;};
    auto MapY=[&](float Y){return MR.Y+FMath::Clamp((Y+BattlefieldLayout::ExtentY)/(2*BattlefieldLayout::ExtentY),0.f,1.f)*MR.H;};
    auto Region=[&](float X,float Y,float A,float B,FLinearColor C){Rect(C,MapX(X),MapY(Y),MapX(A)-MapX(X),MapY(B)-MapY(Y));};
    if(Mode->HasRadar(0))
    {
    if(UTexture2D* MapBackground=InterfaceTexture(TEXT("T_UI_CityHeroMap_V3")))Texture(MapBackground,MR);
    else
    {
        Rect(FLinearColor(.21f,.18f,.12f,1),MR.X,MR.Y,MR.W,MR.H);
        Region(1000,-4500,6500,4500,FLinearColor(.18f,.19f,.18f,1));Region(-10000,1000,-5000,6000,FLinearColor(.20f,.26f,.16f,1));Region(7000,2500,10000,5500,FLinearColor(.28f,.20f,.13f,1));
        Region(BattlefieldLayout::RiverX-BattlefieldLayout::RiverHalfWidth,-BattlefieldLayout::ExtentY,BattlefieldLayout::RiverX+BattlefieldLayout::RiverHalfWidth,BattlefieldLayout::ExtentY,FLinearColor(.055f,.24f,.29f,1));
        for(float Y:{-BattlefieldLayout::BridgeOffsetY,BattlefieldLayout::BridgeOffsetY})Region(BattlefieldLayout::RiverX-BattlefieldLayout::RiverHalfWidth,Y-BattlefieldLayout::BridgeHalfWidth,BattlefieldLayout::RiverX+BattlefieldLayout::RiverHalfWidth,Y+BattlefieldLayout::BridgeHalfWidth,FLinearColor(.48f,.46f,.36f,1));
    }
    for(int I=0;I<8;++I){Line(MR.X+MR.W*I/8,MR.Y,MR.X+MR.W*I/8,MR.Y+MR.H,FLinearColor(.5f,.55f,.4f,.12f),.5f);Line(MR.X,MR.Y+MR.H*I/8,MR.X+MR.W,MR.Y+MR.H*I/8,FLinearColor(.5f,.55f,.4f,.12f),.5f);}
    if(Mode->FogTexture)Texture(Mode->FogTexture,MR);
    else if(Mode->Visibility.GetWidth()==0)Rect(FLinearColor::Black,MR.X,MR.Y,MR.W,MR.H);
    else for(int32 Y=0;Y<Mode->Visibility.GetHeight();++Y)for(int32 X=0;X<Mode->Visibility.GetWidth();++X)
    {
        const FVector Center=Mode->Visibility.GetCellCenter(X,Y);const float Half=Mode->Visibility.GetCellSize()*.5f;
        Region(Center.X-Half,Center.Y-Half,Center.X+Half,Center.Y+Half,FLinearColor(0,0,0,Mode->Visibility.GetFogAlpha(0,Center)));
    }
    for(const auto& Node:Mode->SalvageNodes)for(int32 CellIndex=0;CellIndex<Node.Field.Num();++CellIndex)
    {
        const FVector Position=Node.CellPosition(CellIndex);if(!Mode->Visibility.IsExplored(0,Position))continue;
        const bool Visible=Mode->Visibility.IsVisible(0,Position);
        if(Visible&&Node.Field.GetAmount(CellIndex)<=0.f&&Node.Field.GetStyle()==EHREconomyStyle::FiniteSupply)continue;
        Rect(Visible?Gold:FLinearColor(.98f,.77f,.3f,.35f),MapX(Position.X)-.8f,MapY(Position.Y)-.8f,1.6f,1.6f);
    }
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It){if(It->Health<=0||It->IsActorBeingDestroyed()||!Mode->CanTeamSee(0,*It))continue;const FVector P=It->GetActorLocation();const float D=It->IsStructure()?7:4;
        if(It->bSelected)Rect(White,MapX(P.X)-D/2-1,MapY(P.Y)-D/2-1,D+2,D+2);Rect(It->Team==0?Mint:Red,MapX(P.X)-D/2,MapY(P.Y)-D/2,D,D);}
    // Ground intersection of the actual four viewport rays forms the camera footprint.
    FVector2D Corners[4];bool ValidFootprint=true;
    for(int I=0;I<4;++I){FVector Origin,Direction;const FVector2D Screen(I==1||I==2?Canvas->SizeX:0,I>=2?Canvas->SizeY:0);
        if(!Controller->DeprojectScreenPositionToWorld(Screen.X,Screen.Y,Origin,Direction)||Direction.Z>=-.001f){ValidFootprint=false;break;}
        const FVector P=Origin+Direction*((35-Origin.Z)/Direction.Z);Corners[I]=FVector2D(MapX(P.X),MapY(P.Y));}
    if(ValidFootprint)for(int I=0;I<4;++I)Line(Corners[I].X,Corners[I].Y,Corners[(I+1)%4].X,Corners[(I+1)%4].Y,Cyan,1.2f);
    }
    else
    {
        Rect(FLinearColor(.008f,.016f,.021f,1),MR.X,MR.Y,MR.W,MR.H);
        for(int32 Scan=0;Scan<15;++Scan)Line(MR.X+8,MR.Y+10+Scan*12,MR.X+MR.W-8,MR.Y+10+Scan*12,FLinearColor(.08f,.14f,.17f,.35f),.5f);
        Glyph(14,MR.X+MR.W*.5f-32,MR.Y+41,64,Muted);
        Text(TEXT("RADAR OFFLINE"),MR.X+60,MR.Y+120,Muted,1.05f);
        Text(Mode->IsPowered(0)?TEXT("RADARZENTRUM ERRICHTEN"):TEXT("STROMVERSORGUNG HERSTELLEN"),MR.X+25,MR.Y+151,PowerColor,.72f);
    }
    FitText(Mode->HasRadar(0)?TEXT("KLICK: KAMERA VERSCHIEBEN"):TEXT("RADAR: STROM + RADARZENTRUM"),Mini.X+16,Mini.Y+261,Mini.W-32,Mode->HasRadar(0)?Cyan:Muted,.78f);
    Text(PlayerFaction?PlayerFaction->DisplayName.ToString():TEXT("HERO"),Mini.X+17,Mini.Y+287,Mint,.8f);Text(TEXT("FEIND"),Mini.X+101,Mini.Y+287,Red,.8f);Text(TEXT("TEMPEST"),Mini.X+195,Mini.Y+287,Gold,.8f);
    const float HQRatio=IsValid(Mode->PlayerHQ)?Mode->PlayerHQ->Health/Mode->PlayerHQ->MaxHealth:0;
    Bar(Mini.X+17,Mini.Y+311,276,HQRatio,Mint);

    auto* Producer=Controller->GetSelectedProducer();
    if(Producer&&Producer->bHasRallyPoint){const float X=MapX(Producer->RallyPoint.X),Y=MapY(Producer->RallyPoint.Y);if(Mode->HasRadar(0)){Line(X-5,Y,X+5,Y,Cyan,2);Line(X,Y-5,X,Y+5,Cyan,2);}
        FVector2D P;if(Controller->ProjectWorldLocationToScreen(Producer->RallyPoint+FVector(0,0,100),P)&&!IsOverInterface(P)){Line(P.X/S,P.Y/S-20,P.X/S,P.Y/S+4,Cyan,2);Rect(Cyan,P.X/S+2,P.Y/S-20,15,9);}}
    const auto Selection=L.Selection();Frame(Selection,Cyan);Text(TEXT("AUSGEWÄHLTES OBJEKT"),Selection.X+25,Selection.Y+12,White,1.04f);
    const FPrototypeHUDRect PortraitRect{Selection.X+18,Selection.Y+43,150,163};
    Rect(FLinearColor(.03f,.065f,.08f),PortraitRect.X,PortraitRect.Y,PortraitRect.W,PortraitRect.H);
    Rect(FLinearColor(.018f,.028f,.031f),PortraitRect.X+1,PortraitRect.Y+1,PortraitRect.W-2,PortraitRect.H-2);
    if(Chosen>0&&Single)Image(Single->Role,PortraitRect.X,PortraitRect.Y+1,PortraitRect.W,PortraitRect.W,1.f,Single->Team);
    else {Glyph(0,Selection.X+48,Selection.Y+77,88,Cyan);Text(TEXT("HERO KOMMANDO"),Selection.X+30,Selection.Y+181,Muted,.77f);}
    NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),PortraitRect,10);
    if(Chosen>0&&Single&&PortraitRect.Contains(Mouse))
    {
        TooltipTitle=RoleName(Single->Role,Single->Team);
        TooltipBody=TEXT("STRG + 0–9: Gruppe speichern. 0–9: Gruppe wählen. SHIFT erweitert die Auswahl. Rechtsklick im Gelände: Befehl; bei Produktionsgebäuden Sammelpunkt.");
        TooltipStats=TEXT("GRUPPENSTEUERUNG / RECHTSKLICK-BEFEHLE");
    }
    const FString Title=Chosen==1&&Single?RoleName(Single->Role,Single->Team):(Chosen>1?FString::Printf(TEXT("EINSATZGRUPPE / %02d"),Chosen):TEXT("KEINE AUSWAHL"));
    FitText(Title,Selection.X+182,Selection.Y+47,369,Cyan,1.04f);
    FString UnitFamily=TEXT("KLICKEN ODER AUSWAHLRAHMEN ZIEHEN");
    if(Chosen==1&&Single)
    {
        const UHRFactionDefinition* Faction=Mode->GetFactionDefinition(Single->Team);
        UnitFamily=(Faction?Faction->DisplayName.ToString():TEXT("HERO"))+TEXT(" / ")+
            (Single->IsStructure()?TEXT("BASIS"):Single->Role==EPrototypeRole::Infantry?TEXT("INFANTERIE"):TEXT("FAHRZEUG"));
    }
    FitText(UnitFamily,Selection.X+182,Selection.Y+69,369,Muted,.75f);
    FitText(FString::Printf(TEXT("ZUSTAND   %.0f / %.0f"),Health,Maximum),Selection.X+182,Selection.Y+89,369,White,.85f);
    Bar(Selection.X+182,Selection.Y+112,369,Maximum>0?Health/Maximum:0,Mint);
    FString Detail=TEXT("SHIFT: AUSWAHL ERWEITERN");
    if(Chosen==1&&Single){
        if(!Single->IsCompleted())Detail=FString::Printf(TEXT("BAU   %d%%"),FMath::RoundToInt(Single->ConstructionProgress*100));
        else if(Single->Role==EPrototypeRole::Collector)
        {
            const TCHAR* State=TEXT("FELDSUCHE");
            switch(Single->CollectorState)
            {
            case EPrototypeCollectorState::Harvesting: State=TEXT("ABBAU"); break;
            case EPrototypeCollectorState::MovingToRefinery: State=TEXT("RÜCKFAHRT"); break;
            case EPrototypeCollectorState::WaitingForDock: State=TEXT("DOCK WARTEN"); break;
            case EPrototypeCollectorState::Unloading: State=TEXT("ENTLADEN"); break;
            default: break;
            }
            Detail=FString::Printf(TEXT("%.0f / 100 TEMPEST | %s | ZELLE %d"),Single->Cargo,State,Single->HarvestCell==INDEX_NONE?0:Single->HarvestCell+1);
        }
        else if(Single->IsStructure())Detail=FString::Printf(TEXT("WARTESCHLANGE   %d / 5"),Mode->GetQueueCount(Single));
        else Detail=FString::Printf(TEXT("SCHADEN %.0f  REICHW. %.1fm  RANG %d"),Single->GetEffectiveDamage(),Single->GetEffectiveRange()/100.f,Single->CombatState.Rank);
    }
    if(!(Chosen==1&&Single&&Single->IsCombatant()))FitText(Detail,Selection.X+182,Selection.Y+126,369,Gold,.78f);
    if(Chosen==1&&Single&&Single->IsStructure())
    {
        Bar(Selection.X+182,Selection.Y+149,369,Single->IsCompleted()?Mode->GetProductionProgress(Single):Single->ConstructionProgress,Gold);
        Text(Single->bManuallyPoweredDown?TEXT("MANUELL AUS"):Mode->IsUnitOperational(Single)?TEXT("BETRIEBSBEREIT"):TEXT("STROM / BAU FEHLT"),Selection.X+182,Selection.Y+170,Single->bManuallyPoweredDown?Red:PowerColor,.78f);
    }
    else if(Chosen==1&&Single&&Single->IsCombatant())
    {
        Text(TEXT("SCHADEN"),Selection.X+182,Selection.Y+126,Muted,.69f);
        Text(FString::Printf(TEXT("%.0f"),Single->GetEffectiveDamage()),Selection.X+262,Selection.Y+125,White,.86f);
        Text(TEXT("REICHWEITE"),Selection.X+345,Selection.Y+126,Muted,.69f);
        Text(FString::Printf(TEXT("%.1f m"),Single->GetEffectiveRange()/100.f),Selection.X+457,Selection.Y+125,White,.86f);
        Text(TEXT("PANZERUNG"),Selection.X+182,Selection.Y+145,Muted,.69f);
        Text(FString::Printf(TEXT("x%.2f"),Single->GetEffectiveArmor()),Selection.X+277,Selection.Y+144,White,.86f);
        Text(TEXT("TEMPO"),Selection.X+345,Selection.Y+145,Muted,.69f);
        Text(FString::Printf(TEXT("%.1f m/s"),Single->GetEffectiveSpeed()/100.f),Selection.X+431,Selection.Y+144,White,.86f);
        Text(FString::Printf(TEXT("RANG %d / XP %.0f"),Single->CombatState.Rank,Single->CombatState.Experience),Selection.X+182,Selection.Y+164,Gold,.71f);
        const FString Status=Single->CombatState.IsSuppressed()?TEXT("UNTERDRÜCKT"):Single->CombatState.bDeployed?TEXT("STATIONÄR ENTFALTET"):Single->IsInCover()?TEXT("IN DECKUNG / -30% SCHADEN"):TEXT("KEINE DECKUNG");
        Text(Status,Selection.X+182,Selection.Y+183,Single->CombatState.IsSuppressed()?Red:Single->IsInCover()?Mint:Muted,.65f);
        if(Single->Role==EPrototypeRole::Infantry)Bar(Selection.X+182,Selection.Y+203,225,Single->CombatState.Suppression/100.f,Single->CombatState.IsSuppressed()?Red:Gold);
    }
    const auto Focus=L.Focus();Rect(Chosen>0?FLinearColor(.055f,.14f,.18f):Dark,Focus.X,Focus.Y,Focus.W,Focus.H);
    NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),Focus,8);Glyph(15,Focus.X+8,Focus.Y+7,26,Chosen>0?Cyan:Muted);
    Text(TEXT("F / FOKUS"),Focus.X+40,Focus.Y+13,Chosen>0?White:Muted,.84f);
    if(Chosen==1&&Single&&Single->IsStructure())
    {
        const auto Toggle=L.PowerToggle(),Sell=L.Sell();
        const bool PowerAvailable=Mode->Outcome==0&&!GetWorld()->IsPaused()&&Single->IsCompleted();
        const bool SellAvailable=Mode->Outcome==0&&!GetWorld()->IsPaused()&&!Single->bHQ;
        for(int32 Action=0;Action<2;++Action)
        {
            const auto Button=Action==0?Toggle:Sell;const bool Enabled=Action==0?PowerAvailable:SellAvailable;
            Rect(Enabled?FLinearColor(.055f,.14f,.18f):Dark,Button.X,Button.Y,Button.W,Button.H);NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),Button,8);
            Glyph(Action==0?13:10,Button.X+7,Button.Y+6,25,Enabled?(Action==0?Cyan:Gold):Muted);
            FitText(Action==0?(Single->bManuallyPoweredDown?TEXT("K / EIN"):TEXT("K / AUS")):TEXT("DEL / VERK."),Button.X+36,Button.Y+13,Button.W-42,Enabled?White:Muted,.76f);
            if(Button.Contains(Mouse))
            {
                TooltipTitle=Action==0?TEXT("STROM UMSCHALTEN"):TEXT("GEBÄUDE VERKAUFEN");
                TooltipBody=Action==0?TEXT("Abgeschaltete Gebäude verbrauchen und liefern keinen Strom. Produktion, Sicht, Radar und Abwehr des Gebäudes ruhen."):
                    TEXT("50% des Gebäudepreises und sämtliche bezahlten Warteschlangenaufträge werden erstattet. Der Hub kann nicht verkauft werden.");
                TooltipStats=Action==0?(Single->bManuallyPoweredDown?TEXT("MANUELL ABGESCHALTET"):TEXT("MANUELL EINGESCHALTET")):FString::Printf(TEXT("BASIS-ERSTATTUNG  %.0f KREDITS"),APrototypeGameMode::RoleCost(Single->Role)*.5f);
            }
        }
    }
    else for(int32 AbilityIndex=0;AbilityIndex<3;++AbilityIndex)
    {
        const auto Button=L.Ability(AbilityIndex);const EHRAbility Ability=static_cast<EHRAbility>(AbilityIndex);
        bool Supported=false,Enabled=false;float Cooldown=TNumericLimits<float>::Max();
        for(APrototypeUnit* Unit:Controller->Selected)if(SupportsAbility(Unit,Ability))
        {
            Supported=true;Cooldown=FMath::Min(Cooldown,Unit->CombatState.GetCooldown(Ability));
            Enabled|=!GetWorld()->IsPaused()&&Mode->Outcome==0&&Unit->CombatState.CanActivate(Ability,true,Mode->IsUnitOperational(Unit),Ability!=EHRAbility::Grenade||HasGrenadeTarget(Unit,Mode));
        }
        Rect(Enabled?FLinearColor(.055f,.14f,.18f):Dark,Button.X,Button.Y,Button.W,Button.H);NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),Button,8);
        const TCHAR* AbilityNames[]={TEXT("G / WURF"),TEXT("T / RAUCH"),TEXT("Y / MODUS")};
        Glyph(AbilityIndex==0?11:AbilityIndex==1?5:8,Button.X+6,Button.Y+7,23,Enabled?Cyan:Muted);
        FitText(Supported&&Cooldown>0.f?FString::Printf(TEXT("%d SEK"),FMath::CeilToInt(Cooldown)):AbilityNames[AbilityIndex],Button.X+33,Button.Y+13,Button.W-39,Enabled?White:Muted,.76f);
        if(Button.Contains(Mouse))
        {
            TooltipTitle=AbilityIndex==0?TEXT("G / GRANATENWURF"):AbilityNames[AbilityIndex];
            TooltipBody=AbilityIndex==0?TEXT("Infanterie wirft eine Granate auf den nächsten sichtbaren Gegner innerhalb von 8 Metern. Flächenschaden, 12 Sekunden Cooldown."):
                AbilityIndex==1?TEXT("AEGIS-Kampfeinheiten erzeugen 6 Sekunden Rauch. Gegner verlieren Sicht durch die Wolke, eigene Sicht bleibt frei. 18 Sekunden Cooldown."):
                TEXT("Belagerer wechseln in den stationären Modus: 1,5-fache Reichweite und Panzerung. Eine Sekunde Umschaltzeit. Bewegung erst nach dem Einklappen.");
            TooltipStats=!Supported?TEXT("KEINE PASSENDE EINHEIT AUSGEWÄHLT"):Cooldown>0.f?FString::Printf(TEXT("COOLDOWN  %d SEK"),FMath::CeilToInt(Cooldown)):Enabled?TEXT("BEREIT"):TEXT("SICHTBARES ZIEL / BETRIEB ERFORDERLICH");
        }
    }

    const auto Production=L.Production();Frame(Production,Cyan);Text(TEXT("BAU / PRODUKTION"),Production.X+25,Production.Y+12,White,1.1f);
    const bool Active=Mode->Outcome==0&&IsValid(Mode->PlayerHQ)&&Mode->PlayerHQ->Health>0.f&&!GetWorld()->IsPaused();
    const bool Ready=!Producer||Mode->IsUnitOperational(Producer),Queue=!Producer||Mode->GetQueueCount(Producer)<5;
    const int32 Category=VisibleCategory(this,Producer);
    const TCHAR* TabNames[5]={TEXT("BASIS"),TEXT("ABWEHR"),TEXT("INF."),TEXT("FAHRZ."),TEXT("SPEZIAL")};
    for(int32 Index=0;Index<5;++Index){const auto Tab=L.ProductionTab(Index);const bool Enabled=CategoryHasContent(Index),Selected=Index==Category;
        Rect(Selected?FLinearColor(.045f,.15f,.19f,.97f):Dark,Tab.X,Tab.Y,Tab.W,Tab.H);
        Glyph(Index,Tab.X+28,Tab.Y+2,18,Enabled?(Selected?Cyan:Muted):FLinearColor(.29f,.35f,.38f,.55f));
        FitText(TabNames[Index],Tab.X+5,Tab.Y+23,Tab.W-10,Enabled?(Selected?Cyan:Muted):FLinearColor(.3f,.36f,.38f),CompactFont(.78f,14.f));
        Line(Tab.X,Tab.Y+Tab.H,Tab.X+Tab.W,Tab.Y+Tab.H,Selected?Cyan:Metal,Selected?2.f:.6f);
        if(!Enabled){Rect(Ink,Tab.X+59,Tab.Y+7,7,8);Line(Tab.X+60,Tab.Y+6,Tab.X+65,Tab.Y+6,Muted,.7f);}
    }
    FString Context=Category==4?FString::Printf(TEXT("%d BEFEHLSPUNKTE / GLOBALER COOLDOWN %.0fs"),FMath::FloorToInt(Mode->Commander[0].Points),Mode->Commander[0].GlobalCooldown):Producer?RoleName(Producer->Role):TEXT("BASISKOMMANDO");
    FitText(Context,Production.X+19,Production.Y+86,Production.W-38,Muted,.75f);
    for(int I=0;I<FPrototypeHUDLayout::CategoryCardCount(Category);++I){const auto T=L.ProductionCard(I);
        if(Category==4)
        {
            const EHRDoctrine Doctrine=PlayerDoctrine(I);const float Cost=FHRCommanderState::Cost(Doctrine);
            const bool Available=Active&&Mode->Commander[0].CanActivate(Doctrine,APrototypeGameMode::GetFactionIndex(0),true);
            const bool Hover=T.Contains(Mouse);
            Rect(Hover&&Available?FLinearColor(.055f,.14f,.18f,.97f):Dark,T.X,T.Y,T.W,T.H);
            Glyph(I==0?7:11,T.X+18,T.Y+8,36,Available?Cyan:Muted);NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),T,10);
            Text(I==0?TEXT("Z"):TEXT("X"),T.X+9,T.Y+9,Available?Cyan:Muted,.82f);
            const FString Label=DoctrineName(Doctrine);
            const FString ShortDoctrine=Doctrine==EHRDoctrine::RepairPulse?TEXT("REPAR."):Doctrine==EHRDoctrine::PrecisionStrike?TEXT("PRÄZIS."):Doctrine==EHRDoctrine::Scorch?TEXT("BRAND"):TEXT("TEMPO");
            FitText(ShortDoctrine,T.X+5,T.Y+52,T.W-10,Available?White:Muted,.78f);
            Text(FString::Printf(TEXT("%.0f BP"),Cost),T.X+5,T.Y+75,Available?Gold:Muted,CompactFont(.92f,14.f));
            if(Hover)
            {
                TooltipTitle=Label;TooltipStats=FString::Printf(TEXT("%.0f PUNKTE / 20 SEK GLOBALER COOLDOWN"),Cost);
                TooltipBody=Doctrine==EHRDoctrine::RepairPulse?TEXT("Heilt eigene beschädigte Einheiten und Gebäude im Zielgebiet. Erst ein gültiger Geländeklick verbraucht Punkte."):
                    Doctrine==EHRDoctrine::PrecisionStrike?TEXT("Explosiver Präzisionsschlag auf sichtbare Feinde. Ziele im Kriegsnebel werden abgewiesen, ohne Punkte zu verbrauchen."):
                    Doctrine==EHRDoctrine::Scorch?TEXT("Tempest-Flächenschaden auf sichtbare Gegner. Einen Bereich im Gelände anklicken; Kriegsnebel verbirgt gültige Ziele."):
                    TEXT("Eigene mobile Kampfeinheiten im Zielgebiet erhalten für acht Sekunden höheren Schaden und höheres Tempo.");
            }
            continue;
        }
        EPrototypeRole R;if(!CategoryRole(Category,I,R))continue;
        bool Available=Active&&Mode->Credits>=APrototypeGameMode::RoleCost(R);
        APrototypeUnit* RequiredProducer=nullptr;
        if(Category==2||Category==3)
        {
            RequiredProducer=Producer?Producer:Mode->FindCompletedStructure(0,ProducerRole(R));
            Available=Available&&Ready&&Queue&&RequiredProducer&&RequiredProducer->Role==ProducerRole(R)
                &&Mode->IsUnitOperational(RequiredProducer)&&Mode->GetQueueCount(RequiredProducer)<5;
        }
        const bool Hover=T.Contains(Mouse);Rect(Hover&&Available?FLinearColor(.055f,.14f,.18f,.97f):Dark,T.X,T.Y,T.W,T.H);
        Rect(FLinearColor(.027f,.038f,.041f,.9f),T.X+1,T.Y+1,T.W-2,55);
        Image(R,T.X+10,T.Y+1,54,54,Available?1.f:.38f);
        Rect(Ink,T.X+3,T.Y+56,T.W-6,36);Line(T.X+4,T.Y+56,T.X+T.W-4,T.Y+56,Metal,.6f);
        NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),T,10);
        const TCHAR* Key=R==EPrototypeRole::Depot?TEXT("V"):R==EPrototypeRole::Factory?TEXT("B"):R==EPrototypeRole::Combat?TEXT("P"):R==EPrototypeRole::Collector?TEXT("C"):TEXT("");
        if(*Key){Rect(Ink,T.X+3,T.Y+3,16,16);Text(Key,T.X+7,T.Y+5,Available?Cyan:Muted,.7f);}
        const FString Label=R==EPrototypeRole::Depot?TEXT("RAFFIN."):R==EPrototypeRole::Factory?TEXT("FABRIK"):R==EPrototypeRole::PowerPlant?TEXT("STROM"):R==EPrototypeRole::Barracks?TEXT("KASERNE"):R==EPrototypeRole::Radar?TEXT("RADAR"):R==EPrototypeRole::Defense?TEXT("ABWEHR"):R==EPrototypeRole::Collector?TEXT("SAMMLER"):R==EPrototypeRole::Infantry?TEXT("INFANT."):R==EPrototypeRole::Siege?TEXT("BELAG."):TEXT("PANZER");
        FitText(Label,T.X+5,T.Y+57,T.W-10,Available?White:Muted,CompactFont(.78f,14.f));
        Text(FString::Printf(TEXT("%.0f"),APrototypeGameMode::RoleCost(R)),T.X+5,T.Y+76,Available?Gold:Muted,CompactFont(.94f,14.f));

        if(Hover&&Available)Line(T.X+10,T.Y+3,T.X+T.W-10,T.Y+3,Cyan,2.f);
        if(Hover)
        {
            const UHRUnitDefinition* Definition=Mode->GetRoleDefinition(R,0);
            TooltipTitle=RoleName(R);TooltipBody=RoleDescription(R);
            TooltipStats=FString::Printf(TEXT("%.0f KREDITS / %.0f SEK%s"),APrototypeGameMode::RoleCost(R),APrototypeGameMode::RoleDuration(R),Available?TEXT(""):TEXT(" / NICHT VERFÜGBAR"));
            if(const UHRBuildingDefinition* Building=Cast<UHRBuildingDefinition>(Definition))
                TooltipStats+=FString::Printf(TEXT(" / E +%d -%d"),Building->PowerSupply,Building->PowerDemand);
            if((Category==2||Category==3)&&(!RequiredProducer||RequiredProducer->Role!=ProducerRole(R)))
                TooltipBody=RoleName(ProducerRole(R))+TEXT(" auswählen oder errichten. ")+TooltipBody;
            else if(Mode->Credits<APrototypeGameMode::RoleCost(R))TooltipBody=TEXT("Zu wenig Kredits. Sammler zur Raffinerie zurückkehren lassen. ")+TooltipBody;
            else if(!Queue)TooltipBody=TEXT("Die Warteschlange des ausgewählten Gebäudes ist voll. ")+TooltipBody;
        }
    }
    const int32 QueueCount=Producer?Mode->GetQueueCount(Producer):Mode->GetQueueCount();
    Glyph(14,Production.X+19,Production.Y+215,18,Cyan);
    Text(FString::Printf(TEXT("WARTESCHLANGE  %02d%s"),QueueCount,Producer?TEXT(" / 5"):TEXT(" GESAMT")),Production.X+49,Production.Y+218,Cyan,.74f);
    int32 Slot=0;
    for(const FPrototypeProduction& Item:Mode->ProductionQueue){if(!Item.Producer.IsValid()||Item.Producer->Team!=0||(Producer&&Item.Producer.Get()!=Producer))continue;
        const FPrototypeHUDRect QueueSlot=L.QueueSlot(Slot);Rect(Dark,QueueSlot.X,QueueSlot.Y,QueueSlot.W,QueueSlot.H);
        Image(Item.Role,QueueSlot.X+13,QueueSlot.Y-1,42,42);NineSlice(InterfaceTexture(TEXT("T_UI_Frame_V3")),QueueSlot,7);
        if(Slot==0)Bar(QueueSlot.X+5,QueueSlot.Y+33,58,Item.Progress/FMath::Max(.01f,Item.Duration),Gold);
        if(QueueSlot.Contains(Mouse)){TooltipTitle=RoleName(Item.Role,0);TooltipStats=FString::Printf(TEXT("FORTSCHRITT %.0f%% / BEZAHLT %.0f"),100.f*FMath::Clamp(Item.Progress/FMath::Max(.01f,Item.Duration),0.f,1.f),Item.PaidCost);
            TooltipBody=TEXT("Aufträge laufen pro Gebäude in Reihenfolge. Die Stornierung entfernt den letzten bezahlten Auftrag und erstattet seinen tatsächlichen Preis.");}
        if(++Slot>=5)break;
    }
    for(;Slot<5;++Slot){const auto Empty=L.QueueSlot(Slot);Rect(FLinearColor(.009f,.017f,.02f,.7f),Empty.X,Empty.Y,Empty.W,Empty.H);Line(Empty.X,Empty.Y,Empty.X+Empty.W,Empty.Y,FLinearColor(.15f,.22f,.25f),.5f);}
    const auto Cancel=L.Cancel();Rect(QueueCount>0&&Producer?FLinearColor(.20f,.08f,.06f,.96f):Dark,Cancel.X,Cancel.Y,Cancel.W,Cancel.H);
    Glyph(10,Cancel.X+8,Cancel.Y+5,18,QueueCount>0&&Producer?Red:Muted);
    FitText(TEXT("RÜCKTASTE / LETZTEN AUFTRAG STORNIEREN"),Cancel.X+32,Cancel.Y+8,Cancel.W-42,QueueCount>0&&Producer?White:Muted,.75f);
    FitText(Producer?TEXT("RECHTSKLICK: SAMMELPUNKT"):Category<=1?TEXT("BAUKARTE / Q: DREHEN"):Category==4?TEXT("DOKTRIN / DANN ZIEL WÄHLEN"):TEXT("PRODUKTIONSGEBÄUDE WÄHLEN"),Production.X+20,Production.Y+321,Production.W-40,Muted,.78f);
    const auto Events=L.Events();Frame(Events,Mint);Text(TEXT("EREIGNISSE"),Events.X+25,Events.Y+12,White,.9f);
    FString Notice=FeedbackNotice(Mode);
    // Wrap by measured font width; messages remain within the same panel that owns hit testing.
    if(Notice.IsEmpty())Notice=TEXT("Sammler versorgen die Basis. Fabrik errichten, Striker produzieren.");
    TArray<FString> Words;Notice.ParseIntoArray(Words,TEXT(" "),true);FString Current;int32 Row=0;
    for(const FString& Word:Words){FString Candidate=Current.IsEmpty()?Word:Current+TEXT(" ")+Word;float TW=0,TH=0;GetTextSize(Candidate,TW,TH,nullptr,FontScale(.72f));
        if(TW>(Events.W-40)*S&&!Current.IsEmpty()){Text(Current,Events.X+20,Events.Y+40+Row*18.5f,Gold,.72f);Current=Word;++Row;if(Row>=3)break;}else Current=Candidate;}
    if(Row<3&&!Current.IsEmpty())Text(Current,Events.X+20,Events.Y+40+Row*18.5f,Gold,.72f);

    if(Controller->bPlacing&&Mode->Outcome==0){const auto R=L.Placement();Frame(R,Mint);Text(FString(RoleName(Controller->PlacementRole))+TEXT(" PLATZIEREN"),R.X+25,R.Y+12,Mint,1.f);Text(FString::Printf(TEXT("GRÜN: GÜLTIG / Q: %d° DREHUNG / ESC: ABBRUCH"),Controller->PlacementRotation*90),R.X+20,R.Y+39,White,.8f);}
    if(const FDoctrineAim* Aim=DoctrineAims.Find(this))
    {
        const auto R=L.Placement();Frame(R,Cyan);Text(DoctrineName(Aim->Doctrine)+TEXT(" / ZIEL WÄHLEN"),R.X+25,R.Y+12,Cyan,.94f);
        FitText(TEXT("ZIELKLICK: BESTÄTIGEN / ESC ODER RECHTSKLICK: ABBRUCH"),R.X+20,R.Y+39,R.W-40,White,.78f);
        if(!IsOverInterface(Mouse*S)){Line(Mouse.X-11,Mouse.Y,Mouse.X+11,Mouse.Y,Cyan,1.5f);Line(Mouse.X,Mouse.Y-11,Mouse.X,Mouse.Y+11,Cyan,1.5f);}
    }
    const FString CameraHelp=TEXT("WASD: KAMERA / MAUSRAD: ZOOM / F10: PAUSE");
    float HelpWidth=0.f,HelpHeight=0.f;GetTextSize(CameraHelp,HelpWidth,HelpHeight,nullptr,FontScale(.78f));
    const float HelpX=(W-HelpWidth/S)*.5f,HelpY=H-(HelpHeight+2.f)/S;
    Rect(FLinearColor(.006f,.009f,.009f,.88f),HelpX-6.f/S,HelpY-1.f/S,(HelpWidth+12.f)/S,(HelpHeight+3.f)/S);
    Text(CameraHelp,HelpX,HelpY,White,.78f);
    for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It){
        if(It->Health<=0||It->IsActorBeingDestroyed()||!Mode->CanTeamSee(0,*It)||(!It->bHQ&&!It->bSelected&&It->Health>=It->MaxHealth&&It->IsCompleted()))continue;
        FVector Origin,Extent;It->GetActorBounds(false,Origin,Extent);
        FVector2D P;if(!Controller->ProjectWorldLocationToScreen(FVector(It->GetActorLocation().X,It->GetActorLocation().Y,Origin.Z+Extent.Z+35.f),P)||IsOverInterface(P)||P.Y<0||P.X<0||P.X>Canvas->SizeX||P.Y>Canvas->SizeY)continue;
        const float A=It->IsStructure()?108:46;Rect(Ink,P.X/S-A/2-2,P.Y/S-2,A+4,9);Bar(P.X/S-A/2,P.Y/S,A,It->Health/It->MaxHealth,It->Team==0?Mint:Red);
        if(It->IsStructure()){Text(FString(It->Team==1?TEXT("FEIND / "):TEXT(""))+RoleName(It->Role,It->Team),P.X/S-A/2,P.Y/S-(FontScale(.65f)*FontHeight+4.f)/S,White,.65f);if(!It->IsCompleted())Bar(P.X/S-A/2,P.Y/S+8,A,It->ConstructionProgress,Gold);}
        else if(It->bSelected&&It->IsCombatant()&&It->CombatState.Rank>0)Text(FString::Printf(TEXT("RANG %d"),It->CombatState.Rank),P.X/S-A/2,P.Y/S-(FontScale(.58f)*FontHeight+4.f)/S,Gold,.58f);
    }
    if(Controller->bDragging){FVector2D P;Controller->GetMousePosition(P.X,P.Y);const FVector2D A=Controller->DragStart;
        DrawRect(FLinearColor(.25f,.7f,.5f,.08f),FMath::Min(A.X,P.X),FMath::Min(A.Y,P.Y),FMath::Abs(A.X-P.X),FMath::Abs(A.Y-P.Y));
        DrawLine(A.X,A.Y,P.X,A.Y,Mint,S);DrawLine(P.X,A.Y,P.X,P.Y,Mint,S);DrawLine(P.X,P.Y,A.X,P.Y,Mint,S);DrawLine(A.X,P.Y,A.X,A.Y,Mint,S);}
    if(!TooltipTitle.IsEmpty()&&!DoctrineAims.Contains(this))
    {
        const FPrototypeHUDRect R{Production.X-372,FMath::Clamp(static_cast<float>(Mouse.Y)-195.f,122.f,H-208.f),358,186};
        Frame(R,Cyan);FitText(TooltipTitle,R.X+25,R.Y+12,R.W-48,White,.88f);
        auto Wrapped=[&](const FString& Message,float Y,FLinearColor Color,float Font,int32 MaximumRows)
        {
            TArray<FString> Parts;Message.ParseIntoArray(Parts,TEXT(" "),true);FString Pending;int32 LineIndex=0;
            for(const FString& Part:Parts)
            {
                const FString Candidate=Pending.IsEmpty()?Part:Pending+TEXT(" ")+Part;float TextWidth=0.f,TextHeight=0.f;GetTextSize(Candidate,TextWidth,TextHeight,nullptr,FontScale(Font));
                if(TextWidth>(R.W-40)*S&&!Pending.IsEmpty())
                {
                    Text(Pending,R.X+20,Y+LineIndex*18.5f,Color,Font);Pending=Part;
                    if(++LineIndex>=MaximumRows)return;
                }
                else Pending=Candidate;
            }
            if(LineIndex<MaximumRows&&!Pending.IsEmpty())Text(Pending,R.X+20,Y+LineIndex*18.5f,Color,Font);
        };
        Wrapped(TooltipStats,R.Y+43,Gold,.68f,2);Wrapped(TooltipBody,R.Y+84,Muted,.73f,5);
    }
    if(Mode->Outcome!=0){const FPrototypeHUDRect R{W*.5f-260,H*.5f-75,520,150};Frame(R,Mode->Outcome>0?Mint:Red);
        Text(Mode->Outcome>0?TEXT("SEKTOR GESICHERT"):TEXT("HUB VERLOREN"),R.X+26,R.Y+25,Mode->Outcome>0?Mint:Red,2.f);
        Text(Mode->Outcome>0?TEXT("Feindliches Kommando ausgeschaltet."):TEXT("Das HERO-Kommando wurde zerstört."),R.X+26,R.Y+76,White,1.f);Text(TEXT("[ R ] NEUEN EINSATZ STARTEN"),R.X+26,R.Y+115,Cyan,1.f);}
}

bool APrototypeHUD::IsOverInterface(const FVector2D& ScreenPosition) const
{
    auto* Controller=Cast<APrototypeController>(PlayerOwner);if(!Controller)return false;
    int32 X=0,Y=0;Controller->GetViewportSize(X,Y);if(X<=0||Y<=0)return false;
    const FPrototypeHUDLayout L(X,Y);
    return L.IsOverPanel(ScreenPosition)||((Controller->bPlacing||DoctrineAims.Contains(const_cast<APrototypeHUD*>(this)))&&L.Placement().Contains(ScreenPosition/L.S));
}

bool APrototypeHUD::HandleClick(const FVector2D& ScreenPosition)
{
    auto* Mode=GetWorld()->GetAuthGameMode<APrototypeGameMode>();auto* Controller=Cast<APrototypeController>(PlayerOwner);
    if(!Mode||!Controller)return false;int32 X=0,Y=0;Controller->GetViewportSize(X,Y);if(X<=0||Y<=0)return false;
    const FPrototypeHUDLayout L(X,Y);const FVector2D P=ScreenPosition/L.S;const auto Map=MapContent(L);
    if((Controller->bPlacing||DoctrineAims.Contains(this))&&L.Placement().Contains(P))return true;
    if(Map.Contains(P))
    {
        if(Mode->HasRadar(0))Controller->PanToWorld(FVector((P.X-Map.X)/Map.W*2*BattlefieldLayout::ExtentX-BattlefieldLayout::ExtentX,(P.Y-Map.Y)/Map.H*2*BattlefieldLayout::ExtentY-BattlefieldLayout::ExtentY,35));
        return true;
    }
    if(L.Focus().Contains(P)){Controller->FocusSelection();return true;}
    for(int32 Category=0;Category<5;++Category)if(L.ProductionTab(Category).Contains(P)){
        if(CategoryHasContent(Category))ProductionCategory=Category;
        return true;
    }
    if(Mode->Outcome==0&&!GetWorld()->IsPaused()){
        for(int32 Index=0;Index<2;++Index)if(L.DoctrineCard(Index).Contains(P)){BeginPrototypeDoctrineTarget(this,PlayerDoctrine(Index));return true;}
        if(const FDoctrineAim* Aim=DoctrineAims.Find(this);Aim&&!L.IsOverPanel(ScreenPosition))
        {
            const EHRDoctrine Doctrine=Aim->Doctrine;FVector Origin,Direction;
            if(Controller->DeprojectScreenPositionToWorld(ScreenPosition.X,ScreenPosition.Y,Origin,Direction)&&Direction.Z<-.001f)
            {
                const FVector Target=Origin+Direction*((35.f-Origin.Z)/Direction.Z);
                if(Mode->ActivateDoctrine(Doctrine,0,Target))CancelPrototypeDoctrineTarget(this);
                else Mode->Feedback=TEXT("Doktrin-Ziel ungültig: passenden eigenen Verband oder sichtbare Gegner anklicken. Keine Punkte verbraucht.");
            }
            return true;
        }
        APrototypeUnit* SelectedStructure=nullptr;
        if(Controller->Selected.Num()==1)
        {
            APrototypeUnit* Unit=Controller->Selected[0];
            if(IsValid(Unit)&&Unit->Team==0&&Unit->Health>0.f&&!Unit->IsActorBeingDestroyed()&&Unit->IsStructure())SelectedStructure=Unit;
        }
        if(SelectedStructure)
        {
            if(L.PowerToggle().Contains(P)){CancelPrototypeDoctrineTarget(this);Mode->TogglePower(SelectedStructure);return true;}
            if(L.Sell().Contains(P)){CancelPrototypeDoctrineTarget(this);Mode->SellBuilding(SelectedStructure);return true;}
        }
        else for(int32 AbilityIndex=0;AbilityIndex<3;++AbilityIndex)if(L.Ability(AbilityIndex).Contains(P))
        {
            CancelPrototypeDoctrineTarget(this);const EHRAbility Ability=static_cast<EHRAbility>(AbilityIndex);bool Activated=false;
            const TArray<APrototypeUnit*> SelectionCopy=Controller->Selected;
            for(APrototypeUnit* Unit:SelectionCopy)if(SupportsAbility(Unit,Ability))Activated|=Unit->TryActivateAbility(Ability);
            if(!Activated)Mode->Feedback=TEXT("Fähigkeit nicht bereit: Einheitentyp, Cooldown und sichtbare Ziele prüfen.");
            return true;
        }
        const int32 Category=VisibleCategory(this,Controller->GetSelectedProducer());
        for(int Card=0;Card<FPrototypeHUDLayout::CategoryCardCount(Category);++Card)if(L.ProductionCard(Card).Contains(P)){
            if(Category==4){BeginPrototypeDoctrineTarget(this,PlayerDoctrine(Card));return true;}
            EPrototypeRole CardRole;if(!CategoryRole(Category,Card,CardRole))return true;
            CancelPrototypeDoctrineTarget(this);
            if(Category==0||Category==1)Controller->BeginPlacement(CardRole);
            else if(auto* Producer=Controller->GetSelectedProducer())Mode->RequestProductionAt(Producer,CardRole);
            else Mode->RequestProduction(CardRole);return true;}
        if(L.Cancel().Contains(P)){if(auto* Producer=Controller->GetSelectedProducer())Mode->CancelProduction(Producer);return true;}
    }
    return L.IsOverPanel(ScreenPosition);
}
