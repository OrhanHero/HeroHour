"""Import original Ember APC; change only D_Ember_Combat.static_mesh."""
import hashlib
import json
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'Art/Models/ReferenceEmberAPCV7'
NAME = 'SM_HR_Ember_APC8'
DEST = '/Game/HERO/ReferenceV7/Meshes'
metadata = json.loads((SOURCE / (NAME + '.json')).read_text(encoding='utf-8'))
assert metadata['validation'] == 'PASS' and metadata['fbx_roundtrip']['passed']
assert metadata['wheel_count'] == metadata['actual_boolean_arch_count'] == 8
assert hashlib.sha256((SOURCE / (NAME + '.fbx')).read_bytes()).hexdigest() == metadata['fbx_sha256']
definition = unreal.load_asset('/Game/HERO/DataV4/D_Ember_Combat')
assert definition
# The importer writes just the static mesh field; assert that the existing
# gameplay stats and separately managed icon survive the import unchanged.
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
# Names deliberately omit M_CV2_*: the runtime material pass therefore retains
# the supplied concept's graphite/green art instead of applying red team paint.
# Enemy health bars, rings and HUD retain their existing gameplay colors.
concept_surfaces = {}
for surface, tint in [('Team', (.025, .32, .13)), ('Neon', (.025, .70, .28))]:
    name = 'M_RV7_Ember_' + surface + 'Green'
    folder = '/Game/HERO/ReferenceV7/Materials'
    parent = unreal.load_asset('/Game/HERO/PresentationV5/Materials/M_HR_Ember_' + surface)
    assert parent, surface
    material = unreal.load_asset(folder + '/' + name)
    if not material:
        material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            name, folder, unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew())
    assert isinstance(material, unreal.MaterialInstanceConstant)
    unreal.MaterialEditingLibrary.set_material_instance_parent(material, parent)
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        material, 'Tint', unreal.LinearColor(*tint, 1.0))
    unreal.MaterialEditingLibrary.update_material_instance(material)
    assert unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False)
    concept_surfaces[surface] = material
for index, slot in enumerate(mesh.get_editor_property('static_materials')):
    key = str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
    material = concept_surfaces.get(key) or unreal.load_asset('/Game/HERO/ConceptV2/Materials/M_CV2_' + key)
    assert material, key
    mesh.set_material(index, material)
bounds = mesh.get_bounding_box()
size = bounds.max - bounds.min
actual_dimensions = [size.x, size.y, size.z]
assert all(abs(a - m * 100) < .5 for a, m in
           zip(actual_dimensions, metadata['dimensions_m'])), actual_dimensions
assert size.y <= 188.0 and abs(bounds.min.z + 35.0) < .5
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
definition.modify()
definition.set_editor_property('static_mesh', mesh)
assert unreal.EditorAssetLibrary.save_loaded_asset(definition, only_if_is_dirty=False)
assert definition.get_editor_property('icon') == old_icon
assert str(definition.get_editor_property('stats')) == old_stats
record = dict(
    mesh=mesh.get_path_name(), definition=definition.get_path_name(),
    changed_property='static_mesh', previous_mesh=old_mesh,
    dimensions_cm=actual_dimensions, pivot_min_z=bounds.min.z,
    runtime_scale=1, runtime_ground_z_cm=35 + bounds.min.z,
    triangles=metadata['triangles'], wheel_count=8,
    stats_and_icon_unchanged=True,
    concept_surface_materials={key: material.get_path_name() for key, material in concept_surfaces.items()},
    concept_surface_tints=metadata['concept_surface_tints'],
    gameplay_identification='Enemy red health bars, rings and UI retained; source concept graphite/green surface identity',
    limits='Static wheels and turret, no LOD or unique baked weathering; matching portrait is managed separately')
(ROOT / 'Saved/Research').mkdir(parents=True, exist_ok=True)
(ROOT / 'Saved/Research/ReferenceEmberAPCV7-Import.json').write_text(
    json.dumps(record, indent=2), encoding='utf-8')
unreal.log('REFERENCE_EMBER_APC_V7_IMPORT_PASS ' + json.dumps(record))
