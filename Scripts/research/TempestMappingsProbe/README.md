# Bounded UE 5.4 reflection-to-CUE4Parse mappings probe

Built and executed by root on 17 October 2026 with UE5.4.4 CL35576357: 36 structs and 11 enums exported, process Exit 0. No DLL injection is involved: this is a normal editor plugin commandlet using the compatible engine's reflected types. It was installed only in the disposable ModKit copy, never in the original. The first newer-MSVC build failed on UE5.4's missing `__has_feature`; a probe-local definition and no-PCH mode fixed the build without modifying the engine.

Copy this plugin directory to `<disposable project>/Plugins/TempestMappingsProbe` and enable it in that copy. Build the editor plugin through UBT/the editor target; root must choose the target that the copied project supports. Run the matching UnrealEditor-Cmd.exe with:

```text
"<copy>/Tempest.uproject" -run=TempestMappingsProbe -Output="<HeroHour>/Saved/Research/TempestMappings54" -unattended -nullrhi -nosplash -nop4
```

Default roots: `/Script/Engine.StaticMesh`, `/Script/Engine.Texture2D`, `/Script/Engine.MaterialInstanceConstant`. `-Classes="/Script/Package.Type,..."` replaces these roots and can request actual reflected Tempest types once their module has loaded. Parents and nested struct properties are recursively included. The probe neither opens map assets nor saves game assets.

`structs.json` follows CUE4Parse.MappingsProvider.JsonTypeMappingsProvider.AddStructs: name, superType, propertyCount, properties(index,name,arraySize,mappingType). Unknown metadata fields sourcePath/propertyFlagsHex aid verification and are ignored by that parser. Mapping types recursively supply type/structType/innerType/valueType/enumName/isEnumAsByte.

Critical indexing: actual UE5.4 UnversionedPropertySerialization.cpp iterates PropertyLink in serialization order, expands ArrayDim and skips IsEditorOnlyProperty for cooked data. It does **not** remove transient slots. Export only properties whose owner is the current struct, and let CUE4Parse append its mapped super type. No alphabetic sorting or guessed offsets.

`enums-numeric.json` deliberately preserves enum numeric values as strings to avoid 64-bit JSON precision loss. Do not pass that file to AddEnums, whose implementation assumes contiguous zero-based values. A small JsonTypeMappingsProvider subclass can call AddStructs, then populate `MappingsForGame.Enums[name]` with `Dictionary<long,string>` parsed from each numericValues value/name. Implement abstract Load(path), Load(bytes), Reload around the files; no Usmap conversion is required.

The generated mappings were then actually used by CUE4Parse to decode/export the cooked GDF container and its textures. This establishes compatibility for that bounded candidate, not every Tempest class or asset. Native engine properties cooked by a custom game build may still differ even when UE CL matches. The plugin records actual editor reflection, not an invented schema.

The preparing agent wrote source only; root performed the subsequent build, engine invocation and candidate decode. See `docs/research/Tempest-Container-Transfer-2026-10-17.md`.
