"""Import detailed own buildings and update only six DataV4 static_mesh references.
No statistics, footprints, icons or runtime C++ modifications. Root executes serially.
"""
from pathlib import Path
import json,unreal
ROOT=Path(__file__).resolve().parents[2];SOURCE=ROOT/'Art/Models/MilitaryDetailV6'
DEST='/Game/HERO/MilitaryDetailV6/Meshes';TOOLS=unreal.AssetToolsHelpers.get_asset_tools();ASSETS=unreal.EditorAssetLibrary
manifest=json.loads((SOURCE/'manifest.json').read_text(encoding='utf-8'))
records=[]
for meta in manifest['assets']:
    assert meta['validation']=='PASS' and meta['source_unchanged'] and meta['pivot_preserved']
    name=meta['name'];task=unreal.AssetImportTask();task.filename=str(SOURCE/(name+'.fbx'))
    task.destination_path=DEST;task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True
    options=unreal.FbxImportUI();options.import_mesh=True;options.import_as_skeletal=False
    options.import_materials=False;options.import_textures=False
    options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False
    options.static_mesh_import_data.generate_lightmap_u_vs=True;task.options=options
    TOOLS.import_asset_tasks([task]);mesh=unreal.load_asset(DEST+'/'+name);assert mesh
    for index,slot in enumerate(mesh.get_editor_property('static_materials')):
        semantic=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
        mat=unreal.load_asset('/Game/HERO/ConceptV2/Materials/M_CV2_'+semantic);assert mat,(name,semantic)
        mesh.set_material(index,mat)
    bounds=mesh.get_bounding_box()
    for expected,actual in zip(meta['unreal_bounds_m'],(bounds.min,bounds.max)):
        assert all(abs(m*100-cm)<1 for m,cm in zip(expected,(actual.x,actual.y,actual.z))),(name,expected,actual)
    ASSETS.save_loaded_asset(mesh,only_if_is_dirty=False)
    changed=[]
    for faction in ('Aegis','Ember'):
        definition=unreal.load_asset('/Game/HERO/DataV4/D_'+faction+'_'+meta['role'])
        assert definition,(faction,meta['role'])
        previous=definition.get_editor_property('static_mesh')
        definition.set_editor_property('static_mesh',mesh);ASSETS.save_loaded_asset(definition,only_if_is_dirty=False)
        assert definition.get_editor_property('static_mesh')==mesh
        changed.append(dict(definition=definition.get_path_name(),previous_mesh=previous.get_path_name() if previous else None))
    records.append(dict(mesh=mesh.get_path_name(),role=meta['role'],definitions=changed,source_bounds_preserved=True))
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/MilitaryDetailV6-Import.json').write_text(json.dumps(records,indent=2),encoding='utf-8')
unreal.log('MILITARY_DETAIL_V6_IMPORT_PASS '+json.dumps(records))
