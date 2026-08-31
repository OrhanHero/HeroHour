#include "HHPrototypeGameMode.h"

#include "HHRTSCameraPawn.h"
#include "HHRTSPlayerController.h"
#include "HHPrototypeEconomyComponent.h"
#include "HHPrototypeFogOfWarActor.h"
#include "PrototypeHUD.h"
#include "Engine/Engine.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AHHPrototypeGameMode::AHHPrototypeGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = AHHRTSCameraPawn::StaticClass();
    PlayerControllerClass = AHHRTSPlayerController::StaticClass();
    HUDClass = APrototypeHUD::StaticClass();
    HeroEconomy = CreateDefaultSubobject<UHHPrototypeEconomyComponent>(TEXT("HeroEconomy"));
    USAEconomy = CreateDefaultSubobject<UHHPrototypeEconomyComponent>(TEXT("USAEconomy"));
}

void AHHPrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();
    // CitySample supplies the finished environment. Keep the generated arena only as
    // a lightweight fallback for maps that do not contain authored world geometry.
    const FString MapName = GetWorld() ? GetWorld()->GetMapName() : FString();
    const bool bUsesCitySample = MapName.Contains(TEXT("Small_City_LVL"))
        || MapName.Contains(TEXT("Big_City_LVL"));
    if (!bUsesCitySample)
    {
        BuildArena();
    }

    HeroEconomy->Configure(EHHPrototypeFaction::Hero, 2500);
    USAEconomy->Configure(EHHPrototypeFaction::USA, 2500);
    HeroEconomy->OnProductionFinished.AddDynamic(this, &AHHPrototypeGameMode::HandleProductionFinished);
    USAEconomy->OnProductionFinished.AddDynamic(this, &AHHPrototypeGameMode::HandleProductionFinished);
    FogOfWar = GetWorld()->SpawnActor<AHHPrototypeFogOfWarActor>();

    HeroHQ = SpawnCombatant(EHHPrototypeFaction::Hero, EHHPrototypeUnitRole::Infantry, true, FVector(-3000, 0, 8));
    USAHQ = SpawnCombatant(EHHPrototypeFaction::USA, EHHPrototypeUnitRole::Infantry, true, FVector(3000, 0, 8));
    const EHHPrototypeUnitRole Roles[] =
        { EHHPrototypeUnitRole::Infantry, EHHPrototypeUnitRole::Vehicle, EHHPrototypeUnitRole::Air };
    for (int32 Index = 0; Index < 3; ++Index)
    {
        const float Height = Roles[Index] == EHHPrototypeUnitRole::Air ? 180.0f : 75.0f;
        HeroUnits.Add(SpawnCombatant(EHHPrototypeFaction::Hero, Roles[Index], false,
            FVector(-2500, (Index - 1) * 300, Height)));
        USAUnits.Add(SpawnCombatant(EHHPrototypeFaction::USA, Roles[Index], false,
            FVector(2500, (Index - 1) * 300, Height)));
    }
}

void AHHPrototypeGameMode::BuildArena()
{
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    UMaterialInterface* ColorMaterial = LoadObject<UMaterialInterface>(
        nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));

    auto SpawnBlock = [this, Cube, ColorMaterial](const FVector& Location, const FVector& Scale,
        const FLinearColor& Color, const FRotator& Rotation = FRotator::ZeroRotator, const bool bCollision = true)
    {
        AStaticMeshActor* Block = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (Block && Cube)
        {
            Block->GetStaticMeshComponent()->SetStaticMesh(Cube);
            Block->SetActorScale3D(Scale);
            Block->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
            Block->GetStaticMeshComponent()->SetCollisionEnabled(
                bCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
            if (ColorMaterial)
            {
                Block->GetStaticMeshComponent()->SetMaterial(0, ColorMaterial);
                if (UMaterialInstanceDynamic* MID = Block->GetStaticMeshComponent()->CreateAndSetMaterialInstanceDynamic(0))
                {
                    MID->SetVectorParameterValue(TEXT("Color"), Color);
                }
            }
        }
        return Block;
    };

    auto SpawnProp = [this](const TCHAR* AssetPath, const FVector& Location, const FVector& Scale,
        const FRotator& Rotation = FRotator::ZeroRotator)
    {
        UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, AssetPath);
        if (!Mesh) return static_cast<AStaticMeshActor*>(nullptr);
        AStaticMeshActor* Prop = GetWorld()->SpawnActor<AStaticMeshActor>(Location, Rotation);
        if (Prop)
        {
            Prop->GetStaticMeshComponent()->SetStaticMesh(Mesh);
            Prop->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
            Prop->SetActorScale3D(Scale);
            FVector Min, Max;
            Prop->GetStaticMeshComponent()->GetLocalBounds(Min, Max);
            Prop->GetStaticMeshComponent()->SetRelativeLocation(FVector(0, 0, -Min.Z));
        }
        return Prop;
    };

    const FLinearColor Grass(0.34f, 0.22f, 0.10f);
    const FLinearColor Dirt(0.25f, 0.20f, 0.12f);
    const FLinearColor Asphalt(0.055f, 0.065f, 0.07f);
    const FLinearColor Concrete(0.27f, 0.29f, 0.27f);
    const FLinearColor HeroBlue(0.015f, 0.12f, 0.34f);
    const FLinearColor USARed(0.34f, 0.035f, 0.018f);

    // Layered ground makes each strategic area readable from the RTS camera.
    SpawnBlock(FVector(0, 0, -50), FVector(80, 50, 1), Grass);
    SpawnBlock(FVector(0, 0, 2), FVector(80, 2.35f, 0.035f), Asphalt, FRotator::ZeroRotator, false);
    SpawnBlock(FVector(0, 0, 6), FVector(80, 0.035f, 0.015f), FLinearColor(0.72f, 0.62f, 0.18f), FRotator::ZeroRotator, false);
    SpawnBlock(FVector(-3000, 0, 3), FVector(7.0f, 8.0f, 0.05f), FLinearColor(0.10f,0.12f,0.12f), FRotator::ZeroRotator, false);
    SpawnBlock(FVector(3000, 0, 3), FVector(7.0f, 8.0f, 0.05f), Concrete, FRotator::ZeroRotator, false);
    SpawnBlock(FVector(-3470, 0, 8), FVector(0.10f, 8.0f, 0.12f), FLinearColor(0.05f,0.65f,0.36f), FRotator::ZeroRotator, false);
    SpawnBlock(FVector(3470, 0, 8), FVector(0.10f, 8.0f, 0.12f), FLinearColor(0.08f,0.42f,0.95f), FRotator::ZeroRotator, false);

    // CITY HERO layout: a turquoise river, two crossings and a dense downtown combat corridor.
    const FLinearColor Water(0.015f, 0.18f, 0.25f);
    const FLinearColor CityDark(0.055f, 0.065f, 0.075f);
    const FLinearColor CityMid(0.10f, 0.115f, 0.12f);
    const FLinearColor Sandstone(0.31f, 0.20f, 0.11f);
    SpawnBlock(FVector(-1050, 0, 0), FVector(3.8f, 50.0f, 0.05f), Water, FRotator::ZeroRotator, false);
    for (const float BridgeY : {-760.0f, 0.0f, 760.0f})
    {
        SpawnBlock(FVector(-1050, BridgeY, 26), FVector(5.2f, 1.25f, 0.22f), Asphalt);
        SpawnBlock(FVector(-1050, BridgeY-105, 52), FVector(5.2f, 0.08f, 0.42f), Concrete, FRotator::ZeroRotator, false);
        SpawnBlock(FVector(-1050, BridgeY+105, 52), FVector(5.2f, 0.08f, 0.42f), Concrete, FRotator::ZeroRotator, false);
    }

    // Downtown Core: varied high-rise silhouettes, alleys and a clear central attack avenue.
    for (int32 X=0; X<6; ++X) for (int32 Y=0; Y<5; ++Y)
    {
        const float BX=-480.0f+X*300.0f;
        const float BY=-1120.0f+Y*560.0f;
        if (FMath::Abs(BY)<210.0f) continue;
        const float Height=180.0f+((X*71+Y*43)%5)*85.0f;
        const FLinearColor Tone=((X+Y)%2)==0?CityDark:CityMid;
        SpawnBlock(FVector(BX,BY,Height*0.5f), FVector(1.05f,2.0f,Height/100.0f), Tone);
        SpawnBlock(FVector(BX,BY,Height+18), FVector(0.72f,1.52f,0.18f), Concrete, FRotator::ZeroRotator, false);
        SpawnBlock(FVector(BX-92,BY,Height*0.55f), FVector(0.035f,1.25f,Height/190.0f),
            ((X+Y)%3)==0?FLinearColor(0.02f,0.75f,0.55f):FLinearColor(0.04f,0.30f,0.42f), FRotator::ZeroRotator, false);
    }

    // West Heights: low residential compounds and palms/utility towers around the HERO approach.
    for (int32 I=0; I<10; ++I)
    {
        const float Angle=I*PI*2.0f/10.0f;
        const FVector P(-1900.0f+FMath::Cos(Angle)*560.0f, FMath::Sin(Angle)*1120.0f, 70.0f);
        SpawnBlock(P, FVector(1.45f,1.0f,0.70f), I%2?Sandstone:CityMid);
        SpawnBlock(P+FVector(0,0,82), FVector(0.9f,0.65f,0.12f), Concrete, FRotator::ZeroRotator, false);
    }

    // Refinery District: tanks, pipe racks and chimney stacks on the USA side.
    for (int32 I=0; I<8; ++I)
    {
        const float RX=1650.0f+(I%4)*310.0f;
        const float RY=(I/4==0?-1.0f:1.0f)*(690.0f+(I%4)*105.0f);
        SpawnBlock(FVector(RX,RY,80), FVector(1.15f,1.15f,0.8f), Concrete);
        SpawnBlock(FVector(RX,RY,170), FVector(0.34f,0.34f,1.1f), I%2?USARed:CityDark);
    }
    for (const float Y : {-1450.0f,1450.0f})
    {
        SpawnBlock(FVector(2150,Y,170), FVector(0.55f,0.55f,3.4f), CityDark);
        SpawnBlock(FVector(2400,Y,120), FVector(2.2f,0.34f,0.34f), Concrete);
    }

    // Central checkpoint: two protected shoulders with a deliberate fighting lane.
    SpawnBlock(FVector(0, 760, 90), FVector(3.8f, 1.15f, 1.8f), Dirt);
    SpawnBlock(FVector(0, -760, 90), FVector(3.8f, 1.15f, 1.8f), Dirt);
    SpawnBlock(FVector(0, 405, 58), FVector(0.7f, 1.25f, 0.58f), Concrete);
    SpawnBlock(FVector(0, -405, 58), FVector(0.7f, 1.25f, 0.58f), Concrete);

    // Symmetrical cover pockets keep all three unit roles visible and useful.
    const float Sides[] = {-1.0f, 1.0f};
    for (const float Side : Sides)
    {
        SpawnBlock(FVector(Side * 730, 610, 55), FVector(1.6f, 0.38f, 0.55f), Concrete, FRotator(0, Side * 16.0f, 0));
        SpawnBlock(FVector(Side * 730, -610, 55), FVector(1.6f, 0.38f, 0.55f), Concrete, FRotator(0, -Side * 16.0f, 0));
        SpawnBlock(FVector(Side * 1270, 760, 42), FVector(0.35f, 1.25f, 0.42f), Dirt);
        SpawnBlock(FVector(Side * 1270, -760, 42), FVector(0.35f, 1.25f, 0.42f), Dirt);

        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Barrier_004.Barrier_004"),
            FVector(Side * 520, 340, 4), FVector(0.32f), FRotator(0, Side < 0 ? -18.0f : 198.0f, 0));
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Barrier_006.Barrier_006"),
            FVector(Side * 520, -340, 4), FVector(0.32f), FRotator(0, Side < 0 ? 18.0f : 162.0f, 0));
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Box_003.Box_003"),
            FVector(Side * 910, 690, 4), FVector(0.36f), FRotator(0, Side * 25.0f, 0));
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Box_003.Box_003"),
            FVector(Side * 980, -690, 4), FVector(0.30f), FRotator(0, Side * -18.0f, 0));
    }

    // Base perimeters and recognizable military silhouettes, mirrored for fairness.
    for (int32 BaseSide = -1; BaseSide <= 1; BaseSide += 2)
    {
        const float X = BaseSide * 3000.0f;
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Barrier_004.Barrier_004"), FVector(X, 790, 4), FVector(0.40f));
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Barrier_004.Barrier_004"), FVector(X, -790, 4), FVector(0.40f));
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Box_003.Box_003"), FVector(X + BaseSide * 260, 560, 4), FVector(0.42f));
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Box_003.Box_003"), FVector(X + BaseSide * 260, -560, 4), FVector(0.42f));
        SpawnProp(TEXT("/Game/LocalAssets/MilitaryFree/Tower_003.Tower_003"),
            FVector(X - BaseSide * 210, 690, 4), FVector(0.18f), FRotator(0, BaseSide < 0 ? -90.0f : 90.0f, 0));
    }
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

    USAProductionTimer -= DeltaSeconds;
    if (USAEconomy && USAProductionTimer <= 0.0f && !USAEconomy->IsProducing())
    {
        const EHHPrototypeUnitRole Rotation[] = {
            EHHPrototypeUnitRole::Infantry, EHHPrototypeUnitRole::Vehicle, EHHPrototypeUnitRole::Air };
        USAEconomy->RequestProduction(Rotation[USAProductionChoice++ % 3]);
        USAProductionTimer = 2.5f;
    }

    if (FogOfWar)
    {
        for (AHHPrototypeCombatActor* Enemy : USAUnits)
        {
            if (IsValid(Enemy))
                Enemy->SetActorHiddenInGame(!FogOfWar->IsWorldLocationVisible(Enemy->GetActorLocation()));
        }
        if (IsValid(USAHQ))
            USAHQ->SetActorHiddenInGame(!FogOfWar->IsWorldLocationVisible(USAHQ->GetActorLocation()));
    }
}

bool AHHPrototypeGameMode::RequestHeroProduction(const EHHPrototypeUnitRole UnitRole)
{
    return HeroEconomy && HeroEconomy->RequestProduction(UnitRole);
}

bool AHHPrototypeGameMode::CancelHeroProduction()
{
    return HeroEconomy && HeroEconomy->CancelProduction();
}

void AHHPrototypeGameMode::HandleProductionFinished(const EHHPrototypeFaction Faction,
    const EHHPrototypeUnitRole UnitRole)
{
    const bool bHero = Faction == EHHPrototypeFaction::Hero;
    int32& Serial = bHero ? HeroProductionSerial : USAProductionSerial;
    const float Side = bHero ? -1.0f : 1.0f;
    const float Lane = ((Serial++ % 5) - 2) * 145.0f;
    const float Height = UnitRole == EHHPrototypeUnitRole::Air ? 180.0f : 75.0f;
    AHHPrototypeCombatActor* Unit = SpawnCombatant(Faction, UnitRole, false,
        FVector(Side * 2500.0f, Lane, Height));
    if (Unit)
    {
        (bHero ? HeroUnits : USAUnits).Add(Unit);
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
