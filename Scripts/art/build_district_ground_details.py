"""Original low-poly desert pebbles; metre-based UVs and no borrowed assets.

Run with Blender --background --python this_file.py. The joined FBX keeps
discrete stone islands, a ground pivot and the existing DistrictV3 material
slot names. It is intended as sparse decorative ground cover, without collision.
"""
import json
import math
import random
from pathlib import Path

import bpy
from mathutils import Vector


OUT = Path(__file__).resolve().parents[2] / "Art/Models/DistrictV3"
NAME = "SM_DV3_PebbleCluster"
SEED = 43182
MATERIALS = {
    "Gravel": (0.27, 0.26, 0.22),
    "Sandstone": (0.38, 0.34, 0.27),
}


def reset():
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system = "METRIC"
    bpy.context.scene.unit_settings.scale_length = 1
    for name, color in MATERIALS.items():
        mat = bpy.data.materials.get(name) or bpy.data.materials.new(name)
        mat.use_nodes = True
        mat.diffuse_color = (*color, 1)
        shader = mat.node_tree.nodes.get("Principled BSDF")
        shader.inputs["Base Color"].default_value = (*color, 1)
        shader.inputs["Roughness"].default_value = 0.9
        shader.inputs["Metallic"].default_value = 0


def rock(rng, index, center, diameter, material):
    """Four irregular perimeter rings give convex weathered silhouettes."""
    sides = rng.choice([7, 8, 9])
    radius_x = diameter * 0.5
    radius_y = radius_x * rng.uniform(0.65, 0.96)
    height = diameter * rng.uniform(0.29, 0.58)
    theta = rng.uniform(0, math.tau)
    angles = [theta + k * math.tau / sides + rng.uniform(-0.065, 0.065)
              for k in range(sides)]
    radii = [rng.uniform(0.82, 1.10) for _ in range(sides)]
    lean_x, lean_y = rng.uniform(-0.1, 0.1), rng.uniform(-0.12, 0.12)
    vertices = []
    # A flat contact patch sits at z=0; the shoulder and crown taper differently.
    for ring, (radius, z) in enumerate([(0.59, 0), (1.0, 0.22),
                                      (0.80, 0.72), (0.26, 0.94)]):
        for k, angle in enumerate(angles):
            jitter = 1 if ring == 0 else rng.uniform(0.90, 1.08)
            z_jitter = 0 if ring == 0 else rng.uniform(-0.055, 0.055)
            vertices.append((math.cos(angle) * radius_x * radius * radii[k] * jitter + lean_x * height * z,
                             math.sin(angle) * radius_y * radius * radii[k] * jitter + lean_y * height * z,
                             height * (z + z_jitter)))
    bottom = len(vertices)
    vertices.append((0, 0, 0))
    top = len(vertices)
    vertices.append((lean_x * height, lean_y * height, height))
    faces = []
    for k in range(sides):
        nxt = (k + 1) % sides
        faces.append((bottom, nxt, k))
        for ring in range(3):
            a, b = ring * sides + k, ring * sides + nxt
            c, d = (ring + 1) * sides + nxt, (ring + 1) * sides + k
            faces.append((a, b, c, d))
        faces.append((3 * sides + k, 3 * sides + nxt, top))
    mesh = bpy.data.meshes.new(f"Weathered pebble {index:02d}")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(f"Pebble {index:02d}", mesh)
    bpy.context.collection.objects.link(obj)
    obj.location = (*center, 0)
    obj.data.materials.append(bpy.data.materials[material])
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bevel = obj.modifiers.new("Eroded stone edges", "BEVEL")
    bevel.width = min(0.0025, diameter * 0.019)
    bevel.segments = 1
    bevel.limit_method = "ANGLE"
    bevel.angle_limit = math.radians(45)
    bevel.affect = "EDGES"
    bpy.ops.object.modifier_apply(modifier=bevel.name)
    # Preserve broad irregular facets, with actual bevel facets between them.
    for poly in obj.data.polygons:
        poly.use_smooth = False
    obj.select_set(False)
    return obj


def metre_uv(mesh):
    """Project each facet onto its dominant plane; 1 UV unit equals 1 metre."""
    layer = mesh.uv_layers.new(name="PhysicalMetres")
    for face in mesh.polygons:
        normal = face.normal
        axis = max(range(3), key=lambda j: abs(normal[j]))
        coordinates = ((1, 2), (0, 2), (0, 1))[axis]
        for loop_index in face.loop_indices:
            vertex = mesh.vertices[mesh.loops[loop_index].vertex_index].co
            layer.data[loop_index].uv = (vertex[coordinates[0]], vertex[coordinates[1]])


def export(objects):
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
    old_names = [mat.name for mat in obj.data.materials]
    names = list(MATERIALS)
    indices = [names.index(old_names[face.material_index]) for face in obj.data.polygons]
    obj.data.materials.clear()
    for name in names:
        obj.data.materials.append(bpy.data.materials[name])
    for face, index in zip(obj.data.polygons, indices):
        face.material_index = index
    # Bevel arithmetic can put contact vertices a few nanometres below z=0.
    floor_z = min(vertex.co.z for vertex in obj.data.vertices)
    for vertex in obj.data.vertices:
        vertex.co.z -= floor_z
    obj.data.update()
    metre_uv(obj.data)
    bounds_min = [min(vertex.co[j] for vertex in obj.data.vertices) for j in range(3)]
    bounds_max = [max(vertex.co[j] for vertex in obj.data.vertices) for j in range(3)]
    triangles = sum(len(face.vertices) - 2 for face in obj.data.polygons)
    uv_finite = all(math.isfinite(component) for uv in obj.data.uv_layers.active.data
                    for component in uv.uv)
    degenerate = [face.index for face in obj.data.polygons if face.area < 1e-10]
    assert 1000 <= triangles <= 3000, triangles
    assert obj.dimensions.x <= 1.7 and obj.dimensions.y <= 1.2, list(obj.dimensions)
    assert 0 <= bounds_min[2] <= 0.001 and bounds_max[2] <= 0.20, bounds_max
    assert uv_finite and not degenerate, degenerate
    OUT.mkdir(parents=True, exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (NAME + ".blend")))
    bpy.ops.export_scene.fbx(filepath=str(OUT / (NAME + ".fbx")), use_selection=True,
                            object_types={"MESH"}, axis_forward="X", axis_up="Z",
                            global_scale=1, apply_unit_scale=False, add_leaf_bones=False,
                            mesh_smooth_type="FACE")
    report = {
        "name": NAME, "seed": SEED, "stone_count": len(objects),
        "vertices": len(obj.data.vertices), "triangles": triangles,
        "dimensions_m": list(obj.dimensions), "bounds_min_m": bounds_min,
        "bounds_max_m": bounds_max, "materials": names,
        "pivot": "ground origin", "collision": "none; decorative",
        "uv_mapping": "dominant-plane physical metres; 1 UV unit equals 1 metre",
        "uv_finite": uv_finite, "degenerate_faces": len(degenerate),
        "validation": "PASS",
    }
    (OUT / (NAME + ".json")).write_text(json.dumps(report, indent=2), encoding="utf-8")
    print("GROUND_DETAILS_EXPORTED", json.dumps(report))


reset()
rng = random.Random(SEED)
placements = []
# Asymmetric drifting patch: three loose pockets and a few scattered outliers.
pockets = [(-0.35, 0.08, 0.26), (0.14, -0.12, 0.25), (0.38, 0.23, 0.14)]
for index in range(21):
    diameter = rng.uniform(0.052, 0.176)
    for attempt in range(150):
        if index < 17:
            cx, cy, spread = pockets[index % len(pockets)]
            angle, radius = rng.uniform(0, math.tau), spread * math.sqrt(rng.random())
            center = (cx + math.cos(angle) * radius, cy + math.sin(angle) * radius * 0.72)
        else:
            center = (rng.uniform(-0.66, 0.65), rng.uniform(-0.40, 0.40))
        if all(math.dist(center, previous) > (diameter + previous_diameter) * 0.43 + 0.018
               for previous, previous_diameter in placements):
            break
    else:
        raise RuntimeError("Could not distribute nonoverlapping stone islands")
    placements.append((center, diameter))

# Spread the same 5–18 cm stones over a roughly 1.4 x 1 m footprint.
placements = [((center[0] * 1.15, center[1] * 1.25), diameter)
              for center, diameter in placements]

objects = [rock(rng, index, center, diameter,
                "Sandstone" if rng.random() < 0.32 else "Gravel")
           for index, (center, diameter) in enumerate(placements)]
export(objects)
