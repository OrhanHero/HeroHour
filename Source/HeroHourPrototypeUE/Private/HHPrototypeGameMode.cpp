#include "HHPrototypeGameMode.h"

#include "HHRTSCameraPawn.h"
#include "HHRTSPlayerController.h"
#include "PrototypeHUD.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AHHPrototypeGameMode::AHHPrototypeGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = AHHRTSCameraPawn::StaticClass();
    PlayerControllerClass = AHHRTSPlayerController::StaticClass();
    HUDClass = APrototypeHUD::StaticClass();
}

void AHHPrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();
    BuildArena();

    HeroHQ = SpawnCombatant(EHHPrototypeFaction::Hero, EHHPrototypeUnitRole::Infantry, true, FVector(-1550, 0, 180));
    USAHQ = SpawnCombatant(EHHPrototypeFaction::USA, EHHPrototypeUnitRole::Infantry, true, FVector(1550, 0, 180));
    const EHHPrototypeUnitRole Roles[] =
        { EHHPrototypeUnitRole::Infantry, EHHPrototypeUnitRole::Vehicle, EHHPrototypeUnitRole::Air };
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const float Height = Roles[Index] == EHHPrototypeUnitRole::Air ? 180.0f : 75.0f;
        HeroUnits.Add(SpawnCombatant(EHHPrototypeFaction::Hero, Roles[Index], false,
            FVector(-1100, (Index - 1) * 260, Height)));
        USAUnits.Add(SpawnCombatant(EHHPrototypeFaction::USA, Roles[Index], false,
            FVector(1100, (Index - 1) * 260, Height)));
    }
}

void AHHPrototypeGameMode::BuildArena()
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    auto SpawnBlock = [this, Cube](const FVector& Location, const FVector& Scale)
    {
        AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, FRotator::ZeroRotator);
        if (Block && Cube)
        {
            Block->GetStaticMeshComponent()->SetStaticMesh(Cube);
            Block->SetActorScale3D(Scale);
            Block->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
        }
    };

    SpawnBlock(FVector(0, 0, -50), FVector(40, 24, 1));
    SpawnBlock(FVector(0, 700, 100), FVector(3.5f, 1.2f, 3));
    SpawnBlock(FVector(0, -700, 100), FVector(3.5f, 1.2f, 3));
    SpawnBlock(FVector(0, 0, 100), FVector(1.2f, 3.0f, 3));
}

AHHPrototypeCombatActor* AHHPrototypeGameMode::SpawnCombatant(const EHHPrototypeFaction Faction,
    const EHHPrototypeUnitRole InRole, const bool bHQ, const FVector& Location)
{
    AHHPrototypeCombatActor* Actor = GetWorld()->SpawnActor<AHHPrototypeCombatActor>(Location, FRotator::ZeroRotator);
    if (Actor) Actor->Configure(Faction, InRole, bHQ);
    return Actor;
}

void AHHPrototypeGameMode::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (MatchResult != EHHPrototypeMatchResult::InProgress) return;

    if (!IsValid(HeroHQ) || !HeroHQ->IsAlive()) FinishMatch(EHHPrototypeMatchResult::USAVictory);
    else if (!IsValid(USAHQ) || !USAHQ->IsAlive()) FinishMatch(EHHPrototypeMatchResult::HeroVictory);

    USAOrderTimer -= DeltaSeconds;
    if (USAOrderTimer <= 0.0f)
    {
        IssueUSAOrder();
        bUSAStarted = true;
        USAOrderTimer = 3.0f;
    }
}

void AHHPrototypeGameMode::IssueUSAOrder()
{
    for (AHHPrototypeCombatActor* Unit : USAUnits)
    {
        if (!IsValid(Unit) || !Unit->IsAlive()) continue;
        AHHPrototypeCombatActor* Target = nullptr;
        float BestDistance = TNumericLimits<float>::Max();
        for (AHHPrototypeCombatActor* Candidate : HeroUnits)
        {
            if (IsValid(Candidate) && Candidate->IsAlive())
            {
                const float Distance = FVector::DistSquared2D(Unit->GetActorLocation(), Candidate->GetActorLocation());
                if (Distance < BestDistance) { BestDistance = Distance; Target = Candidate; }
            }
        }
        Unit->SetAttackTarget(Target ? Target : HeroHQ.Get());
    }
}

void AHHPrototypeGameMode::FinishMatch(const EHHPrototypeMatchResult Result)
{
    MatchResult = Result;
    const FString Message = Result == EHHPrototypeMatchResult::HeroVictory
        ? TEXT("HERO VICTORY") : TEXT("USA VICTORY - DEFEAT");
    UE_LOG(LogTemp, Display, TEXT("%s"), *Message);
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Yellow, Message);
}
