#include "PrototypeGameMode.h"
#include "BattlefieldLayout.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/Paths.h"
#include "TimerManager.h"
#include "UnrealClient.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMeshActor.h"

APrototypeUnit::APrototypeUnit()
{
    PrimaryActorTick.bCanEverTick = true;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Technical(TEXT("/Game/HERO/Meshes/SM_HERO_Technical.SM_HERO_Technical"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Prototype/M_Team.M_Team"));
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Hull"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetStaticMesh(Technical.Succeeded() ? Technical.Object : Cube.Object);
    Mesh->SetRelativeScale3D(FVector::OneVector);
    Turret = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret"));
    Turret->SetupAttachment(RootComponent);
    Turret->SetStaticMesh(Cube.Object);
    Turret->SetRelativeLocation(FVector(0, 0, 40));
    Turret->SetRelativeScale3D(FVector(0.65f, 0.65f, 0.35f));
    Turret->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Turret->SetVisibility(false);
    Barrel = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Barrel"));
    Barrel->SetupAttachment(RootComponent);
    Barrel->SetStaticMesh(Cube.Object);
    Barrel->SetRelativeLocation(FVector(65, 0, 45));
    Barrel->SetRelativeScale3D(FVector(0.8f, 0.13f, 0.13f));
    Barrel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Barrel->SetVisibility(false);
    Selection = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Selection"));
    Selection->SetupAttachment(RootComponent);
    Selection->SetStaticMesh(Cube.Object);
    Selection->SetRelativeLocation(FVector(0, 0, -28));
    Selection->SetRelativeScale3D(FVector(3.1f, 2.0f, 0.025f));
    Selection->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Selection->SetVisibility(false);
    if (Material.Succeeded())
    {
        Selection->SetMaterial(0, Material.Object);
    }
}

void APrototypeUnit::Configure(int32 InTeam, bool bInHQ)
{
    Team = InTeam;
    Destination = GetActorLocation();
    ConfigureRole(bInHQ ? EPrototypeRole::Headquarters : EPrototypeRole::Combat);
}

void APrototypeUnit::ConfigureRole(EPrototypeRole InRole)
{
    Role = InRole;
    bHQ = Role == EPrototypeRole::Headquarters;
    const TCHAR* Name = TEXT("SM_HERO_Technical");
    if (bHQ) Name = TEXT("SM_HERO_BlackMarket");
    else if (Role == EPrototypeRole::Factory) Name = TEXT("SM_HERO_Workshop");
    else if (Role == EPrototypeRole::Depot) Name = TEXT("SM_HERO_Depot");
    else if (Role == EPrototypeRole::Collector) Name = TEXT("SM_HERO_Salvager");
    const FString Path = FString::Printf(TEXT("/Game/HERO/Meshes/%s.%s"), Name, Name);
    if (UStaticMesh* Model = LoadObject<UStaticMesh>(nullptr, *Path)) Mesh->SetStaticMesh(Model);
    MaxHealth = bHQ ? 800.f : Role == EPrototypeRole::Factory ? 600.f : Role == EPrototypeRole::Depot ? 400.f : 100.f;
    Health = MaxHealth;
    Mesh->SetRelativeRotation(FRotator::ZeroRotator);
    Mesh->SetRelativeScale3D(FVector::OneVector);
    if (Role == EPrototypeRole::Combat)
    {
        if (UStaticMesh* Tank = LoadObject<UStaticMesh>(nullptr,TEXT("/Game/LocalAssets/RTSModern/ATV_N1_LOD0.ATV_N1_LOD0")))
        {
            Mesh->SetStaticMesh(Tank);
            Mesh->SetRelativeRotation(FRotator(0.f,90.f,0.f));
            Mesh->SetRelativeScale3D(FVector(.65f));
        }
    }
    Selection->SetRelativeScale3D(IsStructure() ? FVector(APrototypeGameMode::Footprint(Role)/45.f, APrototypeGameMode::Footprint(Role)/45.f, .025f) : FVector(3.1f,2.f,.025f));
    for (const FName Slot : {FName(TEXT("Team")), FName(TEXT("Neon"))})
    {
        const int32 Index = Mesh->GetMaterialIndex(Slot);
        if (Index == INDEX_NONE) continue;
        UMaterialInstanceDynamic* Material = Mesh->CreateDynamicMaterialInstance(Index);
        if (Material) Material->SetVectorParameterValue(TEXT("Tint"), Team == 0 ? FLinearColor(.025f,.35f,.17f) : FLinearColor(.65f,.06f,.035f));
        if (Slot == FName(TEXT("Team"))) TeamMaterial = Material;
    }
}

void APrototypeUnit::Select(bool bValue)
{
    bSelected = bValue;
    Selection->SetVisibility(bValue);
}

void APrototypeUnit::OrderMove(const FVector& Position)
{
    if (IsStructure()) return;
    AttackTarget = nullptr;
    StrategicTarget = nullptr;
    bMoveOrdered = true;
    const FVector PreviousDestination = Destination;
    Destination = BattlefieldLayout::ClampPoint(FVector(Position.X,Position.Y,35.f));
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!It->IsStructure() || It->Health <= 0 || It->IsActorBeingDestroyed()) continue;
        FVector Offset = Destination - It->GetActorLocation();
        if (Offset.SizeSquared2D() < FMath::Square(APrototypeGameMode::Footprint(It->Role) + 90.f))
        {
            if (Offset.SizeSquared2D() < 1.f) Offset = GetActorLocation() - It->GetActorLocation();
            if (Offset.SizeSquared2D() < 1.f) Offset = FVector(0.f, 1.f, 0.f);
            const FVector Outside = It->GetActorLocation() + Offset.GetSafeNormal2D() * (APrototypeGameMode::Footprint(It->Role) + 90.f);
            Destination = BattlefieldLayout::ClampPoint(FVector(Outside.X,Outside.Y,35.f));
        }
    }
    if (FVector::DistSquared2D(PreviousDestination,Destination)>100.f)
    {
        Waypoints.Empty(); WaypointIndex=0; RepathCooldown=0.f;
    }
    ScanCooldown = 0.6f;
}

void APrototypeUnit::OrderAttack(APrototypeUnit* Target)
{
    if (Role != EPrototypeRole::Combat) return;
    if (IsValid(Target) && !Target->IsActorBeingDestroyed() && Target->Team != Team && Target->Health > 0)
    {
        bMoveOrdered = false;
        AttackTarget = Target;
        StrategicTarget = Target->bHQ ? Target : nullptr;
        ScanCooldown = 0.f;
    }
}

void APrototypeUnit::ReceiveHit(float Damage)
{
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if (!Mode || Mode->Outcome != 0 || Damage <= 0 || Health <= 0) return;
    Health = FMath::Max(0.f, Health - Damage);
    FlashTime = 0.12f;
    if (Health <= 0)
    {
        if (bHQ) Mode->OnHeadquartersDestroyed(Team);
        DrawDebugSphere(GetWorld(), GetActorLocation(), bHQ ? 220.f : 75.f, 12, FColor::Orange, false, 0.5f, 0, 4.f);
        Destroy();
    }
}

void APrototypeUnit::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if (!Mode || Mode->Outcome != 0) return;
    if (FlashTime > 0)
    {
        FlashTime -= DeltaSeconds;
        if (TeamMaterial) TeamMaterial->SetVectorParameterValue(TEXT("Tint"), FlashTime > 0 ? FLinearColor::White :
            (Team == 0 ? FLinearColor(0.025f, 0.35f, 0.17f) : FLinearColor(0.65f, 0.06f, 0.035f)));
    }
    if (IsStructure())
    {
        Mesh->SetRelativeScale3D(FVector(1.f,1.f,.15f+.85f*ConstructionProgress));
        return;
    }
    if (Role == EPrototypeRole::Collector) Mode->TickCollector(this, DeltaSeconds);
    FireCooldown -= DeltaSeconds;
    ScanCooldown -= DeltaSeconds;
    if (!IsValid(AttackTarget) || AttackTarget->IsActorBeingDestroyed() || AttackTarget->Health <= 0) AttackTarget = nullptr;
    if (!IsValid(StrategicTarget) || StrategicTarget->IsActorBeingDestroyed() || StrategicTarget->Health <= 0) StrategicTarget = nullptr;
    if (!AttackTarget && StrategicTarget) AttackTarget = StrategicTarget;
    if (Role == EPrototypeRole::Combat && ScanCooldown <= 0)
    {
        ScanCooldown = 0.35f;
        if (!bMoveOrdered && (!AttackTarget || AttackTarget->bHQ))
        {
            float Nearest = FMath::Square(650.f);
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            {
                if (It->IsActorBeingDestroyed() || It->Team == Team || It->Health <= 0) continue;
                if (StrategicTarget && It->bHQ) continue;
                float Distance = FVector::DistSquared2D(GetActorLocation(), It->GetActorLocation());
                if (Distance < Nearest) { Nearest = Distance; AttackTarget = *It; }
            }
        }
    }
    if (AttackTarget)
    {
        const FVector Delta = AttackTarget->GetActorLocation() - GetActorLocation();
        const float Range = AttackTarget->bHQ ? 620.f : 480.f;
        SetActorRotation(FRotator(0, Delta.Rotation().Yaw, 0));
        if (Delta.SizeSquared2D() <= FMath::Square(Range))
        {
            if (FireCooldown <= 0)
            {
                FireCooldown = 1.f;
                DrawDebugLine(GetWorld(), GetActorLocation() + FVector(0, 0, 45),
                    AttackTarget->GetActorLocation() + FVector(0, 0, 45), FColor::Yellow, false, 0.12f, 0, 4.f);
                AttackTarget->ReceiveHit(20.f);
            }
            return;
        }
        const FVector Approach = GetActorLocation()-AttackTarget->GetActorLocation();
        Destination = BattlefieldLayout::ClampPoint(AttackTarget->GetActorLocation()+Approach.GetSafeNormal2D()*Range*.88f);
        Destination.Z = 35.f;
    }
    RepathCooldown -= DeltaSeconds;
    if (FVector::DistSquared2D(GetActorLocation(),Destination)<=25.f) { bMoveOrdered=false; return; }
    if (FVector::DistSquared2D(PathGoal,Destination)>FMath::Square(100.f)
        || (WaypointIndex>=Waypoints.Num() && RepathCooldown<=0.f))
    {
        PathGoal=Destination; WaypointIndex=0; RepathCooldown=1.f;
        FPrototypeNavigation::FindWorldPath(GetWorld(),GetActorLocation(),Destination,Waypoints);
    }
    // Intermediate waypoints allow gentle turns; the final goal must be reached precisely.
    // A 30-unit final tolerance could stop a collector just outside the depot's delivery radius.
    while (Waypoints.IsValidIndex(WaypointIndex) && FVector::DistSquared2D(GetActorLocation(),Waypoints[WaypointIndex])
        < FMath::Square(WaypointIndex == Waypoints.Num() - 1 ? 5.f : 30.f)) ++WaypointIndex;
    if (!Waypoints.IsValidIndex(WaypointIndex)) return;
    const FVector Delta=Waypoints[WaypointIndex]-GetActorLocation();
    const FVector Direction=Delta.GetSafeNormal2D();
    const FVector Step=Direction*FMath::Min(220.f*DeltaSeconds,Delta.Size2D());
    const FVector Location=GetActorLocation()+Step;
    if (FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),GetActorLocation(),Location))
    {
        SetActorLocation(Location);
        if (!AttackTarget) SetActorRotation(FRotator(0,Direction.Rotation().Yaw,0));
    }
    else { Waypoints.Empty(); WaypointIndex=0; RepathCooldown=.2f; }
}

APrototypeCamera::APrototypeCamera()
{
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("CameraRoot")));
    auto* Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(RootComponent);
    Camera->SetRelativeRotation(FRotator(-52.f, 45.f, 0.f));
}

APrototypeController::APrototypeController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
}

void APrototypeController::BeginPlay()
{
    Super::BeginPlay();
    SetInputMode(FInputModeGameOnly());
    if (GetPawn()) GetPawn()->SetActorLocation(FVector(-13000.f,-11000.f,18000.f));
    SetViewTarget(GetPawn());
}

void APrototypeController::ClearSelection()
{
    for (auto* Unit : Selected) if (IsValid(Unit)) Unit->Select(false);
    Selected.Empty();
    bDragging = false;
    bPlacing = false;
    if (IsValid(PlacementGhost)) PlacementGhost->Destroy();
    PlacementGhost = nullptr;
}

void APrototypeController::BeginPlacement(EPrototypeRole InRole)
{
    ClearSelection();
    PlacementRole = InRole;
    bPlacing = true;
    PlacementGhost = GetWorld()->SpawnActor<AStaticMeshActor>();
    if (PlacementGhost)
    {
        auto* Component = PlacementGhost->GetStaticMeshComponent();
        Component->SetMobility(EComponentMobility::Movable);
        const TCHAR* Name = InRole == EPrototypeRole::Factory ? TEXT("SM_HERO_Workshop") : TEXT("SM_HERO_Depot");
        Component->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,*FString::Printf(TEXT("/Game/HERO/Meshes/%s.%s"),Name,Name)));
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetCastShadow(false);
        Component->SetVisibility(false);
    }
}

void APrototypeController::AddSelected(APrototypeUnit* Unit)
{
    if (IsValid(Unit) && Unit->Team == 0 && !Unit->bHQ && Unit->Health > 0)
    {
        Selected.AddUnique(Unit);
        Unit->Select(true);
    }
}

void APrototypeController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);
    auto* Mode = GetWorld()->GetAuthGameMode<APrototypeGameMode>();
    if (!Mode) return;
    if (Mode->Outcome != 0 && bPlacing) ClearSelection();
    Selected.RemoveAll([](APrototypeUnit* Unit) { return !IsValid(Unit); });
    if (WasInputKeyJustPressed(EKeys::R)) { Mode->ResetMatch(); return; }
    if (APawn* Camera = GetPawn())
    {
        const UCameraComponent* View = Camera->FindComponentByClass<UCameraComponent>();
        const FRotator Yaw(0.f, View ? View->GetComponentRotation().Yaw : Camera->GetActorRotation().Yaw, 0.f);
        const FVector Direction = FRotationMatrix(Yaw).GetUnitAxis(EAxis::X) * (IsInputKeyDown(EKeys::W) - IsInputKeyDown(EKeys::S))
            + FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y) * (IsInputKeyDown(EKeys::D) - IsInputKeyDown(EKeys::A));
        FVector Location = Camera->GetActorLocation() + Direction.GetClampedToMaxSize(1) * 4500.f * DeltaTime;
        if (WasInputKeyJustPressed(EKeys::MouseScrollUp)) Location.Z -= 900.f;
        if (WasInputKeyJustPressed(EKeys::MouseScrollDown)) Location.Z += 900.f;
        Location.Z = FMath::Clamp(Location.Z,1600.f,28000.f);
        Location.X = FMath::Clamp(Location.X,-22000.f,15000.f);
        Location.Y = FMath::Clamp(Location.Y,-18000.f,12000.f);
        Camera->SetActorLocation(Location);
    }
    if (Mode->Outcome != 0) return;
    if (WasInputKeyJustPressed(EKeys::P)) Mode->TrainTank();
    if (WasInputKeyJustPressed(EKeys::C)) Mode->RequestProduction(EPrototypeRole::Collector);
    if (WasInputKeyJustPressed(EKeys::B)) BeginPlacement(EPrototypeRole::Factory);
    if (WasInputKeyJustPressed(EKeys::V)) BeginPlacement(EPrototypeRole::Depot);
    if (WasInputKeyJustPressed(EKeys::Escape)) ClearSelection();
    if (WasInputKeyJustPressed(EKeys::SpaceBar))
    {
        ClearSelection();
        for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) if (It->Role == EPrototypeRole::Combat) AddSelected(*It);
    }
    FVector2D Mouse;
    if (!GetMousePosition(Mouse.X, Mouse.Y)) return;
    if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
        if (auto* HUD = Cast<APrototypeHUD>(GetHUD())) if (HUD->HandleClick(Mouse)) return;
    if (bPlacing)
    {
        if (WasInputKeyJustPressed(EKeys::RightMouseButton)) { ClearSelection(); return; }
        FVector Origin, Direction;
        if (DeprojectMousePositionToWorld(Origin, Direction) && Direction.Z < -.001f)
        {
            FVector Position = Origin + Direction * ((35.f-Origin.Z)/Direction.Z);
            const float Radius = APrototypeGameMode::Footprint(PlacementRole);
            const bool Valid = Mode->CanPlaceBuilding(PlacementRole, Position) && Mode->Credits >= APrototypeGameMode::RoleCost(PlacementRole);
            if (IsValid(PlacementGhost))
            {
                PlacementGhost->SetActorLocation(Position);
                PlacementGhost->GetStaticMeshComponent()->SetVisibility(true);
            }
            DrawDebugBox(GetWorld(), Position, FVector(Radius,Radius,30), Valid ? FColor::Green : FColor::Red, false, 0.f, 0, 5.f);
            if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
                if (Mode->BeginBuilding(PlacementRole,Position)) ClearSelection();
        }
        return;
    }
    int32 ViewWidth = 0, ViewHeight = 0;
    GetViewportSize(ViewWidth, ViewHeight);
    const float HUDScale = FMath::Min(ViewWidth / 1440.f, ViewHeight / 900.f);
    if (Mouse.Y < 110.f * HUDScale || Mouse.Y > ViewHeight - 216.f * HUDScale)
    {
        if (WasInputKeyJustReleased(EKeys::LeftMouseButton)) bDragging = false;
        return;
    }
    if (WasInputKeyJustPressed(EKeys::LeftMouseButton))
    {
        if (!IsInputKeyDown(EKeys::LeftShift) && !IsInputKeyDown(EKeys::RightShift)) ClearSelection();
        DragStart = Mouse;
        bDragging = true;
    }
    if (WasInputKeyJustReleased(EKeys::LeftMouseButton) && bDragging)
    {
        bDragging = false;
        if (FVector2D::Distance(Mouse, DragStart) < 8.f)
        {
            FHitResult Hit;
            GetHitResultUnderCursor(ECC_Visibility, false, Hit);
            AddSelected(Cast<APrototypeUnit>(Hit.GetActor()));
        }
        else
        {
            FVector2D Min(FMath::Min(Mouse.X, DragStart.X), FMath::Min(Mouse.Y, DragStart.Y));
            FVector2D Max(FMath::Max(Mouse.X, DragStart.X), FMath::Max(Mouse.Y, DragStart.Y));
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            {
                FVector2D Point;
                if (ProjectWorldLocationToScreen(It->GetActorLocation(), Point)
                    && Point.X >= Min.X && Point.X <= Max.X && Point.Y >= Min.Y && Point.Y <= Max.Y) AddSelected(*It);
            }
        }
    }
    if (WasInputKeyJustPressed(EKeys::RightMouseButton))
    {
        FHitResult Hit;
        if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            auto* Target = Cast<APrototypeUnit>(Hit.GetActor());
            int32 Index = 0;
            const int32 Columns = FMath::Max(1, FMath::CeilToInt(FMath::Sqrt(static_cast<float>(Selected.Num()))));
            for (auto* Unit : Selected)
            {
                if (Target && Target->Team != 0) Unit->OrderAttack(Target);
                else Unit->OrderMove(Hit.ImpactPoint + FVector((Index / Columns) * 150.f, (Index % Columns - (Columns - 1) * 0.5f) * 150.f, 0));
                ++Index;
            }
            DrawDebugSphere(GetWorld(), Hit.ImpactPoint + FVector(0, 0, 10), 40, 12, FColor::Green, false, 0.8f, 0, 3.f);
        }
    }
}

APrototypeGameMode::APrototypeGameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultPawnClass = APrototypeCamera::StaticClass();
    PlayerControllerClass = APrototypeController::StaticClass();
    HUDClass = APrototypeHUD::StaticClass();
}

APrototypeUnit* APrototypeGameMode::SpawnUnit(int32 Team, FVector Position, bool bHQ)
{
    FActorSpawnParameters Parameters;
    Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    auto* Unit = GetWorld()->SpawnActor<APrototypeUnit>(Position, FRotator(0, Team == 0 ? 0 : 180, 0), Parameters);
    if (Unit) Unit->Configure(Team, bHQ);
    return Unit;
}

void APrototypeGameMode::ResetMatch()
{
    if (auto* Controller = Cast<APrototypeController>(GetWorld()->GetFirstPlayerController()))
    {
        Controller->ClearSelection();
        if (Controller->GetPawn()) Controller->GetPawn()->SetActorLocation(FVector(-13000,-11000,18000));
    }
    TArray<APrototypeUnit*> Old;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) Old.Add(*It);
    for (auto* Unit : Old) Unit->Destroy();
    Outcome = 0;
    Wave = 0;
    MatchTime = 0;
    Feedback.Empty();
    ResetEconomy();
    UE_LOG(LogTemp, Display, TEXT("HEROHOUR_MATCH_RESET"));
}

void APrototypeGameMode::TrainTank()
{
    RequestProduction(EPrototypeRole::Combat);
}

void APrototypeGameMode::OnHeadquartersDestroyed(int32 Team)
{
    if (Outcome != 0) return;
    Outcome = Team == 1 ? 1 : -1;
    UE_LOG(LogTemp, Display, TEXT("HEROHOUR_MATCH_RESULT=%d"), Outcome);
}

void APrototypeGameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    TickEconomy(DeltaSeconds);
}

void APrototypeGameMode::TestCheck(bool bCondition, const TCHAR* Label)
{
    bTestPassed &= bCondition;
    UE_LOG(LogTemp, Display, TEXT("COMBAT_TEST %s: %s"), Label, bCondition ? TEXT("PASS") : TEXT("FAIL"));
}

void APrototypeGameMode::RunCombatTest()
{
    auto* Friendly = SpawnUnit(0, FVector(-6000,0,35));
    auto* Enemy = SpawnUnit(1, FVector(-5700,0,35));
    Friendly->OrderAttack(Enemy);
    FTimerHandle TestTimer;
    GetWorldTimerManager().SetTimer(TestTimer, [this, Friendly, Enemy]()
    {
        TestCheck(!IsValid(Enemy) || Enemy->Health < 100, TEXT("real ranged attack deals damage"));
        TestCheck(!IsValid(Friendly) || Friendly->Health < 100, TEXT("enemy acquires and attacks player"));
        // Isolated tactical fixtures use real unit ticks without waiting for a wave or random production.
        if (IsValid(Friendly)) Friendly->Destroy();
        if (IsValid(Enemy)) Enemy->Destroy();
        // Remove the starting armies too: a second nearby defender must legitimately
        // interrupt the HQ order, so it cannot remain in this isolated return-to-HQ test.
        TArray<APrototypeUnit*> StartingArmy;
        for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            if (!It->bHQ) StartingArmy.Add(*It);
        for (APrototypeUnit* Unit : StartingArmy) Unit->Destroy();
        auto* Retreating = SpawnUnit(0, FVector(-6000,450,35));
        auto* NearbyEnemy = SpawnUnit(1, FVector(-5700,450,35));
        if (Retreating && NearbyEnemy)
        {
            Retreating->SetActorTickEnabled(false);
            NearbyEnemy->SetActorTickEnabled(false);
            Retreating->OrderMove(FVector(-6500,450,35));
            for (int32 Step = 0; Step < 3; ++Step) Retreating->Tick(0.35f);
            TestCheck(Retreating->bMoveOrdered && !IsValid(Retreating->AttackTarget)
                && Retreating->GetActorLocation().X < -6150.f, TEXT("retreat remains protected across hostile acquisition scans"));
            Retreating->OrderMove(PlayerHQ->GetActorLocation());
            TestCheck(FVector::DistSquared2D(Retreating->Destination, PlayerHQ->GetActorLocation()) >= FMath::Square(439.f)
                && BattlefieldLayout::InBounds(Retreating->Destination), TEXT("move destination avoids HQ footprint inside road corridor"));
            Retreating->Destroy();
            NearbyEnemy->Destroy();
        }
        else TestCheck(false, TEXT("retreat fixtures spawn"));
        auto* Attacker = SpawnUnit(1, FVector(-6100,-450,35));
        auto* Defender = SpawnUnit(0, FVector(-5800,-450,35));
        if (Attacker && Defender)
        {
            Attacker->SetActorTickEnabled(false);
            Defender->SetActorTickEnabled(false);
            Attacker->OrderAttack(PlayerHQ);
            Attacker->Tick(0.4f);
            TestCheck(Attacker->AttackTarget == Defender && Attacker->StrategicTarget == PlayerHQ
                && Defender->Health < Defender->MaxHealth, TEXT("HQ attacker engages nearby defender with real damage"));
            Defender->Destroy();
            Attacker->Tick(0.4f);
            TestCheck(Attacker->AttackTarget == PlayerHQ && Attacker->StrategicTarget == PlayerHQ,
                TEXT("HQ attacker resumes strategic objective after defender destruction"));
            Attacker->Destroy();
        }
        else TestCheck(false, TEXT("strategic attack fixtures spawn"));
        // Combat fixtures provide a completed factory; the live player must build their own.
        auto* TestFactory = SpawnUnit(0, FVector(-7400,-1600,35));
        if (TestFactory) TestFactory->ConfigureRole(EPrototypeRole::Factory);
        TestCheck(TestFactory && TestFactory->IsCompleted(), TEXT("combat production fixture factory ready"));
        Credits = 99;
        TrainTank();
        TestCheck(Credits == 99, TEXT("insufficient credits rejected"));
        Credits = 100;
        TrainTank();
        TestCheck(Credits == 0 && GetQueueCount(TestFactory) == 1, TEXT("reinforcement queue charges 100"));
        EnemyHQ->ReceiveHit(800);
        TestCheck(Outcome == 1, TEXT("enemy HQ destruction wins"));
        float Before = PlayerHQ->Health;
        PlayerHQ->ReceiveHit(100);
        TestCheck(PlayerHQ->Health == Before, TEXT("combat stops after match"));
        auto* TestController = Cast<APrototypeController>(GetWorld()->GetFirstPlayerController());
        if (TestController)
        {
            TestController->Selected.Add(PlayerHQ);
            PlayerHQ->Select(true);
            TestController->bDragging = true;
        }
        ResetMatch();
        TestCheck(TestController && TestController->Selected.IsEmpty() && !TestController->bDragging,
            TEXT("restart clears selection and drag state"));
        TestCheck(Outcome == 0 && Credits == 800 && Wave == 0 && MatchTime == 0, TEXT("restart resets match state"));
        int32 Count = 0;
        for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It) ++Count;
        TestCheck(Count == 13, TEXT("restart removes old army"));
        PlayerHQ->ReceiveHit(800);
        TestCheck(Outcome == -1, TEXT("player HQ destruction loses"));
        UE_LOG(LogTemp, Display, TEXT("COMBAT_TEST_RESULT=%s"), bTestPassed ? TEXT("PASS") : TEXT("FAIL"));
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
    }, 2.5f, false);
}

void APrototypeGameMode::RunMatchTest()
{
    bool AllPassed = true;
    int32 Wins = 0;
    for (int32 Round = 0; Round < 3; ++Round)
    {
        ResetMatch();
        const bool Built = BeginBuilding(EPrototypeRole::Factory,FVector(-7400,-1600,35));
        bool PathsSafe = Built;
        float Delivered = 0.f;
        int32 Steps = 0;
        for (; Steps < 6000 && Outcome == 0; ++Steps)
        {
            TickEconomy(.1f);
            int32 CombatCount = 0;
            TArray<APrototypeUnit*> Snapshot;
            for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
            {
                if (It->IsActorBeingDestroyed() || It->Health <= 0.f) continue;
                Snapshot.Add(*It);
                if (It->Team == 0 && It->Role == EPrototypeRole::Combat) ++CombatCount;
            }
            APrototypeUnit* Factory = FindCompletedStructure(0,EPrototypeRole::Factory);
            if (Factory && GetQueueCount(Factory) < 3 && CombatCount + GetQueueCount(Factory) < 20)
                RequestProduction(EPrototypeRole::Combat);
            for (APrototypeUnit* Unit : Snapshot)
            {
                if (!IsValid(Unit) || Unit->IsActorBeingDestroyed() || Unit->Health <= 0.f || Outcome != 0) continue;
                if (Unit->Team == 0 && Unit->Role == EPrototypeRole::Combat
                    && (CombatCount >= 8 || MatchTime >= 100.f) && !IsValid(Unit->StrategicTarget)) Unit->OrderAttack(EnemyHQ);
                const FVector Previous = Unit->GetActorLocation();
                const float BeforeCredits = Credits;
                Unit->Tick(.1f);
                if (Unit->Team == 0 && Unit->Role == EPrototypeRole::Collector) Delivered += FMath::Max(0.f,Credits-BeforeCredits);
                if (!Unit->IsStructure() && FVector::DistSquared2D(Previous,Unit->GetActorLocation()) > 0.01f)
                    PathsSafe &= FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Previous,Unit->GetActorLocation());
            }
        }
        Wins += Outcome == 1 ? 1 : 0;
        const bool RoundPassed = Built && PathsSafe && Delivered > 800.f && Outcome != 0;
        AllPassed &= RoundPassed;
        UE_LOG(LogTemp,Display,TEXT("MATCH_TEST round=%d outcome=%d simulation=%.1fs delivered=%.0f paths=%s result=%s"),
            Round+1,Outcome,MatchTime,Delivered,PathsSafe ? TEXT("SAFE") : TEXT("UNSAFE"),RoundPassed ? TEXT("PASS") : TEXT("FAIL"));
    }
    AllPassed &= Wins >= 1;
    UE_LOG(LogTemp,Display,TEXT("MATCH_TEST_RESULT=%s wins=%d/3"),AllPassed ? TEXT("PASS") : TEXT("FAIL"),Wins);
    ResetMatch();
    if (auto* Controller = GetWorld()->GetFirstPlayerController()) Controller->ConsoleCommand(TEXT("quit"));
}

void APrototypeGameMode::BeginPlay()
{
    Super::BeginPlay();
    ResetMatch();
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeCombatTest"))) RunCombatTest();
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeEconomyTest"))) RunEconomyTest();
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeMatchTest"))) RunMatchTest();
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeNavigationTest")))
    {
        bool Pass=FPrototypeNavigation::RunSelfTests();
        APrototypeUnit* Traveler = SpawnUnit(0, FVector(-8000,-2500,35));
        const FVector DestinationPoint(-1000,-2500,35);
        bool Safe = Traveler != nullptr;
        int32 Steps = 0;
        if (Traveler)
        {
            Traveler->SetActorTickEnabled(false);
            Traveler->OrderMove(DestinationPoint);
            FVector Previous = Traveler->GetActorLocation();
            for (; Steps < 5000 && FVector::DistSquared2D(Traveler->GetActorLocation(),DestinationPoint)>FMath::Square(10.f); ++Steps)
            {
                Traveler->Tick(.05f);
                Safe &= FPrototypeNavigation::SegmentWorldWalkable(GetWorld(),Previous,Traveler->GetActorLocation());
                Previous = Traveler->GetActorLocation();
            }
            Safe &= Steps < 5000 && FVector::DistSquared2D(Traveler->GetActorLocation(),DestinationPoint)<=FMath::Square(10.f);
            UE_LOG(LogTemp,Display,TEXT("NAVIGATION_TEST actual unit reaches opposite bank without crossing water: %s (steps=%d, location=%s)"),
                Safe ? TEXT("PASS") : TEXT("FAIL"),Steps,*Traveler->GetActorLocation().ToString());
            Traveler->Destroy();
        }
        Pass &= Safe;
        UE_LOG(LogTemp,Display,TEXT("NAVIGATION_TEST_RESULT=%s"),Pass ? TEXT("PASS") : TEXT("FAIL"));
        GetWorld()->GetFirstPlayerController()->ConsoleCommand(TEXT("quit"));
    }
    if (FParse::Param(FCommandLine::Get(), TEXT("PrototypeCapture")))
    {
        FTimerHandle CaptureTimer;
        GetWorldTimerManager().SetTimer(CaptureTimer, []()
        {
            FScreenshotRequest::RequestScreenshot(FPaths::ProjectSavedDir() / TEXT("Screenshots/Battle.png"), true, false);
        }, 8.f, false);
    }
}
