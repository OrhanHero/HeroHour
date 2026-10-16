"""Read-only Cycles inspection of the actual TRELLIS factory candidate GLB.

Never changes or exports source geometry/materials. Outputs only real studio
renders and machine-readable evidence to stdout for the process log.
"""
import hashlib
import json
import os
import time
from pathlib import Path
import bpy
from mathutils import Vector

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'Art/Models/TrellisFactoryV9-Candidate'
SOURCE=OUT/json.loads((OUT/'verification.json').read_text(encoding='utf-8'))['glb']
EXPECTED=next(x['sha256'] for x in json.loads((ROOT/'Saved/Research/TrellisFactoryV9/status.json').read_text(encoding='utf-8'))['outputs'] if x['file'].endswith(SOURCE.name))
digest=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
before=digest(SOURCE);assert before==EXPECTED
started=time.monotonic()
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.gltf(filepath=str(SOURCE))
meshes=[obj for obj in bpy.context.scene.objects if obj.type=='MESH']
assert meshes
bpy.context.view_layer.update()
points=[obj.matrix_world@Vector(p) for obj in meshes for p in obj.bound_box]
low=Vector([min(p[i] for p in points) for i in range(3)])
high=Vector([max(p[i] for p in points) for i in range(3)])
dimensions=high-low;center=(high+low)*.5;radius=max(dimensions)
assert min(dimensions)>radius*.1,'Flat/degenerate geometry'
geometry=[{'object':obj.name,'vertices':len(obj.data.vertices),
           'triangles':sum(len(p.vertices)-2 for p in obj.data.polygons),
           'uv_layers':[uv.name for uv in obj.data.uv_layers],
           'computed_face_normal_directions':len({tuple(round(c,3) for c in p.normal) for p in obj.data.polygons})}
          for obj in meshes]
materials=[]
for material in {slot.material for obj in meshes for slot in obj.material_slots if slot.material}:
    materials.append({'name':material.name,'node_types':[n.bl_idname for n in material.node_tree.nodes],
                      'textures':[{'name':n.image.name,'size':list(n.image.size),
                                   'colorspace':n.image.colorspace_settings.name}
                                  for n in material.node_tree.nodes if n.bl_idname=='ShaderNodeTexImage' and n.image]})

scene=bpy.context.scene;scene.render.engine='CYCLES'
scene.cycles.samples=48;scene.cycles.use_denoising=True
scene.render.threads_mode='FIXED';scene.render.threads=8
backend='CPU'
preferences=bpy.context.preferences.addons['cycles'].preferences
for candidate in ('OPTIX','CUDA','HIP','ONEAPI'):
    try:
        preferences.compute_device_type=candidate;preferences.get_devices()
        if any(d.type==candidate for d in preferences.devices):
            for device in preferences.devices:device.use=device.type==candidate
            scene.cycles.device='GPU';backend=candidate;break
    except (TypeError,RuntimeError):pass
if backend=='CPU':scene.cycles.device='CPU'
scene.render.resolution_x=1200;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGB'
scene.render.film_transparent=False
scene.view_settings.view_transform='AgX';scene.view_settings.exposure=0;scene.view_settings.gamma=1
scene.view_settings.look='None'
world=bpy.data.worlds.new('Neutral inspection');world.use_nodes=True;scene.world=world
background=world.node_tree.nodes.new('ShaderNodeBackground');output=world.node_tree.nodes.new('ShaderNodeOutputWorld')
background.inputs[0].default_value=(.20,.20,.20,1);background.inputs[1].default_value=.5
world.node_tree.links.new(background.outputs[0],output.inputs[0])
floor_mat=bpy.data.materials.new('Neutral floor');floor_mat.use_nodes=True
floor_shader=floor_mat.node_tree.nodes.get('Principled BSDF')
if not floor_shader:
    floor_shader=floor_mat.node_tree.nodes.new('ShaderNodeBsdfPrincipled')
    floor_output=floor_mat.node_tree.nodes.new('ShaderNodeOutputMaterial')
    floor_mat.node_tree.links.new(floor_shader.outputs[0],floor_output.inputs[0])
floor_shader.inputs['Base Color'].default_value=(.18,.18,.18,1)
floor_shader.inputs['Roughness'].default_value=.90
bpy.ops.mesh.primitive_plane_add(size=radius*200,location=(center.x,center.y,low.z-radius*.01))
floor=bpy.context.object;floor.name='Inspection floor only';floor.data.materials.append(floor_mat)
for offset,power in [((1,-1.8,2),105),((-1.7,-.2,1.5),80),((.5,1.7,1.9),120)]:
    bpy.ops.object.light_add(type='AREA',location=center+Vector(offset)*radius)
    lamp=bpy.context.object;lamp.data.energy=power*radius*radius
    lamp.data.shape='DISK';lamp.data.size=radius*1.4
    lamp.rotation_euler=(center-lamp.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=center+Vector((1.7,-2,1.25))*radius)
camera=bpy.context.object;camera.data.type='ORTHO';scene.camera=camera

wire=bpy.data.materials.new('Inspection wire override');wire.use_nodes=True
nodes=wire.node_tree.nodes;nodes.clear();links=wire.node_tree.links
wire_node=nodes.new('ShaderNodeWireframe');wire_node.use_pixel_size=True;wire_node.inputs['Size'].default_value=.65
gray=nodes.new('ShaderNodeBsdfDiffuse');gray.inputs['Color'].default_value=(.42,.44,.45,1)
line=nodes.new('ShaderNodeEmission');line.inputs['Color'].default_value=(.003,.004,.005,1)
mix=nodes.new('ShaderNodeMixShader');wire_output=nodes.new('ShaderNodeOutputMaterial')
links.new(wire_node.outputs[0],mix.inputs[0]);links.new(gray.outputs[0],mix.inputs[1]);links.new(line.outputs[0],mix.inputs[2]);links.new(mix.outputs[0],wire_output.inputs[0])
views=[('Preview-PBR-A.png',(1.7,-2,1.25),False),
       ('Preview-PBR-B.png',(-1.7,2,1.25),False),
       ('Preview-PBR-C.png',(-1.7,-2,1.25),False),
       ('Preview-Wire-A.png',(1.7,-2,1.25),True),
       ('Preview-Wire-B.png',(-1.7,2,1.25),True)]
records=[]
for filename,offset,is_wire in views:
    camera.location=center+Vector(offset)*radius
    camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    bpy.context.view_layer.update();inverse=camera.matrix_world.inverted()
    projected=[inverse@p for p in points]
    xextent=max(p.x for p in projected)-min(p.x for p in projected)
    yextent=max(p.y for p in projected)-min(p.y for p in projected)
    camera.data.ortho_scale=max(xextent,yextent*1.2)*1.16
    scene.view_layers[0].material_override=wire if is_wire else None
    scene.render.filepath=str(OUT/filename)
    bpy.ops.render.render(write_still=True)
    records.append({'file':filename,'camera_offset':offset,'material_override':is_wire,'sha256':digest(OUT/filename)})
assert digest(SOURCE)==before,'Source GLB changed'
assert geometry==[{'object':obj.name,'vertices':len(obj.data.vertices),
                  'triangles':sum(len(p.vertices)-2 for p in obj.data.polygons),
                  'uv_layers':[uv.name for uv in obj.data.uv_layers],
                  'computed_face_normal_directions':len({tuple(round(c,3) for c in p.normal) for p in obj.data.polygons})}
                 for obj in meshes],'Inspection changed geometry'
print('TRELLIS_FACTORY_V9_INSPECTION_PASS '+json.dumps({'pid':os.getpid(),'source':str(SOURCE.relative_to(ROOT)),
      'source_sha256':before,'source_unchanged':True,'geometry_unchanged':True,'dimensions_blender_units':list(dimensions),
      'geometry':geometry,'imported_pbr_materials':materials,'backend':backend,'views':records,
      'elapsed_seconds':round(time.monotonic()-started,2)}),flush=True)
