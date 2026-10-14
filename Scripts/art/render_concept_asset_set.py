"""Render the actual exported asset pack together under warm desert lighting."""
import bpy, math, random
from pathlib import Path
from mathutils import Vector
root=Path(__file__).resolve().parents[2]
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
layout=[('SM_HERO_CommandHub',(-3.9,2.6,1.1)),('SM_HERO_VehiclePlant',(3.5,2.6,.35)),
        ('SM_HERO_Refinery',(-4.0,-3.7,.35)),('SM_HERO_Striker',(3.6,-3.6,.35))]
for name,pos in layout:
    with bpy.data.libraries.load(str(root/'Art/Models/ConceptV2'/(name+'.blend')),link=False) as (source,dest):
        dest.objects=[name]
    for obj in dest.objects:
        bpy.context.collection.objects.link(obj);obj.location=pos

sand=bpy.data.materials.new('Showcase desert sand');sand.use_nodes=True
p=sand.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(.31,.215,.12,1);p.inputs['Roughness'].default_value=.95
n=sand.node_tree.nodes.new('ShaderNodeTexNoise');n.inputs['Scale'].default_value=55;n.inputs['Detail'].default_value=3
b=sand.node_tree.nodes.new('ShaderNodeBump');b.inputs['Strength'].default_value=.2;b.inputs['Distance'].default_value=.045
sand.node_tree.links.new(n.outputs['Fac'],b.inputs['Height']);sand.node_tree.links.new(b.outputs['Normal'],p.inputs['Normal'])
bpy.ops.mesh.primitive_plane_add(size=200);ground=bpy.context.object;ground.name='Presentation desert ground';ground.data.materials.append(sand)
random.seed(41)
for i in range(32):
    angle=random.random()*math.tau;radius=random.uniform(8,11)
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1,radius=1,location=(math.cos(angle)*radius,math.sin(angle)*radius,.06))
    o=bpy.context.object;o.scale=(random.uniform(.10,.40),random.uniform(.12,.35),random.uniform(.10,.25));o.data.materials.append(sand)
scene=bpy.context.scene;world=bpy.data.worlds.new('Desert atmosphere');scene.world=world;world.use_nodes=True
world.node_tree.nodes.clear();background=world.node_tree.nodes.new('ShaderNodeBackground');world_output=world.node_tree.nodes.new('ShaderNodeOutputWorld')
background.inputs[0].default_value=(.17,.22,.28,1);background.inputs[1].default_value=.4
world.node_tree.links.new(background.outputs[0],world_output.inputs[0])
bpy.ops.object.light_add(type='SUN',location=(0,0,12));sun=bpy.context.object;sun.rotation_euler=(math.radians(25),math.radians(-22),math.radians(-30))
sun.data.energy=2.0;sun.data.angle=math.radians(12);sun.data.color=(1,.85,.68)
bpy.ops.object.light_add(type='AREA',location=(-7,-5,11));fill=bpy.context.object;fill.data.energy=900;fill.data.size=12
fill.rotation_euler=(Vector((0,0,1))-fill.location).to_track_quat('-Z','Y').to_euler();fill.data.color=(.63,.77,1)
bpy.ops.object.camera_add(location=(14,-19,17));cam=bpy.context.object
cam.rotation_euler=(Vector((0,0,1.7))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=19.2;scene.camera=cam
scene.render.engine='CYCLES';scene.cycles.samples=64;scene.cycles.use_denoising=True
scene.render.resolution_x=1800;scene.render.resolution_y=1300;scene.render.resolution_percentage=100
scene.view_settings.exposure=-.45
scene.render.image_settings.file_format='PNG';scene.render.filepath=str(root/'Art/Previews/ConceptV2/AssetSet.png')
bpy.ops.wm.save_as_mainfile(filepath=str(root/'Art/Models/ConceptV2/ConceptAssetSet.blend'))
bpy.ops.render.render(write_still=True)
print('ACTUAL_ASSET_SET_RENDER_PASS')
