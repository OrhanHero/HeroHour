"""Root executes serially: import three merged original low-profile road overlays."""
from pathlib import Path
import unreal,json
ROOT=Path(__file__).resolve().parents[2];SOURCE=ROOT/'Art/Models/RoadFinishV6';DEST='/Game/HERO/RoadFinishV6';manifest=json.loads((SOURCE/'manifest.json').read_text())
TOOLS=unreal.AssetToolsHelpers.get_asset_tools();ASSETS=unreal.EditorAssetLibrary;LIB=unreal.MaterialEditingLibrary
materials={}
for key,rgb in manifest['materials'].items():
    path=DEST+'/Materials/M_RF6_'+key;mat=unreal.load_asset(path) if ASSETS.does_asset_exist(path) else TOOLS.create_asset('M_RF6_'+key,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    LIB.delete_all_material_expressions(mat)
    col=LIB.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,0,0);col.set_editor_property('constant',unreal.LinearColor(*rgb,1));LIB.connect_material_property(col,'',unreal.MaterialProperty.MP_BASE_COLOR)
    rough=LIB.create_material_expression(mat,unreal.MaterialExpressionConstant,0,0);rough.set_editor_property('r',.94);LIB.connect_material_property(rough,'',unreal.MaterialProperty.MP_ROUGHNESS)
    LIB.recompile_material(mat);ASSETS.save_loaded_asset(mat,only_if_is_dirty=False);materials[key]=mat
meshes={}
for meta in manifest['assets']:
    name=meta['name'];assert meta['validation']=='PASS' and meta['fbx_roundtrip']
    task=unreal.AssetImportTask();task.filename=str(SOURCE/(name+'.fbx'));task.destination_path=DEST+'/Meshes';task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True
    options=unreal.FbxImportUI();options.import_materials=False;options.import_textures=False;options.import_as_skeletal=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH;options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;task.options=options
    TOOLS.import_asset_tasks([task]);mesh=unreal.load_asset(DEST+'/Meshes/'+name);assert mesh
    bounds=mesh.get_bounding_box()
    for side in ('min','max'):
        actual=getattr(bounds,side)
        assert all(abs(getattr(actual,k)-expected)<2 for k,expected in zip(('x','y','z'),meta['expected_ue_bounds_cm'][side])),(name,side,actual,meta['expected_ue_bounds_cm'][side])
    mesh.set_material(0,materials[meta['material']]);ASSETS.save_loaded_asset(mesh,only_if_is_dirty=False);meshes[name]=mesh
LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);assert LEVEL.load_level('/Game/Maps/CityHero')
for a in ACTORS.get_all_level_actors():
    if a.get_actor_label().startswith('RFV6 '):ACTORS.destroy_actor(a)
for name,mesh in meshes.items():
    a=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(0,0,0),unreal.Rotator(pitch=0,yaw=0,roll=0));a.set_actor_label('RFV6 '+name);a.tags=[unreal.Name('HERO_RoadFinishV6')]
    c=a.static_mesh_component;c.modify();c.set_static_mesh(mesh);c.set_collision_profile_name(unreal.Name('NoCollision'));c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);c.set_editor_property('can_ever_affect_navigation',False)
assert LEVEL.save_current_level()
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/RoadFinishV6-Import.json').write_text(json.dumps(dict(result='PASS',actors=len(meshes),pieces=sum(x['pieces'] for x in manifest['assets']),triangles=sum(x['triangles'] for x in manifest['assets']),source_audit_sha256=manifest['audit_sha256'],navigation_changed=False),indent=2))
unreal.log('ROAD_FINISH_V6_IMPORT_PASS '+str(len(meshes)))
