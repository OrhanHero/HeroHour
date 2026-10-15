"""Idempotent environment pass; preserves gameplay coordinates and existing landmarks."""
from pathlib import Path
import unreal,random,json
ROOT=Path(__file__).resolve().parents[2]
random.seed(81)
LIB=unreal.MaterialEditingLibrary;TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
DEST='/Game/HERO/EnvironmentV2'
def node(m,kind):return LIB.create_material_expression(m,kind,0,0)
def constant(m,value):
    n=node(m,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);return n
def color(m,value):
    n=node(m,unreal.MaterialExpressionConstant3Vector);n.set_editor_property('constant',unreal.LinearColor(*value,1));return n
def mix(m,a,b,alpha):
    n=node(m,unreal.MaterialExpressionLinearInterpolate)
    LIB.connect_material_expressions(a,'',n,'A');LIB.connect_material_expressions(b,'',n,'B');LIB.connect_material_expressions(alpha,'',n,'Alpha');return n
def noise(m,scale):
    n=node(m,unreal.MaterialExpressionNoise);n.set_editor_property('scale',scale);n.set_editor_property('quality',1)
    n.set_editor_property('output_min',0);n.set_editor_property('output_max',1)
    return n
def surface(name,dark,light,rough,scale):
    path=DEST+'/Materials/M_ENV_'+name
    m=unreal.load_asset(path)
    if not m:m=TOOLS.create_asset('M_ENV_'+name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    LIB.delete_all_material_expressions(m)
    base=mix(m,color(m,dark),color(m,light),noise(m,scale))
    LIB.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
    LIB.connect_material_property(constant(m,rough),'',unreal.MaterialProperty.MP_ROUGHNESS)
    LIB.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m);return m
sand=surface('Sand',(.39,.285,.165),(.46,.35,.225),.97,.00045)
stone=surface('Sandstone',(.27,.205,.14),(.36,.28,.19),.91,.005)
road=surface('Asphalt',(.065,.067,.062),(.073,.075,.069),.93,.009)
water=surface('Water',(.013,.067,.074),(.018,.082,.087),.32,.0007)
# A geometric ring leaves terrain visible without an opacity shader.
selection=surface('SelectionRing',(.035,.65,.34),(.035,.65,.34),.5,.001)
selection.set_editor_property('blend_mode',unreal.BlendMode.BLEND_OPAQUE)
selection.set_editor_property('shading_model',unreal.MaterialShadingModel.MSM_UNLIT)
selection.set_editor_property('two_sided',True)
LIB.connect_material_property(color(selection,(.035,.65,.34)),'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
LIB.recompile_material(selection);unreal.EditorAssetLibrary.save_loaded_asset(selection)
meshes={}
for name in ['SM_ENV_Bank_East','SM_ENV_Bank_West','SM_ENV_SelectionRing']+['SM_ENV_Sandstone_'+str(i) for i in range(4)]:
    task=unreal.AssetImportTask();task.filename=str(ROOT/'Art/Models/EnvironmentV2'/(name+'.fbx'))
    task.destination_path=DEST+'/Meshes';task.destination_name=name;task.automated=True;task.save=True;task.replace_existing=True
    options=unreal.FbxImportUI();options.import_materials=False;options.import_textures=False
    options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False
    task.options=options;TOOLS.import_asset_tasks([task]);mesh=unreal.load_asset(DEST+'/Meshes/'+name);assert mesh,name
    mesh.set_material(0,selection if name=='SM_ENV_SelectionRing' else stone);unreal.EditorAssetLibrary.save_loaded_asset(mesh);meshes[name]=mesh
    unreal.log('ENV_BOUNDS '+name+' '+str(mesh.get_bounds()))
LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert LEVEL.load_level('/Game/Maps/CityHero')
for actor in ACTORS.get_all_level_actors():
    label=actor.get_actor_label()
    if label.startswith('ENV2 ') or label.startswith('Riverbank'):ACTORS.destroy_actor(actor);continue
    if isinstance(actor,unreal.StaticMeshActor):
        component=actor.static_mesh_component
        if label=='CITYHERO Desert terrain':component.set_material(0,sand);component.set_material(1,stone)
        elif label=='CITYHERO River':
            component.set_material(0,water)
            position=actor.get_actor_location();position.z=-145;actor.set_actor_location(position,False,False)
        elif any(s in label.lower() for s in ('road','street','avenue','lane','parking','runway')) and 'marking' not in label.lower():component.set_material(0,road)
        if label.startswith('Downtown tower'):
            position=actor.get_actor_location();scale=actor.get_actor_scale3d()
            height_rng=random.Random(round(position.x)*31+round(position.y))
            scale.z=height_rng.uniform(.9,1.5);actor.set_actor_scale3d(scale)
        if label.startswith('East road'):
            position=actor.get_actor_location();position.x=5100;actor.set_actor_location(position,False,False)
            scale=actor.get_actor_scale3d();scale.x=148;actor.set_actor_scale3d(scale)
        if label.startswith('West road'):
            position=actor.get_actor_location();position.x=-7600;actor.set_actor_location(position,False,False)
            scale=actor.get_actor_scale3d();scale.x=78;actor.set_actor_scale3d(scale)
    if isinstance(actor,unreal.DirectionalLight):
        actor.set_actor_rotation(unreal.Rotator(pitch=-42,yaw=-32,roll=0),False)
        actor.light_component.set_editor_property('intensity',5.5)
        actor.light_component.set_editor_property('light_color',unreal.Color(r=255,g=240,b=221,a=255))
    if isinstance(actor,unreal.SkyLight):actor.light_component.set_editor_property('intensity',1.35)
def prop(name,pos,scale=(1,1,1),yaw=0):
    a=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos),unreal.Rotator(pitch=0,yaw=yaw,roll=0))
    a.set_actor_label('ENV2 '+name);a.static_mesh_component.set_static_mesh(meshes[name]);a.set_actor_scale3d(unreal.Vector(*scale))
    a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);return a
for side in ('East','West'):prop('SM_ENV_Bank_'+side,(0,0,0))
# Outcrops sit outside the playable bounds; river stones stay in impassable water.
for i in range(95):
    x=random.uniform(-14500,14500);y=random.choice((-1,1))*random.uniform(9400,11300)
    prop('SM_ENV_Sandstone_'+str(i%4),(x,y,-50),(random.uniform(1.4,3.2),random.uniform(1.2,2.6),random.uniform(.8,2)),random.uniform(0,360))
for i in range(45):
    y=random.uniform(-8600,8600)
    if min(abs(y-3200),abs(y+3200))<1050:continue
    x=-3000+random.choice((-1,1))*random.uniform(500,590)
    prop('SM_ENV_Sandstone_'+str(i%4),(x,y,-130),(.12,.15,.22),random.uniform(0,360))
assert LEVEL.save_current_level()
report={'map':'/Game/Maps/CityHero','seed':81,'navigation_changed':False,'environment_meshes':list(meshes),'playable_extents_cm':[12000,9000]}
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/environment-v2.json').write_text(json.dumps(report,indent=2))
unreal.log('ENVIRONMENT_V2_POLISH_PASS')
