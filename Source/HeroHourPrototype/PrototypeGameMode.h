#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "GameFramework/HUD.h"
#include "PrototypeEconomyTypes.h"
#include "PrototypeGameMode.generated.h"

UCLASS()
class HEROHOURPROTOTYPE_API APrototypeUnit : public AActor
{
    GENERATED_BODY()
public:
    APrototypeUnit();
    virtual void Tick(float DeltaSeconds) override;
    void Configure(int32 InTeam, bool bInHQ = false);
    void ConfigureRole(EPrototypeRole InRole);
    EPrototypeRole Role = EPrototypeRole::Combat;
    EPrototypeRole GetRole() const { return Role; }
    bool IsStructure() const { return Role == EPrototypeRole::Headquarters || Role == EPrototypeRole::Depot || Role == EPrototypeRole::Factory; }
    bool IsCompleted() const { return ConstructionProgress >= 1.f; }
    float ConstructionProgress = 1.f;
    float Cargo = 0.f;
    float HarvestTimer = 0.f;
    int32 HarvestNode = INDEX_NONE;
    void Select(bool bValue);
    void ReceiveHit(float Damage);
    void OrderMove(const FVector& Position);
    void OrderAttack(APrototypeUnit* Target);
    int32 Team = 0;
    bool bHQ = false;
    bool bSelected = false;
    float Health = 100.f;
    float MaxHealth = 100.f;
    FVector Destination;
    UPROPERTY() APrototypeUnit* AttackTarget;
    UPROPERTY() APrototypeUnit* StrategicTarget = nullptr;
    bool bMoveOrdered = false;
private:
    UPROPERTY() class UStaticMeshComponent* Mesh;
    UPROPERTY() class UStaticMeshComponent* Turret;
    UPROPERTY() class UStaticMeshComponent* Barrel;
    UPROPERTY() class UStaticMeshComponent* Selection;
    UPROPERTY() class UMaterialInstanceDynamic* TeamMaterial;
    float FireCooldown = 0.f;
    float ScanCooldown = 0.f;
    float FlashTime = 0.f;
    TArray<FVector> Waypoints;
    FVector PathGoal = FVector::ZeroVector;
    int32 WaypointIndex = 0;
    float RepathCooldown = 0.f;
};

UCLASS()
class HEROHOURPROTOTYPE_API APrototypeCamera : public APawn
{
    GENERATED_BODY()
public:
    APrototypeCamera();
};

UCLASS()
class HEROHOURPROTOTYPE_API APrototypeController : public APlayerController
{
    GENERATED_BODY()
public:
    APrototypeController();
    virtual void PlayerTick(float DeltaTime) override;
    virtual void BeginPlay() override;
    void ClearSelection();
    void BeginPlacement(EPrototypeRole InRole);
    bool bPlacing = false;
    EPrototypeRole PlacementRole = EPrototypeRole::Factory;
    UPROPERTY() class AStaticMeshActor* PlacementGhost = nullptr;
    UPROPERTY() TArray<APrototypeUnit*> Selected;
    bool bDragging = false;
    FVector2D DragStart;
private:
    void AddSelected(APrototypeUnit* Unit);
};

UCLASS()
class HEROHOURPROTOTYPE_API APrototypeHUD : public AHUD
{
    GENERATED_BODY()
public:
    virtual void DrawHUD() override;
    bool HandleClick(const FVector2D& ScreenPosition);
};

UCLASS()
class HEROHOURPROTOTYPE_API APrototypeGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    APrototypeGameMode();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    void ResetMatch();
    void TrainTank();
    void ResetEconomy();
    void TickEconomy(float DeltaSeconds);
    void TickCollector(APrototypeUnit* Unit, float DeltaSeconds);
    bool BeginBuilding(EPrototypeRole Role, const FVector& Position, int32 Team = 0);
    bool CanPlaceBuilding(EPrototypeRole Role, const FVector& Position, int32 Team = 0) const;
    bool RequestProduction(EPrototypeRole Role, int32 Team = 0);
    APrototypeUnit* FindCompletedStructure(int32 Team, EPrototypeRole Role, const FVector& Near = FVector::ZeroVector) const;
    int32 GetQueueCount(const APrototypeUnit* Producer = nullptr, int32 Team = 0) const;
    float GetProductionProgress(const APrototypeUnit* Producer) const;
    static float Footprint(EPrototypeRole Role);
    static float RoleCost(EPrototypeRole Role);
    static float RoleDuration(EPrototypeRole Role);
    float EnemyCredits = 800.f;
    float BotDecisionTime = 0.f;
    TArray<FPrototypeSalvage> SalvageNodes;
    TArray<FPrototypeProduction> ProductionQueue;
    void RunEconomyTest();
    void OnHeadquartersDestroyed(int32 Team);
    APrototypeUnit* SpawnUnit(int32 Team, FVector Position, bool bHQ = false);
    UPROPERTY() APrototypeUnit* PlayerHQ;
    UPROPERTY() APrototypeUnit* EnemyHQ;
    int32 Outcome = 0;
    int32 Wave = 0;
    float MatchTime = 0.f;
    float Credits = 300.f;
    float NextWave = 30.f;
    FString Feedback;
private:
    bool bTestPassed = true;
    void TestCheck(bool bCondition, const TCHAR* Label);
    void RunCombatTest();
    void RunMatchTest();
};
