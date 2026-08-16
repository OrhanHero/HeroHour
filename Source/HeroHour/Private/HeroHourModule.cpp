// HeroHourModule.cpp
// HeroHour Module Implementation

#include "HeroHour.h"
#include "Modules/ModuleManager.h"
#include "OnlineSubsystemModule.h"
#include "OnlineSubsystemEOS.h"
#include "Online/HeroHourEOSSubsystem.h"

IMPLEMENT_MODULE(FHeroHourModule, HeroHour);

DEFINE_LOG_CATEGORY(LogHeroHourEOS);

void FHeroHourModule::StartupModule()
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour Module Starting Up..."));

    // Ensure OnlineSubsystemEOS is loaded
    if (FModuleManager::Get().IsModuleLoaded("OnlineSubsystemEOS"))
    {
        UE_LOG(LogHeroHourEOS, Log, TEXT("OnlineSubsystemEOS module is loaded"));
    }
    else
    {
        UE_LOG(LogHeroHourEOS, Warning, TEXT("OnlineSubsystemEOS module not loaded - attempting to load"));
        FModuleManager::Get().LoadModule("OnlineSubsystemEOS");
    }

    // Register the EOS subsystem with GameInstance
    // This is typically done in the GameInstance's Init() but we can also do it here
    // for editor/game contexts

    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour Module Started Successfully"));
}

void FHeroHourModule::ShutdownModule()
{
    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour Module Shutting Down..."));

    // Clean up if needed

    UE_LOG(LogHeroHourEOS, Log, TEXT("HeroHour Module Shutdown Complete"));
}