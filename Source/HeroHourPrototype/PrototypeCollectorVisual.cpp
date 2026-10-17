#include "PrototypeCollectorVisual.h"
#include "PrototypeGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"

namespace
{
const TCHAR* PartNames[] = {TEXT("ArmAssembly"), TEXT("CutterDrum"), TEXT("RearCoupler"),
    TEXT("Gauge0"), TEXT("Gauge1"), TEXT("Gauge2"), TEXT("Gauge3"), TEXT("Gauge4"), TEXT("Gauge5"), TEXT("Gauge6")};

UStaticMesh* LoadPart(const TCHAR* Name)
{
    const FString Asset=FString::Printf(TEXT("/Game/HERO/HarvesterAnimationV9/Meshes/SM_HA9_%s.SM_HA9_%s"),Name,Name);
    if(!FPackageName::DoesPackageExist(FPackageName::ObjectPathToPackageName(Asset)))return nullptr;
    return LoadObject<UStaticMesh>(nullptr,*Asset,nullptr,LOAD_NoWarn);
}

void ApplyNamedMaterials(UStaticMeshComponent* Component,const TMap<FName,UMaterialInterface*>& Materials)
{
    Component->EmptyOverrideMaterials();
    for(const auto& Entry:Materials)
    {
        const int32 Index=Component->GetMaterialIndex(Entry.Key);
        if(Index!=INDEX_NONE && Entry.Value)Component->SetMaterial(Index,Entry.Value);
    }
}
}

UPrototypeCollectorVisualComponent::UPrototypeCollectorVisualComponent()
{
    PrimaryComponentTick.bCanEverTick=false;
}

void UPrototypeCollectorVisualComponent::SyncRole(APrototypeUnit* Unit,UStaticMeshComponent* Hull)
{
    if(!IsValid(Unit)||!IsValid(Hull))return;
    auto* Existing=Unit->FindComponentByClass<UPrototypeCollectorVisualComponent>();
    // ConfigureRole has already installed its new original mesh; do not restore
    // an old collector hull over another role's newly selected model.
    if(Existing)Existing->DestroyComponent();
    if(Unit->Role!=EPrototypeRole::Collector)return;
    auto* Visual=NewObject<UPrototypeCollectorVisualComponent>(Unit);
    Unit->AddInstanceComponent(Visual);
    Visual->RegisterComponent();
    Visual->Initialize(Hull);
}

void UPrototypeCollectorVisualComponent::TickVisual(APrototypeUnit* Unit,float DeltaSeconds)
{
    if(IsValid(Unit))if(auto* Visual=Unit->FindComponentByClass<UPrototypeCollectorVisualComponent>())Visual->Advance(DeltaSeconds);
}

bool UPrototypeCollectorVisualComponent::Initialize(UStaticMeshComponent* Hull)
{
    auto* Unit=Cast<APrototypeUnit>(GetOwner());
    if(!Unit||!Hull||Unit->Role!=EPrototypeRole::Collector)return false;
    BaseMesh=LoadPart(TEXT("Base"));
    TArray<UStaticMesh*> Models;
    for(const TCHAR* Name:PartNames)Models.Add(LoadPart(Name));
    if(!BaseMesh||Models.Contains(nullptr))return false;
    HullComponent=Hull;
    OriginalHull=Hull->GetStaticMesh();
    for(int32 Index=0;Index<Hull->GetNumMaterials();++Index)OriginalMaterials.Add(Hull->GetMaterial(Index));
    TMap<FName,UMaterialInterface*> Materials;
    for(const FName Slot:Hull->GetMaterialSlotNames())Materials.Add(Slot,Hull->GetMaterial(Hull->GetMaterialIndex(Slot)));
    // Source coordinates include the original exact floor normalization. These
    // pivots are matched to the generated manifest, not inferred game timings.
    const FVector ArmPivot(115.f,0.f,53.5f);
    const FVector DrumFromArm(62.f,0.f,-38.5f);
    const FVector CouplerPivot(-183.9f,0.f,83.5f);
    for(int32 Index=0;Index<Models.Num();++Index)
    {
        const FName Name=MakeUniqueObjectName(Unit,UStaticMeshComponent::StaticClass(),*FString::Printf(TEXT("CollectorVisual_%s"),PartNames[Index]));
        auto* Part=NewObject<UStaticMeshComponent>(Unit,Name);
        Unit->AddInstanceComponent(Part);
        Part->SetMobility(EComponentMobility::Movable);
        Part->SetStaticMesh(Models[Index]);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetCanEverAffectNavigation(false);
        Part->SetCastShadow(false);
        Part->SetupAttachment(Index==1?Parts[0].Get():Hull);
        Part->SetRelativeLocation(Index==0?ArmPivot:Index==1?DrumFromArm:Index==2?CouplerPivot:FVector::ZeroVector);
        ApplyNamedMaterials(Part,Materials);
        Part->RegisterComponent();
        Parts.Add(Part);
    }
    Hull->SetStaticMesh(BaseMesh);
    ApplyNamedMaterials(Hull,Materials);
    DisplayCargo=FMath::Clamp(static_cast<float>(Unit->Cargo/100.),0.f,1.f);
    bReady=true;
    Advance(.001f);
    return true;
}

void UPrototypeCollectorVisualComponent::Advance(float DeltaSeconds)
{
    const auto* Unit=Cast<APrototypeUnit>(GetOwner());
    if(!bReady||!Unit||Unit->Role!=EPrototypeRole::Collector||Unit->Health<=0.f||!FMath::IsFinite(DeltaSeconds)||DeltaSeconds<=0.f)return;
    // Presentation constants are ours. No claim that these came from Tempest.
    const float Dt=FMath::Min(DeltaSeconds,.25f);
    const bool bHarvesting=Unit->CollectorState==EPrototypeCollectorState::Harvesting;
    const bool bUnloading=Unit->CollectorState==EPrototypeCollectorState::Unloading;
    const float Blend=1.f-FMath::Exp(-Dt/.18f);
    const float TargetArmAngle=bHarvesting?-25.f:
        Unit->CollectorState==EPrototypeCollectorState::SeekingField?18.f:
        Unit->CollectorState==EPrototypeCollectorState::WaitingForDock?6.f:
        Unit->CollectorState==EPrototypeCollectorState::MovingToRefinery?12.f:10.f;
    ArmAngle=FMath::Lerp(ArmAngle,TargetArmAngle,Blend);
    CouplerExtension=FMath::Lerp(CouplerExtension,bUnloading?1.f:0.f,Blend);
    DisplayCargo=FMath::Lerp(DisplayCargo,FMath::Clamp(static_cast<float>(Unit->Cargo/100.),0.f,1.f),1.f-FMath::Exp(-Dt/.12f));
    if(bHarvesting)DrumAngle=FMath::Fmod(DrumAngle+Dt*240.f,360.f);
    Parts[0]->SetRelativeRotation(FRotator(ArmAngle,0,0));
    Parts[1]->SetRelativeRotation(FRotator(DrumAngle,0,0));
    // Stretch the existing bellows slightly, keeping its forward contact fixed.
    Parts[2]->SetRelativeScale3D(FVector(1.f+.6f*CouplerExtension,1.f,1.f));
    Parts[2]->SetRelativeLocation(FVector(-183.9f-1.74f*CouplerExtension,0,83.5f));
    for(int32 Index=0;Index<7;++Index)
        Parts[3+Index]->SetVisibility(DisplayCargo>(Index+.5f)/7.f);
}

void UPrototypeCollectorVisualComponent::RemoveParts()
{
    for(auto& Part:Parts)if(IsValid(Part))Part->DestroyComponent();
    Parts.Empty();
    bReady=false;
}

void UPrototypeCollectorVisualComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    const auto* Unit=Cast<APrototypeUnit>(GetOwner());
    if(!bDestroyingHierarchy&&Unit&&Unit->Role==EPrototypeRole::Collector&&IsValid(HullComponent)
        &&HullComponent->GetStaticMesh()==BaseMesh&&OriginalHull)
    {
        HullComponent->SetStaticMesh(OriginalHull);
        HullComponent->EmptyOverrideMaterials();
        for(int32 Index=0;Index<OriginalMaterials.Num();++Index)HullComponent->SetMaterial(Index,OriginalMaterials[Index]);
    }
    RemoveParts();
    Super::OnComponentDestroyed(bDestroyingHierarchy);
}
