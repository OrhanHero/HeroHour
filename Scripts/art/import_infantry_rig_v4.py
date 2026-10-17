"""Import and validate original infantry skeletal mesh plus three real clips.

Run with Unreal Python after the root completes the C++ build. This script
changes only /Game/HERO/InfantryRigV4 and uses existing ConceptV2 materials.
"""
import json
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Art/Models/InfantryRigV4"
DEST = "/Game/HERO/InfantryRigV4"
NAME = "SK_HR_Infantry"
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
METADATA = json.loads((SOURCE / "InfantryRigV4-manifest.json").read_text(encoding="utf-8"))
assert METADATA["fbx_roundtrip"]["passed"] and METADATA["bone_count"] == 14


def task_for(name, options):
    source = SOURCE / (name + ".fbx")
    assert source.is_file(), str(source)
    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = DEST
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    TOOLS.import_asset_tasks([task])
    asset = unreal.load_asset(DEST + "/" + name)
    assert asset, (name, list(task.imported_object_paths))
    return asset


options = unreal.FbxImportUI()
options.import_mesh = True
options.import_as_skeletal = True
options.import_animations = False
options.import_materials = False
options.import_textures = False
options.create_physics_asset = False
options.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
existing_skeleton = unreal.load_asset(DEST + "/" + NAME + "_Skeleton")
if existing_skeleton:
    options.skeleton = existing_skeleton
mesh = task_for(NAME, options)
assert isinstance(mesh, unreal.SkeletalMesh), mesh
skeleton = mesh.get_editor_property("skeleton")
assert isinstance(skeleton, unreal.Skeleton), "Missing skeleton"
materials = mesh.get_editor_property("materials")
for index, slot in enumerate(materials):
    key = str(slot.get_editor_property("imported_material_slot_name")).split(".")[0]
    material = unreal.load_asset("/Game/HERO/ConceptV2/Materials/M_CV2_" + key)
    assert material, ("Missing existing material", key)
    slot.set_editor_property("material_interface", material)
    materials[index] = slot
    unreal.MaterialEditingLibrary.set_material_usage(material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
    LIB.save_loaded_asset(material, only_if_is_dirty=False)
mesh.modify()
mesh.set_editor_property("materials", materials)
assert all(slot.get_editor_property("material_interface").get_path_name().startswith("/Game/HERO/")
           for slot in mesh.get_editor_property("materials")), "Skeletal material array lost assignments"
assert LIB.save_loaded_asset(mesh, only_if_is_dirty=False)
assert LIB.save_loaded_asset(skeleton)

records = []
for clip, seconds in METADATA["clips"].items():
    name = "AN_HR_Infantry_" + clip
    options = unreal.FbxImportUI()
    options.import_mesh = False
    options.import_as_skeletal = True
    options.import_animations = True
    options.import_materials = False
    options.import_textures = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_ANIMATION
    options.skeleton = skeleton
    options.anim_sequence_import_data.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    options.anim_sequence_import_data.set_editor_property("use_default_sample_rate", True)
    options.anim_sequence_import_data.set_editor_property("import_bone_tracks", True)
    options.anim_sequence_import_data.set_editor_property("import_custom_attribute", False)
    sequence = task_for(name, options)
    assert isinstance(sequence, unreal.AnimSequence), name
    assert sequence.get_editor_property("skeleton") == skeleton, (name, "wrong skeleton")
    measured = sequence.get_play_length()
    assert abs(measured - float(seconds)) < .04, (name, measured, seconds)
    tracks = [str(bone) for bone in unreal.AnimationLibrary.get_animation_track_names(sequence)]
    assert set(METADATA["fbx_roundtrip"]["bones"]).issubset(set(tracks)), (name, tracks)
    assert LIB.save_loaded_asset(sequence)
    records.append(dict(asset=sequence.get_path_name(), seconds=measured, tracks=tracks))

output = ROOT / "Saved/Research/InfantryRigV4-Import.json"
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(dict(mesh=mesh.get_path_name(), skeleton=skeleton.get_path_name(),
    clips=records, materials=[str(slot.get_editor_property("imported_material_slot_name")) for slot in materials]),
    indent=2), encoding="utf-8")
unreal.log("INFANTRY_RIG_V4_IMPORT_PASS " + str(mesh.get_path_name()))
