#include "../PrototypeGameMode.h"
#include "../PrototypeCollectorVisual.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"
#include "Sound/SoundWave.h"
#include "HRRTSDefinitions.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Misc/App.h"
#if WITH_EDITOR
#include "AssetCompilingManager.h"
#endif

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
    for(int32 Team=0;Team<2;++Team)
    for(int32 RoleIndex=0;Mode && RoleIndex<static_cast<int32>(EPrototypeRole::Count);++RoleIndex)
    {
        APrototypeUnit* Unit=Mode->SpawnUnit(Team,FVector(-14000,-12000,35));
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
            if(Expected && Expected->GetName().StartsWith(TEXT("M_CV2_")))
            {
                const FString Name=FString::Printf(TEXT("M_HR_%s_%s"),Mode->GetFactionIndex(Team)==0?TEXT("Aegis"):TEXT("Ember"),*Expected->GetName().RightChop(6));
                Expected=LoadObject<UMaterialInterface>(nullptr,*(TEXT("/Game/HERO/PresentationV5/Materials/")+Name+TEXT(".")+Name));
            }
            bSlotsMatch &= Actual && Expected && Actual->GetMaterial()==Expected->GetMaterial();
        }
        Check(bSlotsMatch,*FString::Printf(TEXT("team %d role %d uses the correct saved faction surface in every authored slot"),Team,RoleIndex));
        const UHRUnitDefinition* Definition=Unit?Unit->Definition:nullptr;
        UTexture2D* Portrait=Definition?Definition->Icon.LoadSynchronous():nullptr;
#if WITH_EDITOR
        if(Portrait)
        {
            TArray<UObject*> PortraitAssets{Portrait};
            FAssetCompilingManager::Get().FinishCompilationForObjects(PortraitAssets);
        }
#endif
        const FString ExpectedPortrait=Definition?TEXT("T_HR_")+Definition->GetName().RightChop(2):FString();
        const FString ExpectedPortraitPath=TEXT("/Game/HERO/PresentationV5/Portraits/")+ExpectedPortrait+TEXT(".")+ExpectedPortrait;
        bool bPortraitResolution=Portrait && Portrait->GetSizeX()==512 && Portrait->GetSizeY()==512;
#if WITH_EDITOR
        // NullRHI deliberately skips UTexture2D::PostLoad's platform-data cache
        // (FApp::CanEverRender is false). No compilation job exists to finish,
        // so GetSizeX/Y are zero for uncooked editor textures in this mode.
        // Validate the actual imported source instead; rendered editor and all
        // cooked runs must still expose the full 512x512 platform texture.
        if(Portrait && !FApp::CanEverRender())
        {
            const FIntPoint ImportedSize=Portrait->GetImportedSize();
            bPortraitResolution=Portrait->Source.IsValid()
                && ImportedSize==FIntPoint(512,512)
                && Portrait->Source.GetSizeX()==512 && Portrait->Source.GetSizeY()==512;
        }
#endif
        UE_LOG(LogTemp,Display,TEXT("PRESENTATION_PORTRAIT actual=%s expected=%s platform=%dx%d can_render=%d"),
            *GetPathNameSafe(Portrait),*ExpectedPortraitPath,Portrait?Portrait->GetSizeX():0,Portrait?Portrait->GetSizeY():0,FApp::CanEverRender());
        Check(Portrait && Portrait->GetPathName()==ExpectedPortraitPath && bPortraitResolution,
            *FString::Printf(TEXT("team %d role %d loads its own faction portrait at full UI resolution"),Team,RoleIndex));
        if(Unit) Unit->Destroy();
    }
    // Validate the rendered faction identity and transformed geometry, not just
    // the soft references: role configuration can otherwise silently substitute
    // a fallback, apply the wrong faction, or leave a scaled asset underground.
    struct FReferenceFixture
    {
        int32 Faction;
        EPrototypeRole Role;
        const TCHAR* Mesh;
        const TCHAR* Surface;
        int32 AtlasSize;
    };
    const FReferenceFixture ReferenceFixtures[]={
        {0,EPrototypeRole::Headquarters,TEXT("SM_HQ9_AegisHQ"),TEXT("Aegis_HQ_V9"),2048},
        {0,EPrototypeRole::Combat,TEXT("SM_HR_Aegis_APC8"),TEXT("Aegis_APC8"),1024},
        {1,EPrototypeRole::Headquarters,TEXT("SM_RV7_EmberHQ"),TEXT("Ember_HQ"),2048},
        {1,EPrototypeRole::Combat,TEXT("SM_HR_Ember_APC8"),TEXT("Ember_APC8"),1024}};
    const int32 AegisTeam=Mode && Mode->GetFactionIndex(0)==0?0:1;
    TSet<UStaticMesh*> ReferenceGeometry;
    for(const FReferenceFixture& Fixture:ReferenceFixtures)
    {
        const EPrototypeRole Role=Fixture.Role;
        const bool bHeadquarters=Role==EPrototypeRole::Headquarters;
        const bool bHQ9=Fixture.Faction==0 && bHeadquarters;
        const FVector Position(-14000.f,-12000.f,bHeadquarters?110.f:35.f);
        const int32 Team=Fixture.Faction==0?AegisTeam:1-AegisTeam;
        APrototypeUnit* Unit=Mode?Mode->SpawnUnit(Team,Position,bHeadquarters):nullptr;
        UStaticMeshComponent* Hull=Unit?Unit->FindComponentByClass<UStaticMeshComponent>():nullptr;
        UStaticMesh* Mesh=Hull?Hull->GetStaticMesh():nullptr;
        const FString MeshRoot=bHQ9?TEXT("/Game/HERO/TrellisHQV9/Meshes/"):TEXT("/Game/HERO/ReferenceV7/Meshes/");
        const FString MeshPath=MeshRoot+Fixture.Mesh+TEXT(".")+Fixture.Mesh;
        Check(Unit && Unit->Role==Role && Hull && Hull->IsVisible() && Mesh
            && Mesh->GetPathName()==MeshPath && Unit->Definition
            && Unit->Definition->StaticMesh.LoadSynchronous()==Mesh && !ReferenceGeometry.Contains(Mesh),
            *FString::Printf(TEXT("%s displays its exact saved %s mesh with geometry distinct from the other reference units"),Fixture.Surface,bHQ9?TEXT("V9"):TEXT("V7")));
        if(Mesh)ReferenceGeometry.Add(Mesh);
        const FBox GroundBounds=Mesh && Hull
            ?Mesh->GetBoundingBox().TransformBy(Hull->GetComponentTransform()):FBox(ForceInit);
        UE_LOG(LogTemp,Display,TEXT("PRESENTATION_REFERENCE_BOUNDS role=%s min=%s max=%s"),
            Fixture.Surface,*GroundBounds.Min.ToString(),*GroundBounds.Max.ToString());
        Check(GroundBounds.IsValid && FMath::Abs(GroundBounds.Min.Z)<1.f
            && GroundBounds.GetSize().Z>(bHeadquarters?650.f:170.f),
            *FString::Printf(TEXT("%s final component transform puts the authored base on world ground"),Fixture.Surface));
        const FVector Size=GroundBounds.GetSize();
        const FBox SourceBounds=Mesh?Mesh->GetBoundingBox():FBox(ForceInit);
        const FVector SourceSize=SourceBounds.GetSize();
        const bool bHQ9Proportions=!bHQ9 || (SourceBounds.IsValid
            && FMath::IsNearlyEqual(SourceBounds.Min.Z,-110.f,1.f)
            && FMath::IsNearlyEqual(SourceSize.X,780.f,1.f)
            && FMath::IsNearlyEqual(SourceSize.Y,618.186f,1.f)
            && FMath::IsNearlyEqual(SourceSize.Z,465.962f,1.f));
        Check(GroundBounds.IsValid && bHQ9Proportions && (bHeadquarters
            ?Size.X>900.f && Size.Y>900.f && Size.X<=2.f*APrototypeGameMode::Footprint(Role)
                && Size.Y<=2.f*APrototypeGameMode::Footprint(Role)
            :Size.X>330.f && Size.X<410.f && Size.Y>170.f && Size.Y<210.f),
            *FString::Printf(TEXT("%s keeps the intended RTS scale and placement envelope"),Fixture.Surface));
        const FString MaterialName=bHQ9?TEXT("M_HQ9_Baked_AegisHQ"):FString(TEXT("M_RV7_Baked_"))+Fixture.Surface;
        const FString MaterialRoot=bHQ9?TEXT("/Game/HERO/TrellisHQV9/Materials/"):TEXT("/Game/HERO/ReferenceV7/Materials/");
        const FString MaterialPath=MaterialRoot+MaterialName+TEXT(".")+MaterialName;
        bool bSavedAtlas=Mesh && Hull && Hull->GetNumMaterials()>0;
        bool bNeutralTint=bSavedAtlas;
        UMaterialInterface* Atlas=Mesh?Mesh->GetMaterial(0):nullptr;
        if(bHQ9)
        {
            UTexture* FictionalEmission=nullptr;
            bSavedAtlas &= Atlas && Hull && Hull->GetMaterialIndex(TEXT("Team"))==0
                && !Atlas->GetTextureParameterValue(FHashedMaterialParameterInfo(TEXT("EmissionMask")),FictionalEmission);
        }
        for(int32 Slot=0;Hull && Mesh && Slot<Hull->GetNumMaterials();++Slot)
        {
            UMaterialInterface* Saved=Mesh->GetMaterial(Slot);
            UMaterialInterface* Actual=Hull->GetMaterial(Slot);
            bSavedAtlas &= Saved && Saved->GetPathName()==MaterialPath && Actual
                && Actual->GetMaterial()==Saved->GetMaterial();
            FLinearColor Tint=FLinearColor::Black;
            bNeutralTint &= Actual && Actual->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),Tint)
                && Tint.Equals(FLinearColor::White,.001f);
        }
        Check(bSavedAtlas && bNeutralTint,
            *FString::Printf(TEXT("%s keeps its own baked atlas in every live slot with neutral tint rather than a second faction recolor"),Fixture.Surface));
        const TArray<const TCHAR*> Channels=bHQ9
            ?TArray<const TCHAR*>{TEXT("Albedo"),TEXT("MetallicRoughness")}
            :TArray<const TCHAR*>{TEXT("Albedo"),TEXT("ORM"),TEXT("EmissionMask")};
        for(const TCHAR* Channel:Channels)
        {
            const bool bAlbedo=FCString::Strcmp(Channel,TEXT("Albedo"))==0;
            const FString TextureName=bHQ9
                ?FString(bAlbedo?TEXT("T_HQ9_AegisHQ_Albedo"):TEXT("T_HQ9_AegisHQ_MR"))
                :FString::Printf(TEXT("T_RV7_%s_%s"),Fixture.Surface,Channel);
            const FString TextureRoot=bHQ9?TEXT("/Game/HERO/TrellisHQV9/Textures/"):TEXT("/Game/HERO/ReferenceV7/Textures/");
            const FString TexturePath=TextureRoot+TextureName+TEXT(".")+TextureName;
            UTexture* Parameter=nullptr;
            const bool bParameter=Atlas && Atlas->GetTextureParameterValue(FHashedMaterialParameterInfo(Channel),Parameter);
            UTexture2D* Texture=Cast<UTexture2D>(Parameter);
            bool bLiveBinding=Hull && Hull->GetNumMaterials()>0;
            for(int32 Slot=0;Hull && Slot<Hull->GetNumMaterials();++Slot)
            {
                UTexture* LiveTexture=nullptr;
                UMaterialInterface* LiveMaterial=Hull->GetMaterial(Slot);
                bLiveBinding &= LiveMaterial
                    && LiveMaterial->GetTextureParameterValue(FHashedMaterialParameterInfo(Channel),LiveTexture)
                    && LiveTexture==Texture;
            }
#if WITH_EDITOR
            if(Texture)
            {
                TArray<UObject*> Textures{Texture};
                FAssetCompilingManager::Get().FinishCompilationForObjects(Textures);
            }
#endif
            bool bResolution=Texture && Texture->GetSizeX()==Fixture.AtlasSize && Texture->GetSizeY()==Fixture.AtlasSize;
#if WITH_EDITOR
            if(Texture && !FApp::CanEverRender())
                bResolution=Texture->Source.IsValid()
                    && Texture->GetImportedSize()==FIntPoint(Fixture.AtlasSize,Fixture.AtlasSize)
                    && Texture->Source.GetSizeX()==Fixture.AtlasSize && Texture->Source.GetSizeY()==Fixture.AtlasSize;
#endif
            UE_LOG(LogTemp,Display,TEXT("PRESENTATION_REFERENCE_TEXTURE model=%s parameter=%s actual=%s expected=%s dimensions=%dx%d"),
                Fixture.Surface,Channel,*GetPathNameSafe(Texture),*TexturePath,Texture?Texture->GetSizeX():0,Texture?Texture->GetSizeY():0);
            Check(bParameter && bLiveBinding && Texture && Texture->GetPathName()==TexturePath && bResolution
                && Texture->SRGB==bAlbedo && Texture->CompressionSettings==(bAlbedo?TC_Default:TC_Masks),
                *FString::Printf(TEXT("%s %s is bound to its exact cooked atlas with correct resolution and color space"),Fixture.Surface,Channel));
        }
        const int32 TeamSlot=Hull?Hull->GetMaterialIndex(TEXT("Team")):INDEX_NONE;
        UMaterialInterface* TeamSurface=TeamSlot!=INDEX_NONE?Hull->GetMaterial(TeamSlot):nullptr;
        FLinearColor RestTint=FLinearColor::Black,HitTint=FLinearColor::Black,RestoredTint=FLinearColor::Black;
        const bool bRest=TeamSurface && TeamSurface->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),RestTint);
        const float HealthBefore=Unit?Unit->Health:0.f;
        if(Unit){Unit->ReceiveHit(1.f);Unit->Tick(.03f);}
        const bool bHit=TeamSurface && TeamSurface->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),HitTint);
        Check(Unit && FMath::IsNearlyEqual(Unit->Health,HealthBefore-1.f,.001f) && bRest && bHit
            && !HitTint.Equals(RestTint,.001f),
            *FString::Printf(TEXT("%s actual damage tick visibly flashes its baked team surface"),Fixture.Surface));
        if(Unit)Unit->Tick(.2f);
        Check(TeamSurface && bRest && TeamSurface->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),RestoredTint)
            && RestTint.Equals(FLinearColor::White,.001f) && RestoredTint.Equals(RestTint,.001f),
            *FString::Printf(TEXT("%s damage flash expires to its original neutral atlas tint"),Fixture.Surface));
        if(Unit)Unit->Destroy();
    }
    // V10 factory uses the selected faction mapping, exact live geometry and
    // actual material bindings; successful imports alone cannot satisfy this.
    {
        APrototypeUnit* Factory=Mode?Mode->SpawnUnit(AegisTeam,FVector(-14000.f,-12000.f,35.f)):nullptr;
        if(Factory)Factory->ConfigureRole(EPrototypeRole::Factory);
        UStaticMeshComponent* Hull=Factory?Factory->FindComponentByClass<UStaticMeshComponent>():nullptr;
        UStaticMesh* Mesh=Hull?Hull->GetStaticMesh():nullptr;
        Check(Factory && Mode->GetFactionIndex(Factory->Team)==0 && Factory->Role==EPrototypeRole::Factory
            && Factory->Definition==Mode->GetRoleDefinition(EPrototypeRole::Factory,AegisTeam)
            && Hull && Hull->IsVisible() && Mesh
            && Mesh->GetPathName()==TEXT("/Game/HERO/TrellisFactoryV10/Meshes/SM_FV10_AegisFactory.SM_FV10_AegisFactory")
            && Factory->Definition && Factory->Definition->StaticMesh.LoadSynchronous()==Mesh,
            TEXT("V10 Aegis factory resolves selected faction and displays exact saved definition mesh"));
        const FBox Source=Mesh?Mesh->GetBoundingBox():FBox(ForceInit);
        const FVector SourceSize=Source.GetSize();
        Check(Source.IsValid && FMath::IsNearlyEqual(Source.Min.Z,-35.f,1.f)
            && FMath::IsNearlyEqual(SourceSize.X,620.f,1.f)
            && FMath::IsNearlyEqual(SourceSize.Y,486.845f,1.f)
            && FMath::IsNearlyEqual(SourceSize.Z,201.627f,1.f),
            TEXT("V10 factory preserves uniform source proportions and authored minus35cm base"));
        const FBox Bounds=Mesh && Hull?Source.TransformBy(Hull->GetComponentTransform()):FBox(ForceInit);
        Check(Bounds.IsValid && FMath::Abs(Bounds.Min.Z)<1.f,
            TEXT("V10 factory actual component transform grounds its authored base at world zero"));
        const FVector Size=Bounds.GetSize();
        Check(Bounds.IsValid && Hull && Hull->GetComponentScale().Equals(FVector(1.7f),.001f)
            && FMath::IsNearlyEqual(Size.X,1054.f,1.f) && FMath::IsNearlyEqual(Size.Y,827.637f,1.f)
            && FMath::IsNearlyEqual(Size.Z,342.766f,1.f)
            && FMath::IsNearlyEqual(2.f*APrototypeGameMode::Footprint(EPrototypeRole::Factory),1080.f,.001f)
            && Size.X<=1080.f && Size.Y<=1080.f,
            TEXT("V10 factory live scale and dimensions fit the authoritative 1080cm placement envelope"));
        UMaterialInterface* Saved=Mesh?Mesh->GetMaterial(0):nullptr;
        UMaterialInterface* Live=Hull?Hull->GetMaterial(0):nullptr;
        FLinearColor Rest=FLinearColor::Black;
        UTexture* FictionalEmission=nullptr;
        Check(Mesh && Mesh->GetStaticMaterials().Num()==1 && Hull && Hull->GetNumMaterials()==1
            && Hull->GetMaterialIndex(TEXT("Team"))==0 && Saved
            && Saved->GetPathName()==TEXT("/Game/HERO/TrellisFactoryV10/Materials/M_FV10_Baked_AegisFactory.M_FV10_Baked_AegisFactory")
            && Cast<UMaterialInstanceDynamic>(Live) && Live->GetMaterial()==Saved->GetMaterial()
            && Live->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),Rest)
            && Rest.Equals(FLinearColor::White,.001f)
            && !Saved->GetTextureParameterValue(FHashedMaterialParameterInfo(TEXT("EmissionMask")),FictionalEmission),
            TEXT("V10 factory live Team material retains exact baked PBR and neutral white Tint without fictional emission"));
        for(bool bAlbedo:{true,false})
        {
            const TCHAR* Parameter=bAlbedo?TEXT("Albedo"):TEXT("MetallicRoughness");
            const FString Name=bAlbedo?TEXT("T_FV10_AegisFactory_Albedo"):TEXT("T_FV10_AegisFactory_MR");
            const FString Path=TEXT("/Game/HERO/TrellisFactoryV10/Textures/")+Name+TEXT(".")+Name;
            UTexture* SavedTexture=nullptr;UTexture* LiveTexture=nullptr;
            const bool bSaved=Saved && Saved->GetTextureParameterValue(FHashedMaterialParameterInfo(Parameter),SavedTexture);
            const bool bLive=Live && Live->GetTextureParameterValue(FHashedMaterialParameterInfo(Parameter),LiveTexture);
            UTexture2D* Texture=Cast<UTexture2D>(SavedTexture);
#if WITH_EDITOR
            if(Texture){TArray<UObject*> Textures{Texture};FAssetCompilingManager::Get().FinishCompilationForObjects(Textures);}
#endif
            bool bSize=Texture && Texture->GetSizeX()==2048 && Texture->GetSizeY()==2048;
#if WITH_EDITOR
            if(Texture && !FApp::CanEverRender())bSize=Texture->Source.IsValid()
                && Texture->GetImportedSize()==FIntPoint(2048,2048)
                && Texture->Source.GetSizeX()==2048 && Texture->Source.GetSizeY()==2048;
#endif
            Check(bSaved && bLive && Texture && LiveTexture==SavedTexture && Texture->GetPathName()==Path
                && bSize && Texture->SRGB==bAlbedo && Texture->CompressionSettings==(bAlbedo?TC_Default:TC_Masks),
                *FString::Printf(TEXT("V10 factory %s resolves exact saved and live 2048 PBR texture with correct color space"),Parameter));
        }
        const float Health=Factory?Factory->Health:0.f;
        if(Factory){Factory->ReceiveHit(1.f);Factory->Tick(.03f);}
        FLinearColor Flash=FLinearColor::Black,Restored=FLinearColor::Black;
        Check(Factory && FMath::IsNearlyEqual(Factory->Health,Health-1.f,.001f) && Live
            && Live->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),Flash)
            && !Flash.Equals(Rest,.001f),TEXT("V10 factory real damage visibly flashes its baked Team material"));
        if(Factory)Factory->Tick(.2f);
        Check(Live && Rest.Equals(FLinearColor::White,.001f)
            && Live->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),Restored)
            && Restored.Equals(Rest,.001f),TEXT("V10 factory damage flash restores its original neutral atlas Tint"));
        if(Factory)Factory->Destroy();
    }
    // Exercise the common legacy surface path independently of the four baked
    // models. Team index is resolved from the selected faction, not hardcoded.
    APrototypeUnit* EmberDepot=Mode?Mode->SpawnUnit(1-AegisTeam,FVector(-14000.f,-12000.f,35.f)):nullptr;
    if(EmberDepot)EmberDepot->ConfigureRole(EPrototypeRole::Depot);
    UStaticMeshComponent* DepotHull=EmberDepot?EmberDepot->FindComponentByClass<UStaticMeshComponent>():nullptr;
    const FName MarkerNames[]={FName(TEXT("Team")),FName(TEXT("Neon"))};
    const FLinearColor MarkerColors[]={FLinearColor(.025f,.32f,.13f),FLinearColor(.025f,.70f,.28f)};
    UMaterialInterface* DepotMarkers[2]={nullptr,nullptr};
    bool bLegacyMarkers=EmberDepot && EmberDepot->Role==EPrototypeRole::Depot
        && Mode->GetFactionIndex(EmberDepot->Team)==1
        && EmberDepot->Definition==Mode->GetRoleDefinition(EPrototypeRole::Depot,EmberDepot->Team);
    for(int32 Index=0;Index<2;++Index)
    {
        const int32 Slot=DepotHull?DepotHull->GetMaterialIndex(MarkerNames[Index]):INDEX_NONE;
        DepotMarkers[Index]=Slot!=INDEX_NONE?DepotHull->GetMaterial(Slot):nullptr;
        FLinearColor Tint=FLinearColor::Black;
        bLegacyMarkers &= Cast<UMaterialInstanceDynamic>(DepotMarkers[Index])
            && DepotMarkers[Index]->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),Tint)
            && Tint.Equals(MarkerColors[Index],.001f);
    }
    Check(bLegacyMarkers,TEXT("legacy Ember refinery resolves the selected faction and preserves its green Team and Neon markers"));
    const float DepotHealth=EmberDepot?EmberDepot->Health:0.f;
    if(EmberDepot){EmberDepot->ReceiveHit(1.f);EmberDepot->Tick(.03f);}
    FLinearColor FlashTint=FLinearColor::Black,NeonDuringHit=FLinearColor::Black;
    const bool bLegacyFlash=EmberDepot && FMath::IsNearlyEqual(EmberDepot->Health,DepotHealth-1.f,.001f)
        && DepotMarkers[0] && DepotMarkers[0]->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),FlashTint)
        && !FlashTint.Equals(MarkerColors[0],.001f)
        && DepotMarkers[1] && DepotMarkers[1]->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),NeonDuringHit)
        && NeonDuringHit.Equals(MarkerColors[1],.001f);
    if(EmberDepot)EmberDepot->Tick(.2f);
    bool bLegacyRestore=bLegacyMarkers && bLegacyFlash;
    for(int32 Index=0;Index<2;++Index)
    {
        FLinearColor Tint=FLinearColor::Black;
        bLegacyRestore &= DepotMarkers[Index]
            && DepotMarkers[Index]->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),Tint)
            && Tint.Equals(MarkerColors[Index],.001f);
    }
    Check(bLegacyRestore,TEXT("legacy Ember refinery damage flash restores green Team tint while Neon remains its authored green"));
    if(EmberDepot)EmberDepot->Destroy();
    for(const TCHAR* Name:{TEXT("RockCluster"),TEXT("DryScrub"),TEXT("CurbStrip"),TEXT("LowBarrier"),TEXT("SupplyCrates"),TEXT("PipeBundle")})
    {
        const FString DetailAsset=FString::Printf(TEXT("SM_PV5_%s"),Name);
        UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,*(TEXT("/Game/HERO/PresentationV5/Meshes/")+DetailAsset+TEXT(".")+DetailAsset));
        Check(Mesh && Mesh->GetBounds().SphereRadius>1.f,*FString::Printf(TEXT("%s original detail mesh is cooked"),*DetailAsset));
    }
    for(const TCHAR* Name:{TEXT("M_PV5_DesertGround"),TEXT("M_PV5_WeatheredAsphalt")})
        Check(OwnMaterial(LoadObject<UMaterialInterface>(nullptr,*FString::Printf(TEXT("/Game/HERO/PresentationV5/Materials/%s.%s"),Name,Name))),
            *FString::Printf(TEXT("%s world surface is cooked"),Name));
    int32 DetailCount=0;bool bNonBlocking=true;
    for(TActorIterator<AStaticMeshActor> It(World);It;++It)
        if(It->ActorHasTag(TEXT("HERO_DecorationV5")))
        { ++DetailCount;bNonBlocking &= It->GetStaticMeshComponent()->GetCollisionEnabled()==ECollisionEnabled::NoCollision; }
    Check(DetailCount>=200 && bNonBlocking,TEXT("authored world details load from the map without adding collision"));
    // Fixed authored contracts from the V6 manifests, checked after loading the map
    // so unsaved collision/profile/navigation changes cannot pass in an importer.
    const TCHAR* CityMeshNames[]={TEXT("SM_CC6_Courtyard"),TEXT("SM_CC6_Office"),TEXT("SM_CC6_Tower"),TEXT("SM_CC6_Service")};
    const TCHAR* TerrainMeshNames[]={TEXT("SM_CTV6_CanyonSouth"),TEXT("SM_CTV6_CanyonNorth"),TEXT("SM_CTV6_CanyonWest"),TEXT("SM_CTV6_CanyonEast"),TEXT("SM_CTV6_RiverMarginWest"),TEXT("SM_CTV6_RiverMarginEast")};
    TSet<FString> CityPaths,TerrainPaths;
    auto CheckV6Mesh=[&](const TCHAR* Folder,const TCHAR* Name,TSet<FString>& Paths)
    {
        const FString Path=FString::Printf(TEXT("/Game/HERO/%s/Meshes/%s.%s"),Folder,Name,Name);
        Paths.Add(Path);
        UStaticMesh* Mesh=LoadObject<UStaticMesh>(nullptr,*Path);
        bool bMaterials=Mesh && !Mesh->GetStaticMaterials().IsEmpty();
        if(Mesh)for(const FStaticMaterial& Slot:Mesh->GetStaticMaterials())bMaterials &= OwnMaterial(Slot.MaterialInterface);
        Check(Mesh && Mesh->GetBounds().SphereRadius>1.f && bMaterials,
            *FString::Printf(TEXT("%s V6 authored mesh and own materials load from cooked content"),Name));
    };
    for(const TCHAR* Name:CityMeshNames)CheckV6Mesh(TEXT("ConceptCityV6"),Name,CityPaths);
    for(const TCHAR* Name:TerrainMeshNames)CheckV6Mesh(TEXT("ConceptTerrainV6"),Name,TerrainPaths);
    int32 CityCount=0,TerrainCount=0,LegacyCityCount=0,HouseCount=0;
    bool bCityUpright=true,bHousesUpright=true,bHousesAboveGround=true;
    bool bCityNoCollision=true,bCityNoNavigation=true,bTerrainNoCollision=true,bTerrainNoNavigation=true,bTerrainExact=true;
    TSet<FString> SeenTerrain;
    for(TActorIterator<AStaticMeshActor> It(World);It;++It)
    {
        UStaticMeshComponent* Component=It->GetStaticMeshComponent();
        UStaticMesh* Mesh=Component?Component->GetStaticMesh():nullptr;
        const FString MeshPath=GetPathNameSafe(Mesh);
        if(Mesh && Mesh->GetName().StartsWith(TEXT("SM_DV3_Civic_")))++LegacyCityCount;
        if(CityPaths.Contains(MeshPath))
        {
            ++CityCount;
            const FRotator Rotation=It->GetActorRotation();
            bCityUpright &= FMath::Abs(Rotation.Pitch)<.1f && FMath::Abs(Rotation.Roll)<.1f;
            bCityNoCollision &= Component->GetCollisionEnabled()==ECollisionEnabled::NoCollision
                && Component->GetCollisionProfileName()==FName(TEXT("NoCollision"));
            bCityNoNavigation &= !Component->CanEverAffectNavigation();
        }
        if(Mesh && Mesh->GetName()==TEXT("SM_CH_House"))
        {
            ++HouseCount;
            const FRotator Rotation=It->GetActorRotation();
            const FBox Bounds=Component->Bounds.GetBox();
            bHousesUpright &= FMath::Abs(Rotation.Pitch)<.1f && FMath::Abs(Rotation.Roll)<.1f;
            bHousesAboveGround &= Bounds.Min.Z>=It->GetActorLocation().Z-1.f
                && Bounds.Max.Z>It->GetActorLocation().Z+250.f;
        }
        if(It->ActorHasTag(TEXT("HERO_ConceptTerrainV6")))
        {
            ++TerrainCount;
            bTerrainExact &= TerrainPaths.Contains(MeshPath) && !SeenTerrain.Contains(MeshPath);
            SeenTerrain.Add(MeshPath);
            bTerrainNoCollision &= Component && Component->GetCollisionEnabled()==ECollisionEnabled::NoCollision
                && Component->GetCollisionProfileName()==FName(TEXT("NoCollision"));
            bTerrainNoNavigation &= Component && !Component->CanEverAffectNavigation();
        }
    }
    Check(CityCount==28,TEXT("all 28 authored city actors use exact ConceptCityV6 manifest meshes"));
    Check(LegacyCityCount==0,TEXT("no legacy civic mesh remains after V6 replacement"));
    Check(CityCount==28 && bCityUpright,TEXT("all V6 city actors rotate around world up rather than tipping onto facades"));
    Check(HouseCount==64 && bHousesUpright,TEXT("all 64 West Heights homes stand upright after positional Rotator correction"));
    Check(HouseCount==64 && bHousesAboveGround,TEXT("all West Heights house world bounds extend above ground rather than below it"));
    Check(CityCount==28 && bCityNoCollision,TEXT("all V6 city components retain saved NoCollision profile and mode"));
    Check(CityCount==28 && bCityNoNavigation,TEXT("all V6 city components remain excluded from navigation"));
    Check(TerrainCount==6 && SeenTerrain.Num()==6 && bTerrainExact,TEXT("exactly six tagged terrain actors each use one distinct authored V6 manifest mesh"));
    Check(TerrainCount==6 && bTerrainNoCollision,TEXT("all V6 terrain components retain saved NoCollision profile and mode"));
    Check(TerrainCount==6 && bTerrainNoNavigation,TEXT("all V6 terrain components remain excluded from navigation"));
    for(const TCHAR* Name:{TEXT("SM_RF6_Yellow"),TEXT("SM_RF6_White"),TEXT("SM_RF6_Dust")})
    {
        UStaticMesh* RoadMesh=LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/HERO/RoadFinishV6/Meshes/%s.%s"),Name,Name));
        Check(RoadMesh && RoadMesh->GetBounds().SphereRadius>1.f,TEXT("merged V6 road finish mesh loads from actual content"));
    }
    int32 RoadCount=0;bool bRoadNonBlocking=true;
    for(TActorIterator<AStaticMeshActor> It(World);It;++It)
        if(It->ActorHasTag(TEXT("HERO_RoadFinishV6")))
        { ++RoadCount;bRoadNonBlocking &= It->GetStaticMeshComponent()->GetCollisionEnabled()==ECollisionEnabled::NoCollision
            && !It->GetStaticMeshComponent()->CanEverAffectNavigation(); }
    Check(RoadCount==3 && bRoadNonBlocking,TEXT("three merged road finish actors preserve collision-free navigable streets"));
    // V8 is a saved-map contract: an importer report alone does not establish
    // that both decorative courtyards survive the final cook with safe flags.
    const TSet<FString> BaseyardPaths={
        TEXT("/Game/HERO/ReferenceV8/Baseyard/Meshes/SM_BY8_WestBaseyard.SM_BY8_WestBaseyard"),
        TEXT("/Game/HERO/ReferenceV8/Baseyard/Meshes/SM_BY8_EastBaseyard.SM_BY8_EastBaseyard")};
    TSet<FString> BaseyardMaterials;
    for(const TCHAR* Name:{TEXT("Concrete"),TEXT("ConcreteDark"),TEXT("Metal"),TEXT("Dust"),TEXT("Track"),TEXT("Paint")})
        BaseyardMaterials.Add(FString::Printf(TEXT("/Game/HERO/ReferenceV8/Baseyard/Materials/MI_BY8_%s.MI_BY8_%s"),Name,Name));
    int32 BaseyardCount=0;TSet<FString> SeenBaseyards;
    bool bBaseyardExact=true,bBaseyardNonBlocking=true,bBaseyardMaterials=true;
    for(TActorIterator<AStaticMeshActor> It(World);It;++It)
    {
        UStaticMeshComponent* Component=It->GetStaticMeshComponent();
        UStaticMesh* Mesh=Component?Component->GetStaticMesh():nullptr;
        const FString MeshPath=GetPathNameSafe(Mesh);
        if(!It->ActorHasTag(TEXT("HERO_BaseyardV8")) && !BaseyardPaths.Contains(MeshPath))continue;
        ++BaseyardCount;
        bBaseyardExact &= It->ActorHasTag(TEXT("HERO_BaseyardV8")) && BaseyardPaths.Contains(MeshPath)
            && !SeenBaseyards.Contains(MeshPath);
        SeenBaseyards.Add(MeshPath);
        bBaseyardNonBlocking &= Component && !It->GetActorEnableCollision()
            && Component->GetCollisionEnabled()==ECollisionEnabled::NoCollision
            && Component->GetCollisionProfileName()==FName(TEXT("NoCollision"))
            && !Component->CanEverAffectNavigation() && !Component->CastShadow;
        TSet<FString> SeenMaterials;
        bool bBindings=Mesh && Component && Mesh->GetStaticMaterials().Num()==6 && Component->GetNumMaterials()==6;
        if(bBindings)for(int32 Slot=0;Slot<6;++Slot)
        {
            UMaterialInterface* Material=Component->GetMaterial(Slot);
            const FString MaterialPath=GetPathNameSafe(Material);
            bBindings &= BaseyardMaterials.Contains(MaterialPath) && !SeenMaterials.Contains(MaterialPath)
                && Mesh->GetStaticMaterials()[Slot].MaterialInterface==Material && OwnMaterial(Material);
            SeenMaterials.Add(MaterialPath);
        }
        bBaseyardMaterials &= bBindings && SeenMaterials.Num()==6;
    }
    Check(BaseyardCount==2 && SeenBaseyards.Num()==2 && bBaseyardExact,
        TEXT("exactly two tagged V8 baseyards each load their distinct authored mesh from the saved map"));
    Check(BaseyardCount==2 && bBaseyardNonBlocking,
        TEXT("both V8 baseyards retain disabled actor and component collision, navigation exclusion and no shadows"));
    Check(BaseyardCount==2 && bBaseyardMaterials,
        TEXT("both V8 baseyards preserve all six distinct exact saved PBR bindings in their runtime slots"));
    // Validate the final saved terrain actors, not just the importer's report.
    const TSet<FString> TerrainV9Paths={
        TEXT("/Game/HERO/ReferenceV9/Terrain/Meshes/SM_TRV9_RiverNorth.SM_TRV9_RiverNorth"),
        TEXT("/Game/HERO/ReferenceV9/Terrain/Meshes/SM_TRV9_RiverSouth.SM_TRV9_RiverSouth"),
        TEXT("/Game/HERO/ReferenceV9/Terrain/Meshes/SM_TRV9_WestTalus.SM_TRV9_WestTalus"),
        TEXT("/Game/HERO/ReferenceV9/Terrain/Meshes/SM_TRV9_EastTalus.SM_TRV9_EastTalus")};
    TSet<FString> TerrainV9Materials;
    // Dust exists as a shared library material but has no used source faces:
    // each mesh has these five actual sections, never an invented sixth one.
    for(const TCHAR* Key:{TEXT("Concrete"),TEXT("ConcreteDark"),TEXT("Metal"),TEXT("Track"),TEXT("Paint")})
        TerrainV9Materials.Add(FString::Printf(TEXT("/Game/HERO/ReferenceV9/Terrain/Materials/MI_TRV9_%s.MI_TRV9_%s"),Key,Key));
    const FString TerrainV9Parent=TEXT("/Game/HERO/ReferenceV9/Terrain/Materials/M_TRV9_SurfacePBR.M_TRV9_SurfacePBR");
    int32 TerrainV9Count=0;TSet<FString> SeenTerrainV9;
    bool bTerrainV9Exact=true,bTerrainV9SafeBounds=true,bTerrainV9Flags=true,bTerrainV9Bindings=true;
    for(TActorIterator<AStaticMeshActor> It(World);It;++It)
    {
        auto* Component=It->GetStaticMeshComponent();
        UStaticMesh* Mesh=Component?Component->GetStaticMesh():nullptr;
        const FString MeshPath=GetPathNameSafe(Mesh);
        if(!It->ActorHasTag(TEXT("HERO_TerrainV9"))&&!TerrainV9Paths.Contains(MeshPath))continue;
        ++TerrainV9Count;
        bTerrainV9Exact &= It->ActorHasTag(TEXT("HERO_TerrainV9"))&&TerrainV9Paths.Contains(MeshPath)&&!SeenTerrainV9.Contains(MeshPath);
        SeenTerrainV9.Add(MeshPath);
        if(Component&&Mesh)
        {
            const FBox Bounds=Mesh->GetBoundingBox().TransformBy(Component->GetComponentTransform());
            const bool bRiver=Bounds.Min.X>=-3680.f&&Bounds.Max.X<=-2320.f&&Bounds.Min.Y>=-2140.f&&Bounds.Max.Y<=2140.f;
            const bool bOutside=Bounds.Max.X<=-12060.f||Bounds.Min.X>=12060.f;
            bTerrainV9SafeBounds &= Bounds.IsValid&&!Bounds.Min.ContainsNaN()&&!Bounds.Max.ContainsNaN()&&(bRiver||bOutside);
        }
        else bTerrainV9SafeBounds=false;
        bTerrainV9Flags &= Component&&!It->GetActorEnableCollision()&&Component->GetCollisionEnabled()==ECollisionEnabled::NoCollision
            &&Component->GetCollisionProfileName()==FName(TEXT("NoCollision"))&&!Component->CanEverAffectNavigation()
            &&!Component->CastShadow&&!Component->bAffectDistanceFieldLighting
            &&It->GetActorScale3D().Equals(FVector::OneVector,.001f)&&It->GetActorRotation().IsNearlyZero(.001f)
            &&Component->GetRelativeScale3D().Equals(FVector::OneVector,.001f);
        bool bBindings=Mesh&&Component&&Mesh->GetStaticMaterials().Num()==5&&Component->GetNumMaterials()==5;
        TSet<FString> SeenMaterials;
        if(bBindings)for(int32 Slot=0;Slot<5;++Slot)
        {
            auto* Material=Component->GetMaterial(Slot);
            auto* Instance=Cast<UMaterialInstance>(Material);
            const FString Path=GetPathNameSafe(Material);
            bBindings &= TerrainV9Materials.Contains(Path)&&!SeenMaterials.Contains(Path)
                &&Mesh->GetStaticMaterials()[Slot].MaterialInterface==Material&&OwnMaterial(Material)
                &&Instance&&GetPathNameSafe(Instance->Parent)==TerrainV9Parent
                &&GetPathNameSafe(Material?Material->GetMaterial():nullptr)==TerrainV9Parent;
            SeenMaterials.Add(Path);
        }
        bTerrainV9Bindings &= bBindings&&SeenMaterials.Num()==5;
    }
    Check(TerrainV9Count==4&&SeenTerrainV9.Num()==4&&bTerrainV9Exact,
        TEXT("exactly four tagged V9 terrain actors each use a distinct expected saved mesh"));
    Check(TerrainV9Count==4&&bTerrainV9SafeBounds,
        TEXT("all V9 terrain world bounds stay inside the river corridor or outside authoritative west east play space"));
    Check(TerrainV9Count==4&&bTerrainV9Flags,
        TEXT("all V9 terrain retains identity scale rotation and no actor component collision navigation shadows or distance fields"));
    Check(TerrainV9Count==4&&bTerrainV9Bindings,
        TEXT("all V9 terrain preserves five exact distinct used PBR section bindings and their valid saved parent"));
    // V9 exercises actual registered moving meshes independently of harvesting.
    // TickVisual may observe these fixture states, but must never advance money,
    // cargo, reservations or movement as a side effect of presentation.
    for(int32 Team=0;Mode && Team<2;++Team)
    {
        APrototypeUnit* Collector=Mode->SpawnUnit(Team,FVector(-14000,-11000,35));
        if(Collector){Collector->ConfigureRole(EPrototypeRole::Collector);Collector->SetActorTickEnabled(false);}
        auto GetParts=[](APrototypeUnit* Unit)
        {
            TArray<UStaticMeshComponent*> Result,All;
            if(Unit)Unit->GetComponents<UStaticMeshComponent>(All);
            for(auto* Component:All)if(IsValid(Component)&&Component->IsRegistered()&&Component->GetStaticMesh()
                &&Component->GetStaticMesh()->GetPathName().StartsWith(TEXT("/Game/HERO/HarvesterAnimationV9/Meshes/")))Result.Add(Component);
            return Result;
        };
        auto Parts=GetParts(Collector);
        auto FindPart=[&](const TCHAR* Key)->UStaticMeshComponent*
        {
            const FString Name=FString(TEXT("SM_HA9_"))+Key;
            for(auto* Part:Parts)if(Part->GetStaticMesh()->GetName()==Name)return Part;
            return nullptr;
        };
        auto* Visual=Collector?Collector->FindComponentByClass<UPrototypeCollectorVisualComponent>():nullptr;
        auto* Hull=FindPart(TEXT("Base"));auto* Arm=FindPart(TEXT("ArmAssembly"));
        auto* Drum=FindPart(TEXT("CutterDrum"));auto* Coupler=FindPart(TEXT("RearCoupler"));
        TSet<FName> Names;
        for(auto* Part:Parts)Names.Add(Part->GetStaticMesh()->GetFName());
        Check(Visual&&Visual->HasArticulatedAssets()&&Parts.Num()==11&&Names.Num()==11&&Hull&&Arm&&Drum&&Coupler,
            *FString::Printf(TEXT("team %d collector installs all eleven distinct authored V9 meshes without fallback or duplication"),Team));
        bool bSafe=Parts.Num()==11,bMaterials=bSafe;
        const bool bAegis=Mode->GetFactionIndex(Team)==0;
        for(auto* Part:Parts)
        {
            // The original gameplay hull keeps its existing actor contract;
            // every newly added decorative part must remain non-interactive.
            bSafe &= Part==Hull||(Part->GetCollisionEnabled()==ECollisionEnabled::NoCollision&&!Part->CanEverAffectNavigation()
                &&!Part->GetGenerateOverlapEvents()&&!Part->CastShadow);
            bMaterials &= Part->GetNumMaterials()>0;
            for(int32 Slot=0;Slot<Part->GetNumMaterials();++Slot)
            {
                const FName SlotName=Part->GetStaticMesh()->GetStaticMaterials()[Slot].MaterialSlotName;
                const FString MaterialName=FString::Printf(TEXT("M_HR_%s_%s"),bAegis?TEXT("Aegis"):TEXT("Ember"),*SlotName.ToString());
                auto* Expected=LoadObject<UMaterialInterface>(nullptr,*(TEXT("/Game/HERO/PresentationV5/Materials/")+MaterialName+TEXT(".")+MaterialName));
                auto* Actual=Part->GetMaterial(Slot);
                FLinearColor ExpectedTint,ActualTint;
                bool bTint=Expected&&Expected->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),ExpectedTint);
                if(SlotName==FName(TEXT("Team")))ExpectedTint=bAegis?FLinearColor(.025f,.18f,.48f):FLinearColor(.025f,.32f,.13f);
                if(SlotName==FName(TEXT("Neon")))ExpectedTint=bAegis?FLinearColor(.025f,.35f,.8f):FLinearColor(.025f,.7f,.28f);
                bMaterials &= OwnMaterial(Actual)&&Expected&&Actual->GetMaterial()==Expected->GetMaterial()&&bTint
                    &&Actual->GetVectorParameterValue(FHashedMaterialParameterInfo(TEXT("Tint")),ActualTint)&&ActualTint.Equals(ExpectedTint,.001f);
            }
        }
        Check(bSafe,*FString::Printf(TEXT("team %d added collector parts are collision overlap and navigation free"),Team));
        Check(bMaterials,*FString::Printf(TEXT("team %d all split collector material slots preserve actual faction bindings and tint"),Team));
        auto* Original=LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/ConceptV2/Meshes/SM_HERO_Harvester.SM_HERO_Harvester"));
        bool bBounds=false;
        if(Hull&&Arm&&Drum&&Coupler&&Original)
        {
            // Compare exact source assembly pose, before any decorative lift.
            // Component transforms, not actor aggregate bounds, exclude selection
            // rings and invisible generic combat parts from this comparison.
            Arm->SetRelativeRotation(FRotator::ZeroRotator);Drum->SetRelativeRotation(FRotator::ZeroRotator);
            FBox Assembly(ForceInit);
            for(auto* Part:Parts)Assembly+=Part->GetStaticMesh()->GetBoundingBox().TransformBy(Part->GetComponentTransform());
            const FBox Expected=Original->GetBoundingBox().TransformBy(Hull->GetComponentTransform());
            bBounds=Assembly.Min.Equals(Expected.Min,1.f)&&Assembly.Max.Equals(Expected.Max,1.f)
                &&Drum->GetAttachParent()==Arm&&Arm->GetAttachParent()==Hull;
        }
        Check(bBounds,*FString::Printf(TEXT("team %d source pose reproduces original harvester bounds and articulated parent chain"),Team));
        if(Collector)
        {
            auto Step=[&](){for(int32 Index=0;Index<12;++Index)UPrototypeCollectorVisualComponent::TickVisual(Collector,.1f);};
            auto LitGauges=[&](){int32 Num=0;for(int32 Index=0;Index<7;++Index)
                if(auto* Gauge=FindPart(*FString::Printf(TEXT("Gauge%d"),Index));Gauge&&Gauge->IsVisible())++Num;return Num;};
            Collector->Cargo=0.;Collector->CollectorState=EPrototypeCollectorState::SeekingField;Step();
            const FRotator SeekingArm=Arm?Arm->GetRelativeRotation():FRotator::ZeroRotator;
            const FRotator SeekingDrum=Drum?Drum->GetRelativeRotation():FRotator::ZeroRotator;
            const bool bEmpty=LitGauges()==0;
            Collector->Cargo=100.;Collector->CollectorState=EPrototypeCollectorState::Harvesting;
            const FVector Destination=Collector->Destination,Location=Collector->GetActorLocation();
            const double Wallet=Mode->Credits,EnemyWallet=Mode->EnemyCredits;
            Step();
            Check(Arm&&Drum&&!Arm->GetRelativeRotation().Equals(SeekingArm,1.f)&&!Drum->GetRelativeRotation().Equals(SeekingDrum,1.f),
                *FString::Printf(TEXT("team %d harvesting moves the real arm and toothed drum"),Team));
            Check(bEmpty&&LitGauges()==7,*FString::Printf(TEXT("team %d original tank segments show empty then full actual cargo"),Team));
            Check(Collector->Cargo==100.&&Collector->CollectorState==EPrototypeCollectorState::Harvesting
                &&Collector->Destination.Equals(Destination,0.f)&&Collector->GetActorLocation().Equals(Location,0.f)
                &&Mode->Credits==Wallet&&Mode->EnemyCredits==EnemyWallet,
                *FString::Printf(TEXT("team %d visual ticking preserves cargo state destination transform and both wallets"),Team));
            Collector->CollectorState=EPrototypeCollectorState::MovingToRefinery;Step();
            const FRotator MovingArm=Arm?Arm->GetRelativeRotation():FRotator::ZeroRotator;
            const FVector Retracted=Coupler?Coupler->GetRelativeScale3D():FVector::ZeroVector;
            const FRotator StoppedDrum=Drum?Drum->GetRelativeRotation():FRotator::ZeroRotator;
            Collector->CollectorState=EPrototypeCollectorState::WaitingForDock;Step();
            Check(Arm&&Drum&&!Arm->GetRelativeRotation().Equals(MovingArm,1.f)&&Drum->GetRelativeRotation().Equals(StoppedDrum,.001f),
                *FString::Printf(TEXT("team %d dock waiting changes arm readiness while cutter stays stopped"),Team));
            Collector->CollectorState=EPrototypeCollectorState::Unloading;Step();
            Check(Coupler&&Coupler->GetRelativeScale3D().X>Retracted.X+.4f&&Collector->Cargo==100.
                &&Collector->CollectorState==EPrototypeCollectorState::Unloading&&Mode->Credits==Wallet&&Mode->EnemyCredits==EnemyWallet,
                *FString::Printf(TEXT("team %d unloading extends real coupler without performing a resource transfer"),Team));
            TArray<TWeakObjectPtr<UStaticMeshComponent>> OldParts;
            for(auto* Part:Parts)if(Part!=Hull)OldParts.Add(Part);
            Collector->ConfigureRole(EPrototypeRole::Combat);
            auto* CombatHull=Collector->FindComponentByClass<UStaticMeshComponent>();
            const auto* CombatDefinition=Mode->GetRoleDefinition(EPrototypeRole::Combat,Team);
            UStaticMesh* ExpectedCombat=CombatDefinition?CombatDefinition->StaticMesh.LoadSynchronous():nullptr;
            bool bRemoved=GetParts(Collector).IsEmpty()&&!Collector->FindComponentByClass<UPrototypeCollectorVisualComponent>();
            for(const auto& Old:OldParts)bRemoved &= !Old.IsValid()||!Old->IsRegistered();
            Check(bRemoved&&CombatHull&&ExpectedCombat&&CombatHull->GetStaticMesh()==ExpectedCombat,
                *FString::Printf(TEXT("team %d collector to combat removes parts and keeps the actual combat mesh"),Team));
            Collector->ConfigureRole(EPrototypeRole::Collector);Parts=GetParts(Collector);
            TArray<UPrototypeCollectorVisualComponent*> Visuals;Collector->GetComponents<UPrototypeCollectorVisualComponent>(Visuals);
            Check(Parts.Num()==11&&Visuals.Num()==1&&Visuals[0]->HasArticulatedAssets(),
                *FString::Printf(TEXT("team %d returning to collector installs one clean presentation without component leaks"),Team));
            TArray<TWeakObjectPtr<UStaticMeshComponent>> DestroyedParts;for(auto* Part:Parts)DestroyedParts.Add(Part);
            TWeakObjectPtr<UPrototypeCollectorVisualComponent> DestroyedVisual=Visuals.Num()?Visuals[0]:nullptr;
            Collector->Destroy();
            bool bDestroyed=!DestroyedVisual.IsValid()||!DestroyedVisual->IsRegistered();
            for(const auto& Part:DestroyedParts)bDestroyed &= !Part.IsValid()||!Part->IsRegistered();
            Check(bDestroyed,*FString::Printf(TEXT("team %d actor destruction unregisters all collector presentation components"),Team));
        }
        else Check(false,TEXT("collector visual fixture actor spawned for state and cleanup checks"));
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
