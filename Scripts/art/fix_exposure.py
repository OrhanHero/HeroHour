import unreal
assert unreal.EditorLevelLibrary.load_level('/Game/Maps/Prototype')
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
 if isinstance(actor,unreal.PostProcessVolume):
  s=actor.get_editor_property('settings')
  s.set_editor_property('override_auto_exposure_apply_physical_camera_exposure',True)
  s.set_editor_property('auto_exposure_apply_physical_camera_exposure',False)
  s.set_editor_property('auto_exposure_bias',0.0)
  actor.set_editor_property('settings',s)
 if isinstance(actor,unreal.PointLight):
  actor.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
 if isinstance(actor,unreal.DirectionalLight):
  actor.light_component.set_editor_property('mobility',unreal.ComponentMobility.MOVABLE)
  actor.light_component.set_editor_property('intensity',6.0)
assert unreal.EditorLevelLibrary.save_current_level()
unreal.log('HERO_EXPOSURE_FIX_SAVED')
