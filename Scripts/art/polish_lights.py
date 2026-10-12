import unreal
assert unreal.EditorLevelLibrary.load_level('/Game/Maps/Prototype')
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
 if isinstance(actor,unreal.PointLight):
  actor.light_component.set_editor_property('light_color',unreal.Color(r=255,g=188,b=126,a=255))
  actor.light_component.set_editor_property('intensity',850.0)
 if isinstance(actor,unreal.DirectionalLight):
  actor.light_component.set_editor_property('light_color',unreal.Color(r=226,g=231,b=240,a=255))
  actor.light_component.set_editor_property('intensity',9.0)
 if isinstance(actor,unreal.SkyLight):
  actor.light_component.set_editor_property('intensity',1.0)
# Explicitly document imported mesh/material identities and avoid ambiguity.
mesh=unreal.load_asset('/Game/HERO/Meshes/SM_HERO_Technical')
for slot in mesh.get_editor_property('static_materials'):
 unreal.log('HERO_SLOT '+str(slot.get_editor_property('material_slot_name'))+' '+str(slot.get_editor_property('material_interface')))
assert unreal.EditorLevelLibrary.save_current_level()
unreal.log('HERO_LIGHT_POLISH_SAVED')
