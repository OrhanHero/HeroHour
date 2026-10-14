#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "GameFramework/HUD.h"
#include "PrototypeEconomyTypes.h"
#include "RTS/HRRefineryDocking.h"
#include "PrototypeGameMode.generated.h"

enum class EPrototypeCollectorState : uint8 { SeekingField, Harvesting, MovingToRefinery, WaitingForDock, Unloading };

DECLARE_MULTICAST_DELEGATE_TwoParams(FPrototypeResourceDepleted, int32, bool);

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
    FVector BaseVisualScale = FVector::OneVector;
    double Cargo = 0.;
    EPrototypeCollectorState CollectorState = EPrototypeCollectorState::SeekingField;
    TWeakObjectPtr<APrototypeUnit> DeliveryDepot;
    FVector DockApproach = FVector::ZeroVector;
    uint64 UnloadOperation = 0;
    bool bRetainedUnloadRemainder = false;
    float HarvestTimer = 0.f;
    int32 HarvestNode = INDEX_NONE;
    float HarvestRouteCheck = 0.f;
    bool bHasRallyPoint = false;
    FVector RallyPoint = FVector::ZeroVector;
    void Select(bool bValue);
    void ReceiveHit(float Damage);
    void OrderMove(const FVector& Position);
    bool OrderManualMove(const FVector& Position);
    bool bCollectorManualOrder = false;
    void OrderAttack(APrototypeUnit* Target);
    int32 Team = 0;
    bool bHQ = false;
    bool bSelected = false;
    float Health = 100.f;
    float MaxHealth = 100.f;
    UPROPERTY() class UHRUnitDefinition* Definition = nullptr;
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
    APrototypeUnit* GetSelectedProducer() const;
    void FocusSelection();
    void PanToWorld(const FVector& Position);
    bool bPlacing = false;
    EPrototypeRole PlacementRole = EPrototypeRole::Factory;
    UPROPERTY() class AStaticMeshActor* PlacementGhost = nullptr;
    UPROPERTY() TArray<APrototypeUnit*> Selected;
    bool bDragging = false;
    FVector2D DragStart;
private:
    void AddSelected(APrototypeUnit* Unit);
    TMap<int32, TArray<TWeakObjectPtr<APrototypeUnit>>> ControlGroups;
    float LastSelectionClickTime = -1.f;
    TWeakObjectPtr<APrototypeUnit> LastSelectionClick;
    int32 LastRecalledGroup = INDEX_NONE;
    float LastGroupRecallTime = -1.f;
};

UCLASS()
class HEROHOURPROTOTYPE_API APrototypeHUD : public AHUD
{
    GENERATED_BODY()
public:
    // -1 follows the selected producer; explicit sidebar tabs override it.
    int32 ProductionCategory = -1;
    virtual void DrawHUD() override;
    bool HandleClick(const FVector2D& ScreenPosition);
    bool IsOverInterface(const FVector2D& ScreenPosition) const;
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
    void UpdateResourceVisuals();
    FPrototypeResourceDepleted OnResourceDepletionChanged;
    void TickCollector(APrototypeUnit* Unit, float DeltaSeconds);
    void ReleaseSalvageReservation(APrototypeUnit* Unit, bool bReleaseDock = true);
    TMap<TWeakObjectPtr<APrototypeUnit>, FHRRefineryDocking> RefineryDocks;
    bool BeginBuilding(EPrototypeRole Role, const FVector& Position, int32 Team = 0);
    bool CanPlaceBuilding(EPrototypeRole Role, const FVector& Position, int32 Team = 0) const;
    bool RequestProduction(EPrototypeRole Role, int32 Team = 0);
    bool RequestProductionAt(APrototypeUnit* Producer, EPrototypeRole Role);
    bool CancelProduction(APrototypeUnit* Producer);
    bool SetRallyPoint(APrototypeUnit* Producer, const FVector& Position);
    APrototypeUnit* FindCompletedStructure(int32 Team, EPrototypeRole Role, const FVector& Near = FVector::ZeroVector) const;
    int32 GetQueueCount(const APrototypeUnit* Producer = nullptr, int32 Team = 0) const;
    float GetProductionProgress(const APrototypeUnit* Producer) const;
    static float Footprint(EPrototypeRole Role);
    static float RoleCost(EPrototypeRole Role);
    static float RoleDuration(EPrototypeRole Role);
    static const class UHRUnitDefinition* GetRoleDefinition(EPrototypeRole Role);
    static const class UHRTempestFieldDefinitionAsset* GetFieldDefinition();
    double EnemyCredits = 800.;
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
    double Credits = 300.;
    float NextWave = 30.f;
    FString Feedback;
private:
    // Hard references on the game-mode default object keep definitions cooked and alive.
    UPROPERTY() TArray<TObjectPtr<class UHRUnitDefinition>> RoleDefinitions;
    UPROPERTY() TObjectPtr<class UHRTempestFieldDefinitionAsset> FieldDefinition;
    bool bTestPassed = true;
    void TestCheck(bool bCondition, const TCHAR* Label);
    void RunCombatTest();
    void RunUITest();
    void RunMatchTest();
};
