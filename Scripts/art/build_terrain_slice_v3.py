"""Build an original eroded RTS valley in metres, preserving current gameplay heights.

Run with Blender 5.2:
  blender --background --python Scripts/art/build_terrain_slice_v3.py

No commercial game geometry is used. The interior is intentionally level because
the prototype currently moves units at a fixed Z. Geology outside that interior
frames the playable valley; the river remains compatible with its current bridges.
"""
from pathlib import Path
import json
import math
import bpy
from mathutils import Vector, noise

PROJECT = Path(__file__).resolve().parents[2]
OUT = PROJECT / 'Art/Models/DistrictV3'
PREVIEW = PROJECT / 'Art/Previews'
NAME = 'SM_DV3_Terrain'
SEED = 314159
GROUND_Z = -.05
RIVER_Z = -3.5
STEP = 1.25


def smoothstep(a, b, value):
    t = max(0.0, min(1.0, (value - a) / (b - a)))
    return t * t * (3.0 - 2.0 * t)


def field(x, y, scale, offset=0):
    """Fixed coordinates, so no dependency on Blender's random state."""
    return noise.noise(Vector((x * scale + 28.351 + offset,
                               y * scale - 76.163 + offset * .37,
                               12.193 + SEED * .000013)),
                       noise_basis='PERLIN_ORIGINAL')


# Distinct, deliberately asymmetric sandstone outcrops. Lengths/radii are metres;
# all centres are outside the playable rectangle. Gaps form natural side valleys.
RIDGES = [
    (-147, -40, 15, 73, -.045, 27),
    (-148, 77, 19, 34, .20, 17),
    (-110, 116, 41, 17, -.16, 23),
    (-57, 119, 27, 14, .08, 15),
    (10, 117, 43, 16, -.08, 29),
    (94, 118, 35, 17, .10, 21),
    (148, 43, 17, 62, .09, 25),
    (145, -48, 13, 24, -.22, 15),
    (137, -95, 21, 26, -.45, 20),
    (56, -118, 52, 15, .05, 26),
    (-8, -117, 24, 15, .10, 17),
    (-103, -116, 39, 17, -.12, 22),
]


def terrain_height(x, y, channel):
    if channel:
        if abs(y) <= 90:
            return RIVER_Z
        # Keep the channel open through the surrounding land, rather than ending
        # it at an artificial vertical wall at the gameplay boundary.
        return RIVER_Z + .10 * field(x, y, .15) * smoothstep(90, 102, abs(y))
    if abs(x) <= 120 and abs(y) <= 90:
        return GROUND_Z

    outside = max(abs(x) - 120, abs(y) - 90, 0)
    apron = smoothstep(0, 12, outside)
    # Warped ridge axes break uniform elliptical silhouettes.
    wx = x + 4.2 * field(x, y, .042, 3)
    wy = y + 4.0 * field(x, y, .046, 7)
    mountain = 0.0
    for cx, cy, rx, ry, angle, height in RIDGES:
        dx, dy = wx - cx, wy - cy
        u = (dx * math.cos(angle) + dy * math.sin(angle)) / rx
        v = (-dx * math.sin(angle) + dy * math.cos(angle)) / ry
        radial = math.sqrt(u * u + v * v)
        # A broken mesa crest, steep middle slope and long talus foot, instead of
        # a repeated dome. Each radius is locally changed by coarse erosion.
        edge = radial + .12 * field(x, y, .12, 15)
        crest = 1.0 - smoothstep(.48, 1.18, edge)
        toe = .16 * math.exp(-radial * radial * .77)
        mountain = max(mountain, height * (crest + toe))

    # Branching gullies use warped coordinates. Subtracting narrow channels from
    # the ridges makes drainage reach the talus and breaks a continuous wall.
    warp = field(x, y, .065, 28)
    erosion = abs(field(x + warp * 8, y - warp * 5, .115, 43))
    gullies = (1.0 - smoothstep(.015, .18, erosion)) * min(5.7, mountain * .25)
    weather = (1.2 * field(x, y, .11, 51) + .48 * field(x, y, .32, 61)
               + .14 * field(x, y, .81, 75)) * smoothstep(1, 12, mountain)
    # A low irregular ridge connects mesa toes. This avoids an evenly spaced
    # procession of isolated pillars around an otherwise completely flat board.
    foothills = max(0, 2.3 + 3.4 * field(x, y, .035, 98))
    raw = max(foothills, mountain - gullies + weather)
    # Softer alternating strata are real geometric erosion shelves; keep their
    # amplitude under a metre so they read as sediment, not a height staircase.
    strata = .46 * math.sin(raw * math.tau / 3.2 + .25 * field(x, y, .18, 87))
    raw += strata * smoothstep(7, 17, raw)
    foot = .40 + .65 * field(x, y, .055, 94)
    return GROUND_Z + apron * max(0.0, min(35.0, raw + foot))


def make_material(name, color, roughness, bump_strength):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    mat.diffuse_color = (*color, 1)
    nodes, links = mat.node_tree.nodes, mat.node_tree.links
    nodes.clear()
    output = nodes.new('ShaderNodeOutputMaterial')
    shader = nodes.new('ShaderNodeBsdfPrincipled')
    shader.inputs['Base Color'].default_value = (*color, 1)
    shader.inputs['Roughness'].default_value = roughness
    texture = nodes.new('ShaderNodeTexNoise')
    texture.inputs['Scale'].default_value = 1.3
    texture.inputs['Detail'].default_value = 3.2
    coordinates = nodes.new('ShaderNodeTexCoord')
    links.new(coordinates.outputs['Object'], texture.inputs['Vector'])
    ramp = nodes.new('ShaderNodeValToRGB')
    ramp.color_ramp.elements[0].position = .18
    ramp.color_ramp.elements[0].color = tuple(c * .72 for c in color) + (1,)
    ramp.color_ramp.elements[1].position = .82
    ramp.color_ramp.elements[1].color = tuple(min(1, c * 1.14) for c in color) + (1,)
    links.new(texture.outputs['Fac'], ramp.inputs['Fac'])
    links.new(ramp.outputs['Color'], shader.inputs['Base Color'])
    bump = nodes.new('ShaderNodeBump')
    bump.inputs['Strength'].default_value = bump_strength
    bump.inputs['Distance'].default_value = .16
    links.new(texture.outputs['Fac'], bump.inputs['Height'])
    links.new(bump.outputs['Normal'], shader.inputs['Normal'])
    links.new(shader.outputs['BSDF'], output.inputs['Surface'])
    return mat


def build():
    OUT.mkdir(parents=True, exist_ok=True)
    PREVIEW.mkdir(parents=True, exist_ok=True)
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    scene = bpy.context.scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = 1

    xs = sorted(set([-170 + STEP * i for i in range(273)] +
                    [-120, -37, -23, 120]))
    ys = sorted(set([-135 + STEP * i for i in range(217)] + [-90, 90]))
    # Duplicate boundary columns create vertical cut banks. A interpolated
    # sloping quad would otherwise intrude into the perfectly level walk surface.
    columns = []
    for x in xs:
        if x == -37:
            columns.extend([(x, False), (x, True)])
        elif x == -23:
            columns.extend([(x, True), (x, False)])
        else:
            columns.append((x, -37 < x < -23))

    verts = [(x, y, terrain_height(x, y, channel))
             for y in ys for x, channel in columns]
    faces = []
    nx = len(columns)
    for j in range(len(ys) - 1):
        for i in range(nx - 1):
            a = j * nx + i
            # Alternating diagonals reduce the visual directional bias of a
            # regular mesh without changing any navigation-compatible heights.
            if (i + j) % 2:
                faces.extend([(a, a + 1, a + nx),
                              (a + 1, a + nx + 1, a + nx)])
            else:
                faces.extend([(a, a + 1, a + nx + 1),
                              (a, a + nx + 1, a + nx)])

    mesh = bpy.data.meshes.new(NAME)
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(NAME, mesh)
    bpy.context.collection.objects.link(obj)
    mesh.materials.append(make_material('Sand', (.48, .34, .20), .94, .18))
    mesh.materials.append(make_material('Sandstone', (.29, .195, .12), .90, .32))
    steep_cosine = math.cos(math.radians(31))
    for face in mesh.polygons:
        face.material_index = 1 if face.normal.z < steep_cosine else 0
        face.use_smooth = abs(face.normal.z) > .025

    # UV coordinates are physical metres. The Unreal material divides these by
    # its own tile size; applying an additional scale here would double-scale it.
    # Vertical banks use Y/Z so their UV triangles do not collapse to a line.
    uv = mesh.uv_layers.new(name='TerrainMetres')
    for face in mesh.polygons:
        vertical = abs(face.normal.z) < .1
        for loop_index in face.loop_indices:
            vertex = mesh.vertices[mesh.loops[loop_index].vertex_index].co
            uv.data[loop_index].uv = ((vertex.y, vertex.z)
                                      if vertical else (vertex.x, vertex.y))

    ground_vertices = [v.co.z for v in mesh.vertices
                       if abs(v.co.x) <= 120 and abs(v.co.y) <= 90
                       and (v.co.x < -37 or v.co.x > -23)]
    bed_vertices = [v.co.z for v in mesh.vertices
                    if -37 < v.co.x < -23 and abs(v.co.y) <= 90]
    assert ground_vertices and max(abs(z - GROUND_Z) for z in ground_vertices) < 1e-6
    assert bed_vertices and max(abs(z - RIVER_Z) for z in bed_vertices) < 1e-6
    assert max(v.co.z for v in mesh.vertices) > 25
    assert len(mesh.polygons) > 100000
    assert all(p.area > 1e-7 for p in mesh.polygons)

    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    bpy.context.view_layer.update()
    minimum = [min(v.co[k] for v in mesh.vertices) for k in range(3)]
    maximum = [max(v.co[k] for v in mesh.vertices) for k in range(3)]
    report = {
        'name': NAME, 'seed': SEED, 'source': 'original authored procedural geology',
        'vertices': len(mesh.vertices), 'triangles': len(mesh.polygons),
        'dimensions_m': list(obj.dimensions),
        'bounds_min_m': minimum, 'bounds_max_m': maximum,
        'materials': [m.name for m in mesh.materials],
        'material_triangles': {m.name: sum(p.material_index == i for p in mesh.polygons)
                               for i, m in enumerate(mesh.materials)},
        'sandstone_min_slope_degrees': 31,
        'playable_extent_m': [120, 90], 'playable_surface_z_m': GROUND_Z,
        'river_x_range_m': [-37, -23], 'river_surface_z_m': RIVER_Z,
        'grid_spacing_m': STEP, 'flat_vertex_checks': len(ground_vertices),
        'river_vertex_checks': len(bed_vertices),
        'uv_scale_m': 1, 'uv_coordinates': 'physical metres; material controls tile size',
        'fbx_axes': 'X forward, Z up',
        'fbx_smoothing': 'FACE', 'origin_m': [0, 0, 0],
        'boundary': 'open surface: no artificial extruded board rim',
    }
    (OUT / (NAME + '.json')).write_text(json.dumps(report, indent=2), encoding='utf-8')
    bpy.ops.export_scene.fbx(filepath=str(OUT / (NAME + '.fbx')),
                             use_selection=True, object_types={'MESH'},
                             axis_forward='X', axis_up='Z', global_scale=1,
                             apply_unit_scale=False, add_leaf_bones=False,
                             mesh_smooth_type='FACE')
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (NAME + '.blend')))
    print('TERRAIN_V3_EXPORTED', json.dumps(report))
    return obj


def render_preview(terrain):
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 32
    scene.cycles.use_denoising = True
    scene.world.use_nodes = True
    scene.world.node_tree.nodes['Background'].inputs['Color'].default_value = (.42, .54, .69, 1)
    scene.world.node_tree.nodes['Background'].inputs['Strength'].default_value = .42
    bpy.ops.object.light_add(type='SUN', location=(-70, -100, 140))
    sun = bpy.context.object
    sun.data.energy = 3.2
    sun.data.angle = math.radians(2.5)
    sun.rotation_euler = (math.radians(24), math.radians(-35), math.radians(-25))
    bpy.ops.object.camera_add(location=(220, -295, 250))
    camera = bpy.context.object
    camera.rotation_euler = (Vector((0, 0, 0)) - camera.location).to_track_quat('-Z', 'Y').to_euler()
    camera.data.type = 'ORTHO'
    camera.data.ortho_scale = 390
    scene.camera = camera
    scene.render.resolution_x = 1800
    scene.render.resolution_y = 1300
    scene.render.resolution_percentage = 100
    scene.view_settings.view_transform = 'AgX'
    scene.render.filepath = str(PREVIEW / 'DistrictV3-TerrainGeometry.png')
    bpy.ops.render.render(write_still=True)


if __name__ == '__main__':
    terrain = build()
    render_preview(terrain)
