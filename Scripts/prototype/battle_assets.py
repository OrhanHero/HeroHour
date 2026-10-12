import unreal

tools = unreal.AssetToolsHelpers.get_asset_tools()
path = '/Game/Prototype/M_Team'
material = unreal.load_asset(path)
if not material:
    material = tools.create_asset('M_Team', '/Game/Prototype', unreal.Material, unreal.MaterialFactoryNew())
    tint = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -400, 0)
    tint.set_editor_property('parameter_name', 'Tint')
    tint.set_editor_property('default_value', unreal.LinearColor(0.1, 0.4, 0.8, 1))
    unreal.MaterialEditingLibrary.connect_material_property(tint, '', unreal.MaterialProperty.MP_BASE_COLOR)
    roughness = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant, -400, 200)
    roughness.set_editor_property('r', 0.8)
    unreal.MaterialEditingLibrary.connect_material_property(roughness, '', unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
assert unreal.EditorLevelLibrary.load_level('/Game/Maps/Prototype')
actors = unreal.EditorLevelLibrary.get_all_level_actors()
for actor in actors:
    if actor.get_actor_label() == 'Prototype Ground':
        instance = unreal.load_asset('/Game/Prototype/MI_Ground')
        if not instance:
            instance = tools.create_asset('MI_Ground', '/Game/Prototype', unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, material)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(instance, 'Tint', unreal.LinearColor(0.07, 0.13, 0.1, 1))
        unreal.EditorAssetLibrary.save_loaded_asset(instance)
        actor.static_mesh_component.set_material(0, instance)
start = next(a for a in actors if isinstance(a, unreal.PlayerStart))
start.set_actor_location(unreal.Vector(-1700, 0, 2300), False, False)
unreal.EditorLevelLibrary.set_level_viewport_camera_info(unreal.Vector(-2600, 0, 3400), unreal.Rotator(-65, 0, 0))
assert unreal.EditorLevelLibrary.save_current_level()
unreal.log('HEROHOUR_BATTLE_ASSETS_READY')
