#include "PrototypeGameMode.h"
#include "BattlefieldLayout.h"
#include "RTS/HRRTSDefinitions.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMemory.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "Misc/App.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "UnrealClient.h"
#include "TimerManager.h"

extern void RunPrototypeTeamBot(APrototypeGameMode* Mode,int32 Team);

namespace
{
bool AcceptanceLive(const APrototypeUnit* Unit)
{
    return IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health>0.f;
}

int32 CountRole(APrototypeGameMode* Mode,int32 Team,EPrototypeRole Role)
{
    int32 Count=0;
    for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        if (AcceptanceLive(*It) && It->Team==Team && It->Role==Role) ++Count;
    return Count;
}

bool BuildPaidSite(APrototypeGameMode* Mode,int32 Team,EPrototypeRole Role)
{
    const APrototypeUnit* HQ=Team==0 ? Mode->PlayerHQ : Mode->EnemyHQ;
    if (!AcceptanceLive(HQ)) return false;
    const int32 PreviousRotation=Mode->BuildingQuarterTurns;
    Mode->BuildingQuarterTurns=0;
    for (int32 Y=-2000;Y<=2000;Y+=200)
    for (int32 X=-2000;X<=2000;X+=200)
    {
        const FVector Site=Mode->SnapBuildPosition(HQ->GetActorLocation()+FVector(X,Y,0));
        if (!Mode->CanPlaceBuilding(Role,Site,Team)) continue;
        const bool Built=Mode->BeginBuilding(Role,Site,Team);
        Mode->BuildingQuarterTurns=PreviousRotation;
        return Built;
    }
    Mode->BuildingQuarterTurns=PreviousRotation;
    return false;
}

void ManualStep(APrototypeGameMode* Mode,float DeltaSeconds)
{
    Mode->TickEconomy(DeltaSeconds);
    Mode->Commander[0].Advance(DeltaSeconds); Mode->Commander[1].Advance(DeltaSeconds);
    Mode->Visibility.Advance(DeltaSeconds); Mode->UpdateVisibility();
    TArray<APrototypeUnit*> Snapshot;
    for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        if (AcceptanceLive(*It)) { It->SetActorTickEnabled(false); Snapshot.Add(*It); }
    for (APrototypeUnit* Unit:Snapshot)
        if (AcceptanceLive(Unit) && Mode->Outcome==0) Unit->Tick(DeltaSeconds);
}

template<typename Predicate>
bool SimulateUntil(APrototypeGameMode* Mode,float BudgetSeconds,Predicate&& PredicateFunction)
{
    for (int32 Step=0;Step<FMath::CeilToInt(BudgetSeconds/.1f) && Mode->Outcome==0;++Step)
    {
        if (PredicateFunction()) return true;
        ManualStep(Mode,.1f);
    }
    return PredicateFunction();
}

struct FCollectorWatch
{
    FVector Position=FVector::ZeroVector;
    double Cargo=0.;
    float LastProgressSimulation=0.f;
    EPrototypeCollectorState State=EPrototypeCollectorState::SeekingField;
};

struct FSoakRun
{
    TWeakObjectPtr<APrototypeGameMode> Mode;
    double Start=0.,LastFrame=0.,NextSample=0.;
    float PlayerDecision=0.f;
    int32 CompletedRounds=0,Wins=0,Losses=0;
    bool bFailed=false,bComplete=false,bRealRHI=false;
    TArray<double> FrameMilliseconds;
    TArray<FString> Samples;
    TMap<TWeakObjectPtr<APrototypeUnit>,FCollectorWatch> CollectorWatches;
    FString Path;
    FString StartedUtc;
};

FSoakRun Soak;

void ExitTest(APrototypeGameMode* Mode)
{
    if (APlayerController* Controller=Mode->GetWorld()->GetFirstPlayerController()) Controller->ConsoleCommand(TEXT("quit"));
    else FPlatformMisc::RequestExit(false);
}

void WriteSoakReport(APrototypeGameMode* Mode,double Elapsed)
{
    TArray<double> Sorted=Soak.FrameMilliseconds;
    Sorted.Sort();
    double Sum=0.;
    for (double Value:Sorted) Sum+=Value;
    const double Mean=Sorted.IsEmpty() ? 0. : Sum/Sorted.Num();
    const double P95=Sorted.IsEmpty() ? 0. : Sorted[FMath::Clamp(FMath::CeilToInt(Sorted.Num()*.95)-1,0,Sorted.Num()-1)];
    const double Worst=Sorted.IsEmpty() ? 0. : Sorted.Last();
    const FString Report=FString::Printf(TEXT("{\n  \"schema\": \"HeroHour.RealRHISoak.v1\",\n  \"started_utc\": \"%s\",\n  \"elapsed_wall_seconds\": %.3f,\n  \"required_wall_seconds\": 1200,\n  \"complete\": %s,\n  \"real_rhi\": %s,\n  \"result\": \"%s\",\n  \"frame_samples\": %d,\n  \"mean_frame_ms\": %.4f,\n  \"p95_frame_ms\": %.4f,\n  \"worst_frame_ms\": %.4f,\n  \"mean_fps_from_wall_frames\": %.3f,\n  \"completed_rounds\": %d,\n  \"player_wins\": %d,\n  \"player_losses\": %d,\n  \"samples\": [\n%s\n  ]\n}\n"),
        *Soak.StartedUtc,Elapsed,Soak.bComplete ? TEXT("true") : TEXT("false"),
        Soak.bRealRHI ? TEXT("true") : TEXT("false"),Soak.bComplete ? (Soak.bFailed ? TEXT("FAIL") : TEXT("PASS")) : TEXT("RUNNING"),
        Sorted.Num(),Mean,P95,Worst,Mean>0. ? 1000./Mean : 0.,Soak.CompletedRounds,Soak.Wins,Soak.Losses,
        *FString::Join(Soak.Samples,TEXT(",\n")));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Soak.Path),true);
    if (!FFileHelper::SaveStringToFile(Report,*Soak.Path)) Soak.bFailed=true;
}
}

bool RunHRFullMatchTests(UWorld* World)
{
    APrototypeGameMode* Mode=World ? World->GetAuthGameMode<APrototypeGameMode>() : nullptr;
    bool Passed=true;
    int32 Count=0;
    auto Check=[&](bool Condition,const TCHAR* Label)
    {
        ++Count; Passed &= Condition;
        UE_LOG(LogTemp,Display,TEXT("FULLMATCH_TEST %s: %s"),Label,Condition ? TEXT("PASS") : TEXT("FAIL"));
    };
    if (!Mode)
    {
        Check(false,TEXT("full-match acceptance has an authoritative world"));
        UE_LOG(LogTemp,Display,TEXT("FULLMATCH_TEST_RESULT=FAIL")); return false;
    }
    Mode->ResetMatch(); Mode->BotDecisionTime=10000.f;
    const UHRFactionDefinition* FactionZero=Mode->GetFactionDefinition(0),*FactionOne=Mode->GetFactionDefinition(1);
    Check(FactionZero && FactionOne && FactionZero!=FactionOne && FactionZero->FactionId!=FactionOne->FactionId
        && Mode->GetRoleDefinition(EPrototypeRole::Combat,0)!=Mode->GetRoleDefinition(EPrototypeRole::Combat,1),
        TEXT("both live teams use distinct saved faction and unit definitions"));
    Check(Mode->SalvageNodes.Num()==3 && Mode->GetFieldCellCount(0)==16 && Mode->GetFieldCellCount(1)==16
        && Mode->SalvageNodes[0].Field.GetStyle()==EHREconomyStyle::RegrowingField,
        TEXT("full match starts with spatial regrowing fields and separate home economies"));
    Check(!Mode->CanTeamSee(0,Mode->EnemyHQ) && !Mode->CanTeamSee(1,Mode->PlayerHQ),
        TEXT("opposing bases begin hidden by independent fog of war"));
    const double InitialZero=Mode->Credits,InitialOne=Mode->EnemyCredits;
    Check(SimulateUntil(Mode,45.f,[&]() { return Mode->Credits>=InitialZero+100. && Mode->EnemyCredits>=InitialOne+100.; }),
        TEXT("both teams harvest, travel, dock and deliver a paid load in real unit simulation"));
    for (int32 Team=0;Team<2;++Team)
    {
        const double Before=Team==0 ? Mode->Credits : Mode->EnemyCredits;
        const float Cost=Mode->RoleCost(EPrototypeRole::PowerPlant,Team);
        const int32 OldPlants=CountRole(Mode,Team,EPrototypeRole::PowerPlant);
        const bool Built=BuildPaidSite(Mode,Team,EPrototypeRole::PowerPlant);
        Check(Built && (Team==0 ? Mode->Credits : Mode->EnemyCredits)==Before-Cost
            && CountRole(Mode,Team,EPrototypeRole::PowerPlant)==OldPlants+1,
            Team==0 ? TEXT("player power plant is placed and paid through the public construction command")
                : TEXT("enemy power plant is placed and paid through the same construction command"));
    }
    Check(SimulateUntil(Mode,25.f,[&]()
    {
        int32 Complete=0;
        for (TActorIterator<APrototypeUnit> It(World);It;++It)
            if (AcceptanceLive(*It) && It->Role==EPrototypeRole::PowerPlant && It->IsCompleted()) ++Complete;
        return Complete>=4 && Mode->IsPowered(0) && Mode->IsPowered(1);
    }),TEXT("paid power plants complete on simulation time and both grids are online"));
    for (int32 Team=0;Team<2;++Team)
    {
        const bool HasFactory=Mode->FindCompletedStructure(Team,EPrototypeRole::Factory)!=nullptr;
        Check((HasFactory || BuildPaidSite(Mode,Team,EPrototypeRole::Factory))
            && BuildPaidSite(Mode,Team,EPrototypeRole::Barracks),
            Team==0 ? TEXT("player builds separate vehicle and infantry producers")
                : TEXT("enemy builds separate vehicle and infantry producers"));
    }
    Check(SimulateUntil(Mode,50.f,[&]()
    {
        return Mode->FindCompletedStructure(0,EPrototypeRole::Factory) && Mode->FindCompletedStructure(1,EPrototypeRole::Factory)
            && Mode->FindCompletedStructure(0,EPrototypeRole::Barracks) && Mode->FindCompletedStructure(1,EPrototypeRole::Barracks)
            && Mode->Credits>=180. && Mode->EnemyCredits>=180.;
    }),TEXT("construction lanes finish and resource delivery finances both production categories"));
    const int32 TanksZero=CountRole(Mode,0,EPrototypeRole::Combat),TanksOne=CountRole(Mode,1,EPrototypeRole::Combat);
    for (int32 Team=0;Team<2;++Team)
        Check(Mode->RequestProduction(EPrototypeRole::Combat,Team) && Mode->RequestProduction(EPrototypeRole::Infantry,Team),
            Team==0 ? TEXT("player queues infantry and vehicles independently") : TEXT("enemy queues infantry and vehicles independently"));
    ManualStep(Mode,1.f);
    Check(Mode->GetProductionProgress(Mode->FindCompletedStructure(0,EPrototypeRole::Factory))>0.f
        && Mode->GetProductionProgress(Mode->FindCompletedStructure(0,EPrototypeRole::Barracks))>0.f,
        TEXT("both player production categories advance during the same simulation step"));
    Check(SimulateUntil(Mode,15.f,[&]()
    {
        return CountRole(Mode,0,EPrototypeRole::Combat)>TanksZero && CountRole(Mode,1,EPrototypeRole::Combat)>TanksOne
            && CountRole(Mode,0,EPrototypeRole::Infantry)>0 && CountRole(Mode,1,EPrototypeRole::Infantry)>0;
    }),TEXT("paid category queues spawn usable faction-specific infantry and vehicles"));

    // A controlled assault exercises navigation, reveal, target acquisition,
    // typed weapons, HQ result and restart together. The opposing army receives
    // a legal retreat order rather than being removed or receiving fake damage.
    for (int32 AttackingTeam=0;AttackingTeam<2;++AttackingTeam)
    {
        if (AttackingTeam==1)
        {
            Mode->ResetMatch(); Mode->BotDecisionTime=10000.f;
        }
        APrototypeUnit* TargetHQ=AttackingTeam==0 ? Mode->EnemyHQ : Mode->PlayerHQ;
        APrototypeUnit* RetreatHQ=TargetHQ;
        TArray<APrototypeUnit*> Attackers;
        for (TActorIterator<APrototypeUnit> It(World);It;++It)
        {
            if (!AcceptanceLive(*It) || !It->IsCombatant() || It->IsStructure()) continue;
            if (It->Team==AttackingTeam) Attackers.Add(*It);
            else
            {
                FVector Retreat=RetreatHQ->GetActorLocation()+FVector(It->Team==0 ? -2200.f : 2200.f,-2500.f,0);
                Retreat.Z=35.f;
                It->OrderManualMove(Retreat);
            }
        }
        for (APrototypeUnit* Unit:Attackers) Unit->OrderAttack(TargetHQ);
        bool SawTarget=false,SawActualDamage=false;
        const bool Finished=SimulateUntil(Mode,260.f,[&]()
        {
            SawTarget |= Mode->CanTeamSee(AttackingTeam,TargetHQ);
            SawActualDamage |= AcceptanceLive(TargetHQ) && TargetHQ->Health<TargetHQ->MaxHealth;
            return Mode->Outcome!=0;
        });
        Check(Finished && SawTarget && SawActualDamage && Mode->Outcome==(AttackingTeam==0 ? 1 : -1),
            AttackingTeam==0 ? TEXT("player assault reveals the enemy HQ and wins through actual typed weapon damage")
                : TEXT("enemy assault reveals the player HQ and produces the real defeat result"));
    }
    Mode->ResetMatch();
    Check(Mode->Outcome==0 && Mode->MatchTime==0.f && Mode->Credits==800. && Mode->EnemyCredits==800.
        && Mode->ProductionQueue.IsEmpty() && Mode->Commander[0].Points==0.f && Mode->Commander[1].Points==0.f
        && !Mode->CanTeamSee(0,Mode->EnemyHQ),TEXT("restart resets economies, queues, doctrines, result and exploration together"));
    UE_LOG(LogTemp,Display,TEXT("FULLMATCH_TEST_COUNT=%d"),Count);
    UE_LOG(LogTemp,Display,TEXT("FULLMATCH_TEST_RESULT=%s"),Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}

void TickHRSoakTest(APrototypeGameMode* Mode,float DeltaSeconds)
{
    if (!Mode || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds<=0.f) return;
    const double Now=FPlatformTime::Seconds();
    if (Soak.Mode.Get()!=Mode)
    {
        Soak=FSoakRun(); Soak.Mode=Mode; Soak.StartedUtc=FDateTime::UtcNow().ToIso8601(); Soak.Start=Soak.LastFrame=Now; Soak.NextSample=Now;
        Soak.Path=FPaths::ProjectSavedDir()/TEXT("Research")/FString::Printf(TEXT("Soak-%s-%s-%u.json"),
            Mode->GetFactionIndex(0)==0?TEXT("Aegis"):TEXT("Ember"),*FDateTime::UtcNow().ToString(TEXT("%Y%m%d-%H%M%S")),FPlatformProcess::GetCurrentProcessId());
        Soak.bRealRHI=FApp::CanEverRender() && !FParse::Param(FCommandLine::Get(),TEXT("nullrhi"));
        if (!Soak.bRealRHI)
        {
            Soak.bFailed=true; Soak.bComplete=true; WriteSoakReport(Mode,0.);
            UE_LOG(LogTemp,Error,TEXT("SOAK_TEST_RESULT=FAIL: real rendering is required")); ExitTest(Mode); return;
        }
        UE_LOG(LogTemp,Display,TEXT("SOAK_TEST_START duration=1200 wall seconds report=%s"),*Soak.Path);
    }
    if (Soak.bComplete) return;
    const double FrameMilliseconds=(Now-Soak.LastFrame)*1000.; Soak.LastFrame=Now;
    if (FrameMilliseconds>0. && FMath::IsFinite(FrameMilliseconds)) Soak.FrameMilliseconds.Add(FrameMilliseconds);
    const double Elapsed=Now-Soak.Start;
    if (Mode->Outcome!=0)
    {
        ++Soak.CompletedRounds;
        if (Mode->Outcome==1) ++Soak.Wins; else ++Soak.Losses;
        Mode->ResetMatch(); Soak.CollectorWatches.Empty(); Soak.PlayerDecision=0.f;
    }
    Soak.PlayerDecision-=DeltaSeconds;
    if (Soak.PlayerDecision<=0.f)
    {
        RunPrototypeTeamBot(Mode,0); Soak.PlayerDecision=.75f;
    }
    double Cargo[2]={0.,0.},Stock=0.;
    int32 Collectors[2]={0,0},Army[2]={0,0},Harvesting=0,Waiting=0,Unloading=0;
    for (const FPrototypeSalvage& Field:Mode->SalvageNodes) Stock+=Field.Field.GetTotalAmount();
    for (TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
    {
        if (!AcceptanceLive(*It) || (It->Team!=0 && It->Team!=1)) continue;
        if (It->IsCombatant() && !It->IsStructure()) ++Army[It->Team];
        if (It->Role!=EPrototypeRole::Collector) continue;
        ++Collectors[It->Team]; Cargo[It->Team]+=It->Cargo;
        if (!FMath::IsFinite(It->Cargo) || It->Cargo<0. || It->Cargo>100.00001) Soak.bFailed=true;
        Harvesting+=It->CollectorState==EPrototypeCollectorState::Harvesting;
        Waiting+=It->CollectorState==EPrototypeCollectorState::WaitingForDock;
        Unloading+=It->CollectorState==EPrototypeCollectorState::Unloading;
        FCollectorWatch* Watch=Soak.CollectorWatches.Find(*It);
        if (!Watch)
        {
            FCollectorWatch Initial; Initial.Position=It->GetActorLocation(); Initial.Cargo=It->Cargo;
            Initial.State=It->CollectorState; Initial.LastProgressSimulation=Mode->MatchTime;
            Soak.CollectorWatches.Add(*It,Initial); continue;
        }
        if (FVector::DistSquared2D(Watch->Position,It->GetActorLocation())>FMath::Square(10.f)
            || FMath::Abs(Watch->Cargo-It->Cargo)>.0001 || Watch->State!=It->CollectorState)
        {
            Watch->Position=It->GetActorLocation(); Watch->Cargo=It->Cargo;
            Watch->State=It->CollectorState; Watch->LastProgressSimulation=Mode->MatchTime;
        }
        if (!It->bCollectorManualOrder && Stock>1. && Mode->FindCompletedStructure(It->Team,EPrototypeRole::Depot)
            && Mode->MatchTime-Watch->LastProgressSimulation>120.f)
        {
            Soak.bFailed=true; Watch->LastProgressSimulation=Mode->MatchTime;
            const auto Obstacles=FPrototypeNavigation::GetWorldObstacles(Mode->GetWorld());
            UE_LOG(LogTemp,Error,TEXT("SOAK_TEST collector stalled for 120 simulation seconds: %s team=%d state=%d position=%s destination=%s cargo=%.17g node=%d cell=%d depot=%s approach=%s walkable=%d goalwalkable=%d power=%d/%d"),
                *It->GetName(),It->Team,static_cast<int32>(It->CollectorState),*It->GetActorLocation().ToString(),
                *It->Destination.ToString(),It->Cargo,It->HarvestNode,It->HarvestCell,*GetNameSafe(It->DeliveryDepot.Get()),
                *It->DockApproach.ToString(),FPrototypeNavigation::IsWalkable(It->GetActorLocation(),Obstacles),
                FPrototypeNavigation::IsWalkable(It->Destination,Obstacles),Mode->PowerGrid[It->Team].CurrentPower,
                Mode->PowerGrid[It->Team].RequiredPower);
        }
    }
    for (auto It=Soak.CollectorWatches.CreateIterator();It;++It) if (!AcceptanceLive(It.Key().Get())) It.RemoveCurrent();
    if (!FMath::IsFinite(Mode->Credits) || !FMath::IsFinite(Mode->EnemyCredits) || Mode->Credits<-.0001 || Mode->EnemyCredits<-.0001)
        Soak.bFailed=true;
    if (Now>=Soak.NextSample || Elapsed>=1200.)
    {
        const FPlatformMemoryStats Memory=FPlatformMemory::GetStats();
        const FString Sample=FString::Printf(TEXT("    {\"wall_s\":%.3f,\"match_s\":%.3f,\"round\":%d,\"credits\":[%.6f,%.6f],\"cargo\":[%.6f,%.6f],\"field_stock\":%.6f,\"collectors\":[%d,%d],\"army\":[%d,%d],\"harvesting\":%d,\"waiting_for_dock\":%d,\"unloading\":%d,\"production_orders\":%d,\"physical_memory_bytes\":%llu,\"peak_physical_memory_bytes\":%llu}"),
            Elapsed,Mode->MatchTime,Soak.CompletedRounds+1,Mode->Credits,Mode->EnemyCredits,Cargo[0],Cargo[1],Stock,
            Collectors[0],Collectors[1],Army[0],Army[1],Harvesting,Waiting,Unloading,Mode->ProductionQueue.Num(),
            static_cast<unsigned long long>(Memory.UsedPhysical),static_cast<unsigned long long>(Memory.PeakUsedPhysical));
        Soak.Samples.Add(Sample); Soak.NextSample=Now+60.;
        if(Elapsed>=60. && Elapsed<1195.)
        {
            if(auto* Controller=Cast<APrototypeController>(Mode->GetWorld()->GetFirstPlayerController()))
            {
                APrototypeUnit* Focus=Mode->PlayerHQ;
                for(TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
                    if(AcceptanceLive(*It) && It->Team==0 && It->Role==EPrototypeRole::Infantry) { Focus=*It;break; }
                if(AcceptanceLive(Focus))
                {
                    Controller->ClearSelection();Controller->Selected.Add(Focus);Focus->Select(true);
                    if(APawn* Camera=Controller->GetPawn()) { FVector Position=Camera->GetActorLocation();Position.Z=4000.f;Camera->SetActorLocation(Position); }
                    Controller->PanToWorld(Focus->GetActorLocation());
                }
                FTimerHandle Shutter;
                const FString Path=FPaths::ProjectSavedDir()/FString::Printf(TEXT("Screenshots/Soak-%s-%04d.png"),
                    Mode->GetFactionIndex(0)==0?TEXT("Aegis"):TEXT("Ember"),FMath::RoundToInt(Elapsed));
                Mode->GetWorldTimerManager().SetTimer(Shutter,[Path]() { FScreenshotRequest::RequestScreenshot(Path,true,false); },1.f,false);
            }
        }
        UE_LOG(LogTemp,Display,TEXT("SOAK_TEST_PROGRESS wall=%.1f/1200 rounds=%d collectors=%d/%d credits=%.1f/%.1f"),
            Elapsed,Soak.CompletedRounds,Collectors[0],Collectors[1],Mode->Credits,Mode->EnemyCredits);
        if (Elapsed>=1200.)
        {
            Soak.bComplete=true;
            if (Soak.CompletedRounds<2 || Soak.FrameMilliseconds.Num()<1200) Soak.bFailed=true;
        }
        WriteSoakReport(Mode,Elapsed);
        if (Soak.bComplete)
        {
            UE_LOG(LogTemp,Display,TEXT("SOAK_TEST_RESULT=%s"),Soak.bFailed ? TEXT("FAIL") : TEXT("PASS")); ExitTest(Mode);
        }
    }
}
