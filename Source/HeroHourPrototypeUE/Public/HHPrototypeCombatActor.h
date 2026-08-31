#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HHPrototypeCombatActor.generated.h"

class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EHHPrototypeFaction : uint8
{
    Hero,
    USA
};

UENUM(BlueprintType)
enum class EHHPrototypeUnitRole : uint8
{
    Infantry,
    Vehicle,
    Air
};

/** Small, asset-free combatant used only by the vertical slice. */
UCLASS()
class HEROHOURPROTOTYPEUE_API AHHPrototypeCombatActor : public AActor
{
    GENERATED_BODY()

public:
    AHHPrototypeCombatActor();
    virtual void Tick(float DeltaSeconds) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    void Configure(EHHPrototypeFaction InFaction, EHHPrototypeUnitRole InRole, bool bInHeadquarters);

    UFUNCTION(BlueprintCallable, Category="HeroHour|Prototype")
    void SetMoveTarget(const FVector& InTarget);

    UFUNCTION(BlueprintCallable, Category="HeroHour|Prototype")
    void SetAttackTarget(AHHPrototypeCombatActor* InTarget);

    UFUNCTION(BlueprintCallable, Category="HeroHour|Prototype")
    void SetSelected(bool bInSelected) { bSelected = bInSelected; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    bool IsAlive() const { return Health > 0.0f; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    bool IsHeadquarters() const { return bHeadquarters; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    EHHPrototypeFaction GetFaction() const { return Faction; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    EHHPrototypeUnitRole GetRole() const { return Role; }

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UStaticMeshComponent> Mesh;

    UPROPERTY()
    TObjectPtr<AHHPrototypeCombatActor> AttackTarget;

    EHHPrototypeFaction Faction = EHHPrototypeFaction::Hero;
    EHHPrototypeUnitRole Role = EHHPrototypeUnitRole::Infantry;
    FVector MoveTarget = FVector::ZeroVector;
    float Health = 100.0f;
    float MaxHealth = 100.0f;
    float MoveSpeed = 220.0f;
    float Damage = 12.0f;
    float AttackRange = 180.0f;
    float AttackInterval = 0.8f;
    float AttackCooldown = 0.0f;
    bool bHasMoveTarget = false;
    bool bHeadquarters = false;
    bool bSelected = false;
};
