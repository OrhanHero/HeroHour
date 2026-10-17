#include "HRResourceFieldVisualLiveTests.h"
#include "ResourceFieldVisualPresenter.h"
#include "../PrototypeGameMode.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "TimerManager.h"

namespace
{
struct FFieldLiveSnapshot
{
    TArray<FTransform> Roots,Pods;
    TArray<float> RootData,PodData;
    bool bReadable=true;
};

FFieldLiveSnapshot ReadField(AResourceFieldVisualPresenter* Presenter)
{
    FFieldLiveSnapshot Result;
    if(!IsValid(Presenter)){Result.bReadable=false;return Result;}
    TArray<UInstancedStaticMeshComponent*> Components;Presenter->GetComponents(Components);
    if(Components.Num()!=2)Result.bReadable=false;
    for(auto* Component:Components)
    {
        const bool bRoot=Component->GetName()==TEXT("OrganicRootLinks");
        auto& Transforms=bRoot?Result.Roots:Result.Pods;
        auto& Data=bRoot?Result.RootData:Result.PodData;
        for(int32 I=0;I<Component->GetInstanceCount();++I)
        {
            FTransform Transform;
            Result.bReadable &= Component->GetInstanceTransform(I,Transform,true);
            Transforms.Add(Transform);
        }
        // UE5.8 InstancedStaticMeshComponent.h declares this public array and
        // specifies its instance-major NumCustomDataFloats * InstanceCount layout.
        Data=Component->PerInstanceSMCustomData;
        Result.bReadable &= Component->NumCustomDataFloats==2 && Data.Num()==Transforms.Num()*2;
    }
    return Result;
}

struct FFieldLiveState
{
    TWeakObjectPtr<UWorld> World;
    TWeakObjectPtr<APlayerController> Controller;
    TWeakObjectPtr<AResourceFieldVisualPresenter> Presenter;
    TArray<TWeakObjectPtr<UInstancedStaticMeshComponent>> OldComponents;
    TArray<FResourceFieldVisualSample> Samples;
    FFieldLiveSnapshot Full;
    FTimerHandle Timer;
    int32 Stage=0,Count=0;
    bool bPassed=true;

    void Check(bool Condition,const TCHAR* Label)
    {
        ++Count;bPassed &= Condition;
        UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_LIVE_TEST %s: %s"),Label,Condition?TEXT("PASS"):TEXT("FAIL"));
    }
    void Finish()
    {
        if(auto* W=World.Get())W->GetTimerManager().ClearTimer(Timer);
        if(auto* P=Presenter.Get())P->Destroy();
        UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_LIVE_TEST_COUNT=%d"),Count);
        UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_LIVE_TEST_RESULT=%s"),bPassed?TEXT("PASS"):TEXT("FAIL"));
        if(auto* PC=Controller.Get())PC->ConsoleCommand(TEXT("quit"),true);
    }
    void Advance()
    {
        auto* P=Presenter.Get();
        if(Stage<4 && !IsValid(P))
        {
            Check(false,TEXT("isolated presenter remains valid during live phases"));Finish();return;
        }
        if(Stage==0)
        {
            Full=ReadField(P);
            Check(Full.bReadable && P->GetEdgeCount()==24 && Full.Roots.Num()==72 && Full.Pods.Num()==64,
                TEXT("real timer produces 24 connected edges 72 root segments and 64 pods"));
            bool Bounded=true,Nonblocking=!P->GetActorEnableCollision();
            TArray<UInstancedStaticMeshComponent*> Components;P->GetComponents(Components);
            for(auto* C:Components)
            {
                OldComponents.Add(C);
                Nonblocking &= C->GetCollisionEnabled()==ECollisionEnabled::NoCollision
                    && !C->GetGenerateOverlapEvents() && !C->CanEverAffectNavigation() && !C->CastShadow;
                for(int32 I=0;I<C->GetInstanceCount();++I)
                {
                    FTransform T;Bounded &= C->GetInstanceTransform(I,T,true) && !T.ContainsNaN();
                    const FBox Bounds=C->GetStaticMesh()->GetBoundingBox().TransformBy(T);
                    Bounded &= Bounds.IsValid && Bounds.Min.X>198000 && Bounds.Max.X<202000
                        && Bounds.Min.Y>198000 && Bounds.Max.Y<202000
                        && Bounds.Min.Z>-100 && Bounds.Max.Z<200;
                }
            }
            Check(Bounded && Nonblocking,TEXT("actual original mesh bounds stay local and all components remain nonblocking"));
            bool Active=true;
            for(const auto* Data:{&Full.RootData,&Full.PodData})for(int32 I=0;I+1<Data->Num();I+=2)
                Active &= (*Data)[I]==1.f && (*Data)[I+1]==1.f;
            Check(Active && Full.RootData.Num()>0 && Full.PodData.Num()>0,
                TEXT("visible full instances carry active and visible material inputs"));
            Check(Full.Roots.Num()>=3 && !Full.Roots[0].GetRotation().Equals(Full.Roots[1].GetRotation(),.001f),
                TEXT("connected root segments follow a curved rather than one straight orientation"));
            for(auto& S:Samples)
            {
                S.bCurrentlyVisible=false;S.Activity=0;S.bFiniteDepleted=true;
                S.Position+=FVector(5000,6000,7000);
            }
            P->SubmitSnapshot(Samples);
        }
        else if(Stage==1)
        {
            const auto Hidden=ReadField(P);
            bool Same=Hidden.bReadable && Hidden.Roots.Num()==Full.Roots.Num() && Hidden.Pods.Num()==Full.Pods.Num();
            if(Same)
            {
                for(int32 I=0;I<Full.Roots.Num();++I)Same &= Full.Roots[I].Equals(Hidden.Roots[I],.001f);
                for(int32 I=0;I<Full.Pods.Num();++I)Same &= Full.Pods[I].Equals(Hidden.Pods[I],.001f);
            }
            Check(Same && P->GetEdgeCount()==24,TEXT("hidden regrowth depletion and position changes leave last-seen actual geometry unchanged"));
            bool Masked=Hidden.RootData.Num()>0 && Hidden.PodData.Num()>0;
            for(const auto* Data:{&Hidden.RootData,&Hidden.PodData})for(int32 I=0;I+1<Data->Num();I+=2)
                Masked &= (*Data)[I]==1.f && (*Data)[I+1]==0.f;
            Check(Masked,TEXT("hidden actual instance visibility input is zero while last-seen activity is retained"));
            for(auto& S:Samples){S.bCurrentlyVisible=true;S.Position-=FVector(5000,6000,7000);}
            P->SubmitSnapshot(Samples);
        }
        else if(Stage==2)
        {
            const auto Empty=ReadField(P);
            Check(Empty.bReadable && Empty.Roots.IsEmpty() && Empty.Pods.IsEmpty()
                && Empty.RootData.IsEmpty() && Empty.PodData.IsEmpty() && P->GetEdgeCount()==0,
                TEXT("visible finite depletion removes actual instances and all material instance inputs"));
            P->ResetPresentation();
            for(auto& S:Samples){S.Activity=1.f;S.bFiniteDepleted=false;}
            P->SubmitSnapshot(Samples);
        }
        else if(Stage==3)
        {
            const auto Restored=ReadField(P);
            Check(Restored.bReadable && Restored.Roots.Num()==72 && Restored.Pods.Num()==64 && P->GetEdgeCount()==24,
                TEXT("reset followed by real timer restores exact original instance budgets"));
            TArray<UInstancedStaticMeshComponent*> Components;P->GetComponents(Components);
            Check(Components.Num()==2,TEXT("reset does not accumulate additional components"));
            Check(P->Destroy(),TEXT("isolated live presenter destruction succeeds"));
        }
        else
        {
            bool Unregistered=true;
            for(const auto& Component:OldComponents)Unregistered &= !Component.IsValid() || !Component->IsRegistered();
            Check(Unregistered,TEXT("destroy unregisters every retained original ISM component"));
            UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_LIVE_SHADER_PIXELS=NOT_MEASURED instance inputs are not a pixel shader proof"));
            Finish();return;
        }
        ++Stage;
    }
};
}

void RunHRResourceFieldVisualLiveTests(APrototypeGameMode* Mode)
{
    auto State=MakeShared<FFieldLiveState>();
    UWorld* World=Mode?Mode->GetWorld():nullptr;
    State->World=World;State->Controller=World?World->GetFirstPlayerController():nullptr;
    if(!World){State->Check(false,TEXT("live test requires an actual game world"));State->Finish();return;}
    FActorSpawnParameters Parameters;Parameters.ObjectFlags|=RF_Transient;
    Parameters.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* P=World->SpawnActor<AResourceFieldVisualPresenter>(FVector::ZeroVector,FRotator::ZeroRotator,Parameters);
    State->Presenter=P;
    if(!P){State->Check(false,TEXT("isolated live presenter spawns"));State->Finish();return;}
    auto* RootMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/SM_TFV10_OrganicRootSegment.SM_TFV10_OrganicRootSegment"));
    auto* PodMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/SM_TFV10_SeedPod.SM_TFV10_SeedPod"));
    auto* RootMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/M_TFV10_OrganicRoot.M_TFV10_OrganicRoot"));
    auto* PodMaterial=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/ReferenceV10/TempestField/M_TFV10_SeedPod.M_TFV10_SeedPod"));
    P->Configure(RootMesh,PodMesh,RootMaterial,PodMaterial);
    State->Check(P->HasUsableAssets(),TEXT("live fixture loads all four real V10 original assets"));
    if(!P->HasUsableAssets()){State->Finish();return;}
    const float Offset[]={0,-260,260,520};
    for(int32 Y=0;Y<4;++Y)for(int32 X=0;X<4;++X)
    {
        FResourceFieldVisualSample S;S.CellId=State->Samples.Num();
        S.Position=FVector(200000+Offset[X],200000+Offset[Y],35);
        S.Activity=1;S.bCurrentlyVisible=true;S.bExplored=true;State->Samples.Add(S);
    }
    P->SubmitSnapshot(State->Samples);
    World->GetTimerManager().SetTimer(State->Timer,FTimerDelegate::CreateLambda([State](){State->Advance();}),.35f,true);
    UE_LOG(LogTemp,Display,TEXT("FIELD_VISUAL_LIVE_TEST_STARTED phases use natural 0.35s world timers; authoritative simulation untouched"));
}
