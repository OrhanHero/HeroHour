#include "HHPrototypeCombatActor.h"

#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
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
    if (bHeadquarters)
    {
        MaxHealth = 1200.0f;
        Damage = 0.0f;
        Scale = FVector(2.8f, 2.8f, 1.8f);
    }
    else if (Role == EHHPrototypeUnitRole::Infantry)
    {
        MaxHealth = 100.0f;
        MoveSpeed = 260.0f;
        Damage = 10.0f;
        AttackRange = 230.0f;
        AttackInterval = 0.65f;
        Scale = FVector(0.38f, 0.38f, 0.70f);
    }
    else if (Role == EHHPrototypeUnitRole::Vehicle)
    {
        MaxHealth = 260.0f;
        MoveSpeed = 185.0f;
        Damage = 26.0f;
        AttackRange = 260.0f;
        AttackInterval = 1.15f;
        Scale = FVector(0.85f, 0.60f, 0.48f);
    }
    else
    {
        MaxHealth = 150.0f;
        MoveSpeed = 340.0f;
        Damage = 18.0f;
        AttackRange = 330.0f;
        AttackInterval = 0.85f;
        Scale = FVector(0.75f, 0.48f, 0.25f);
    }

    Health = MaxHealth;
    SetActorScale3D(Scale);

    if (UMaterialInstanceDynamic* Material = Mesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        const FLinearColor Color = Faction == EHHPrototypeFaction::Hero
            ? FLinearColor(0.05f, 0.3f, 1.0f) : FLinearColor(0.85f, 0.08f, 0.04f);
        Material->SetVectorParameterValue(TEXT("Color"), Color);
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
    if (!IsAlive()) return;

    const FColor FactionColor = Faction == EHHPrototypeFaction::Hero ? FColor::Cyan : FColor::Red;
    const FVector Extent = bHeadquarters ? FVector(280.0f, 280.0f, 180.0f) : FVector(55.0f, 55.0f, 75.0f);
    DrawDebugBox(GetWorld(), GetActorLocation(), Extent, FactionColor, false, 0.0f, 0, bHeadquarters ? 10.0f : 5.0f);

    const FVector HealthStart = GetActorLocation() + FVector(-Extent.X, 0.0f, Extent.Z + 45.0f);
    DrawDebugLine(GetWorld(), HealthStart, HealthStart + FVector(Extent.X * 2.0f, 0.0f, 0.0f), FColor::Red, false, 0.0f, 0, 8.0f);
    DrawDebugLine(GetWorld(), HealthStart, HealthStart + FVector(Extent.X * 2.0f * (Health / MaxHealth), 0.0f, 0.0f), FColor::Green, false, 0.0f, 0, 8.0f);

    if (bSelected)
    {
        DrawDebugCircle(GetWorld(), GetActorLocation() - FVector(0.0f, 0.0f, Extent.Z), Extent.X * 1.25f,
            32, FColor::Yellow, false, 0.0f, 0, 8.0f, FVector(1, 0, 0), FVector(0, 1, 0), false);
    }

    const TCHAR* RoleName = bHeadquarters ? TEXT("HQ")
        : Role == EHHPrototypeUnitRole::Infantry ? TEXT("INFANTRY")
        : Role == EHHPrototypeUnitRole::Vehicle ? TEXT("VEHICLE") : TEXT("AIR");
    DrawDebugString(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, Extent.Z + 85.0f), RoleName,
        nullptr, FactionColor, 0.0f, true, 1.05f);

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
        SetActorEnableCollision(false);
        SetActorHiddenInGame(true);
        SetLifeSpan(0.2f);
    }
    return DamageAmount;
}
