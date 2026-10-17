#include "HRCombat.h"
#include "HRRTSDefinitions.h"
#include "HRRTSTags.h"

namespace
{
float ValidNonnegative(float Value)
{
    return FMath::IsFinite(Value) ? FMath::Max(0.f, Value) : 0.f;
}

FGameplayTag DamageTag(EHRDamageType Type)
{
    return Type == EHRDamageType::Explosive ? HRRTSTags::Damage_Explosive
        : Type == EHRDamageType::Tempest ? HRRTSTags::Damage_Tempest : HRRTSTags::Damage_Kinetic;
}

FGameplayTag ArmorTag(EHRArmorClass Type)
{
    return Type == EHRArmorClass::Infantry ? HRRTSTags::Armor_Infantry
        : Type == EHRArmorClass::Structure ? HRRTSTags::Armor_Structure : HRRTSTags::Armor_Vehicle;
}
}

void FHRCombatState::Advance(float DeltaSeconds)
{
    if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.f) return;
    GrenadeCooldown = FMath::Max(0.f, GrenadeCooldown - DeltaSeconds);
    SmokeCooldown = FMath::Max(0.f, SmokeCooldown - DeltaSeconds);
    DeployCooldown = FMath::Max(0.f, DeployCooldown - DeltaSeconds);
    OverdriveRemaining = FMath::Max(0.f, OverdriveRemaining - DeltaSeconds);
    // Recovery begins two simulation seconds after the most recent hit.
    const float RecoverySeconds = FMath::Max(0.f, LastDamageAgo + DeltaSeconds - 2.f)
        - FMath::Max(0.f, LastDamageAgo - 2.f);
    Suppression = FMath::Max(0.f, Suppression - RecoverySeconds * 18.f);
    LastDamageAgo = FMath::Min(1000.f, LastDamageAgo + DeltaSeconds);
}

void FHRCombatState::AddExperience(float ActualDamage, bool bKilled)
{
    if (!FMath::IsFinite(ActualDamage) || ActualDamage <= 0.f) return;
    Experience = FMath::Min(1000000.f, Experience + ActualDamage + (bKilled ? 50.f : 0.f));
    Rank = Experience >= 500.f ? 3 : Experience >= 250.f ? 2 : Experience >= 100.f ? 1 : 0;
}

#if defined(_MSC_VER)
// /fp:fast can narrow explicit double intermediates back to float. Preserve
// this threshold-sensitive calculation without changing the whole module.
#pragma float_control(precise,on,push)
#endif
void FHRCombatState::AddSuppression(float ActualDamage, float MaxHealth)
{
    if (!FMath::IsFinite(ActualDamage) || !FMath::IsFinite(MaxHealth) || ActualDamage <= 0.f || MaxHealth <= 0.f) return;
    // Keep the damage fraction in double precision until the final state write.
    // Optimized float reciprocal multiplication can turn exactly 20% damage
    // into 39.999996 suppression and incorrectly miss the >=40 threshold.
    const double Accumulated=static_cast<double>(Suppression)
        +static_cast<double>(ActualDamage)/static_cast<double>(MaxHealth)*200.0;
    Suppression=static_cast<float>(FMath::Clamp(Accumulated,0.0,100.0));
    LastDamageAgo = 0.f;
}
#if defined(_MSC_VER)
#pragma float_control(pop)
#endif

FHRCombatModifiers FHRCombatState::GetVeterancyModifiers(const UHRUnitDefinition* Definition) const
{
    FHRCombatModifiers Result;
    if (Definition && !Definition->VeterancyLevels.IsEmpty())
    {
        // Select the highest reached cumulative threshold; never multiply all levels.
        int32 BestThreshold = -1;
        for (const FHRVeterancyLevel& Level : Definition->VeterancyLevels)
        {
            if (Level.XPThreshold < 0 || Level.XPThreshold > Experience || Level.XPThreshold < BestThreshold) continue;
            if (!FMath::IsFinite(Level.DamageMultiplier) || Level.DamageMultiplier <= 0.f
                || !FMath::IsFinite(Level.RangeMultiplier) || Level.RangeMultiplier <= 0.f
                || !FMath::IsFinite(Level.ArmorMultiplier) || Level.ArmorMultiplier <= 0.f) continue;
            BestThreshold = Level.XPThreshold;
            Result = {Level.DamageMultiplier, Level.RangeMultiplier, Level.ArmorMultiplier};
        }
        return Result;
    }
    if (Rank == 1) return {1.15f, 1.08f, 1.10f};
    if (Rank == 2) return {1.30f, 1.16f, 1.20f};
    if (Rank >= 3) return {1.50f, 1.25f, 1.35f};
    return Result;
}

float FHRCombatState::GetCooldown(EHRAbility Ability) const
{
    return Ability == EHRAbility::Grenade ? GrenadeCooldown
        : Ability == EHRAbility::Smoke ? SmokeCooldown : DeployCooldown;
}

bool FHRCombatState::CanActivate(EHRAbility Ability, bool bSupported, bool bOperational, bool bHasTarget) const
{
    if (static_cast<uint8>(Ability) > static_cast<uint8>(EHRAbility::Deploy)) return false;
    return bSupported && bOperational && bHasTarget && GetCooldown(Ability) <= 0.f;
}

bool FHRCombatState::CommitAbility(EHRAbility Ability, bool bSupported, bool bOperational, bool bHasTarget)
{
    if (!CanActivate(Ability, bSupported, bOperational, bHasTarget)) return false;
    if (Ability == EHRAbility::Grenade) GrenadeCooldown = 12.f;
    else if (Ability == EHRAbility::Smoke) SmokeCooldown = 18.f;
    else { DeployCooldown = 1.f; bDeployed = !bDeployed; }
    return true;
}

float FHRDamageTable::GetMultiplier(EHRDamageType Damage, EHRArmorClass Armor, const UHRDamageMatrix* AuthoredMatrix)
{
    if (static_cast<uint8>(Damage) > 2 || static_cast<uint8>(Armor) > 2) return 0.f;
    if (AuthoredMatrix) return AuthoredMatrix->GetDamageMultiplier(DamageTag(Damage), ArmorTag(Armor));
    // Rows: kinetic / explosive / Tempest. Columns: infantry / vehicle / structure.
    static constexpr float Matrix[3][3] = {{1.1f,1.f,.5f}, {1.5f,.85f,1.4f}, {1.f,1.2f,.9f}};
    return Matrix[static_cast<uint8>(Damage)][static_cast<uint8>(Armor)];
}

float FHRDamageTable::Resolve(float RawDamage, EHRDamageType Damage, EHRArmorClass Armor,
    float ArmorFactor, bool bInCover, const UHRDamageMatrix* AuthoredMatrix)
{
    if (!FMath::IsFinite(RawDamage) || RawDamage <= 0.f || !FMath::IsFinite(ArmorFactor) || ArmorFactor <= 0.f) return 0.f;
    const float Cover = bInCover && Armor == EHRArmorClass::Infantry ? .70f : 1.f;
    const double Resolved = static_cast<double>(RawDamage) * GetMultiplier(Damage, Armor, AuthoredMatrix)
        / FMath::Max(.1f, ArmorFactor) * Cover;
    return static_cast<float>(FMath::Min(Resolved, static_cast<double>(TNumericLimits<float>::Max())));
}

void FHRCommanderState::Advance(float DeltaSeconds)
{
    if (!FMath::IsFinite(DeltaSeconds) || DeltaSeconds <= 0.f) return;
    GlobalCooldown = FMath::Max(0.f, GlobalCooldown - DeltaSeconds);
    Earn(DeltaSeconds * .25f);
}

void FHRCommanderState::Earn(float Amount)
{
    Points = FMath::Min(100.f, Points + ValidNonnegative(Amount));
}

float FHRCommanderState::Cost(EHRDoctrine Doctrine)
{
    switch (Doctrine)
    {
        case EHRDoctrine::RepairPulse: return 35.f;
        case EHRDoctrine::PrecisionStrike: return 60.f;
        case EHRDoctrine::Scorch: return 50.f;
        case EHRDoctrine::Overdrive: return 40.f;
        default: return TNumericLimits<float>::Max();
    }
}

bool FHRCommanderState::BelongsToTeam(EHRDoctrine Doctrine, int32 Team)
{
    return Team == 0 ? (Doctrine == EHRDoctrine::RepairPulse || Doctrine == EHRDoctrine::PrecisionStrike)
        : Team == 1 && (Doctrine == EHRDoctrine::Scorch || Doctrine == EHRDoctrine::Overdrive);
}

bool FHRCommanderState::CanActivate(EHRDoctrine Doctrine, int32 Team, bool bHasValidTarget) const
{
    return BelongsToTeam(Doctrine, Team) && bHasValidTarget && GlobalCooldown <= 0.f
        && FMath::IsFinite(Points) && Points >= Cost(Doctrine);
}

bool FHRCommanderState::TryActivate(EHRDoctrine Doctrine, int32 Team, bool bHasValidTarget)
{
    if (!CanActivate(Doctrine, Team, bHasValidTarget)) return false;
    Points -= Cost(Doctrine);
    GlobalCooldown = 20.f;
    return true;
}
