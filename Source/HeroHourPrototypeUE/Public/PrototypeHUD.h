#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "PrototypeHUD.generated.h"

UCLASS()
class HEROHOURPROTOTYPEUE_API APrototypeHUD final : public AHUD
{
    GENERATED_BODY()

public:
    virtual void DrawHUD() override;
};
