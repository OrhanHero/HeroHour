#include "../PrototypeGameMode.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Sound/SoundWave.h"

// Real runtime assets and evaluated component poses, also exercised in cooked builds.
bool RunHRPresentationSelfTests(UWorld* World)
{
    bool Passed=true;
    int32 Count=0;
    auto Check=[&](bool Condition,const TCHAR* Label)
    {
        ++Count;Passed&=Condition;
        UE_LOG(LogTemp,Display,TEXT("PRESENTATION_TEST %s: %s"),Label,Condition?TEXT("PASS"):TEXT("FAIL"));
    };
    auto OwnMaterial=[](UMaterialInterface* Material)
    {
        return Material && Material->GetMaterial() && Material->GetMaterial()->GetPathName().StartsWith(TEXT("/Game/HERO/"));
    };
    APrototypeGameMode* Mode=World?World->GetAuthGameMode<APrototypeGameMode>():nullptr;
    Check(Mode!=nullptr,TEXT("presentation test runs inside actual HeroHour world"));
    APrototypeUnit* Infantry=Mode?Mode->SpawnUnit(0,FVector(-6500,-2500,35)):nullptr;
    if(Infantry) Infantry->ConfigureRole(EPrototypeRole::Infantry);
    USkeletalMeshComponent* Rig=Infantry?Infantry->FindComponentByClass<USkeletalMeshComponent>():nullptr;
    USkeletalMesh* Asset=Rig?Rig->GetSkeletalMeshAsset():nullptr;
    Check(Rig && Rig->IsVisible() && Asset && Infantry->FindComponentByClass<UStaticMeshComponent>() && !Infantry->FindComponentByClass<UStaticMeshComponent>()->IsVisible(),
        TEXT("spawned infantry displays skeletal mesh instead of static fallback"));
    if(Asset)
    {
        const FBox Box=Asset->GetBounds().GetBox();
        const FVector Size=Box.GetSize();
        UE_LOG(LogTemp,Display,TEXT("PRESENTATION_RIG_BOUNDS size=%s min=%s"),*Size.ToString(),*Box.Min.ToString());
        Check(FMath::Abs(Size.X-90.f)<18.f && FMath::Abs(Size.Y-70.f)<14.f && FMath::Abs(Size.Z-180.f)<36.f
            && FMath::Abs(Box.Min.Z+35.f)<10.f,TEXT("imported infantry has human scale and authored minus35cm origin"));
        bool Materials=Rig->GetNumMaterials()>0;
        for(int32 Index=0;Index<Rig->GetNumMaterials();++Index)
        {
            UMaterialInterface* Material=Rig->GetMaterial(Index);
            UE_LOG(LogTemp,Display,TEXT("PRESENTATION_RIG_MATERIAL slot=%d interface=%s base=%s"),Index,*GetPathNameSafe(Material),*GetPathNameSafe(Material?Material->GetMaterial():nullptr));
            Materials&=OwnMaterial(Material);
        }
        Check(Materials,TEXT("all actual infantry material slots resolve to original HeroHour materials"));
    }
    else
    {
        Check(false,TEXT("imported infantry has human scale and authored minus35cm origin"));
        Check(false,TEXT("all actual infantry material slots resolve to original HeroHour materials"));
    }
    const TCHAR* Clips[]={TEXT("Idle"),TEXT("Walk"),TEXT("Fire")};
    const float Lengths[]={2.f,.8f,.3f};
    UAnimSequence* Animations[3]={nullptr,nullptr,nullptr};
    for(int32 Index=0;Index<3;++Index)
    {
        const FString Name=FString::Printf(TEXT("AN_HR_Infantry_%s"),Clips[Index]);
        Animations[Index]=LoadObject<UAnimSequence>(nullptr,*FString::Printf(TEXT("/Game/HERO/InfantryRigV4/%s.%s"),*Name,*Name));
        Check(Asset && Animations[Index] && Animations[Index]->GetSkeleton()==Asset->GetSkeleton()
            && FMath::Abs(Animations[Index]->GetPlayLength()-Lengths[Index])<.04f,
            *FString::Printf(TEXT("%s clip has correct duration and exact runtime skeleton"),Clips[Index]));
    }
    if(Rig && Asset && Infantry && Animations[1] && Animations[2])
    {
        Infantry->SetActorTickEnabled(false);
        Rig->VisibilityBasedAnimTickOption=EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        Rig->bEnableUpdateRateOptimizations=false;
        const FTransform Origin=Infantry->GetActorTransform();
        const int32 Left=Rig->GetBoneIndex(TEXT("foot_l")),Right=Rig->GetBoneIndex(TEXT("foot_r")),Spine=Rig->GetBoneIndex(TEXT("spine"));
        Check(Left!=INDEX_NONE && Right!=INDEX_NONE && Spine!=INDEX_NONE,TEXT("runtime skeleton contains both feet and recoil spine"));
        auto Evaluate=[Rig](UAnimSequence* Clip,float Time)
        {
            Rig->PlayAnimation(Clip,false);
            Rig->SetPosition(Time,false);
            Rig->TickAnimation(0.f,false);
            Rig->RefreshBoneTransforms();
        };
        if(Left!=INDEX_NONE && Right!=INDEX_NONE && Spine!=INDEX_NONE)
        {
            Evaluate(Animations[1],0.f);
            const FVector LeftStart=Rig->GetBoneTransform(Left).GetLocation(),RightStart=Rig->GetBoneTransform(Right).GetLocation();
            Evaluate(Animations[1],.2f);
            Check(FVector::Dist(LeftStart,Rig->GetBoneTransform(Left).GetLocation())>1.f
                && FVector::Dist(RightStart,Rig->GetBoneTransform(Right).GetLocation())>1.f,
                TEXT("walk clip actually moves both evaluated feet between zero and point2 seconds"));
            Evaluate(Animations[2],0.f);
            const FQuat SpineStart=Rig->GetBoneTransform(Spine).GetRotation();
            // Recoil peaks at .045s and has already recovered by .2s in the authored .3s clip.
            Evaluate(Animations[2],.045f);
            Check(SpineStart.AngularDistance(Rig->GetBoneTransform(Spine).GetRotation())>.005f,
                TEXT("fire clip actually changes evaluated spine rotation at recoil peak"));
        }
        else
        {
            Check(false,TEXT("walk clip actually moves both evaluated feet between zero and point2 seconds"));
            Check(false,TEXT("fire clip actually changes evaluated spine rotation at recoil peak"));
        }
        Check(Infantry->GetActorTransform().Equals(Origin,.001f),TEXT("in-place animations preserve actor origin and gameplay transform"));
    }
    else
    {
        Check(false,TEXT("runtime skeleton contains both feet and recoil spine"));
        Check(false,TEXT("walk clip actually moves both evaluated feet between zero and point2 seconds"));
        Check(false,TEXT("fire clip actually changes evaluated spine rotation at recoil peak"));
        Check(false,TEXT("in-place animations preserve actor origin and gameplay transform"));
    }
    for(const TCHAR* Role:{TEXT("PowerPlant"),TEXT("Barracks"),TEXT("Radar"),TEXT("Defense"),TEXT("Infantry"),TEXT("Siege")})
    {
        UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/HERO/FrontlinesV4/Meshes/SM_HR_%s.SM_HR_%s"),Role,Role));
        bool Materials=Mesh && !Mesh->GetStaticMaterials().IsEmpty();
        if(Mesh) for(const FStaticMaterial& Slot:Mesh->GetStaticMaterials()) Materials&=OwnMaterial(Slot.MaterialInterface);
        Check(Mesh && Mesh->GetBounds().SphereRadius>1.f && Materials,
            *FString::Printf(TEXT("%s original mesh and all material slots load without fallback"),Role));
    }
    for(const TCHAR* Name:{TEXT("M_PP_Fog"),TEXT("M_FX_Smoke"),TEXT("M_FX_Impact")})
        Check(OwnMaterial(LoadObject<UMaterialInterface>(nullptr,*FString::Printf(TEXT("/Game/HERO/EffectsV4/%s.%s"),Name,Name))),
            *FString::Printf(TEXT("%s effect material loads from cooked content"),Name));
    for(int32 RoleIndex=0;Mode && RoleIndex<static_cast<int32>(EPrototypeRole::Count);++RoleIndex)
    {
        APrototypeUnit* Unit=Mode->SpawnUnit(0,FVector(-14000,-12000,35));
        if(Unit) Unit->ConfigureRole(static_cast<EPrototypeRole>(RoleIndex));
        bool bSlotsMatch=Unit!=nullptr;
        UStaticMeshComponent* Hull=Unit?Unit->FindComponentByClass<UStaticMeshComponent>():nullptr;
        USkeletalMeshComponent* Character=Unit?Unit->FindComponentByClass<USkeletalMeshComponent>():nullptr;
        const bool bCharacter=Character && Character->IsVisible() && Character->GetSkeletalMeshAsset();
        const int32 Num=bCharacter?Character->GetNumMaterials():Hull?Hull->GetNumMaterials():0;
        bSlotsMatch &= Num>0;
        for(int32 Index=0;Index<Num;++Index)
        {
            UMaterialInterface* Expected=bCharacter?Character->GetSkeletalMeshAsset()->GetMaterials()[Index].MaterialInterface.Get()
                :Hull->GetStaticMesh()->GetMaterial(Index);
            UMaterialInterface* Actual=bCharacter?Character->GetMaterial(Index):Hull->GetMaterial(Index);
            bSlotsMatch &= Actual && Expected && Actual->GetMaterial()==Expected->GetMaterial();
        }
        Check(bSlotsMatch,*FString::Printf(TEXT("role %d preserves each authored material slot after default vehicle configuration"),RoleIndex));
        if(Unit) Unit->Destroy();
    }
    for(const TCHAR* Name:{TEXT("A_Impact"),TEXT("A_Rifle"),TEXT("A_Cannon")})
    {
        USoundWave* Sound=LoadObject<USoundWave>(nullptr,*FString::Printf(TEXT("/Game/HERO/EffectsV4/%s.%s"),Name,Name));
        Check(Sound && Sound->GetDuration()>0.f && Sound->GetDuration()<10.f,
            *FString::Printf(TEXT("%s original sound wave has positive finite duration"),Name));
    }
    if(Infantry) Infantry->Destroy();
    UE_LOG(LogTemp,Display,TEXT("PRESENTATION_TEST_COUNT=%d"),Count);
    UE_LOG(LogTemp,Display,TEXT("PRESENTATION_TEST_RESULT=%s"),Passed?TEXT("PASS"):TEXT("FAIL"));
    return Passed;
}
