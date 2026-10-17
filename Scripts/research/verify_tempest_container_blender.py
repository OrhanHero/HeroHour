"""Compare real exporter outputs in Blender; render an albedo-only baseline."""
import bpy
import datetime
import json
from mathutils import Vector
from pathlib import Path
import uuid

ROOT = Path(r'C:/Users/Hero/HeroHour/Saved/Research/TempestAssetTransfer')
GLB = next((ROOT / 'Container-Probe-02/Exports').rglob('SM_GDF_Container.glb'))
ALBEDO = next((ROOT / 'Container-Probe-02/Exports').rglob('T_GDF_Container_D.png'))
FBX = next(path / 'SM_GDF_Container.fbx' for path in sorted(ROOT.glob('Engine54-*'), reverse=True)
           if (path / 'SM_GDF_Container.fbx').is_file())
OUT = ROOT / 'VisualQA' / (datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + uuid.uuid4().hex[:8])
OUT.mkdir(parents=True, exist_ok=False)


def clear():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)


def inspect(path):
    clear()
    if path.suffix == '.glb':
        bpy.ops.import_scene.gltf(filepath=str(path))
    else:
        bpy.ops.import_scene.fbx(filepath=str(path))
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
    points = [obj.matrix_world @ Vector(corner) for obj in meshes for corner in obj.bound_box]
    minimum = Vector(tuple(min(p[index] for p in points) for index in range(3)))
    maximum = Vector(tuple(max(p[index] for p in points) for index in range(3)))
    records = []
    for obj in meshes:
        obj.data.calc_loop_triangles()
        records.append(dict(name=obj.name, vertices=len(obj.data.vertices),
                            triangles=len(obj.data.loop_triangles), polygons=len(obj.data.polygons),
                            uv_layers=[layer.name for layer in obj.data.uv_layers],
                            material_slots=[slot.name for slot in obj.material_slots],
                            transform_scale=list(obj.scale)))
    return dict(path=str(path), minimum=list(minimum), maximum=list(maximum),
                dimensions=list(maximum-minimum), objects=records,
                units='Blender world units, glTF meters; FBX importer conversion measured by bounds'), meshes, minimum, maximum


glb, meshes, minimum, maximum = inspect(GLB)
fbx, _, _, _ = inspect(FBX)
glb, meshes, minimum, maximum = inspect(GLB)
material = bpy.data.materials.new('Baseline_Original_Albedo_Only')
material.use_nodes = True
nodes = material.node_tree.nodes
shader = nodes.get('Principled BSDF')
shader.inputs['Roughness'].default_value = .65
shader.inputs['Metallic'].default_value = 0
texture = nodes.new('ShaderNodeTexImage')
texture.image = bpy.data.images.load(str(ALBEDO), check_existing=False)
texture.image.colorspace_settings.name = 'sRGB'
material.node_tree.links.new(texture.outputs['Color'], shader.inputs['Base Color'])
for obj in meshes:
    obj.data.materials.clear()
    obj.data.materials.append(material)
    for polygon in obj.data.polygons:
        polygon.material_index = 0
center = (minimum+maximum)*.5
extent = max(maximum-minimum)
bpy.ops.mesh.primitive_plane_add(size=extent*5, location=(center.x, center.y, minimum.z-.01))
floor = bpy.context.object
floor_mat = bpy.data.materials.new('NeutralFloor')
floor_mat.diffuse_color = (.14,.16,.18,1)
floor.data.materials.append(floor_mat)
bpy.ops.object.camera_add(location=center+Vector((1.3,-1.8,1.2))*extent)
camera = bpy.context.object
camera.rotation_euler = (center-camera.location).to_track_quat('-Z','Y').to_euler()
camera.data.type = 'ORTHO'
camera.data.ortho_scale = extent*1.45
bpy.context.scene.camera = camera
for location, energy, size in [((1,-2,3),1600,extent*2),((-2,1,2),900,extent*2)]:
    bpy.ops.object.light_add(type='AREA', location=center+Vector(location)*extent)
    light = bpy.context.object
    light.data.energy = energy*extent*extent/20
    light.data.shape = 'DISK'
    light.data.size = size
    light.rotation_euler = (center-light.location).to_track_quat('-Z','Y').to_euler()
scene = bpy.context.scene
scene.render.engine = 'CYCLES'
scene.cycles.device = 'CPU'
scene.cycles.samples = 24
scene.world.color = (.2,.2,.2)
scene.render.resolution_x = 1024
scene.render.resolution_y = 768
scene.render.resolution_percentage = 100
scene.render.image_settings.file_format = 'PNG'
scene.view_settings.view_transform = 'AgX'
scene.render.filepath = str(OUT/'Container-OriginalAlbedo-Baseline.png')
bpy.ops.render.render(write_still=True)
report = dict(glb=glb, fbx=fbx,
              bounds_dimension_ratio_glb_over_fbx=[a/b if b else None for a,b in zip(glb['dimensions'],fbx['dimensions'])],
              preview=scene.render.filepath, albedo=str(ALBEDO),
              limitation='Original albedo only; no metallic/roughness/normal/teammask or original shader parity.')
(OUT/'Container-Blender-Verification.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('TEMPEST_CONTAINER_BLENDER_PASS '+str(OUT))
