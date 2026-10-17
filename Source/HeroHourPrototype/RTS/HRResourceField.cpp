#include "HRResourceField.h"
#include "GameFramework/Actor.h"
#include <cmath>

namespace
{
    bool IsFiniteNonNegative(float Value)
    {
        return FMath::IsFinite(Value) && Value >= 0.f;
    }
}

bool FHRResourceField::Reset(const FHRTempestFieldDefinition& InDefinition,
    EHREconomyStyle InStyle, int32 CellCount)
{
    if (CellCount < 0 || CellCount > 1000000
        || (InStyle != EHREconomyStyle::FiniteSupply && InStyle != EHREconomyStyle::RegrowingField)
        || !IsFiniteNonNegative(InDefinition.Capacity)
        || !IsFiniteNonNegative(InDefinition.InitialAmount)
        || !IsFiniteNonNegative(InDefinition.HarvestRate)
        || !IsFiniteNonNegative(InDefinition.RegrowthPerSecond)
        || !IsFiniteNonNegative(InDefinition.RegrowthDelay)
        || !IsFiniteNonNegative(InDefinition.SeedFloor))
    {
        return false;
    }

    Definition = InDefinition;
    Definition.InitialAmount = FMath::Min(Definition.InitialAmount, Definition.Capacity);
    Definition.SeedFloor = FMath::Min(Definition.SeedFloor, Definition.Capacity);
    Style = InStyle;
    SimulationSeconds = 0.0;
    Cells.SetNum(CellCount);
    RegrowthReadyAt.Init(0.0, CellCount);
    LastHarvestOperation.Init(0, CellCount);
    for (int32 Index = 0; Index < CellCount; ++Index)
    {
        Cells[Index].CellId = Index;
        Cells[Index].Amount = Definition.InitialAmount;
        Cells[Index].ReservedCollector.Reset();
    }
    return true;
}

bool FHRResourceField::IsLiveCollector(const AActor* Collector)
{
    return IsValid(Collector) && !Collector->IsActorBeingDestroyed();
}

bool FHRResourceField::Advance(float DeltaSeconds)
{
    if (!IsFiniteNonNegative(DeltaSeconds)) return false;
    const double StartSeconds = SimulationSeconds;
    const double EndSeconds = StartSeconds + static_cast<double>(DeltaSeconds);
    if (!FMath::IsFinite(EndSeconds)) return false;
    SimulationSeconds = EndSeconds;
    ReleaseInvalidReservations();

    if (Style != EHREconomyStyle::RegrowingField || Definition.RegrowthPerSecond <= 0.f)
        return true;

    for (int32 Index = 0; Index < Cells.Num(); ++Index)
    {
        FHRTempestCellState& Cell = Cells[Index];
        if (Cell.Amount >= Definition.Capacity) continue;
        const double GrowingSeconds = FMath::Max(0.0, EndSeconds - FMath::Max(StartSeconds, RegrowthReadyAt[Index]));
        const double Growth = GrowingSeconds * static_cast<double>(Definition.RegrowthPerSecond);
        Cell.Amount = static_cast<float>(FMath::Min(static_cast<double>(Definition.Capacity),
            static_cast<double>(Cell.Amount) + Growth));
    }
    return true;
}

bool FHRResourceField::TryReserve(int32 CellId, AActor* Collector)
{
    if (!Cells.IsValidIndex(CellId) || !IsLiveCollector(Collector)) return false;
    ReleaseInvalidReservations();
    FHRTempestCellState& Cell = Cells[CellId];
    if (Cell.ReservedCollector.Get() == Collector) return true;
    if (Cell.ReservedCollector.IsValid() || GetHarvestableAmount(CellId) <= 0.f
        || GetReservedCell(Collector) != INDEX_NONE) return false;
    Cell.ReservedCollector = Collector;
    return true;
}

bool FHRResourceField::HasValidReservation(int32 CellId, const AActor* Collector) const
{
    return Cells.IsValidIndex(CellId) && IsLiveCollector(Collector)
        && Cells[CellId].ReservedCollector.Get() == Collector;
}

int32 FHRResourceField::GetReservedCell(const AActor* Collector) const
{
    if (!IsLiveCollector(Collector)) return INDEX_NONE;
    for (const FHRTempestCellState& Cell : Cells)
        if (Cell.ReservedCollector.Get() == Collector) return Cell.CellId;
    return INDEX_NONE;
}

void FHRResourceField::Release(const AActor* Collector)
{
    if (!Collector) return;
    for (FHRTempestCellState& Cell : Cells)
        if (Cell.ReservedCollector.Get() == Collector) Cell.ReservedCollector.Reset();
}

bool FHRResourceField::ReleaseCell(int32 CellId, const AActor* Collector)
{
    if (!Cells.IsValidIndex(CellId) || !Collector || Cells[CellId].ReservedCollector.Get() != Collector)
        return false;
    Cells[CellId].ReservedCollector.Reset();
    return true;
}

void FHRResourceField::ReleaseInvalidReservations()
{
    ReleaseInvalidReservations([](const AActor*) { return true; });
}

void FHRResourceField::ReleaseInvalidReservations(TFunctionRef<bool(const AActor*)> IsEligibleCollector)
{
    for (FHRTempestCellState& Cell : Cells)
    {
        const AActor* Owner = Cell.ReservedCollector.Get();
        if (!IsLiveCollector(Owner) || !IsEligibleCollector(Owner))
            Cell.ReservedCollector.Reset();
    }
}

float FHRResourceField::Harvest(int32 CellId, AActor* Collector, float FreeCargoCapacity,
    float HarvestSeconds, uint64 OperationId)
{
    if (!HasValidReservation(CellId, Collector) || !IsFiniteNonNegative(FreeCargoCapacity)
        || !IsFiniteNonNegative(HarvestSeconds) || FreeCargoCapacity <= 0.f || HarvestSeconds <= 0.f
        || (OperationId != 0 && OperationId <= LastHarvestOperation[CellId]))
        return 0.f;

    // Calculate in double: finite float rate and dt may have an overflowing float product.
    const double Requested = static_cast<double>(Definition.HarvestRate) * HarvestSeconds;
    const float Before = Cells[CellId].Amount;
    const float Seed = Style == EHREconomyStyle::RegrowingField ? Definition.SeedFloor : 0.f;
    const double Harvestable = FMath::Max(0.0, static_cast<double>(Before) - Seed);
    const double BoundedRequest = FMath::Min(Requested,
        FMath::Min(static_cast<double>(FreeCargoCapacity), Harvestable));
    if (BoundedRequest <= 0.0 || Before <= 0.f) return 0.f;

    // Use the spacing immediately below the current float stock as a common
    // debit quantum. Both Taken and Before - Taken are then representable:
    // no cargo appears through rounding, even when taking most of a cell.
    const double Quantum = static_cast<double>(Before) - std::nextafter(Before, 0.f);
    double Quanta = std::floor(BoundedRequest / Quantum);
    if (Quanta <= 0.0) return 0.f;
    float Taken = static_cast<float>(Quanta * Quantum);
    float After = Before - Taken;
    // A tiny protected seed can disappear from Before - Seed even in double.
    // Keep one more quantum in that case rather than crossing the seed floor.
    if (After < Seed)
    {
        Quanta -= 1.0;
        Taken = static_cast<float>(Quanta * Quantum);
        After = Before - Taken;
    }
    if (Taken <= 0.f) return 0.f;

    Cells[CellId].Amount = After;
    RegrowthReadyAt[CellId] = SimulationSeconds + static_cast<double>(Definition.RegrowthDelay);
    if (OperationId != 0) LastHarvestOperation[CellId] = OperationId;
    return Taken;
}

bool FHRResourceField::SetAmount(int32 CellId, float Amount)
{
    if (!Cells.IsValidIndex(CellId) || !IsFiniteNonNegative(Amount)) return false;
    Cells[CellId].Amount = FMath::Min(Amount, Definition.Capacity);
    return true;
}

const FHRTempestCellState* FHRResourceField::GetCell(int32 CellId) const
{
    return Cells.IsValidIndex(CellId) ? &Cells[CellId] : nullptr;
}

float FHRResourceField::GetAmount(int32 CellId) const
{
    const FHRTempestCellState* Cell = GetCell(CellId);
    return Cell ? Cell->Amount : 0.f;
}

float FHRResourceField::GetHarvestableAmount(int32 CellId) const
{
    const float ProtectedSeed = Style == EHREconomyStyle::RegrowingField ? Definition.SeedFloor : 0.f;
    return Cells.IsValidIndex(CellId) ? FMath::Max(0.f, Cells[CellId].Amount - ProtectedSeed) : 0.f;
}

double FHRResourceField::GetTotalAmount() const
{
    double Total = 0.0;
    for (const FHRTempestCellState& Cell : Cells) Total += static_cast<double>(Cell.Amount);
    return Total;
}
