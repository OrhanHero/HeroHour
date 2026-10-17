#pragma once

#include "CoreMinimal.h"

class UWorld;
class UHRUnitDefinition;
class UHRDamageMatrix;

// HeroHour-owned combat balance. These values are not extracted Tempest Rising data.
enum class EHRDamageType : uint8 { Kinetic, Explosive, Tempest };
enum class EHRArmorClass : uint8 { Infantry, Vehicle, Structure };
enum class EHRAbility : uint8 { Grenade, Smoke, Deploy };
enum class EHRDoctrine : uint8 { RepairPulse, PrecisionStrike, Scorch, Overdrive };

struct HEROHOURPROTOTYPE_API FHRCombatModifiers
{
    float Damage = 1.f;
    float Range = 1.f;
    float Armor = 1.f;
};

/** Simulation state only: no timers or wall-clock queries, so pause freezes everything. */
struct HEROHOURPROTOTYPE_API FHRCombatState
{
    float Experience = 0.f;
    int32 Rank = 0;
    float Suppression = 0.f;
    float LastDamageAgo = 1000.f;
    float GrenadeCooldown = 0.f;
    float SmokeCooldown = 0.f;
    float DeployCooldown = 0.f;
    float OverdriveRemaining = 0.f;
    bool bDeployed = false;

    void Advance(float DeltaSeconds);
    void AddExperience(float ActualDamage, bool bKilled);
    void AddSuppression(float ActualDamage, float MaxHealth);
    bool IsSuppressed() const { return Suppression >= 40.f; }
    FHRCombatModifiers GetVeterancyModifiers(const UHRUnitDefinition* Definition = nullptr) const;
    float GetCooldown(EHRAbility Ability) const;
    bool CanActivate(EHRAbility Ability, bool bSupported, bool bOperational, bool bHasTarget) const;
    bool CommitAbility(EHRAbility Ability, bool bSupported, bool bOperational, bool bHasTarget);
};

struct HEROHOURPROTOTYPE_API FHRDamageTable
{
    static float GetMultiplier(EHRDamageType Damage, EHRArmorClass Armor,
        const UHRDamageMatrix* AuthoredMatrix = nullptr);
    static float Resolve(float RawDamage, EHRDamageType Damage, EHRArmorClass Armor,
        float ArmorFactor, bool bInCover, const UHRDamageMatrix* AuthoredMatrix = nullptr);
};

/** Per-team doctrine wallet. The caller validates the target before committing. */
struct HEROHOURPROTOTYPE_API FHRCommanderState
{
    float Points = 0.f;
    float GlobalCooldown = 0.f;
    void Advance(float DeltaSeconds);
    void Earn(float Amount);
    bool CanActivate(EHRDoctrine Doctrine, int32 Team, bool bHasValidTarget) const;
    bool TryActivate(EHRDoctrine Doctrine, int32 Team, bool bHasValidTarget);
    static float Cost(EHRDoctrine Doctrine);
    static bool BelongsToTeam(EHRDoctrine Doctrine, int32 Team);
};

HEROHOURPROTOTYPE_API bool RunHRCombatSelfTests(UWorld* World);
