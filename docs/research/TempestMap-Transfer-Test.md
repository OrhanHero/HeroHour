# Tempest map: concrete inspection and transfer gate

17 October 2026. **Prepared, not executed in Unreal.** No editor launch, asset migration or map conversion has occurred in this task. Python syntax validation is not an Unreal API/runtime test.

## Actual starting point

The supplied manual, PDF page 1, identifies SetdressingExampleMap as a complete environment example. Its persistent, environment, foliage and navigation maps exist under `Mods/SetdressingExampleMap/Content/Maps/Multiplayer/4Player/SEM/`. Use that scene rather than creating a hypothetical substitute. The environment art guide documents the actual landscape, environment, prop, decal, foliage, collision and runtime virtual texture workflows; these are interconnected dependencies, not just a list of interchangeable static meshes.

Manual pages 27–29 distinguish cooked game content, editor placeholders, map-mod packaging and absent gameplay in PIE. None proves that a particular landscape or mesh cannot transfer. Each dependency needs an individual result.

## Read-only inspection script ready now

`Scripts/research/inspect_tempest_map_editor.py` inventories the open world, loaded actor classes/transforms, mesh/material references, instance counts, streaming level visibility and package references. It writes JSON only beneath HeroHour's Saved directory. It does not save assets, duplicate objects, export geometry, migrate content, change sublevel visibility or enable plugins.

Use a disposable copy of the ModKit in **UE 5.4.4**, with Python Editor Script Plugin available. Do not enable a missing plugin in the original project as part of this inspection. Open the persistent Setdressing map manually first; this makes plugin mount names visible rather than guessing them from disk paths. In the Unreal Python console:

```python
import os
os.environ['TEMPEST_INSPECTION_OUTPUT'] = 'C:/Users/Hero/HeroHour/Saved/Research/TempestRising/MapTransfer'
__file__ = 'C:/Users/Hero/HeroHour/Scripts/research/inspect_tempest_map_editor.py'
exec(compile(open('C:/Users/Hero/HeroHour/Scripts/research/inspect_tempest_map_editor.py', encoding='utf-8').read(), 'C:/Users/Hero/HeroHour/Scripts/research/inspect_tempest_map_editor.py', 'exec'))
```

The console example explicitly supplies the script path through `__file__`. Running the script through the editor's Execute Python Script command supplies that normally.

An optional `TEMPEST_INSPECTION_LOAD_MAP` package path requests a map load; the script refuses replacement when dirty packages exist. Default operation does not replace the open map. Expected marker: `TEMPEST_MAP_INSPECTION status=PASS ...`. `PASS_WITH_API_GAPS` means listed queries failed and must be repaired before relying on those fields. `FAIL` remains a failed inspection, not a usable transfer result. Every report has a unique timestamp and records errors.

## The first transfer experiment, after the inventory

1. Select a bounded, recognizable environment section containing landscape, rocks/props, a decal and representative foliage. Record the real camera and reference screenshot. Pick it from the existing scene, not a newly composed mock-up.
2. Classify that section's dependencies: standard Unreal classes; Tempest classes; editable source assets; cooked/placeholder assets; unresolved references. Record evidence per item rather than assigning the whole map one unsupported label.
3. In a separate **5.4.4 sandbox**, test the editor's standard migration path only for assets actually shown to be editable. This step is not performed by the inspection script. Preserve reference paths and record any failed load/compile warnings.
4. Replace only demonstrated Tempest gameplay dependencies with HeroHour equivalents. Initially validate environment appearance independently of player starts, harvesting and combat.
5. Open a separate copy in HeroHour's engine version only after the 5.4.4 sandbox loads. Compare terrain silhouette, material layers/RVT, lighting, foliage density and collisions from the same camera. Do not convert either original project in place.
6. If a dependency is cooked or editor-only, record exactly what failed and investigate that boundary through REA. Do not report successful migration because the persistent map opens while its environment or materials are absent.

## Acceptance evidence

The first milestone is one **loaded and visually matched real map section**, with a report of missing/replaced dependencies and no destructive edits to the original. Then prove HeroHour unit navigation and resource placement in that section. A map-mod packaging success alone is not evidence of standalone HeroHour compatibility. UI direction remains Zero Hour layout with Tempest surface styling; this inspection does not change UI sources.
