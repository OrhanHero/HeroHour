"""Import only original split harvester assets. Does not edit maps/definitions."""
import hashlib
import json
from pathlib import Path
import unreal

ROOT = Path(unreal.Paths.project_dir()).resolve()
SOURCE = ROOT / 'Art/Models/HarvesterAnimationV9'
DEST = '/Game/HERO/HarvesterAnimationV9/Meshes'
manifest = json.loads((SOURCE/'manifest.json').read_text(encoding='utf-8'))
assert manifest['validation'].startswith('PASS')
assert len(manifest['parts']) == 11
assert sum(p['triangles'] for p in manifest['parts']) == manifest['full_original']['triangles']
tools = unreal.AssetToolsHelpers.get_asset_tools()
records = []
for part in manifest['parts']:
    source = SOURCE / (part['name']+'.fbx')
    assert hashlib.sha256(source.read_bytes()).hexdigest() == part['fbx_sha256']
    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = DEST
    task.destination_name = part['name']
    task.automated = True
    task.replace_existing = True
    task.save = False
    settings = unreal.FbxImportUI()
    settings.import_mesh = True
    settings.import_as_skeletal = False
    settings.import_materials = False
    settings.import_textures = False
    settings.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    settings.static_mesh_import_data.combine_meshes = True
    settings.static_mesh_import_data.auto_generate_collision = False
    task.options = settings
    tools.import_asset_tasks([task])
    mesh = unreal.load_asset(DEST+'/'+part['name'])
    assert isinstance(mesh,unreal.StaticMesh),part['name']
    slots = mesh.get_editor_property('static_materials')
    actual_slots = []
    for index,slot in enumerate(slots):
        key = str(slot.material_slot_name).removeprefix('M_CV2_').split('.')[0]
        assert key in part['material_slots'],(part['name'],key)
        material = unreal.load_asset('/Game/HERO/ConceptV2/Materials/M_CV2_'+key)
        assert material,key
        mesh.set_material(index,material)
        actual_slots.append(key)
    assert set(actual_slots)==set(part['material_slots'])
    bounds = mesh.get_bounding_box()
    actual_min = [bounds.min.x,bounds.min.y,bounds.min.z]
    actual_max = [bounds.max.x,bounds.max.y,bounds.max.z]
    # Blender-to-Unreal reflects Y. Bounds and origin are independently checked,
    # so a scale-correct but accidentally re-centred component is rejected.
    expected_min = [part['bounds_min_m'][0]*100,-part['bounds_max_m'][1]*100,part['bounds_min_m'][2]*100]
    expected_max = [part['bounds_max_m'][0]*100,-part['bounds_min_m'][1]*100,part['bounds_max_m'][2]*100]
    for actual,expected in ((actual_min,expected_min),(actual_max,expected_max)):
        assert max(abs(a-b) for a,b in zip(actual,expected))<.2,(part['name'],actual,expected)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    records.append(dict(name=part['name'],path=mesh.get_path_name(),bounds_min_cm=actual_min,
                        bounds_max_cm=actual_max,material_slots=actual_slots))
out = ROOT/'Saved/Research/HarvesterAnimationV9-import.json'
out.parent.mkdir(parents=True,exist_ok=True)
out.write_text(json.dumps(dict(result='PASS',parts=records),indent=2),encoding='utf-8')
unreal.log('HARVESTER_ANIMATION_V9_IMPORT_PASS '+str(len(records)))
