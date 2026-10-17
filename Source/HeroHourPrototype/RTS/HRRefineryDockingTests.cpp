#include "HRRefineryDocking.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Misc/ScopeExit.h"
#include <limits>

bool RunHRRefineryDockingSelfTests(UWorld* TestWorld)
{
    bool Passed = true;
    int32 Count = 0;
    auto Check = [&](bool Condition, const TCHAR* Label)
    {
        ++Count;
        Passed &= Condition;
        UE_LOG(LogTemp, Display, TEXT("DOCKING_TEST %s: %s"), Label, Condition ? TEXT("PASS") : TEXT("FAIL"));
    };
    TArray<AActor*> Actors;
    ON_SCOPE_EXIT
    {
        for (AActor* Actor : Actors)
            if (IsValid(Actor) && !Actor->IsActorBeingDestroyed()) Actor->Destroy();
    };
    auto Spawn = [&]() -> AActor*
    {
        if (!TestWorld) return nullptr;
        FActorSpawnParameters Parameters;
        Parameters.ObjectFlags |= RF_Transient;
        Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        AActor* Actor = TestWorld->SpawnActor<AActor>(FVector::ZeroVector, FRotator::ZeroRotator, Parameters);
        if (Actor) Actors.Add(Actor);
        return Actor;
    };
    AActor* Refinery = Spawn();
    AActor* A = Spawn();
    AActor* B = Spawn();
    AActor* C = Spawn();
    Check(Refinery && A && B && C, TEXT("transient actors available"));
    if (!Passed)
    {
        UE_LOG(LogTemp, Display, TEXT("DOCKING_TEST_COUNT=%d"), Count);
        UE_LOG(LogTemp, Display, TEXT("DOCKING_TEST_RESULT=FAIL"));
        return false;
    }
    FHRRefineryDocking Dock;
    Check(Dock.Reset(Refinery) && !Dock.Submit(nullptr) && !Dock.Submit(Refinery), TEXT("valid refinery rejects invalid collector requests"));
    Check(Dock.Submit(A) && Dock.Submit(A) && Dock.Submit(B) && Dock.NumQueued() == 2
        && Dock.GetNextWaiting() == A, TEXT("FIFO requests are idempotent"));
    double CargoA = 100.0, CargoB = 100.0, Wallet = 25.0;
    Check(!Dock.TryAcquireNext(B, true) && !Dock.TryAcquireNext(A, false)
        && Dock.Unload(A, 1.0, CargoA, Wallet, 1) == 0.0 && Wallet == 25.0,
        TEXT("queue promotion requires head and actual reachable arrival without payment"));
    Check(Dock.TryAcquireNext(A, true) && Dock.TryAcquireNext(A, true)
        && Dock.IsDocked(A) && !Dock.IsQueued(A) && Dock.IsQueued(B)
        && Dock.NumDocked() == 1 && !Dock.TryAcquireNext(B, true), TEXT("single slot ownership is exclusive"));
    Check(Dock.Unload(B, 1.0, CargoB, Wallet, 1) == 0.0 && CargoB == 100.0 && Wallet == 25.0,
        TEXT("waiting collector cannot spend another collector dock"));
    Check(Dock.Unload(A, 0.25, CargoA, Wallet, 1) == 10.0 && CargoA == 90.0 && Wallet == 35.0,
        TEXT("own rate caps actual transfer at forty per second"));
    Check(Dock.Unload(A, 0.25, CargoA, Wallet, 1) == 0.0
        && Dock.Unload(A, 0.25, CargoA, Wallet, 0) == 0.0 && CargoA == 90.0 && Wallet == 35.0,
        TEXT("committed operation and zero ID cannot replay"));
    const double NaN = std::numeric_limits<double>::quiet_NaN();
    const double Infinity = std::numeric_limits<double>::infinity();
    Check(Dock.Unload(A, 0.0, CargoA, Wallet, 2) == 0.0
        && Dock.Unload(A, -1.0, CargoA, Wallet, 2) == 0.0
        && Dock.Unload(A, NaN, CargoA, Wallet, 2) == 0.0
        && Dock.Unload(A, Infinity, CargoA, Wallet, 2) == 0.0
        && CargoA == 90.0 && Wallet == 35.0, TEXT("invalid time has no mutation"));
    double BadCargo = NaN, GoodWallet = 10.0, GoodCargo = 5.0, BadWallet = -1.0;
    Check(Dock.Unload(A, 1.0, BadCargo, GoodWallet, 2) == 0.0 && GoodWallet == 10.0
        && Dock.Unload(A, 1.0, GoodCargo, BadWallet, 2) == 0.0 && GoodCargo == 5.0,
        TEXT("invalid cargo or wallet cannot commit"));
    BadCargo = -5.0;
    BadWallet = Infinity;
    Check(Dock.Unload(A, 1.0, BadCargo, GoodWallet, 2) == 0.0 && BadCargo == -5.0
        && Dock.Unload(A, 1.0, GoodCargo, BadWallet, 2) == 0.0 && GoodCargo == 5.0,
        TEXT("negative stock and infinite wallet cannot mutate"));
    CargoA = 2.125;
    const double BeforeTotal = CargoA + Wallet;
    Check(Dock.Unload(A, 0.03125, CargoA, Wallet, 2) == 1.25
        && CargoA == 0.875 && CargoA + Wallet == BeforeTotal,
        TEXT("fractional debit and wallet preserve exact representable balance"));
    Check(Dock.Unload(A, 1.0, CargoA, Wallet, 3) == 0.875 && CargoA == 0.0
        && Dock.Unload(A, 1.0, CargoA, Wallet, 4) == 0.0 && CargoA + Wallet == BeforeTotal,
        TEXT("last cargo fraction pays once without overdraw"));
    CargoA = 50.0;
    Dock.Release(A);
    Check(CargoA == 50.0 && !Dock.IsDocked(A) && Dock.TryAcquireNext(B, true),
        TEXT("manual release preserves cargo and next FIFO owner acquires"));
    Dock.Release(B);
    Dock.Submit(A);
    Dock.TryAcquireNext(A, true);
    Check(Dock.Unload(A, 1.0, CargoA, Wallet, 3) == 0.0 && CargoA == 50.0,
        TEXT("release and requeue cannot replay a committed payment"));
    Dock.Submit(B);
    Dock.ReleaseInvalid([&](AActor* Actor) { return Actor != A; });
    Check(!Dock.IsDocked(A) && CargoA == 50.0 && Dock.TryAcquireNext(B, true),
        TEXT("role health or manual predicate invalidates claim without touching cargo"));
    Dock.Submit(C);
    B->Destroy();
    Dock.ReleaseInvalid([](AActor*) { return true; });
    Check(!Dock.IsDocked(B) && Dock.TryAcquireNext(C, true), TEXT("destroyed collector frees weak slot"));
    Refinery->Destroy();
    double CargoC = 20.0;
    const double BeforeDestroyWallet = Wallet;
    Check(Dock.Unload(C, 1.0, CargoC, Wallet, 1) == 0.0 && CargoC == 20.0
        && Wallet == BeforeDestroyWallet && !Dock.Submit(A), TEXT("destroyed refinery cannot pay or accept requests"));
    Dock.ReleaseInvalid([](AActor*) { return true; });
    Check(Dock.NumQueued() == 0 && Dock.NumDocked() == 0 && CargoC == 20.0,
        TEXT("refinery loss clears queue and slots preserving cargo"));
    AActor* NewRefinery = Spawn();
    Check(Dock.Reset(NewRefinery, 2, 40.0) && Dock.Submit(A) && Dock.Submit(C)
        && Dock.TryAcquireNext(A, true) && Dock.TryAcquireNext(C, true) && Dock.NumDocked() == 2,
        TEXT("reset supports configured multiple slots and clears old match claims"));
    Check(Dock.Unload(A, 0.25, CargoA, Wallet, 1) == 10.0
        && !Dock.Reset(NewRefinery, 0) && !Dock.Reset(NewRefinery, 1, NaN)
        && Dock.IsDocked(A), TEXT("match reset clears replay history and malformed reset preserves state"));
    CargoA = 3.0;
    Wallet = 1e16;
    const double HugeWallet = Wallet;
    Check(Dock.Unload(A, 0.075, CargoA, Wallet, 2) == 2.0
        && CargoA == 1.0 && Wallet - HugeWallet == 2.0,
        TEXT("coarse wallet ULP rounds transfer down without minting extra credit"));
    Check(Dock.Unload(A, 1.0, CargoA, Wallet, 3) == 0.0 && CargoA == 1.0
        && Wallet - HugeWallet == 2.0,
        TEXT("unrepresentable small remainder remains cargo"));
    CargoA = 0.3;
    Wallet = 0.07;
    const double DecimalCargo = CargoA, DecimalWallet = Wallet;
    const double DecimalTransfer = Dock.Unload(A, 0.00175, CargoA, Wallet, 4);
    Check(DecimalTransfer > 0.0 && DecimalTransfer <= 40.0 * 0.00175
        && DecimalCargo - CargoA == Wallet - DecimalWallet
        && DecimalTransfer == Wallet - DecimalWallet,
        TEXT("nonbinary fractions book identical real deltas within rate budget"));
    CargoA = 0.3;
    Wallet = 0.0;
    const double ZeroWalletTransfer = Dock.Unload(A, 1.0, CargoA, Wallet, 5);
    Check(ZeroWalletTransfer == 0.3 && CargoA == 0.0 && Wallet == 0.3,
        TEXT("zero wallet uses cargo quantum and supports complete discharge"));
    const double Tiny = std::numeric_limits<double>::denorm_min();
    CargoA = Tiny * 3.0;
    Wallet = 0.0;
    const double TinyTransfer = Dock.Unload(A, Tiny, CargoA, Wallet, 6);
    Check(TinyTransfer == Tiny * 3.0 && CargoA == 0.0 && Wallet == TinyTransfer,
        TEXT("subnormal cargo and zero wallet commit the same represented amount"));
    CargoA = 3.0;
    Wallet = std::numeric_limits<double>::max();
    Check(Dock.Unload(A, 1.0, CargoA, Wallet, 7) == 0.0 && CargoA == 3.0
        && Wallet == std::numeric_limits<double>::max(),
        TEXT("maximum finite wallet overflow preserves both balances"));
    Dock.Release(A);
    Dock.Release(C);
    Check(Dock.NumDocked() == 0 && Dock.NumQueued() == 0, TEXT("explicit release clears all claims"));
    UE_LOG(LogTemp, Display, TEXT("DOCKING_TEST_COUNT=%d"), Count);
    UE_LOG(LogTemp, Display, TEXT("DOCKING_TEST_RESULT=%s"), Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}
