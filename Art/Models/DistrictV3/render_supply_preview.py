import bpy
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parent
bpy.ops.wm.open_mainfile(filepath=str(ROOT/'SM_DV3_SupplyCache.blend'))
bpy.ops.mesh.primitive_plane_add(size=30);g=bpy.context.object;m=bpy.data.materials.new('Preview floor');m.diffuse_color=(.25,.22,.18,1);g.data.materials.append(m)
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True;scene.world.color=(.3,.3,.3)
for pos,power,size in [((2,-4,7),800,5),((-4,2,5),600,4)]:
    bpy.ops.object.light_add(type='AREA',location=pos);l=bpy.context.object;l.data.energy=power;l.data.size=size;l.rotation_euler=(Vector((0,0,1))-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(5,-7,6));cam=bpy.context.object;cam.rotation_euler=(Vector((0,0,.9))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=5.5;scene.camera=cam
scene.render.resolution_x=1000;scene.render.resolution_y=800;scene.render.resolution_percentage=100;scene.render.filepath=str(ROOT/'SupplyCacheGeometryPreview.png');bpy.ops.render.render(write_still=True)
