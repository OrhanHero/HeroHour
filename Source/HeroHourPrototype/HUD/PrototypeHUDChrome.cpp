#include "PrototypeHUDChrome.h"

#include "CanvasTypes.h"
#include "Engine/Canvas.h"
#include "GameFramework/HUD.h"

namespace HeroHUDChrome
{
namespace
{
    void Bevel(AHUD* HUD, float S, const FPrototypeHUDRect& R, float K,
        FLinearColor Light, FLinearColor Dark, float Thickness)
    {
        const float X=R.X, Y=R.Y, W=R.W, H=R.H;
        auto Line=[&](float A,float B,float C,float D,FLinearColor Tint)
        { HUD->DrawLine(A*S,B*S,C*S,D*S,Tint,FMath::Max(.7f,Thickness*S)); };
        Line(X+K,Y,X+W-K,Y,Light);
        Line(X+W-K,Y,X+W,Y+K,Light);
        Line(X+W,Y+K,X+W,Y+H-K,Dark);
        Line(X+W,Y+H-K,X+W-K,Y+H,Dark);
        Line(X+W-K,Y+H,X+K,Y+H,Dark);
        Line(X+K,Y+H,X,Y+H-K,Dark);
        Line(X,Y+H-K,X,Y+K,Light);
        Line(X,Y+K,X+K,Y,Light);
    }
}

void DrawSteelFrame(AHUD* HUD, const FPrototypeHUDRect& R, float S,
    FLinearColor Accent, bool bHeavy, bool bFill)
{
    if(!HUD || S<=0.f || R.W<12.f || R.H<12.f) return;
    auto Fill=[&](FLinearColor C,float X,float Y,float W,float H)
    { HUD->DrawRect(C,X*S,Y*S,W*S,H*S); };
    const float Rim=bHeavy?5.f:2.f;
    if(bFill)
    {
        Fill(FLinearColor(0,0,0,.6f),R.X+3,R.Y+5,R.W,R.H);
        Fill(FLinearColor(.008f,.009f,.011f,.97f),R.X,R.Y,R.W,R.H);
        Fill(FLinearColor(.016f,.019f,.024f,.98f),R.X+Rim,R.Y+Rim,R.W-2*Rim,R.H-2*Rim);
    }
    // Fine inset cells must not read as bright freestanding silver buttons.
    // Heavy radar/sidebar frames retain the third structural bevel.
    Bevel(HUD,S,R,bHeavy?6.f:3.f,
        bHeavy?FLinearColor(.25f,.27f,.29f):FLinearColor(.16f,.18f,.20f),
        FLinearColor(.045f,.052f,.062f),bHeavy?1.2f:.8f);
    Bevel(HUD,S,{R.X+2,R.Y+2,R.W-4,R.H-4},bHeavy?4.f:2.f,
        FLinearColor(.055f,.065f,.078f),
        bHeavy?FLinearColor(.23f,.25f,.27f):FLinearColor(.115f,.13f,.15f),.75f);
    if(bHeavy)
        Bevel(HUD,S,{R.X+Rim+1,R.Y+Rim+1,R.W-2*Rim-2,R.H-2*Rim-2},2,
            FLinearColor(.01f,.012f,.016f),FLinearColor(.12f,.135f,.15f),.7f);
    // Broad, restrained highlights: structural steel with a warm state stripe.
    const float Stripe=FMath::Min(R.W*.24f,68.f);
    Fill(bHeavy?Accent:Accent*.65f,R.X+12,R.Y+2,Stripe,bHeavy?1.5f:1.f);
    Fill(Accent*.4f,R.X+R.W-12-Stripe,R.Y+R.H-3,Stripe,1.f);
    if(bHeavy)
    {
        Fill(FLinearColor(.13f,.14f,.15f),R.X+8,R.Y+7,R.W-16,2);
        Fill(FLinearColor(.005f,.006f,.008f),R.X+8,R.Y+10,R.W-16,1);
        for(int32 I=0;I<3;++I)
        {
            const float X=R.X+R.W*.5f-8+I*7;
            HUD->DrawLine(X*S,(R.Y+R.H-4)*S,(X+3)*S,(R.Y+R.H-1)*S,Accent,S);
        }
    }
}

void DrawTabFill(AHUD* HUD, const FPrototypeHUDRect& R, float S,
    FLinearColor Accent, bool bSelected, bool bEnabled)
{
    if(!HUD || S<=0.f) return;
    const FLinearColor Background=bSelected?FLinearColor(.29f,.21f,.075f,.97f):
        bEnabled?FLinearColor(.045f,.043f,.037f,.98f):FLinearColor(.022f,.025f,.028f,.98f);
    HUD->DrawRect(Background,R.X*S,R.Y*S,R.W*S,R.H*S);
    const FLinearColor Light=bSelected?FLinearColor(.9f,.73f,.34f):FLinearColor(.29f,.29f,.27f);
    Bevel(HUD,S,R,2,Light,FLinearColor(.09f,.09f,.085f),1.f);
    if(bSelected)
    {
        HUD->DrawRect(FLinearColor(.48f,.33f,.10f,.95f),(R.X+3)*S,(R.Y+3)*S,(R.W-6)*S,FMath::Max(1.f,2*S));
        HUD->DrawRect(Accent,(R.X+2)*S,(R.Y+R.H-3)*S,(R.W-4)*S,FMath::Max(1.f,2*S));
    }
}

void DrawVerticalLabel(AHUD* HUD, UCanvas* Canvas, const FPrototypeHUDRect& R,
    float S, const FString& Label, FLinearColor Color, float ReferenceFontScale)
{
    if(!HUD || !Canvas || !Canvas->Canvas || S<=0.f || Label.IsEmpty()) return;
    float FW=0,FH=0;
    HUD->GetTextSize(TEXT("Ag"),FW,FH,nullptr,1.f);
    const float FontScale=FMath::Max(ReferenceFontScale*S,12.f/FMath::Max(1.f,FH));
    FString Visible=Label;
    float TW=0,TH=0;
    HUD->GetTextSize(Visible,TW,TH,nullptr,FontScale);
    const float Available=FMath::Max(1.f,R.H*S-10.f);
    while(TW>Available && Visible.Len()>1)
    {
        Visible.LeftChopInline(1);
        HUD->GetTextSize(Visible+TEXT("..."),TW,TH,nullptr,FontScale);
    }
    if(Visible!=Label) Visible+=TEXT("...");
    HUD->GetTextSize(Visible,TW,TH,nullptr,FontScale);
    // Row-vector matrix: local +X becomes screen -Y, local +Y becomes +X.
    // This rotates bottom-to-top and centers the measured glyph run in the tab.
    const float TX=(R.X+R.W*.5f)*S-TH*.5f;
    const float TY=(R.Y+R.H*.5f)*S+TW*.5f;
    const FMatrix Rotation(FPlane(0,-1,0,0),FPlane(1,0,0,0),
        FPlane(0,0,1,0),FPlane(TX,TY,0,1));
    Canvas->Canvas->PushRelativeTransform(Rotation);
    HUD->DrawText(Visible,FLinearColor(0,0,0,.9f),.8f,.8f,nullptr,FontScale);
    HUD->DrawText(Visible,Color,0,0,nullptr,FontScale);
    Canvas->Canvas->PopTransform();
}
}
