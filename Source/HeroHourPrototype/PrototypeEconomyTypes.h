#pragma once
#include "CoreMinimal.h"
#include "RTS/HRResourceField.h"

enum class EPrototypeRole : uint8
{
    Combat, Collector, Headquarters, Depot, Factory
};

struct FPrototypeSalvage
{
    FVector Position = FVector::ZeroVector;
    float Remaining = 3000.f;
    // The old visible point remains a compatibility view of one authoritative cell.
    FHRResourceField Field;
    TWeakObjectPtr<class APrototypeUnit> Collector;
    TWeakObjectPtr<class AActor> VisualActor;
    bool bVisualDepleted = false;
};

struct FPrototypeProduction
{
    TWeakObjectPtr<class APrototypeUnit> Producer;
    EPrototypeRole Role = EPrototypeRole::Combat;
    float Progress = 0.f;
    float Duration = 5.f;
    float PaidCost = 0.f;
};
