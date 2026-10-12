"""Recover the existing Fab ATV mesh and its real camouflage textures."""
from pathlib import Path
import unreal

DEST = "/Game/LocalAssets/RTSModern"
SOURCE = Path(r"E:\HeroHour\Art\Imported\RTSModern")
CLEAN_MESH = Path(__file__).resolve().parents[2] / "Art/Models/Imported/ATV_N1_LOD0.fbx"
assets = unreal.AssetToolsHelpers.get_asset_tools()

mesh_options = unreal.FbxImportUI()
mesh_options.import_mesh = True
mesh_options.import_as_skeletal = False
mesh_options.import_materials = False
mesh_options.import_textures = False
mesh_options.static_mesh_import_data.combine_meshes = True
mesh_options.static_mesh_import_data.auto_generate_collision = True
task = unreal.AssetImportTask()
if not CLEAN_MESH.is_file():
    raise RuntimeError(f"Missing cleaned LOD0 mesh: {CLEAN_MESH}; run clean_legacy_atv.py in Blender first")
task.filename = str(CLEAN_MESH)
task.destination_path = DEST
task.destination_name = "ATV_N1_LOD0"
task.automated = True
task.replace_existing = True
task.save = True
task.options = mesh_options
assets.import_asset_tasks([task])
mesh = unreal.load_asset(DEST + "/ATV_N1_LOD0")
if not isinstance(mesh, unreal.StaticMesh):
    raise RuntimeError("ATV mesh import failed")

texture_files = {
    "body": SOURCE / "textures/LOD0/0_Main/ATV_N1_LOD0_Camo_0.png",
    "weapon": SOURCE / "textures/LOD0/1_Weapon/AC1_LOD0_Camo_0.png",
    "wheels": SOURCE / "textures/LOD0/2_Wheels_Tracks/ATV_N1_Wheels_Tracks_LOD0.png",
    "rotation": SOURCE / "textures/LOD0/3_Wheels_Rotation/ATV_N1_Wheels_Rotation_LOD0.png",
    "emissive": SOURCE / "textures/LOD0/0_Main/ATV_N1_LOD0_Emissive.png",
}
textures = {}
for key, filename in texture_files.items():
    if not filename.is_file():
        raise RuntimeError(f"Missing texture {filename}")
    texture_task = unreal.AssetImportTask()
    texture_task.filename = str(filename)
    texture_task.destination_path = DEST + "/Textures"
    texture_task.destination_name = "T_ATV_" + key
    texture_task.automated = True
    texture_task.replace_existing = True
    texture_task.save = True
    assets.import_asset_tasks([texture_task])
    textures[key] = unreal.load_asset(DEST + "/Textures/T_ATV_" + key)
    if not textures[key]:
        raise RuntimeError(f"Texture import failed: {key}")

materials = {}
for key in ("body", "weapon", "wheels", "rotation"):
    name = "M_ATV_" + key
    path = DEST + "/Materials/" + name
    material = unreal.load_asset(path)
    if not material:
        material = assets.create_asset(name, DEST + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
    unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    sample = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureSample, -450, 0)
    sample.texture = textures[key]
    unreal.MaterialEditingLibrary.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -250, 200)
    roughness.set_editor_property("r", 0.7)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    if key == "body":
        emissive = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionTextureSample, -450, 350)
        emissive.texture = textures["emissive"]
        unreal.MaterialEditingLibrary.connect_material_property(emissive, "RGB", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    unreal.MaterialEditingLibrary.recompile_material(material)
    materials[key] = material

for index, slot in enumerate(mesh.static_materials):
    slot_name = str(slot.material_slot_name)
    lowered = slot_name.lower()
    if "rotation" in lowered:
        key = "rotation"
    elif "wheel" in lowered or "track" in lowered:
        key = "wheels"
    elif "weapon" in lowered or "ac1" in lowered:
        key = "weapon"
    else:
        key = "body"
    mesh.set_material(index, materials[key])
    unreal.log(f"LEGACY_ATV_SLOT index={index} name={slot_name} assignment={key} material={materials[key].get_path_name()}")
unreal.EditorAssetLibrary.save_directory(DEST, only_if_is_dirty=False, recursive=True)
for name in ("Tank_006", "Hummer_003", "Helicopter_001", "Tent_002", "Tower_003", "Barrier_004", "Barrier_006", "Box_003"):
    existing = unreal.load_asset("/Game/LocalAssets/MilitaryFree/" + name)
    if isinstance(existing, unreal.StaticMesh):
        bounds = existing.get_bounds()
        unreal.log(f"LEGACY_MILITARY_BOUNDS name={name} origin={bounds.origin} extent={bounds.box_extent} slots={len(existing.static_materials)}")
        for index, slot in enumerate(existing.static_materials):
            material_path = slot.material_interface.get_path_name() if slot.material_interface else "NONE"
            unreal.log(f"LEGACY_MILITARY_SLOT mesh={name} index={index} name={slot.material_slot_name} material={material_path}")
    else:
        unreal.log_warning(f"LEGACY_MILITARY_MISSING name={name}")
unreal.log(f"LEGACY_ATV_BOUNDS origin={mesh.get_bounds().origin} extent={mesh.get_bounds().box_extent}")
unreal.log("LEGACY_MILITARY_IMPORT_RESULT=PASS")
