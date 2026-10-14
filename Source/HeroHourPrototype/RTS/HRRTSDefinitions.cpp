#include "HRRTSDefinitions.h"

float UHRDamageMatrix::GetDamageMultiplier(FGameplayTag DamageType, FGameplayTag ArmorType) const
{
    if (!DamageType.IsValid() || !ArmorType.IsValid())
    {
        return 1.f;
    }

    for (const FHRDamageRule& Rule : Rules)
    {
        if (Rule.DamageType == DamageType && Rule.ArmorType == ArmorType)
        {
            return FMath::IsFinite(Rule.Multiplier) && Rule.Multiplier >= 0.f
                ? Rule.Multiplier : 1.f;
        }
    }

    return 1.f;
}
