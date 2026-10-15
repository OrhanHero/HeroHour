"""Presentation-only pass. Gameplay coordinates, river and all blockers remain unchanged."""
from pathlib import Path
import json,math,random
import unreal

ROOT=Path(__file__).resolve().parents[2]
DEST='/Game/HERO/PresentationV5'
LIB=unreal.MaterialEditingLibrary
ASSETS=unreal.EditorAssetLibrary
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

def material(name):
    m=unreal.load_asset(DEST+'/Materials/'+name)
    if not m:m=TOOLS.create_asset(name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    LIB.delete_all_material_expressions(m)
    return m
def node(m,kind):return LIB.create_material_expression(m,kind,0,0)
def link(a,out,b,pin):assert LIB.connect_material_expressions(a,out,b,pin)
def scalar(m,v):
    n=node(m,unreal.MaterialExpressionConstant);n.set_editor_property('r',v);return n
def color(m,v):
    n=node(m,unreal.MaterialExpressionConstant3Vector);n.set_editor_property('constant',unreal.LinearColor(*v,1));return n
def op(m,kind,a,b):
    n=node(m,kind);link(a,'',n,'A');link(b,'',n,'B');return n
def mix(m,a,b,mask):
    n=node(m,unreal.MaterialExpressionLinearInterpolate);link(a,'',n,'A');link(b,'',n,'B');link(mask,'',n,'Alpha');return n
def noise(m,scale):
    n=node(m,unreal.MaterialExpressionNoise)
    n.set_editor_property('scale',scale);n.set_editor_property('quality',1)
    n.set_editor_property('output_min',0);n.set_editor_property('output_max',1)
    return n
def surface(name,source,tile,dark,light,roughness):
    # Preflight dependencies before touching a saved graph.
    textures={role:unreal.load_asset('/Game/HERO/DistrictV3/Textures/T_DV3_'+source+'_'+role) for role in ['Albedo','Normal','Roughness']}
    assert all(textures.values()),source
    m=material(name)
    pos=node(m,unreal.MaterialExpressionWorldPosition)
    xy=node(m,unreal.MaterialExpressionComponentMask);xy.set_editor_property('r',True);xy.set_editor_property('g',True);link(pos,'',xy,'')
    uv=op(m,unreal.MaterialExpressionDivide,xy,scalar(m,tile))
    samples={}
    for role,texture in textures.items():
        s=node(m,unreal.MaterialExpressionTextureSample);s.set_editor_property('texture',texture)
        s.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if role=='Normal' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS if role=='Roughness' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
        link(uv,'',s,'UVs');samples[role]=s
    macro=noise(m,.00065)
    tint=mix(m,color(m,dark),color(m,light),macro)
    albedo=op(m,unreal.MaterialExpressionMultiply,samples['Albedo'],tint)
    LIB.connect_material_property(albedo,'',unreal.MaterialProperty.MP_BASE_COLOR)
    LIB.connect_material_property(samples['Normal'],'RGB',unreal.MaterialProperty.MP_NORMAL)
    LIB.connect_material_property(scalar(m,roughness),'',unreal.MaterialProperty.MP_ROUGHNESS)
    LIB.recompile_material(m);ASSETS.save_loaded_asset(m,only_if_is_dirty=False)
    return m

ground=surface('M_PV5_DesertGround','Sand',240.,(.69,.54,.35),(1.04,.95,.77),.94)
road=surface('M_PV5_WeatheredAsphalt','Asphalt',180.,(.62,.66,.64),(1.03,1.02,.94),.89)
stone=unreal.load_asset('/Game/HERO/DistrictV3/Materials/M_DV3_Sandstone')
assert LEVEL.load_level('/Game/Maps/CityHero')
actors=ACTORS.get_all_level_actors()
before=[a.get_actor_label() for a in actors if not a.get_actor_label().startswith('PV5 ')]
for a in actors:
    label=a.get_actor_label()
    if label.startswith('PV5 '):ACTORS.destroy_actor(a);continue
    if not isinstance(a,unreal.StaticMeshActor):continue
    c=a.static_mesh_component
    if label=='CITYHERO Desert terrain':c.set_material(0,ground)
    elif any(word in label.lower() for word in ['road','street','avenue','lane','parking','runway']) and 'marking' not in label.lower():
        # Marking/pavement meshes carry separate concrete/paint slots. Change asphalt only.
        for i in range(c.get_num_materials()):
            old=c.get_material(i)
            if old and any(s in old.get_name().lower() for s in ['asphalt','road']):c.set_material(i,road)

names=['RockCluster','DryScrub','CurbStrip','LowBarrier','SupplyCrates','PipeBundle']
meshes={n:unreal.load_asset(DEST+'/Meshes/SM_PV5_'+n) for n in names}
assert all(meshes.values()),'Import ground detail kit first'
placements=[]
def spawn(name,x,y,z=0,yaw=0,scale=1):
    a=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,z),unreal.Rotator(0,yaw,0))
    a.set_actor_label('PV5 '+name)
    a.tags=[unreal.Name('HERO_DecorationV5')]
    c=a.static_mesh_component;c.modify();c.set_static_mesh(meshes[name])
    # Set the serialized profile as well: BlockAll otherwise re-applies on reload.
    c.set_collision_profile_name(unreal.Name('NoCollision'))
    c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    try:c.set_editor_property('can_ever_affect_navigation',False)
    except Exception:unreal.log_warning('PV5 navigation relevance setter unavailable; custom A* excludes decorative actors')
    # The prototype's authoritative obstacle registry only includes unit structures;
    # these decorative static actors never become movement/build blockers.
    a.set_actor_scale3d(unreal.Vector(scale,scale,scale))
    placements.append(dict(mesh=name,position=[x,y,z],yaw=yaw,scale=scale,collision='NoCollision'))
    return a
rng=random.Random(150026)
# Low vegetation is traversable, like grass. Keep harvest pickups and apron lanes legible.
for hx in [-8000,8000]:
    for i in range(100):
        x=hx+rng.uniform(-2700,2700);y=rng.uniform(-2600,2600)
        if abs(x)>11500:continue
        if abs(x-hx)<1350 and abs(y)<1350:continue
        if abs(y)<400 or abs(y+3200)<650:continue
        if math.hypot(x-(-6500 if hx<0 else 6880),y-(1600 if hx<0 else 550))<1150:continue
        if math.hypot(x-(-7500 if hx<0 else 7500),y-1600)<850:continue
        if hx>0 and 8200<x<9800 and -2400<y<300:continue
        spawn('DryScrub',x,y,-5,rng.uniform(0,360),rng.uniform(.55,.95))
        if i%4==0:spawn('RockCluster',x+55,y-40,-3,rng.uniform(0,360),rng.uniform(.07,.14))
    # Supply props are inside the existing HQ and refinery blocker footprints.
    spawn('SupplyCrates',hx-420,-420,4,90,.8)
    spawn('PipeBundle',hx+420,-390,4,90,.7)
for dx in [-7500,7500]:
    spawn('PipeBundle',dx+230,1600-240,5,90,.65)
    spawn('SupplyCrates',dx-220,1600+220,5,0,.6)
# Already-blocked downtown parcels receive street-level detail, not new obstacles.
for col,x in enumerate([1500,3000,4500,6000]):
    for row,y in enumerate([-4500,-3000,-1500,0,1500,3000,4500]):
        spawn('SupplyCrates',x-300,y+230,8,90,.65)
        spawn('LowBarrier',x+300,y-270,4,90,.65)
# Border geology sits outside the playable rectangle. It never creates invisible blockers.
for sign in [-1,1]:
    for y in range(-8200,8201,650):
        spawn('RockCluster',sign*12350,y,0,rng.uniform(0,360),rng.uniform(1.1,1.8))
# Short existing-street edge sections read as worn drainage kerbs, only 20cm high.
for y in [-3200,3200]:
    for x in [-10400,-9200,-8000,-6800,-5600,0,1200,2400,3600,4800,7200,8400,9600]:
        spawn('CurbStrip',x,y-620,0,0,.85)
assert LEVEL.save_current_level()
after=[a.get_actor_label() for a in ACTORS.get_all_level_actors() if not a.get_actor_label().startswith('PV5 ')]
assert sorted(before)==sorted(after),'Presentation pass must retain every prior actor'
report=dict(seed=150026,materials=[ground.get_path_name(),road.get_path_name()],placements=placements,
            retained_original_actor_count=len(before),navigation='No changes to gameplay layout, unit structures, water or blockers',result='PASS')
(ROOT/'Saved/Research/PresentationV5-Scene.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('WORLD_PRESENTATION_V5_PASS '+str(len(placements)))
