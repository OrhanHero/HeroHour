#include "HRRefineryDocking.h"
#include "GameFramework/Actor.h"
#include <cmath>
#include <limits>

namespace
{
bool DockActorValid(AActor* Actor)
{
    return IsValid(Actor) && !Actor->IsActorBeingDestroyed();
}
}

bool FHRRefineryDocking::HasValidRefinery() const { return DockActorValid(Owner.Get()); }

bool FHRRefineryDocking::Reset(AActor* Refinery, int32 Slots, double UnloadRate)
{
    if (!DockActorValid(Refinery) || Slots <= 0 || !FMath::IsFinite(UnloadRate) || UnloadRate <= 0.0)
        return false;
    Owner = Refinery;
    SlotCount = Slots;
    Rate = UnloadRate;
    Waiting.Reset();
    Docked.Reset();
    LastCommittedOperation.Reset();
    return true;
}

bool FHRRefineryDocking::IsQueued(AActor* Collector) const
{
    return HasValidRefinery() && DockActorValid(Collector) && Waiting.Contains(TWeakObjectPtr<AActor>(Collector));
}
bool FHRRefineryDocking::IsDocked(AActor* Collector) const
{
    return HasValidRefinery() && DockActorValid(Collector) && Docked.Contains(TWeakObjectPtr<AActor>(Collector));
}
bool FHRRefineryDocking::Submit(AActor* Collector)
{
    if (!HasValidRefinery() || !DockActorValid(Collector) || Collector == Owner.Get()) return false;
    if (IsQueued(Collector) || IsDocked(Collector)) return true;
    Waiting.Add(Collector);
    return true;
}
AActor* FHRRefineryDocking::GetNextWaiting() const
{
    if (!HasValidRefinery()) return nullptr;
    for (const auto& Candidate : Waiting)
        if (DockActorValid(Candidate.Get())) return Candidate.Get();
    return nullptr;
}
bool FHRRefineryDocking::TryAcquireNext(AActor* Collector, bool bAtReachableDockPosition)
{
    if (!HasValidRefinery() || !DockActorValid(Collector) || !bAtReachableDockPosition) return false;
    if (IsDocked(Collector)) return true;
    Waiting.RemoveAll([](const auto& Item) { return !DockActorValid(Item.Get()); });
    Docked.RemoveAll([](const auto& Item) { return !DockActorValid(Item.Get()); });
    if (Docked.Num() >= SlotCount || Waiting.IsEmpty() || Waiting[0].Get() != Collector) return false;
    Waiting.RemoveAt(0);
    Docked.Add(Collector);
    return true;
}
void FHRRefineryDocking::Release(AActor* Collector)
{
    if (!Collector) return;
    const TWeakObjectPtr<AActor> Key(Collector);
    Waiting.Remove(Key);
    Docked.Remove(Key);
    // Preserve committed operation history: release must not make a paid operation replayable.
}
void FHRRefineryDocking::ReleaseInvalid(TFunctionRef<bool(AActor*)> IsEligible)
{
    if (!HasValidRefinery()) { Waiting.Reset(); Docked.Reset(); return; }
    auto Invalid = [&](const auto& Item) { return !DockActorValid(Item.Get()) || !IsEligible(Item.Get()); };
    Waiting.RemoveAll(Invalid);
    Docked.RemoveAll(Invalid);
    for (auto It = LastCommittedOperation.CreateIterator(); It; ++It)
        if (!DockActorValid(It.Key().Get())) It.RemoveCurrent();
}
double FHRRefineryDocking::Unload(AActor* Collector, double DeltaSeconds, double& Cargo, double& Wallet, uint64 OperationId)
{
    if (!IsDocked(Collector) || OperationId == 0 || !FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.0
        || !FMath::IsFinite(Cargo) || Cargo <= 0.0 || !FMath::IsFinite(Wallet) || Wallet < 0.0) return 0.0;
    const TWeakObjectPtr<AActor> Key(Collector);
    const uint64* Previous = LastCommittedOperation.Find(Key);
    if (Previous && OperationId <= *Previous) return 0.0;
    const double Budget = Rate * DeltaSeconds;
    if (!FMath::IsFinite(Budget) || Budget <= 0.0) return 0.0;
    const double Bounded = FMath::Min(Cargo, Budget);
    // Commit only a common representable debit/credit quantum. Round downward:
    // a wallet with coarse spacing must never round three cargo into four credits.
    const double WalletQuantum = std::nextafter(Wallet, std::numeric_limits<double>::infinity()) - Wallet;
    const double CargoQuantum = Cargo - std::nextafter(Cargo, 0.0);
    const double Quantum = FMath::Max(WalletQuantum, CargoQuantum);
    if (!FMath::IsFinite(Quantum) || Quantum <= 0.0) return 0.0;
    const double Transfer = std::floor(Bounded / Quantum) * Quantum;
    if (!FMath::IsFinite(Transfer) || Transfer <= 0.0 || Transfer > Bounded) return 0.0;
    const double NewCargo = Cargo - Transfer;
    const double NewWallet = Wallet + Transfer;
    if (!FMath::IsFinite(NewWallet) || NewWallet <= Wallet || NewCargo >= Cargo) return 0.0;
    const double ActualDebit = Cargo - NewCargo;
    const double ActualCredit = NewWallet - Wallet;
    // Exponent boundaries can change spacing; reject rather than invent or lose money.
    if (ActualDebit != ActualCredit || ActualDebit != Transfer || ActualDebit > Bounded) return 0.0;
    Cargo = NewCargo;
    Wallet = NewWallet;
    LastCommittedOperation.Add(Key, OperationId);
    return Transfer;
}
