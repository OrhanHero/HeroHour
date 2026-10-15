#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "HRRTSDefinitions.generated.h"

class AActor;
class UStaticMesh;
class UTexture2D;

/** Values use Unreal centimetres and seconds; damage/armour categories are tags. */
USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRUnitStats
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="1.0", UIMin="1.0"))
    float Health = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement", meta=(ClampMin="0.0", UIMin="0.0", Units="cm/s"))
    float MoveSpeedCm = 450.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Vision", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
    float SightRangeCm = 1600.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="0.0", UIMin="0.0"))
    float WeaponDamage = 20.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
    float WeaponRangeCm = 1200.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat", meta=(ClampMin="0.01", UIMin="0.01", Units="s"))
    float AttackInterval = .8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    FGameplayTag ArmorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat")
    FGameplayTag DamageType;
};

/** Cumulative XP required for a level, and its explicitly authored modifiers. */
USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRVeterancyLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Veterancy", meta=(ClampMin="0", UIMin="0"))
    int32 XPThreshold = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Veterancy", meta=(ClampMin="0.0", UIMin="0.0"))
    float DamageMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Veterancy", meta=(ClampMin="0.0", UIMin="0.0"))
    float RangeMultiplier = 1.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Veterancy", meta=(ClampMin="0.0", UIMin="0.0"))
    float ArmorMultiplier = 1.f;
};

/** One production definition can describe either a mobile unit or a structure. */
UCLASS(BlueprintType)
class HEROHOURPROTOTYPE_API UHRUnitDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
    FGameplayTag UnitId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
    FGameplayTag FactionId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Production")
    FGameplayTag ProductionType;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
    FHRUnitStats Stats;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Production", meta=(ClampMin="0", UIMin="0"))
    int32 CreditCost = 100;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Production", meta=(ClampMin="0.01", UIMin="0.01", Units="s"))
    float ProductionSeconds = 5.f;

    /** Required capability/technology tags; evaluation belongs to the production system. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Production")
    FGameplayTagContainer Prerequisites;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Abilities")
    FGameplayTagContainer Abilities;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Veterancy")
    TArray<FHRVeterancyLevel> VeterancyLevels;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Presentation")
    TSoftObjectPtr<UStaticMesh> StaticMesh;
};

/** Foundation asset schema; the current prototype does not automatically consume it. */
UCLASS(BlueprintType)
class HEROHOURPROTOTYPE_API UHRBuildingDefinition : public UHRUnitDefinition
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement", meta=(ClampMin="1", UIMin="1"))
    FIntPoint FootprintCells = FIntPoint(4, 4);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Placement", meta=(ClampMin="0.0", UIMin="0.0", Units="cm"))
    float ConnectedBuildRadiusCm = 2300.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Power", meta=(ClampMin="0", UIMin="0"))
    int32 PowerSupply = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Power", meta=(ClampMin="0", UIMin="0"))
    int32 PowerDemand = 0;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Production")
    FGameplayTagContainer AllowedProductionTypes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Power")
    bool bProvidesRadar = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Power")
    bool bRequiresPowerForDefense = true;
};

UENUM(BlueprintType)
enum class EHREconomyStyle : uint8
{
    FiniteSupply UMETA(DisplayName="Finite supply"),
    RegrowingField UMETA(DisplayName="Regrowing resource field")
};

UENUM(BlueprintType)
enum class EHRBuildStyle : uint8
{
    BuilderUnit UMETA(DisplayName="Construction unit"),
    SidebarPlacement UMETA(DisplayName="Production sidebar placement")
};

UCLASS(BlueprintType)
class HEROHOURPROTOTYPE_API UHRFactionDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Faction", meta=(ClampMin="0.1"))
    float MoveMultiplier = 1.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Faction", meta=(ClampMin="0.1"))
    float ArmorMultiplier = 1.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
    FGameplayTag FactionId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Identity")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Roster")
    TArray<TSoftObjectPtr<UHRUnitDefinition>> Units;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Roster")
    TArray<TSoftObjectPtr<UHRUnitDefinition>> Buildings;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Economy")
    EHREconomyStyle EconomyStyle = EHREconomyStyle::FiniteSupply;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Construction")
    EHRBuildStyle BuildStyle = EHRBuildStyle::BuilderUnit;
};

/** An exact damage/armour pair. Zero is an explicit immunity multiplier. */
USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRDamageRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    FGameplayTag DamageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage")
    FGameplayTag ArmorType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Damage", meta=(ClampMin="0.0", UIMin="0.0"))
    float Multiplier = 1.f;
};

UCLASS(BlueprintType)
class HEROHOURPROTOTYPE_API UHRDamageMatrix : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    /** First exact pair wins; absent/invalid tags or malformed multipliers use 1.0. */
    UFUNCTION(BlueprintPure, Category="RTS|Damage")
    float GetDamageMultiplier(FGameplayTag DamageType, FGameplayTag ArmorType) const;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Damage")
    TArray<FHRDamageRule> Rules;
};

/** Configuration shared by cells of a resource field; no harvesting logic is embedded. */
USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRTempestFieldDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource", meta=(ClampMin="0.0", UIMin="0.0"))
    float Capacity = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resource", meta=(ClampMin="0.0", UIMin="0.0"))
    float InitialAmount = 1000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Harvesting", meta=(ClampMin="0.0", UIMin="0.0"))
    float HarvestRate = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Regrowth", meta=(ClampMin="0.0", UIMin="0.0"))
    float RegrowthPerSecond = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Regrowth", meta=(ClampMin="0.0", UIMin="0.0", Units="s"))
    float RegrowthDelay = 10.f;

    /** Zero disables a protected seed floor; consumers must also cap this at Capacity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Regrowth", meta=(ClampMin="0.0", UIMin="0.0"))
    float SeedFloor = 0.f;
};

/** Authorable field settings; integration with a field component is explicit. */
UCLASS(BlueprintType)
class HEROHOURPROTOTYPE_API UHRTempestFieldDefinitionAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Resource")
    FHRTempestFieldDefinition Settings;
};

/** Transient per-cell state. The owning field assigns its stable index and amount. */
USTRUCT(BlueprintType)
struct HEROHOURPROTOTYPE_API FHRTempestCellState
{
    GENERATED_BODY()

    UPROPERTY(Transient, BlueprintReadOnly, Category="Resource")
    int32 CellId = INDEX_NONE;

    UPROPERTY(Transient, BlueprintReadOnly, Category="Resource")
    float Amount = 0.f;

    /** A reservation never extends a collector's lifetime. */
    UPROPERTY(Transient, BlueprintReadOnly, Category="Resource")
    TWeakObjectPtr<AActor> ReservedCollector;
};
