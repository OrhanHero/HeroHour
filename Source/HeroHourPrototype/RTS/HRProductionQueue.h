#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "HRProductionQueue.generated.h"

class UHRUnitDefinition;

UENUM(BlueprintType)
enum class EHRPowerState : uint8 { Online, LowPower, Offline };

USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRPowerGridState
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) int32 CurrentPower=0;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) int32 RequiredPower=0;
    EHRPowerState GetState() const;
    float GetProductionRate() const;
    bool CanRunRadarAndDefense() const { return GetState()==EHRPowerState::Online; }
};

USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRProductionOrder
{
    GENERATED_BODY()
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) FGuid OrderId;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) TObjectPtr<UHRUnitDefinition> Definition=nullptr;
    // Snapshot at payment: later balance changes must not change the refund.
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) int32 PaidCredits=0;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) float DurationSeconds=1.f;
    UPROPERTY(VisibleAnywhere,BlueprintReadOnly) float WorkSeconds=0.f;
};

// One FIFO per team + production category. A live team component owns these
// values through UPROPERTY; UI only reads the state and requests commands.
// This tested migration example is not wired to the legacy match queue yet.
USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRProductionLane
{
    GENERATED_BODY()
    bool Initialize(FGameplayTag InFaction,FGameplayTag InType,int32 InCapacity=5);
    bool TryEnqueue(UHRUnitDefinition* Definition,const FGameplayTagContainer& CompletedTech,
        int64& TeamCredits,FGuid& OutOrderId);
    bool Cancel(FGuid OrderId,int64& TeamCredits);
    void Advance(float DeltaSeconds,float ProductionRate);
    const FHRProductionOrder* PeekReady() const;
    // Call only after an authoritative successful spawn/place transaction.
    // A blocked exit or failed spawn leaves the paid ready order in the lane.
    bool CommitSpawn(FGuid ReadyOrderId);
    int32 Num() const { return Orders.Num(); }
    const TArray<FHRProductionOrder>& GetOrders() const { return Orders; }

private:
    UPROPERTY() FGameplayTag Faction;
    UPROPERTY() FGameplayTag Type;
    UPROPERTY() int32 Capacity=5;
    UPROPERTY() TArray<FHRProductionOrder> Orders;
};

HEROHOURPROTOTYPE_API bool RunHRRTSArchitectureSelfTests();
