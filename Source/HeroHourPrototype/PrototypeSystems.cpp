#include "PrototypeGameMode.h"
#include "BattlefieldLayout.h"
#include "RTS/HRRTSDefinitions.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Misc/ScopeExit.h"
#include "DrawDebugHelpers.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include <limits>

namespace
{
bool Live(const APrototypeUnit* Unit)
{
    return IsValid(Unit) && !Unit->IsActorBeingDestroyed() && Unit->Health > 0.f;
}

bool TeamValid(int32 Team) { return Team == 0 || Team == 1; }

bool FinitePosition(const FVector& Position)
{
    return FMath::IsFinite(Position.X) && FMath::IsFinite(Position.Y) && FMath::IsFinite(Position.Z);
}

const UHRBuildingDefinition* BuildingDefinition(const APrototypeUnit* Unit)
{
    return Unit ? Cast<UHRBuildingDefinition>(Unit->Definition) : nullptr;
}

double& TeamWallet(APrototypeGameMode* Mode, int32 Team)
{
    return Team == 0 ? Mode->Credits : Mode->EnemyCredits;
}

void UpdateFogPresentation(APrototypeGameMode* Mode)
{
    // Environment-review captures expose authored art only. Simulation visibility,
    // enemy targeting and all normal gameplay captures retain the actual fog.
    if (FParse::Param(FCommandLine::Get(),TEXT("PrototypeArtReview"))
        && (FParse::Param(FCommandLine::Get(),TEXT("PrototypeOverviewCapture"))
            || FParse::Param(FCommandLine::Get(),TEXT("PrototypeDistrictCapture")))) return;
    const int32 Width = Mode->Visibility.GetWidth(), Height = Mode->Visibility.GetHeight();
    if (Width <= 0 || Height <= 0 || !Mode->GetWorld()) return;
    if (!Mode->FogTexture || Mode->FogTexture->GetSizeX() != Width || Mode->FogTexture->GetSizeY() != Height)
    {
        Mode->FogTexture = UTexture2D::CreateTransient(Width,Height,PF_B8G8R8A8,TEXT("HeroHourFogMask"));
        if (!Mode->FogTexture) return;
        Mode->FogTexture->SRGB = false;
        Mode->FogTexture->Filter = TF_Bilinear;
        Mode->FogTexture->AddressX = TA_Clamp;
        Mode->FogTexture->AddressY = TA_Clamp;
        Mode->FogTexture->NeverStream = true;
    }
    TArray<FColor> Pixels;
    Mode->Visibility.GetFogPixels(0,Pixels);
    FTexturePlatformData* PlatformData = Mode->FogTexture->GetPlatformData();
    if (PlatformData && PlatformData->Mips.Num() > 0 && Pixels.Num() == Width * Height)
    {
        FTexture2DMipMap& Mip = PlatformData->Mips[0];
        void* Destination = Mip.BulkData.Lock(LOCK_READ_WRITE);
        if (Destination) FMemory::Memcpy(Destination,Pixels.GetData(),Pixels.Num()*sizeof(FColor));
        Mip.BulkData.Unlock();
        Mode->FogTexture->UpdateResource();
    }
    if (!Mode->FogMaterial)
    {
        UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr,
            TEXT("/Game/HERO/EffectsV4/M_PP_Fog.M_PP_Fog"));
        if (Material) Mode->FogMaterial = UMaterialInstanceDynamic::Create(Material,Mode);
    }
    if (!Mode->FogMaterial) return;
    Mode->FogMaterial->SetTextureParameterValue(TEXT("FogMask"),Mode->FogTexture);
    Mode->FogMaterial->SetVectorParameterValue(TEXT("FogBounds"),FLinearColor(-12000.f,-9000.f,24000.f,18000.f));
    if (!IsValid(Mode->FogVolume))
    {
        Mode->FogVolume = Mode->GetWorld()->SpawnActor<APostProcessVolume>();
        if (Mode->FogVolume)
        {
            Mode->FogVolume->bUnbound = true;
            Mode->FogVolume->BlendWeight = 1.f;
            Mode->FogVolume->Priority = 100.f;
            Mode->FogVolume->Settings.AddBlendable(Mode->FogMaterial,1.f);
        }
    }
}
}

float APrototypeGameMode::GetFactionMoveMultiplier(int32 Team) const
{
    const UHRFactionDefinition* Faction = GetFactionDefinition(Team);
    if (Faction && FMath::IsFinite(Faction->MoveMultiplier) && Faction->MoveMultiplier > 0.f)
        return Faction->MoveMultiplier;
    return GetFactionIndex(Team) == 1 ? 280.f/220.f : 1.f;
}

float APrototypeGameMode::GetFactionArmorMultiplier(int32 Team) const
{
    const UHRFactionDefinition* Faction = GetFactionDefinition(Team);
    if (Faction && FMath::IsFinite(Faction->ArmorMultiplier) && Faction->ArmorMultiplier > 0.f)
        return Faction->ArmorMultiplier;
    return GetFactionIndex(Team) == 1 ? .9f : 1.15f;
}

void APrototypeGameMode::RefreshPowerGrid()
{
    int64 Supply[2] = {0,0}, Demand[2] = {0,0};
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Live(*It) || !TeamValid(It->Team) || !It->IsStructure() || !It->IsCompleted()
            || It->bManuallyPoweredDown) continue;
        if (const UHRBuildingDefinition* Definition = BuildingDefinition(*It))
        {
            Supply[It->Team] += FMath::Max(0,Definition->PowerSupply);
            Demand[It->Team] += FMath::Max(0,Definition->PowerDemand);
        }
        else
        {
            // Source-only bootstrap before DataV4 is imported.
            if (It->Role == EPrototypeRole::PowerPlant) Supply[It->Team] += 200;
            else Demand[It->Team] += It->Role == EPrototypeRole::Factory ? 60
                : It->Role == EPrototypeRole::Radar ? 40 : It->Role == EPrototypeRole::Headquarters ? 10 : 30;
        }
    }
    for (int32 Team=0;Team<2;++Team)
    {
        PowerGrid[Team].CurrentPower = static_cast<int32>(FMath::Min(Supply[Team],static_cast<int64>(MAX_int32)));
        PowerGrid[Team].RequiredPower = static_cast<int32>(FMath::Min(Demand[Team],static_cast<int64>(MAX_int32)));
    }
}

float APrototypeGameMode::GetProductionRate(int32 Team) const
{
    return TeamValid(Team) ? PowerGrid[Team].GetProductionRate() : 0.f;
}

bool APrototypeGameMode::IsPowered(int32 Team) const
{
    return TeamValid(Team) && PowerGrid[Team].CanRunRadarAndDefense();
}

bool APrototypeGameMode::IsUnitOperational(const APrototypeUnit* Unit) const
{
    if (!Live(Unit) || !TeamValid(Unit->Team) || !Unit->IsCompleted() || Unit->bManuallyPoweredDown) return false;
    const UHRBuildingDefinition* Definition = BuildingDefinition(Unit);
    const bool bNeedsFullPower = Unit->Role == EPrototypeRole::Defense || Unit->Role == EPrototypeRole::Radar
        || (Definition && (Definition->bProvidesRadar || (Unit->IsCombatant() && Definition->bRequiresPowerForDefense)));
    return !bNeedsFullPower || IsPowered(Unit->Team);
}

bool APrototypeGameMode::HasRadar(int32 Team) const
{
    if (!IsPowered(Team)) return false;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Live(*It) || It->Team != Team || !IsUnitOperational(*It)) continue;
        const UHRBuildingDefinition* Definition = BuildingDefinition(*It);
        if (It->Role == EPrototypeRole::Radar || (Definition && Definition->bProvidesRadar)) return true;
    }
    return false;
}

bool APrototypeGameMode::TogglePower(APrototypeUnit* Unit)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || !Live(Unit) || !Unit->IsStructure() || !Unit->IsCompleted() || !TeamValid(Unit->Team)) return false;
    Unit->bManuallyPoweredDown = !Unit->bManuallyPoweredDown;
    RefreshPowerGrid();
    UpdateVisibility();
    if (Unit->Team == 0) Feedback = Unit->bManuallyPoweredDown ? TEXT("Gebaeude manuell abgeschaltet.") : TEXT("Gebaeude wieder eingeschaltet.");
    return true;
}

void APrototypeGameMode::UpdateVisibility()
{
    if (Visibility.GetWidth() == 0)
        Visibility.Reset(FVector2D(-BattlefieldLayout::ExtentX,-BattlefieldLayout::ExtentY),
            FVector2D(BattlefieldLayout::ExtentX,BattlefieldLayout::ExtentY),400.f,2);
    TArray<FHRVisionSource> Sources;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Live(*It) || !TeamValid(It->Team) || !It->IsCompleted() || (It->IsStructure() && !IsUnitOperational(*It))) continue;
        FHRVisionSource Source;
        Source.Team = It->Team;
        Source.Position = It->GetActorLocation();
        Source.SightRadius = It->Definition && It->Definition->Stats.SightRangeCm > 0.f
            ? It->Definition->Stats.SightRangeCm : It->IsStructure() ? 2200.f : 2600.f;
        Sources.Add(Source);
    }
    Visibility.RebuildVision(Sources);
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Live(*It)) continue;
        const bool bVisibleToPlayer = It->Team == 0 || CanTeamSee(0,*It);
        It->SetActorHiddenInGame(!bVisibleToPlayer);
        // Hidden enemies must not intercept selection traces through fog.
        It->SetActorEnableCollision(bVisibleToPlayer);
    }
    UpdateFogPresentation(this);
}

bool APrototypeGameMode::CanTeamSee(int32 Team, const AActor* Actor) const
{
    if (!TeamValid(Team) || !IsValid(Actor) || Actor->IsActorBeingDestroyed()) return false;
    if (const APrototypeUnit* Unit = Cast<APrototypeUnit>(Actor))
    {
        if (Unit->Health <= 0.f) return false;
        if (Unit->Team == Team) return true;
        return Unit->IsStructure()
            ? Visibility.IsAnyFootprintVisible(Team,Unit->GetActorLocation(),Footprint(Unit->Role))
            : Visibility.IsVisible(Team,Unit->GetActorLocation());
    }
    return Visibility.IsVisible(Team,Actor->GetActorLocation());
}

void APrototypeGameMode::AddSmokeArea(int32 Team, const FVector& Position, float Radius, float Duration)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || !TeamValid(Team)) return;
    if (Visibility.GetWidth() == 0) UpdateVisibility();
    const bool bVisibleBeforeCast = Team == 0 || Visibility.IsVisible(0,Position);
    if (Visibility.AddSmoke(Team,Position,Radius,Duration) == INDEX_NONE) return;
    UpdateVisibility();
    UStaticMesh* Sphere=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    UMaterialInterface* Smoke=LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/HERO/EffectsV4/M_FX_Smoke.M_FX_Smoke"));
    if (!Sphere || !Smoke) return;
    // Overlapping translucent volumes create a smoke bank. Lifespan uses the
    // paused world timer; mechanics remain authoritative in Visibility.
    const FVector Offsets[] = {FVector::ZeroVector,FVector(240,0,30),FVector(-240,0,60),
        FVector(0,240,45),FVector(0,-240,15),FVector(170,170,75),FVector(-170,-170,95)};
    for (const FVector& Offset : Offsets)
    {
        AStaticMeshActor* Cloud=GetWorld()->SpawnActor<AStaticMeshActor>(Position+Offset+FVector(0,0,130),FRotator::ZeroRotator);
        if (!Cloud) continue;
        Cloud->Tags.Add(TEXT("HERO_RuntimeFX"));
        Cloud->SetActorHiddenInGame(!bVisibleBeforeCast);
        UStaticMeshComponent* Mesh=Cloud->GetStaticMeshComponent();
        Mesh->SetMobility(EComponentMobility::Movable);
        Mesh->SetStaticMesh(Sphere);
        Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Mesh->SetCastShadow(false);
        UMaterialInstanceDynamic* Tint=UMaterialInstanceDynamic::Create(Smoke,Cloud);
        if (Tint) Tint->SetVectorParameterValue(TEXT("Tint"),FLinearColor(.18f,.20f,.22f,.35f));
        Mesh->SetMaterial(0,Tint);
        Cloud->SetActorScale3D(FVector(5.f,5.f,3.5f));
        Cloud->SetLifeSpan(FMath::Min(Duration,6.f));
    }
}

void APrototypeGameMode::AwardCommanderPoints(int32 Team, float Points)
{
    if (Outcome == 0 && TeamValid(Team)) Commander[Team].Earn(Points);
}

bool APrototypeGameMode::ActivateDoctrine(EHRDoctrine Doctrine, int32 Team, const FVector& Target)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || !TeamValid(Team) || !FinitePosition(Target) || !BattlefieldLayout::InBounds(Target)) return false;
    const bool bFriendlyPower = Doctrine == EHRDoctrine::RepairPulse || Doctrine == EHRDoctrine::Overdrive;
    if (!bFriendlyPower && !Visibility.IsVisible(Team,Target)) return false;
    const float Radius = Doctrine == EHRDoctrine::RepairPulse ? 900.f
        : Doctrine == EHRDoctrine::Overdrive ? 1100.f : Doctrine == EHRDoctrine::Scorch ? 650.f : 400.f;
    TArray<APrototypeUnit*> Affected;
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Live(*It) || FVector::DistSquared2D(Target,It->GetActorLocation()) > FMath::Square(Radius)) continue;
        if (bFriendlyPower)
        {
            if (It->Team != Team || !It->IsCompleted()) continue;
            if (Doctrine == EHRDoctrine::RepairPulse && It->Health >= It->MaxHealth) continue;
            if (Doctrine == EHRDoctrine::Overdrive && (!It->IsCombatant() || It->IsStructure()
                || It->CombatState.OverdriveRemaining > 0.f)) continue;
        }
        else if (It->Team == Team || !CanTeamSee(Team,*It)) continue;
        Affected.Add(*It);
    }
    if (!Commander[Team].TryActivate(Doctrine,GetFactionIndex(Team),!Affected.IsEmpty())) return false;
    for (APrototypeUnit* Unit : Affected)
    {
        if (!Live(Unit)) continue;
        if (Doctrine == EHRDoctrine::RepairPulse) Unit->Health = FMath::Min(Unit->MaxHealth,Unit->Health+80.f);
        else if (Doctrine == EHRDoctrine::Overdrive) Unit->CombatState.OverdriveRemaining = 8.f;
        else Unit->ReceiveCombatHit(Doctrine == EHRDoctrine::PrecisionStrike ? 100.f : 65.f,
            Doctrine == EHRDoctrine::PrecisionStrike ? EHRDamageType::Explosive : EHRDamageType::Tempest,nullptr);
    }
    DrawDebugSphere(GetWorld(),Target,Radius,24,bFriendlyPower ? FColor::Cyan : FColor::Orange,false,.75f,0,6.f);
    Feedback = TEXT("Commander-Faehigkeit eingesetzt.");
    return true;
}

bool APrototypeGameMode::SellBuilding(APrototypeUnit* Unit)
{
    if (Outcome != 0 || GetWorld()->IsPaused() || !Live(Unit) || !TeamValid(Unit->Team) || !Unit->IsStructure() || Unit->bHQ) return false;
    const int32 Team = Unit->Team;
    const float Cost = Unit->Definition ? FMath::Max(0,Unit->Definition->CreditCost) : RoleCost(Unit->Role);
    double Refund = Cost * .5;
    for (int32 Index=ProductionQueue.Num()-1;Index>=0;--Index)
    {
        if (ProductionQueue[Index].Producer.Get() != Unit) continue;
        Refund += FMath::Max(0.f,ProductionQueue[Index].PaidCost);
        ProductionQueue.RemoveAt(Index);
    }
    for (TActorIterator<APrototypeUnit> It(GetWorld()); It; ++It)
    {
        if (!Live(*It) || It->Role != EPrototypeRole::Collector || It->DeliveryDepot.Get() != Unit) continue;
        ReleaseSalvageReservation(*It);
        It->DeliveryDepot.Reset();
        It->CollectorState = It->Cargo > 0.0 ? EPrototypeCollectorState::MovingToRefinery : EPrototypeCollectorState::SeekingField;
        It->Destination = It->GetActorLocation();
        It->bMoveOrdered = false;
    }
    RefineryDocks.Remove(Unit);
    ReleaseSalvageReservation(Unit);
    if (APrototypeController* Controller = Cast<APrototypeController>(GetWorld()->GetFirstPlayerController()))
    {
        Controller->Selected.Remove(Unit);
        Controller->bDragging = false;
    }
    Unit->Select(false);
    Unit->Health = 0.f;
    Unit->Destroy();
    TeamWallet(this,Team) += Refund;
    RefreshPowerGrid();
    UpdateVisibility();
    if (Team == 0) Feedback = FString::Printf(TEXT("Gebaeude verkauft: %.0f Credits inklusive Warteschlangen-Erstattung."),Refund);
    return true;
}

FVector APrototypeGameMode::SnapBuildPosition(const FVector& Position) const
{
    if (!FinitePosition(Position)) return FVector::ZeroVector;
    return FVector(FMath::RoundToDouble(Position.X/BattlefieldLayout::CellSize)*BattlefieldLayout::CellSize,
        FMath::RoundToDouble(Position.Y/BattlefieldLayout::CellSize)*BattlefieldLayout::CellSize,35.f);
}

FIntPoint APrototypeGameMode::GetBuildDimensions(EPrototypeRole InRole, int32 Rotation, int32 Team) const
{
    FIntPoint Size;
    if (const UHRBuildingDefinition* Definition = Cast<UHRBuildingDefinition>(GetRoleDefinition(InRole,Team)))
        Size = FIntPoint(FMath::Clamp(Definition->FootprintCells.X,1,100),FMath::Clamp(Definition->FootprintCells.Y,1,100));
    else
    {
        const int32 Side = FMath::Max(1,FMath::CeilToInt(Footprint(InRole)*2.f/BattlefieldLayout::CellSize));
        Size = FIntPoint(Side,Side);
    }
    if ((Rotation % 2 + 2) % 2 == 1) Swap(Size.X,Size.Y);
    return Size;
}

// Optional helper for the authoritative Economy.cpp placement gate. Rotations
// are quarter turns, so axis-aligned rectangles are sufficient after swapping.
bool HRBuildRectanglesOverlap(const FVector& A, FIntPoint ASize, const FVector& B, FIntPoint BSize, float Clearance)
{
    if (!FinitePosition(A) || !FinitePosition(B) || ASize.X<=0 || ASize.Y<=0 || BSize.X<=0 || BSize.Y<=0
        || !FMath::IsFinite(Clearance) || Clearance<0.f) return true;
    return FMath::Abs(A.X-B.X) < (ASize.X+BSize.X)*BattlefieldLayout::CellSize*.5f+Clearance
        && FMath::Abs(A.Y-B.Y) < (ASize.Y+BSize.Y)*BattlefieldLayout::CellSize*.5f+Clearance;
}

bool RunHRSystemsSelfTests(UWorld* World)
{
    bool Passed = true;
    int32 Count = 0;
    auto Check = [&](bool Condition, const TCHAR* Label)
    {
        ++Count; Passed &= Condition;
        UE_LOG(LogTemp,Display,TEXT("SYSTEMS_TEST %s: %s"),Label,Condition ? TEXT("PASS") : TEXT("FAIL"));
    };
    auto Close = [](float A,float B) { return FMath::IsNearlyEqual(A,B,.001f); };
    APrototypeGameMode* Mode = World ? World->GetAuthGameMode<APrototypeGameMode>() : nullptr;
    if (!Mode)
    {
        Check(false,TEXT("systems fixtures have an authoritative game world"));
        UE_LOG(LogTemp,Display,TEXT("SYSTEMS_TEST_COUNT=%d"),Count);
        UE_LOG(LogTemp,Display,TEXT("SYSTEMS_TEST_RESULT=FAIL"));
        return false;
    }
    Mode->ResetMatch();
    {
        const float SavedDecision=Mode->BotDecisionTime;
        const int32 SavedRotation=Mode->BuildingQuarterTurns;
        Mode->BotDecisionTime=10000.f; Mode->BuildingQuarterTurns=0;
        const FVector Site(-7400,-1600,35);
        const double BeforeCredits=Mode->Credits;
        APrototypeUnit* CornerVehicle=nullptr;
        UHRBuildingDefinition* FactoryDefinition=const_cast<UHRBuildingDefinition*>(
            Cast<UHRBuildingDefinition>(Mode->GetRoleDefinition(EPrototypeRole::Factory,0)));
        const FIntPoint SavedCells=FactoryDefinition ? FactoryDefinition->FootprintCells : FIntPoint::ZeroValue;
        const bool HasField=Mode->SalvageNodes.IsValidIndex(0) && Mode->SalvageNodes[0].CellOffsets.Num()>0;
        const FVector SavedPosition=HasField ? Mode->SalvageNodes[0].Position : FVector::ZeroVector;
        const TArray<FVector> SavedOffsets=HasField ? Mode->SalvageNodes[0].CellOffsets : TArray<FVector>();
        ON_SCOPE_EXIT
        {
            if(Live(CornerVehicle)) CornerVehicle->Destroy();
            if(FactoryDefinition) FactoryDefinition->FootprintCells=SavedCells;
            if(HasField) {Mode->SalvageNodes[0].Position=SavedPosition;Mode->SalvageNodes[0].CellOffsets=SavedOffsets;}
            Mode->BuildingQuarterTurns=SavedRotation; Mode->BotDecisionTime=SavedDecision;
            Mode->UpdateResourceVisuals();
        };
        Check(Mode->CanPlaceBuilding(EPrototypeRole::Factory,Site,0),
            TEXT("grid corner regression starts with the unoccupied canonical factory site"));
        CornerVehicle=Mode->SpawnUnit(0,Site+FVector(590,590,0));
        Check(Live(CornerVehicle) && !Mode->CanPlaceBuilding(EPrototypeRole::Factory,Site,0)
            && !Mode->BeginBuilding(EPrototypeRole::Factory,Site,0) && Mode->Credits==BeforeCredits,
            TEXT("mobile unit in rectangular factory corner rejects preview and paid placement without debit"));
        if(Live(CornerVehicle)) CornerVehicle->Destroy();
        Check(Mode->CanPlaceBuilding(EPrototypeRole::Factory,Site,0),
            TEXT("factory site becomes valid again after its corner vehicle is removed"));
        if(HasField)
        {
            FPrototypeSalvage& Field=Mode->SalvageNodes[0];
            Check(Field.Field.GetHarvestableAmount(0)>0.f,
                TEXT("field corner regression uses a positive live resource cell"));
            Field.CellOffsets[0]=Site+FVector(590,590,0)-Field.Position;
            Check(!Mode->CanPlaceBuilding(EPrototypeRole::Factory,Site,0)
                && !Mode->BeginBuilding(EPrototypeRole::Factory,Site,0) && Mode->Credits==BeforeCredits,
                TEXT("resource cell in rectangular factory corner rejects preview and payment without circle loophole"));
            Field.Position=SavedPosition; Field.CellOffsets=SavedOffsets;
            Check(Mode->CanPlaceBuilding(EPrototypeRole::Factory,Site,0),
                TEXT("restoring resource cell position releases the factory corner"));
            if(FactoryDefinition)
            {
                // Candidate dimensions come from the shared loaded definition.
                // Change them only in memory under scope restoration; never save.
                FactoryDefinition->FootprintCells=FIntPoint(6,4);
                Field.CellOffsets[0]=Site+FVector(590,490,0)-Field.Position;
                Mode->BuildingQuarterTurns=0;
                const bool WideBlocked=!Mode->CanPlaceOverFields(EPrototypeRole::Factory,Site,0);
                Mode->BuildingQuarterTurns=1;
                const bool RotatedClear=Mode->CanPlaceOverFields(EPrototypeRole::Factory,Site,0);
                Check(WideBlocked && RotatedClear,
                    TEXT("actual nonsquare candidate rotation swaps rectangular field occupancy rather than retaining a circular veto"));
                FactoryDefinition->FootprintCells=SavedCells;
                Field.Position=SavedPosition; Field.CellOffsets=SavedOffsets;
            }
            else Check(false,TEXT("saved factory building definition exists for rotated rectangle regression"));
        }
        else Check(false,TEXT("live field exists for rectangular resource occupancy regression"));
    }
    const double InitialCredits=Mode->Credits, InitialEnemyCredits=Mode->EnemyCredits;
    const float InitialBotDecision=Mode->BotDecisionTime;
    const float InitialMatchTime=Mode->MatchTime;
    const int32 InitialRotation=Mode->BuildingQuarterTurns;
    APlayerController* PauseController=World->GetFirstPlayerController();
    const bool InitialPaused=World->IsPaused();
    const FHRCommanderState InitialCommanderZero=Mode->Commander[0], InitialCommanderOne=Mode->Commander[1];
    const TArray<FPrototypeProduction> InitialProduction=Mode->ProductionQueue;
    TMap<TWeakObjectPtr<APrototypeUnit>,bool> ExistingPowerStates;
    for (TActorIterator<APrototypeUnit> It(World); It; ++It)
        if (Live(*It) && It->IsStructure())
        {
            ExistingPowerStates.Add(*It,It->bManuallyPoweredDown);
            It->bManuallyPoweredDown=true;
        }
    TArray<APrototypeUnit*> Fixtures;
    ON_SCOPE_EXIT
    {
        if (PauseController) PauseController->SetPause(InitialPaused);
        for (APrototypeUnit* Unit : Fixtures) if (Live(Unit)) Unit->Destroy();
        for (const auto& Pair : ExistingPowerStates)
            if (Live(Pair.Key.Get())) Pair.Key->bManuallyPoweredDown=Pair.Value;
        Mode->Credits=InitialCredits; Mode->EnemyCredits=InitialEnemyCredits;
        Mode->BotDecisionTime=InitialBotDecision;
        Mode->MatchTime=InitialMatchTime; Mode->BuildingQuarterTurns=InitialRotation;
        Mode->Commander[0]=InitialCommanderZero; Mode->Commander[1]=InitialCommanderOne;
        Mode->ProductionQueue=InitialProduction;
        Mode->RefreshPowerGrid(); Mode->UpdateVisibility();
    };
    Mode->ProductionQueue.Empty(); Mode->BotDecisionTime=100000.f;
    auto Spawn = [&](int32 Team,EPrototypeRole Role,const FVector& Position) -> APrototypeUnit*
    {
        APrototypeUnit* Unit=Mode->SpawnUnit(Team,Position);
        if (Unit)
        {
            Unit->ConfigureRole(Role); Unit->SetActorTickEnabled(false);
            Fixtures.Add(Unit);
        }
        return Unit;
    };
    auto PowerDefinition = [&](APrototypeUnit* Unit,int32 Supply,int32 Demand)
    {
        UHRBuildingDefinition* Definition=NewObject<UHRBuildingDefinition>(Unit);
        if (Unit->Definition) Definition->Stats=Unit->Definition->Stats;
        Definition->PowerSupply=Supply; Definition->PowerDemand=Demand;
        Definition->bProvidesRadar=Unit->Role==EPrototypeRole::Radar;
        Definition->bRequiresPowerForDefense=true;
        Definition->CreditCost=Unit->Role==EPrototypeRole::Factory ? 350 : 200;
        Unit->Definition=Definition;
    };

    Check(Mode->SnapBuildPosition(FVector(119,-309,0)).Equals(FVector(200,-400,35),.001),
        TEXT("building placement snaps positive and negative coordinates to the 200 cm grid"));
    Check(Mode->SnapBuildPosition(FVector(std::numeric_limits<double>::quiet_NaN(),0,0))==FVector::ZeroVector,
        TEXT("malformed build coordinate is rejected by the snap helper"));
    const FIntPoint Base=Mode->GetBuildDimensions(EPrototypeRole::Factory,0),
        Rotated=Mode->GetBuildDimensions(EPrototypeRole::Factory,1), NegativeRotated=Mode->GetBuildDimensions(EPrototypeRole::Factory,-1);
    Check(Base.X>0 && Base.Y>0 && Rotated==FIntPoint(Base.Y,Base.X) && Rotated==NegativeRotated,
        TEXT("quarter-turn placement swaps authored rectangular footprint dimensions"));
    Check(HRBuildRectanglesOverlap(FVector::ZeroVector,FIntPoint(2,4),FVector(390,0,0),FIntPoint(2,4),0)
        && !HRBuildRectanglesOverlap(FVector::ZeroVector,FIntPoint(2,4),FVector(400,0,0),FIntPoint(2,4),0)
        && HRBuildRectanglesOverlap(FVector::ZeroVector,FIntPoint(2,4),FVector(400,0,0),FIntPoint(2,4),30),
        TEXT("rectangular occupancy distinguishes overlap, touching boundaries and clearance"));
    Mode->RefreshPowerGrid();
    Check(Mode->PowerGrid[0].CurrentPower==0 && Mode->PowerGrid[0].RequiredPower==0,
        TEXT("manually disabled infrastructure contributes neither supply nor demand"));

    APrototypeUnit* Plant=Spawn(0,EPrototypeRole::PowerPlant,FVector(-11000,-8000,35));
    APrototypeUnit* Factory=Spawn(0,EPrototypeRole::Factory,FVector(-10000,-8000,35));
    APrototypeUnit* Radar=Spawn(0,EPrototypeRole::Radar,FVector(-11000,-6800,35));
    APrototypeUnit* Defense=Spawn(0,EPrototypeRole::Defense,FVector(-10000,-6800,35));
    if (Plant && Factory && Radar && Defense)
    {
        PowerDefinition(Plant,100,0); PowerDefinition(Factory,0,60);
        PowerDefinition(Radar,0,40); PowerDefinition(Defense,0,20);
        Mode->RefreshPowerGrid();
        Check(Mode->PowerGrid[0].CurrentPower==100 && Mode->PowerGrid[0].RequiredPower==120
            && Mode->PowerGrid[0].GetState()==EHRPowerState::LowPower && Mode->GetProductionRate(0)==.5f,
            TEXT("live grid counts all demand during low power and halves production"));
        Check(!Mode->HasRadar(0) && !Mode->IsUnitOperational(Defense),
            TEXT("live low power disables radar and defenses"));
        Plant->ConstructionProgress=.5f; Mode->RefreshPowerGrid();
        Check(Mode->PowerGrid[0].CurrentPower==0 && Mode->PowerGrid[0].RequiredPower==120
            && Mode->PowerGrid[0].GetState()==EHRPowerState::Offline && Mode->GetProductionRate(0)==0.f,
            TEXT("unfinished power plant supplies no power and offline production stops"));
        Plant->ConstructionProgress=1.f; Plant->bManuallyPoweredDown=true; Mode->RefreshPowerGrid();
        Check(Mode->PowerGrid[0].CurrentPower==0 && Mode->PowerGrid[0].RequiredPower==120,
            TEXT("manually disabled power plant does not supply electricity"));
        Plant->bManuallyPoweredDown=false; Radar->bManuallyPoweredDown=true; Mode->RefreshPowerGrid();
        Check(Mode->PowerGrid[0].CurrentPower==100 && Mode->PowerGrid[0].RequiredPower==80
            && Mode->IsPowered(0) && Mode->IsUnitOperational(Defense) && !Mode->HasRadar(0),
            TEXT("manual load shedding restores defense but leaves the disabled radar off"));
        Check(Mode->TogglePower(Radar) && Mode->PowerGrid[0].RequiredPower==120 && !Mode->IsPowered(0),
            TEXT("live power toggle restores radar demand and returns an overloaded grid to low power"));
        FPrototypeProduction Order;
        Order.Producer=Factory; Order.Role=EPrototypeRole::Combat; Order.Duration=100.f; Order.PaidCost=100.f;
        Mode->ProductionQueue.Add(Order);
        Mode->TickEconomy(2.f);
        Check(Mode->ProductionQueue.Num()==1 && Close(Mode->ProductionQueue[0].Progress,1.f),
            TEXT("real production queue earns one second of work in two low-power simulation seconds"));
        Plant->bManuallyPoweredDown=true; Mode->RefreshPowerGrid();
        Mode->TickEconomy(2.f);
        Check(Mode->ProductionQueue.Num()==1 && Close(Mode->ProductionQueue[0].Progress,1.f),
            TEXT("real production queue retains paid work while the power grid is offline"));
        Plant->bManuallyPoweredDown=false; Radar->bManuallyPoweredDown=true; Mode->RefreshPowerGrid();
        Mode->TickEconomy(2.f);
        Check(Mode->ProductionQueue.Num()==1 && Close(Mode->ProductionQueue[0].Progress,3.f),
            TEXT("real production queue resumes at full speed after enough demand is disabled"));
        Factory->bManuallyPoweredDown=true; Mode->RefreshPowerGrid();
        Mode->TickEconomy(2.f);
        Check(Mode->ProductionQueue.Num()==1 && Close(Mode->ProductionQueue[0].Progress,3.f)
            && !Mode->IsUnitOperational(Factory), TEXT("manually disabled producer retains its paid queue without advancing"));
        Factory->bManuallyPoweredDown=false; Mode->ProductionQueue.Empty();
        Radar->bManuallyPoweredDown=false; Plant->Definition->Stats.SightRangeCm=2600.f;
        Mode->RefreshPowerGrid(); Mode->UpdateVisibility();
        APrototypeUnit* Target=Spawn(1,EPrototypeRole::Combat,Defense->GetActorLocation()+FVector(300,0,0));
        if (Target)
        {
            Target->Health=Target->MaxHealth=1000.f;
            Mode->UpdateVisibility();
            Defense->OrderAttack(Target); Defense->Tick(1.1f);
            Check(Target->Health==1000.f,TEXT("live defense cannot fire under low power"));
            Radar->bManuallyPoweredDown=true; Mode->RefreshPowerGrid(); Mode->UpdateVisibility();
            Defense->Tick(1.1f);
            Check(Target->Health<1000.f,TEXT("live defense starts firing after its grid returns online"));
            Target->Destroy();
        }
        else Check(false,TEXT("live powered defense target spawns"));
        Plant->Destroy(); Factory->Destroy(); Radar->Destroy(); Defense->Destroy();
    }
    else Check(false,TEXT("live power infrastructure fixtures spawn"));

    APrototypeUnit* Friendly=Spawn(0,EPrototypeRole::Combat,FVector(-11000,-7600,35));
    APrototypeUnit* Enemy=Spawn(1,EPrototypeRole::Combat,FVector(-10500,-7600,35));
    APrototypeUnit* Ember=Spawn(1,EPrototypeRole::Infantry,FVector(-10500,-8200,35));
    if (Friendly && Enemy && Ember)
    {
        Friendly->Health=Friendly->MaxHealth=1000.f;
        Enemy->Health=Enemy->MaxHealth=1000.f;
        Mode->Commander[0].Points=100.f; Mode->Commander[0].GlobalCooldown=0.f;
        Mode->Commander[1].Points=100.f; Mode->Commander[1].GlobalCooldown=0.f;
        Mode->UpdateVisibility();
        Check(!Mode->ActivateDoctrine(EHRDoctrine::PrecisionStrike,0,FVector(10000,8000,35))
            && Mode->Commander[0].Points==100.f && Mode->Commander[0].GlobalCooldown==0.f,
            TEXT("attack doctrine rejects hidden or empty target without charging points"));
        Check(!Mode->ActivateDoctrine(EHRDoctrine::RepairPulse,0,Friendly->GetActorLocation())
            && Mode->Commander[0].Points==100.f,TEXT("repair doctrine does not charge when all friendly units are healthy"));
        Friendly->Health=850.f;
        Check(Mode->ActivateDoctrine(EHRDoctrine::RepairPulse,0,Friendly->GetActorLocation())
            && Friendly->Health==930.f && Mode->Commander[0].Points==65.f && Enemy->Health==1000.f,
            TEXT("live repair doctrine heals 80 health, charges 35 points and preserves enemies"));
        const float EnemyBefore=Enemy->Health;
        Check(!Mode->ActivateDoctrine(EHRDoctrine::PrecisionStrike,0,Enemy->GetActorLocation())
            && Enemy->Health==EnemyBefore && Mode->Commander[0].Points==65.f,
            TEXT("live doctrine global cooldown blocks another power without extra charge or damage"));
        Mode->Commander[0].Advance(20.f);
        Friendly->SetActorLocation(Enemy->GetActorLocation()+FVector(-150,0,0));
        Mode->UpdateVisibility();
        const float FriendlyBefore=Friendly->Health;
        Check(Mode->ActivateDoctrine(EHRDoctrine::PrecisionStrike,0,Enemy->GetActorLocation())
            && Enemy->Health<EnemyBefore && Friendly->Health==FriendlyBefore,
            TEXT("live precision strike uses typed damage on visible enemies without friendly fire"));
        const float EmberSpeed=Ember->GetEffectiveSpeed(), EmberDamage=Ember->GetEffectiveDamage();
        Check(Mode->ActivateDoctrine(EHRDoctrine::Overdrive,1,Ember->GetActorLocation())
            && Ember->CombatState.OverdriveRemaining==8.f && Close(Ember->GetEffectiveSpeed(),EmberSpeed*1.35f)
            && Close(Ember->GetEffectiveDamage(),EmberDamage*1.25f) && Mode->Commander[1].Points==60.f,
            TEXT("Ember overdrive charges 40 points and applies an eight second speed/damage buff"));
        Ember->CombatState.Advance(8.f);
        Check(Ember->CombatState.OverdriveRemaining==0.f && Close(Ember->GetEffectiveSpeed(),EmberSpeed)
            && Close(Ember->GetEffectiveDamage(),EmberDamage),TEXT("overdrive expires on simulation time and restores base stats"));
        Mode->Commander[1].GlobalCooldown=0.f;
        const float BeforeScorch=Friendly->Health;
        const float EmberHealthBefore=Ember->Health;
        Check(Mode->ActivateDoctrine(EHRDoctrine::Scorch,1,Friendly->GetActorLocation())
            && Friendly->Health<BeforeScorch && Ember->Health==EmberHealthBefore && Mode->Commander[1].Points==10.f,
            TEXT("Ember scorch damages visible opposing units and charges its own faction wallet"));
        Friendly->Destroy(); Enemy->Destroy(); Ember->Destroy();
    }
    else Check(false,TEXT("live doctrine fixtures spawn"));

    APrototypeUnit* Sold=Spawn(0,EPrototypeRole::Factory,FVector(-11000,-8000,35));
    if (Sold)
    {
        PowerDefinition(Sold,0,60);
        Mode->Credits=0.; Mode->ProductionQueue.Empty();
        FPrototypeProduction First; First.Producer=Sold; First.PaidCost=100.f;
        FPrototypeProduction Second; Second.Producer=Sold; Second.PaidCost=150.f;
        Mode->ProductionQueue.Add(First); Mode->ProductionQueue.Add(Second);
        Check(Mode->SellBuilding(Sold) && Mode->Credits==425. && Mode->ProductionQueue.IsEmpty()
            && !Live(Sold),TEXT("selling refunds 50 percent building cost plus each exact paid queue snapshot once"));
        Check(!Mode->SellBuilding(Sold) && Mode->Credits==425.,TEXT("repeated sell cannot refund a destroyed building twice"));
    }
    else Check(false,TEXT("live sale fixture spawns"));
    APrototypeUnit* Depot=Spawn(0,EPrototypeRole::Depot,FVector(-11000,-8000,35));
    APrototypeUnit* Collector=Spawn(0,EPrototypeRole::Collector,FVector(-10510,-8000,35));
    if (Depot && Collector)
    {
        PowerDefinition(Depot,0,30);
        Collector->Cargo=17.; Collector->DeliveryDepot=Depot;
        FHRRefineryDocking& Dock=Mode->RefineryDocks.FindOrAdd(Depot);
        Dock.Reset(Depot); Dock.Submit(Collector);
        const double BeforeWallet=Mode->Credits;
        Check(Mode->SellBuilding(Depot) && Collector->Cargo==17. && !Collector->DeliveryDepot.IsValid()
            && Collector->CollectorState==EPrototypeCollectorState::MovingToRefinery
            && !Mode->RefineryDocks.Contains(Depot) && Mode->Credits==BeforeWallet+100.,
            TEXT("selling refinery releases its docking queue and preserves undelivered collector cargo"));
        Collector->Destroy();
    }
    else Check(false,TEXT("live refinery sale fixtures spawn"));
    Check(!Mode->SellBuilding(Mode->PlayerHQ),TEXT("headquarters cannot be sold through the ordinary building sale command"));
    // Exercise authority directly, rather than merely testing disabled HUD
    // presentation: these same calls previously remained usable through keys.
    Mode->ProductionQueue.Empty(); Mode->Credits=4000.;
    APrototypeUnit* PauseFactory=Spawn(0,EPrototypeRole::Factory,FVector(-11000,-8000,35));
    APrototypeUnit* PauseInfantry=Spawn(0,EPrototypeRole::Infantry,FVector(-11000,-7000,35));
    APrototypeUnit* PauseEnemy=Spawn(1,EPrototypeRole::Combat,FVector(-10500,-7000,35));
    const int32 SmokeTeam=Mode->GetFactionIndex(0)==0 ? 0 : 1;
    APrototypeUnit* PauseSmoke=Spawn(SmokeTeam,EPrototypeRole::Combat,FVector(-11000,-6000,35));
    if (PauseController && PauseFactory && PauseInfantry && PauseEnemy && PauseSmoke)
    {
        Mode->RefreshPowerGrid(); Mode->UpdateVisibility();
        Mode->Commander[0].Points=100.f; Mode->Commander[0].GlobalCooldown=0.f;
        PauseInfantry->Health=PauseInfantry->MaxHealth*.5f;
        const EHRDoctrine Doctrine=Mode->GetFactionIndex(0)==0 ? EHRDoctrine::RepairPulse : EHRDoctrine::Overdrive;
        Check(Mode->CanTeamSee(0,PauseEnemy) && Mode->IsUnitOperational(PauseInfantry)
            && PauseInfantry->CombatState.GrenadeCooldown==0.f && PauseSmoke->CombatState.SmokeCooldown==0.f,
            TEXT("paused ability regression starts with operational units, ready cooldowns and a visible grenade target"));
        FVector BuildSite=FVector::ZeroVector; bool HasSite=false;
        for (int32 Y=-2200;Y<=2200 && !HasSite;Y+=200)
        for (int32 X=-2200;X<=2200 && !HasSite;X+=200)
        {
            const FVector Candidate=Mode->SnapBuildPosition(Mode->PlayerHQ->GetActorLocation()+FVector(X,Y,0));
            if (Mode->CanPlaceBuilding(EPrototypeRole::PowerPlant,Candidate,0)) {BuildSite=Candidate;HasSite=true;}
        }
        Check(HasSite,TEXT("paused construction regression starts with an otherwise legal affordable site"));
        const double WalletBeforePause=Mode->Credits;
        const float TargetHealthBeforePause=PauseEnemy->Health, FriendlyHealthBeforePause=PauseInfantry->Health;
        const float MatchBeforePause=Mode->MatchTime;
        const bool FactoryPowerBeforePause=PauseFactory->bManuallyPoweredDown;
        const int32 QueuesBeforePause=Mode->ProductionQueue.Num();
        const bool Paused=PauseController->SetPause(true);
        Check(Paused && World->IsPaused(),TEXT("authority regression enters real world pause through the player controller"));
        Check(!PauseInfantry->TryActivateAbility(EHRAbility::Grenade)
            && PauseInfantry->CombatState.GrenadeCooldown==0.f && PauseEnemy->Health==TargetHealthBeforePause,
            TEXT("paused grenade rejects commit without cooldown consumption or target damage"));
        Check(!PauseSmoke->TryActivateAbility(EHRAbility::Smoke) && PauseSmoke->CombatState.SmokeCooldown==0.f,
            TEXT("paused smoke rejects commit without consuming its ready cooldown"));
        Check(!Mode->TogglePower(PauseFactory) && PauseFactory->bManuallyPoweredDown==FactoryPowerBeforePause,
            TEXT("paused power command leaves the selected building unchanged"));
        Check(!Mode->ActivateDoctrine(Doctrine,0,PauseInfantry->GetActorLocation())
            && Mode->Commander[0].Points==100.f && Mode->Commander[0].GlobalCooldown==0.f
            && PauseInfantry->Health==FriendlyHealthBeforePause && PauseInfantry->CombatState.OverdriveRemaining==0.f,
            TEXT("paused doctrine rejects without points, cooldown, repair or overdrive mutation"));
        Check(!Mode->RequestProduction(EPrototypeRole::Combat,0)
            && !Mode->RequestProductionAt(PauseFactory,EPrototypeRole::Combat)
            && Mode->ProductionQueue.Num()==QueuesBeforePause && Mode->Credits==WalletBeforePause,
            TEXT("both paused production entry points reject without payment or a new order"));
        int32 ActorCountBeforePause=0;
        for (TActorIterator<APrototypeUnit> It(World);It;++It) if(Live(*It)) ++ActorCountBeforePause;
        const bool BuiltWhilePaused=HasSite && Mode->BeginBuilding(EPrototypeRole::PowerPlant,BuildSite,0);
        int32 ActorCountAfterPause=0;
        for (TActorIterator<APrototypeUnit> It(World);It;++It) if(Live(*It)) ++ActorCountAfterPause;
        Check(!BuiltWhilePaused && ActorCountBeforePause==ActorCountAfterPause && Mode->Credits==WalletBeforePause,
            TEXT("paused building command creates no construction actor and spends no credits"));
        Check(!Mode->SellBuilding(PauseFactory) && Live(PauseFactory) && Mode->Credits==WalletBeforePause,
            TEXT("paused sale preserves the building and cannot award a refund"));
        Mode->TickEconomy(10.f);
        Check(Mode->MatchTime==MatchBeforePause && Mode->Credits==WalletBeforePause,
            TEXT("direct economy advancement remains inert during the same paused authority test"));
        PauseController->SetPause(false);
    }
    else Check(false,TEXT("paused command fixtures and player controller exist"));
    for(APrototypeUnit* Unit:{PauseFactory,PauseInfantry,PauseEnemy,PauseSmoke}) if(Live(Unit)) Unit->Destroy();

    // The only surviving original sources, including HQ, are manually off.
    // A paid power plant must progress independently of an earlier base site.
    Mode->Credits=4000.; Mode->ProductionQueue.Empty(); Mode->BuildingQuarterTurns=0;
    APrototypeUnit* RecoveryLoad=Spawn(0,EPrototypeRole::Radar,FVector(-11000,-8000,35));
    if(RecoveryLoad)
    {
        PowerDefinition(RecoveryLoad,0,40); Mode->RefreshPowerGrid();
        Check(Mode->PowerGrid[0].CurrentPower==0 && Mode->PowerGrid[0].RequiredPower==40
            && Mode->GetProductionRate(0)==0.f && Mode->PlayerHQ->bManuallyPoweredDown,
            TEXT("recovery starts with manually disabled HQ and plants, zero supply and real offline demand"));
        auto BuildPaid=[&](EPrototypeRole Role) -> APrototypeUnit*
        {
            for(int32 Y=-2200;Y<=2200;Y+=200)
            for(int32 X=-2200;X<=2200;X+=200)
            {
                const FVector Site=Mode->SnapBuildPosition(Mode->PlayerHQ->GetActorLocation()+FVector(X,Y,0));
                if(!Mode->CanPlaceBuilding(Role,Site,0)) continue;
                const double Before=Mode->Credits;
                if(!Mode->BeginBuilding(Role,Site,0)) return nullptr;
                for(TActorIterator<APrototypeUnit> It(World);It;++It)
                    if(Live(*It) && It->Team==0 && It->Role==Role && !It->IsCompleted()
                        && It->GetActorLocation().Equals(Site,.01f))
                    {
                        Fixtures.Add(*It); It->SetActorTickEnabled(false);
                        Check(Mode->Credits==Before-Mode->RoleCost(Role,0),
                            TEXT("offline recovery construction pays the public command price exactly once"));
                        return *It;
                    }
                return nullptr;
            }
            return nullptr;
        };
        APrototypeUnit* EarlierFactory=BuildPaid(EPrototypeRole::Factory);
        APrototypeUnit* RecoveryPlant=BuildPaid(EPrototypeRole::PowerPlant);
        Check(EarlierFactory && RecoveryPlant,TEXT("offline recovery places the blocked base site before its paid replacement power source"));
        if(EarlierFactory && RecoveryPlant)
        {
            const float Duration=Mode->RoleDuration(EPrototypeRole::PowerPlant,0);
            Mode->TickEconomy(1.f);
            Check(Close(RecoveryPlant->ConstructionProgress,1.f/Duration) && EarlierFactory->ConstructionProgress==0.f,
                TEXT("power construction progresses at full rate in its own lane while earlier offline factory remains frozen"));
            Mode->TickEconomy(Duration);
            Mode->RefreshPowerGrid();
            Check(RecoveryPlant->IsCompleted() && Mode->IsPowered(0) && Mode->PowerGrid[0].CurrentPower>0,
                TEXT("paid replacement power plant finishes and restores the actual team grid without toggling old sources"));
            Mode->TickEconomy(1.f);
            Check(EarlierFactory->ConstructionProgress>0.f,
                TEXT("restored supply resumes the previously blocked base construction lane"));
        }
    }
    else Check(false,TEXT("offline recovery demand fixture spawns"));
    UE_LOG(LogTemp,Display,TEXT("SYSTEMS_TEST_COUNT=%d"),Count);
    UE_LOG(LogTemp,Display,TEXT("SYSTEMS_TEST_RESULT=%s"),Passed ? TEXT("PASS") : TEXT("FAIL"));
    return Passed;
}
