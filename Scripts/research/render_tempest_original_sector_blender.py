"""Render actual exported meshes at original actor transforms; no invented floor.

Run with Blender --background --python. Original albedo/normal baseline only.
CUE glTF swaps UE Y/Z; Blender glTF import yields UE -> Blender (X,-Y,Z).
"""
import bpy
import json
import math
from mathutils import Matrix, Vector
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BASE = ROOT / "Saved/Research/TempestSectorTransfer"
SOURCE = BASE / "Export-01/Exports"
OUT = BASE / "VisualQA"
OUT.mkdir(parents=True, exist_ok=True)
selection = json.loads((BASE / "FullSector.json").read_text(encoding="utf-8"))
bindings = json.loads((BASE / "MaterialBindings-06/Bindings.json").read_text(encoding="utf-8"))
if bindings["result"] != "PASS":
    raise RuntimeError("Material bindings incomplete")
lookup = {x["source_material"]: x for x in bindings["materials"]}
lookup.update({x["source_material"].rsplit(".",1)[-1]: x for x in bindings["materials"] if x["source_material"].startswith("/Game/")})
bpy.ops.object.select_all(action="SELECT")
bpy.ops.object.delete(use_global=False)
materials = {}


def material(name):
    if name in materials:
        return materials[name]
    record = lookup[name]
    mat = bpy.data.materials.new(name + "_Original_DN_Baseline")
    mat.use_nodes = True
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    bsdf = nodes.get("Principled BSDF")
    bsdf.inputs["Roughness"].default_value = .65
    for semantic in ["albedo", "normal"]:
        path = record["resolved_textures"][semantic]["png"]
        tex = nodes.new("ShaderNodeTexImage")
        tex.image = bpy.data.images.load(path, check_existing=True)
        tex.image.colorspace_settings.name = "sRGB" if semantic == "albedo" else "Non-Color"
        if semantic == "albedo":
            links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
        else:
            normal = nodes.new("ShaderNodeNormalMap")
            # DirectX -> OpenGL normal convention is a preview adaptation;
            # source UE sampler implicitly handles signed normal decoding.
            separate = nodes.new("ShaderNodeSeparateColor")
            combine = nodes.new("ShaderNodeCombineColor")
            invert_g = nodes.new("ShaderNodeMath")
            invert_g.operation = "SUBTRACT"
            invert_g.inputs[0].default_value = 1
            links.new(tex.outputs["Color"], separate.inputs["Color"])
            links.new(separate.outputs["Green"], invert_g.inputs[1])
            links.new(separate.outputs["Red"], combine.inputs["Red"])
            links.new(invert_g.outputs[0], combine.inputs["Green"])
            links.new(separate.outputs["Blue"], combine.inputs["Blue"])
            links.new(combine.outputs["Color"], normal.inputs["Color"])
            links.new(normal.outputs["Normal"], bsdf.inputs["Normal"])
    materials[name] = mat
    return mat


def actor_matrix(record):
    rot, scale, pos = record["rotation"], record["scale"], record["relative_location"]
    pitch, yaw, roll = [math.radians(rot[x]) for x in ("pitch", "yaw", "roll")]
    cp, sp, cy, sy, cr, sr = math.cos(pitch), math.sin(pitch), math.cos(yaw), math.sin(yaw), math.cos(roll), math.sin(roll)
    rotation = Matrix(((cp*cy, sr*sp*cy-cr*sy, -(cr*sp*cy+sr*sy)),
                       (cp*sy, sr*sp*sy+cr*cy, cy*sr-cr*sp*sy),
                       (sp, -sr*cp, cr*cp)))
    handedness = Matrix.Diagonal((1, -1, 1))
    matrix = (handedness @ rotation @ Matrix.Diagonal(tuple(scale[x] for x in ("x", "y", "z"))) @ handedness).to_4x4()
    matrix.translation = Vector((pos["x"]*.01, -pos["y"]*.01, pos["z"]*.01))
    return matrix


cache = {}
for path in selection["mesh_paths"]:
    name = path.rsplit(".", 1)[-1]
    files = list(SOURCE.rglob(name + ".glb"))
    if len(files) != 1:
        raise RuntimeError("Ambiguous or missing original GLB: " + name)
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.gltf(filepath=str(files[0]))
    objects = [o for o in bpy.context.scene.objects if o not in before]
    meshes = [o for o in objects if o.type == "MESH"]
    if len(meshes) != 1:
        raise RuntimeError("Expected single original mesh: " + name)
    obj = meshes[0]
    data = obj.data.copy()
    data.transform(obj.matrix_world)
    names = [slot.material.name for slot in obj.material_slots]
    # Blender appends .001 for repeated imported material datablocks.
    names = [n if n in lookup else n.rsplit(".", 1)[0] for n in names]
    cache[path] = (data, names)
    for obj in objects:
        bpy.data.objects.remove(obj, do_unlink=True)

records, points = [], []
for index, source in enumerate(selection["actors"]):
    data, names = cache[source["mesh_source"]]
    data = data.copy()
    obj = bpy.data.objects.new(f"Original_{index:02d}_" + source["label"], data)
    bpy.context.collection.objects.link(obj)
    obj.matrix_world = actor_matrix(source)
    overrides = source["components"][0].get("materials", [])
    assigned = []
    for slot in range(len(data.materials)):
        name = overrides[slot] if slot < len(overrides) and overrides[slot] else names[slot]
        data.materials[slot] = material(name)
        assigned.append(name)
    data.calc_loop_triangles()
    points.extend(obj.matrix_world @ Vector(corner) for corner in obj.bound_box)
    records.append(dict(source=source["path"], mesh=source["mesh_source"], triangles=len(data.loop_triangles),
                        materials=assigned, matrix_blender=[list(row) for row in obj.matrix_world]))
minimum = Vector(tuple(min(p[i] for p in points) for i in range(3)))
maximum = Vector(tuple(max(p[i] for p in points) for i in range(3)))
center, extent = (minimum+maximum)*.5, max(maximum-minimum)
bpy.ops.object.camera_add(location=center + Vector((1.2, -1.5, 1.35))*extent)
camera = bpy.context.object
camera.rotation_euler = (center-camera.location).to_track_quat("-Z", "Y").to_euler()
camera.data.type = "ORTHO"
camera.data.ortho_scale = extent*1.35
scene = bpy.context.scene
scene.camera = camera
for offset, energy in [((1,-2,3), 1400), ((-2,1,2), 900)]:
    bpy.ops.object.light_add(type="AREA", location=center+Vector(offset)*extent)
    light = bpy.context.object
    light.data.energy = energy*extent*extent/20
    light.data.shape = "DISK"
    light.data.size = extent*2
    light.rotation_euler = (center-light.location).to_track_quat("-Z", "Y").to_euler()
scene.render.engine = "CYCLES"
scene.cycles.device = "CPU"
scene.cycles.samples = 24
scene.world.color = (.15,.15,.15)
scene.render.resolution_x = 1600
scene.render.resolution_y = 1100
scene.render.resolution_percentage = 100
scene.render.image_settings.file_format = "PNG"
scene.render.film_transparent = False
scene.view_settings.view_transform = "AgX"
scene.render.filepath = str(OUT / "OriginalSector.png")
bpy.ops.render.render(write_still=True)
report = dict(result="PASS", actors=len(records), unique_meshes=len(cache), materials=len(materials),
              instantiated_triangles=sum(x["triangles"] for x in records), minimum_m=list(minimum), maximum_m=list(maximum),
              dimensions_m=list(maximum-minimum), records=records, preview=scene.render.filepath,
              limitation="Original D/N baseline only; no original RMO/tint/color-mask/grunge/weather. Blender normal G inverted for preview convention, not a recovered Tempest shader operation. Actor transforms verified separately by ComponentVerification54.json. No invented ground plane.")
(OUT / "OriginalSector-Verification.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")
print("TEMPEST_SECTOR_BLENDER_PASS " + str(OUT))
