#pragma once
#include "Commandlets/Commandlet.h"
#include "TempestMappingsProbeCommandlet.generated.h"
UCLASS()
class UTempestMappingsProbeCommandlet : public UCommandlet
{
 GENERATED_BODY()
public:
 UTempestMappingsProbeCommandlet();
 virtual int32 Main(const FString& Params) override;
};
