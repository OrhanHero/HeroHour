#pragma once
#include "CoreMinimal.h"

enum class EPrototypeRole : uint8
{
    Combat, Collector, Headquarters, Depot, Factory
};

struct FPrototypeSalvage
{
    FVector Position = FVector::ZeroVector;
    float Remaining = 3000.f;
};

struct FPrototypeProduction
{
    TWeakObjectPtr<class APrototypeUnit> Producer;
    EPrototypeRole Role = EPrototypeRole::Combat;
    float Progress = 0.f;
    float Duration = 5.f;
};
