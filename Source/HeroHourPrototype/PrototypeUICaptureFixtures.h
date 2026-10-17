#pragma once
#include "CoreMinimal.h"

class APrototypeGameMode;
class APrototypeController;

// Explicitly synthetic visual-QA states; never called by ordinary match setup.
HEROHOURPROTOTYPE_API bool PreparePrototypeUICaptureFixture(APrototypeGameMode* Mode,APrototypeController* Controller,const FString& State);
