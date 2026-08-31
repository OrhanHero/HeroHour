#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HHPrototypeCombatActor.h"
#include "HHPrototypeEconomyComponent.generated.h"

USTRUCT(BlueprintType)
struct FHHPrototypeUnitOffer
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Economy")
    EHHPrototypeUnitRole Role = EHHPrototypeUnitRole::Infantry;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Economy", meta=(ClampMin="0"))
    int32 Cost = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="HeroHour|Economy", meta=(ClampMin="0.01"))
    float BuildTime = 5.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHHCreditsChanged, int32, NewCredits, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHHProductionStarted, EHHPrototypeUnitRole, Role, float, BuildTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHHProductionProgressed, float, NormalizedProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHHProductionFinished, EHHPrototypeFaction, Faction, EHHPrototypeUnitRole, Role);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHHProductionCancelled, EHHPrototypeUnitRole, Role, int32, Refund);

/** Small, deterministic prototype economy and one-slot production queue.
 *  Add one component per faction to the authoritative GameMode/GameState actor.
 */
UCLASS(ClassGroup=(HeroHour), meta=(BlueprintSpawnableComponent))
class HEROHOURPROTOTYPEUE_API UHHPrototypeEconomyComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHHPrototypeEconomyComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="HeroHour|Economy")
    void Configure(EHHPrototypeFaction InFaction, int32 InStartingCredits);

    UFUNCTION(BlueprintCallable, Category="HeroHour|Economy")
    void AddCredits(int32 Amount);

    /** Charges immediately. Returns false for an occupied queue, unknown offer or insufficient credits. */
    UFUNCTION(BlueprintCallable, Category="HeroHour|Economy")
    bool RequestProduction(EHHPrototypeUnitRole Role);

    /** Cancels the active order and refunds its full price. */
    UFUNCTION(BlueprintCallable, Category="HeroHour|Economy")
    bool CancelProduction();

    UFUNCTION(BlueprintPure, Category="HeroHour|Economy")
    int32 GetCredits() const { return Credits; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Economy")
    EHHPrototypeFaction GetFaction() const { return Faction; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Economy")
    bool IsProducing() const { return bIsProducing; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Economy")
    EHHPrototypeUnitRole GetQueuedRole() const { return QueuedRole; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Economy")
    float GetProductionProgress() const;

    UFUNCTION(BlueprintPure, Category="HeroHour|Economy")
    bool GetOffer(EHHPrototypeUnitRole Role, FHHPrototypeUnitOffer& OutOffer) const;

    UPROPERTY(BlueprintAssignable, Category="HeroHour|Economy") FHHCreditsChanged OnCreditsChanged;
    UPROPERTY(BlueprintAssignable, Category="HeroHour|Economy") FHHProductionStarted OnProductionStarted;
    UPROPERTY(BlueprintAssignable, Category="HeroHour|Economy") FHHProductionProgressed OnProductionProgressed;
    /** GameMode hook: spawn/configure the produced combat actor when this fires. */
    UPROPERTY(BlueprintAssignable, Category="HeroHour|Economy") FHHProductionFinished OnProductionFinished;
    UPROPERTY(BlueprintAssignable, Category="HeroHour|Economy") FHHProductionCancelled OnProductionCancelled;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HeroHour|Economy")
    EHHPrototypeFaction Faction = EHHPrototypeFaction::Hero;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HeroHour|Economy", meta=(ClampMin="0"))
    int32 StartingCredits = 2500;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HeroHour|Economy", meta=(ClampMin="0"))
    int32 SupplyIncome = 100;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HeroHour|Economy", meta=(ClampMin="0.1"))
    float SupplyInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="HeroHour|Economy")
    TArray<FHHPrototypeUnitOffer> UnitOffers;

private:
    void CompleteProduction();

    int32 Credits = 0;
    int32 QueuedCost = 0;
    float SupplyAccumulator = 0.0f;
    float ProductionElapsed = 0.0f;
    float ProductionDuration = 0.0f;
    float LastBroadcastProgress = -1.0f;
    EHHPrototypeUnitRole QueuedRole = EHHPrototypeUnitRole::Infantry;
    bool bIsProducing = false;
};
