"""Import original district kit and dress CityHero without moving gameplay start points."""
from pathlib import Path
import json,unreal,math,random
ROOT=Path(__file__).resolve().parents[2];DEST='/Game/HERO/DistrictV3'
TOOLS=unreal.AssetToolsHelpers.get_asset_tools();LIB=unreal.MaterialEditingLibrary;ASSETS=unreal.EditorAssetLibrary
META=json.loads((ROOT/'Art/Textures/DistrictV3/metadata.json').read_text())
def load(path):return unreal.load_asset(path) if ASSETS.does_asset_exist(path) else None
def expr(m,kind):return LIB.create_material_expression(m,kind,0,0)
def link(source,output,target,input_name):
    assert LIB.connect_material_expressions(source,output,target,input_name),(source.get_class().get_name(),target.get_class().get_name(),input_name)
def scalar(m,value):
    n=expr(m,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);return n
def col(m,value):
    n=expr(m,unreal.MaterialExpressionConstant3Vector);n.set_editor_property('constant',unreal.LinearColor(*value,1));return n
textures={}
for name,info in META['materials'].items():
    textures[name]={}
    for role,file in info['files'].items():
        task=unreal.AssetImportTask();task.filename=str(ROOT/'Art/Textures/DistrictV3'/file)
        task.destination_path=DEST+'/Textures';task.destination_name='T_DV3_'+name+'_'+role
        task.automated=True;task.replace_existing=True;task.save=True;TOOLS.import_asset_tasks([task])
        t=load(DEST+'/Textures/'+task.destination_name);assert t
        t.set_editor_property('srgb',role=='Albedo')
        if role=='Normal':t.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
        elif role=='Roughness':t.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_MASKS)
        ASSETS.save_loaded_asset(t);textures[name][role]=t
def material(name,world=False,source=None,tint=(1,1,1)):
    source=source or name;suffix='_World' if world else ''
    asset_name='M_DV3_'+name+suffix;m=load(DEST+'/Materials/'+asset_name)
    if not m:m=TOOLS.create_asset(asset_name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    LIB.delete_all_material_expressions(m)
    if source in textures:
        if world:
            pos=expr(m,unreal.MaterialExpressionWorldPosition)
            xy=expr(m,unreal.MaterialExpressionComponentMask)
            for key,value in [('r',True),('g',True),('b',False),('a',False)]:xy.set_editor_property(key,value)
            link(pos,'',xy,'')
            uv=expr(m,unreal.MaterialExpressionDivide)
            link(xy,'',uv,'A');link(scalar(m,META['materials'][source]['tile_meters']*100),'',uv,'B')
        else:
            coords=expr(m,unreal.MaterialExpressionTextureCoordinate)
            uv=expr(m,unreal.MaterialExpressionDivide)
            link(coords,'',uv,'A');link(scalar(m,META['materials'][source]['tile_meters']),'',uv,'B')
        for role,texture in textures[source].items():
            n=expr(m,unreal.MaterialExpressionTextureSample);n.set_editor_property('texture',texture)
            n.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if role=='Normal' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if role=='Albedo' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
            link(uv,'',n,'UVs')
            output='RGB' if role!='Roughness' else 'R'
            target=unreal.MaterialProperty.MP_BASE_COLOR if role=='Albedo' else unreal.MaterialProperty.MP_NORMAL if role=='Normal' else unreal.MaterialProperty.MP_ROUGHNESS
            if role=='Albedo' and tint!=(1,1,1):
                mult=expr(m,unreal.MaterialExpressionMultiply);link(n,output,mult,'A');link(col(m,tint),'',mult,'B');n=mult;output=''
            LIB.connect_material_property(n,output,target)
    else:
        color=(.026,.085,.095) if name=='Glass' else (.52,.47,.30) if name=='Paint' else (.095,.12,.04) if name=='Foliage' else (.32,.29,.23)
        LIB.connect_material_property(col(m,color),'',unreal.MaterialProperty.MP_BASE_COLOR)
        LIB.connect_material_property(scalar(m,.22 if name=='Glass' else .82),'',unreal.MaterialProperty.MP_ROUGHNESS)
        LIB.connect_material_property(scalar(m,.5 if name=='Glass' else 0),'',unreal.MaterialProperty.MP_METALLIC)
    if name=='Foliage':m.set_editor_property('two_sided',True)
    LIB.recompile_material(m);ASSETS.save_loaded_asset(m);return m
mats={name:material(name,tint=(.68,.66,.62) if name=='Plaster' else (1.45,1.4,1.3) if name=='Asphalt' else (1,1,1)) for name in ('Sand','Asphalt','Concrete','Plaster','Metal','Glass','Paint','Foliage','Gravel','CompactedDirt')}
mats['Sandstone']=material('Sandstone',source='Gravel',tint=(.85,.8,.72))
mats['Dust']=material('Dust',source='Sand',tint=(.75,.75,.75))
mats['TempestEnergy']=material('TempestEnergy')
energy=mats['TempestEnergy'];LIB.delete_all_material_expressions(energy)
LIB.connect_material_property(col(energy,(.72,.055,.012)),'',unreal.MaterialProperty.MP_BASE_COLOR)
activity=expr(energy,unreal.MaterialExpressionScalarParameter)
activity.set_editor_property('parameter_name','FieldActivity');activity.set_editor_property('default_value',1.0)
activity.set_editor_property('use_custom_primitive_data',True);activity.set_editor_property('primitive_data_index',0)
glow=expr(energy,unreal.MaterialExpressionMultiply)
link(col(energy,(2.1,.16,.035)),'',glow,'A');link(activity,'',glow,'B')
LIB.connect_material_property(glow,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
LIB.connect_material_property(scalar(energy,.4),'',unreal.MaterialProperty.MP_ROUGHNESS)
LIB.recompile_material(energy);ASSETS.save_loaded_asset(energy)
root_material=material('TempestRoot',source='Gravel',tint=(.16,.14,.12))
world_mats={name:material(name,True,tint=(1.45,1.4,1.3) if name=='Asphalt' else (1,1,1)) for name in ('Sand','Asphalt','Concrete')}
def layered_ground():
    name='M_DV3_LayeredGround';m=load(DEST+'/Materials/'+name)
    if not m:m=TOOLS.create_asset(name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    LIB.delete_all_material_expressions(m)
    pos=expr(m,unreal.MaterialExpressionWorldPosition);xy=expr(m,unreal.MaterialExpressionComponentMask)
    xy.set_editor_property('r',True);xy.set_editor_property('g',True);link(pos,'',xy,'')
    uv=expr(m,unreal.MaterialExpressionDivide);link(xy,'',uv,'A');link(scalar(m,400),'',uv,'B')
    def patch(scale,threshold,power):
        n=expr(m,unreal.MaterialExpressionNoise);n.set_editor_property('scale',scale);n.set_editor_property('quality',1)
        n.set_editor_property('output_min',0);n.set_editor_property('output_max',1)
        sub=expr(m,unreal.MaterialExpressionSubtract);link(n,'',sub,'A');link(scalar(m,threshold),'',sub,'B')
        mult=expr(m,unreal.MaterialExpressionMultiply);link(sub,'',mult,'A');link(scalar(m,power),'',mult,'B')
        sat=expr(m,unreal.MaterialExpressionSaturate);link(mult,'',sat,'');return sat
    gravel_mask=patch(.00045,.58,1.0);dirt_mask=patch(.00022,.54,.75)
    def sample(source,role):
        n=expr(m,unreal.MaterialExpressionTextureSample);n.set_editor_property('texture',textures[source][role])
        n.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if role=='Normal' else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if role=='Albedo' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
        link(uv,'',n,'UVs');return n
    for role,target in [('Albedo',unreal.MaterialProperty.MP_BASE_COLOR),('Normal',unreal.MaterialProperty.MP_NORMAL),('Roughness',unreal.MaterialProperty.MP_ROUGHNESS)]:
        output='R' if role=='Roughness' else 'RGB'
        base=sample('Sand',role)
        first=expr(m,unreal.MaterialExpressionLinearInterpolate)
        link(base,output,first,'A');link(sample('Gravel',role),output,first,'B');link(gravel_mask,'',first,'Alpha')
        second=expr(m,unreal.MaterialExpressionLinearInterpolate)
        link(first,'',second,'A');link(sample('CompactedDirt',role),output,second,'B');link(dirt_mask,'',second,'Alpha')
        if role=='Normal':
            normalized=expr(m,unreal.MaterialExpressionNormalize);link(second,'',normalized,'VectorInput');second=normalized
        if role=='Albedo':
            toned=expr(m,unreal.MaterialExpressionMultiply)
            link(second,'',toned,'A');link(col(m,(1.0,.9,.78)),'',toned,'B');second=toned
        LIB.connect_material_property(second,'',target)
    LIB.recompile_material(m);ASSETS.save_loaded_asset(m);return m
ground=layered_ground()
meshes={};records=[]
for file in sorted((ROOT/'Art/Models/DistrictV3').glob('SM_DV3_*.fbx')):
    name=file.stem;task=unreal.AssetImportTask();task.filename=str(file);task.destination_path=DEST+'/Meshes';task.destination_name=name
    task.automated=True;task.replace_existing=True;task.save=True
    settings=unreal.FbxImportUI();settings.import_materials=False;settings.import_textures=False
    settings.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    settings.static_mesh_import_data.combine_meshes=True;settings.static_mesh_import_data.auto_generate_collision=False
    task.options=settings;TOOLS.import_asset_tasks([task]);mesh=load(DEST+'/Meshes/'+name);assert mesh,name
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0];assert key in mats,(name,key)
        mesh.set_material(i,world_mats.get(key,mats[key]) if name not in ('SM_DV3_Civic_0','SM_DV3_Civic_1','SM_DV3_Civic_2','SM_DV3_Lamp','SM_DV3_Barrier') else mats[key])
        if name=='SM_DV3_TempestPatch' and key=='Sandstone':mesh.set_material(i,root_material)
    b=mesh.get_bounding_box();s=b.max-b.min
    expected=json.loads(file.with_suffix('.json').read_text())['dimensions_m']
    assert all(abs(v-e*100)<max(2,e*2) for v,e in zip((s.x,s.y,s.z),expected)),(name,s,expected)
    ASSETS.save_loaded_asset(mesh);meshes[name]=mesh;records.append({'name':name,'bounds_cm':[s.x,s.y,s.z]})
LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert LEVEL.load_level('/Game/Maps/CityHero')
for actor in ACTORS.get_all_level_actors():
    label=actor.get_actor_label()
    if label.startswith('DV3 ') or label.startswith('Downtown tower') or label.startswith('Downtown block pavement') or label.startswith('Desert rocks') or label.startswith('Salvage supply'):
        ACTORS.destroy_actor(actor);continue
    if isinstance(actor,unreal.DirectionalLight):
        actor.light_component.set_editor_property('light_color',unreal.Color(r=255,g=244,b=221,a=255))
        actor.light_component.set_editor_property('light_source_angle',1.2)
    if isinstance(actor,unreal.SkyLight):actor.light_component.set_editor_property('intensity',1.5)
    if not isinstance(actor,unreal.StaticMeshActor):continue
    c=actor.static_mesh_component
    if label.startswith('North south road') and abs(actor.get_actor_location().x+8000)<1:
        ACTORS.destroy_actor(actor);continue
    if label=='CITYHERO Desert terrain':
        if 'SM_DV3_Terrain' in meshes:c.set_static_mesh(meshes['SM_DV3_Terrain'])
        c.set_material(0,ground);c.set_material(1,mats['Sandstone'])
    elif label=='CITYHERO River':
        scale=actor.get_actor_scale3d();scale.y=270;actor.set_actor_scale3d(scale)
    elif label.startswith('Bridge arterial road'):
        scale=actor.get_actor_scale3d();scale.y=9;actor.set_actor_scale3d(scale);c.set_material(0,world_mats['Asphalt'])
    elif any(s in label.lower() for s in ('road','street','avenue','lane','parking','runway')) and 'marking' not in label.lower():c.set_material(0,world_mats['Asphalt'])
    elif label.startswith('West Heights dwelling'):
        for i,slot in enumerate(c.static_mesh.get_editor_property('static_materials')):
            key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
            if key=='Concrete':c.set_material(i,mats['Plaster'])
            elif key in ('Paint','Steel'):c.set_material(i,mats['Concrete' if key=='Paint' else 'Metal'])
def spawn(name,pos,yaw=0,scale=(1,1,1)):
    a=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos),unreal.Rotator(0,yaw,0));a.set_actor_label('DV3 '+name)
    a.static_mesh_component.set_static_mesh(meshes[name]);a.set_actor_scale3d(unreal.Vector(*scale))
    a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);return a
for col,x in enumerate((1500,3000,4500,6000)):
    for row,y in enumerate((-4500,-3000,-1500,0,1500,3000,4500)):
        style=2 if col in (1,2) and abs(y)<=1500 else (row+col)%2
        spawn('SM_DV3_Pavement',(x,y,0),scale=(1.7,1.7,1))
        spawn('SM_DV3_Civic_'+str(style),(x,y,12),yaw=(row+col)%4*90,scale=(2,2,1))
        # Street furniture is inside the same blocked city footprint, not on a driving lane.
        spawn('SM_DV3_Lamp',(x-470,y-250,12));spawn('SM_DV3_Drain',(x+480,y,13),90)
for x in (-8000,8000):
    spawn('SM_DV3_BaseApron',(x,0,2))
    for y in (-1100,1100):
        for dx in (-1000,1000):spawn('SM_DV3_Lamp',(x+dx,y,0))
for x,y in ((-7500,1600),(7500,1600),(7600,-1600)):
    spawn('SM_DV3_ServicePad',(x,y,1))
spawn('SM_DV3_BaseApproach',(-8000,0,2))
for index,(x,y) in enumerate(((-6500,1600),(7000,750),(0,-3200))):
    cache=spawn('SM_DV3_TempestPatch' if 'SM_DV3_TempestPatch' in meshes else 'SM_DV3_SupplyCache',(x,y+400,0))
    cache.set_editor_property('tags',[unreal.Name('HERO_Salvage_'+str(index))])
# Finished southern arterial: sidewalks, curbs and line markings in measured modules.
for x in range(-10800,10801,1200):
    if -4300<x<-1700:continue
    spawn('SM_DV3_RoadStraight',(x,-3200,5))
    if (x+10800)%2400==0:
        spawn('SM_DV3_Lamp',(x,-3780,0))
for x in (-8000,0,7500):spawn('SM_DV3_Crosswalk',(x,-3200,10),90)
for x in (750,2250,3750,5250,6750):
    for y in (-4500,-3000,-1500,0,1500,3000,4500):spawn('SM_DV3_UrbanStreet',(x,y,6),90)
for y in (-5250,-3750,-2250,-750,750,2250,3750,5250):
    for x in (1500,3000,4500,6000):spawn('SM_DV3_UrbanStreet',(x,y,6))
# Small groundcover has no navigation collision. Keep it clear of roads and actual footprints.
rng=random.Random(241014)
for i in range(1600):
    x=rng.uniform(-11500,11500);y=rng.uniform(-8400,8400)
    if abs(x+3000)<1100 or abs(y-3200)<850 or abs(y+3200)<850:continue
    if any(abs(x-road)<450 for road in (-8000,-5000,0,7500)):continue
    if any(abs(y-road)<450 for road in (-6000,0,6000)):continue
    if 0<x<7300 and abs(y)<6100:continue
    if -10800<x<-4700 and 1900<y<8000:continue
    if any(math.hypot(x-px,y-py)<1300 for px,py in ((-8000,0),(8000,0),(3000,7000),(-9000,-6000),(8900,3900),(10300,5600),(8700,7000))):continue
    if any(math.hypot(x-px,y-py)<650 for px,py in ((-7500,1600),(7500,1600),(7600,-1600))):continue
    name='SM_DV3_PebbleCluster' if i%5<2 and 'SM_DV3_PebbleCluster' in meshes else 'SM_DV3_DesertShrub' if i%3==0 else 'SM_DV3_DryGrass'
    size=rng.uniform(.65,1.2);spawn(name,(x,y,0),rng.uniform(0,360),(size,size,size))
assert LEVEL.save_current_level()
(ROOT/'Saved/Research/district-v3-import.json').write_text(json.dumps({'meshes':records,'materials':list(mats),'city_blocker_radius_cm':600,'city_centres':28},indent=2))
unreal.log('DISTRICT_V3_IMPORT_PASS')
