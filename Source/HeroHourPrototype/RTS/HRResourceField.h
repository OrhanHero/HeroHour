#pragma once

#include "CoreMinimal.h"
#include "HRRTSDefinitions.h"

class UWorld;

/**
 * Authoritative resource amounts and reservations for one field.
 *
 * Cells have stable array indices until Reset. Their positions, navigation,
 * visuals and collector cargo belong to the game mode. Call Advance once per
 * simulation step and add only the amount returned by Harvest to cargo.
 * This value type owns no actors and is intended for game-thread use.
 */
class HEROHOURPROTOTYPE_API FHRResourceField
{
public:
    /** Valid reset clears every reservation, regrowth timer and operation ID. */
    bool Reset(const FHRTempestFieldDefinition& InDefinition, EHREconomyStyle InStyle, int32 CellCount);

    /** Advances regrowth by the part of the step occurring after its delay. */
    bool Advance(float DeltaSeconds);

    /** One collector can reserve at most one cell of this field. */
    bool TryReserve(int32 CellId, AActor* Collector);
    bool HasValidReservation(int32 CellId, const AActor* Collector) const;
    int32 GetReservedCell(const AActor* Collector) const;
    void Release(const AActor* Collector);
    bool ReleaseCell(int32 CellId, const AActor* Collector);

    /** Destroyed actors are removed automatically. Add gameplay death/manual-order checks here. */
    void ReleaseInvalidReservations();
    void ReleaseInvalidReservations(TFunctionRef<bool(const AActor*)> IsEligibleCollector);

    /**
     * Exact-owner debit bounded by rate * seconds, free cargo and harvestable stock.
     * Returns the representable amount actually removed; sub-quantum pulses return zero.
     * Nonzero OperationId values must increase per cell; accepted duplicates or
     * older IDs cannot debit it again. Zero opts out for a trusted pulse caller.
     * Failed/zero harvests do not consume an operation ID or delay regrowth.
     */
    float Harvest(int32 CellId, AActor* Collector, float FreeCargoCapacity,
        float HarvestSeconds, uint64 OperationId = 0);

    /** Explicit compatibility/editor restore; clamps at capacity without resetting timers. */
    bool SetAmount(int32 CellId, float Amount);

    int32 Num() const { return Cells.Num(); }
    const FHRTempestCellState* GetCell(int32 CellId) const;
    const TArray<FHRTempestCellState>& GetCells() const { return Cells; }
    float GetAmount(int32 CellId) const;
    float GetHarvestableAmount(int32 CellId) const;
    double GetTotalAmount() const;
    double GetSimulationSeconds() const { return SimulationSeconds; }
    EHREconomyStyle GetStyle() const { return Style; }
    const FHRTempestFieldDefinition& GetDefinition() const { return Definition; }

private:
    static bool IsLiveCollector(const AActor* Collector);

    FHRTempestFieldDefinition Definition;
    EHREconomyStyle Style = EHREconomyStyle::FiniteSupply;
    TArray<FHRTempestCellState> Cells;
    TArray<double> RegrowthReadyAt;
    TArray<uint64> LastHarvestOperation;
    double SimulationSeconds = 0.0;
};

/** Uses transient world actors to exercise weak ownership and destruction. */
HEROHOURPROTOTYPE_API bool RunHRResourceFieldSelfTests(UWorld* TestWorld);
