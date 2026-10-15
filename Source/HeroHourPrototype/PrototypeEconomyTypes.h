#pragma once
#include "CoreMinimal.h"
#include "RTS/HRResourceField.h"

enum class EPrototypeRole : uint8
{
    Combat, Collector, Headquarters, Depot, Factory,
    PowerPlant, Barracks, Radar, Defense, Infantry, Siege, Count
};

struct FPrototypeSalvage
{
    FVector Position = FVector::ZeroVector;
    float Remaining = 3000.f;
    // Read-only presentation mirror. Only Field owns resource stock.
    FHRResourceField Field;
    TArray<FVector> CellOffsets;
    TArray<TWeakObjectPtr<class AActor>> CellVisuals;
    TArray<bool> CellDepleted;
    // First cell alias retained for the HUD and existing visual inspection hooks.
    TWeakObjectPtr<class AActor> VisualActor;
    bool bVisualDepleted = false;

    FVector CellPosition(int32 CellIndex) const
    {
        return CellOffsets.IsValidIndex(CellIndex) ? Position + CellOffsets[CellIndex] : Position;
    }
    void RefreshAmount() { Remaining = static_cast<float>(Field.GetTotalAmount()); }
};

struct FPrototypeProduction
{
    TWeakObjectPtr<class APrototypeUnit> Producer;
    EPrototypeRole Role = EPrototypeRole::Combat;
    float Progress = 0.f;
    float Duration = 5.f;
    float PaidCost = 0.f;
};
