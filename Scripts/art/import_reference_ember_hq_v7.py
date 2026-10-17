"""Import the original Ember HQ; change only D_Ember_HQ.static_mesh.

Run through Unreal Python after the Blender exporter. No map or gameplay edits.
"""
import hashlib
import json
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'Art/Models/ReferenceEmberHQV7'
NAME = 'SM_RV7_EmberHQ'
DEST = '/Game/HERO/ReferenceV7/Meshes'
meta = json.loads((SOURCE / (NAME + '.json')).read_text(encoding='utf-8'))
assert meta['validation'] == 'PASS' and meta['fbx_roundtrip_pass']
assert meta['triangles'] <= 35000
assert abs(meta['zoffset_m'] + 1.10) < .001
assert all(m * 150 <= 1200 for m in meta['dimensions_m'][:2])
assert hashlib.sha256((SOURCE / (NAME + '.fbx')).read_bytes()).hexdigest() == meta['fbx_sha256']

definition = unreal.load_asset('/Game/HERO/DataV4/D_Ember_HQ')
assert definition
old_icon = definition.get_editor_property('icon')
old_stats = str(definition.get_editor_property('stats'))
old_mesh = str(definition.get_editor_property('static_mesh'))
task = unreal.AssetImportTask()
task.filename = str(SOURCE / (NAME + '.fbx'))
task.destination_path = DEST
task.destination_name = NAME
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
options.static_mesh_import_data.auto_generate_collision = False
options.static_mesh_import_data.generate_lightmap_u_vs = True
task.options = options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh = unreal.load_asset(DEST + '/' + NAME)
assert isinstance(mesh, unreal.StaticMesh)
mesh.modify()
# Dedicated green instances follow the reference. The shared names match the
# Ember APC importer and intentionally bypass runtime M_CV2 color replacement.
green_surfaces = {}
for surface, tint in [('Team', (.025, .32, .13)), ('Neon', (.025, .70, .28)),
                      ('Glass', (.012, .055, .032))]:
    name = 'M_RV7_Ember_' + surface + 'Green'
    path = '/Game/HERO/ReferenceV7/Materials/' + name
    parent = unreal.load_asset('/Game/HERO/PresentationV5/Materials/M_HR_Ember_' + surface)
    assert parent
    material = unreal.load_asset(path)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, '/Game/HERO/ReferenceV7/Materials', unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew())
    assert isinstance(material, unreal.MaterialInstanceConstant)
    unreal.MaterialEditingLibrary.set_material_instance_parent(material, parent)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        material, 'Tint', unreal.LinearColor(*tint, 1.0))
    unreal.MaterialEditingLibrary.update_material_instance(material)
    assert unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    green_surfaces[surface] = material
# Other semantic slots retain normal runtime Ember graphite PBR replacement.
for index, slot in enumerate(mesh.get_editor_property('static_materials')):
    key = str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
    material = green_surfaces.get(key) or unreal.load_asset('/Game/HERO/ConceptV2/Materials/M_CV2_' + key)
    assert material, key
    mesh.set_material(index, material)
bounds = mesh.get_bounding_box()
actual = [[bounds.min.x, bounds.max.x], [bounds.min.y, bounds.max.y],
          [bounds.min.z, bounds.max.z]]
assert all(abs(value - other) < 1 for pair, expected in zip(actual, meta['unreal_expected_bounds_cm'])
           for value, other in zip(pair, expected)), (actual, meta['unreal_expected_bounds_cm'])
assert abs(bounds.min.z + 110) < .5
ground_z = 110 + 55 + bounds.min.z * 1.5
assert abs(ground_z) < 1
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
definition.modify()
definition.set_editor_property('static_mesh', mesh)
assert unreal.EditorAssetLibrary.save_loaded_asset(definition, only_if_is_dirty=False)
assert definition.get_editor_property('icon') == old_icon
assert str(definition.get_editor_property('stats')) == old_stats
report = dict(mesh=mesh.get_path_name(), definition=definition.get_path_name(),
              changed_property='static_mesh', previous_mesh=old_mesh,
              bounds_cm=actual, triangles=meta['triangles'], runtime_scale=1.5,
              runtime_ground_z_cm=ground_z, stats_and_icon_unchanged=True,
              color_decision=meta['color_decision'], limits=meta['limits'])
(ROOT / 'Saved/Research').mkdir(parents=True, exist_ok=True)
(ROOT / 'Saved/Research/ReferenceEmberHQV7-Import.json').write_text(
    json.dumps(report, indent=2) + '\n', encoding='utf-8')
unreal.log('REFERENCE_EMBER_HQ_V7_IMPORT_PASS ' + json.dumps(report))
