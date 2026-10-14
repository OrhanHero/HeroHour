#pragma once

#include "CoreMinimal.h"

class AActor;
class UWorld;

// HeroHour policy: FIFO, one dock by default, 40 credits/second. Not measured original balancing.
class HEROHOURPROTOTYPE_API FHRRefineryDocking
{
public:
    bool Reset(AActor* Refinery, int32 Slots = 1, double UnloadRate = 40.0);
    bool Submit(AActor* Collector);
    bool IsQueued(AActor* Collector) const;
    bool IsDocked(AActor* Collector) const;
    AActor* GetNextWaiting() const;
    bool TryAcquireNext(AActor* Collector, bool bAtReachableDockPosition);
    void Release(AActor* Collector);
    void ReleaseInvalid(TFunctionRef<bool(AActor*)> IsEligible);
    // Positive, monotonically increasing operation IDs prevent replay across release/requeue.
    double Unload(AActor* Collector, double DeltaSeconds, double& Cargo, double& Wallet, uint64 OperationId);
    int32 NumQueued() const { return Waiting.Num(); }
    int32 NumDocked() const { return Docked.Num(); }

private:
    bool HasValidRefinery() const;
    TWeakObjectPtr<AActor> Owner;
    TArray<TWeakObjectPtr<AActor>> Waiting;
    TArray<TWeakObjectPtr<AActor>> Docked;
    TMap<TWeakObjectPtr<AActor>, uint64> LastCommittedOperation;
    int32 SlotCount = 1;
    double Rate = 40.0;
};

HEROHOURPROTOTYPE_API bool RunHRRefineryDockingSelfTests(UWorld* TestWorld);
