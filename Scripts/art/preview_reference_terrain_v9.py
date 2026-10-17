"""Render real V9 geometry with read-only existing terrain; no Unreal writes."""
from pathlib import Path
import bpy
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'Art/Models/ReferenceTerrainV9'
for name,origin,target,eye,scale in [
 ('SM_TRV9_RiverNorth',(-30,11),(-36,13,.2),(-48,-2,15),25),
 ('SM_TRV9_WestTalus',(-124,0),(-124,-5,4),(-103,-25,21),37)]:
 bpy.ops.wm.open_mainfile(filepath=str(OUT/(name+'.blend')))
 obj=bpy.data.objects[name];obj.location=(origin[0],-origin[1],0)
 for o in bpy.context.scene.objects:
  o.hide_viewport=False;o.hide_render=False
 # A plain water surface supplies only known map contact height, not new assets.
 if 'River' in name:
  bpy.ops.mesh.primitive_plane_add(size=1,location=(-30,-11,-1.45))
  water=bpy.context.object;water.scale=(14,38,1)
  m=bpy.data.materials.new('PreviewWater');m.use_nodes=True
  p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(.035,.15,.19,1);p.inputs['Roughness'].default_value=.25
  water.data.materials.append(m)
 scene=bpy.context.scene;scene.world=bpy.data.worlds.new('V9 Preview daylight');scene.world.use_nodes=True
 nodes=scene.world.node_tree.nodes;nodes.clear();bg=nodes.new('ShaderNodeBackground');out=nodes.new('ShaderNodeOutputWorld');scene.world.node_tree.links.new(bg.outputs[0],out.inputs[0])
 bg.inputs['Color'].default_value=(.32,.37,.43,1);bg.inputs['Strength'].default_value=.65
 bpy.ops.object.light_add(type='AREA',location=(target[0]-8,-target[1]-10,25))
 light=bpy.context.object;light.data.energy=4500;light.data.size=12
 t=Vector((target[0],-target[1],target[2]));light.rotation_euler=(t-light.location).to_track_quat('-Z','Y').to_euler()
 bpy.ops.object.camera_add(location=(eye[0],-eye[1],eye[2]));cam=bpy.context.object
 cam.rotation_euler=(t-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=scale;scene.camera=cam
 scene.render.engine='CYCLES';scene.cycles.samples=32;scene.cycles.use_denoising=True
 prefs=bpy.context.preferences.addons['cycles'].preferences;prefs.compute_device_type='OPTIX';prefs.get_devices()
 for device in prefs.devices:device.use=device.type=='OPTIX'
 scene.cycles.device='GPU'
 scene.render.resolution_x=1400;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
 scene.render.image_settings.file_format='PNG';scene.render.filepath=str(OUT/(name+'-Preview.png'))
 bpy.ops.render.render(write_still=True)
print('REFERENCE_TERRAIN_V9_PREVIEW_PASS',flush=True)
