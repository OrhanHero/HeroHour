#include "HeroHourPrototypeUE.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogHeroHourPrototype);

class FHeroHourPrototypeUEModule final : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule() override
    {
        UE_LOG(LogHeroHourPrototype, Display, TEXT("HeroHour prototype module started."));
    }
};

IMPLEMENT_PRIMARY_GAME_MODULE(FHeroHourPrototypeUEModule, HeroHourPrototypeUE, "HeroHourPrototypeUE");
