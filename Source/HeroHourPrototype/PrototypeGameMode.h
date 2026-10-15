#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "GameFramework/HUD.h"
#include "PrototypeEconomyTypes.h"
#include "RTS/HRCombat.h"
#include "RTS/HRVisibility.h"
#include "RTS/HRProductionQueue.h"
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
    bool IsStructure() const { return Role == EPrototypeRole::Headquarters || Role == EPrototypeRole::Depot || Role == EPrototypeRole::Factory || Role == EPrototypeRole::PowerPlant || Role == EPrototypeRole::Barracks || Role == EPrototypeRole::Radar || Role == EPrototypeRole::Defense; }
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
    int32 HarvestCell = INDEX_NONE;
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
    FHRCombatState CombatState;
    bool bManuallyPoweredDown = false;
    int32 BuildRotation = 0;
    bool TryActivateAbility(EHRAbility Ability);
    float ReceiveCombatHit(float RawDamage, EHRDamageType DamageType, APrototypeUnit* Attacker);
    float GetEffectiveDamage() const;
    float GetEffectiveRange() const;
    float GetEffectiveSpeed() const;
    float GetEffectiveArmor() const;
    bool IsCombatant() const;
    bool IsInCover() const;
    FVector Destination;
    UPROPERTY() APrototypeUnit* AttackTarget;
    UPROPERTY() APrototypeUnit* StrategicTarget = nullptr;
    bool bMoveOrdered = false;
private:
    UPROPERTY() class UStaticMeshComponent* Mesh;
    UPROPERTY() class USkeletalMeshComponent* InfantryMesh;
    UPROPERTY() class UAnimationAsset* InfantryIdle;
    UPROPERTY() class UAnimationAsset* InfantryWalk;
    UPROPERTY() class UAnimationAsset* InfantryFire;
    uint8 InfantryAnimationState = 255;
    UPROPERTY() class UStaticMeshComponent* Turret;
    UPROPERTY() class UStaticMeshComponent* Barrel;
    UPROPERTY() class UStaticMeshComponent* Selection;
    UPROPERTY() class UMaterialInstanceDynamic* TeamMaterial;
    FLinearColor TeamMaterialRestTint = FLinearColor::White;
    FLinearColor TeamMaterialFlashTint = FLinearColor::White;
    float FireCooldown = 0.f;
    float ScanCooldown = 0.f;
    float FlashTime = 0.f;
    TArray<FVector> Waypoints;
    FVector PathGoal = FVector::ZeroVector;
    int32 WaypointIndex = 0;
    float RepathCooldown = 0.f;
    TWeakObjectPtr<APrototypeUnit> ApproachTarget;
    FVector CachedStructureApproach = FVector::ZeroVector;
    bool bHasStructureApproach = false;
    bool UpdateStructureApproach(APrototypeUnit* Structure);
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
    int32 PlacementRotation = 0;
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
    void TickBot(float DeltaSeconds);
    int32 GetFieldCellCount(int32 FieldIndex) const;
    FVector GetFieldCellPosition(int32 FieldIndex, int32 CellIndex) const;
    bool ResetFieldAmount(int32 FieldIndex, float TotalAmount);
    double GetFieldAmount(int32 FieldIndex) const;
    bool CanPlaceOverFields(EPrototypeRole Role, const FVector& Position, int32 Team = 0) const;
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
    static float RoleCost(EPrototypeRole Role, int32 Team = 0);
    static float RoleDuration(EPrototypeRole Role, int32 Team = 0);
    static const class UHRUnitDefinition* GetRoleDefinition(EPrototypeRole Role, int32 Team = 0);
    static const class UHRTempestFieldDefinitionAsset* GetFieldDefinition();
    static const class UHRFactionDefinition* GetFactionDefinition(int32 Team);
    static int32 GetFactionIndex(int32 Team);
    const class UHRDamageMatrix* GetDamageMatrix() const;
    float GetFactionMoveMultiplier(int32 Team) const;
    float GetFactionArmorMultiplier(int32 Team) const;
    void RefreshPowerGrid();
    float GetProductionRate(int32 Team) const;
    bool IsUnitOperational(const APrototypeUnit* Unit) const;
    bool IsPowered(int32 Team) const;
    bool HasRadar(int32 Team) const;
    bool TogglePower(APrototypeUnit* Unit);
    FHRPowerGridState PowerGrid[2];
    FHRVisibilityGrid Visibility;
    void UpdateVisibility();
    UPROPERTY() TObjectPtr<class UTexture2D> FogTexture;
    UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> FogMaterial;
    UPROPERTY() TObjectPtr<class APostProcessVolume> FogVolume;
    float VisibilityTimer = 0.f;
    bool CanTeamSee(int32 Team, const AActor* Actor) const;
    void AddSmokeArea(int32 Team, const FVector& Position, float Radius, float Duration);
    FHRCommanderState Commander[2];
    void AwardCommanderPoints(int32 Team, float Points);
    bool ActivateDoctrine(EHRDoctrine Doctrine, int32 Team, const FVector& Target);
    bool SellBuilding(APrototypeUnit* Unit);
    void SpawnCombatEffect(const FVector& Position, int32 Team, bool bLarge = false);
    void SpawnWeaponEffect(const FVector& Start, const FVector& End, int32 Team, EPrototypeRole Role);
    FVector SnapBuildPosition(const FVector& Position) const;
    FIntPoint GetBuildDimensions(EPrototypeRole Role, int32 Rotation = 0, int32 Team = 0) const;
    int32 BuildingQuarterTurns = 0;
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
    UPROPERTY() TArray<TObjectPtr<class UHRUnitDefinition>> EmberDefinitions;
    UPROPERTY() TArray<TObjectPtr<class UHRFactionDefinition>> FactionDefinitions;
    UPROPERTY() TObjectPtr<class UHRDamageMatrix> DamageMatrix;
    UPROPERTY() TObjectPtr<class UHRTempestFieldDefinitionAsset> FieldDefinition;
    bool bTestPassed = true;
    void TestCheck(bool bCondition, const TCHAR* Label);
    void RunCombatTest();
    void RunUITest();
    void RunMatchTest();
};
