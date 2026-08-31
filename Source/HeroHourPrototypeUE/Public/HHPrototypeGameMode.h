#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HHPrototypeCombatActor.h"
#include "HHPrototypeGameMode.generated.h"

class AHHPrototypeFogOfWarActor;
class UHHPrototypeEconomyComponent;

UENUM(BlueprintType)
enum class EHHPrototypeMatchResult : uint8
{
    InProgress,
    HeroVictory,
    USAVictory
};

/** Runs the HERO-vs-USA vertical slice on an authored city map, with a generated fallback arena. */
UCLASS()
class HEROHOURPROTOTYPEUE_API AHHPrototypeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AHHPrototypeGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    EHHPrototypeMatchResult GetMatchResult() const { return MatchResult; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    bool HasUSAStarted() const { return bUSAStarted; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    float GetUSAOrderTimeRemaining() const { return FMath::Max(0.0f, USAOrderTimer); }

    UFUNCTION(BlueprintCallable, Category="HeroHour|Prototype")
    bool RequestHeroProduction(EHHPrototypeUnitRole UnitRole);

    UFUNCTION(BlueprintCallable, Category="HeroHour|Prototype")
    bool CancelHeroProduction();

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    UHHPrototypeEconomyComponent* GetHeroEconomy() const { return HeroEconomy; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    UHHPrototypeEconomyComponent* GetUSAEconomy() const { return USAEconomy; }

    UFUNCTION(BlueprintPure, Category="HeroHour|Prototype")
    AHHPrototypeFogOfWarActor* GetFogOfWar() const { return FogOfWar; }

private:
    void BuildArena();
    AHHPrototypeCombatActor* SpawnCombatant(EHHPrototypeFaction Faction, EHHPrototypeUnitRole InRole, bool bHQ,
        const FVector& Location);
    void IssueUSAOrder();
    void FinishMatch(EHHPrototypeMatchResult Result);

    UFUNCTION()
    void HandleProductionFinished(EHHPrototypeFaction Faction, EHHPrototypeUnitRole UnitRole);

    UPROPERTY()
    TObjectPtr<AHHPrototypeCombatActor> HeroHQ;
    UPROPERTY()
    TObjectPtr<AHHPrototypeCombatActor> USAHQ;
    UPROPERTY()
    TArray<TObjectPtr<AHHPrototypeCombatActor>> HeroUnits;
    UPROPERTY()
    TArray<TObjectPtr<AHHPrototypeCombatActor>> USAUnits;

    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UHHPrototypeEconomyComponent> HeroEconomy;
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<UHHPrototypeEconomyComponent> USAEconomy;
    UPROPERTY()
    TObjectPtr<AHHPrototypeFogOfWarActor> FogOfWar;

    EHHPrototypeMatchResult MatchResult = EHHPrototypeMatchResult::InProgress;
    float USAOrderTimer = 30.0f;
    float USAProductionTimer = 2.0f;
    int32 HeroProductionSerial = 0;
    int32 USAProductionSerial = 0;
    int32 USAProductionChoice = 0;
    bool bUSAStarted = false;
};
