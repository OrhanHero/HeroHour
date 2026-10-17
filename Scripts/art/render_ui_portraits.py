"""Render real mesh portraits for the concept UI, preserving transparent backgrounds."""
import bpy, sys
from pathlib import Path
from mathutils import Vector
root=Path(__file__).resolve().parents[2]
out=root/'Art/UI/Portraits';out.mkdir(parents=True,exist_ok=True)
names=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else ['SM_HERO_CommandHub','SM_HERO_VehiclePlant','SM_HERO_Refinery','SM_HERO_Striker','SM_HERO_Harvester']
for name in names:
    source_dir='Art/Models/Generated' if name=='SM_HERO_Salvager' else 'Art/Models/ConceptV2'
    bpy.ops.wm.open_mainfile(filepath=str(root/source_dir/(name+'.blend')))
    mesh=bpy.data.objects[name]
    scene=bpy.context.scene
    points=[mesh.matrix_world@Vector(p) for p in mesh.bound_box]
    center=sum(points,Vector())/len(points);radius=max(mesh.dimensions)
    world=bpy.data.worlds.new('Portrait studio');scene.world=world;world.use_nodes=True
    world.node_tree.nodes.clear();bg=world.node_tree.nodes.new('ShaderNodeBackground');output=world.node_tree.nodes.new('ShaderNodeOutputWorld')
    bg.inputs[0].default_value=(.24,.28,.30,1);bg.inputs[1].default_value=.6;world.node_tree.links.new(bg.outputs[0],output.inputs[0])
    for pos,power in [((radius,-radius,2*radius),1300),((-radius,-radius,radius),900),((0,radius,2*radius),1500)]:
        bpy.ops.object.light_add(type='AREA',location=pos);o=bpy.context.object;o.data.energy=power;o.data.size=radius
        o.rotation_euler=(center-o.location).to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.camera_add(location=center+Vector((radius*1.2,-radius*1.4,radius*1.05)))
    cam=bpy.context.object;cam.rotation_euler=(center-cam.location).to_track_quat('-Z','Y').to_euler()
    cam.data.type='ORTHO';cam.data.ortho_scale=radius*1.5;scene.camera=cam
    scene.render.engine='CYCLES';scene.cycles.samples=32;scene.cycles.use_denoising=True
    scene.render.film_transparent=True;scene.render.resolution_x=512;scene.render.resolution_y=512;scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA'
    scene.render.filepath=str(out/('T_HERO_'+name.removeprefix('SM_HERO_')+'.png'))
    bpy.ops.render.render(write_still=True)
    print('UI_PORTRAIT_PASS',name)
