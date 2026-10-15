"""Import/place six original canyon meshes. Root runs serially in Unreal commandlet.
Idempotent: removes only CTV6 actors; never edits authoritative navigation/map actors.
"""
from pathlib import Path
import json
import unreal
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/Models/ConceptTerrainV6'
DEST='/Game/HERO/ConceptTerrainV6/Meshes'
manifest=json.loads((SOURCE/'manifest.json').read_text(encoding='utf-8'))
TOOLS=unreal.AssetToolsHelpers.get_asset_tools();ASSETS=unreal.EditorAssetLibrary
LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
stone=unreal.load_asset('/Game/HERO/DistrictV3/Materials/M_DV3_Sandstone');assert stone
meshes={}
for meta in manifest['assets']:
    assert meta['validation']=='PASS' and meta['fbx_roundtrip']
    name=meta['name'];task=unreal.AssetImportTask()
    task.filename=str(SOURCE/(name+'.fbx'));task.destination_path=DEST;task.destination_name=name
    task.automated=True;task.replace_existing=True;task.save=True
    options=unreal.FbxImportUI();options.import_mesh=True;options.import_as_skeletal=False
    options.import_materials=False;options.import_textures=False
    options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True
    options.static_mesh_import_data.auto_generate_collision=False
    options.static_mesh_import_data.generate_lightmap_u_vs=True
    options.static_mesh_import_data.normal_import_method=unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS
    task.options=options;TOOLS.import_asset_tasks([task]);mesh=unreal.load_asset(DEST+'/'+name);assert mesh
    mesh.set_material(0,stone)
    bounds=mesh.get_bounding_box();size=bounds.max-bounds.min
    assert all(abs(cm-m*100)<2 for cm,m in zip((size.x,size.y,size.z),meta['dimensions_m'])),name
    ASSETS.save_loaded_asset(mesh,only_if_is_dirty=False);meshes[name]=mesh
assert LEVEL.load_level('/Game/Maps/CityHero')
original=[a.get_actor_label() for a in ACTORS.get_all_level_actors() if not a.get_actor_label().startswith('CTV6 ')]
for a in ACTORS.get_all_level_actors():
    if a.get_actor_label().startswith('CTV6 '):ACTORS.destroy_actor(a)
for name,mesh in meshes.items():
    actor=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(0,0,0),unreal.Rotator(0,0,0))
    actor.set_actor_label('CTV6 '+name);actor.tags=[unreal.Name('HERO_ConceptTerrainV6')]
    c=actor.static_mesh_component;c.modify();c.set_static_mesh(mesh)
    c.set_collision_profile_name(unreal.Name('NoCollision'));c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    c.set_editor_property('can_ever_affect_navigation',False)
assert sorted(original)==sorted(a.get_actor_label() for a in ACTORS.get_all_level_actors() if not a.get_actor_label().startswith('CTV6 '))
assert LEVEL.save_current_level()
report=dict(map='/Game/Maps/CityHero',actors=len(meshes),triangles=manifest['total_triangles'],navigation_unchanged=True,river_and_bridges_unchanged=True,material=stone.get_path_name(),assets=list(meshes),source_manifest='Art/Models/ConceptTerrainV6/manifest.json')
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/ConceptTerrainV6-Import.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('CONCEPT_TERRAIN_V6_IMPORT_PASS '+json.dumps(report))
