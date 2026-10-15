"""Import ONLY new original Aegis APC and change ONLY Aegis Combat StaticMesh."""
import json
from pathlib import Path
import unreal
ROOT=Path(__file__).resolve().parents[2]; SOURCE=ROOT/'Art/Models/ReferenceAPCV7'; NAME='SM_HR_Aegis_APC8'; DEST='/Game/HERO/ReferenceV7/Meshes'
meta=json.loads((SOURCE/(NAME+'.json')).read_text()); assert meta['validation']=='PASS' and meta['fbx_roundtrip']['passed']
definition=unreal.load_asset('/Game/HERO/DataV4/D_Aegis_Combat');assert definition
old_icon=definition.get_editor_property('icon'); old_stats=str(definition.get_editor_property('stats'))
task=unreal.AssetImportTask();task.filename=str(SOURCE/(NAME+'.fbx'));task.destination_path=DEST;task.destination_name=NAME;task.automated=True;task.replace_existing=True;task.save=True
options=unreal.FbxImportUI();options.import_mesh=True;options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;options.static_mesh_import_data.generate_lightmap_u_vs=True;task.options=options
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(DEST+'/'+NAME);assert mesh
mesh.modify()
for i,slot in enumerate(mesh.get_editor_property('static_materials')):
    key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0];mat=unreal.load_asset('/Game/HERO/ConceptV2/Materials/M_CV2_'+key);assert mat,key;mesh.set_material(i,mat)
b=mesh.get_bounding_box();size=b.max-b.min; actual=[size.x,size.y,size.z];assert all(abs(a-m*100)<.5 for a,m in zip(actual,meta['dimensions_m'])),actual
assert abs(b.min.z+35)<.5,b.min.z
unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
definition.modify();definition.set_editor_property('static_mesh',mesh);unreal.EditorAssetLibrary.save_loaded_asset(definition,only_if_is_dirty=False)
assert definition.get_editor_property('icon')==old_icon and str(definition.get_editor_property('stats'))==old_stats
record=dict(mesh=mesh.get_path_name(),dimensions_cm=actual,pivot_min_z=b.min.z,definition=definition.get_path_name(),changed_property='static_mesh',stats_and_icon_unchanged=True)
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True);(ROOT/'Saved/Research/ReferenceAPCV7-Import.json').write_text(json.dumps(record,indent=2));unreal.log('REFERENCE_APC_V7_IMPORT_PASS '+json.dumps(record))
