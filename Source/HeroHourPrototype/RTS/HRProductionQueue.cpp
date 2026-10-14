#include "HRProductionQueue.h"
#include "HRRTSDefinitions.h"

EHRPowerState FHRPowerGridState::GetState() const
{
    const int32 Supply=FMath::Max(0,CurrentPower),Demand=FMath::Max(0,RequiredPower);
    if (Demand>0 && Supply==0) return EHRPowerState::Offline;
    return Supply<Demand ? EHRPowerState::LowPower : EHRPowerState::Online;
}

float FHRPowerGridState::GetProductionRate() const
{
    switch (GetState())
    {
    case EHRPowerState::Online: return 1.f;
    case EHRPowerState::LowPower: return .5f;
    default: return 0.f;
    }
}

bool FHRProductionLane::Initialize(FGameplayTag InFaction,FGameplayTag InType,int32 InCapacity)
{
    if (!Orders.IsEmpty() || !InFaction.IsValid() || !InType.IsValid() || InCapacity<=0) return false;
    Faction=InFaction;Type=InType;Capacity=InCapacity;return true;
}

bool FHRProductionLane::TryEnqueue(UHRUnitDefinition* Definition,const FGameplayTagContainer& CompletedTech,
    int64& TeamCredits,FGuid& OutOrderId)
{
    OutOrderId.Invalidate();
    if (!IsValid(Definition) || !Faction.IsValid() || !Type.IsValid() || !Definition->UnitId.IsValid()
        || Definition->FactionId!=Faction || Definition->ProductionType!=Type
        || Orders.Num()>=Capacity || Definition->CreditCost<0 || TeamCredits<Definition->CreditCost
        || !FMath::IsFinite(Definition->ProductionSeconds) || Definition->ProductionSeconds<=0.f
        || !CompletedTech.HasAllExact(Definition->Prerequisites)) return false;
    FHRProductionOrder Order;
    Order.OrderId=FGuid::NewGuid();Order.Definition=Definition;
    Order.PaidCredits=Definition->CreditCost;Order.DurationSeconds=Definition->ProductionSeconds;
    Orders.Add(Order);
    TeamCredits-=Order.PaidCredits;
    OutOrderId=Order.OrderId;
    return true;
}

bool FHRProductionLane::Cancel(FGuid OrderId,int64& TeamCredits)
{
    const int32 Index=Orders.IndexOfByPredicate([OrderId](const FHRProductionOrder& Order){return Order.OrderId==OrderId;});
    if (Index==INDEX_NONE || TeamCredits>TNumericLimits<int64>::Max()-Orders[Index].PaidCredits) return false;
    TeamCredits+=Orders[Index].PaidCredits;
    Orders.RemoveAt(Index);
    return true;
}

void FHRProductionLane::Advance(float DeltaSeconds,float ProductionRate)
{
    if (Orders.IsEmpty() || !FMath::IsFinite(DeltaSeconds) || !FMath::IsFinite(ProductionRate)
        || DeltaSeconds<=0.f || ProductionRate<=0.f) return;
    FHRProductionOrder& Active=Orders[0];
    Active.WorkSeconds=FMath::Min(Active.DurationSeconds,
        Active.WorkSeconds+DeltaSeconds*FMath::Clamp(ProductionRate,0.f,1.f));
}

const FHRProductionOrder* FHRProductionLane::PeekReady() const
{
    return !Orders.IsEmpty() && Orders[0].WorkSeconds>=Orders[0].DurationSeconds ? &Orders[0] : nullptr;
}

bool FHRProductionLane::CommitSpawn(FGuid ReadyOrderId)
{
    const FHRProductionOrder* Ready=PeekReady();
    if (!Ready || Ready->OrderId!=ReadyOrderId) return false;
    Orders.RemoveAt(0);return true;
}
