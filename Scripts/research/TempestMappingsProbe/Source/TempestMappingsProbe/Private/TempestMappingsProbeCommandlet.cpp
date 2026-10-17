#include "TempestMappingsProbeCommandlet.h"
#include "UObject/UnrealType.h"
#include "UObject/UObjectGlobals.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/Parse.h"

namespace
{
struct FProbe
{
 TArray<UStruct*> Pending;
 TSet<UStruct*> Seen;
 TSet<UEnum*> Enums;
 TArray<TSharedPtr<FJsonValue>> Structs;
 void Add(UStruct* S) { if(S&&!Seen.Contains(S)){Seen.Add(S);Pending.Add(S);} }
 TSharedPtr<FJsonObject> Type(FProperty* P)
 {
  auto J=MakeShared<FJsonObject>();
  J->SetStringField(TEXT("type"),P->GetClass()->GetName());
  if(auto* V=CastField<FStructProperty>(P)){J->SetStringField(TEXT("structType"),V->Struct->GetName());Add(V->Struct);}
  if(auto* V=CastField<FArrayProperty>(P))J->SetObjectField(TEXT("innerType"),Type(V->Inner));
  if(auto* V=CastField<FSetProperty>(P))J->SetObjectField(TEXT("innerType"),Type(V->ElementProp));
  if(auto* V=CastField<FMapProperty>(P)){J->SetObjectField(TEXT("innerType"),Type(V->KeyProp));J->SetObjectField(TEXT("valueType"),Type(V->ValueProp));}
  if(auto* V=CastField<FEnumProperty>(P)){Enums.Add(V->GetEnum());J->SetStringField(TEXT("enumName"),V->GetEnum()->GetName());J->SetObjectField(TEXT("innerType"),Type(V->GetUnderlyingProperty()));J->SetBoolField(TEXT("isEnumAsByte"),false);}
  if(auto* V=CastField<FByteProperty>(P)){if(V->Enum){Enums.Add(V->Enum);J->SetStringField(TEXT("enumName"),V->Enum->GetName());J->SetBoolField(TEXT("isEnumAsByte"),true);}}
  return J;
 }
 void Run()
 {
  for(int32 N=0;N<Pending.Num();++N)
  {
   UStruct* S=Pending[N]; Add(S->GetSuperStruct());
   auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("name"),S->GetName());
   J->SetStringField(TEXT("sourcePath"),S->GetPathName());
   if(S->GetSuperStruct())J->SetStringField(TEXT("superType"),S->GetSuperStruct()->GetName());
   TArray<TSharedPtr<FJsonValue>> Props; int32 Index=0;
   // Exactly UE's cooked schema order, retaining transient slots too.
   for(FProperty* P=S->PropertyLink;P;P=P->PropertyLinkNext)
   {
    if(P->GetOwnerStruct()!=S||P->IsEditorOnlyProperty())continue;
    auto Q=MakeShared<FJsonObject>();Q->SetNumberField(TEXT("index"),Index);Q->SetStringField(TEXT("name"),P->GetName());Q->SetNumberField(TEXT("arraySize"),P->ArrayDim);Q->SetObjectField(TEXT("mappingType"),Type(P));
    Q->SetStringField(TEXT("propertyFlagsHex"),FString::Printf(TEXT("%016llx"),(uint64)P->GetPropertyFlags()));
    Props.Add(MakeShared<FJsonValueObject>(Q));Index+=P->ArrayDim;
   }
   J->SetNumberField(TEXT("propertyCount"),Index);J->SetArrayField(TEXT("properties"),Props);Structs.Add(MakeShared<FJsonValueObject>(J));
  }
 }
};
bool Save(const FString& Path,const TArray<TSharedPtr<FJsonValue>>& V)
{
 FString Out;auto W=TJsonWriterFactory<>::Create(&Out);FJsonSerializer::Serialize(V,W);
 return FFileHelper::SaveStringToFile(Out,*Path,FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}
}
UTempestMappingsProbeCommandlet::UTempestMappingsProbeCommandlet(){IsClient=false;IsServer=false;IsEditor=true;LogToConsole=true;}
int32 UTempestMappingsProbeCommandlet::Main(const FString& Params)
{
 FString Dir; if(!FParse::Value(*Params,TEXT("Output="),Dir)){UE_LOG(LogTemp,Error,TEXT("Explicit -Output directory required"));return 1;}
 Dir=FPaths::ConvertRelativePathToFull(Dir);IFileManager::Get().MakeDirectory(*Dir,true);
 FString ClassNames=TEXT("/Script/Engine.StaticMesh,/Script/Engine.Texture2D,/Script/Engine.MaterialInstanceConstant");FParse::Value(*Params,TEXT("Classes="),ClassNames);
 TArray<FString> Names;ClassNames.ParseIntoArray(Names,TEXT(","),true);FProbe P;
 for(const FString& Name:Names){UStruct* S=LoadObject<UStruct>(nullptr,*Name);if(!S){UE_LOG(LogTemp,Error,TEXT("Missing requested reflected type %s"),*Name);return 2;}P.Add(S);}
 P.Run();TArray<TSharedPtr<FJsonValue>> EnumRows;
 for(UEnum* E:P.Enums)
 {
  if(!E)continue;auto J=MakeShared<FJsonObject>();J->SetStringField(TEXT("name"),E->GetName());TArray<TSharedPtr<FJsonValue>> Values;
  for(int32 I=0;I<E->NumEnums();++I){auto V=MakeShared<FJsonObject>();V->SetStringField(TEXT("name"),E->GetNameStringByIndex(I));V->SetStringField(TEXT("value"),LexToString(E->GetValueByIndex(I)));Values.Add(MakeShared<FJsonValueObject>(V));}
  J->SetArrayField(TEXT("numericValues"),Values);EnumRows.Add(MakeShared<FJsonValueObject>(J));
 }
 if(!Save(Dir/TEXT("structs.json"),P.Structs)||!Save(Dir/TEXT("enums-numeric.json"),EnumRows))return 3;
 UE_LOG(LogTemp,Display,TEXT("TEMPEST_MAPPINGS_PROBE PASS structs=%d enums=%d output=%s"),P.Structs.Num(),EnumRows.Num(),*Dir);return 0;
}
