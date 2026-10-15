#include "PrototypeGameMode.h"
#include "PrototypeHUDLayout.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Containers/Ticker.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#if WITH_EDITOR
#include "AssetCompilingManager.h"
#endif

void APrototypeGameMode::RunUITest()
{
    FTimerHandle Timer;
    GetWorldTimerManager().SetTimer(Timer,[this]()
    {
        // Commands can create lifespan timers while this delegate is executing.
        // Keep its owner on the stack instead of rereading the delegate capture.
        APrototypeGameMode* const Mode=this;
        bool Passed=true;
        auto Check=[&](bool Condition,const TCHAR* Label){Passed&=Condition;UE_LOG(LogTemp,Display,TEXT("UI_TEST %s: %s"),Label,Condition?TEXT("PASS"):TEXT("FAIL"));};
        auto* Controller=Cast<APrototypeController>(Mode->GetWorld()->GetFirstPlayerController());
        auto* HUD=Controller?Cast<APrototypeHUD>(Controller->GetHUD()):nullptr;
        int32 Width=0,Height=0;if(Controller)Controller->GetViewportSize(Width,Height);
        Check(HUD&&Controller&&Controller->GetPawn()&&Width>0&&Height>0,TEXT("live controller HUD and viewport ready"));
        if(HUD&&Controller&&Controller->GetPawn()&&Width>0&&Height>0)
        {
            Controller->ClearSelection();Controller->Selected.Add(Mode->PlayerHQ);Mode->PlayerHQ->Select(true);
            const FPrototypeHUDLayout L(Width,Height);
            auto Center=[&](FPrototypeHUDRect R){return FVector2D(R.X+R.W*.5f,R.Y+R.H*.5f)*L.S;};
            const float Before=Mode->Credits;
            Check(HUD->HandleClick(Center(L.ProductionTab(3)))&&HUD->ProductionCategory==3&&Mode->Credits==Before,
                TEXT("vehicle category switches visible cards without charging credits"));
            Check(HUD->HandleClick(Center(L.ProductionTab(1)))&&HUD->ProductionCategory==1&&Mode->GetQueueCount(Mode->PlayerHQ)==0&&Mode->Credits==Before,
                TEXT("defense category activates without changing economy"));
            HUD->HandleClick(Center(L.ProductionTab(3)));
            Check(HUD->HandleClick(Center(L.ProductionTile(1,0)))&&Mode->GetQueueCount(Mode->PlayerHQ)==1&&Mode->Credits==Before-150,
                TEXT("collector card dispatches production to selected HQ"));
            Check(HUD->HandleClick(Center(L.ProductionTile(0,0)))&&Mode->GetQueueCount(Mode->PlayerHQ)==1&&Mode->Credits==Before-150,
                TEXT("unsupported HQ combat card cannot charge or mutate queue"));
            Check(HUD->HandleClick(Center(L.Cancel()))&&Mode->GetQueueCount(Mode->PlayerHQ)==0&&Mode->Credits==Before,
                TEXT("cancel card refunds selected HQ order exactly once"));
            HUD->HandleClick(Center(L.Cancel()));
            Check(Mode->Credits==Before,TEXT("empty cancel card cannot create credits"));
            const FVector CameraBefore=Controller->GetPawn()->GetActorLocation();
            Check(HUD->HandleClick(Center(L.MiniMap()))&&Controller->GetPawn()->GetActorLocation().Equals(CameraBefore),
                TEXT("minimap without an operational radar cannot disclose or pan the map"));
            auto* Radar=Mode->SpawnUnit(0,FVector(-10400,0,35));if(Radar)Radar->ConfigureRole(EPrototypeRole::Radar);
            Mode->RefreshPowerGrid();Mode->UpdateVisibility();
            Check(HUD->HandleClick(Center(L.MiniMap()))&&FVector::DistSquared2D(CameraBefore,Controller->GetPawn()->GetActorLocation())>10000,
                TEXT("minimap click changes actual camera pawn position"));
            const FVector BeforeFocus=Controller->GetPawn()->GetActorLocation();
            Check(HUD->HandleClick(Center(L.Focus()))&&FVector::DistSquared2D(BeforeFocus,Controller->GetPawn()->GetActorLocation())>10000,
                TEXT("selected object focus card changes actual camera position"));
            Check(HUD->HandleClick(Center(L.ProductionTab(0)))&&HUD->ProductionCategory==0,
                TEXT("base category routes to construction cards"));
            Check(HUD->HandleClick(Center(L.ProductionTile(1,0)))&&Controller->bPlacing&&Controller->PlacementRole==EPrototypeRole::Factory&&Mode->Credits==Before,
                TEXT("factory card starts real placement without paying before a valid site"));
            // Every authored building card occupies the same compact five-column row.
            const EPrototypeRole ExpectedBuildings[]={EPrototypeRole::Depot,EPrototypeRole::Factory,EPrototypeRole::PowerPlant,EPrototypeRole::Barracks,EPrototypeRole::Radar};
            for(int32 Card=0;Card<5;++Card)
            {
                Controller->ClearSelection();
                const bool Clicked=HUD->HandleClick(Center(L.ProductionCard(Card)));
                Check(Clicked&&Controller->bPlacing&&Controller->PlacementRole==ExpectedBuildings[Card]&&Mode->Credits==Before,
                    *FString::Printf(TEXT("five-column building card %d starts correct placement without charging"),Card));
            }
            auto Overlaps=[](const FPrototypeHUDRect& A,const FPrototypeHUDRect& B){return A.X<B.X+B.W&&A.X+A.W>B.X&&A.Y<B.Y+B.H&&A.Y+A.H>B.Y;};
            for(const FIntPoint Resolution:{FIntPoint(1600,900),FIntPoint(1920,1080),FIntPoint(1280,720)})
            {
                const FPrototypeHUDLayout Layout(Resolution.X,Resolution.Y);
                bool GeometrySafe=!Overlaps(Layout.Resource(),Layout.Army())&&!Overlaps(Layout.Resource(),Layout.Mission())&&!Overlaps(Layout.Army(),Layout.Mission());
                const auto Panel=Layout.Production();
                for(int32 Card=0;Card<5;++Card){const auto Tile=Layout.ProductionCard(Card);
                    GeometrySafe&=Tile.X>=Panel.X&&Tile.Y>=Panel.Y&&Tile.X+Tile.W<=Panel.X+Panel.W&&Tile.Y+Tile.H<=Panel.Y+Panel.H;
                    GeometrySafe&=FMath::IsNearlyEqual(Tile.Y,Layout.ProductionCard(0).Y)&&!Overlaps(Tile,Layout.QueueSlot(0))&&!Overlaps(Tile,Layout.Cancel());
                    for(int32 Other=Card+1;Other<5;++Other)GeometrySafe&=!Overlaps(Tile,Layout.ProductionCard(Other));
                }
                Check(GeometrySafe,*FString::Printf(TEXT("five-column cards and top panels do not overlap at %dx%d"),Resolution.X,Resolution.Y));
            }
            Controller->ClearSelection();
            Check(HUD->IsOverInterface(Center(L.Production()))&&HUD->IsOverInterface(Center(L.Selection()))
                &&!HUD->IsOverInterface(FVector2D(Width*.5f,Height*.4f)),
                TEXT("separate panels consume clicks while central battlefield stays interactive"));
            auto* Frame=LoadObject<UTexture2D>(nullptr,TEXT("/Game/HERO/UI/ConceptV3/T_UI_Frame_V3.T_UI_Frame_V3"));
            auto* MapFrame=LoadObject<UTexture2D>(nullptr,TEXT("/Game/HERO/UI/ConceptV3/T_UI_MinimapFrame_V3.T_UI_MinimapFrame_V3"));
            auto* Glass=LoadObject<UTexture2D>(nullptr,TEXT("/Game/HERO/UI/ConceptV3/T_UI_PanelGlass_V3.T_UI_PanelGlass_V3"));
            auto* Title=LoadObject<UTexture2D>(nullptr,TEXT("/Game/HERO/UI/ConceptV3/T_UI_TitlePlate_V3.T_UI_TitlePlate_V3"));
            auto* Glyphs=LoadObject<UTexture2D>(nullptr,TEXT("/Game/HERO/UI/ConceptV3/T_UI_GlyphAtlas_V3.T_UI_GlyphAtlas_V3"));
            auto* TacticalMap=LoadObject<UTexture2D>(nullptr,TEXT("/Game/HERO/UI/ConceptV3/T_UI_CityHeroMap_V3.T_UI_CityHeroMap_V3"));
#if WITH_EDITOR
            TArray<UObject*> InterfaceAssets={Frame,MapFrame,Glass,Title,Glyphs,TacticalMap};
            FAssetCompilingManager::Get().FinishCompilationForObjects(InterfaceAssets);
#endif
            UE_LOG(LogTemp,Display,TEXT("UI_MAP_RUNTIME_SIZE=%dx%d"),TacticalMap?TacticalMap->GetSizeX():0,TacticalMap?TacticalMap->GetSizeY():0);
            Check(TacticalMap&&TacticalMap->GetSizeX()==1024&&TacticalMap->GetSizeY()==768,
                TEXT("own tactical map background resolves at original aspect ratio"));
            Check(Frame&&MapFrame&&Glass&&Title&&Glyphs&&Frame->GetSizeX()==256&&Glyphs->GetSizeX()==512&&Glyphs->GetSizeY()==512,
                TEXT("concept frame glass title and category glyph textures resolve from cooked content"));
            for(int32 Category=0;Category<5;++Category)
                Check(HUD->HandleClick(Center(L.ProductionTab(Category)))&&HUD->ProductionCategory==Category,
                    *FString::Printf(TEXT("functional production category %d receives actual click"),Category));
            // Controlled UI fixture: exercise actual target selection and terrain-click confirmation,
            // not a direct commander mutation masquerading as a mouse command.
            Controller->ClearSelection();
            const FHRCommanderState SavedCommander=Mode->Commander[0];
            const FVector2D TargetScreen(Width*.5f,Height*.4f);FVector RayOrigin,RayDirection;
            const bool GroundRay=Controller->DeprojectScreenPositionToWorld(TargetScreen.X,TargetScreen.Y,RayOrigin,RayDirection)&&RayDirection.Z<-.001f;
            Check(GroundRay,TEXT("commander target fixture deprojects a genuine battlefield click"));
            if(GroundRay)
            {
                const FVector Ground=RayOrigin+RayDirection*((35.f-RayOrigin.Z)/RayDirection.Z);
                for(int32 Source=0;Source<2;++Source)for(int32 Card=0;Card<2;++Card)
                {
                    CancelPrototypeDoctrineTarget(HUD);
                    HUD->HandleClick(Center(L.ProductionTab(4)));
                    const EHRDoctrine Doctrine=GetFactionIndex(0)==0?(Card==0?EHRDoctrine::RepairPulse:EHRDoctrine::PrecisionStrike):(Card==0?EHRDoctrine::Scorch:EHRDoctrine::Overdrive);
                    const float Cost=FHRCommanderState::Cost(Doctrine);const float WalletBefore=Mode->Credits;
                    const FVector2D Button=Center(Source==0?L.ProductionCard(Card):L.DoctrineCard(Card));
                    Mode->Commander[0].Points=Cost-1.f;Mode->Commander[0].GlobalCooldown=0.f;
                    const bool Refused=HUD->HandleClick(Button);
                    Check(Refused&&!HasPrototypeDoctrineTarget(HUD)&&Mode->Commander[0].Points==Cost-1.f&&Mode->Credits==WalletBefore,
                        *FString::Printf(TEXT("doctrine source %d card %d refuses insufficient points without charging"),Source,Card));
                    auto* Friendly=Mode->SpawnUnit(0,Ground);auto* Enemy=Mode->SpawnUnit(1,Ground+FVector(50,0,0));
                    if(Friendly)Friendly->Health=FMath::Max(1.f,Friendly->MaxHealth-50.f);
                    Mode->UpdateVisibility();Mode->Commander[0].Points=100.f;Mode->Commander[0].GlobalCooldown=0.f;
                    const bool Armed=HUD->HandleClick(Button);
                    Check(Armed&&HasPrototypeDoctrineTarget(HUD)&&Mode->Commander[0].Points==100.f&&Mode->Credits==WalletBefore,
                        *FString::Printf(TEXT("doctrine source %d card %d arms public targeting without premature payment"),Source,Card));
                    const bool Confirmed=HUD->HandleClick(TargetScreen);
                    Check(Confirmed&&!HasPrototypeDoctrineTarget(HUD)&&FMath::IsNearlyEqual(Mode->Commander[0].Points,100.f-Cost)&&Mode->Commander[0].GlobalCooldown>0.f&&Mode->Credits==WalletBefore,
                        *FString::Printf(TEXT("doctrine source %d card %d terrain confirmation pays exact point cost only"),Source,Card));
                    CancelPrototypeDoctrineTarget(HUD);
                    if(IsValid(Friendly))Friendly->Destroy();if(IsValid(Enemy))Enemy->Destroy();
                }
            }
            Mode->Commander[0]=SavedCommander;
            auto* Infantry=Mode->SpawnUnit(0,FVector(-10400,-1400,35));if(Infantry)Infantry->ConfigureRole(EPrototypeRole::Infantry);
            if(Infantry)
            {
                Controller->ClearSelection();Controller->Selected.Add(Infantry);Infantry->Select(true);
                const bool bSupportsSmoke=APrototypeGameMode::GetFactionIndex(0)==0;
                Check(HUD->HandleClick(Center(L.Ability(1)))&&((Infantry->CombatState.SmokeCooldown>0.f)==bSupportsSmoke),
                    TEXT("unit ability card applies the selected faction smoke capability"));
            }
        }
        if (Controller)
        {
            const float BeforePause=Mode->MatchTime;
            const bool Paused=Controller->SetPause(true);
            const TWeakObjectPtr<APrototypeGameMode> WeakMode(Mode);
            const TWeakObjectPtr<APrototypeController> WeakController(Controller);
            FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([WeakMode,WeakController,BeforePause,Passed,Paused](float)
            {
                if (!WeakMode.IsValid() || !WeakController.IsValid()) return false;
                const bool Frozen=Paused && WeakMode->GetWorld()->IsPaused() && FMath::IsNearlyEqual(WeakMode->MatchTime,BeforePause,.001f);
                UE_LOG(LogTemp,Display,TEXT("UI_TEST world simulation stays frozen during real-time pause: %s"),Frozen?TEXT("PASS"):TEXT("FAIL"));
                WeakController->SetPause(false);
                UE_LOG(LogTemp,Display,TEXT("UI_TEST_RESULT=%s"),Passed&&Frozen?TEXT("PASS"):TEXT("FAIL"));
                WeakController->ConsoleCommand(TEXT("quit"));
                return false;
            }),.35f);
        }
        else UE_LOG(LogTemp,Display,TEXT("UI_TEST_RESULT=FAIL"));
    },1.5f,false);
}
