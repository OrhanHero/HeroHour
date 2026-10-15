"""Import authored faceted Aegis HQ; update only D_Aegis_HQ.static_mesh.
Does not modify maps, gameplay statistics, Ember meshes or portraits.
"""
import json
from pathlib import Path
import unreal
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/Models/ReferenceHQV7'; NAME='SM_RV7_AegisHQ'
DEST='/Game/HERO/ReferenceV7/Meshes'
meta=json.loads((SOURCE/(NAME+'.json')).read_text(encoding='utf-8'))
assert meta['fbx_roundtrip_pass'] and abs(meta['zoffset_m']+1.1)<.001
assert all(m*150<=1200 for m in meta['dimensions_m'][:2])
task=unreal.AssetImportTask(); task.filename=str(SOURCE/(NAME+'.fbx'))
task.destination_path=DEST; task.destination_name=NAME
task.automated=True;task.replace_existing=True;task.save=True
options=unreal.FbxImportUI();options.import_mesh=True;options.import_as_skeletal=False
options.import_materials=False;options.import_textures=False
options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
options.static_mesh_import_data.combine_meshes=True
options.static_mesh_import_data.auto_generate_collision=False
options.static_mesh_import_data.generate_lightmap_u_vs=True
task.options=options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(DEST+'/'+NAME);assert isinstance(mesh,unreal.StaticMesh)
mesh.modify()
# Local material instances preserve the source PBR maps while overriding Tint.
# Their names intentionally bypass runtime M_CV2_* replacement.
hq_surfaces={}
for surface,tint in [('Armor',(.60,.64,.64)),('Concrete',(.45,.47,.45))]:
 name='M_RV7_Aegis_HQ'+surface
 path='/Game/HERO/ReferenceV7/Materials/'+name
 parent=unreal.load_asset('/Game/HERO/PresentationV5/Materials/M_HR_Aegis_'+surface);assert parent
 mat=unreal.load_asset(path)
 if not mat:
  mat=unreal.AssetToolsHelpers.get_asset_tools().create_asset(name,'/Game/HERO/ReferenceV7/Materials',unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
 assert isinstance(mat,unreal.MaterialInstanceConstant)
 unreal.MaterialEditingLibrary.set_material_instance_parent(mat,parent)
 unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(mat,'Tint',unreal.LinearColor(*tint,1.0))
 unreal.MaterialEditingLibrary.update_material_instance(mat)
 assert unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=False)
 hq_surfaces[surface]=mat
for index,slot in enumerate(mesh.get_editor_property('static_materials')):
 key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
 mat=hq_surfaces.get(key) or unreal.load_asset('/Game/HERO/ConceptV2/Materials/M_CV2_'+key);assert mat,key
 mesh.set_material(index,mat)
bounds=mesh.get_bounding_box()
actual=[[bounds.min.x,bounds.max.x],[bounds.min.y,bounds.max.y],[bounds.min.z,bounds.max.z]]
expected=meta['unreal_expected_bounds_cm']
assert all(abs(a-b)<1 for pair,wanted in zip(actual,expected) for a,b in zip(pair,wanted)),(actual,expected)
assert abs(bounds.min.z+110)<1
assert unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
definition=unreal.load_asset('/Game/HERO/DataV4/D_Aegis_HQ');assert definition
previous=definition.get_editor_property('static_mesh')
old_mesh=previous.get_path_name() if previous else None
old_icon=definition.get_editor_property('icon');old_stats=str(definition.get_editor_property('stats'))
definition.modify();definition.set_editor_property('static_mesh',mesh)
assert unreal.EditorAssetLibrary.save_loaded_asset(definition,only_if_is_dirty=False)
assert definition.get_editor_property('icon')==old_icon and str(definition.get_editor_property('stats'))==old_stats
report=dict(mesh=mesh.get_path_name(),definition=definition.get_path_name(),changed_property='static_mesh',previous_mesh=old_mesh,stats_and_icon_unchanged=True,bounds_cm=actual,triangles=meta['triangles'],runtime_scale=1.5,runtime_ground_z_cm=110+55+bounds.min.z*1.5,material_slots=[str(s.get_editor_property('imported_material_slot_name')) for s in mesh.get_editor_property('static_materials')],limits='Static roof dish; local UV-baked materials and matching portrait are imported in separate final stages')
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/ReferenceHQV7-Import.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('REFERENCE_HQ_V7_IMPORT_PASS '+json.dumps(report))
