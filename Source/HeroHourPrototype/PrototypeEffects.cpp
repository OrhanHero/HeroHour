#include "PrototypeGameMode.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

namespace
{
AStaticMeshActor* Effect(UWorld* World,const FVector& Position,const FVector& Scale,const FRotator& Rotation,
    const TCHAR* Shape,const FLinearColor& Color,float Lifetime,const TCHAR* MaterialPath=TEXT("/Game/HERO/EffectsV4/M_FX_Impact.M_FX_Impact"))
{
    UStaticMesh* Model=LoadObject<UStaticMesh>(nullptr,Shape);
    UMaterialInterface* Material=LoadObject<UMaterialInterface>(nullptr,MaterialPath);
    if (!Model || !Material) return nullptr;
    auto* Actor=World->SpawnActor<AStaticMeshActor>(Position,Rotation);
    if (!Actor) return nullptr;
    Actor->Tags.Add(TEXT("HERO_RuntimeFX"));
    auto* Mesh=Actor->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(Model);Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);Mesh->SetCastShadow(false);
    auto* Dynamic=UMaterialInstanceDynamic::Create(Material,Actor);Dynamic->SetVectorParameterValue(TEXT("Tint"),Color);
    Mesh->SetMaterial(0,Dynamic);Actor->SetActorScale3D(Scale);Actor->SetLifeSpan(Lifetime);
    return Actor;
}
void Sound(UWorld* World,const FVector& Position,const TCHAR* Path,float Volume)
{
    if(auto* Wave=LoadObject<USoundWave>(nullptr,Path)) UGameplayStatics::PlaySoundAtLocation(World,Wave,Position,Volume);
}
}

void APrototypeGameMode::SpawnCombatEffect(const FVector& Position,int32 Team,bool bLarge)
{
    if (!Visibility.IsVisible(0,Position)) return;
    const float Size=bLarge?2.5f:.6f;
    Effect(GetWorld(),Position+FVector(0,0,70),FVector(Size),FRotator::ZeroRotator,
        TEXT("/Engine/BasicShapes/Sphere.Sphere"),FLinearColor(1.f,.35f,.035f),bLarge?.4f:.12f);
    if(bLarge) Effect(GetWorld(),Position+FVector(0,0,130),FVector(2.5f,2.5f,3.5f),FRotator::ZeroRotator,
        TEXT("/Engine/BasicShapes/Sphere.Sphere"),FLinearColor(.1f,.11f,.12f,.4f),1.5f,TEXT("/Game/HERO/EffectsV4/M_FX_Smoke.M_FX_Smoke"));
    Sound(GetWorld(),Position,TEXT("/Game/HERO/EffectsV4/A_Impact.A_Impact"),bLarge?.22f:.08f);
}

void APrototypeGameMode::SpawnWeaponEffect(const FVector& Start,const FVector& End,int32 Team,EPrototypeRole InRole)
{
    if(!Visibility.IsVisible(0,Start) && !Visibility.IsVisible(0,End)) return;
    const FVector Delta=End-Start;
    const FLinearColor Color=GetFactionIndex(Team)==0?FLinearColor(.55f,.85f,1.f):FLinearColor(1.f,.25f,.035f);
    Effect(GetWorld(),(Start+End)*.5f,FVector(Delta.Size()/100.f,.025f,.025f),Delta.Rotation(),
        TEXT("/Engine/BasicShapes/Cube.Cube"),Color,.065f);
    Effect(GetWorld(),Start,FVector(InRole==EPrototypeRole::Siege?.45f:.18f),FRotator::ZeroRotator,
        TEXT("/Engine/BasicShapes/Sphere.Sphere"),Color,.085f);
    SpawnCombatEffect(End,Team,InRole==EPrototypeRole::Siege);
    Sound(GetWorld(),Start,InRole==EPrototypeRole::Infantry?TEXT("/Game/HERO/EffectsV4/A_Rifle.A_Rifle"):TEXT("/Game/HERO/EffectsV4/A_Cannon.A_Cannon"),.12f);
}
