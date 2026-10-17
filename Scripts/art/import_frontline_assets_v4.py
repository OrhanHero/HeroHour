"""Import only six original FrontlinesV4 meshes using existing ConceptV2 materials.

Run inside Unreal Python. This does not edit maps or material graphs. Portrait
PNGs are imported separately by the root integration step.
"""
import json
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Art/Models/FrontlinesV4"
DEST = "/Game/HERO/FrontlinesV4/Meshes"
ROLES = ("PowerPlant", "Barracks", "Radar", "Defense", "Infantry", "Siege")
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
RECORDS = []

for role in ROLES:
    name = "SM_HR_" + role
    metadata = json.loads((SOURCE / (name + ".json")).read_text(encoding="utf-8"))
    assert metadata["fbx_roundtrip"]["passed"], name
    source = SOURCE / (name + ".fbx")
    assert source.is_file(), str(source)
    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = DEST
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.save = True
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = False
    options.import_textures = False
    options.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.auto_generate_collision = True
    task.options = options
    TOOLS.import_asset_tasks([task])
    mesh = unreal.load_asset(DEST + "/" + name)
    assert mesh, name
    slots = mesh.get_editor_property("static_materials")
    material_names = []
    for index, slot in enumerate(slots):
        key = str(slot.get_editor_property("imported_material_slot_name")).split(".")[0]
        material = unreal.load_asset("/Game/HERO/ConceptV2/Materials/M_CV2_" + key)
        assert material, (name, "missing existing material", key)
        mesh.set_material(index, material)
        material_names.append(key)
    bounds = mesh.get_bounding_box()
    size = bounds.max - bounds.min
    actual = (size.x, size.y, size.z)
    for measured, metres in zip(actual, metadata["dimensions_m"]):
        assert abs(measured - metres * 100) < 1.0, (name, actual, metadata["dimensions_m"])
    assert abs(bounds.min.z + 35) < 1.0, (name, bounds.min.z)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    RECORDS.append({"name": name, "path": mesh.get_path_name(), "bounds_cm": actual,
        "min_z_cm": bounds.min.z, "runtime_scale": 1.0, "materials": material_names})

(ROOT / "Saved/Research").mkdir(parents=True, exist_ok=True)
(ROOT / "Saved/Research/FrontlinesV4-Import.json").write_text(json.dumps(RECORDS, indent=2), encoding="utf-8")
unreal.log("FRONTLINES_V4_IMPORT_PASS " + json.dumps(RECORDS))
