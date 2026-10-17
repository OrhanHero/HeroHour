# TRELLIS mesh diagnosis

The preview images show the actual raw, directly decimated, unwrapped and repaired HQ meshes. Geometry hashes/counts and native workflow evidence are versioned under `docs/verification/2026-10-16/reference-v8/trellis/`.

Large intermediate GLBs are retained locally and ignored by Git. They are reproducible from the recorded own image, seeds, model manifest and native workflows. They are not runtime assets. The accepted textured source is separately retained in `Art/Models/TrellisV8-Repaired/`, and its normalized Unreal authoring source in `Art/Models/TrellisHQV9/`.

The initial direct simplification was rejected after real multi-view inspection. Remeshing before reduction preserved the recognizable satellite dish and architecture. This is a repaired original generated asset, not extracted Tempest Rising geometry.
