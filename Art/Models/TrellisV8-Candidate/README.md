# Aegis HQ – TRELLIS.2 V8 candidate

Generated locally from the project's own isolated concept reference, not extracted Tempest Rising geometry. This folder deliberately does not replace a HeroHour runtime asset.

- `AegisHQ-Trellis512_00001_.glb`: original unmodified successful output; 18,456 triangles, 20,682 UV vertices.
- `AegisHQ-Trellis512-embedded-0.png`: original embedded BaseColor PNG, 1024×1024. Interpret as color/sRGB.
- `AegisHQ-Trellis512-embedded-1.png`: original embedded glTF Metallic/Roughness PNG, 1024×1024. Linear data; G is roughness, B is metallic, R is constant255 and is not a measured AO bake.
- `generation-workflow-api.json`: the actual 27-node native ComfyUI workflow, 512 shape resolution and 1024 texture bake, fixed seeds56/42/43.
- `candidate-verification.json`: exact byte hashes, geometry/material checks, provenance and measurement limits.

Prompt `03754b72-3374-40bf-a4ed-e2112d023fa5` completed in26.297 seconds on the project's RTX4070Ti SUPER. The observed whole-card peak was10,671 MiB with five-second sampling; this is not an exact CUDA allocation peak. Only one prompt was submitted and no retry occurred.

Input: `Art/Concepts/GeneratedV8/AegisHQ-TrellisInput-v1.png`, SHA-256 `c28f14775d447077c6a32b20bddc9a344cab4353b719966c190750f750ed020e`. The input is a generated working reference based on the user's concept.

This is a candidate awaiting visual review. The GLB has UV0 and real PBR texture bindings, but no exported vertex normals: calculate normals in the downstream authoring tool. Coordinates are normalized with glTF Y-up; the pivot is not grounded and no world scale, collision, navigation footprint, LODs, sockets or emissive mask is authored. The hidden sides are inferred from one image. No Unreal import or production acceptance is implied.

Raw history, monitor and model manifests: `Saved/Research/TrellisV8/`. Summary: `docs/research/Trellis2-LocalFeasibilityV8.md`.
