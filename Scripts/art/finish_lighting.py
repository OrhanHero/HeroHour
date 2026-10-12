import unreal
assert unreal.EditorLevelLibrary.load_level('/Game/Maps/Prototype')
unreal.AssetRegistryHelpers.get_asset_registry().scan_paths_synchronous(['/Engine/MapTemplates/Sky'], True)
path='/Game/HERO/Lighting/DaylightAmbient'
cube=unreal.load_asset(path)
if not cube:
 cube=unreal.EditorAssetLibrary.duplicate_asset('/Engine/MapTemplates/Sky/DaylightAmbientCubemap',path)
assert cube
for a in unreal.EditorLevelLibrary.get_all_level_actors():
 if isinstance(a,(unreal.DirectionalLight,unreal.SkyLight,unreal.PointLight)):
  a.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
 if isinstance(a,unreal.SkyLight):
  a.light_component.set_editor_property('source_type',unreal.SkyLightSourceType.SLS_SPECIFIED_CUBEMAP)
  a.light_component.set_editor_property('cubemap',cube)
  a.light_component.set_editor_property('intensity',.8)
# Keep visible resource piles synchronized with the runtime.
for a in unreal.EditorLevelLibrary.get_all_level_actors():
 if a.get_actor_label().startswith('HERO_ENV_Salvage'):
  position=a.get_actor_location()
  if position.x < -100: position.x=-650
  elif position.x > 100: position.x=650
  a.set_actor_location(position,False,False)
# Keep visible resource piles synchronized with the runtime.
for a in unreal.EditorLevelLibrary.get_all_level_actors():
 if a.get_actor_label().startswith('HERO_ENV_Salvage'):
  position=a.get_actor_location()
  if position.x < -100: position.x=-650
  elif position.x > 100: position.x=650
  a.set_actor_location(position,False,False)
world=unreal.EditorLevelLibrary.get_editor_world()
world.get_world_settings().set_editor_property('force_no_precomputed_lighting',True)
assert unreal.EditorLevelLibrary.save_current_level()
unreal.EditorAssetLibrary.save_loaded_asset(cube)
unreal.log('HERO_DYNAMIC_LIGHTING_SAVED')
