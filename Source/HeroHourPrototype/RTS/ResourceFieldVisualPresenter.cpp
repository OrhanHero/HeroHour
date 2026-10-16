#include "ResourceFieldVisualPresenter.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "TimerManager.h"

AResourceFieldVisualPresenter::AResourceFieldVisualPresenter()
{
    PrimaryActorTick.bCanEverTick=false;
    SetActorEnableCollision(false);
    Roots=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("OrganicRootLinks"));
    SetRootComponent(Roots);
    Pods=CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ResourcePods"));
    Pods->SetupAttachment(Roots);
    for(auto* Component:{Roots.Get(),Pods.Get()})
    {
        Component->SetMobility(EComponentMobility::Movable);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetCanEverAffectNavigation(false);
        Component->SetReceivesDecals(false);
        Component->SetCastShadow(false);
        Component->NumCustomDataFloats=2; // Activity, current visibility: material emission gated by both.
    }
}

void AResourceFieldVisualPresenter::Configure(UStaticMesh* RootMesh,UStaticMesh* PodMesh,UMaterialInterface* RootMaterial,UMaterialInterface* PodMaterial)
{
    Roots->SetStaticMesh(RootMesh);Pods->SetStaticMesh(PodMesh);
    Roots->SetMaterial(0,RootMaterial);Pods->SetMaterial(0,PodMaterial);
    bConfiguredAssets=RootMesh && PodMesh && RootMaterial && PodMaterial;
    bHasSignature=false;
}

void AResourceFieldVisualPresenter::SubmitSnapshot(const TArray<FResourceFieldVisualSample>& Samples)
{
    Pending.Reset();TSet<int32> Unique;
    for(const auto& Sample:Samples)
    {
        if(Pending.Num()>=MaxCells)break;
        if(Sample.CellId<0 || Unique.Contains(Sample.CellId) || Sample.Position.ContainsNaN() || !FMath::IsFinite(Sample.Activity))continue;
        Unique.Add(Sample.CellId);Pending.Add(Sample);
    }
    Pending.Sort([](const auto& A,const auto& B){return A.CellId<B.CellId;});
    // No per-frame actor tick and at most five mesh updates/second, independent of simulation cadence.
    if(!GetWorldTimerManager().IsTimerActive(PresentationTimer))
        GetWorldTimerManager().SetTimer(PresentationTimer,this,&AResourceFieldVisualPresenter::PresentPending,.2f,true);
}

TArray<FResourceFieldVisualSample> FResourceFieldVisualMemory::Observe(const TArray<FResourceFieldVisualSample>& Samples)
{
    TArray<FResourceFieldVisualSample> Display;
    TSet<int32> Submitted;
    for(const auto& Incoming:Samples)
    {
        Submitted.Add(Incoming.CellId);
        if(Incoming.bCurrentlyVisible)
        {
            auto Seen=Incoming;Seen.Activity=FMath::Clamp(Seen.Activity,0.f,1.f);
            LastSeen.Add(Seen.CellId,Seen);
        }
        // Hidden current activity/depletion is NEVER read for display: only last observed values.
        const auto* Cached=LastSeen.Find(Incoming.CellId);
        if(!Incoming.bExplored || !Cached)continue;
        auto Sample=*Cached;Sample.bCurrentlyVisible=Incoming.bCurrentlyVisible;
        Display.Add(Sample);
    }
    for(auto It=LastSeen.CreateIterator();It;++It)if(!Submitted.Contains(It.Key()))It.RemoveCurrent();
    return Display;
}

TArray<FIntPoint> BuildResourceFieldVisualEdges(const TArray<FResourceFieldVisualSample>& Display)
{
    TArray<FIntPoint> Edges;
    for(int32 A=0;A<Display.Num();++A)
    {
        if(Display[A].bFiniteDepleted)continue;
        for(int32 B=A+1;B<Display.Num() && Edges.Num()<AResourceFieldVisualPresenter::MaxEdges;++B)
        {
            if(Display[B].bFiniteDepleted)continue;
            const FVector Delta=Display[B].Position-Display[A].Position;
            const bool bRow=FMath::Abs(Delta.Y)<1.f,bColumn=FMath::Abs(Delta.X)<1.f;
            if(!(bRow^bColumn) || Delta.SizeSquared2D()>FMath::Square(300.f))continue;
            bool bBetween=false;
            for(int32 C=0;C<Display.Num();++C)
            {
                if(C==A || C==B)continue;
                const FVector D=Display[C].Position-Display[A].Position;
                bBetween|=(bRow?FMath::Abs(D.Y)<1.f:FMath::Abs(D.X)<1.f)
                    && FVector::DotProduct(D,Delta)>0.f && D.SizeSquared2D()<Delta.SizeSquared2D();
            }
            if(!bBetween)Edges.Emplace(A,B);
        }
    }
    return Edges;
}

void AResourceFieldVisualPresenter::PresentPending()
{
    if(!HasUsableAssets())
    {
        Roots->ClearInstances();Pods->ClearInstances();EdgeCount=0;bHasSignature=false;
        return;
    }
    const TArray<FResourceFieldVisualSample> Display=Memory.Observe(Pending);
    uint32 Signature=0;
    for(const auto& Sample:Display)
    {
        Signature=HashCombine(Signature,HashCombine(GetTypeHash(Sample.CellId),GetTypeHash(FMath::RoundToInt(Sample.Activity*20.f))));
        Signature=HashCombine(Signature,GetTypeHash(Sample.bCurrentlyVisible));
        Signature=HashCombine(Signature,GetTypeHash(Sample.bFiniteDepleted));
        Signature=HashCombine(Signature,GetTypeHash(Sample.Position));
    }
    if(bHasSignature && Signature==LastSignature)return;
    LastSignature=Signature;bHasSignature=true;Roots->ClearInstances();Pods->ClearInstances();EdgeCount=0;
    auto Add=[&](UInstancedStaticMeshComponent* Component,const FTransform& Transform,float Activity,bool bVisible)
    {
        const int32 Instance=Component->AddInstance(Transform,true);
        Component->SetCustomDataValue(Instance,0,Activity,false);
        Component->SetCustomDataValue(Instance,1,bVisible?1.f:0.f,false);
    };
    for(int32 A=0;A<Display.Num();++A)
    {
        const auto& Cell=Display[A];if(Cell.bFiniteDepleted)continue;
        for(int32 Pod=0;Pod<4 && Pods->GetInstanceCount()<MaxPods;++Pod)
        {
            const float Angle=(Cell.CellId*47+Pod*103)*PI/180.f;
            const float Radius=Pod==0?18.f:42.f+((Cell.CellId+Pod)%3)*13.f;
            FVector Position=Cell.Position+FVector(FMath::Cos(Angle)*Radius,FMath::Sin(Angle)*Radius,-35.f);
            const float Scale=.18f+Cell.Activity*(.30f+Pod*.035f);
            Add(Pods,FTransform(FRotator(0,Angle*180.f/PI,0),Position,FVector(Scale,Scale,Scale)),Cell.Activity,Cell.bCurrentlyVisible);
        }
    }
    for(const FIntPoint& Pair:BuildResourceFieldVisualEdges(Display))
    {
            const auto& Cell=Display[Pair.X];const auto& Other=Display[Pair.Y];
            const FVector Delta=Other.Position-Cell.Position;
            ++EdgeCount;
            const FVector Side=FVector(-Delta.Y,Delta.X,0).GetSafeNormal();
            const float Bend=((Cell.CellId+Other.CellId)%2?1.f:-1.f)*(27.f+((Cell.CellId*7+Other.CellId)%19));
            auto Curve=[&](float T){return FMath::Lerp(Cell.Position,Other.Position,T)+Side*(FMath::Sin(PI*T)*Bend)+FVector(0,0,-30.f+FMath::Sin(PI*T)*5.f);};
            const float Activity=FMath::Min(Cell.Activity,Other.Activity);
            for(int32 Segment=0;Segment<SegmentsPerEdge;++Segment)
            {
                const FVector Start=Curve(Segment/3.f),End=Curve((Segment+1)/3.f),Direction=End-Start;
                const float Width=.35f+Activity*.65f;
                Add(Roots,FTransform(Direction.Rotation(),(Start+End)*.5f,FVector(Direction.Size()/100.f,Width,Width)),Activity,Cell.bCurrentlyVisible&&Other.bCurrentlyVisible);
            }
    }
    Roots->MarkRenderStateDirty();Pods->MarkRenderStateDirty();
}

int32 AResourceFieldVisualPresenter::GetPodCount()const{return Pods->GetInstanceCount();}
int32 AResourceFieldVisualPresenter::GetRootSegmentCount()const{return Roots->GetInstanceCount();}

bool AResourceFieldVisualPresenter::HasUsableAssets()const
{
    return bConfiguredAssets && Roots->GetStaticMesh() && Pods->GetStaticMesh() && Roots->GetMaterial(0) && Pods->GetMaterial(0);
}

void AResourceFieldVisualPresenter::ResetPresentation()
{
    GetWorldTimerManager().ClearTimer(PresentationTimer);Pending.Reset();Memory.Reset();
    Roots->ClearInstances();Pods->ClearInstances();EdgeCount=0;bHasSignature=false;
}

void AResourceFieldVisualPresenter::EndPlay(const EEndPlayReason::Type Reason)
{
    ResetPresentation();Super::EndPlay(Reason);
}
