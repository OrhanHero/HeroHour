#include "HHPrototypeCombatActor.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AHHPrototypeCombatActor::AHHPrototypeCombatActor()
{
    PrimaryActorTick.bCanEverTick = true;
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);
    Mesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (Cube.Succeeded())
    {
        Mesh->SetStaticMesh(Cube.Object);
    }
}

void AHHPrototypeCombatActor::Configure(const EHHPrototypeFaction InFaction, const EHHPrototypeUnitRole InRole,
    const bool bInHeadquarters)
{
    Faction = InFaction;
    Role = InRole;
    bHeadquarters = bInHeadquarters;

    FVector Scale;
    FRotator VisualRotation = FRotator::ZeroRotator;
    const TCHAR* LocalMeshPath = nullptr;
    if (bHeadquarters)
    {
        MaxHealth = 1200.0f;
        Damage = 0.0f;
        LocalMeshPath = Faction == EHHPrototypeFaction::Hero
            ? TEXT("/Game/LocalAssets/MilitaryFree/Tent_002.Tent_002")
            : TEXT("/Game/LocalAssets/MilitaryFree/Tower_003.Tower_003");
        Scale = FVector(0.34f);
        VisualRadius = 230.0f;
        VisualHeight = 210.0f;
    }
    else if (Role == EHHPrototypeUnitRole::Infantry)
    {
        MaxHealth = 100.0f;
        MoveSpeed = 260.0f;
        Damage = 10.0f;
        AttackRange = 230.0f;
        AttackInterval = 0.65f;
        Scale = FVector(0.38f, 0.38f, 0.70f);
        VisualRadius = 46.0f;
        VisualHeight = 92.0f;
    }
    else if (Role == EHHPrototypeUnitRole::Vehicle)
    {
        MaxHealth = 260.0f;
        MoveSpeed = 185.0f;
        Damage = 26.0f;
        AttackRange = 260.0f;
        AttackInterval = 1.15f;
        LocalMeshPath = TEXT("/Game/LocalAssets/RTSModern/ATV_N1_LE.ATV_N1_LE");
        Scale = FVector(0.38f);
        VisualRotation.Yaw = -90.0f;
        VisualRadius = 78.0f;
        VisualHeight = 72.0f;
    }
    else
    {
        MaxHealth = 150.0f;
        MoveSpeed = 340.0f;
        Damage = 18.0f;
        AttackRange = 330.0f;
        AttackInterval = 0.85f;
        LocalMeshPath = Faction == EHHPrototypeFaction::Hero
            ? TEXT("/Game/LocalAssets/RTSModern/MSH_N2_LE.MSH_N2_LE")
            : TEXT("/Game/LocalAssets/RTSModern/FA_N26_LE.FA_N26_LE");
        Scale = FVector(0.22f);
        VisualRotation.Yaw = -90.0f;
        VisualRadius = 92.0f;
        VisualHeight = 185.0f;
    }

    // Fab assets intentionally remain local and ignored by Git. When they are not
    // installed, the constructor's engine cube keeps the prototype fully playable.
    if (LocalMeshPath)
    {
        if (UStaticMesh* LocalMesh = LoadObject<UStaticMesh>(nullptr, LocalMeshPath))
        {
            Mesh->SetStaticMesh(LocalMesh);
            bUsingGreybox = false;
        }
    }

    Health = MaxHealth;
    SetActorScale3D(Scale);
    Mesh->SetRelativeRotation(VisualRotation);

    // Normalize imported pivots so every local Fab model visibly rests on its actor's
    // ground plane. Aircraft keep a readable tactical altitude above that plane.
    FVector BoundsMin;
    FVector BoundsMax;
    Mesh->GetLocalBounds(BoundsMin, BoundsMax);
    const float GroundOffset = -BoundsMin.Z + (Role == EHHPrototypeUnitRole::Air && !bHeadquarters ? 520.0f : 0.0f);
    Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, GroundOffset));

    UMaterialInterface* ColorBase = LoadObject<UMaterialInterface>(
        nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
    const int32 MaterialSlots = FMath::Max(1, Mesh->GetNumMaterials());
    for (int32 Slot = 0; Slot < MaterialSlots; ++Slot)
    {
        // The generic FBX package ships without a UE-native material graph. A
        // deliberate faction finish is more readable than its plain white import.
        if (ColorBase) Mesh->SetMaterial(Slot, ColorBase);
        if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(Slot))
        {
            const FLinearColor Color = Faction == EHHPrototypeFaction::Hero
                ? FLinearColor(0.02f, 0.22f, 0.8f) : FLinearColor(0.72f, 0.035f, 0.02f);
            Material->SetVectorParameterValue(TEXT("Color"), Color);
            Material->SetVectorParameterValue(TEXT("TintColor"), Color);
        }
    }
}

void AHHPrototypeCombatActor::SetMoveTarget(const FVector& InTarget)
{
    if (bHeadquarters) return;
    MoveTarget = FVector(InTarget.X, InTarget.Y, GetActorLocation().Z);
    bHasMoveTarget = true;
    AttackTarget = nullptr;
}

void AHHPrototypeCombatActor::SetAttackTarget(AHHPrototypeCombatActor* InTarget)
{
    if (!bHeadquarters && IsValid(InTarget) && InTarget->Faction != Faction)
    {
        AttackTarget = InTarget;
        bHasMoveTarget = false;
    }
}

void AHHPrototypeCombatActor::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    AttackCooldown = FMath::Max(0.0f, AttackCooldown - DeltaSeconds);
    DamageFlashTime = FMath::Max(0.0f, DamageFlashTime - DeltaSeconds);
    ShotEffectTime = FMath::Max(0.0f, ShotEffectTime - DeltaSeconds);
    if (!IsAlive()) return;

    const FColor FactionColor = Faction == EHHPrototypeFaction::Hero ? FColor::Cyan : FColor::Red;
    const FVector GroundLocation = GetActorLocation() + FVector(0.0f, 0.0f, 3.0f);

    // A small faction marker reads cleanly without hiding the actual model.
    DrawDebugCircle(GetWorld(), GroundLocation, VisualRadius, 36, FactionColor, false, 0.0f, 0,
        bHeadquarters ? 7.0f : 3.0f, FVector(1, 0, 0), FVector(0, 1, 0), false);

    // Health is shown for selected, damaged and strategic units, reducing HUD clutter.
    if (bSelected || bHeadquarters || Health < MaxHealth)
    {
        const float BarHalfWidth = bHeadquarters ? 115.0f : 55.0f;
        const FVector HealthStart = GetActorLocation() + FVector(-BarHalfWidth, 0.0f, VisualHeight + 42.0f);
        DrawDebugLine(GetWorld(), HealthStart, HealthStart + FVector(BarHalfWidth * 2.0f, 0.0f, 0.0f),
            FColor(55, 8, 8), false, 0.0f, 0, 9.0f);
        DrawDebugLine(GetWorld(), HealthStart, HealthStart + FVector(BarHalfWidth * 2.0f * (Health / MaxHealth), 0.0f, 0.0f),
            Health / MaxHealth > 0.35f ? FColor::Green : FColor::Orange, false, 0.0f, 0, 7.0f);
    }

    if (bSelected)
    {
        DrawDebugCircle(GetWorld(), GroundLocation + FVector(0, 0, 2), VisualRadius + 10.0f,
            40, FColor::Yellow, false, 0.0f, 0, 7.0f, FVector(1, 0, 0), FVector(0, 1, 0), false);
    }

    const TCHAR* RoleName = bHeadquarters ? TEXT("HQ")
        : Role == EHHPrototypeUnitRole::Infantry ? TEXT("INFANTRY")
        : Role == EHHPrototypeUnitRole::Vehicle ? TEXT("VEHICLE") : TEXT("AIR");
    if (bSelected || bHeadquarters)
    {
        DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, VisualHeight + 72.0f), RoleName,
            nullptr, FactionColor, 0.0f, true, bHeadquarters ? 1.2f : 1.0f);
    }

    if (DamageFlashTime > 0.0f)
    {
        DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0, 0, VisualHeight * 0.45f),
            VisualRadius * 0.8f, 12, FColor::White, false, 0.0f, 0, 4.0f);
    }
    if (ShotEffectTime > 0.0f)
    {
        const FColor ShotColor = Role == EHHPrototypeUnitRole::Air ? FColor::Orange : FColor::Yellow;
        DrawDebugLine(GetWorld(), LastShotStart, LastShotEnd, ShotColor, false, 0.0f, 0,
            Role == EHHPrototypeUnitRole::Vehicle ? 7.0f : 3.5f);
        DrawDebugSphere(GetWorld(), LastShotEnd, Role == EHHPrototypeUnitRole::Vehicle ? 24.0f : 13.0f,
            10, FColor::Orange, false, 0.0f, 0, 3.0f);
    }

    if (bHeadquarters) return;

    if (IsValid(AttackTarget) && AttackTarget->IsAlive())
    {
        const FVector Delta = AttackTarget->GetActorLocation() - GetActorLocation();
        if (Delta.SizeSquared2D() > FMath::Square(AttackRange))
        {
            SetActorLocation(GetActorLocation() + Delta.GetSafeNormal2D() * MoveSpeed * DeltaSeconds, true);
        }
        else if (AttackCooldown <= 0.0f)
        {
            LastShotStart = GetActorLocation() + FVector(0, 0, VisualHeight * 0.55f);
            LastShotEnd = AttackTarget->GetActorLocation() + FVector(0, 0, AttackTarget->VisualHeight * 0.4f);
            ShotEffectTime = 0.14f;
            FDamageEvent Event;
            AttackTarget->TakeDamage(Damage, Event, nullptr, this);
            AttackCooldown = AttackInterval;
        }
    }
    else if (bHasMoveTarget)
    {
        const FVector Delta = MoveTarget - GetActorLocation();
        if (Delta.SizeSquared2D() < FMath::Square(12.0f)) bHasMoveTarget = false;
        else SetActorLocation(GetActorLocation() + Delta.GetSafeNormal2D() * MoveSpeed * DeltaSeconds, true);
    }
}

float AHHPrototypeCombatActor::TakeDamage(const float DamageAmount, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    Health = FMath::Max(0.0f, Health - (Applied > 0.0f ? Applied : DamageAmount));
    DamageFlashTime = 0.12f;
    if (!bHeadquarters && !IsValid(AttackTarget))
    {
        if (AHHPrototypeCombatActor* Attacker = Cast<AHHPrototypeCombatActor>(DamageCauser);
            Attacker && Attacker->Faction != Faction)
        {
            SetAttackTarget(Attacker);
        }
    }
    if (Health <= 0.0f)
    {
        const FVector EffectLocation = GetActorLocation() + FVector(0, 0, VisualHeight * 0.4f);
        DrawDebugSphere(GetWorld(), EffectLocation, VisualRadius * 1.45f, 18, FColor::Orange, false, 0.75f, 0, 8.0f);
        DrawDebugSphere(GetWorld(), EffectLocation, VisualRadius * 0.8f, 14, FColor::Yellow, false, 0.45f, 0, 5.0f);
        for (int32 Ray = 0; Ray < 10; ++Ray)
        {
            const FVector Direction = FMath::VRand().GetSafeNormal();
            DrawDebugLine(GetWorld(), EffectLocation, EffectLocation + Direction * VisualRadius * 2.2f,
                FColor::Red, false, 0.65f, 0, 3.0f);
        }
        SetActorEnableCollision(false);
        SetActorHiddenInGame(true);
        SetLifeSpan(0.2f);
    }
    return DamageAmount;
}
