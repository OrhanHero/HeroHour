# Aegis factory V10 preparation

Prepared original HeroHour factory for a serial Unreal import; no UnitDefinition, runtime source, map or other agent files changed.

The source is the genuinely generated, PBR-reviewed V9 candidate. SHA256: `d26273577175ed8a556de00d73fe7082dba07d695329f272e0bb6ad2e01151a9`. Source bytes and both embedded texture exports are preserved. Existing source PBR views A/B/C and wireframe views remain under `Art/Models/TrellisFactoryV9-Candidate`.

`prepare_trellis_factory_v10.py` imports the GLB, uniformly scales its longest horizontal dimension to 6.2 m, rotates 180 degrees around Z, centers XY and grounds the source minimum at -0.35 m. Source garage front is Blender -Y; prepared front is Blender +Y and Unreal -Y using the proven FBX axes. At runtime scale 1.7, dimensions are 1054 x 827.64 x 342.77 cm. Actor35 + mesh-relative24.5 + source-ground(-35)*1.7 = 0 cm. The 1054 cm width fits the 1080 cm footprint diameter.

CPU Blender export and FBX reimport passed bounds agreement below 1 mm, Team material slot, one UV channel, custom normals, unchanged vertex/polygon counts across the FBX roundtrip. Prepared geometry has 485694 vertices and 699446 triangles. Structural GLB evidence counts 699468 triangles: the 22-triangle difference occurs on GLB import, its cause has not been independently established. No decimation or mesh modifiers were applied.

Textures are real 2048-square Albedo and MetallicRoughness, byte-identical to the generated candidate. The Unreal importer enables Nanite and creates `M_FV10_Baked_AegisFactory` with white Tint, linear MR G=roughness/B=metallic, AO constant1 and no emission. It imports authored normals/tangents. It snapshots both faction Factory definitions and verifies they are unchanged. It never activates the new model.

Root must run `import_trellis_factory_v10.py` serially in Unreal and inspect actual ground, front, material, visual silhouette and Nanite rendering before any definition switch. No Unreal import or new GPU render is claimed in this preparation. The runtime material-name recognition also requires explicit integration by root; this preparation does not modify it.

Evidence: `Art/Models/TrellisFactoryV10/readiness-manifest.json`, `SM_FV10_AegisFactory.json`, CPU log `Saved/Logs/FactoryV10-Prepare.log`. Python compile and actual Pillow dimensions/hash checks passed.

## Pending UI portrait

`render_trellis_factory_portrait_v10.py` rendered the actual FV10 mesh and PBR in the proven orthographic 512 RGBA UI style. Root reviewed no UI activation in this pass: `Art/Models/TrellisFactoryV10/Portrait-Pending.png` is the sole new portrait output. The active factory image and the complete active metadata file remained byte-identical, asserted in the render script. Front garage and blue service wing are readable in the directly inspected render.

`docs/verification/2026-10-16/reference-v10/factory-portrait/ProposedRecord.json` holds only the proposed factory metadata record. `Preserved21.json` records the other 21 current image hashes and metadata records. Once runtime factory activation is accepted, root must take a fresh preservation snapshot, copy the pending image to the existing factory image path, and replace only its metadata record before invoking `import_trellis_factory_portrait_v10.py`. The importer is prepared but was not run. It preserves the existing texture asset path and checks the Factory definition mesh/icon/stats remain unchanged during reimport.

## Runtime acceptance additions

The independent V10 Factory block in `HRPresentationTests.cpp` adds nine checks per Presentation run, without changing existing assertions: selected-faction/definition/live mesh identity; source bounding box; final ground contact; actual 1.7 scale and placement envelope; single exact saved/live Team material and neutral Tint/no invented EmissionMask; two actual saved/live Albedo and MetallicRoughness 2048 bindings and color spaces; real damage flash; restoration to original white Tint. Thus an existing 168-check run becomes 177 checks when all previous branches execute. Tests have been added but not yet built or executed by this agent. Root owns compilation and execution.

Runtime `PrototypeGameMode.cpp` already recognizes the `M_FV10_Baked_` prefix for authored neutral atlas tint; read-only inspection confirmed it. No runtime source was edited in this pass.
