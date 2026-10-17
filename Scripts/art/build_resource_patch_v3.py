"""Generate and round-trip an original rooted energy-resource patch in Blender.

No game geometry or textures are used. Coordinates are metres, with a ground
pivot and a maximum 5 m footprint. The Unreal importer supplies its own
Gravel/Sandstone/TempestEnergy materials. The preview is a geometry inspection,
not a screenshot of the game or a claim of visual parity with a reference.

Run: blender --background --python Scripts/art/build_resource_patch_v3.py
"""
import json
import math
import random
from collections import Counter
from pathlib import Path

import bmesh
import bpy
from mathutils import Vector


PROJECT = Path(__file__).resolve().parents[2]
OUT = PROJECT / "Art/Models/DistrictV3"
PREVIEW = PROJECT / "Saved/Research/TempestPatch-QA.png"
NAME = "SM_DV3_TempestPatch"
SEED = 982341
MATERIALS = ("Gravel", "Sandstone", "TempestEnergy")
RNG = random.Random(SEED)


def reset():
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1
    colors = ((.17, .16, .13), (.062, .054, .043), (.72, .064, .013))
    for name, color in zip(MATERIALS, colors):
        material = bpy.data.materials.get(name) or bpy.data.materials.new(name)
        material.use_nodes = True
        material.diffuse_color = (*color, 1)
        shader = material.node_tree.nodes.get("Principled BSDF")
        shader.inputs["Base Color"].default_value = (*color, 1)
        shader.inputs["Roughness"].default_value = .67 if name == "Sandstone" else .88
        shader.inputs["Metallic"].default_value = .14 if name == "Sandstone" else 0
        if name == "TempestEnergy":
            shader.inputs["Emission Color"].default_value = (*color, 1)
            shader.inputs["Emission Strength"].default_value = 3.1
            shader.inputs["Roughness"].default_value = .3
        else:
            noise = material.node_tree.nodes.new("ShaderNodeTexNoise")
            noise.inputs["Scale"].default_value = 115
            noise.inputs["Roughness"].default_value = .72
            bump = material.node_tree.nodes.new("ShaderNodeBump")
            bump.inputs["Strength"].default_value = .26
            bump.inputs["Distance"].default_value = .025
            material.node_tree.links.new(noise.outputs["Fac"], bump.inputs["Height"])
            material.node_tree.links.new(bump.outputs["Normal"], shader.inputs["Normal"])


def mesh_object(label, vertices, faces, assignments=None, smooth=False):
    mesh = bpy.data.meshes.new(label)
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(label, mesh)
    bpy.context.collection.objects.link(obj)
    for name in MATERIALS:
        mesh.materials.append(bpy.data.materials[name])
    for index, polygon in enumerate(mesh.polygons):
        polygon.material_index = assignments[index] if assignments else 1
        polygon.use_smooth = smooth
    return obj


def root(label, cx, cy, angle, length, thickness):
    """Bent tapered closed tubes, each a separate watertight mesh island."""
    vertices, faces = [], []
    side_count, sections = 7, 7
    skew = RNG.uniform(-.42, .42)
    for section in range(sections):
        t = section / (sections - 1)
        bent_angle = angle + skew * t
        radius = thickness * (1 - t * .78)
        z = .048 + thickness * .50 + math.sin(t * math.pi) * thickness * .46
        center = Vector((cx + math.cos(bent_angle) * length * t,
                         cy + math.sin(bent_angle) * length * t, z))
        horizontal = Vector((-math.sin(bent_angle), math.cos(bent_angle), 0))
        for side in range(side_count):
            a = math.tau * side / side_count
            wobble = 1 + RNG.uniform(-.10, .10)
            vertex = center + horizontal * (math.cos(a) * radius * wobble)
            vertex.z += math.sin(a) * radius * .60 * wobble
            vertices.append(tuple(vertex))
    # Caps deliberately contain center vertices: no open edges or large n-gons.
    start_center = len(vertices)
    vertices.append(tuple(sum((Vector(v) for v in vertices[:side_count]), Vector()) / side_count))
    end_center = len(vertices)
    vertices.append(tuple(sum((Vector(v) for v in vertices[-side_count-1:-1]), Vector()) / side_count))
    for side in range(side_count):
        nxt = (side + 1) % side_count
        faces.append((start_center, nxt, side))
        for section in range(sections - 1):
            a, b = section * side_count + side, section * side_count + nxt
            c, d = (section + 1) * side_count + nxt, (section + 1) * side_count + side
            faces.extend(((a, b, c), (a, c, d)))
        faces.append(((sections - 1) * side_count + side,
                      (sections - 1) * side_count + nxt, end_center))
    return mesh_object(label, vertices, faces, smooth=True)


def pod(label, cx, cy, radius, height):
    """Faceted rooted pod: thick outer rind with restrained luminous fractures."""
    broad_facets = 9
    sides = broad_facets * 2
    rotation = RNG.uniform(0, math.tau)
    profile = ((.94, .014), (1.14, .15), (.96, .47), (.82, .70),
               (.53, .84), (.25, .97))
    # The narrow face beside every broad facet is a physical fissure. It can
    # glow without painting an entire large triangle pink in a distant view.
    angles = [rotation + math.tau * i / broad_facets + offset
              for i in range(broad_facets) for offset in (0, .045)]
    lean = Vector((RNG.uniform(-.10, .10), RNG.uniform(-.10, .10), 0))
    vertices, faces, slots = [], [], []
    broad_jitter = [RNG.uniform(.82, 1.14) for _ in range(broad_facets)]
    radial_jitter = [jitter for jitter in broad_jitter for _ in (0, 1)]
    breadth = RNG.uniform(.65, 1.03)
    for ring, (width, z) in enumerate(profile):
        for side, a in enumerate(angles):
            vertices.append((cx + math.cos(a) * radius * width * radial_jitter[side] + lean.x * z,
                             cy + math.sin(a) * radius * width * radial_jitter[side] * breadth + lean.y * z,
                             .034 + height * z))
    start = len(vertices)
    vertices.append((cx, cy, .034 + height * profile[0][1]))
    top = len(vertices)
    vertices.append((cx + lean.x, cy + lean.y, .034 + height))
    lit_faces = [side * 2 for side in RNG.sample(range(broad_facets), 4)]
    for side in range(sides):
        nxt = (side + 1) % sides
        faces.append((start, nxt, side)); slots.append(1)
        for ring in range(len(profile) - 1):
            a, b = ring * sides + side, ring * sides + nxt
            c, d = (ring + 1) * sides + nxt, (ring + 1) * sides + side
            faces.extend(((a, b, c), (a, c, d)))
            # Several small fractured panels, never the whole silhouette.
            lit = side in lit_faces and ring in (1, 2, 3, 4)
            slots.extend((2 if lit else 1, 2 if lit else 1))
        faces.append(((len(profile) - 1) * sides + side,
                      (len(profile) - 1) * sides + nxt, top))
        slots.append(2 if side in lit_faces else 1)
    return mesh_object(label, vertices, faces, slots)


def gravel(label, cx, cy, radius):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=1,
                                         location=(cx, cy, radius * .20))
    obj = bpy.context.object
    obj.name = label
    obj.scale = (radius, radius * RNG.uniform(.6, 1.0), radius * .24)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.rotation_euler.z = RNG.uniform(0, math.tau)
    obj.data.materials.append(bpy.data.materials["Gravel"])
    for vertex in obj.data.vertices:
        vertex.co *= RNG.uniform(.88, 1.12)
    return obj


def metre_uv(mesh):
    layer = mesh.uv_layers.new(name="PhysicalMetres")
    for face in mesh.polygons:
        axis = max(range(3), key=lambda i: abs(face.normal[i]))
        pair = ((1, 2), (0, 2), (0, 1))[axis]
        for index in face.loop_indices:
            vertex = mesh.vertices[mesh.loops[index].vertex_index].co
            layer.data[index].uv = (vertex[pair[0]], vertex[pair[1]])


def topology(mesh):
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bm.normal_update()
    nonmanifold = sum(not edge.is_manifold for edge in bm.edges)
    visited, islands, inward = set(), 0, 0
    for vertex in bm.verts:
        if vertex in visited:
            continue
        islands += 1
        stack, component = [vertex], set()
        while stack:
            current = stack.pop()
            if current in component:
                continue
            component.add(current)
            stack.extend(edge.other_vert(current) for edge in current.link_edges)
        visited.update(component)
        faces = {face for vertex in component for face in vertex.link_faces}
        volume = sum(face.calc_area() * face.normal.dot(face.calc_center_median()) / 3 for face in faces)
        inward += volume <= 1e-9
    bm.free()
    return {"closed_islands": islands, "nonmanifold_edges": nonmanifold,
            "inward_or_zero_volume_islands": inward,
            "degenerate_faces": sum(face.area < 1e-10 for face in mesh.polygons),
            "uv_finite": bool(mesh.uv_layers) and all(math.isfinite(x)
                 for value in mesh.uv_layers.active.data for x in value.uv)}


def build():
    objects = []
    centers = [(0.10, -.10)]
    for ring, count, radius in ((1, 6, .86), (2, 10, 1.65)):
        for index in range(count):
            angle = math.tau * index / count + ring * .28 + RNG.uniform(-.06, .06)
            centers.append((math.cos(angle) * radius + RNG.uniform(-.1, .1),
                            math.sin(angle) * radius * .84 + RNG.uniform(-.08, .08)))
    for index, (cx, cy) in enumerate(centers):
        radius = RNG.uniform(.14, .23)
        height = RNG.uniform(.37, .84)
        if index == 0:
            radius, height = .26, .91
        objects.append(pod(f"Energy pod {index:02d}", cx, cy, radius, height))
        for branch in range(3):
            angle = math.tau * branch / 3 + RNG.uniform(-.65, .65)
            objects.append(root(f"Root {index:02d}:{branch}", cx, cy, angle,
                                RNG.uniform(.36, .62), RNG.uniform(.045, .076)))
    for index in range(22):
        angle, radius = RNG.uniform(0, math.tau), RNG.uniform(.35, 2.06)
        objects.append(gravel(f"Loose gravel {index:02d}", math.cos(angle) * radius,
                              math.sin(angle) * radius * .87, RNG.uniform(.032, .089)))
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    obj = bpy.context.object
    obj.name = NAME
    bpy.context.scene.cursor.location = (0, 0, 0)
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR")
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    original = [material.name for material in obj.data.materials]
    indices = [MATERIALS.index(original[face.material_index]) for face in obj.data.polygons]
    obj.data.materials.clear()
    for material in MATERIALS:
        obj.data.materials.append(bpy.data.materials[material])
    for face, index in zip(obj.data.polygons, indices):
        face.material_index = index
    # Recalculate every closed island outward before FBX triangulation/import.
    bm = bmesh.new(); bm.from_mesh(obj.data)
    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    bm.to_mesh(obj.data); bm.free()
    low = min(vertex.co.z for vertex in obj.data.vertices)
    for vertex in obj.data.vertices:
        vertex.co.z -= low
    obj.data.update()
    metre_uv(obj.data)
    return obj


def report(obj):
    world = [obj.matrix_world @ vertex.co for vertex in obj.data.vertices]
    low = [min(v[i] for v in world) for i in range(3)]
    high = [max(v[i] for v in world) for i in range(3)]
    return {"vertices": len(world), "triangles": sum(len(p.vertices) - 2 for p in obj.data.polygons),
            "dimensions_m": [high[i] - low[i] for i in range(3)],
            "bounds_min_m": low, "bounds_max_m": high,
            "materials": [m.name for m in obj.data.materials],
            "material_triangles": dict(Counter({m.name: sum(len(p.vertices) - 2
                 for p in obj.data.polygons if p.material_index == i)
                 for i, m in enumerate(obj.data.materials)})), **topology(obj.data)}


def verify(result):
    assert 5000 <= result["triangles"] <= 12000, result
    assert max(result["dimensions_m"][:2]) <= 5, result
    assert -.001 <= result["bounds_min_m"][2] <= .001, result
    assert result["bounds_max_m"][2] <= 1, result
    assert result["materials"] == list(MATERIALS), result
    assert result["closed_islands"] == 90, result
    assert result["nonmanifold_edges"] == 0, result
    assert result["inward_or_zero_volume_islands"] == 0, result
    assert result["degenerate_faces"] == 0 and result["uv_finite"], result


def render(obj):
    bpy.ops.mesh.primitive_plane_add(size=200, location=(0, 0, -.014))
    plane = bpy.context.object
    floor = bpy.data.materials.new("Preview ground only")
    floor.use_nodes = True
    shader = floor.node_tree.nodes.get("Principled BSDF")
    shader.inputs["Base Color"].default_value = (.115, .126, .102, 1)
    shader.inputs["Roughness"].default_value = .93
    plane.data.materials.append(floor)
    bpy.ops.object.camera_add(location=(5.0, -6.9, 5.4))
    camera = bpy.context.object
    camera.rotation_euler = (Vector((0, 0, .18)) - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = 5.70
    bpy.context.scene.camera = camera
    for position, power, size, color in (((-3, -4, 8), 1500, 6, (1, .91, .82)),
                                        ((3, 2, 5), 900, 5, (.68, .78, 1))):
        bpy.ops.object.light_add(type="AREA", location=position)
        light = bpy.context.object
        light.data.energy, light.data.size, light.data.color = power, size, color
        light.rotation_euler = (-light.location).to_track_quat("-Z", "Y").to_euler()
    scene = bpy.context.scene
    scene.world.color = (.10, .10, .10)
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 48
    scene.cycles.use_denoising = True
    scene.render.resolution_x, scene.render.resolution_y = 1400, 1050
    scene.render.resolution_percentage = 100
    scene.view_settings.view_transform = "AgX"
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(PREVIEW)
    PREVIEW.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.render.render(write_still=True)


reset()
asset = build()
source = report(asset)
verify(source)
OUT.mkdir(parents=True, exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (NAME + ".blend")))
bpy.ops.export_scene.fbx(filepath=str(OUT / (NAME + ".fbx")), use_selection=True,
                         object_types={"MESH"}, axis_forward="X", axis_up="Z",
                         global_scale=1, apply_unit_scale=False,
                         add_leaf_bones=False, mesh_smooth_type="FACE")
bpy.ops.object.select_all(action="SELECT"); bpy.ops.object.delete(use_global=False)
# Remove existing materials so imported FBX slot names are verified without .001 suffixes.
for material in list(bpy.data.materials):
    bpy.data.materials.remove(material)
bpy.ops.import_scene.fbx(filepath=str(OUT / (NAME + ".fbx")))
imported = next(obj for obj in bpy.context.scene.objects if obj.type == "MESH")
roundtrip = report(imported)
verify(roundtrip)
for key in ("triangles", "closed_islands", "materials"):
    assert source[key] == roundtrip[key], (key, source[key], roundtrip[key])
assert max(abs(a-b) for a, b in zip(source["dimensions_m"], roundtrip["dimensions_m"])) < 1e-5
result = {"name": NAME, "seed": SEED, "cluster_count": 17,
          "provenance": "Original procedural geometry; no third-party game assets",
          "pivot": "ground origin", "uv_mapping": "dominant-plane physical metres",
          "preview": str(PREVIEW), "validation": "PASS", **source,
          "fbx_roundtrip": roundtrip}
(OUT / (NAME + ".json")).write_text(json.dumps(result, indent=2), encoding="utf-8")
# Reset only shader graphs; leave the verified imported geometry for rendering.
colors = ((.17, .16, .13), (.062, .054, .043), (.72, .064, .013))
for name, color in zip(MATERIALS, colors):
    material = bpy.data.materials[name]
    material.use_nodes = True
    shader = material.node_tree.nodes.get("Principled BSDF")
    shader.inputs["Base Color"].default_value = (*color, 1)
    shader.inputs["Roughness"].default_value = .67 if name == "Sandstone" else .88
    if name == "TempestEnergy":
        shader.inputs["Emission Color"].default_value = (*color, 1)
        shader.inputs["Emission Strength"].default_value = 3.1
        shader.inputs["Roughness"].default_value = .3
    else:
        noise = material.node_tree.nodes.new("ShaderNodeTexNoise")
        noise.inputs["Scale"].default_value = 115
        noise.inputs["Roughness"].default_value = .72
        bump = material.node_tree.nodes.new("ShaderNodeBump")
        bump.inputs["Strength"].default_value = .30
        bump.inputs["Distance"].default_value = .025
        material.node_tree.links.new(noise.outputs["Fac"], bump.inputs["Height"])
        material.node_tree.links.new(bump.outputs["Normal"], shader.inputs["Normal"])
print("TEMPEST_PATCH_VALIDATION_PASS", json.dumps(result))
render(imported)
