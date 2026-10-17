# Tempest container: actual material graph, 17 October 2026

This is read-only static evidence from the user's supplied ModKit copy. No editor
was launched for this task, no native session opened, no installed asset changed,
and no game textures or geometry are added to Git. REA skill applied to the shipped
artifact; CUE4Parse is the focused package reader, not evidence of executed gameplay.

## Reproducible result

The previous root-object inspection omitted the material expressions. The new
`inspect-package` option in `Scripts/research/TempestAssetProbe/Program.cs` serializes
all exports of the selected package. The original batch-object export path remains
unchanged. It successfully decoded **506 master-package exports and 4 instance
exports**, including editor-only material graph and instance static switches.

Local evidence (not production Content):

- `Saved/Research/TempestAssetTransfer/Container-MasterPackage-01/Package-00.json`
  SHA256 `58b420b7f67470aec3829a86fe6b0c89594c93f6cbd009aae2093595d43d6966`.
- `Saved/Research/TempestAssetTransfer/Container-MIPackage-01/Package-00.json`
  SHA256 `e8a17d9689670e8e388c64f16779ab4b2be3650be50d64811bcf316dbd3631b8`.
- `Saved/Research/TempestAssetTransfer/Container-MasterPackage-01/Graph-Evidence.json`.
- Validator `Scripts/research/inspect_tempest_material54.py`: **18 graph-edge checks
  PASS**. It checks original indices and output sockets, not texture-name guesses.

Source: `/Game/Tempest/MaterialLibrary/Props/M_MASTER_Props.M_MASTER_Props`, with
instance `/Game/Tempest/Props/Faction_Containers/Materials/MI_GDF_Container_01`.
Reflection mappings SHA256:
`9c8113e82d047e566095b8e4bb1df7445d8af1c49a381fb6d863e7e62bae295f`.

## Observed channel assignment

All numbers below are **export-array indices** in the master package, not names
guessed from RMO abbreviations.

| Channel | Actual graph | Meaning and limitation |
|---|---|---|
| R | Texture #460 output1/MaskR -> Lerp #117, endpoints Roughness Min #364 / Max #365 -> Saturate #287 -> Append3 #156.A | Roughness base is `saturate(lerp(min,max,R))`; defaults 0 and 1, neither overridden by this instance. Rain, vertex wetness and snow modify the final roughness later. |
| G | Texture #460 output2/MaskG -> Append3 #156.B -> SplitComponents #128 output2 -> Reroute #249 -> #230 -> #233 -> final snow Lerp #113.A -> #411 | Metallic base is G. Snow can blend toward metallic 0; direct G is only the dry/no-snow base. |
| B | Texture #460 output3/MaskB -> Append3 #156.C -> SplitComponents #128 output3 -> #241 -> #229 -> #256 -> AmbientOcclusion root | Ambient occlusion is B. It also participates in albedo shading, not just the AO material input. |
| A | Texture #460 output4 -> switch #432.B | Optional emissive path; not proof that this container's RMO image has usable emissive alpha or that emission is active. |

`#128` explicitly references the Engine `SplitComponents` function and lists its
outputs as RGB, R, G, B. `#156` is `MaterialExpressionMaterialXAppend3Vector`.

## Observed albedo / color-mask recipe

The instance enables **Use Color Mask** and **Use Seperate RGB Channels for Color
Mask** (original spelling retained). Consequently plain D texture is not the final
base color.

1. D texture #459 -> multiply #179 by **Albedo Brightness = 1.2** (#346)
   -> saturate #293 -> desaturate #91 (parameter #345, no instance override)
   -> multiply #178 by **Albedo Tint = (0.432292, 0.418660, 0.357937)** in linear color.
2. Color texture #467 -> SplitComponents #142. Its R/G/B separately multiply
   Color Mask R/G/B Color (#488/#489/#490), then add (#15/#16).
3. That sum multiplies #187 by `saturate(desaturate(D) * 0.8)` (#89/#207/#289),
   using **Base Albedo Brightness for Color Mask = 0.8**.
4. #125 blends tinted base and recolored base. Its alpha is #267 -> #296,
   the saturated RGB component sum from Engine AddComponents #141 output1.
5. #116 blends this result with itself multiplied by AO (#182), using saturated
   **Albedo AO = 0.5** (#353). This is an AO interpolation, **not** blindly `D * AO`.

Instance mask colors (linear): R `(0.501562, 0.515334, 0.557292)`,
G `(0.421875, 0.229058, 0.084375)`, B uses master default `(1,1,1)`.
Do not substitute the serialized `Hex` preview values for linear vectors.

## Observed normal recipe

Normal Map #462 -> ComponentMask #56 selects RG -> Multiply #175 by
**Normal Map Intensity** #341 (master default 1, not overridden) -> AppendVector
#25 appends the sampled normal's B. Thus strength affects XY while preserving Z
before subsequent optional detail/custom, rain and snow stages. This does **not**
show a green-channel inversion. A generic PNG color sample is not equivalent to
an Unreal normal-map sampler.

## What a faithful reconstruction still requires

The instance also enables **Add Grunge (UV)** and **Dynamic Snow**. Master Rain
defaults true. Grunge enters through function call #129; rain/wetness through
#130 and MPC `MPC_DayAndNight`; snow through #131 and additional mask logic.
Instance overrides: Grunge Tex Power 2, Grunge Tile 4, Grunge Amount 0.75,
Snow Mask Power 4, Snow Mask Multiplier 0.75, Top Snow Mask Power 6.

These dependent material functions and the original map's live MPC values were
not decoded/rendered in this task. A dry baseline can deliberately omit those
features, but must be labelled **baseline**, not full original-shader parity.
The instance's Emissive Power 100 is observed; it alone does not prove active
emission because static branches and mask inputs determine whether it contributes.

The recovered graph exposes ordinary Engine material expressions; this material
task does not demonstrate missing Tempest native-class schemas. Native gameplay
or map Actor schemas are a separate investigation. No shader visual-equivalence
test has yet been performed by this task.

## Next concrete implementation

Build the evidenced dry material in the isolated UE5.8 test project: correct
linear RMO sampler, normal sampler, explicit color-mask recipe and AO blend.
Compare against a UE5.4 original-material render using matched lighting/camera.
Then decode #129/#130/#131 functions and MPC settings as needed. Root coordinates
all Unreal jobs; this agent has only prepared and validated offline evidence.

## Follow-up: original sector material inheritance

`resolve_tempest_material_bindings54.py` now walks the actual child-to-parent
material references and exports inherited textures. Successful final manifest:
`Saved/Research/TempestSectorTransfer/MaterialBindings-04/Bindings.json`.
**9/9 materials have evidenced Albedo and Normal bindings; 28 unique original
textures were exported** (also RMO and color/player masks). Parent chains contain
2 or 3 objects. Checks confirmed all exported PNG paths exist.

The seven apparently missing bindings were not seven missing albedo assets:

- AirControlTower and WarFactory cooked instances expose property names with
  schema indices, e.g. `Parent[9]`, `TextureParameterValues[16]`. The resolver
  normalizes only in-memory lookup keys, rejects collisions and leaves source
  JSON intact. Their own original structure D/N textures resolve successfully.
- CoarseMetal black/01 inherit `MI_TrimsCoarseMetal` textures.
- HarbourCrane light/grey inherit `MI_TrimsHarbourCrane` textures.
- RepairBay has no Albedo instance override; its real master-graph default is
  `T_BaseWhite_D`. This is an observed default, not an invented replacement.

The isolated sector import script reads this final manifest and uses exact PNG
paths. Missing albedo now aborts rather than silently producing diagnostic gray.
Its material output remains an **albedo/normal baseline**, not full Tempest shader
reconstruction. Original tint/color-mask effects are intentionally still reported
as outstanding. The first three bounded attempts are retained under Saved, with
their PARTIAL/FAIL status; only attempt 04 is the successful dependency manifest.
No engine import was run by this agent. Root owns that subsequent verification.
