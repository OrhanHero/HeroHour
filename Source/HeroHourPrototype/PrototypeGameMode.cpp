#include "PrototypeGameMode.h"
#include "BattlefieldLayout.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMeshActor.h"
#include "RTS/HRProductionQueue.h"
#include "RTS/HRRTSDefinitions.h"
#include "RTS/HRRefineryDocking.h"
#include "RTS/HRCombat.h"
#include "RTS/HRRTSTags.h"
#include "PrototypeHUDLayout.h"
#include "Misc/PackageName.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/AnimationAsset.h"

namespace
{
float ConceptStructureScale(EPrototypeRole Role)
{
    return Role==EPrototypeRole::Headquarters ? 1.5f : Role==EPrototypeRole::Factory ? 1.7f : Role==EPrototypeRole::Depot ? 1.5f : 1.f;
}
FLinearColor FactionMarkingColor(int32 Team,bool bEmissive=false)
{
    const bool bAegis=APrototypeGameMode::GetFactionIndex(Team)==0;
    return bAegis ? (bEmissive?FLinearColor(.025f,.35f,.80f):FLinearColor(.025f,.18f,.48f))
        : (bEmissive?FLinearColor(.90f,.12f,.025f):FLinearColor(.48f,.045f,.018f));
}
void ApplyFactionSurfaces(UMeshComponent* Component,int32 Team)
{
    const TCHAR* Faction=APrototypeGameMode::GetFactionIndex(Team)==0?TEXT("Aegis"):TEXT("Ember");
    for(int32 Index=0;Component && Index<Component->GetNumMaterials();++Index)
    {
        UMaterialInterface* Original=Component->GetMaterial(Index);
        if(!Original || !Original->GetName().StartsWith(TEXT("M_CV2_"))) continue;
        const FString Surface=Original->GetName().RightChop(6);
        const FString Name=FString::Printf(TEXT("M_HR_%s_%s"),Faction,*Surface);
        const FString Package=TEXT("/Game/HERO/PresentationV5/Materials/")+Name;
        if(FPackageName::DoesPackageExist(Package))
            if(auto* Variant=LoadObject<UMaterialInterface>(nullptr,*(Package+TEXT(".")+Name))) Component->SetMaterial(Index,Variant);
    }
}
}

APrototypeUnit::APrototypeUnit()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SelectionPlane(TEXT("/Engine/BasicShapes/Plane.Plane"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Technical(TEXT("/Game/HERO/Meshes/SM_HERO_Technical.SM_HERO_Technical"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Prototype/M_Team.M_Team"));
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetStaticMesh(Technical.Succeeded() ? Technical.Object : Cube.Object);
    Mesh->SetRelativeScale3D(FVector::OneVector);
    InfantryMesh=CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("InfantryRig"));
    InfantryMesh->SetupAttachment(RootComponent);InfantryMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    InfantryMesh->SetVisibility(false);
    Turret = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret"));
    Turret->SetupAttachment(RootComponent);
    Turret->SetStaticMesh(Cube.Object);
    Turret->SetRelativeLocation(FVector(0, 0, 40));
    Turret->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.35f));
    Turret->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Turret->SetVisibility(false);
    Barrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel"));
    Barrel->SetupAttachment(RootComponent);
    Barrel->SetStaticMesh(Cube.Object);
    Barrel->SetRelativeLocation(FVector(65, 0, 45));
    Barrel->SetRelativeScale3D(FVector(0.8f, 0.13f, 0.13f));
    Barrel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Barrel->SetVisibility(false);
    Selection = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Selection"));
    Selection->SetupAttachment(RootComponent);
    Selection->SetStaticMesh(SelectionPlane.Succeeded() ? SelectionPlane.Object : Cube.Object);
    Selection->SetRelativeLocation(FVector(0, 0, -31));
    Selection->SetRelativeScale3D(FVector(3.1f, 2.0f, 0.025f));
    Selection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Selection->SetVisibility(false);
    if (Material.Succeeded())
    {
        Selection->SetMaterial(0, Material.Object);
    }
}

void APrototypeUnit::Configure(int32 InTeam, bool bInHQ)
{
    Team = InTeam;
    Destination = GetActorLocation();
    ConfigureRole(bInHQ ? EPrototypeRole::Headquarters : EPrototypeRole::Combat);
}

void APrototypeUnit::ConfigureRole(EPrototypeRole InRole)
{
    // Material-slot indices differ across the authored meshes. A role change
    // must not carry the previous mesh's team-color overrides into armor slots.
    Mesh->EmptyOverrideMaterials();InfantryMesh->EmptyOverrideMaterials();TeamMaterial=nullptr;
    Role = InRole;
    CombatState = FHRCombatState();
    ApproachTarget.Reset();bHasStructureApproach=false;
    Definition = const_cast<UHRUnitDefinition*>(APrototypeGameMode::GetRoleDefinition(Role, Team));
    bHQ = Role == EPrototypeRole::Headquarters;
    const TCHAR* Name = TEXT("SM_HERO_Technical");
    if (bHQ) Name = TEXT("SM_HERO_BlackMarket");
    else if (Role == EPrototypeRole::Factory) Name = TEXT("SM_HERO_Workshop");
    else if (Role == EPrototypeRole::Depot) Name = TEXT("SM_HERO_Depot");
    else if (Role == EPrototypeRole::Collector) Name = TEXT("SM_HERO_Salvager");
    const FString Path = FString::Printf(TEXT("/Game/HERO/Meshes/%s.%s"), Name, Name);
    if (UStaticMesh* Model = LoadObject<UStaticMesh>(nullptr, *Path)) Mesh->SetStaticMesh(Model);
    MaxHealth = bHQ ? 800.f : Role == EPrototypeRole::Factory ? 600.f : Role == EPrototypeRole::Depot ? 400.f : 100.f;
    if (Definition) MaxHealth = FMath::Max(1.f, Definition->Stats.Health);
    Health = MaxHealth;
    Mesh->SetRelativeRotation(FRotator::ZeroRotator);
    Mesh->SetRelativeScale3D(FVector::OneVector);
    Mesh->SetRelativeLocation(FVector::ZeroVector);
    const TCHAR* ConceptName = bHQ ? TEXT("SM_HERO_CommandHub")
        : Role == EPrototypeRole::Factory ? TEXT("SM_HERO_VehiclePlant")
        : Role == EPrototypeRole::Depot ? TEXT("SM_HERO_Refinery")
        : Role == EPrototypeRole::Combat ? TEXT("SM_HERO_Striker") : nullptr;
    UStaticMesh* ConceptModel = ConceptName ? LoadObject<UStaticMesh>(nullptr,
        *FString::Printf(TEXT("/Game/HERO/ConceptV2/Meshes/%s.%s"), ConceptName, ConceptName)) : nullptr;
    if (Definition && !Definition->StaticMesh.IsNull())
        if (UStaticMesh* AuthoredModel = Definition->StaticMesh.LoadSynchronous()) ConceptModel = AuthoredModel;
    if (ConceptModel)
    {
        Mesh->SetStaticMesh(ConceptModel);
        const float StructureScale = ConceptStructureScale(Role);
        Mesh->SetRelativeScale3D(FVector(StructureScale));
        Mesh->SetRelativeLocation(FVector(0,0,(StructureScale-1.f)*(bHQ?110.f:35.f)));
    }
    else if (Role == EPrototypeRole::Combat)
    {
        if (UStaticMesh* Tank = LoadObject<UStaticMesh>(nullptr,TEXT("/Game/LocalAssets/RTSModern/ATV_N1_LOD0.ATV_N1_LOD0")))
        {
            Mesh->SetStaticMesh(Tank);
            Mesh->SetRelativeRotation(FRotator(0.f,90.f,0.f));
            Mesh->SetRelativeScale3D(FVector(.65f));
        }
    }
    BaseVisualScale=Mesh->GetRelativeScale3D();
    Mesh->SetVisibility(true);InfantryMesh->SetVisibility(false);InfantryAnimationState=255;
    if(Role==EPrototypeRole::Infantry && FPackageName::DoesPackageExist(TEXT("/Game/HERO/InfantryRigV4/SK_HR_Infantry")))
    {
        auto* Rig=LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/HERO/InfantryRigV4/SK_HR_Infantry.SK_HR_Infantry"));
        if(Rig)
        {
            InfantryMesh->SetSkeletalMesh(Rig);InfantryMesh->SetVisibility(true);Mesh->SetVisibility(false);
            InfantryIdle=LoadObject<UAnimationAsset>(nullptr,TEXT("/Game/HERO/InfantryRigV4/AN_HR_Infantry_Idle.AN_HR_Infantry_Idle"));
            InfantryWalk=LoadObject<UAnimationAsset>(nullptr,TEXT("/Game/HERO/InfantryRigV4/AN_HR_Infantry_Walk.AN_HR_Infantry_Walk"));
            InfantryFire=LoadObject<UAnimationAsset>(nullptr,TEXT("/Game/HERO/InfantryRigV4/AN_HR_Infantry_Fire.AN_HR_Infantry_Fire"));
        }
    }
    Selection->SetRelativeLocation(FVector(0,0,4.f-GetActorLocation().Z));
    Selection->SetRelativeScale3D(IsStructure() ? FVector(APrototypeGameMode::Footprint(Role)/45.f, APrototypeGameMode::Footprint(Role)/45.f, .025f) : FVector(3.1f,2.f,.025f));
    if (auto* Ring = LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/EnvironmentV2/Materials/M_ENV_SelectionRing.M_ENV_SelectionRing"))) Selection->SetMaterial(0,Ring);
    if (auto* RingMesh = LoadObject<UStaticMesh>(nullptr,TEXT("/Game/HERO/EnvironmentV2/Meshes/SM_ENV_SelectionRing.SM_ENV_SelectionRing"))) Selection->SetStaticMesh(RingMesh);
    ApplyFactionSurfaces(Mesh,Team);
    if(InfantryMesh->IsVisible()) ApplyFactionSurfaces(InfantryMesh,Team);
    for (const FName Slot : {FName(TEXT("Team")), FName(TEXT("Neon"))})
    {
        const int32 Index = InfantryMesh->IsVisible()?InfantryMesh->GetMaterialIndex(Slot):Mesh->GetMaterialIndex(Slot);
        if (Index == INDEX_NONE) continue;
        UMaterialInstanceDynamic* Material = InfantryMesh->IsVisible()?InfantryMesh->CreateDynamicMaterialInstance(Index):Mesh->CreateDynamicMaterialInstance(Index);
        if (Material) Material->SetVectorParameterValue(TEXT("Tint"),FactionMarkingColor(Team,Slot==FName(TEXT("Neon"))));
        if (Slot == FName(TEXT("Team"))) TeamMaterial = Material;
    }
}

void APrototypeUnit::Select(bool bValue)
{
    bSelected = bValue;
    Selection->SetVisibility(bValue);
}

void APrototypeUnit::OrderMove(const FVector& Position)
{
    if (GetWorld()->IsPaused() || IsStructure() || CombatState.bDeployed) return;
    AttackTarget = nullptr;
    StrategicTarget = nullptr;
    bMoveOrdered = true;
    const FVector PreviousDestination = Destination;
    Destination = BattlefieldLayout::ClampPoint(FVector(Position.X,Position.Y,35.f));
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!It->IsStructure() || It->Health <= 0 || It->IsActorBeingDestroyed()) continue;
        Destination=FPrototypeNavigation::ProjectOutsideStructure(*It,Destination,
            GetActorLocation()-It->GetActorLocation(),90.f);
    }
    if (FVector::DistSquared2D(PreviousDestination,Destination)>100.f)
    {
        Waypoints.Empty(); WaypointIndex=0; RepathCooldown=0.f;
    }
    ScanCooldown = 0.6f;
}

bool APrototypeUnit::OrderManualMove(const FVector& Position)
{
    if (GetWorld()->IsPaused() || IsStructure() || CombatState.bDeployed || IsActorBeingDestroyed() || Health <= 0.f) return false;
    TArray<FVector> Route;
    const FVector Goal(Position.X,Position.Y,35.f);
    if (!FPrototypeNavigation::FindWorldPath(GetWorld(),GetActorLocation(),Goal,Route)) return false;
    if (Role == EPrototypeRole::Collector)
        if (auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>()) Mode->ReleaseSalvageReservation(this);
    OrderMove(Goal);
    bCollectorManualOrder = Role == EPrototypeRole::Collector;
    Waypoints = MoveTemp(Route);
    WaypointIndex = 0;
    PathGoal = Destination;
    RepathCooldown = 1.f;
    return true;
}

void APrototypeUnit::OrderAttack(APrototypeUnit* Target)
{
    if (GetWorld()->IsPaused() || !IsCombatant()) return;
    if (IsValid(Target) && !Target->IsActorBeingDestroyed() && Target->Team != Team && Target->Health > 0)
    {
        auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
        if (!Mode || (!Mode->CanTeamSee(Team, Target) && !Target->bHQ)) return;
        bMoveOrdered = false;
        AttackTarget = Mode->CanTeamSee(Team, Target) ? Target : nullptr;
        StrategicTarget = Target->bHQ ? Target : nullptr;
        ScanCooldown = 0.f;
    }
}

bool APrototypeUnit::IsCombatant() const
{
    return Role == EPrototypeRole::Combat || Role == EPrototypeRole::Infantry
        || Role == EPrototypeRole::Siege || Role == EPrototypeRole::Defense;
}

bool APrototypeUnit::IsInCover() const
{
    if (Role != EPrototypeRole::Infantry || !GetWorld()) return false;
    const FVector Position = GetActorLocation();
    auto NearBoundary = [&Position](const FVector& Center, float Radius)
    {
        const float Distance = FVector::Dist2D(Position, Center);
        return Distance >= Radius && Distance <= Radius + 240.f;
    };
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (*It != this && It->IsStructure() && It->Team == Team && It->IsCompleted()
            && It->Health > 0.f && !It->IsActorBeingDestroyed()
            && NearBoundary(It->GetActorLocation(), APrototypeGameMode::Footprint(It->Role))) return true;
    for (float X : BattlefieldLayout::CityColumns)
    for (float Y : BattlefieldLayout::CityRows)
        if (NearBoundary(FVector(X,Y,0), BattlefieldLayout::CityObstacleRadius)) return true;
    for (float X : BattlefieldLayout::WestHouseColumns)
    for (float Y : BattlefieldLayout::WestHouseRows)
        if (NearBoundary(FVector(X,Y,0), BattlefieldLayout::WestHouseRadius)) return true;
    return false;
}

float APrototypeUnit::GetEffectiveDamage() const
{
    const float Base = Definition ? Definition->Stats.WeaponDamage
        : Role == EPrototypeRole::Infantry ? 12.f : Role == EPrototypeRole::Siege ? 38.f
        : Role == EPrototypeRole::Defense ? 28.f : 20.f;
    return FMath::Max(0.f, Base) * CombatState.GetVeterancyModifiers(Definition).Damage
        * (CombatState.OverdriveRemaining > 0.f ? 1.25f : 1.f);
}

float APrototypeUnit::GetEffectiveRange() const
{
    const float Base = Definition ? Definition->Stats.WeaponRangeCm
        : Role == EPrototypeRole::Infantry ? 420.f : Role == EPrototypeRole::Siege ? 650.f
        : Role == EPrototypeRole::Defense ? 850.f : 480.f;
    return FMath::Max(0.f, Base) * CombatState.GetVeterancyModifiers(Definition).Range
        * (CombatState.bDeployed ? 1.5f : 1.f);
}

float APrototypeUnit::GetEffectiveArmor() const
{
    const auto* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<APrototypeGameMode>() : nullptr;
    const float FactionFactor = Mode ? Mode->GetFactionArmorMultiplier(Team) : (Team == 0 ? 1.15f : .9f);
    return FactionFactor * CombatState.GetVeterancyModifiers(Definition).Armor
        * (CombatState.bDeployed ? 1.5f : 1.f);
}

float APrototypeUnit::GetEffectiveSpeed() const
{
    if (IsStructure() || CombatState.bDeployed) return 0.f;
    const float Base = Definition ? FMath::Max(0.f, Definition->Stats.MoveSpeedCm) : 220.f;
    const auto* Mode = GetWorld() ? GetWorld()->GetAuthGameMode<APrototypeGameMode>() : nullptr;
    const float FactionFactor = Mode ? Mode->GetFactionMoveMultiplier(Team) : (Team == 1 ? 280.f / 220.f : 1.f);
    return Base * FactionFactor
        * (Role == EPrototypeRole::Infantry && CombatState.IsSuppressed() ? .45f : 1.f)
        * (CombatState.OverdriveRemaining > 0.f ? 1.35f : 1.f);
}

float APrototypeUnit::ReceiveCombatHit(float RawDamage, EHRDamageType DamageType, APrototypeUnit* Attacker)
{
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if (!Mode || GetWorld()->IsPaused() || Mode->Outcome != 0 || Health <= 0.f || !FMath::IsFinite(RawDamage) || RawDamage <= 0.f) return 0.f;
    const EHRArmorClass ArmorClass = IsStructure() ? EHRArmorClass::Structure
        : Role == EPrototypeRole::Infantry ? EHRArmorClass::Infantry : EHRArmorClass::Vehicle;
    const float Damage = FHRDamageTable::Resolve(RawDamage, DamageType, ArmorClass,
        GetEffectiveArmor(), IsInCover(), Mode->GetDamageMatrix());
    const float ActualDamage = FMath::Min(Health, Damage);
    if (ActualDamage <= 0.f) return 0.f;
    const bool bKilled = ActualDamage >= Health;
    CombatState.LastDamageAgo = 0.f;
    if (Role == EPrototypeRole::Infantry) CombatState.AddSuppression(ActualDamage, MaxHealth);
    if (IsValid(Attacker) && !Attacker->IsActorBeingDestroyed() && Attacker->Health > 0.f && Attacker->Team != Team)
    {
        Attacker->CombatState.AddExperience(ActualDamage, bKilled);
        Mode->AwardCommanderPoints(Attacker->Team, ActualDamage * .1f + (bKilled ? 10.f : 0.f));
    }
    ReceiveHit(ActualDamage);
    return ActualDamage;
}

bool APrototypeUnit::TryActivateAbility(EHRAbility Ability)
{
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    const bool bOperational = Mode && Mode->Outcome == 0 && !GetWorld()->IsPaused() && Health > 0.f && !IsActorBeingDestroyed()
        && IsCompleted() && Mode->IsUnitOperational(this);
    if (Ability == EHRAbility::Deploy)
    {
        if (!CombatState.CommitAbility(Ability, Role == EPrototypeRole::Siege, bOperational, true)) return false;
        bMoveOrdered = false;
        Destination = GetActorLocation();
        Waypoints.Empty();
        return true;
    }
    if (Ability == EHRAbility::Smoke)
    {
    const bool bSupported = APrototypeGameMode::GetFactionIndex(Team) == 0 && IsCombatant() && !IsStructure();
        if (!CombatState.CommitAbility(Ability, bSupported, bOperational, true)) return false;
        Mode->AddSmokeArea(Team, GetActorLocation(), 600.f, 6.f);
        return true;
    }
    if (Ability != EHRAbility::Grenade || !CombatState.CanActivate(Ability,
        Role == EPrototypeRole::Infantry, bOperational, true)) return false;
    APrototypeUnit* Target = nullptr;
    float Nearest = FMath::Square(800.f);
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (It->Team == Team || It->Health <= 0.f || It->IsActorBeingDestroyed() || !Mode->CanTeamSee(Team, *It)) continue;
        const float Distance = FVector::DistSquared2D(GetActorLocation(), It->GetActorLocation());
        if (Distance <= Nearest) { Nearest = Distance; Target = *It; }
    }
    if (!Target || !CombatState.CommitAbility(Ability, true, bOperational, true)) return false;
    const FVector Impact = Target->GetActorLocation();
    TArray<APrototypeUnit*> Victims;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
        if (It->Team != Team && It->Health > 0.f && !It->IsActorBeingDestroyed()
            && FVector::DistSquared2D(Impact, It->GetActorLocation()) <= FMath::Square(250.f)) Victims.Add(*It);
    for (APrototypeUnit* Victim : Victims)
        if (IsValid(Victim) && !Victim->IsActorBeingDestroyed()) Victim->ReceiveCombatHit(55.f, EHRDamageType::Explosive, this);
    Mode->SpawnCombatEffect(Impact,Team,true);
    return true;
}

void APrototypeUnit::ReceiveHit(float Damage)
{
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if (!Mode || GetWorld()->IsPaused() || Mode->Outcome != 0 || !FMath::IsFinite(Damage) || Damage <= 0 || Health <= 0) return;
    CombatState.LastDamageAgo = 0.f;
    Health = FMath::Max(0.f, Health - Damage);
    FlashTime = 0.12f;
    if (Health <= 0)
    {
        Mode->ReleaseSalvageReservation(this);
        if (bHQ) Mode->OnHeadquartersDestroyed(Team);
        Mode->SpawnCombatEffect(GetActorLocation(),Team,true);
        Destroy();
        Mode->RefreshPowerGrid();Mode->UpdateVisibility();
    }
}

bool APrototypeUnit::UpdateStructureApproach(APrototypeUnit* Structure)
{
    if(!IsValid(Structure) || Structure->IsActorBeingDestroyed()) return false;
    const bool bSame=ApproachTarget.Get()==Structure;
    const auto Obstacles=FPrototypeNavigation::GetWorldObstacles(GetWorld());
    const bool bPointLegal=bHasStructureApproach && FPrototypeNavigation::IsWalkable(CachedStructureApproach,Obstacles);
    const bool bNeedRoute=Waypoints.IsEmpty() && RepathCooldown<=0.f;
    if(bSame && bPointLegal && !bNeedRoute) { Destination=CachedStructureApproach;return true; }
    if(bSame && !bHasStructureApproach && RepathCooldown>0.f) { Destination=GetActorLocation();return false; }
    FVector Candidate;
    ApproachTarget=Structure;
    bHasStructureApproach=FPrototypeNavigation::FindStructureApproach(GetWorld(),Structure,GetActorLocation(),GetEffectiveRange(),Candidate);
    if(bHasStructureApproach) { CachedStructureApproach=Candidate;Destination=Candidate;return true; }
    Destination=GetActorLocation();RepathCooldown=1.f;return false;
}

void APrototypeUnit::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if (!Mode || GetWorld()->IsPaused() || Mode->Outcome != 0 || Health <= 0.f || IsActorBeingDestroyed()
        || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.f) return;
    const float RegenerationSeconds = FMath::Max(0.f, CombatState.LastDamageAgo + DeltaSeconds - 4.f)
        - FMath::Max(0.f, CombatState.LastDamageAgo - 4.f);
    CombatState.Advance(DeltaSeconds);
    if(Role==EPrototypeRole::Infantry && InfantryMesh->IsVisible())
    {
        const float Interval=Definition?Definition->Stats.AttackInterval:.6f;
        const uint8 State=FireCooldown>Interval-.15f?2:FVector::DistSquared2D(GetActorLocation(),Destination)>25.f?1:0;
        if(State!=InfantryAnimationState)
        {
            UAnimationAsset* Animation=State==2?InfantryFire:State==1?InfantryWalk:InfantryIdle;
            if(Animation) InfantryMesh->PlayAnimation(Animation,State!=2);
            InfantryAnimationState=State;
        }
    }
    if (IsCombatant() && CombatState.Rank >= 3 && CombatState.LastDamageAgo >= 4.f)
        Health = FMath::Min(MaxHealth, Health + RegenerationSeconds * 2.f);
    if (FlashTime > 0)
    {
        FlashTime -= DeltaSeconds;
        if (TeamMaterial) TeamMaterial->SetVectorParameterValue(TEXT("Tint"), FlashTime > 0 ? FLinearColor::White :
            FactionMarkingColor(Team));
    }
    if (IsStructure())
    {
        const float HeightFactor=.15f+.85f*ConstructionProgress;
        Mesh->SetRelativeScale3D(BaseVisualScale*FVector(1.f,1.f,HeightFactor));
        Mesh->SetRelativeLocation(FVector(0,0,(BaseVisualScale.Z*HeightFactor-1.f)*(bHQ?110.f:35.f)));
        if (Role != EPrototypeRole::Defense || !IsCompleted() || !Mode->IsUnitOperational(this)) return;
    }
    if (Role == EPrototypeRole::Collector) Mode->TickCollector(this, DeltaSeconds);
    FireCooldown -= DeltaSeconds;
    ScanCooldown -= DeltaSeconds;
    if (!IsValid(AttackTarget) || AttackTarget->IsActorBeingDestroyed() || AttackTarget->Health <= 0) AttackTarget = nullptr;
    if (AttackTarget && !Mode->CanTeamSee(Team, AttackTarget)) AttackTarget = nullptr;
    if (!IsValid(StrategicTarget) || StrategicTarget->IsActorBeingDestroyed() || StrategicTarget->Health <= 0) StrategicTarget = nullptr;
    if (!AttackTarget && StrategicTarget && Mode->CanTeamSee(Team, StrategicTarget)) AttackTarget = StrategicTarget;
    if (!AttackTarget && StrategicTarget && !IsStructure() && !CombatState.bDeployed)
    {
        UpdateStructureApproach(StrategicTarget);
    }
    if (IsCombatant() && Mode->IsUnitOperational(this) && ScanCooldown <= 0)
    {
        ScanCooldown = 0.35f;
        if (!bMoveOrdered && (!AttackTarget || AttackTarget->bHQ))
        {
            float Nearest = TNumericLimits<float>::Max();
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            {
                if (It->IsActorBeingDestroyed() || It->Team == Team || It->Health <= 0 || !Mode->CanTeamSee(Team, *It)) continue;
                if (StrategicTarget && It->bHQ) continue;
                float Distance = FVector::DistSquared2D(GetActorLocation(), It->GetActorLocation());
                const float ScanRange=It->IsStructure()
                    ? FMath::Max(FMath::Max(650.f,GetEffectiveRange()),FPrototypeNavigation::StructureApproachRadius(*It,GetActorLocation()-It->GetActorLocation(),90.f)+170.f)
                    : FMath::Max(650.f,GetEffectiveRange());
                if (Distance < Nearest && Distance < FMath::Square(ScanRange)) { Nearest = Distance; AttackTarget = *It; }
            }
        }
    }
    if (AttackTarget && IsCombatant() && Mode->IsUnitOperational(this))
    {
        const FVector Delta = AttackTarget->GetActorLocation() - GetActorLocation();
        // Structure fire/approach distances include the footprint so enlarged
        // buildings can be attacked from reachable ground outside their blocker.
        const float WeaponRange = GetEffectiveRange();
        const float Range = AttackTarget->IsStructure()
            ? FMath::Max(AttackTarget->bHQ ? FMath::Max(620.f, WeaponRange) : WeaponRange,
                FPrototypeNavigation::StructureApproachRadius(AttackTarget,GetActorLocation()-AttackTarget->GetActorLocation(),90.f)+170.f)
            : WeaponRange;
        SetActorRotation(FRotator(0, Delta.Rotation().Yaw, 0));
        if (Delta.SizeSquared2D() <= FMath::Square(Range))
        {
            if (FireCooldown <= 0)
            {
                FireCooldown = (Definition ? FMath::Max(.01f, Definition->Stats.AttackInterval) : 1.f)
                    * (Role == EPrototypeRole::Infantry && CombatState.IsSuppressed() ? 1.75f : 1.f);
                Mode->SpawnWeaponEffect(GetActorLocation()+FVector(0,0,Role==EPrototypeRole::Infantry?100.f:90.f),
                    AttackTarget->GetActorLocation()+FVector(0,0,90.f),Team,Role);
                EHRDamageType DamageType = Team == 1 ? EHRDamageType::Tempest
                    : Role == EPrototypeRole::Siege ? EHRDamageType::Explosive : EHRDamageType::Kinetic;
                if (Definition && Definition->Stats.DamageType.IsValid())
                    DamageType = Definition->Stats.DamageType == HRRTSTags::Damage_Tempest ? EHRDamageType::Tempest
                        : Definition->Stats.DamageType == HRRTSTags::Damage_Explosive ? EHRDamageType::Explosive : EHRDamageType::Kinetic;
                AttackTarget->ReceiveCombatHit(GetEffectiveDamage(), DamageType, this);
            }
            return;
        }
        if(AttackTarget->IsStructure()) UpdateStructureApproach(AttackTarget);
        else
        {
            const FVector Approach=GetActorLocation()-AttackTarget->GetActorLocation();
            Destination=BattlefieldLayout::ClampPoint(AttackTarget->GetActorLocation()+Approach.GetSafeNormal2D()*Range*.88f);
            Destination.Z=35.f;
        }
    }
    if (GetWorld()->IsPaused() || IsStructure() || CombatState.bDeployed) return;
    RepathCooldown -= DeltaSeconds;
    if (FVector::DistSquared2D(GetActorLocation(),Destination)<=25.f) { bMoveOrdered=false; return; }
    if (FVector::DistSquared2D(PathGoal,Destination)>FMath::Square(100.f)
        || (WaypointIndex>=Waypoints.Num() && RepathCooldown<=0.f))
    {
        PathGoal=Destination; WaypointIndex=0; RepathCooldown=1.f;
        if (!FPrototypeNavigation::FindWorldPath(GetWorld(),GetActorLocation(),Destination,Waypoints)) bCollectorManualOrder=false;
    }
    // A near waypoint may only be skipped when the resulting whole chord is legal.
    // Otherwise reach the original safe bend instead of repeatedly cutting its corner.
    while (Waypoints.IsValidIndex(WaypointIndex)
        && FVector::DistSquared2D(GetActorLocation(),Waypoints[WaypointIndex])
            < FMath::Square(WaypointIndex==Waypoints.Num()-1?5.f:30.f))
    {
        if(Waypoints.IsValidIndex(WaypointIndex+1)
            && !FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),GetActorLocation(),Waypoints[WaypointIndex+1]))
        {
            // A new structure can invalidate the original next leg. Once the bend
            // is reached, discard that route instead of taking zero-length steps forever.
            if(FVector::DistSquared2D(GetActorLocation(),Waypoints[WaypointIndex])<=1.f)
            { Waypoints.Empty();WaypointIndex=0;RepathCooldown=0.f; }
            break;
        }
        ++WaypointIndex;
    }
    if (!Waypoints.IsValidIndex(WaypointIndex)) return;
    const FVector Delta=Waypoints[WaypointIndex]-GetActorLocation();
    const FVector Direction=Delta.GetSafeNormal2D();
    const float MoveSpeed = GetEffectiveSpeed();
    const FVector Step=Direction*FMath::Min(MoveSpeed*DeltaSeconds,Delta.Size2D());
    const FVector Location=GetActorLocation()+Step;
    if (FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),GetActorLocation(),Location))
    {
        SetActorLocation(Location);
        if (!AttackTarget) SetActorRotation(FRotator(0,Direction.Rotation().Yaw,0));
    }
    else { Waypoints.Empty(); WaypointIndex=0; RepathCooldown=.2f; }
}

APrototypeCamera::APrototypeCamera()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot")));
    auto* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetRelativeRotation(FRotator(-52.f, 45.f, 0.f));
}

APrototypeController::APrototypeController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bShouldPerformFullTickWhenPaused = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
}

void APrototypeController::BeginPlay()
{
    Super::BeginPlay();
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    if (GetPawn()) GetPawn()->SetActorLocation(FVector(BattlefieldLayout::PlayerHQX,-1800.f,3300.f));
    PanToWorld(FVector(BattlefieldLayout::PlayerHQX,0,35));
    SetViewTarget(GetPawn());
}

void APrototypeController::ClearSelection()
{
    for (auto* Unit : Selected) if (IsValid(Unit)) Unit->Select(false);
    Selected.Empty();
    bDragging = false;
    bPlacing = false;
    if (IsValid(PlacementGhost)) PlacementGhost->Destroy();
    PlacementGhost = nullptr;
}

void APrototypeController::BeginPlacement(EPrototypeRole InRole)
{
    ClearSelection();
    PlacementRole = InRole;
    PlacementRotation=0;
    if(auto* Mode=GetWorld()->GetAuthGameMode<APrototypeGameMode>()) Mode->BuildingQuarterTurns=0;
    bPlacing = true;
    PlacementGhost = GetWorld()->SpawnActor<AStaticMeshActor>();
    if (PlacementGhost)
    {
        auto* Component = PlacementGhost->GetStaticMeshComponent();
        Component->SetMobility(EComponentMobility::Movable);
        const TCHAR* Name = InRole == EPrototypeRole::Factory ? TEXT("SM_HERO_Workshop") : TEXT("SM_HERO_Depot");
        const TCHAR* ConceptName = InRole == EPrototypeRole::Factory ? TEXT("SM_HERO_VehiclePlant") : TEXT("SM_HERO_Refinery");
        UStaticMesh* GhostMesh = LoadObject<UStaticMesh>(nullptr,
            *FString::Printf(TEXT("/Game/HERO/ConceptV2/Meshes/%s.%s"),ConceptName,ConceptName));
        if (const UHRUnitDefinition* Def=APrototypeGameMode::GetRoleDefinition(InRole))
            if (!Def->StaticMesh.IsNull()) GhostMesh=Def->StaticMesh.LoadSynchronous();
        const bool bConceptMesh=GhostMesh!=nullptr;
        if (!GhostMesh) GhostMesh = LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/HERO/Meshes/%s.%s"),Name,Name));
        Component->SetStaticMesh(GhostMesh);
        PlacementGhost->SetActorScale3D(FVector(bConceptMesh ? ConceptStructureScale(InRole) : 1.f));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCastShadow(false);
        Component->SetVisibility(false);
    }
}

void APrototypeController::AddSelected(APrototypeUnit* Unit)
{
    if (IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Team == 0 && Unit->Health > 0)
    {
        Selected.AddUnique(Unit);
        Unit->Select(true);
    }
}

APrototypeUnit* APrototypeController::GetSelectedProducer() const
{
    for (APrototypeUnit* Unit : Selected)
        if (IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health > 0.f && Unit->Team == 0
            && (Unit->Role == EPrototypeRole::Factory || Unit->Role == EPrototypeRole::Headquarters || Unit->Role==EPrototypeRole::Barracks)) return Unit;
    return nullptr;
}

void APrototypeController::FocusSelection()
{
    APawn* Camera = GetPawn();
    const UCameraComponent* View = Camera ? Camera->FindComponentByClass<UCameraComponent>() : nullptr;
    if (!View) return;
    FVector Center = FVector::ZeroVector;
    int32 Count = 0;
    for (APrototypeUnit* Unit : Selected)
        if (IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health > 0.f) { Center += Unit->GetActorLocation(); ++Count; }
    if (!Count) return;
    Center /= Count;
    PanToWorld(Center);
}

void APrototypeController::PanToWorld(const FVector& Center)
{
    APawn* Camera=GetPawn();
    const UCameraComponent* View=Camera?Camera->FindComponentByClass<UCameraComponent>():nullptr;
    if(!View || Center.ContainsNaN()) return;
    const FVector Forward = View->GetForwardVector();
    if (Forward.Z >= -.001f) return;
    const float Distance = (35.f - Camera->GetActorLocation().Z) / Forward.Z;
    const FVector Position = FVector(Center.X, Center.Y, 35.f) - Forward * Distance;
    Camera->SetActorLocation(Position);
}

void APrototypeController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if (!Mode) return;
    if (WasInputKeyJustPressed(EKeys::Pause) || WasInputKeyJustPressed(EKeys::F10)) SetPause(!GetWorld()->IsPaused());
    if (Mode->Outcome != 0 && bPlacing) ClearSelection();
    Selected.RemoveAll([](APrototypeUnit* Unit) { return !IsValid(Unit) || Unit->IsActorBeingDestroyed() || Unit->Health <= 0.f; });
    if (WasInputKeyJustPressed(EKeys::R)) { Mode->ResetMatch(); return; }
    if (APawn* Camera = GetPawn())
    {
        const UCameraComponent* View = Camera->FindComponentByClass<UCameraComponent>();
        const FRotator Yaw(0.f, View ? View->GetComponentRotation().Yaw : Camera->GetActorRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X) * (IsInputKeyDown(EKeys::W) - IsInputKeyDown(EKeys::S))
            + FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y) * (IsInputKeyDown(EKeys::D) - IsInputKeyDown(EKeys::A));
        FVector Location = Camera->GetActorLocation() + Direction.GetClampedToMaxSize(1) * 4500.f * DeltaTime;
        if (WasInputKeyJustPressed(EKeys::MouseScrollUp)) Location.Z -= 900.f;
        if (WasInputKeyJustPressed(EKeys::MouseScrollDown)) Location.Z += 900.f;
        Location.Z = FMath::Clamp(Location.Z,1600.f,28000.f);
        Location.X = FMath::Clamp(Location.X,-22000.f,15000.f);
        Location.Y = FMath::Clamp(Location.Y,-18000.f,12000.f);
        Camera->SetActorLocation(Location);
    }
    if (Mode->Outcome != 0 || GetWorld()->IsPaused()) return;
    const FKey GroupKeys[] = { EKeys::Zero, EKeys::One, EKeys::Two, EKeys::Three, EKeys::Four,
        EKeys::Five, EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine };
    for (int32 Number = 0; Number < 10; ++Number)
    {
        if (!WasInputKeyJustPressed(GroupKeys[Number])) continue;
        if (IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl))
        {
            TArray<TWeakObjectPtr<APrototypeUnit>>& Group = ControlGroups.FindOrAdd(Number);
            Group.Empty();
            for (APrototypeUnit* Unit : Selected) Group.Add(Unit);
            Mode->Feedback = FString::Printf(TEXT("Group %d saved: %d selected."), Number, Group.Num());
        }
        else
        {
            ClearSelection();
            if (const TArray<TWeakObjectPtr<APrototypeUnit>>* Group = ControlGroups.Find(Number))
                for (const TWeakObjectPtr<APrototypeUnit>& Unit : *Group) AddSelected(Unit.Get());
            const float Now = GetWorld()->GetTimeSeconds();
            if (LastRecalledGroup == Number && Now - LastGroupRecallTime <= .3f) FocusSelection();
            LastRecalledGroup = Number;
            LastGroupRecallTime = Now;
            Mode->Feedback = FString::Printf(TEXT("Group %d selected: %d units."), Number, Selected.Num());
        }
    }
    if (WasInputKeyJustPressed(EKeys::F)) FocusSelection();
    const TArray<APrototypeUnit*> ActionSelection=Selected;
    for (APrototypeUnit* Unit:ActionSelection)
    {
        if (WasInputKeyJustPressed(EKeys::G)) Unit->TryActivateAbility(EHRAbility::Grenade);
        if (WasInputKeyJustPressed(EKeys::T)) Unit->TryActivateAbility(EHRAbility::Smoke);
        if (WasInputKeyJustPressed(EKeys::Y)) Unit->TryActivateAbility(EHRAbility::Deploy);
        if (WasInputKeyJustPressed(EKeys::K)) Mode->TogglePower(Unit);
        if (WasInputKeyJustPressed(EKeys::Delete)) Mode->SellBuilding(Unit);
    }
    if (auto* HUD=Cast<APrototypeHUD>(GetHUD()))
    {
        const bool bAegis=APrototypeGameMode::GetFactionIndex(0)==0;
        if (WasInputKeyJustPressed(EKeys::Z)) BeginPrototypeDoctrineTarget(HUD,bAegis?EHRDoctrine::RepairPulse:EHRDoctrine::Scorch);
        if (WasInputKeyJustPressed(EKeys::X)) BeginPrototypeDoctrineTarget(HUD,bAegis?EHRDoctrine::PrecisionStrike:EHRDoctrine::Overdrive);
        const bool bCancelAim=HasPrototypeDoctrineTarget(HUD)
            && (WasInputKeyJustPressed(EKeys::Escape) || WasInputKeyJustPressed(EKeys::RightMouseButton));
        if (bCancelAim) { CancelPrototypeDoctrineTarget(HUD);return; }
    }
    if (WasInputKeyJustPressed(EKeys::P))
    {
        if (APrototypeUnit* Producer = GetSelectedProducer()) Mode->RequestProductionAt(Producer, EPrototypeRole::Combat);
        else Mode->TrainTank();
    }
    if (WasInputKeyJustPressed(EKeys::C))
    {
        if (APrototypeUnit* Producer = GetSelectedProducer()) Mode->RequestProductionAt(Producer, EPrototypeRole::Collector);
        else Mode->RequestProduction(EPrototypeRole::Collector);
    }
    if (WasInputKeyJustPressed(EKeys::BackSpace)) Mode->CancelProduction(GetSelectedProducer());
    if (WasInputKeyJustPressed(EKeys::B)) BeginPlacement(EPrototypeRole::Factory);
    if (WasInputKeyJustPressed(EKeys::V)) BeginPlacement(EPrototypeRole::Depot);
    if (WasInputKeyJustPressed(EKeys::Escape)) ClearSelection();
    if (WasInputKeyJustPressed(EKeys::SpaceBar))
    {
        ClearSelection();
        for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) if (It->Role == EPrototypeRole::Combat) AddSelected(*It);
    }
    FVector2D Mouse;
    if (!GetMousePosition(Mouse.X, Mouse.Y)) return;
    if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
        if (auto* HUD = Cast<APrototypeHUD>(GetHUD())) if (HUD->HandleClick(Mouse)) return;
    if (bPlacing)
    {
        if(WasInputKeyJustPressed(EKeys::Q)) { PlacementRotation=(PlacementRotation+1)%4;Mode->BuildingQuarterTurns=PlacementRotation; }
        if (WasInputKeyJustPressed(EKeys::RightMouseButton)) { ClearSelection(); return; }
        FVector Origin, Direction;
        if (DeprojectMousePositionToWorld(Origin, Direction) && Direction.Z < -.001f)
        {
            FVector Position = Mode->SnapBuildPosition(Origin + Direction * ((35.f-Origin.Z)/Direction.Z));
            const FIntPoint Dimensions=Mode->GetBuildDimensions(PlacementRole,PlacementRotation);
            const FVector Half(Dimensions.X*100.f,Dimensions.Y*100.f,30.f);
            const bool Valid = Mode->CanPlaceBuilding(PlacementRole, Position) && Mode->Credits >= APrototypeGameMode::RoleCost(PlacementRole);
            if (IsValid(PlacementGhost))
            {
                PlacementGhost->SetActorLocation(Position+FVector(0,0,(PlacementGhost->GetActorScale3D().Z-1.f)*35.f));
                PlacementGhost->SetActorRotation(FRotator(0,PlacementRotation*90.f,0));
                PlacementGhost->GetStaticMeshComponent()->SetVisibility(true);
            }
            const FColor GridColor=Valid?FColor(65,210,170):FColor(230,70,45);
            DrawDebugBox(GetWorld(), Position, Half, GridColor, false, 0.f, 0, 3.f);
            for(float X=-Half.X;X<=Half.X;X+=200.f) DrawDebugLine(GetWorld(),Position+FVector(X,-Half.Y,3),Position+FVector(X,Half.Y,3),GridColor,false,0.f,0,1.f);
            for(float Y=-Half.Y;Y<=Half.Y;Y+=200.f) DrawDebugLine(GetWorld(),Position+FVector(-Half.X,Y,3),Position+FVector(Half.X,Y,3),GridColor,false,0.f,0,1.f);
            if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
                if (Mode->BeginBuilding(PlacementRole,Position)) ClearSelection();
        }
        return;
    }
    int32 ViewWidth = 0, ViewHeight = 0;
    GetViewportSize(ViewWidth, ViewHeight);
    const APrototypeHUD* Interface=Cast<APrototypeHUD>(GetHUD());
    if (Interface && Interface->IsOverInterface(Mouse))
    {
        if (WasInputKeyJustReleased(EKeys::LeftMouseButton)) bDragging = false;
        return;
    }
    if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
    {
        if (!IsInputKeyDown(EKeys::LeftShift) && !IsInputKeyDown(EKeys::RightShift)) ClearSelection();
        DragStart = Mouse;
        bDragging = true;
    }
    if (WasInputKeyJustReleased(EKeys::LeftMouseButton) && bDragging)
    {
        bDragging = false;
        if (FVector2D::Distance(Mouse, DragStart) < 8.f)
        {
            FHitResult Hit;
            GetHitResultUnderCursor(ECC_Visibility, true, Hit);
            APrototypeUnit* Clicked = Cast<APrototypeUnit>(Hit.GetActor());
            const float Now = GetWorld()->GetTimeSeconds();
            const bool bDoubleClick = IsValid(Clicked) && LastSelectionClick.Get() == Clicked
                && Now - LastSelectionClickTime <= .25f;
            AddSelected(Clicked);
            if (bDoubleClick && Clicked->Team == 0 && !Clicked->IsStructure())
                for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
                {
                    FVector2D Point;
                    if (It->Role == Clicked->Role && !It->IsStructure() && ProjectWorldLocationToScreen(It->GetActorLocation(), Point)
                        && Point.X >= 0.f && Point.X <= ViewWidth && Point.Y >= 0.f && Point.Y <= ViewHeight
                        && (!Interface || !Interface->IsOverInterface(Point))) AddSelected(*It);
                }
            LastSelectionClick = Clicked;
            LastSelectionClickTime = Now;
        }
        else
        {
            FVector2D Min(FMath::Min(Mouse.X, DragStart.X), FMath::Min(Mouse.Y, DragStart.Y));
            FVector2D Max(FMath::Max(Mouse.X, DragStart.X), FMath::Max(Mouse.Y, DragStart.Y));
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            {
                FVector2D Point;
                if (!It->IsStructure() && ProjectWorldLocationToScreen(It->GetActorLocation(), Point)
                    && Point.X >= Min.X && Point.X <= Max.X && Point.Y >= Min.Y && Point.Y <= Max.Y) AddSelected(*It);
            }
        }
    }
    if (WasInputKeyJustPressed(EKeys::RightMouseButton))
    {
        FHitResult Hit;
        if (GetHitResultUnderCursor(ECC_Visibility, true, Hit))
        {
            auto* Target = Cast<APrototypeUnit>(Hit.GetActor());
            int32 Index = 0;
            int32 MobileCount = 0;
            for (APrototypeUnit* Unit : Selected) if (!Unit->IsStructure()) ++MobileCount;
            const int32 Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(MobileCount))));
            for (auto* Unit : Selected)
            {
                if (Unit->IsStructure())
                {
                    if (Unit->Role == EPrototypeRole::Factory || Unit->Role == EPrototypeRole::Headquarters || Unit->Role==EPrototypeRole::Barracks)
                        Mode->SetRallyPoint(Unit, Hit.ImpactPoint);
                    continue;
                }
                if (Target && Target->Team != 0 && Mode->CanTeamSee(0,Target)) Unit->OrderAttack(Target);
                else if (!Unit->OrderManualMove(Hit.ImpactPoint + FVector((Index / Columns) * 150.f, (Index % Columns - (Columns - 1) * 0.5f) * 150.f, 0)))
                    Mode->Feedback=TEXT("Move rejected: destination blocked or unreachable.");
                ++Index;
            }
            DrawDebugSphere(GetWorld(), Hit.ImpactPoint + FVector(0, 0, 10), 40, 12, FColor::Green, false, 0.8f, 0, 3.f);
        }
    }
}

APrototypeGameMode::APrototypeGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = APrototypeCamera::StaticClass();
    PlayerControllerClass = APrototypeController::StaticClass();
    HUDClass = APrototypeHUD::StaticClass();
    static ConstructorHelpers::FObjectFinder<UHRUnitDefinition> Striker(TEXT("/Game/HERO/DataV3/DA_HR_Striker.DA_HR_Striker"));
    static ConstructorHelpers::FObjectFinder<UHRUnitDefinition> Harvester(TEXT("/Game/HERO/DataV3/DA_HR_Harvester.DA_HR_Harvester"));
    static ConstructorHelpers::FObjectFinder<UHRUnitDefinition> Headquarters(TEXT("/Game/HERO/DataV3/DA_HR_CommandHub.DA_HR_CommandHub"));
    static ConstructorHelpers::FObjectFinder<UHRUnitDefinition> Refinery(TEXT("/Game/HERO/DataV3/DA_HR_Refinery.DA_HR_Refinery"));
    static ConstructorHelpers::FObjectFinder<UHRUnitDefinition> Factory(TEXT("/Game/HERO/DataV3/DA_HR_VehiclePlant.DA_HR_VehiclePlant"));
    static ConstructorHelpers::FObjectFinder<UHRTempestFieldDefinitionAsset> ResourceField(TEXT("/Game/HERO/DataV3/DA_HR_TempestField.DA_HR_TempestField"));
    RoleDefinitions = {Striker.Object, Harvester.Object, Headquarters.Object, Refinery.Object, Factory.Object};
    FieldDefinition = ResourceField.Object;
    RoleDefinitions.SetNum(static_cast<int32>(EPrototypeRole::Count));
    EmberDefinitions.SetNum(static_cast<int32>(EPrototypeRole::Count));
    const TCHAR* Names[] = {TEXT("Combat"),TEXT("Collector"),TEXT("HQ"),TEXT("Depot"),TEXT("Factory"),TEXT("PowerPlant"),TEXT("Barracks"),TEXT("Radar"),TEXT("Defense"),TEXT("Infantry"),TEXT("Siege")};
    const int32 Costs[] = {100,150,0,200,350,220,180,250,200,80,220};
    const float Durations[] = {5,6,1,8,12,9,7,9,8,4,8};
    const float Healths[] = {100,100,800,400,600,450,450,350,350,70,160};
    const FIntPoint Cells[] = {{1,1},{1,1},{6,6},{4,4},{6,6},{3,3},{3,3},{3,3},{2,2},{1,1},{1,1}};
    const int32 Supply[] = {0,0,60,0,0,150,0,0,0,0,0};
    const int32 Demand[] = {0,0,0,30,80,0,40,60,50,0,0};
    for (int32 Faction=0;Faction<2;++Faction)
    for (int32 Index=0;Index<static_cast<int32>(EPrototypeRole::Count);++Index)
    {
        const FString Package=FString::Printf(TEXT("/Game/HERO/DataV4/D_%s_%s"),Faction==0?TEXT("Aegis"):TEXT("Ember"),Names[Index]);
        UHRUnitDefinition* Authored=FPackageName::DoesPackageExist(Package) ? LoadObject<UHRUnitDefinition>(nullptr,*(Package+TEXT(".")+FPaths::GetBaseFilename(Package))) : nullptr;
        auto& Definitions=Faction==0?RoleDefinitions:EmberDefinitions;
        if (Authored) { Definitions[Index]=Authored;continue; }
        if (Faction==0 && Definitions[Index]) continue;
        const bool bBuilding=Index>=2 && Index<=8;
        const FName SubobjectName(*FString::Printf(TEXT("Fallback_%d_%s"),Faction,Names[Index]));
        UHRUnitDefinition* Def=bBuilding ? CreateDefaultSubobject<UHRBuildingDefinition>(SubobjectName) : CreateDefaultSubobject<UHRUnitDefinition>(SubobjectName);
        Def->DisplayName=FText::FromString(FString(Names[Index]));
        Def->CreditCost=Costs[Index];Def->ProductionSeconds=Durations[Index];
        Def->Stats.Health=Healths[Index];Def->Stats.MoveSpeedCm=Index==9?250.f:Index==10?180.f:220.f;
        Def->Stats.SightRangeCm=bBuilding?2200.f:2600.f;
        Def->Stats.WeaponDamage=Index==9?12.f:Index==10?45.f:Index==8?24.f:20.f;
        Def->Stats.WeaponRangeCm=Index==9?450.f:Index==10?900.f:Index==8?850.f:480.f;
        Def->Stats.AttackInterval=Index==9?.6f:Index==10?2.f:Index==8?.8f:1.f;
        Def->Stats.ArmorType=bBuilding?HRRTSTags::Armor_Structure:Index==9?HRRTSTags::Armor_Infantry:HRRTSTags::Armor_Vehicle;
        Def->Stats.DamageType=Index==10?HRRTSTags::Damage_Explosive:Faction==1?HRRTSTags::Damage_Tempest:HRRTSTags::Damage_Kinetic;
        if (auto* B=Cast<UHRBuildingDefinition>(Def)) { B->FootprintCells=Cells[Index];B->PowerSupply=Supply[Index];B->PowerDemand=Demand[Index];B->bProvidesRadar=Index==7; }
        if (Faction==1 && Index<5 && RoleDefinitions[Index]) { Def->StaticMesh=RoleDefinitions[Index]->StaticMesh;Def->Icon=RoleDefinitions[Index]->Icon; }
        if (Index>=5) Def->StaticMesh=TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(FString::Printf(TEXT("/Game/HERO/FrontlinesV4/Meshes/SM_HR_%s.SM_HR_%s"),Names[Index],Names[Index])));
        Definitions[Index]=Def;
    }
    for (const TCHAR* FactionName : {TEXT("Aegis"),TEXT("Ember")})
    {
        const FString Package=FString::Printf(TEXT("/Game/HERO/DataV4/D_Faction_%s"),FactionName);
        FactionDefinitions.Add(FPackageName::DoesPackageExist(Package) ? LoadObject<UHRFactionDefinition>(nullptr,*(Package+TEXT(".")+FPaths::GetBaseFilename(Package))) : nullptr);
    }
    if (FPackageName::DoesPackageExist(TEXT("/Game/HERO/DataV4/D_DamageMatrix"))) DamageMatrix=LoadObject<UHRDamageMatrix>(nullptr,TEXT("/Game/HERO/DataV4/D_DamageMatrix.D_DamageMatrix"));
}

int32 APrototypeGameMode::GetFactionIndex(int32 Team)
{
    return (Team==1 ? 1:0) ^ (FParse::Param(FCommandLine::Get(),TEXT("FactionEmber"))?1:0);
}

const UHRUnitDefinition* APrototypeGameMode::GetRoleDefinition(EPrototypeRole Role, int32 Team)
{
    const APrototypeGameMode* Defaults = GetDefault<APrototypeGameMode>();
    const int32 Index = static_cast<int32>(Role);
    const auto& Definitions=GetFactionIndex(Team)==0?Defaults->RoleDefinitions:Defaults->EmberDefinitions;
    return Definitions.IsValidIndex(Index) ? Definitions[Index].Get() : nullptr;
}

const UHRFactionDefinition* APrototypeGameMode::GetFactionDefinition(int32 Team)
{
    const auto& Definitions=GetDefault<APrototypeGameMode>()->FactionDefinitions;
    return Definitions.IsValidIndex(GetFactionIndex(Team)) ? Definitions[GetFactionIndex(Team)].Get() : nullptr;
}

const UHRDamageMatrix* APrototypeGameMode::GetDamageMatrix() const { return DamageMatrix.Get(); }

const UHRTempestFieldDefinitionAsset* APrototypeGameMode::GetFieldDefinition()
{
    return GetDefault<APrototypeGameMode>()->FieldDefinition.Get();
}

APrototypeUnit* APrototypeGameMode::SpawnUnit(int32 Team, FVector Position, bool bHQ)
{
    FActorSpawnParameters Parameters;
    Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Unit = GetWorld()->SpawnActor<APrototypeUnit>(Position, FRotator(0, Team == 0 ? 0 : 180, 0), Parameters);
    if (Unit) Unit->Configure(Team, bHQ);
    return Unit;
}

void APrototypeGameMode::ResetMatch()
{
    if (auto* Controller = Cast<APrototypeController>(GetWorld()->GetFirstPlayerController()))
    {
        CancelPrototypeDoctrineTarget(Cast<APrototypeHUD>(Controller->GetHUD()));
        Controller->SetPause(false);
        Controller->ClearSelection();
        if (Controller->GetPawn()) Controller->GetPawn()->SetActorLocation(FVector(BattlefieldLayout::PlayerHQX,-1800.f,3300.f));
        Controller->PanToWorld(FVector(BattlefieldLayout::PlayerHQX,0,35));
    }
    TArray<APrototypeUnit*> Old;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) Old.Add(*It);
    for (auto* Unit : Old) Unit->Destroy();
    TArray<AActor*> OldEffects;
    for(TActorIterator<AActor> It(GetWorld());It;++It) if(It->ActorHasTag(TEXT("HERO_RuntimeFX"))) OldEffects.Add(*It);
    for(AActor* Actor:OldEffects) Actor->Destroy();
    Outcome = 0;
    Wave = 0;
    MatchTime = 0;
    Feedback.Empty();
    Commander[0]=FHRCommanderState();Commander[1]=FHRCommanderState();
    Visibility.Reset(FVector2D(-12000,-9000),FVector2D(12000,9000),400.f,2);
    VisibilityTimer=0.f;
    ResetEconomy();
    RefreshPowerGrid();UpdateVisibility();
    UE_LOG(LogTemp, Display, TEXT("HEROHOUR_MATCH_RESET"));
}

void APrototypeGameMode::TrainTank()
{
    RequestProduction(EPrototypeRole::Combat);
}

void APrototypeGameMode::OnHeadquartersDestroyed(int32 Team)
{
    if (Outcome != 0) return;
    Outcome = Team == 1 ? 1 : -1;
    UE_LOG(LogTemp, Display, TEXT("HEROHOUR_MATCH_RESULT=%d"), Outcome);
}

void APrototypeGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TickEconomy(DeltaSeconds);
    if(FParse::Param(FCommandLine::Get(),TEXT("PrototypeSoakTest")))
    {
        extern void TickHRSoakTest(APrototypeGameMode*,float);
        TickHRSoakTest(this,DeltaSeconds);
    }
    if (Outcome==0 && DeltaSeconds>0.f && FMath::IsFinite(DeltaSeconds))
    {
        Commander[0].Advance(DeltaSeconds);Commander[1].Advance(DeltaSeconds);
        Visibility.Advance(DeltaSeconds);VisibilityTimer-=DeltaSeconds;
        if (VisibilityTimer<=0.f) { UpdateVisibility();VisibilityTimer=.15f; }
    }
}

void APrototypeGameMode::TestCheck(bool bCondition, const TCHAR* Label)
{
    bTestPassed &= bCondition;
    UE_LOG(LogTemp, Display, TEXT("COMBAT_TEST %s: %s"), Label, bCondition ? TEXT("PASS") : TEXT("FAIL"));
}

void APrototypeGameMode::RunCombatTest()
{
    auto* Friendly = SpawnUnit(0, FVector(-6000,0,35));
    auto* Enemy = SpawnUnit(1, FVector(-5700,0,35));
    UpdateVisibility();
    Friendly->OrderAttack(Enemy);
    FTimerHandle TestTimer;
    GetWorldTimerManager().SetTimer(TestTimer, [this, Friendly, Enemy]()
    {
        TestCheck(!IsValid(Enemy) || Enemy->Health < 100, TEXT("real ranged attack deals damage"));
        TestCheck(!IsValid(Friendly) || Friendly->Health < 100, TEXT("enemy acquires and attacks player"));
        // Isolated tactical fixtures use real unit ticks without waiting for a wave or random production.
        if (IsValid(Friendly)) Friendly->Destroy();
        if (IsValid(Enemy)) Enemy->Destroy();
        // Remove the starting armies too: a second nearby defender must legitimately
        // interrupt the HQ order, so it cannot remain in this isolated return-to-HQ test.
        TArray<APrototypeUnit*> StartingArmy;
        for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            if (!It->bHQ) StartingArmy.Add(*It);
        for (APrototypeUnit* Unit : StartingArmy) Unit->Destroy();
        auto* Retreating = SpawnUnit(0, FVector(-6000,450,35));
        auto* NearbyEnemy = SpawnUnit(1, FVector(-5700,450,35));
        if (Retreating && NearbyEnemy)
        {
            Retreating->SetActorTickEnabled(false);
            NearbyEnemy->SetActorTickEnabled(false);
            Retreating->OrderMove(FVector(-6500,450,35));
            for (int32 Step = 0; Step < 3; ++Step) Retreating->Tick(0.35f);
            TestCheck(Retreating->bMoveOrdered && !IsValid(Retreating->AttackTarget)
                && Retreating->GetActorLocation().X < -6150.f, TEXT("retreat remains protected across hostile acquisition scans"));
            Retreating->OrderMove(PlayerHQ->GetActorLocation());
            TestCheck(FVector::DistSquared2D(Retreating->Destination, PlayerHQ->GetActorLocation()) >= FMath::Square(Footprint(PlayerHQ->Role)+89.f)
                && BattlefieldLayout::InBounds(Retreating->Destination), TEXT("move destination avoids HQ footprint inside road corridor"));
            Retreating->Destroy();
            NearbyEnemy->Destroy();
        }
        else TestCheck(false, TEXT("retreat fixtures spawn"));
        auto* Attacker = SpawnUnit(1, FVector(-6100,-450,35));
        auto* Defender = SpawnUnit(0, FVector(-5800,-450,35));
        UpdateVisibility();
        if (Attacker && Defender)
        {
            Attacker->SetActorTickEnabled(false);
            Defender->SetActorTickEnabled(false);
            Attacker->OrderAttack(PlayerHQ);
            Attacker->Tick(0.4f);
            TestCheck(Attacker->AttackTarget == Defender && Attacker->StrategicTarget == PlayerHQ
                && Defender->Health < Defender->MaxHealth, TEXT("HQ attacker engages nearby defender with real damage"));
            Defender->Destroy();
            Attacker->Tick(0.4f);
            TestCheck(Attacker->AttackTarget == PlayerHQ && Attacker->StrategicTarget == PlayerHQ,
                TEXT("HQ attacker resumes strategic objective after defender destruction"));
            Attacker->Destroy();
        }
        else TestCheck(false, TEXT("strategic attack fixtures spawn"));
        // Combat fixtures provide a completed factory; the live player must build their own.
        auto* TestFactory = SpawnUnit(0, FVector(-7400,-1600,35));
        if (TestFactory) TestFactory->ConfigureRole(EPrototypeRole::Factory);
        TestCheck(TestFactory && TestFactory->IsCompleted(), TEXT("combat production fixture factory ready"));
        FCollisionQueryParams SelectionQuery;
        SelectionQuery.bTraceComplex = true;
        FHitResult SelectionHit;
        const FVector HQPosition = PlayerHQ->GetActorLocation();
        TestCheck(GetWorld()->LineTraceSingleByChannel(SelectionHit, HQPosition+FVector(0,0,1500),
            HQPosition-FVector(0,0,200), ECC_Visibility, SelectionQuery) && SelectionHit.GetActor() == PlayerHQ,
            TEXT("concept headquarters mesh responds to selection ray"));
        const FVector FactoryPosition = TestFactory ? TestFactory->GetActorLocation() : FVector::ZeroVector;
        TestCheck(TestFactory && GetWorld()->LineTraceSingleByChannel(SelectionHit, FactoryPosition+FVector(0,0,1000),
            FactoryPosition-FVector(0,0,200), ECC_Visibility, SelectionQuery) && SelectionHit.GetActor() == TestFactory,
            TEXT("concept factory mesh responds to selection ray"));
        Credits = 99;
        TrainTank();
        TestCheck(Credits == 99, TEXT("insufficient credits rejected"));
        Credits = 100;
        TrainTank();
        TestCheck(Credits == 0 && GetQueueCount(TestFactory) == 1, TEXT("reinforcement queue charges 100"));
        EnemyHQ->ReceiveHit(800);
        TestCheck(Outcome == 1, TEXT("enemy HQ destruction wins"));
        float Before = PlayerHQ->Health;
        PlayerHQ->ReceiveHit(100);
        TestCheck(PlayerHQ->Health == Before, TEXT("combat stops after match"));
        auto* TestController = Cast<APrototypeController>(GetWorld()->GetFirstPlayerController());
        if (TestController)
        {
            TestController->Selected.Add(PlayerHQ);
            PlayerHQ->Select(true);
            TestController->bDragging = true;
        }
        ResetMatch();
        TestCheck(TestController && TestController->Selected.IsEmpty() && !TestController->bDragging,
            TEXT("restart clears selection and drag state"));
        TestCheck(Outcome == 0 && Credits == 800 && Wave == 0 && MatchTime == 0, TEXT("restart resets match state"));
        int32 Count = 0;
        for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) ++Count;
        TestCheck(Count == 14, TEXT("restart restores equal starting rosters without a free enemy factory"));
        PlayerHQ->ReceiveHit(800);
        TestCheck(Outcome == -1, TEXT("player HQ destruction loses"));
        UE_LOG(LogTemp, Display, TEXT("COMBAT_TEST_RESULT=%s"), bTestPassed ? TEXT("PASS") : TEXT("FAIL"));
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
    }, 2.5f, false);
}

void APrototypeGameMode::RunMatchTest()
{
    extern void RunPrototypeTeamBot(APrototypeGameMode*,int32);
    bool AllPassed = true;
    int32 Wins = 0;
    for (int32 Round = 0; Round < 3; ++Round)
    {
        ResetMatch();
        const bool Built = BeginBuilding(EPrototypeRole::Factory,FVector(-7400,-1600,35));
        bool PathsSafe = Built;
        double Delivered = 0.;
        float PlayerDecision = 0.f;
        int32 FocusOrders = 0;
        int32 Steps = 0;
        for (; Steps < 6000 && Outcome == 0; ++Steps)
        {
            TickEconomy(.1f); // Includes the enemy bot exactly once.
            Commander[0].Advance(.1f); Commander[1].Advance(.1f);
            Visibility.Advance(.1f); UpdateVisibility();
            PlayerDecision -= .1f;
            if (PlayerDecision <= 0.f)
            {
                PlayerDecision = .75f;
                RunPrototypeTeamBot(this,0);
                // A player can coordinate focus fire against a visible nearby
                // threat. Use public attack orders, without privileged damage,
                // resources, map knowledge or changes to the opposing bot.
                TArray<APrototypeUnit*> Army;
                for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
                    if (!It->IsActorBeingDestroyed() && It->Health>0.f && It->Team==0
                        && It->IsCombatant() && !It->IsStructure()) Army.Add(*It);
                APrototypeUnit* Focus=nullptr;
                float BestScore=TNumericLimits<float>::Max();
                for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
                {
                    if (It->IsActorBeingDestroyed() || It->Health<=0.f || It->Team==0
                        || !It->IsCombatant() || !CanTeamSee(0,*It)) continue;
                    int32 SupportingUnits=0;
                    for (const APrototypeUnit* Unit:Army)
                        SupportingUnits += FVector::DistSquared2D(Unit->GetActorLocation(),It->GetActorLocation())
                            <= FMath::Square(Unit->GetEffectiveRange()+100.f);
                    if (SupportingUnits==0) continue;
                    const float Score=It->Health/SupportingUnits;
                    if (Score<BestScore) { BestScore=Score; Focus=*It; }
                }
                if (Focus)
                    for (APrototypeUnit* Unit:Army)
                        if (FVector::DistSquared2D(Unit->GetActorLocation(),Focus->GetActorLocation())
                            <= FMath::Square(Unit->GetEffectiveRange()+100.f))
                        {
                            Unit->OrderAttack(Focus); ++FocusOrders;
                            if(Unit->Role==EPrototypeRole::Infantry) Unit->TryActivateAbility(EHRAbility::Grenade);
                        }
            }
            TArray<APrototypeUnit*> Snapshot;
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            {
                if (It->IsActorBeingDestroyed() || It->Health <= 0.f) continue;
                It->SetActorTickEnabled(false);
                Snapshot.Add(*It);
            }
            for (APrototypeUnit* Unit : Snapshot)
            {
                if (!IsValid(Unit) || Unit->IsActorBeingDestroyed() || Unit->Health <= 0.f || Outcome != 0) continue;
                const FVector Previous = Unit->GetActorLocation();
                const double BeforeCredits = Credits;
                Unit->Tick(.1f);
                if (Unit->Team == 0 && Unit->Role == EPrototypeRole::Collector) Delivered += FMath::Max(0.,Credits-BeforeCredits);
                if (!Unit->IsStructure() && FVector::DistSquared2D(Previous,Unit->GetActorLocation()) > 0.01f)
                    PathsSafe &= FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Previous,Unit->GetActorLocation());
            }
        }
        if (Outcome==0)
        {
            UE_LOG(LogTemp,Display,TEXT("MATCH_STALL round=%d time=%.1f wallet0=%.3f wallet1=%.3f queue=%d power0=%d/%d power1=%d/%d"),
                Round+1,MatchTime,Credits,EnemyCredits,ProductionQueue.Num(),PowerGrid[0].CurrentPower,PowerGrid[0].RequiredPower,
                PowerGrid[1].CurrentPower,PowerGrid[1].RequiredPower);
            const auto Obstacles=FPrototypeNavigation::GetWorldObstacles(GetWorld());
            for(TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
            {
                if(It->IsActorBeingDestroyed() || It->Health<=0.f) continue;
                UE_LOG(LogTemp,Display,TEXT("MATCH_STALL_UNIT name=%s team=%d role=%d hp=%.3f position=%s destination=%s attack=%s strategic=%s deployed=%d ordered=%d walkable=%d goalwalkable=%d cargo=%.3f collector=%d"),
                    *It->GetName(),It->Team,static_cast<int32>(It->Role),It->Health,*It->GetActorLocation().ToString(),*It->Destination.ToString(),
                    *GetNameSafe(It->AttackTarget),*GetNameSafe(It->StrategicTarget),It->CombatState.bDeployed,It->bMoveOrdered,
                    FPrototypeNavigation::IsWalkable(It->GetActorLocation(),Obstacles),FPrototypeNavigation::IsWalkable(It->Destination,Obstacles),
                    It->Cargo,static_cast<int32>(It->CollectorState));
            }
            for(const FPrototypeProduction& Item:ProductionQueue)
                UE_LOG(LogTemp,Display,TEXT("MATCH_STALL_QUEUE producer=%s role=%d"),*GetNameSafe(Item.Producer.Get()),static_cast<int32>(Item.Role));
        }
        Wins += Outcome == 1 ? 1 : 0;
        const bool RoundPassed = Built && PathsSafe && Delivered > 800. && Outcome != 0;
        AllPassed &= RoundPassed;
        UE_LOG(LogTemp,Display,TEXT("MATCH_TEST round=%d outcome=%d simulation=%.1fs delivered=%.0f paths=%s focus_orders=%d result=%s"),
            Round+1,Outcome,MatchTime,Delivered,PathsSafe ? TEXT("SAFE") : TEXT("UNSAFE"),FocusOrders,RoundPassed ? TEXT("PASS") : TEXT("FAIL"));
    }
    AllPassed &= Wins >= 1;
    UE_LOG(LogTemp,Display,TEXT("MATCH_TEST_RESULT=%s wins=%d/3"),AllPassed ? TEXT("PASS") : TEXT("FAIL"),Wins);
    ResetMatch();
    if (auto* Controller = GetWorld()->GetFirstPlayerController()) Controller->ConsoleCommand(TEXT("quit"));
}

void APrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeArchitectureTest")))
    {
        RunHRRTSArchitectureSelfTests();
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
        return;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeResourceTest")))
    {
        RunHRResourceFieldSelfTests(GetWorld());
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
        return;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeDockingTest")))
    {
        RunHRRefineryDockingSelfTests(GetWorld());
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
        return;
    }
    ResetMatch();
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeLogisticsStressTest")))
    {
        extern bool RunHRLogisticsStressTests(UWorld*);
        RunHRLogisticsStressTests(GetWorld());GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));return;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeVisibilityTest")))
    {
        RunHRVisibilitySelfTests();GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));return;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypePresentationTest")))
    {
        extern bool RunHRPresentationSelfTests(UWorld*);
        RunHRPresentationSelfTests(GetWorld());GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));return;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeFullMatchTest")))
    {
        extern bool RunHRFullMatchTests(UWorld*);
        RunHRFullMatchTests(GetWorld());GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));return;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeTacticalTest")))
    {
        RunHRCombatSelfTests(GetWorld());GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));return;
    }
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeSystemsTest")))
    {
        extern bool RunHRSystemsSelfTests(UWorld*);
        RunHRSystemsSelfTests(GetWorld());GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));return;
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeCombatTest"))) RunCombatTest();
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeUITest"))) RunUITest();
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeEconomyTest"))) RunEconomyTest();
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeMatchTest"))) RunMatchTest();
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeNavigationTest")))
    {
        bool Pass=FPrototypeNavigation::RunSelfTests();
        APrototypeUnit* Traveler = SpawnUnit(0, FVector(-8000,-2500,35));
        const FVector DestinationPoint(-1000,-2500,35);
        bool Safe = Traveler != nullptr;
        int32 Steps = 0;
        if (Traveler)
        {
            Traveler->SetActorTickEnabled(false);
            Traveler->OrderMove(DestinationPoint);
            FVector Previous = Traveler->GetActorLocation();
            for (; Steps < 5000 && FVector::DistSquared2D(Traveler->GetActorLocation(),DestinationPoint)>FMath::Square(10.f); ++Steps)
            {
                Traveler->Tick(.05f);
                Safe &= FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Previous,Traveler->GetActorLocation());
                Previous = Traveler->GetActorLocation();
            }
            Safe &= Steps < 5000 && FVector::DistSquared2D(Traveler->GetActorLocation(),DestinationPoint)<=FMath::Square(10.f);
            UE_LOG(LogTemp,Display,TEXT("NAVIGATION_TEST actual unit reaches opposite bank without crossing water: %s (steps=%d, location=%s)"),
                Safe ? TEXT("PASS") : TEXT("FAIL"),Steps,*Traveler->GetActorLocation().ToString());
            Traveler->Destroy();
        }
        Pass &= Safe;
        UE_LOG(LogTemp,Display,TEXT("NAVIGATION_TEST_RESULT=%s"),Pass ? TEXT("PASS") : TEXT("FAIL"));
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeCapture")) || FParse::Param(FCommandLine::Get(),TEXT("PrototypeOverviewCapture")) || FParse::Param(FCommandLine::Get(),TEXT("PrototypeDistrictCapture")) || FParse::Param(FCommandLine::Get(),TEXT("PrototypeHarvesterCapture")))
    {
        FTimerHandle CaptureTimer;
        GetWorldTimerManager().SetTimer(CaptureTimer, [this]()
        {
            if (auto* Controller = Cast<APrototypeController>(GetWorld()->GetFirstPlayerController()))
            {
                Controller->ClearSelection();
                if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeHarvesterCapture")))
                {
                    for (TActorIterator<APrototypeUnit> It(GetWorld());It;++It)
                        if (It->Team==0 && It->Role==EPrototypeRole::Collector && It->Health>0.f)
                        {
                            Controller->Selected.Add(*It);It->Select(true);
                            if (APawn* Camera=Controller->GetPawn())
                            { FVector Position=Camera->GetActorLocation();Position.Z=1800.f;Camera->SetActorLocation(Position); }
                            Controller->PanToWorld(It->GetActorLocation());break;
                        }
                }
                else if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeOverviewCapture")) || FParse::Param(FCommandLine::Get(),TEXT("PrototypeDistrictCapture")))
                {
                    if (APawn* Camera = Controller->GetPawn())
                    {
                        const bool bDistrict=FParse::Param(FCommandLine::Get(),TEXT("PrototypeDistrictCapture"));
                        FVector Position = Camera->GetActorLocation(); Position.Z = bDistrict ? 8000.f : 16000.f;
                        Camera->SetActorLocation(Position); Controller->PanToWorld(bDistrict ? FVector(-2300,-2500,35) : FVector::ZeroVector);
                    }
                }
                else if (PlayerHQ) { Controller->Selected.Add(PlayerHQ); PlayerHQ->Select(true); }
            }
            const TCHAR* Name=FParse::Param(FCommandLine::Get(),TEXT("PrototypeHarvesterCapture")) ? TEXT("Screenshots/Harvester.png") : FParse::Param(FCommandLine::Get(),TEXT("PrototypeDistrictCapture")) ? TEXT("Screenshots/District.png") : FParse::Param(FCommandLine::Get(),TEXT("PrototypeOverviewCapture")) ? TEXT("Screenshots/EnvironmentOverview.png") : TEXT("Screenshots/Battle.png");
            // Give streaming and occlusion visibility time to settle after moving the camera.
            FTimerHandle ShutterTimer;
            GetWorldTimerManager().SetTimer(ShutterTimer,[this,Name]()
            {
                FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / Name, true, false);
                if(FParse::Param(FCommandLine::Get(),TEXT("PrototypeCaptureExit")))
                {
                    FTimerHandle ExitTimer;
                    GetWorldTimerManager().SetTimer(ExitTimer,[this]()
                    { GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit")); },2.f,false);
                }
            },.8f,false);
        }, 8.f, false);
    }
}
