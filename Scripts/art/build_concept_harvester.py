"""Original six-wheel HERO resource harvester, authored in metre units.

Uses the established ConceptV2 material names and +X-forward convention. The
mesh floor is -0.35 m for the existing RTS unit actor's 35 cm anchor. Geometry
is triangulated, exported, imported again and checked before preview rendering.
No geometry or textures from another game are used. This is a static mesh;
cutter, wheels and unload coupler are visually articulated but not yet rigged.
"""
import json
import math
import sys
from pathlib import Path

import bmesh
import bpy
from mathutils import Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import concept_asset_common as common
from concept_asset_common import bevel, finish


PROJECT = Path(__file__).resolve().parents[2]
OUT = PROJECT / "Art/Models/ConceptV2"
PREVIEW = PROJECT / "Saved/Research/Harvester-QA.png"
NAME = "SM_HERO_Harvester"


def box(label, location, size, material="Armor", edge=.025, rotation=None):
    bpy.ops.mesh.primitive_cube_add(size=1, location=location)
    obj = bpy.context.object
    obj.dimensions = size
    if rotation:
        obj.rotation_euler = rotation
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    return bevel(finish(obj, label, material), min(edge, min(size) * .18), 1)


def cylinder(label, location, radius, depth, material="Steel", rotation=(0, 0, 0), vertices=24):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth,
                                       location=location, rotation=rotation)
    return bevel(finish(bpy.context.object, label, material), min(.008, depth * .12, radius * .12), 1)


def beam(label, start, end, radius=.025, material="Steel"):
    start, end = Vector(start), Vector(end)
    direction = end - start
    obj = cylinder(label, (start + end) / 2, radius, direction.length, material, vertices=10)
    obj.rotation_euler = direction.to_track_quat("Z", "Y").to_euler()
    return obj


def torus(label, location, major, minor=.04, material="Steel", rotation=(0, 0, 0)):
    bpy.ops.mesh.primitive_torus_add(major_radius=major, minor_radius=minor,
                                    major_segments=24, minor_segments=6,
                                    location=location, rotation=rotation)
    return finish(bpy.context.object, label, material)


def hull(label, rings, material):
    vertices = []
    for x, half_width, bottom, top in rings:
        vertices.extend(((x, -half_width, bottom), (x, half_width, bottom),
                         (x, half_width * .83, top), (x, -half_width * .83, top)))
    faces = [(3, 2, 1, 0)]
    for ring in range(len(rings) - 1):
        for corner in range(4):
            faces.append((ring * 4 + corner, ring * 4 + (corner + 1) % 4,
                          (ring + 1) * 4 + (corner + 1) % 4, (ring + 1) * 4 + corner))
    base = (len(rings) - 1) * 4
    faces.append((base, base + 1, base + 2, base + 3))
    mesh = bpy.data.meshes.new(label)
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(label, mesh)
    bpy.context.collection.objects.link(obj)
    return bevel(finish(obj, label, material), .020, 1)


def wheel(x, side):
    y = side * .805
    cylinder("Deep offroad tire", (x, y, .355), .35, .265, "Rubber", (math.pi / 2, 0, 0), 28)
    cylinder("Rim protection", (x, side * .955, .355), .227, .028, "Dark", (math.pi / 2, 0, 0), 20)
    cylinder("Six-stud wheel hub", (x, side * .977, .355), .157, .025, "Steel", (math.pi / 2, 0, 0), 16)
    cylinder("Axle dust cap", (x, side * .994, .355), .078, .018, "Armor", (math.pi / 2, 0, 0), 8)
    for index in range(6):
        angle = math.tau * index / 6
        cylinder("Wheel stud", (x + math.sin(angle) * .113, side * 1.003,
                               .355 + math.cos(angle) * .113), .014, .012,
                 "Steel", (math.pi / 2, 0, 0), 6)
    for index in range(18):
        angle = math.tau * index / 18
        for offset, skew in ((-.066, .20), (.066, -.20)):
            box("Herringbone tread", (x + math.sin(angle) * .346,
                                      y + offset, .355 + math.cos(angle) * .346),
                (.102, .118, .028), "Rubber", .003, (0, angle, side * skew))
    beam("Suspension trailing arm", (x - .17, side * .56, .59), (x, side * .70, .35), .040, "Dark")
    beam("Suspension piston", (x - .10, side * .62, .71), (x + .12, side * .70, .43), .022, "Steel")


def create_geometry():
    hull("Lower armored chassis", [(-1.73, .59, .38, .67), (-1.46, .70, .34, .78),
                                   (.90, .70, .34, .78), (1.45, .57, .43, .69)], "Dark")
    hull("Reinforced deck", [(-1.65, .69, .68, .92), (-1.16, .69, .72, .97),
                              (.72, .68, .72, .96), (1.43, .56, .65, .86)], "Armor")
    for side in (-1, 1):
        for x in (-1.19, -.19, .97):
            wheel(x, side)
        box("Continuous fender spine", (-.15, side * .766, .789), (2.84, .14, .11), "Armor", .024)
        for x in (-1.19, -.19, .97):
            box("Tire armored mudguard", (x, side * .79, .77), (.72, .31, .095), "Steel", .025)
            box("Mudguard team inset", (x, side * .84, .832), (.36, .13, .028), "Team", .006)
        box("Middle step tread", (.35, side * .765, .558), (.31, .18, .075), "Dark", .009)
        for x in (-.30, .36):
            box("Service access armor", (x, side * .686, .90), (.38, .07, .24), "Armor", .012)
            box("Service recessed panel", (x, side * .729, .90), (.28, .025, .15), "Dark", .005)
            beam("Access latch", (x - .055, side * .751, .93), (x + .055, side * .751, .93), .012, "Steel")
        box("Chassis identification line", (-.63, side * .737, .737), (.86, .027, .026), "Team", .004)
        beam("Roof grab rail", (.61, side * .61, 1.56), (1.07, side * .61, 1.40), .018, "Steel")

    # Sloping forward driver pod, with separately framed windscreen and side windows.
    hull("Armored forward cabin", [(.13, .53, .87, 1.60), (.44, .58, .87, 1.67),
                                    (.88, .56, .85, 1.61), (1.36, .44, .79, 1.24)], "Armor")
    box("Windshield steel surround", (1.085, 0, 1.442), (.046, .878, .424), "Steel", .017, (0, -.85, 0))
    box("Laminated front windshield", (1.106, 0, 1.460), (.025, .794, .334), "Glass", .011, (0, -.85, 0))
    box("Windshield central pillar", (1.125, 0, 1.477), (.024, .027, .341), "Armor", .006, (0, -.85, 0))
    for side in (-1, 1):
        box("Side window armored surround", (.64, side * .525, 1.433), (.545, .038, .255), "Steel", .014)
        box("Side window glazing", (.64, side * .550, 1.433), (.458, .022, .190), "Glass", .009)
        box("Side window dividing brace", (.615, side * .565, 1.433), (.024, .018, .196), "Armor", .004)
        box("Driver door team plate", (.66, side * .572, 1.15), (.45, .026, .204), "Team", .009)
        beam("Door handle", (.73, side * .591, 1.232), (.85, side * .591, 1.232), .012, "Steel")
        for x in (.32, .92):
            cylinder("Door hinge", (x, side * .594, 1.193), .019, .12, "Steel", vertices=8)
        beam("Mirror stalk", (1.05, side * .47, 1.45), (1.14, side * .64, 1.50), .013, "Dark")
        box("Mirror housing", (1.16, side * .655, 1.51), (.061, .057, .09), "Armor", .007)
        box("Forward headlamp mount", (1.38, side * .30, 1.08), (.073, .195, .115), "Dark", .009)
        box("Forward worklight", (1.425, side * .30, 1.08), (.019, .15, .069), "Lamp", .004)
    box("Cab roof armored brow", (.61, 0, 1.685), (.78, 1.014, .075), "Steel", .018)
    cylinder("Driver rescue hatch", (.52, 0, 1.743), .196, .028, "Armor", vertices=16)
    beam("Rescue hatch grip", (.47, -.08, 1.777), (.60, -.08, 1.777), .011, "Steel")
    box("Roof sensor block", (.45, .39, 1.76), (.139, .112, .112), "Dark", .011)
    box("Roof sensor glass", (.528, .39, 1.77), (.018, .076, .046), "Glass", .003)
    cylinder("Amber work beacon", (.38, -.37, 1.796), .051, .084, "Lamp", vertices=16)
    cylinder("Beacon plinth", (.38, -.37, 1.735), .068, .035, "Dark", vertices=12)

    # Long horizontal pressure vessel dominates the resource vehicle silhouette.
    cylinder("Resource tank main shell", (-.73, 0, 1.275), .447, 1.58, "Steel", (0, math.pi / 2, 0), 32)
    for x in (-1.51, -.73, .05):
        cylinder("Tank armored collar", (x, 0, 1.275), .474, .085, "Armor", (0, math.pi / 2, 0), 32)
        torus("Tank collar inset", (x, 0, 1.275), .470, .012, "Team", (0, math.pi / 2, 0))
    for x in (-1.52, .06):
        cylinder("Pressure vessel end cap", (x, 0, 1.275), .398, .046, "Armor", (0, math.pi / 2, 0), 24)
    for side in (-1, 1):
        for x in (-1.36, -1.11, -.44, -.19):
            box("Tank longitudinal applique", (x, side * .385, 1.451), (.21, .09, .33), "Armor", .019, (side * .42, 0, 0))
        box("Level gauge black surround", (-.73, side * .457, 1.273), (.55, .028, .16), "Dark", .008)
        box("Level gauge glass", (-.73, side * .477, 1.273), (.48, .016, .115), "Glass", .006)
        for index in range(7):
            box("Resource fill gauge segment", (-.935 + index * .068, side * .489, 1.273),
                (.033, .010, .057), "Neon", .002)
        beam("Resource return pipe", (-1.40, side * .52, 1.0), (-.05, side * .52, 1.0), .037, "Dark")
        for x in (-1.38, -.94, -.44, -.07):
            torus("Return pipe retaining band", (x, side * .52, 1.0), .042, .009, "Steel", (0, math.pi / 2, 0))
    box("Tank dorsal service rail", (-.73, 0, 1.734), (1.22, .45, .048), "Armor", .013)
    for x in (-1.19, -.83, -.42):
        box("Tank dorsal removable plate", (x, 0, 1.768), (.26, .34, .026), "Team", .005)
        for y in (-.115, .115):
            cylinder("Dorsal captive bolt", (x, y, 1.791), .016, .012, "Steel", vertices=6)
    cylinder("Resource tank top inlet", (-1.0, 0, 1.818), .126, .105, "Armor", vertices=16)
    cylinder("Inlet closure", (-1.0, 0, 1.882), .108, .027, "Steel", vertices=16)
    beam("Tank safety valve", (-1.11, .23, 1.72), (-1.11, .23, 1.875), .024, "Dark")
    torus("Safety valve rim", (-1.11, .23, 1.89), .057, .009, "Lamp")

    # Rear unloading interface reads as a dockable industrial coupler.
    box("Unload armored bulkhead", (-1.665, 0, 1.08), (.14, 1.01, .60), "Armor", .025)
    cylinder("Unload flange", (-1.77, 0, 1.18), .224, .10, "Steel", (0, math.pi / 2, 0), 24)
    cylinder("Unload sealing boot", (-1.839, 0, 1.18), .169, .058, "Rubber", (0, math.pi / 2, 0), 20)
    cylinder("Unload port interior", (-1.875, 0, 1.18), .116, .030, "Dark", (0, math.pi / 2, 0), 16)
    torus("Unload indexing rim", (-1.897, 0, 1.18), .108, .012, "Team", (0, math.pi / 2, 0))
    for index in range(8):
        angle = index * math.tau / 8
        cylinder("Flange captive stud", (-1.829, math.cos(angle) * .188,
                                          1.18 + math.sin(angle) * .188), .014, .019,
                 "Steel", (0, math.pi / 2, 0), 6)
    for side in (-1, 1):
        box("Rear taillight protected housing", (-1.765, side * .407, .884), (.065, .138, .10), "Dark", .007)
        box("Rear taillight lens", (-1.806, side * .407, .89), (.019, .096, .053), "Lamp", .003)
        beam("Rear dock guard", (-1.77, side * .48, .66), (-1.77, side * .48, 1.01), .022, "Steel")
        cylinder("Exhaust insulated stack", (-1.43, side * .566, 1.079), .056, .37, "Dark", vertices=16)
        torus("Exhaust raised lip", (-1.43, side * .566, 1.27), .057, .011, "Steel")
        box("Rear reflector panel", (-1.735, side * .275, .70), (.020, .107, .064), "Dust", .004)
    box("Rear tow crossbar", (-1.708, 0, .52), (.12, 1.09, .109), "Steel", .018)
    for y in (-.39, .39):
        torus("Rear recovery eye", (-1.779, y, .548), .055, .014, "Dark", (0, math.pi / 2, 0))
    beam("Radio whip antenna", (-.06, -.35, 1.63), (-.14, -.35, 2.03), .006, "Steel")
    cylinder("Radio antenna boot", (-.06, -.35, 1.63), .023, .065, "Rubber", vertices=12)

    # Articulated cutter: paired load-bearing booms, rams and a toothed cross-drum.
    for side in (-1, 1):
        cylinder("Mining arm pivot", (1.15, side * .47, .88), .083, .080, "Steel", (math.pi / 2, 0, 0), 16)
        beam("Mining lift arm", (1.13, side * .48, .84), (1.68, side * .52, .56), .070, "Armor")
        beam("Lift hydraulic cylinder", (1.14, side * .56, .92), (1.43, side * .59, .72), .043, "Dark")
        beam("Lift hydraulic piston", (1.40, side * .59, .745), (1.73, side * .59, .515), .019, "Steel")
        cylinder("Drum drive shroud", (1.77, side * .667, .495), .181, .10, "Armor", (math.pi / 2, 0, 0), 20)
        cylinder("Cutter bearing hub", (1.77, side * .731, .495), .071, .025, "Steel", (math.pi / 2, 0, 0), 12)
    cylinder("Rotary harvesting drum", (1.77, 0, .495), .155, 1.29, "Dark", (math.pi / 2, 0, 0), 28)
    for index in range(9):
        y = -.55 + index * .1375
        cylinder("Drum serrated collar", (1.77, y, .495), .166, .067, "Steel", (math.pi / 2, 0, 0), 12)
        for tooth in range(6):
            angle = tooth * math.tau / 6 + index * .17
            box("Replaceable cutter tooth", (1.77 + math.sin(angle) * .17, y,
                                             .495 + math.cos(angle) * .17),
                (.065, .054, .072), "Steel", .006, (0, angle, 0))
    box("Cutter armored debris visor", (1.662, 0, .727), (.29, 1.19, .052), "Armor", .014, (0, .12, 0))
    box("Debris visor identification stripe", (1.716, 0, .765), (.068, .95, .014), "Team", .003, (0, .12, 0))


def inspect_mesh(obj):
    points = [obj.matrix_world @ v.co for v in obj.data.vertices]
    low = [min(v[i] for v in points) for i in range(3)]
    high = [max(v[i] for v in points) for i in range(3)]
    bm = bmesh.new(); bm.from_mesh(obj.data)
    nonmanifold = sum(not edge.is_manifold for edge in bm.edges)
    bm.free()
    return {"vertices": len(points), "triangles": len(obj.data.polygons),
            "dimensions_m": [high[i] - low[i] for i in range(3)],
            "bounds_min_m": low, "bounds_max_m": high,
            "materials": [m.name for m in obj.data.materials],
            "all_faces_triangles": all(len(p.vertices) == 3 for p in obj.data.polygons),
            "nonmanifold_edges": nonmanifold,
            "degenerate_faces": sum(p.area < 1e-10 for p in obj.data.polygons),
            "uv_finite": bool(obj.data.uv_layers) and all(math.isfinite(x)
                  for value in obj.data.uv_layers.active.data for x in value.uv)}


def validate(result):
    assert 3.3 <= result["dimensions_m"][0] <= 4.0, result
    assert result["dimensions_m"][1] <= 2.1, result
    assert abs(result["bounds_min_m"][2] + .35) < 1e-5, result
    assert 15000 <= result["triangles"] <= 42000, result
    assert all(name in common.PALETTE for name in result["materials"]), result
    assert result["all_faces_triangles"] and result["uv_finite"], result
    assert result["nonmanifold_edges"] == 0 and result["degenerate_faces"] == 0, result


def render(obj):
    scene = bpy.context.scene
    box("Preview floor", (0, 0, -.425), (20, 20, .15), "Concrete", 0)
    scene.world.use_nodes = True
    scene.world.node_tree.nodes["Background"].inputs[0].default_value = (.17, .19, .22, 1)
    scene.world.node_tree.nodes["Background"].inputs[1].default_value = .35
    for position, power, size, color in (((3, -4, 7), 900, 5, (1, .90, .79)),
                                        ((-4, -2, 5), 1100, 4, (.77, .86, 1)),
                                        ((-2, 4, 6), 1500, 4, (1, .96, .91))):
        bpy.ops.object.light_add(type="AREA", location=position)
        light = bpy.context.object
        light.data.energy, light.data.size, light.data.color = power, size, color
        light.rotation_euler = (Vector((0, 0, .7)) - light.location).to_track_quat("-Z", "Y").to_euler()
    bpy.ops.object.camera_add(location=(5.0, -6.0, 4.0))
    camera = bpy.context.object
    camera.rotation_euler = (Vector((.02, 0, .55)) - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.type, camera.data.ortho_scale = "ORTHO", 4.88
    scene.camera = camera
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 64
    scene.cycles.use_denoising = True
    scene.render.resolution_x, scene.render.resolution_y = 1500, 1125
    scene.render.resolution_percentage = 100
    scene.view_settings.view_transform = "AgX"
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = str(PREVIEW)
    PREVIEW.parent.mkdir(parents=True, exist_ok=True)
    bpy.ops.render.render(write_still=True)


common.reset()
bpy.context.preferences.filepaths.save_version = 0
create_geometry()
# Normalize to an exact contact plane before the runtime anchor offset is applied.
objects = [o for o in bpy.context.scene.objects if o.type == "MESH"]
floor_z = min((o.matrix_world @ v.co).z for o in objects for v in o.data.vertices)
for obj in objects:
    obj.location.z -= floor_z
    bpy.context.view_layer.objects.active = obj
    bm = bmesh.new()
    bm.from_mesh(obj.data)
    bmesh.ops.dissolve_degenerate(bm, dist=1e-7, edges=list(bm.edges))
    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    bm.to_mesh(obj.data)
    bm.free()
    modifier = obj.modifiers.new("Explicit export triangles", "TRIANGULATE")
    bpy.ops.object.modifier_apply(modifier=modifier.name)
mesh = common.export(NAME, -.35)
source = inspect_mesh(mesh)
validate(source)
saved_materials = {name: bpy.data.materials[name] for name in source["materials"]}
for material in saved_materials.values():
    material.use_fake_user = True
bpy.ops.object.select_all(action="SELECT"); bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.fbx(filepath=str(OUT / (NAME + ".fbx")))
imported = next(obj for obj in bpy.context.scene.objects if obj.type == "MESH")
# Restore the exact palette names after the importer creates duplicate materials.
for index, material in enumerate(imported.data.materials):
    imported.data.materials[index] = saved_materials[material.name.split(".")[0]]
roundtrip = inspect_mesh(imported)
validate(roundtrip)
assert source["triangles"] == roundtrip["triangles"]
assert source["materials"] == roundtrip["materials"]
assert max(abs(a - b) for a, b in zip(source["dimensions_m"], roundtrip["dimensions_m"])) < 1e-5
info = {"name": NAME, "origin": "35 cm runtime unit anchor", "zoffset_m": -.35,
        "forward_axis": "+X", "provenance": "Original HeroHour procedural geometry",
        "feature_summary": ["six wheels with herringbone treads", "armored forward cabin",
                            "resource pressure vessel", "articulated rotary mining head",
                            "rear unloading coupler", "team plates and fill gauge"],
        "animation": "static mesh; moving parts require a later rig/component split",
        "validation": "PASS", **source, "fbx_roundtrip": roundtrip}
(OUT / (NAME + ".json")).write_text(json.dumps(info, indent=2), encoding="utf-8")
print("HARVESTER_ASSET_READY", json.dumps(info))
render(imported)
