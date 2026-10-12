import unreal
assert unreal.EditorLevelLibrary.load_level('/Game/Maps/Prototype')
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if isinstance(actor, unreal.DirectionalLight):
        actor.light_component.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
        actor.light_component.set_editor_property('intensity', 5.0)
    if isinstance(actor, unreal.SkyLight):
        actor.light_component.set_editor_property('mobility', unreal.ComponentMobility.MOVABLE)
        actor.light_component.set_editor_property('intensity', 1.0)
unreal.EditorLevelLibrary.set_level_viewport_camera_info(unreal.Vector(-1400, 0, 2000), unreal.Rotator(-55, 0, 0))
assert unreal.EditorLevelLibrary.save_current_level()
unreal.log('HEROHOUR_MAP_FIX_SAVED')
