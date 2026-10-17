"""Neutral same-camera RAW/decimated comparison. Source GLBs never modified."""
import bpy,hashlib,json,time
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'Art/Models/TrellisV8-Diagnostic'
started=time.monotonic();records=[];reference=None
for branch in ['RAW','DECIMATED','UNWRAPPED','REMESHED']:
 source=OUT/(branch+'_00001_.glb');before=hashlib.sha256(source.read_bytes()).hexdigest()
 bpy.ops.wm.read_factory_settings(use_empty=True)
 bpy.ops.import_scene.gltf(filepath=str(source))
 objects=[o for o in bpy.context.scene.objects if o.type=='MESH'];bpy.context.view_layer.update()
 points=[o.matrix_world@Vector(p) for o in objects for p in o.bound_box]
 low=Vector([min(p[i] for p in points) for i in range(3)]);high=Vector([max(p[i] for p in points) for i in range(3)])
 if reference is None:reference=((high+low)/2,max(high-low),low.z)
 center,radius,floorZ=reference
 counts=[(len(o.data.vertices),len(o.data.polygons)) for o in objects]
 mat=bpy.data.materials.new('Neutral gray diagnostic');mat.use_nodes=True
 nodes=mat.node_tree.nodes;nodes.clear();s=nodes.new('ShaderNodeBsdfDiffuse');s.inputs['Color'].default_value=(.45,.45,.45,1)
 output=nodes.new('ShaderNodeOutputMaterial');mat.node_tree.links.new(s.outputs[0],output.inputs[0])
 for obj in objects:obj.data.materials.clear();obj.data.materials.append(mat)
 scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True
 prefs=bpy.context.preferences.addons['cycles'].preferences;prefs.compute_device_type='OPTIX';prefs.get_devices()
 for d in prefs.devices:d.use=d.type=='OPTIX'
 scene.cycles.device='GPU';scene.render.resolution_x=1000;scene.render.resolution_y=850;scene.render.resolution_percentage=100
 scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGB';scene.view_settings.view_transform='AgX'
 world=bpy.data.worlds.new('World');world.use_nodes=True;scene.world=world;world.node_tree.nodes['Background'].inputs[0].default_value=(.2,.2,.2,1)
 world.node_tree.nodes['Background'].inputs[1].default_value=.5
 bpy.ops.mesh.primitive_plane_add(size=100,location=(center.x,center.y,floorZ-.01));bpy.context.object.data.materials.append(mat)
 for offset,power in [((1,-1.8,2),105),((-1.7,-.2,1.5),80),((.5,1.7,1.9),120)]:
  bpy.ops.object.light_add(type='AREA',location=center+Vector(offset)*radius);lamp=bpy.context.object
  lamp.data.energy=power*radius*radius;lamp.data.size=radius*1.4;lamp.rotation_euler=(center-lamp.location).to_track_quat('-Z','Y').to_euler()
 bpy.ops.object.camera_add();camera=bpy.context.object;camera.data.type='ORTHO';camera.data.ortho_scale=1.6*radius;scene.camera=camera
 for label,offset in [('Front',(1.7,-2,1.25)),('Back',(-1.7,2,1.25)),('Side',(-1.7,-2,1.25))]:
  camera.location=center+Vector(offset)*radius;camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
  path=OUT/f'Preview-{branch}-{label}.png';scene.render.filepath=str(path);bpy.ops.render.render(write_still=True)
  records.append({'branch':branch,'view':label,'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'source_sha256':before,'camera_offset':offset,'neutral_material':True})
 assert counts==[(len(o.data.vertices),len(o.data.polygons)) for o in objects]
 assert hashlib.sha256(source.read_bytes()).hexdigest()==before
report={'records':records,'sources_and_imported_geometry_unchanged':True,'same_cameras_from_RAW_bounds':True,'elapsed_seconds':time.monotonic()-started}
(ROOT/'docs/verification/2026-10-16/reference-v8/trellis/neutral-render-evidence.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print('TRELLIS_DIAGNOSTIC_RENDER_PASS',json.dumps(report),flush=True)
