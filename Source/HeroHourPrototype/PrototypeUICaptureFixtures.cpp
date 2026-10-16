#include "PrototypeUICaptureFixtures.h"
#include "PrototypeGameMode.h"
#include "BattlefieldLayout.h"
#include "EngineUtils.h"
#include "Engine/World.h"

bool PreparePrototypeUICaptureFixture(APrototypeGameMode* Mode,APrototypeController* Controller,const FString& State)
{
    if(!IsValid(Mode)||!IsValid(Controller)||!Mode->GetWorld())return false;
    const bool RadarState=State.Equals(TEXT("Radar"),ESearchCase::IgnoreCase);
    const bool LowPowerState=State.Equals(TEXT("LowPower"),ESearchCase::IgnoreCase);
    const bool MultiState=State.Equals(TEXT("Multi"),ESearchCase::IgnoreCase);
    const bool ProductionState=State.Equals(TEXT("Production"),ESearchCase::IgnoreCase);
    const bool TooltipState=State.Equals(TEXT("Tooltip"),ESearchCase::IgnoreCase);
    const bool PauseState=State.Equals(TEXT("Pause"),ESearchCase::IgnoreCase);
    const bool VictoryState=State.Equals(TEXT("Victory"),ESearchCase::IgnoreCase);
    if(!RadarState&&!LowPowerState&&!MultiState&&!ProductionState&&!TooltipState&&!PauseState&&!VictoryState)
    {
        UE_LOG(LogTemp,Error,TEXT("UI_CAPTURE_FIXTURE state=%s unnatural_fixture=1 result=FAIL reason=unknown_state"),*State);
        return false;
    }
    auto* HUD=Cast<APrototypeHUD>(Controller->GetHUD());
    if(!HUD)return false;
    Controller->ClearSelection();HUD->bDoctrineView=false;HUD->ProductionCategory=0;
    FVector HQ=IsValid(Mode->PlayerHQ)?Mode->PlayerHQ->GetActorLocation():FVector(BattlefieldLayout::PlayerHQX,0,35);
    HQ.Z=35.f;
    auto SpawnRole=[&](EPrototypeRole Role,const FVector& Position)
    {
        APrototypeUnit* Unit=Mode->SpawnUnit(0,Position);
        if(Unit){Unit->ConfigureRole(Role);Unit->ConstructionProgress=1.f;}
        return Unit;
    };
    auto Select=[&](APrototypeUnit* Unit)
    {
        if(IsValid(Unit)){Controller->Selected.AddUnique(Unit);Unit->Select(true);}
    };
    bool Ready=true;
    if(ProductionState)
    {
        APrototypeUnit* Factory=SpawnRole(EPrototypeRole::Factory,FVector(-7400,-1600,35));
        Mode->Credits=1600.f;
        if(Factory)
        {
            Select(Factory);HUD->ProductionCategory=3;
            for(int32 Order=0;Order<3;++Order)Ready&=Mode->RequestProductionAt(Factory,EPrototypeRole::Combat);
        }
        else Ready=false;
        UE_LOG(LogTemp,Display,TEXT("UI_CAPTURE_FIXTURE state=Production unnatural_fixture=1 explicit_wallet_seed=1600 real_paid_orders=3 result=%s"),Ready?TEXT("PASS"):TEXT("FAIL"));
    }
    else
    {
        APrototypeUnit* Radar=SpawnRole(EPrototypeRole::Radar,HQ+FVector(1200,-1200,0));
        APrototypeUnit* Ranger=SpawnRole(EPrototypeRole::Infantry,HQ+FVector(0,-1200,0));
        Ready=Radar&&Ranger;
        if(Ranger)
        {
            // Pose refresh only for the selected sample; the match and its
            // production/economy timers continue normally around this actor.
            Ranger->Tick(.016f);Ranger->SetActorTickEnabled(false);Select(Ranger);
        }
        if(MultiState)
        {
            for(int32 Index=0;Index<3;++Index)
            {
                APrototypeUnit* Member=SpawnRole(EPrototypeRole::Combat,HQ+FVector(350+Index*350,-1600,0));
                Ready&=Member!=nullptr;Select(Member);
            }
            Ready&=Controller->HandleControlGroupClick(1,true,false);
        }
        if(LowPowerState)
        {
            Ready&=SpawnRole(EPrototypeRole::Factory,HQ+FVector(-1000,-2000,0))!=nullptr;
            for(TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
                if(It->Team==0&&It->Role==EPrototypeRole::PowerPlant&&It->Health>0.f&&!It->IsActorBeingDestroyed()&&!It->bManuallyPoweredDown)
                    Ready&=Mode->TogglePower(*It);
        }
    }
    Mode->RefreshPowerGrid();Mode->UpdateVisibility();
    const auto& Power=Mode->PowerGrid[0];
    if(LowPowerState)Ready&=Power.CurrentPower<Power.RequiredPower&&!Mode->HasRadar(0);
    else if(!ProductionState)Ready&=Mode->HasRadar(0);
    if(PauseState)Ready&=Controller->SetPause(true)&&Mode->GetWorld()->IsPaused();
    if(VictoryState){Mode->OnHeadquartersDestroyed(1);Ready&=Mode->Outcome==1;}
    UE_LOG(LogTemp,Display,TEXT("UI_CAPTURE_FIXTURE state=%s unnatural_fixture=1 result=%s radar=%d power_current=%d power_required=%d production_rate=%.2f selected=%d queue=%d credits=%.0f group1=%d paused=%d outcome=%d"),
        *State,Ready?TEXT("PASS"):TEXT("FAIL"),Mode->HasRadar(0)?1:0,Power.CurrentPower,Power.RequiredPower,Mode->GetProductionRate(0),Controller->Selected.Num(),Mode->GetQueueCount(),Mode->Credits,Controller->GetControlGroupCount(1),Mode->GetWorld()->IsPaused()?1:0,Mode->Outcome);
    return Ready;
}
