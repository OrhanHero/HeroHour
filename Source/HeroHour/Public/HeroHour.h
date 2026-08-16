// HeroHour.h
// Main module header for HeroHour

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Online/HeroHourEOSSubsystem.h"

class FHeroHourModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static FHeroHourModule& Get()
    {
        return FModuleManager::LoadModuleChecked<FHeroHourModule>("HeroHour");
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("HeroHour");
    }
};

// Logging category
DECLARE_LOG_CATEGORY_EXTERN(LogHeroHourEOS, Log, All);