#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HHPrototypeCombatActor.h"
#include "HHPrototypeGameMode.generated.h"

UENUM(BlueprintType)
enum class EHHPrototypeMatchResult : uint8
{
    InProgress,
    HeroVictory,
    USAVictory
};

/** Creates the complete greybox match at runtime; no Content assets are required. */
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

private:
    void BuildArena();
    AHHPrototypeCombatActor* SpawnCombatant(EHHPrototypeFaction Faction, EHHPrototypeUnitRole InRole, bool bHQ,
        const FVector& Location);
    void IssueUSAOrder();
    void FinishMatch(EHHPrototypeMatchResult Result);

    UPROPERTY()
    TObjectPtr<AHHPrototypeCombatActor> HeroHQ;
    UPROPERTY()
    TObjectPtr<AHHPrototypeCombatActor> USAHQ;
    UPROPERTY()
    TArray<TObjectPtr<AHHPrototypeCombatActor>> HeroUnits;
    UPROPERTY()
    TArray<TObjectPtr<AHHPrototypeCombatActor>> USAUnits;

    EHHPrototypeMatchResult MatchResult = EHHPrototypeMatchResult::InProgress;
    float USAOrderTimer = 12.0f;
    bool bUSAStarted = false;
};
