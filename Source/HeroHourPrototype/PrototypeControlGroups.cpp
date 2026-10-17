#include "PrototypeGameMode.h"
#include "Engine/World.h"

namespace
{
bool IsOwnLiveGroupMember(const APrototypeUnit* Unit)
{
    return IsValid(Unit)&&!Unit->IsActorBeingDestroyed()&&Unit->Team==0&&Unit->Health>0.f;
}
}

int32 APrototypeController::GetControlGroupCount(int32 Number) const
{
    if(Number<0||Number>9)return 0;
    TSet<const APrototypeUnit*> LiveMembers;
    if(const auto* Group=ControlGroups.Find(Number))
        for(const auto& Member:*Group)
            if(IsOwnLiveGroupMember(Member.Get()))LiveMembers.Add(Member.Get());
    return LiveMembers.Num();
}

bool APrototypeController::HandleControlGroupClick(int32 Number,bool bStore,bool bExtend)
{
    UWorld* World=GetWorld();
    auto* Mode=World?World->GetAuthGameMode<APrototypeGameMode>():nullptr;
    if(Number<0||Number>9||!Mode||Mode->Outcome!=0||World->IsPaused())return false;
    if(bStore)
    {
        auto& Group=ControlGroups.FindOrAdd(Number);
        Group.Empty();
        for(APrototypeUnit* Unit:Selected)
            if(IsOwnLiveGroupMember(Unit))Group.AddUnique(Unit);
        LastRecalledGroup=INDEX_NONE;
        LastGroupRecallTime=-1.f;
        Mode->Feedback=FString::Printf(TEXT("Gruppe %d gespeichert: %d Objekte."),Number,Group.Num());
        return true;
    }
    // Copy before changing selection: storing and recalling share weak membership,
    // while destroyed actors must never survive in the active selection.
    TArray<TWeakObjectPtr<APrototypeUnit>> Members;
    if(auto* Group=ControlGroups.Find(Number))
    {
        Group->RemoveAll([](const auto& Member){return !IsOwnLiveGroupMember(Member.Get());});
        Members=*Group;
    }
    if(!bExtend)ClearSelection();
    else
    {
        for(int32 Index=Selected.Num()-1;Index>=0;--Index)
            if(!IsOwnLiveGroupMember(Selected[Index]))
            {
                if(IsValid(Selected[Index]))Selected[Index]->Select(false);
                Selected.RemoveAt(Index);
            }
    }
    for(const auto& Member:Members)AddSelected(Member.Get());
    const float Now=World->GetTimeSeconds();
    if(LastRecalledGroup==Number&&Now>=LastGroupRecallTime&&Now-LastGroupRecallTime<=.3f)FocusSelection();
    LastRecalledGroup=Number;
    LastGroupRecallTime=Now;
    Mode->Feedback=FString::Printf(TEXT("Gruppe %d gewählt: %d Objekte."),Number,Selected.Num());
    return true;
}
