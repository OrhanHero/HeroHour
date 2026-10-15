"""Import original PresentationV5 decoration only. Does not place actors/edit maps.
Run via Unreal Editor Python after build_ground_detail_v5.py completes.
"""
import json
from pathlib import Path
import unreal
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/Models/PresentationV5'
DEST='/Game/HERO/PresentationV5/Meshes'
manifest=json.loads((SOURCE/'manifest.json').read_text(encoding='utf-8'))
records=[]
for meta in manifest['assets']:
    name=meta['name']
    assert meta['validation']=='PASS' and meta['fbx_roundtrip']['passed']
    task=unreal.AssetImportTask()
    task.filename=str(SOURCE/(name+'.fbx'))
    task.destination_path=DEST; task.destination_name=name
    task.automated=True; task.replace_existing=True; task.save=True
    options=unreal.FbxImportUI()
    options.import_mesh=True; options.import_as_skeletal=False
    options.import_materials=False; options.import_textures=False
    options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True
    options.static_mesh_import_data.auto_generate_collision=False
    options.static_mesh_import_data.generate_lightmap_u_vs=True
    task.options=options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    mesh=unreal.load_asset(DEST+'/'+name); assert mesh,name
    mesh.modify()
    for index,slot in enumerate(mesh.get_editor_property('static_materials')):
        key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
        mat=unreal.load_asset('/Game/HERO/ConceptV2/Materials/M_CV2_'+key)
        assert mat,(name,key)
        mesh.set_material(index,mat)
    bounds=mesh.get_bounding_box(); size=bounds.max-bounds.min
    actual=[size.x,size.y,size.z]
    assert all(abs(cm-m*100)<1 for cm,m in zip(actual,meta['dimensions_m'])),(name,actual)
    assert abs(bounds.min.z)<1,(name,bounds.min.z)
    # Imported meshes have no generated hulls. Placement must also use NoCollision
    # and SetCanEverAffectNavigation(false) on the runtime mesh component.
    unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False)
    records.append(dict(name=name,path=mesh.get_path_name(),dimensions_cm=actual,collision='no generated collision',decoration_only=True))
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/PresentationV5-Import.json').write_text(json.dumps(records,indent=2),encoding='utf-8')
unreal.log('PRESENTATION_V5_IMPORT_PASS '+json.dumps(records))
