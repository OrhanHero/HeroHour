#include "HRResourceField.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/ScopeExit.h"
#include <limits>

bool RunHRResourceFieldSelfTests(UWorld* TestWorld)
{
    bool Passed = true;
    int32 Count = 0;
    auto Check = [&](bool Condition, const TCHAR* Label)
    {
        ++Count;
        Passed &= Condition;
        UE_LOG(LogTemp, Display, TEXT("RESOURCE_TEST %s: %s"), Label,
            Condition ? TEXT("PASS") : TEXT("FAIL"));
    };

    TArray<AActor*> Actors;
    ON_SCOPE_EXIT
    {
        for (AActor* Actor : Actors)
            if (IsValid(Actor) && !Actor->IsActorBeingDestroyed()) Actor->Destroy();
    };
    auto SpawnCollector = [&]() -> AActor*
    {
        if (!TestWorld) return nullptr;
        FActorSpawnParameters Parameters;
        Parameters.ObjectFlags |= RF_Transient;
        Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* Actor = TestWorld->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator, Parameters);
        if (Actor) Actors.Add(Actor);
        return Actor;
    };
    AActor* A = SpawnCollector();
    AActor* B = SpawnCollector();
    AActor* C = SpawnCollector();
    if (!A || !B || !C)
    {
        Check(false, TEXT("transient world collectors can be spawned"));
        UE_LOG(LogTemp, Display, TEXT("RESOURCE_TEST_COUNT=%d"), Count);
        UE_LOG(LogTemp, Display, TEXT("RESOURCE_TEST_RESULT=FAIL"));
        return false;
    }

    FHRTempestFieldDefinition Definition;
    Definition.Capacity = 100.f;
    Definition.InitialAmount = 60.f;
    Definition.HarvestRate = 10.f;
    Definition.RegrowthPerSecond = 4.f;
    Definition.RegrowthDelay = 2.f;
    Definition.SeedFloor = 20.f;
    FHRResourceField Field;
    Check(Field.Reset(Definition, EHREconomyStyle::FiniteSupply, 3)
        && Field.Num() == 3 && Field.GetTotalAmount() == 180.0
        && Field.GetCell(0)->CellId == 0 && Field.GetCell(2)->CellId == 2
        && !Field.GetCell(-1) && !Field.GetCell(3),
        TEXT("reset creates stable cells and initial stock is explicitly per cell"));

    Check(Field.TryReserve(0, A) && Field.TryReserve(0, A)
        && !Field.TryReserve(0, B) && !Field.TryReserve(1, A)
        && Field.TryReserve(1, B) && Field.GetReservedCell(A) == 0
        && Field.GetReservedCell(B) == 1,
        TEXT("reservation is exclusive, idempotent, and limited to one cell per collector"));

    const float NaN = std::numeric_limits<float>::quiet_NaN();
    const float Infinity = std::numeric_limits<float>::infinity();
    Check(Field.Harvest(0, B, 10.f, 1.f, 1) == 0.f
        && Field.Harvest(0, nullptr, 10.f, 1.f, 1) == 0.f
        && Field.Harvest(-1, A, 10.f, 1.f, 1) == 0.f
        && Field.Harvest(0, A, -1.f, 1.f, 1) == 0.f
        && Field.Harvest(0, A, NaN, 1.f, 1) == 0.f
        && Field.Harvest(0, A, 10.f, Infinity, 1) == 0.f
        && Field.GetTotalAmount() == 180.0,
        TEXT("wrong owner and malformed inputs cannot debit stock"));

    const float Before = Field.GetAmount(0);
    const float Taken = Field.Harvest(0, A, 7.25f, 2.f, 10);
    Check(Taken == 7.25f && static_cast<double>(Before) - Field.GetAmount(0) == Taken
        && Field.GetTotalAmount() + Taken == 180.0,
        TEXT("cargo limit bounds the actual debit and conserves field plus cargo"));

    const float BeforeRetry = Field.GetAmount(0);
    const bool RejectedRetries = Field.Harvest(0, A, 10.f, 1.f, 10) == 0.f
        && Field.Harvest(0, A, 10.f, 1.f, 9) == 0.f
        && Field.GetAmount(0) == BeforeRetry;
    Check(RejectedRetries && Field.Harvest(0, A, 5.f, 1.f, 11) == 5.f,
        TEXT("accepted nonzero operation IDs reject duplicate and older debits"));

    const float OtherCellTaken = Field.Harvest(1, B, 3.f, 1.f, 10);
    const float TrustedPulseOne = Field.Harvest(0, A, 1.f, 1.f, 0);
    const float TrustedPulseTwo = Field.Harvest(0, A, 1.f, 1.f, 0);
    Check(OtherCellTaken == 3.f && Field.Harvest(1, B, 3.f, 1.f, 10) == 0.f
        && TrustedPulseOne == 1.f && TrustedPulseTwo == 1.f
        && Field.Harvest(0, A, 1.f, 1.f, 11) == 0.f,
        TEXT("operation IDs are per cell and trusted zero-ID pulses retain replay protection"));

    Field.Release(A);
    Check(!Field.HasValidReservation(0, A) && Field.Harvest(0, A, 10.f, 1.f) == 0.f
        && Field.TryReserve(2, A) && !Field.ReleaseCell(2, B)
        && Field.HasValidReservation(2, A) && Field.ReleaseCell(2, A)
        && Field.GetReservedCell(A) == INDEX_NONE,
        TEXT("manual release permits a new cell and another owner cannot release it"));

    Field.TryReserve(2, A);
    Field.ReleaseInvalidReservations([&](const AActor* Collector) { return Collector != B; });
    Check(!Field.HasValidReservation(1, B) && Field.HasValidReservation(2, A),
        TEXT("gameplay death or manual-order eligibility clears only affected ownership"));

    const bool ReservedBeforeDeath = Field.TryReserve(0, C);
    const bool Destroyed = C->Destroy();
    Field.Advance(0.f);
    Check(ReservedBeforeDeath && Destroyed && !Field.HasValidReservation(0, C)
        && !Field.TryReserve(0, C) && Field.TryReserve(0, B),
        TEXT("destroying a world actor invalidates weak ownership and releases its cell"));

    Definition.InitialAmount = 100.f;
    Check(Field.Reset(Definition, EHREconomyStyle::FiniteSupply, 1)
        && Field.GetSimulationSeconds() == 0.0 && Field.GetReservedCell(A) == INDEX_NONE
        && Field.GetReservedCell(B) == INDEX_NONE && Field.TryReserve(0, A)
        && Field.Harvest(0, A, 10.f, 1.f, 1) == 10.f,
        TEXT("valid reset removes old actors, timers, and operation identities"));

    Definition.Capacity = 1100.f;
    Definition.InitialAmount = 1000.123f;
    Definition.HarvestRate = 997.07f;
    FHRResourceField Fractional;
    Fractional.Reset(Definition, EHREconomyStyle::FiniteSupply, 1);
    Fractional.TryReserve(0, A);
    const float FractionalBefore = Fractional.GetAmount(0);
    const float FractionalCargoLimit = 1000.08125f;
    const float FractionalTaken = Fractional.Harvest(0, A, FractionalCargoLimit, 1.f, 1);
    Check(FractionalTaken > 0.f
        && static_cast<double>(FractionalBefore) - Fractional.GetAmount(0) == FractionalTaken
        && static_cast<double>(FractionalTaken) <= static_cast<double>(Definition.HarvestRate)
        && FractionalTaken <= FractionalCargoLimit,
        TEXT("awkward float amounts conserve an exact representable debit below rate and cargo bounds"));

    const float TinyBefore = Fractional.GetAmount(0);
    const float TinyTaken = Fractional.Harvest(0, A, 1.f, 1.e-12f, 2);
    Check(TinyTaken == 0.f && Fractional.GetAmount(0) == TinyBefore
        && Fractional.Harvest(0, A, 1.f, 1.f, 2) > 0.f,
        TEXT("unrepresentable tiny pulses do not consume their operation identity"));

    Definition.Capacity = 100.f;
    Definition.InitialAmount = 100.f;
    Definition.HarvestRate = 20.f;
    Definition.SeedFloor = 10.f;
    FHRResourceField Regrowing;
    Regrowing.Reset(Definition, EHREconomyStyle::RegrowingField, 1);
    Regrowing.TryReserve(0, A);
    Regrowing.Harvest(0, A, 100.f, 1.f, 1);
    const bool BeforeDelay = Regrowing.Advance(1.5f) && Regrowing.GetAmount(0) == 80.f;
    const bool PartialStep = Regrowing.Advance(1.f) && Regrowing.GetAmount(0) == 82.f;
    const bool FullCargo = Regrowing.Harvest(0, A, 0.f, 1.f, 2) == 0.f;
    Regrowing.Advance(.5f);
    const bool FailedPulseDidNotDelay = Regrowing.GetAmount(0) == 84.f;
    const bool NewHarvest = Regrowing.Harvest(0, A, 4.f, 1.f, 2) == 4.f;
    Regrowing.Advance(1.f);
    const bool NewDelay = Regrowing.GetAmount(0) == 80.f;
    Regrowing.Advance(2.f);
    Check(BeforeDelay && PartialStep && FullCargo && FailedPulseDidNotDelay && NewHarvest
        && NewDelay && Regrowing.GetAmount(0) == 84.f,
        TEXT("regrowth uses only elapsed time past delay and only accepted harvest resets it"));

    Regrowing.Advance(1000.f);
    Check(Regrowing.GetAmount(0) == 100.f && Regrowing.SetAmount(0, 1000.f)
        && Regrowing.GetAmount(0) == 100.f && !Regrowing.SetAmount(0, -1.f)
        && !Regrowing.SetAmount(0, NaN) && Regrowing.GetTotalAmount() == 100.0,
        TEXT("regrowth and explicit restore respect capacity and reject malformed amounts"));

    Definition.Capacity = 50.f;
    Definition.InitialAmount = 50.f;
    Definition.SeedFloor = 5.f;
    Definition.HarvestRate = 100.f;
    FHRResourceField Seeded;
    Seeded.Reset(Definition, EHREconomyStyle::RegrowingField, 1);
    Seeded.TryReserve(0, A);
    const bool ProtectedSeed = Seeded.Harvest(0, A, 100.f, 1.f) == 45.f
        && Seeded.GetAmount(0) == 5.f && Seeded.Harvest(0, A, 100.f, 1.f) == 0.f;
    Seeded.Reset(Definition, EHREconomyStyle::FiniteSupply, 1);
    Seeded.TryReserve(0, A);
    const bool FiniteDrains = Seeded.Harvest(0, A, 100.f, 1.f) == 50.f;
    Seeded.Advance(100.f);
    Seeded.Release(A);
    Check(ProtectedSeed && FiniteDrains && Seeded.GetAmount(0) == 0.f
        && !Seeded.TryReserve(0, B),
        TEXT("regrowing style protects seed while finite supply drains fully without regrowth"));

    Definition.Capacity = 400.1234f;
    Definition.InitialAmount = 400.1234f;
    Definition.HarvestRate = 13.7f;
    FHRResourceField Loop;
    Loop.Reset(Definition, EHREconomyStyle::FiniteSupply, 1);
    Loop.TryReserve(0, A);
    const double InitialLoopStock = Loop.GetTotalAmount();
    double Cargo = 0.0;
    double Delivered = 0.0;
    bool LoopBounds = true;
    uint64 Operation = 1;
    for (; Operation <= 1000 && Loop.GetAmount(0) > 0.f; ++Operation)
    {
        const float FreeCargo = static_cast<float>(23.4 - Cargo);
        const float StockBefore = Loop.GetAmount(0);
        const float Pulse = Loop.Harvest(0, A, FreeCargo, .37f, Operation);
        Cargo += Pulse;
        LoopBounds &= Pulse >= 0.f && Pulse <= FreeCargo
            && static_cast<double>(StockBefore) - Loop.GetAmount(0) == Pulse
            && static_cast<double>(Pulse) <= static_cast<double>(Definition.HarvestRate) * .37f
            && FMath::Abs(InitialLoopStock - Loop.GetTotalAmount() - Cargo - Delivered) < 1.e-9;
        if (Pulse == 0.f || Cargo >= 23.39)
        {
            Delivered += Cargo;
            Cargo = 0.0;
        }
    }
    Delivered += Cargo;
    Check(LoopBounds && Operation < 1000 && Loop.GetAmount(0) == 0.f
        && Delivered == InitialLoopStock,
        TEXT("many cargo and delivery cycles preserve every debited unit without exceeding bounds"));

    Definition.Capacity = 50.f;
    Definition.InitialAmount = 80.f;
    Definition.SeedFloor = 500.f;
    FHRResourceField Clamped;
    const bool ClampedAtCapacity = Clamped.Reset(Definition, EHREconomyStyle::RegrowingField, 2)
        && Clamped.GetDefinition().InitialAmount == 50.f && Clamped.GetDefinition().SeedFloor == 50.f
        && Clamped.GetTotalAmount() == 100.0 && Clamped.GetHarvestableAmount(0) == 0.f;
    Definition.InitialAmount = 4.f;
    Definition.SeedFloor = 40.f;
    Check(ClampedAtCapacity && Clamped.Reset(Definition, EHREconomyStyle::RegrowingField, 2)
        && Clamped.GetTotalAmount() == 8.0 && Clamped.GetHarvestableAmount(0) == 0.f,
        TEXT("capacity clamps configuration and a protected seed never inflates initial stock"));

    const double TimeBeforeInvalid = Regrowing.GetSimulationSeconds();
    const double StockBeforeInvalid = Regrowing.GetTotalAmount();
    FHRTempestFieldDefinition Invalid = Definition;
    Invalid.InitialAmount = NaN;
    Check(!Regrowing.Advance(-1.f) && !Regrowing.Advance(NaN) && !Regrowing.Advance(Infinity)
        && !Regrowing.Reset(Invalid, EHREconomyStyle::RegrowingField, 1)
        && !Regrowing.Reset(Definition, EHREconomyStyle::FiniteSupply, -1)
        && !Regrowing.Reset(Definition, static_cast<EHREconomyStyle>(255), 1)
        && Regrowing.GetSimulationSeconds() == TimeBeforeInvalid
        && Regrowing.GetTotalAmount() == StockBeforeInvalid,
        TEXT("invalid time and reset configuration fail without changing an established field"));

    UE_LOG(LogTemp, Display, TEXT("RESOURCE_TEST_COUNT=%d"), Count);
    UE_LOG(LogTemp, Display, TEXT("RESOURCE_TEST_RESULT=%s"), Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}
