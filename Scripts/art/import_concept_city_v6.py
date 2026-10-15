"""Import V6 city meshes and replace ONLY observed DV3 Civic actors in place.
Root executes this serially. Position/rotation/scale/labels/blockers stay intact.
"""
from pathlib import Path
import unreal,json
ROOT=Path(__file__).resolve().parents[2];SOURCE=ROOT/'Art/Models/ConceptCityV6';DEST='/Game/HERO/ConceptCityV6/Meshes'
manifest=json.loads((SOURCE/'manifest.json').read_text());meshes={};records=[]
for meta in manifest['assets']:
    name=meta['name'];assert meta['validation']=='PASS' and meta['fbx_roundtrip']
    task=unreal.AssetImportTask();task.filename=str(SOURCE/(name+'.fbx'));task.destination_path=DEST;task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True
    options=unreal.FbxImportUI();options.import_materials=False;options.import_textures=False;options.import_as_skeletal=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;task.options=options
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task]);mesh=unreal.load_asset(DEST+'/'+name);assert mesh
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
        mat=unreal.load_asset('/Game/HERO/DistrictV3/Materials/M_DV3_'+key);assert mat,key;mesh.set_material(i,mat)
    bounds=mesh.get_bounding_box();size=bounds.max-bounds.min;actual=[size.x,size.y,size.z]
    assert all(abs(cm-m*100)<1 for cm,m in zip(actual,meta['dimensions_m'])),(name,actual)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=False);meshes[name]=mesh;records.append(dict(name=name,dimensions_cm=actual))
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);assert level.load_level('/Game/Maps/CityHero')
names=[a['name'] for a in manifest['assets']];replaced=[]
for a in actors.get_all_level_actors():
    if not isinstance(a,unreal.StaticMeshActor):continue
    c=a.static_mesh_component;old=c.static_mesh
    if not old or not (old.get_name().startswith('SM_DV3_Civic_') or old.get_name().startswith('SM_CC6_')):continue
    pos=a.get_actor_location();col=round((pos.x-1500)/1500);row=round((pos.y+4500)/1500)
    style=2 if col in (1,2) and abs(pos.y)<=1500 else (row+col)%4
    before=dict(position=[pos.x,pos.y,pos.z],scale=list(a.get_actor_scale3d().to_tuple()) if hasattr(a.get_actor_scale3d(),'to_tuple') else [a.get_actor_scale3d().x,a.get_actor_scale3d().y,a.get_actor_scale3d().z])
    c.modify();c.set_static_mesh(meshes[names[style]]);c.set_editor_property('override_materials',[])
    c.set_collision_profile_name(unreal.Name('NoCollision'));c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);c.set_editor_property('can_ever_affect_navigation',False)
    replaced.append(dict(label=a.get_actor_label(),old=old.get_name(),new=names[style],**before))
assert len(replaced)==28,len(replaced)
assert level.save_current_level()
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/ConceptCityV6-Import.json').write_text(json.dumps(dict(meshes=records,replacements=replaced,existing_navigation_blocker_radius_cm=600),indent=2))
unreal.log('CONCEPT_CITY_V6_IMPORT_PASS '+str(len(replaced)))
