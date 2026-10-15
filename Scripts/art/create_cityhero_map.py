from pathlib import Path
"""CITY HERO geography derived from the user's supplied map concepts."""
import unreal, os, random
random.seed(31)
tools=unreal.AssetToolsHelpers.get_asset_tools(); ml=unreal.MaterialEditingLibrary
def material(name,color,rough=.8,noise=False):
 path='/Game/HERO/Materials/M_CH_'+name
 m=unreal.load_asset(path)
 if m: return m
 m=tools.create_asset('M_CH_'+name,'/Game/HERO/Materials',unreal.Material,unreal.MaterialFactoryNew())
 c=ml.create_material_expression(m,unreal.MaterialExpressionVectorParameter,-600,0)
 c.set_editor_property('parameter_name','Tint'); c.set_editor_property('default_value',unreal.LinearColor(*color,1))
 if noise:
  n=ml.create_material_expression(m,unreal.MaterialExpressionNoise,-600,-250); n.set_editor_property('scale',.018)
  d=ml.create_material_expression(m,unreal.MaterialExpressionMultiply,-350,100); d.set_editor_property('const_b',.62); ml.connect_material_expressions(c,'',d,'A')
  l=ml.create_material_expression(m,unreal.MaterialExpressionLinearInterpolate,-100,0)
  ml.connect_material_expressions(d,'',l,'A'); ml.connect_material_expressions(c,'',l,'B'); ml.connect_material_expressions(n,'',l,'Alpha'); c=l
 ml.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 r=ml.create_material_expression(m,unreal.MaterialExpressionConstant,-100,200); r.set_editor_property('r',rough); ml.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
 ml.recompile_material(m); unreal.EditorAssetLibrary.save_loaded_asset(m); return m
sand=material('Sand',(.57,.38,.18),.96,True)
asphalt=material('Road',(.07,.073,.067),.94,True)
water=material('Water',(.025,.22,.25),.22,True)
stone=material('Stone',(.42,.37,.29),.94,True)
line=material('Line',(.64,.59,.38))
meshes={}
for name in ['SM_CH_Tower_0','SM_CH_Tower_1','SM_CH_Tower_2','SM_CH_House','SM_CH_Bridge','SM_CH_Refinery','SM_CH_Terrain','SM_CH_Stadium','SM_CH_PowerPlant']:
 task=unreal.AssetImportTask(); task.filename=os.path.join(str(Path(__file__).resolve().parents[2] / 'Art/Models/Generated'),name+'.fbx')
 task.destination_path='/Game/HERO/Meshes'; task.destination_name=name; task.automated=True; task.save=True; task.replace_existing=True
 options=unreal.FbxImportUI(); options.import_as_skeletal=False; options.import_materials=False; options.import_textures=False
 options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH; options.static_mesh_import_data.combine_meshes=True; options.static_mesh_import_data.auto_generate_collision=True
 task.options=options; tools.import_asset_tasks([task]); mesh=unreal.load_asset('/Game/HERO/Meshes/'+name); assert mesh,name
 for i,slot in enumerate(mesh.get_editor_property('static_materials')):
  key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
  mat=unreal.load_asset('/Game/HERO/Materials/M_'+key)
  if mat: mesh.set_material(i,mat)
 unreal.EditorAssetLibrary.save_loaded_asset(mesh); meshes[name]=mesh
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if unreal.EditorAssetLibrary.does_asset_exist('/Game/Maps/CityHero'):
 assert level.load_level('/Game/Maps/CityHero')
 for actor in actors.get_all_level_actors():
  if not isinstance(actor,unreal.WorldSettings): actors.destroy_actor(actor)
else:
 assert level.new_level('/Game/Maps/CityHero')
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
def prop(mesh,pos,label,scale=None,rot=0,mat=None):
 a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos),unreal.Rotator(pitch=0,yaw=rot,roll=0)); a.set_actor_label(label)
 a.static_mesh_component.set_static_mesh(mesh)
 if scale: a.set_actor_scale3d(unreal.Vector(*scale))
 if mat: a.static_mesh_component.set_material(0,mat)
 return a
def slab(pos,size,mat,label): return prop(cube,pos,label,tuple(v/100 for v in size),mat=mat)
terrain=prop(meshes['SM_CH_Terrain'],(0,0,0),'CITYHERO Desert terrain',mat=sand)
terrain.static_mesh_component.set_material(1,stone)
slab((-3000,0,-80),(1400,18200,20),water,'CITYHERO River')
for side in (-1,1): slab((-3000+side*740,0,-15),(80,18000,35),stone,'Riverbank')
for y in (-3200,3200):
 slab((0,y,1),(23500,1250,2),asphalt,'Bridge arterial road')
 prop(meshes['SM_CH_Bridge'],(-3000,y,2),'CITYHERO Bridge')
for x in (-8000,-5000,0,7500): slab((x,0,1),(550,17000,2),asphalt,'North south road')
for y in (-6000,0,6000):
 slab((-7850,y,1),(7300,450,2),asphalt,'West road')
 slab((4700,y,1),(15800,450,2),asphalt,'East road')
for x in (750,2250,3750,5250,6750): slab((x,0,2),(380,10600,2),asphalt,'Downtown avenue')
for y in (-5250,-3750,-2250,-750,750,2250,3750,5250): slab((3750,y,2),(6500,380,2),asphalt,'Downtown street')
for x in (1500,3000,4500,6000):
 for y in (-4500,-3000,-1500,0,1500,3000,4500):
  slab((x,y,4),(820,820,8),stone,'Downtown block pavement')
  for dx,dy in ((-120,-120),(-120,120),(120,-120),(120,120)):
   style=random.randrange(3); prop(meshes['SM_CH_Tower_'+str(style)],(x+dx,y+dy,8),'Downtown tower',scale=(.42,.42,random.uniform(.55,1.1)),rot=random.choice((0,90,180,270)))
for x in (-9500,-8250,-7000,-5750):
 for y in (3000,4250,5500,6750):
  for dx,dy in ((-190,-190),(-190,190),(190,-190),(190,190)):
   prop(meshes['SM_CH_House'],(x+dx,y+dy,0),'West Heights dwelling',scale=(.55,.55,.8),rot=180)
for x in (-10150,-8850,-7600,-6350,-5100): slab((x,4800,2),(250,6100,2),asphalt,'West Heights street')
for y in (2400,3650,4900,6150,7400): slab((-7600,y,2),(5600,250,2),asphalt,'West Heights lane')
for x,y in ((8900,3900),(10300,5600),(8700,7000)):
 prop(meshes['SM_CH_Refinery'],(x,y,0),'Refinery District processing',scale=(.65,.65,.65))
prop(meshes['SM_CH_Stadium'],(3000,7000,0),'CITYHERO Stadium')
slab((3000,7000,-1),(3200,2500,2),asphalt,'Stadium parking')
prop(meshes['SM_CH_PowerPlant'],(-9000,-6000,0),'CITYHERO Power plant')
slab((-9000,-6000,-1),(2200,1800,2),asphalt,'Power plant apron')
slab((9500,-6500,2),(4500,700,4),asphalt,'Airfield runway')
slab((9500,-5100,2),(4500,1400,4),stone,'Airfield apron')
for x in (8200,9300,10400):
 prop(unreal.load_asset('/Game/HERO/Meshes/SM_HERO_Workshop'),(x,-4650,4),'Airfield hangar',scale=(1.6,1.6,1.4))
for x in range(7600,11500,450): slab((x,-6500,6),(180,22,3),line,'Runway marking')
for x,y in ((-6500,1600),(6500,1600),(0,-3200)):
 prop(unreal.load_asset('/Game/HERO/Meshes/SM_Rubble'),(x,y,0),'Salvage supply',scale=(1.4,1.4,1.4))
for x in range(-11000,11500,700):
 for y in (-3200,3200):
  if abs(x+3000)>1000: slab((x,y,4),(250,12,3),line,'Road marking')
for i in range(130):
 x=random.uniform(-11700,11700); y=random.choice((-1,1))*random.uniform(7500,8600)
 prop(unreal.load_asset('/Game/HERO/Meshes/SM_Rubble'),(x,y,0),'Desert rocks',scale=(random.uniform(1,3),random.uniform(1,3),random.uniform(1,3)))
sun=actors.spawn_actor_from_class(unreal.DirectionalLight,unreal.Vector(0,0,5000),unreal.Rotator(pitch=-48,yaw=-35,roll=0))
sun.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE); sun.light_component.set_editor_property('intensity',7)
sun.light_component.set_editor_property('light_color',unreal.Color(r=255,g=232,b=201,a=255))
sky=actors.spawn_actor_from_class(unreal.SkyLight,unreal.Vector(0,0,1000)); sky.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
sky.light_component.set_editor_property('source_type',unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP); sky.light_component.set_editor_property('cubemap',unreal.load_asset('/Game/HERO/Lighting/DaylightAmbient')); sky.light_component.set_editor_property('intensity',1.1)
post=actors.spawn_actor_from_class(unreal.PostProcessVolume,unreal.Vector()); post.set_editor_property('unbound',True)
settings=post.get_editor_property('settings'); settings.set_editor_property('override_auto_exposure_method',True); settings.set_editor_property('auto_exposure_method',unreal.AutoExposureMethod.AEM_MANUAL)
settings.set_editor_property('override_auto_exposure_apply_physical_camera_exposure',True); settings.set_editor_property('auto_exposure_apply_physical_camera_exposure',False)
settings.set_editor_property('override_auto_exposure_bias',True); settings.set_editor_property('auto_exposure_bias',0); post.set_editor_property('settings',settings)
world=unreal.EditorLevelLibrary.get_editor_world(); world.get_world_settings().set_editor_property('force_no_precomputed_lighting',True)
unreal.EditorLevelLibrary.set_level_viewport_camera_info(unreal.Vector(-13000,-11000,18000),unreal.Rotator(pitch=-52,yaw=45,roll=0))
assert level.save_current_level(); unreal.log('CITYHERO_MAP_CREATED')
