"""Actual geometry review render, independent of Unreal material integration."""
import bpy
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parent
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
for index,name in enumerate(['SM_DV3_Civic_0','SM_DV3_Civic_1','SM_DV3_Civic_2']):
    with bpy.data.libraries.load(str(ROOT/(name+'.blend')),link=False) as (src,dst):dst.objects=[name]
    obj=dst.objects[0];bpy.context.collection.objects.link(obj);obj.location.x=(index-1)*7
bpy.ops.mesh.primitive_plane_add(size=100);ground=bpy.context.object
mat=bpy.data.materials.new('Preview sand');mat.diffuse_color=(.28,.22,.15,1);ground.data.materials.append(mat)
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True
scene.world.color=(.25,.25,.25)
for pos,energy,size in [((0,-12,25),4500,15),((-15,5,18),2800,12),((15,10,25),3500,12)]:
    bpy.ops.object.light_add(type='AREA',location=pos);l=bpy.context.object;l.data.energy=energy;l.data.size=size;l.rotation_euler=(Vector((0,0,10))-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(33,-44,33));cam=bpy.context.object;cam.rotation_euler=(Vector((0,0,9))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=34;scene.camera=cam
scene.render.resolution_x=1400;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
scene.render.filepath=str(ROOT/'DistrictGeometryPreview.png');bpy.ops.render.render(write_still=True)
