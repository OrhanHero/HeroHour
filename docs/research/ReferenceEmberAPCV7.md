# Reference Ember APC V7 — original graphite 8×8

## Reference and actual inspection

Opened the supplied `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_20.png` and inspected its lower-left graphite eight-wheel vehicle before modelling. Also read the Aegis APC source and its validation report to preserve the existing runtime dimensions and material contracts. The reference's broad low armored envelope, eight tires, small upper weapon station and narrow viewing apertures replace the old cabbed utility-vehicle reading.

This is independently authored geometry, not an Aegis recolor. A six-sided belly/shoulder hull terminates in a clipped sloping prow; four real Boolean wheel cavities on each side expose eight tires. Separate canted shoulder blocks, a low wide unmanned turret, paired raised sensor pods, short offset autocannon plus coaxial barrel, rear troop ramp and two separate engine grilles distinguish it from the longer Aegis hull and taller manned turret. There is no large pickup windscreen or open cargo bed.

The actual exported mesh was rendered and `Art/Models/ReferenceEmberAPCV7/SM_HR_Ember_APC8.png` inspected after export. All four near-side wheels and the far-side front wheel read clearly; the continuous armored hull and flat remote turret are visibly different from the Aegis model. The studio uses the existing Ember graphite armor palette, which appears lighter under broad studio lighting than the supplied dark battlefield illustration. The supplied left-hand concept explicitly establishes **graphite with green accents**. The final source uses green identifiers, green tail lamps and small emissive green sensor status strips. Gameplay enemy identification remains in the existing red health bars, rings and HUD; green model paint does not change ownership or targeting.

## Deliverables and reproduction

- `Scripts/art/build_reference_ember_apc_v7.py`: original Blender 5.2 procedural source, using only the repository's shared primitive/export helpers. It does not import the Aegis build script or third-party game geometry.
- `Art/Models/ReferenceEmberAPCV7/SM_HR_Ember_APC8.blend`: editable mesh source, saved before the temporary studio setup.
- The matching `.fbx`, `.json` and `.png`: game mesh, validation/provenance data and actual Cycles render.
- `Scripts/art/import_reference_ember_apc_v7.py`: imports `/Game/HERO/ReferenceV7/Meshes/SM_HR_Ember_APC8` and assigns only `D_Ember_Combat.static_mesh`. Existing icon and gameplay stats are asserted unchanged. The agent did not run Unreal or modify any gameplay source.

Reproduce with Blender 5.2 `--background --python Scripts/art/build_reference_ember_apc_v7.py`. The integration owner then runs the importer through Unreal Python. The importer checks the FBX SHA-256, metadata PASS, dimensions and ground offset before updating the data asset. Graphite armor and neutral surfaces bind existing `M_CV2_*` semantic slots and keep the normal runtime Ember PBR selection. Two dedicated material instances preserve the concept's green accents:

- `/Game/HERO/ReferenceV7/Materials/M_RV7_Ember_TeamGreen`, parent `M_HR_Ember_Team`, linear Tint `(0.025, 0.32, 0.13)`.
- `/Game/HERO/ReferenceV7/Materials/M_RV7_Ember_NeonGreen`, parent `M_HR_Ember_Neon`, linear Tint `(0.025, 0.70, 0.28)`. Emission is inherited from the existing Neon parent.

Their names deliberately omit the `M_CV2_` prefix, so `ApplyFactionSurfaces()` leaves them green instead of replacing them with the red prototype material. These shared material names can also be used by the Ember HQ importer. No health-bar, ring, UI, team-index or gameplay color code is edited by this art pass.

## Validation

Blender log: `Saved/Logs/ReferenceEmberAPCV7-Blender.log`, final marker `REFERENCE_EMBER_APC_V7_PASS`.

- 20,450 triangles, 10,883 vertices, one exported static mesh, including the two new emissive sensor status strips.
- Actual dimensions: 347.400 × 187.800 × 190.100 cm. Width stays below the established 188 cm visual envelope; no claim is made that visual wheel width equals the navigation agent radius.
- Source forward is +X, wheel axles run along Y. Tire tread minimum Z is exactly −35 cm within floating-point precision, matching runtime actor Z +35 cm and scale 1.
- Eight separately authored tires and eight successfully applied Boolean arch cuts are counted before joining.
- Actual source bounds: X −175.500..171.900 cm, Y −93.900..93.900 cm, Z −35.000..155.100 cm.
- Independently reloads the FBX into a cleared Blender scene and compares every axis extremum with the exported source within 2 mm. Its symmetric outer Y envelope is checked despite the export convention's Y reflection.
- JSON records the build-script, helper, supplied reference image and final FBX SHA-256 values. The geometry is original procedural modelling with no extracted or downloaded assets.

## Remaining limits and integration gate

This is a static RTS prototype mesh. Wheels, suspension, turret and ramp have no animation; it has no authored LOD chain or unique baked wear atlas. The exported mesh consists of joined disconnected closed components, with deliberately deep wheel cavities. The separate portrait process must render the new mesh before the UI thumbnail can match. Final Unreal faction surfaces, ground contact, muzzle alignment and the actual packaged gameplay view still require the integration owner's verification. The studio render and Blender bounds PASS alone do not establish complete concept parity.
