from pathlib import Path
import unreal, os, math, random
random.seed(12)
tools=unreal.AssetToolsHelpers.get_asset_tools()
lib=unreal.MaterialEditingLibrary
palette={
 'Armor':((.11,.15,.085),.6,.72,0), 'Rust':((.22,.075,.028),.5,.83,0),
 'Rubber':((.012,.014,.013),0,.92,0), 'Steel':((.09,.12,.13),.8,.5,0),
 'Glass':((.013,.055,.07),.5,.28,.1), 'Concrete':((.20,.22,.21),0,.95,0),
 'Dark':((.018,.023,.026),.3,.8,0), 'Neon':((.015,.7,.38),0,.3,3),
 'Lamp':((1,.5,.12),0,.3,4), 'Paint':((.5,.4,.20),.35,.7,0),
 'Team':((.04,.30,.20),.45,.65,0), 'Asphalt':((.035,.044,.049),0,.96,0),
 'Pavement':((.16,.18,.18),0,.94,0), 'Line':((.55,.49,.25),0,.9,0)}
materials={}
for name,(color,metal,rough,emissive) in palette.items():
 path='/Game/HERO/Materials/M_'+name
 m=unreal.load_asset(path)
 if not m:
  m=tools.create_asset('M_'+name,'/Game/HERO/Materials',unreal.Material,unreal.MaterialFactoryNew())
  tint=lib.create_material_expression(m,unreal.MaterialExpressionVectorParameter,-600,0)
  tint.set_editor_property('parameter_name','Tint'); tint.set_editor_property('default_value',unreal.LinearColor(*color,1))
  lib.connect_material_property(tint,'',unreal.MaterialProperty.MP_BASE_COLOR)
  for value,prop,y in ((metal,unreal.MaterialProperty.MP_METALLIC,200),(rough,unreal.MaterialProperty.MP_ROUGHNESS,300)):
   n=lib.create_material_expression(m,unreal.MaterialExpressionConstant,-600,y); n.set_editor_property('r',value); lib.connect_material_property(n,'',prop)
  if emissive:
   strength=lib.create_material_expression(m,unreal.MaterialExpressionConstant,-600,400); strength.set_editor_property('r',emissive)
   mult=lib.create_material_expression(m,unreal.MaterialExpressionMultiply,-200,400)
   lib.connect_material_expressions(tint,'',mult,'A'); lib.connect_material_expressions(strength,'',mult,'B'); lib.connect_material_property(mult,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
  else:
   # Local-space mottling breaks up flat surfaces without external textures.
   noise=lib.create_material_expression(m,unreal.MaterialExpressionNoise,-600,-250)
   noise.set_editor_property('scale',.06)
   lerp=lib.create_material_expression(m,unreal.MaterialExpressionLinearInterpolate,-250,-100)
   lib.connect_material_expressions(tint,'',lerp,'A'); lib.connect_material_expressions(tint,'',lerp,'B')
   # Roughness variation gives concrete and metal less uniform highlights.
   lib.connect_material_expressions(noise,'',m and lerp,'Alpha')
  lib.recompile_material(m); unreal.EditorAssetLibrary.save_loaded_asset(m)
 materials[name]=m
meshes={}
for name in ('SM_HERO_Technical','SM_HERO_BlackMarket','SM_UrbanBlock','SM_CargoContainer','SM_OilDrums','SM_StreetLamp','SM_Rubble','SM_HERO_Salvager','SM_HERO_Workshop','SM_HERO_Depot'):
 task=unreal.AssetImportTask(); task.set_editor_property('filename',os.path.join(str(Path(__file__).resolve().parents[2] / 'Art/Models/Generated'),name+'.fbx'))
 task.set_editor_property('destination_path','/Game/HERO/Meshes'); task.set_editor_property('destination_name',name)
 task.set_editor_property('automated',True); task.set_editor_property('save',True); task.set_editor_property('replace_existing',True)
 options=unreal.FbxImportUI(); options.set_editor_property('import_mesh',True); options.set_editor_property('import_materials',False); options.set_editor_property('import_textures',False)
 options.set_editor_property('import_as_skeletal',False); options.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_STATIC_MESH)
 options.static_mesh_import_data.set_editor_property('combine_meshes',True)
 options.static_mesh_import_data.set_editor_property('auto_generate_collision',True)
 task.set_editor_property('options',options); tools.import_asset_tasks([task])
 mesh=unreal.load_asset('/Game/HERO/Meshes/'+name)
 assert mesh, name
 for i,slot in enumerate(mesh.get_editor_property('static_materials')):
  key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
  if key in materials: mesh.set_material(i,materials[key])
 unreal.EditorAssetLibrary.save_loaded_asset(mesh); meshes[name]=mesh
 unreal.log('HERO_MESH_BOUNDS '+name+' '+str(mesh.get_bounds().box_extent))
assert unreal.EditorLevelLibrary.load_level('/Game/Maps/Prototype')
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
 if actor.get_actor_label().startswith('HERO_ENV_'): unreal.EditorLevelLibrary.destroy_actor(actor)
cube=unreal.load_asset('/Engine/BasicShapes/Cube')
def prop(mesh,loc,label,rot=0,scale=None):
 a=unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*loc),unreal.Rotator(pitch=0,yaw=rot,roll=0))
 a.set_actor_label('HERO_ENV_'+label); a.static_mesh_component.set_static_mesh(mesh)
 if scale: a.set_actor_scale3d(unreal.Vector(*scale))
 return a
def slab(loc,size,mat,label):
 a=prop(cube,loc,label,scale=tuple(v/100 for v in size)); a.static_mesh_component.set_material(0,materials[mat]); return a
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
 if actor.get_actor_label()=='Prototype Ground':
  actor.set_actor_scale3d(unreal.Vector(100,100,1)); actor.static_mesh_component.set_material(0,materials['Asphalt'])
 if isinstance(actor,unreal.DirectionalLight):
  actor.set_actor_rotation(unreal.Rotator(pitch=-48,yaw=-35,roll=0),False)
  actor.light_component.set_editor_property('intensity',9.0)
  actor.light_component.set_editor_property('light_color',unreal.Color(r=226,g=231,b=240,a=255))
 if isinstance(actor,unreal.SkyLight):
  actor.light_component.set_editor_property('intensity',.7)
slab((0,0,1),(5600,1120,2),'Asphalt','MainRoad')
for y in (-660,660):
 slab((0,y,6),(5700,180,12),'Pavement','Sidewalk')
 slab((0,y-100 if y>0 else y+100,12),(5700,18,24),'Concrete','Curb')
for x in range(-2600,2800,260): slab((x,0,3),(115,12,3),'Line','LaneMark')
for x in range(-2400,2800,620):
 for side in (-1,1):
  y=side*1330
  prop(meshes['SM_UrbanBlock'],(x,y,0),'CityBlock',90 if side==1 else -90,scale=(1,1,random.uniform(.8,1.5)))
for x in (-2200,-600,1000,2400):
 for side in (-1,1):
  prop(meshes['SM_StreetLamp'],(x,side*730,0),'StreetLamp',90 if side==1 else -90)
  light=unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight,unreal.Vector(x,side*675,420))
  light.set_actor_label('HERO_ENV_StreetLight'); light.light_component.set_editor_property('intensity',850)
  light.light_component.set_editor_property('attenuation_radius',650)
  light.light_component.set_editor_property('light_color',unreal.Color(r=255,g=188,b=126,a=255))
for x,y,rot in ((-2050,850,0),(-1150,-920,8),(1950,-850,0),(850,1000,90)):
 prop(meshes['SM_CargoContainer'],(x,y,0),'Container',rot,scale=(.7,.7,.7))
for i in range(25):
 x=random.uniform(-2600,2600); y=random.choice((-1,1))*random.uniform(760,960)
 prop(meshes['SM_Rubble'],(x,y,0),'Debris',random.uniform(0,360),scale=(.8,.8,.8))
for x,y in ((-1550,830),(-1150,-850),(1700,780),(1050,-820)):
 prop(meshes['SM_OilDrums'],(x,y,0),'Drums')
# Salvage heaps coincide with economy locations. No collision: collectors access the center.
for x,y in ((-650,400),(650,400),(0,400)):
 a=prop(meshes['SM_Rubble'],(x,y,4),'Salvage',scale=(1.2,1.2,1.2))
 a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
 slab((x,y,3),(180,180,3),'Paint','SalvagePad').static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
# Green-lit repair bays flank the road without obstructing the battle corridor.
for x in (-1800,1800):
 for y in (-350,350):
  slab((x,y,4),(550,8,4),'Neon','BaseBoundary')
# Backdrop, atmosphere and fixed exposure.
post=unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PostProcessVolume,unreal.Vector())
post.set_actor_label('HERO_ENV_Exposure'); post.set_editor_property('unbound',True)
settings=post.get_editor_property('settings')
settings.set_editor_property('override_auto_exposure_method',True); settings.set_editor_property('auto_exposure_method',unreal.AutoExposureMethod.AEM_MANUAL)
settings.set_editor_property('override_auto_exposure_bias',True); settings.set_editor_property('auto_exposure_bias',0.0)
settings.set_editor_property('override_auto_exposure_apply_physical_camera_exposure',True)
settings.set_editor_property('auto_exposure_apply_physical_camera_exposure',False)
post.set_editor_property('settings',settings)
unreal.EditorLevelLibrary.set_level_viewport_camera_info(unreal.Vector(-2700,-2700,3200),unreal.Rotator(pitch=-52,yaw=45,roll=0))
assert unreal.EditorLevelLibrary.save_current_level()
unreal.log('HERO_ART_PASS_SAVED')
