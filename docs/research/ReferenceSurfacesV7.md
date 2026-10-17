# Reference V7 — original UV-baked surfaces

## Purpose and source contract

The V7 HQ/APC silhouettes used clean single-color surfaces. This pass supplies model-local paint variation, convex-edge wear, crevice/contact AO and lower-body desert dust, while retaining the concept's white/blue Aegis HQ, tan Aegis APC and graphite/green Ember identity.

`Scripts/art/bake_reference_surfaces_v7.py` opens the original exported `.blend` read only and uses its existing Smart-UV channel 0. It never saves that file or changes its FBX. SHA-256 values before/after are asserted equal. The manifest records both source hashes, the bake-script hash, each output texture hash, dimensions, color spaces, backend and measured bake ranges. No Tempest Rising, C&C, downloaded or extracted game textures are used.

Only final sources may be selected. APCs are the default command; HQs require explicit selection after the integration owner releases their geometry:

```powershell
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background --python Scripts/art/bake_reference_surfaces_v7.py -- --only Aegis_APC8 Ember_APC8
# After final HQ-source release:
& 'C:\Program Files\Blender Foundation\Blender 5.2\blender.exe' --background --python Scripts/art/bake_reference_surfaces_v7.py -- --only Aegis_HQ Ember_HQ
```

The manifest merges selected records, so rebaking one model preserves the other models' evidence. The importer verifies every listed source and texture hash before changing Unreal packages; changed geometry requires a fresh bake.

## Actual surface construction

- AO is Cycles ray-traced mesh contact AO, with a short 17 cm APC / 42 cm source-HQ radius and 8 shader samples. Full AO is stored separately; only a restrained 10% contact term is embedded in albedo.
- Wear uses the mesh's convex pointiness, broken into sparse flecks by bounded noise. Broad flat surfaces are not covered with a random scratch filter. Exposed paint edges differ from intact painted faces.
- Paint color variation is limited to 0.94..1.04 times the palette. Roughness varies by ±0.025 before a small dust contribution. Noise is baked once, with no runtime procedural shader noise.
- Dust uses actual height above each mesh's minimum bound, multiplied by coarse noise; tires and lower armor collect more than high surfaces. Glass and luminous markings receive only a very small contribution.
- Metallic is 0.78 exclusively for the named `Steel` surface. Painted armor, concrete, rubber, glass and colored markings remain dielectric. Glass has lower roughness; rubber has high roughness.
- Green Ember identifiers use linear `(0.025,0.32,0.13)`, green luminous parts `(0.025,0.70,0.28)`, and dark green glass `(0.012,0.055,0.032)`. Gameplay enemy health bars, rings and UI remain separate from the model's art palette.

## Texture and Unreal material contract

Each APC has three 1024² PNGs; each HQ has three 2048² PNGs. The keys are `Aegis_APC8`, `Ember_APC8`, `Aegis_HQ`, `Ember_HQ`.

| Channel | Source file | Unreal texture settings | Material connection |
|---|---|---|---|
| Albedo | `T_RV7_{key}_Albedo.png` | sRGB, TC_DEFAULT | RGB multiplied by `Tint`, default white |
| ORM | `T_RV7_{key}_ORM.png` | non-sRGB, TC_MASKS | R=AO, G=roughness, B=metallic |
| EmissionMask | `T_RV7_{key}_EmissionMask.png` | non-sRGB, TC_MASKS | R × albedo × 2, restricted to Neon/Lamp geometry |

`Scripts/art/import_reference_surfaces_v7.py` creates textures under `/Game/HERO/ReferenceV7/Textures/` and one independent material per model under `/Game/HERO/ReferenceV7/Materials/M_RV7_Baked_{key}`. The texture parameter names are `Albedo`, `ORM`, `EmissionMask`; UV coordinate index is explicitly 0. It binds that model's whole atlas to all existing mesh slots without renaming/reordering slots or changing geometry, definition references, stats or icons. Existing shared materials are untouched.

The `M_RV7_Baked_` prefix deliberately bypasses `M_CV2_` faction material replacement. Runtime must preserve neutral white initial/restore Tint for these baked atlases; a 2× white flash supplies the existing temporary hit feedback. Applying faction paint Tint to the whole baked atlas would incorrectly tint already colored surfaces twice. The integration owner implements that small runtime contract separately.

## Inspection, evidence and limits

The script renders `Preview_{key}.png` using the **actual baked PNGs** on the original mesh, rather than rendering only the procedural source nodes. All four final previews were opened and inspected: low tire/body dust, shaded seams and the steel/rubber/paint distinction are visible; wear is restrained. Ember HQ retains the faceted graphite silhouette, green channels and dark green window strip. The original model files remain unchanged. All four records are `PASS`: final per-model OPTIX bake-and-render times were 6.39 / 12.26 seconds for Aegis/Ember APC and 16.37 / 10.47 seconds for Aegis/Ember HQ. Coverage including dilation is approximately 24.0% / 22.9% for APCs and 12.3% / 20.2% for HQs. The script can otherwise use CUDA/HIP/oneAPI or bounded 8-thread CPU fallback; those fallback runtimes are not measured.

The source HQ Smart-UV layout contains many small islands and substantial padding. Aegis HQ's measured atlas footprint is about 12% including dilation, so even a 2K map does not provide the detail density of a hand-unwrapped production asset. Tiny details and narrow wear lines can soften under mipmapping. This pass does not add a normal atlas, authored decal storytelling, unique painted signage, LODs or animated parts. The studio renders establish that real textures were baked and used; fresh Unreal and packaged captures must still confirm color, UV orientation, emission, filtering and damage-flash restore before visual acceptance. All mesh sections are preserved, so this does not claim fewer draw calls.
