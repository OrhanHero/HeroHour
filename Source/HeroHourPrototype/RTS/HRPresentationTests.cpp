#include "../PrototypeGameMode.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
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
        {0,EPrototypeRole::Headquarters,TEXT("SM_RV7_AegisHQ"),TEXT("Aegis_HQ"),2048},
        {0,EPrototypeRole::Combat,TEXT("SM_HR_Aegis_APC8"),TEXT("Aegis_APC8"),1024},
        {1,EPrototypeRole::Headquarters,TEXT("SM_RV7_EmberHQ"),TEXT("Ember_HQ"),2048},
        {1,EPrototypeRole::Combat,TEXT("SM_HR_Ember_APC8"),TEXT("Ember_APC8"),1024}};
    const int32 AegisTeam=Mode && Mode->GetFactionIndex(0)==0?0:1;
    TSet<UStaticMesh*> ReferenceGeometry;
    for(const FReferenceFixture& Fixture:ReferenceFixtures)
    {
        const EPrototypeRole Role=Fixture.Role;
        const bool bHeadquarters=Role==EPrototypeRole::Headquarters;
        const FVector Position(-14000.f,-12000.f,bHeadquarters?110.f:35.f);
        const int32 Team=Fixture.Faction==0?AegisTeam:1-AegisTeam;
        APrototypeUnit* Unit=Mode?Mode->SpawnUnit(Team,Position,bHeadquarters):nullptr;
        UStaticMeshComponent* Hull=Unit?Unit->FindComponentByClass<UStaticMeshComponent>():nullptr;
        UStaticMesh* Mesh=Hull?Hull->GetStaticMesh():nullptr;
        const FString MeshPath=FString::Printf(TEXT("/Game/HERO/ReferenceV7/Meshes/%s.%s"),Fixture.Mesh,Fixture.Mesh);
        Check(Unit && Unit->Role==Role && Hull && Hull->IsVisible() && Mesh
            && Mesh->GetPathName()==MeshPath && Unit->Definition
            && Unit->Definition->StaticMesh.LoadSynchronous()==Mesh && !ReferenceGeometry.Contains(Mesh),
            *FString::Printf(TEXT("%s displays its exact saved V7 mesh with geometry distinct from the other reference units"),Fixture.Surface));
        if(Mesh)ReferenceGeometry.Add(Mesh);
        const FBox GroundBounds=Mesh && Hull
            ?Mesh->GetBoundingBox().TransformBy(Hull->GetComponentTransform()):FBox(ForceInit);
        UE_LOG(LogTemp,Display,TEXT("PRESENTATION_REFERENCE_BOUNDS role=%s min=%s max=%s"),
            Fixture.Surface,*GroundBounds.Min.ToString(),*GroundBounds.Max.ToString());
        Check(GroundBounds.IsValid && FMath::Abs(GroundBounds.Min.Z)<1.f
            && GroundBounds.GetSize().Z>(bHeadquarters?650.f:170.f),
            *FString::Printf(TEXT("%s final component transform puts the authored base on world ground"),Fixture.Surface));
        const FVector Size=GroundBounds.GetSize();
        Check(GroundBounds.IsValid && (bHeadquarters
            ?Size.X>900.f && Size.Y>900.f && Size.X<=2.f*APrototypeGameMode::Footprint(Role)
                && Size.Y<=2.f*APrototypeGameMode::Footprint(Role)
            :Size.X>330.f && Size.X<410.f && Size.Y>170.f && Size.Y<210.f),
            *FString::Printf(TEXT("%s keeps the intended RTS scale and placement envelope"),Fixture.Surface));
        const FString MaterialName=FString(TEXT("M_RV7_Baked_"))+Fixture.Surface;
        const FString MaterialPath=TEXT("/Game/HERO/ReferenceV7/Materials/")+MaterialName+TEXT(".")+MaterialName;
        bool bSavedAtlas=Mesh && Hull && Hull->GetNumMaterials()>0;
        bool bNeutralTint=bSavedAtlas;
        UMaterialInterface* Atlas=Mesh?Mesh->GetMaterial(0):nullptr;
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
        for(const TCHAR* Channel:{TEXT("Albedo"),TEXT("ORM"),TEXT("EmissionMask")})
        {
            const FString TextureName=FString::Printf(TEXT("T_RV7_%s_%s"),Fixture.Surface,Channel);
            const FString TexturePath=TEXT("/Game/HERO/ReferenceV7/Textures/")+TextureName+TEXT(".")+TextureName;
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
            const bool bAlbedo=FCString::Strcmp(Channel,TEXT("Albedo"))==0;
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
