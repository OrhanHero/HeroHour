#include "PrototypeGameMode.h"
#include "BattlefieldLayout.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/ScopeExit.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

extern void RunPrototypeTeamBot(APrototypeGameMode* Mode,int32 Team);

namespace
{
bool StressLive(const APrototypeUnit* Unit)
{
    return IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health>0.f;
}

struct FStressCollectorWatch
{
    FVector Position=FVector::ZeroVector;
    double Cargo=0.;
    float LastProgress=0.f;
    EPrototypeCollectorState State=EPrototypeCollectorState::SeekingField;
};

void ExplainStall(APrototypeGameMode* Mode,APrototypeUnit* Unit,int32 Round,float LastProgress)
{
    const auto Obstacles=FPrototypeNavigation::GetWorldObstacles(Mode->GetWorld());
    TArray<FVector> Route;
    const bool Walkable=FPrototypeNavigation::IsWalkable(Unit->GetActorLocation(),Obstacles);
    const bool DestinationWalkable=FPrototypeNavigation::IsWalkable(Unit->Destination,Obstacles);
    const bool DestinationRoute=FPrototypeNavigation::FindWorldPath(Mode->GetWorld(),Unit->GetActorLocation(),Unit->Destination,Route);
    int32 PositiveCells=0,ReachableCells=0,UnclaimedReachableCells=0;
    bool ClaimRoute=false;
    for(int32 NodeIndex=0;NodeIndex<Mode->SalvageNodes.Num();++NodeIndex)
    {
        const FPrototypeSalvage& Node=Mode->SalvageNodes[NodeIndex];
        for(int32 CellIndex=0;CellIndex<Node.Field.Num();++CellIndex)
        {
            if(Node.Field.GetHarvestableAmount(CellIndex)<=0.f) continue;
            ++PositiveCells;
            const bool Reachable=FPrototypeNavigation::FindWorldPath(Mode->GetWorld(),Unit->GetActorLocation(),Node.CellPosition(CellIndex),Route);
            ReachableCells+=Reachable;
            const FHRTempestCellState* Cell=Node.Field.GetCell(CellIndex);
            UnclaimedReachableCells+=Reachable && Cell && (!Cell->ReservedCollector.IsValid() || Cell->ReservedCollector.Get()==Unit);
            if(NodeIndex==Unit->HarvestNode && CellIndex==Unit->HarvestCell) ClaimRoute=Reachable;
        }
    }
    int32 Depots=0,OperationalDepots=0,ReachableDepots=0;
    for(TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
    {
        if(!StressLive(*It) || It->Team!=Unit->Team || It->Role!=EPrototypeRole::Depot || !It->IsCompleted()) continue;
        ++Depots;
        if(!Mode->IsUnitOperational(*It)) continue;
        ++OperationalDepots;
        bool Reachable=false;
        for(int32 Index=0;Index<16 && !Reachable;++Index)
        {
            const float Angle=Index*2.f*PI/16.f;
            const FVector Direction(FMath::Cos(Angle),FMath::Sin(Angle),0);
            const FVector Point=It->GetActorLocation()+Direction*FPrototypeNavigation::StructureApproachRadius(*It,Direction,90.f);
            Reachable=FPrototypeNavigation::SegmentWorldWalkable(Mode->GetWorld(),Point,It->GetActorLocation(),*It)
                && FPrototypeNavigation::FindWorldPath(Mode->GetWorld(),Unit->GetActorLocation(),Point,Route);
        }
        ReachableDepots+=Reachable;
        const FHRRefineryDocking* Dock=Mode->RefineryDocks.Find(*It);
        UE_LOG(LogTemp,Error,TEXT("LOGISTICS_STRESS_DIAG depot=%s pos=%s operational=1 reachable=%d docked=%d queued=%d owns_slot=%d owns_queue=%d"),
            *It->GetName(),*It->GetActorLocation().ToString(),Reachable,Dock?Dock->NumDocked():0,Dock?Dock->NumQueued():0,
            Dock?Dock->IsDocked(Unit):false,Dock?Dock->IsQueued(Unit):false);
    }
    UE_LOG(LogTemp,Error,TEXT("LOGISTICS_STRESS_STALL round=%d match=%.3f last_progress=%.3f unit=%s team=%d state=%d pos=%s dest=%s cargo=%.17g node=%d cell=%d depot=%s manual=%d retained=%d walkable=%d destination_walkable=%d destination_route=%d claim_route=%d positive_cells=%d reachable_cells=%d unclaimed_reachable=%d depots=%d operational_depots=%d reachable_depots=%d"),
        Round,Mode->MatchTime,LastProgress,*Unit->GetName(),Unit->Team,static_cast<int32>(Unit->CollectorState),
        *Unit->GetActorLocation().ToString(),*Unit->Destination.ToString(),Unit->Cargo,Unit->HarvestNode,Unit->HarvestCell,
        *GetNameSafe(Unit->DeliveryDepot.Get()),Unit->bCollectorManualOrder,Unit->bRetainedUnloadRemainder,
        Walkable,DestinationWalkable,DestinationRoute,ClaimRoute,PositiveCells,ReachableCells,UnclaimedReachableCells,
        Depots,OperationalDepots,ReachableDepots);
    for(TActorIterator<APrototypeUnit> It(Mode->GetWorld());It;++It)
        if(StressLive(*It) && It->IsStructure())
            UE_LOG(LogTemp,Error,TEXT("LOGISTICS_STRESS_DIAG building=%s team=%d role=%d pos=%s progress=%.3f rotation=%d dims=%s"),
                *It->GetName(),It->Team,static_cast<int32>(It->Role),*It->GetActorLocation().ToString(),
                It->ConstructionProgress,It->BuildRotation,*Mode->GetBuildDimensions(It->Role,It->BuildRotation,It->Team).ToString());
}
}

bool RunHRLogisticsStressTests(UWorld* World)
{
    APrototypeGameMode* Mode=World?World->GetAuthGameMode<APrototypeGameMode>():nullptr;
    if(!Mode)
    {
        UE_LOG(LogTemp,Error,TEXT("LOGISTICS_STRESS_TEST_RESULT=FAIL: missing authoritative world"));
        return false;
    }
    ON_SCOPE_EXIT { Mode->ResetMatch(); };
    bool Passed=true;
    int32 FinishedRounds=0,Wins=0,Losses=0,TotalSteps=0;
    // Small real-frame steps reproduce tangent collisions hidden by the older 10Hz match fixtures.
    float StepSeconds=1.f/60.f;
    FParse::Value(FCommandLine::Get(),TEXT("PrototypeLogisticsStressStep="),StepSeconds);
    StepSeconds=FMath::Clamp(StepSeconds,1.f/240.f,.1f);
    for(int32 Round=1;Round<=20 && Passed;++Round)
    {
        Mode->ResetMatch();
        float PlayerDecision=0.f;
        TMap<TWeakObjectPtr<APrototypeUnit>,FStressCollectorWatch> Watches;
        for(int32 Step=0;Step<FMath::CeilToInt(600.f/StepSeconds) && Mode->Outcome==0 && Passed;++Step)
        {
            ++TotalSteps;
            // Enemy decisions occur only through TickEconomy/TickBot. Player
            // decisions match the soak's cadence, without its camera changes.
            Mode->TickEconomy(StepSeconds);
            Mode->Commander[0].Advance(StepSeconds); Mode->Commander[1].Advance(StepSeconds);
            Mode->Visibility.Advance(StepSeconds); Mode->UpdateVisibility();
            PlayerDecision-=StepSeconds;
            if(PlayerDecision<=0.f) { RunPrototypeTeamBot(Mode,0);PlayerDecision=.75f; }
            TArray<APrototypeUnit*> Snapshot;
            for(TActorIterator<APrototypeUnit> It(World);It;++It)
                if(StressLive(*It)) {It->SetActorTickEnabled(false);Snapshot.Add(*It);}
            for(APrototypeUnit* Unit:Snapshot)
                if(StressLive(Unit) && Mode->Outcome==0) Unit->Tick(StepSeconds);
            if(Mode->Outcome!=0) break;
            double Stock=0.;
            for(const FPrototypeSalvage& Node:Mode->SalvageNodes) Stock+=Node.Field.GetTotalAmount();
            for(APrototypeUnit* Unit:Snapshot)
            {
                if(!StressLive(Unit) || Unit->Role!=EPrototypeRole::Collector) continue;
                FStressCollectorWatch* Watch=Watches.Find(Unit);
                if(!Watch)
                {
                    FStressCollectorWatch Initial;
                    Initial.Position=Unit->GetActorLocation();Initial.Cargo=Unit->Cargo;
                    Initial.State=Unit->CollectorState;Initial.LastProgress=Mode->MatchTime;
                    Watches.Add(Unit,Initial);continue;
                }
                if(FVector::DistSquared2D(Watch->Position,Unit->GetActorLocation())>FMath::Square(10.f)
                    || FMath::Abs(Watch->Cargo-Unit->Cargo)>.0001 || Watch->State!=Unit->CollectorState)
                {
                    Watch->Position=Unit->GetActorLocation();Watch->Cargo=Unit->Cargo;
                    Watch->State=Unit->CollectorState;Watch->LastProgress=Mode->MatchTime;
                }
                const bool InvalidCargo=!FMath::IsFinite(Unit->Cargo) || Unit->Cargo<0. || Unit->Cargo>100.00001;
                const bool Stalled=!Unit->bCollectorManualOrder && Stock>1.
                    && Mode->FindCompletedStructure(Unit->Team,EPrototypeRole::Depot)
                    && Mode->MatchTime-Watch->LastProgress>120.f;
                if(InvalidCargo || Stalled)
                {
                    ExplainStall(Mode,Unit,Round,Watch->LastProgress);
                    Passed=false;break;
                }
            }
            for(auto It=Watches.CreateIterator();It;++It) if(!StressLive(It.Key().Get())) It.RemoveCurrent();
            if(!FMath::IsFinite(Mode->Credits) || !FMath::IsFinite(Mode->EnemyCredits)
                || Mode->Credits<-.0001 || Mode->EnemyCredits<-.0001)
            {
                UE_LOG(LogTemp,Error,TEXT("LOGISTICS_STRESS_TEST invalid wallets round=%d credits=%.17g/%.17g"),Round,Mode->Credits,Mode->EnemyCredits);
                Passed=false;
            }
        }
        if(Mode->Outcome==0)
        {
            Passed=false;
            UE_LOG(LogTemp,Error,TEXT("LOGISTICS_STRESS_TEST round=%d no natural result after %.3f simulation seconds"),Round,Mode->MatchTime);
        }
        else
        {
            ++FinishedRounds;Wins+=Mode->Outcome==1;Losses+=Mode->Outcome==-1;
        }
        UE_LOG(LogTemp,Display,TEXT("LOGISTICS_STRESS_TEST round=%d outcome=%d simulation=%.3f result=%s"),
            Round,Mode->Outcome,Mode->MatchTime,Passed?TEXT("PASS"):TEXT("FAIL"));
    }
    UE_LOG(LogTemp,Display,TEXT("LOGISTICS_STRESS_TEST_RESULT=%s rounds=%d/20 wins=%d losses=%d steps=%d"),
        Passed && FinishedRounds==20?TEXT("PASS"):TEXT("FAIL"),FinishedRounds,Wins,Losses,TotalSteps);
    return Passed && FinishedRounds==20;
}
