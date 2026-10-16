#include "ResourceFieldVisualPresenter.h"
#include "Algo/Reverse.h"
#include "HRResourceField.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Misc/ScopeExit.h"

bool RunHRResourceFieldVisualSelfTests(UWorld* TestWorld)
{
    bool Passed=true;int32 Count=0;
    auto Check=[&](bool Condition,const TCHAR* Label)
    {
        ++Count;Passed &= Condition;
        UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_TEST %s: %s"),Label,Condition?TEXT("PASS"):TEXT("FAIL"));
    };
    // Policy checks use the exact memory/edge functions used by PresentPending.
    // No timer advancement, engine frame-counter mutation or force-present test hook.
    FHRTempestFieldDefinition Definition;
    Definition.Capacity=100;Definition.InitialAmount=65;Definition.SeedFloor=5;
    Definition.RegrowthPerSecond=4;Definition.RegrowthDelay=2;
    FHRResourceField Resource;
    Check(Resource.Reset(Definition,EHREconomyStyle::RegrowingField,16),TEXT("real resource baseline initializes"));
    const double TotalBefore=Resource.GetTotalAmount(),TimeBefore=Resource.GetSimulationSeconds();
    TArray<FResourceFieldVisualSample> Samples;
    const float Offsets[]={0,-260,260,520};
    for(int32 Y=0;Y<4;++Y)for(int32 X=0;X<4;++X)
    {
        FResourceFieldVisualSample S;S.CellId=Samples.Num();S.Position=FVector(Offsets[X],Offsets[Y],35);
        S.Activity=Resource.GetHarvestableAmount(S.CellId)/(Definition.Capacity-Definition.SeedFloor);
        S.bCurrentlyVisible=true;S.bExplored=true;Samples.Add(S);
    }
    FResourceFieldVisualMemory Memory;
    auto Display=Memory.Observe(Samples);
    const auto Edges=BuildResourceFieldVisualEdges(Display);
    Check(Display.Num()==16 && Edges.Num()==24,TEXT("unsorted spatial cell offsets form a connected 4x4 field"));
    TSet<int32> Reachable;Reachable.Add(0);
    for(int32 Pass=0;Pass<16;++Pass)for(const auto& Edge:Edges)
    {
        if(Reachable.Contains(Edge.X))Reachable.Add(Edge.Y);
        if(Reachable.Contains(Edge.Y))Reachable.Add(Edge.X);
    }
    TSet<FIntPoint> UniqueEdges;bool DirectOnly=true;
    for(const auto& Edge:Edges)
    {
        UniqueEdges.Add(Edge);
        const FVector Delta=Display[Edge.Y].Position-Display[Edge.X].Position;
        DirectOnly &= FMath::IsNearlyEqual(Delta.Size2D(),260.,.01);
    }
    Check(Reachable.Num()==16 && UniqueEdges.Num()==24 && DirectOnly,TEXT("all cells connect once through direct neighbours without diagonal or skipped links"));
    auto PairIds=[&](const TArray<FResourceFieldVisualSample>& Cells)
    {
        TSet<FIntPoint> Pairs;
        for(const auto& Edge:BuildResourceFieldVisualEdges(Cells))
        {
            const int32 A=Cells[Edge.X].CellId,B=Cells[Edge.Y].CellId;
            Pairs.Add(FIntPoint(FMath::Min(A,B),FMath::Max(A,B)));
        }
        return Pairs;
    };
    const auto BeforePairs=PairIds(Display);
    Algo::Reverse(Display);
    const auto ReversedPairs=PairIds(Display);
    bool SamePairs=BeforePairs.Num()==ReversedPairs.Num();
    for(const auto& Pair:BeforePairs)SamePairs &= ReversedPairs.Contains(Pair);
    Check(SamePairs,TEXT("topology is spatially deterministic when input order changes"));

    // Freeze a partially harvested observation, then alter hidden authoritative inputs.
    Samples[0].Activity=.2f;Display=Memory.Observe(Samples);
    const FVector ObservedPosition=Samples[0].Position;
    Samples[0].bCurrentlyVisible=false;Samples[0].Activity=1.f;
    Samples[0].Position+=FVector(700,900,0);Samples[0].bFiniteDepleted=true;
    Display=Memory.Observe(Samples);
    const auto* Hidden=Display.FindByPredicate([](const auto& S){return S.CellId==0;});
    Check(Hidden && Hidden->Activity==.2f && Hidden->Position==ObservedPosition
        && !Hidden->bFiniteDepleted && !Hidden->bCurrentlyVisible,
        TEXT("hidden regrowth position and depletion cannot replace last observed state"));
    Samples[0].bExplored=false;Display=Memory.Observe(Samples);
    Check(!Display.ContainsByPredicate([](const auto& S){return S.CellId==0;}),TEXT("unexplored cell remains absent even with an old cache"));
    Samples[0].bExplored=true;Samples[0].bCurrentlyVisible=true;
    Samples[0].Position=ObservedPosition;Samples[0].Activity=0.f;Samples[0].bFiniteDepleted=true;
    Display=Memory.Observe(Samples);
    bool FiniteLinks=false;
    for(const auto& Edge:BuildResourceFieldVisualEdges(Display))
        FiniteLinks |= Display[Edge.X].CellId==0 || Display[Edge.Y].CellId==0;
    Check(!FiniteLinks,TEXT("observed finite depletion removes every connection to the empty cell"));
    Memory.Reset();for(auto& S:Samples){S.bCurrentlyVisible=false;S.bExplored=true;}
    Check(Memory.Observe(Samples).IsEmpty(),TEXT("team reset cannot expose former team last-seen cells"));
    Check(Resource.GetTotalAmount()==TotalBefore && Resource.GetSimulationSeconds()==TimeBefore
        && Resource.GetReservedCell(nullptr)==INDEX_NONE,TEXT("presentation policy never advances or debits authoritative resources"));
    bool StockUnchanged=true;
    for(const auto& Cell:Resource.GetCells())StockUnchanged &= Cell.Amount==65.f && !Cell.ReservedCollector.IsValid();
    Check(StockUnchanged,TEXT("all sixteen authoritative amounts and reservation owners remain unchanged"));

    auto* RootMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/SM_TFV10_OrganicRootSegment.SM_TFV10_OrganicRootSegment"));
    auto* PodMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/SM_TFV10_SeedPod.SM_TFV10_SeedPod"));
    auto* RootMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/M_TFV10_OrganicRoot.M_TFV10_OrganicRoot"));
    auto* PodMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/M_TFV10_SeedPod.M_TFV10_SeedPod"));
    Check(RootMesh && PodMesh && RootMaterial && PodMaterial,TEXT("actual original V10 mesh and material assets load"));
    TArray<AResourceFieldVisualPresenter*> Actors;
    ON_SCOPE_EXIT {for(auto* Actor:Actors)if(IsValid(Actor))Actor->Destroy();};
    auto Spawn=[&]()
    {
        if(!TestWorld)return static_cast<AResourceFieldVisualPresenter*>(nullptr);
        FActorSpawnParameters Parameters;Parameters.ObjectFlags|=RF_Transient;
        Parameters.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        auto* Actor=TestWorld->SpawnActor<AResourceFieldVisualPresenter>(FVector::ZeroVector,FRotator::ZeroRotator,Parameters);
        if(Actor)Actors.Add(Actor);return Actor;
    };
    int32 ComponentCount=0;bool Nonblocking=true,Configured=true;
    for(int32 Field=0;Field<3;++Field)
    {
        auto* Actor=Spawn();if(!Actor){Configured=false;continue;}
        Actor->Configure(RootMesh,PodMesh,RootMaterial,PodMaterial);
        Configured &= Actor->HasUsableAssets();
        TArray<UInstancedStaticMeshComponent*> Components;Actor->GetComponents(Components);
        ComponentCount+=Components.Num();
        Nonblocking &= !Actor->GetActorEnableCollision();
        for(auto* Component:Components)
            Nonblocking &= Component->GetCollisionEnabled()==ECollisionEnabled::NoCollision
                && !Component->GetGenerateOverlapEvents() && !Component->CanEverAffectNavigation()
                && !Component->CastShadow;
    }
    Check(Actors.Num()==3 && Configured && ComponentCount==6 && Nonblocking,
        TEXT("three real presenters use six nonblocking non-navigation non-shadow components"));
    if(!Actors.IsEmpty())
    {
        auto* Actor=Actors[0];
        Actor->Configure(nullptr,PodMesh,RootMaterial,PodMaterial);
        const bool MissingMesh=!Actor->HasUsableAssets();
        Actor->Configure(RootMesh,PodMesh,nullptr,PodMaterial);
        Check(MissingMesh && !Actor->HasUsableAssets(),TEXT("missing explicit assets trigger fallback even if a mesh has default materials"));
        Actor->Configure(RootMesh,PodMesh,RootMaterial,PodMaterial);
        TArray<UActorComponent*> Before;Actor->GetComponents(Before);
        for(int32 Restart=0;Restart<10;++Restart){Actor->SubmitSnapshot(Samples);Actor->ResetPresentation();}
        TArray<UActorComponent*> After;Actor->GetComponents(After);
        Check(Before.Num()==After.Num() && Actor->GetPodCount()==0 && Actor->GetRootSegmentCount()==0 && Actor->GetEdgeCount()==0,
            TEXT("ten immediate submit-reset cycles clear pending work without accumulating components"));
    }
    UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_ENGINE_TICK_PENDING=positive ISM output, shader emission and 5Hz cadence require latent real-world test"));
    UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_TEST_COUNT=%d"),Count);
    UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_TEST_RESULT=%s"),Passed?TEXT("PASS"):TEXT("FAIL"));
    return Passed;
}
