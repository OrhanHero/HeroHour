#include "HHPrototypeEconomyComponent.h"

UHHPrototypeEconomyComponent::UHHPrototypeEconomyComponent()
{
    PrimaryComponentTick.bCanEverTick = true;

    UnitOffers = {
        {EHHPrototypeUnitRole::Infantry, 300, 3.0f},
        {EHHPrototypeUnitRole::Vehicle, 800, 7.0f},
        {EHHPrototypeUnitRole::Air, 1200, 10.0f}
    };
}

void UHHPrototypeEconomyComponent::BeginPlay()
{
    Super::BeginPlay();
    Credits = FMath::Max(0, StartingCredits);
}

void UHHPrototypeEconomyComponent::Configure(EHHPrototypeFaction InFaction, int32 InStartingCredits)
{
    Faction = InFaction;
    StartingCredits = FMath::Max(0, InStartingCredits);
    const int32 Delta = StartingCredits - Credits;
    Credits = StartingCredits;
    OnCreditsChanged.Broadcast(Credits, Delta);
}

void UHHPrototypeEconomyComponent::AddCredits(int32 Amount)
{
    if (Amount == 0)
    {
        return;
    }

    const int32 Previous = Credits;
    Credits = FMath::Max(0, Credits + Amount);
    OnCreditsChanged.Broadcast(Credits, Credits - Previous);
}

bool UHHPrototypeEconomyComponent::GetOffer(EHHPrototypeUnitRole Role, FHHPrototypeUnitOffer& OutOffer) const
{
    const FHHPrototypeUnitOffer* Offer = UnitOffers.FindByPredicate(
        [Role](const FHHPrototypeUnitOffer& Candidate) { return Candidate.Role == Role; });
    if (!Offer)
    {
        return false;
    }
    OutOffer = *Offer;
    return true;
}

bool UHHPrototypeEconomyComponent::RequestProduction(EHHPrototypeUnitRole Role)
{
    FHHPrototypeUnitOffer Offer;
    if (bIsProducing || !GetOffer(Role, Offer) || Offer.Cost < 0 || Offer.BuildTime <= 0.0f || Credits < Offer.Cost)
    {
        return false;
    }

    AddCredits(-Offer.Cost);
    QueuedRole = Role;
    QueuedCost = Offer.Cost;
    ProductionDuration = Offer.BuildTime;
    ProductionElapsed = 0.0f;
    LastBroadcastProgress = 0.0f;
    bIsProducing = true;
    OnProductionStarted.Broadcast(Role, ProductionDuration);
    OnProductionProgressed.Broadcast(0.0f);
    return true;
}

bool UHHPrototypeEconomyComponent::CancelProduction()
{
    if (!bIsProducing)
    {
        return false;
    }

    const EHHPrototypeUnitRole CancelledRole = QueuedRole;
    const int32 Refund = QueuedCost;
    bIsProducing = false;
    ProductionElapsed = 0.0f;
    ProductionDuration = 0.0f;
    QueuedCost = 0;
    AddCredits(Refund);
    OnProductionCancelled.Broadcast(CancelledRole, Refund);
    return true;
}

float UHHPrototypeEconomyComponent::GetProductionProgress() const
{
    return bIsProducing && ProductionDuration > 0.0f
        ? FMath::Clamp(ProductionElapsed / ProductionDuration, 0.0f, 1.0f)
        : 0.0f;
}

void UHHPrototypeEconomyComponent::CompleteProduction()
{
    const EHHPrototypeUnitRole CompletedRole = QueuedRole;
    bIsProducing = false;
    ProductionElapsed = ProductionDuration;
    QueuedCost = 0;
    OnProductionProgressed.Broadcast(1.0f);
    OnProductionFinished.Broadcast(Faction, CompletedRole);
}

void UHHPrototypeEconomyComponent::TickComponent(float DeltaTime, ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (SupplyIncome > 0 && SupplyInterval > 0.0f)
    {
        SupplyAccumulator += DeltaTime;
        while (SupplyAccumulator >= SupplyInterval)
        {
            SupplyAccumulator -= SupplyInterval;
            AddCredits(SupplyIncome);
        }
    }

    if (!bIsProducing)
    {
        return;
    }

    ProductionElapsed += DeltaTime;
    const float Progress = GetProductionProgress();
    // Keep HUD updates useful without broadcasting every sub-frame float change.
    if (Progress - LastBroadcastProgress >= 0.01f)
    {
        LastBroadcastProgress = Progress;
        OnProductionProgressed.Broadcast(Progress);
    }
    if (ProductionElapsed >= ProductionDuration)
    {
        CompleteProduction();
    }
}
