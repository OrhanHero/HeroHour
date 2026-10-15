# Original ReferenceV7 surface atlases

Generated only from HeroHour's own ReferenceV7 mesh geometry and analytic Blender nodes. No third-party game texture extraction or source images are involved.

Reproduce with `Scripts/art/bake_reference_surfaces_v7.py` through Blender 5.2. APC maps are 1024²; HQ maps are 2048². Sources remain read only. `metadata.json` records source/bake/output SHA-256 values, source preservation, channel color spaces, actual dimensions, bake timing and validation ranges.

For each model: `Albedo` is sRGB; `ORM` is linear R=contact AO / G=roughness / B=metallic; `EmissionMask` is linear R, restricted to luminous surfaces. Metallic applies only to Steel, and geometry-derived bottom dust and edge wear preserve the faction palette. `Preview_*.png` are actual mesh renders using these baked PNGs.

The separate Unreal importer creates only model-local `M_RV7_Baked_*` materials, textures and saved mesh-material bindings. Full workflow, paths and limitations are in `docs/research/ReferenceSurfacesV7.md`.
