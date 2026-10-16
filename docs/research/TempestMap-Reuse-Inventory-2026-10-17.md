# Tempest map reuse: local evidence

Date: 17 October 2026. Read-only bounded inspection. No game/editor launch, asset export, extraction, migration, copying, encryption bypass or installation changes.

## Verified installation

Root: `F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject/`.

- `README.txt` explicitly requires **Unreal Engine 5.4.4**.
- `Tempest.uproject` associates engine `5.4` and declares native `Tempest` runtime and `TempestEditor` editor modules.
- `Binaries/Win64/TempestEditor.target` records version **5.4.4**, changelist **35576357**, compatible changelist **33043543**, branch `++UE5+Release-5.4`.
- `Binaries/Win64/UnrealEditor-Tempest.dll` and `UnrealEditor-TempestEditor.dll` are supplied. Project `Source/` is absent. `Plugins/NavGrid/Source/` and `Plugins/GameplayCommands/Source/` are also absent at the checked paths.
- HeroHour's current `HeroHour.uproject` associates **UE 5.8** and its own `HeroHourPrototype` runtime module.

## Existing maps to use rather than invent a new reference

**Best visual baseline: SetdressingExampleMap**, described by the supplied manual, page 1, as a complete example demonstrating environment dressing techniques used in Tempest Rising levels.

Paths relative to the ModKit root:

```
Mods/SetdressingExampleMap/Content/Maps/Multiplayer/4Player/SEM/
  MAP_MP_SetdressingExampleMap_4P_P.umap          32,798,442 bytes
  MAP_MP_SetdressingExampleMap_4P_Enviro.umap     97,549,126 bytes
  MAP_MP_SetdressingExampleMap_4P_Foliage.umap        18,577 bytes
  MAP_MP_SetdressingExampleMap_4P_Nav.umap           706,075 bytes
```

The same directory includes landscape, materials, shared assets, foliage, map metadata and a minimap terrain texture. The split persistent/environment/foliage/navigation layout is observable in files; successful editor loading has not been tested in this pass.

**Systems reference:**

`Mods/ZooExampleMap/Content/Maps/Multiplayer/2Player/MP_ZooExampleMap_2P/MAP_MP_ZooExampleMap_2P_P.umap`.

**Templates:** `Content/Tempest/Maps/Templates/MAP_Template_{Small,Medium,Large}.umap`.

## What the editor actually supplies

Previously extracted local `Docs/manual.pdf` text is available at `Saved/Research/TempestRising/manual-pages.json`; page numbering below is one-based PDF numbering. Original PDF hash and prior rendered checks are documented in `TempestRising-MapEditorReview.md`.

- Page 1: the supported output is a **custom map mod for Tempest Rising**, not a standalone source project. Mods may include custom assets and Blueprints, but cannot contain code; other mod types and custom campaigns are not properly supported.
- Page 27: some game assets in `Content/Tempest` are explicitly **cooked**. Map mods reference assets by paths matching the installed game; the assets need not be included in the cooked mod. Some editor placeholders are resolved to actual assets only when loaded by the game.
- Page 28: packaging cooks mod assets/maps to the Tempest mod output directory; this is not documented as a standalone project export.
- Page 29: the map editor project does **not** include gameplay-related code; Play In Editor does not run a playable match.
- The filesystem contains `.uexp` and `.ubulk` companions in `Content/Tempest`, consistent with the manual's cooked-content statement. No individual map's cooked/editor flags were parsed, so the report does not classify every `.umap` as uncooked.

## Feasibility and next concrete step

1. Use the supplied **SetdressingExampleMap in the matching UE 5.4.4 ModKit** as the authoritative visual reference and map-mod editing baseline. This is a real existing map, not a guessed recreation.
2. Record its terrain, lighting, material layering, prop density, playable lanes and camera view. Use ZooExampleMap separately for feature demonstrations.
3. Before claiming migration to HeroHour, inspect actual actor/component dependencies and editor export support in the matching engine. Current evidence does **not** establish an export of a self-contained, editable HeroHour map.
4. HeroHour UE 5.8 cannot simply load the supplied **5.4 native DLLs** as its own modules. Source is absent at the inspected paths; actor/material/Blueprint dependencies, cooked assets and placeholders make copying the `.umap` insufficient. Compatibility and dependency replacement need a tested conversion path.

There is no evidence here that the ModKit supplies the complete Tempest Rising source or an independently runnable RTS framework. A successful map-mod workflow and a successful standalone HeroHour migration are different deliverables. Reuse permissions for distribution in a separate game were not established by this technical inventory.
