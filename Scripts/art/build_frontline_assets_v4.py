"""Author six original RTS meshes, verify FBX units, and render real UI portraits.

Run with Blender 5.2 --background --python this_file.py. Optional names after --
limit a rebuild. No reference-game geometry/textures or external downloads are
used. Dimensions are metres; every ground pivot has minimum Z=-0.35 m so the
prototype's actor Z=35 cm places the geometry on the ground at scale 1.
"""
import json
import math
import sys
from pathlib import Path

import bmesh
import bpy
from mathutils import Matrix, Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import concept_asset_common as c

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Art/Models/FrontlinesV4"
PORTRAITS = ROOT / "Art/UI/Portraits"
PREVIEWS = ROOT / "Art/Previews/FrontlinesV4"
c.OUT = OUT
c.PREVIEW = PREVIEWS
DIMENSIONS = {
    "PowerPlant": (4.8, 4.4, 5.0), "Barracks": (5.0, 4.0, 3.3),
    "Radar": (4.0, 4.0, 6.0), "Defense": (3.0, 3.0, 2.5),
    "Infantry": (.9, .7, 1.8), "Siege": (3.8, 2.2, 2.6),
}


def reset():
    bpy.context.preferences.filepaths.save_version = 0
    c.reset()
    # Match the restrained ConceptV3 finish instead of a coarse granite bump.
    for name, color in {"Armor": (.16, .20, .21), "Steel": (.27, .30, .32),
                        "Dark": (.06, .075, .08)}.items():
        mat = bpy.data.materials[name]
        mat.node_tree.nodes.get("Principled BSDF").inputs["Base Color"].default_value = (*color, 1)
        for node in list(mat.node_tree.nodes):
            if node.type in {"TEX_NOISE", "BUMP"}:
                mat.node_tree.nodes.remove(node)


def panel(name, loc, size, mat="Armor", edge=.035, rot=None):
    return c.box(name, loc, size, mat, edge, rot)


def rivet_row(name, origin, count, spacing, axis="X", normal="Y", radius=.022):
    for index in range(count):
        point = list(origin)
        point["XYZ".index(axis)] += index * spacing
        rot = (math.pi / 2, 0, 0) if normal == "Y" else (0, math.pi / 2, 0) if normal == "X" else (0, 0, 0)
        c.cylinder(name, point, radius, .024, "Steel", rot, 8)


def service_pad(width, depth):
    panel("Concrete chamfer foundation", (0, 0, .16), (width, depth, .32), "Concrete", .065)
    panel("Perimeter armored deck", (0, 0, .36), (width - .16, depth - .16, .14), "Dark", .05)
    for y in (-depth / 2 + .13, depth / 2 - .13):
        panel("Perimeter metal lip", (0, y, .43), (width - .3, .07, .06), "Steel", .015)
        for x in (-width / 2 + .45, width / 2 - .45):
            panel("Amber safety inset", (x, y, .465), (.40, .045, .02), "Lamp", .004)


def power_plant():
    service_pad(4.8, 4.4)
    panel("Generator control building", (-1.10, .10, 1.37), (1.8, 3.2, 1.90), edge=.12)
    panel("Control roof armor", (-1.10, .10, 2.40), (1.98, 3.40, .25), "Steel", .06)
    panel("Main generator door", (-1.10, -1.56, 1.28), (.94, .08, 1.56), "Dark", .025)
    for index in range(8):
        panel("Generator door rib", (-1.10, -1.625, .65 + index * .17), (.84, .05, .055), "Steel", .008)
    panel("Power monitor", (-1.12, -1.652, 1.93), (.60, .025, .17), "Neon", .008)
    for side in (-1, 1):
        c.vent("Generator intake", (-1.10, .10 + side * 1.63, 1.57), 1.20, .75, count=9)
    for x, y in ((.77, -.92), (.77, .99)):
        c.cylinder("Reactor reinforced plinth", (x, y, .68), .81, .50, "Dark", vertices=32)
        c.cone("Tapered reactor armored vessel", (x, y, 2.40), .71, .56, 2.98, "Armor", 24)
        for z in (1.02, 1.65, 2.28, 2.91, 3.57):
            c.torus("Reactor reinforcement ring", (x, y, z), .67 - (z - 1.02) * .038, .057)
        c.cylinder("Cooling head", (x, y, 3.96), .57, .30, "Steel", vertices=32)
        c.cylinder("Cooling grille recess", (x, y, 4.13), .48, .055, "Dark", vertices=24)
        for index in range(7):
            panel("Cooling head fins", (x - .36 + index * .12, y, 4.17), (.038, .69, .06), "Steel", .008)
        for angle in range(0, 360, 45):
            a = math.radians(angle)
            c.beam("Reactor stiffener", (x + math.cos(a) * .72, y + math.sin(a) * .72, 1.0),
                   (x + math.cos(a) * .59, y + math.sin(a) * .59, 3.63), .047, "Steel")
        panel("Vertical flux window", (x + .66, y, 2.25), (.045, .15, 1.70), "Neon", .012)
        c.beam("Insulated reactor transfer pipe", (x, y, .85), (-.35, y, .85), .115, "Dark")
        c.torus("Pipe coupling", (-.12, y, .85), .14, .029, "Steel", (0, math.pi / 2, 0))
    for y in (-.65, .15, .95):
        c.cylinder("Transformer insulator core", (-1.58, y, 3.01), .12, .97, "Dark", vertices=16)
        for z in (2.67, 2.80, 2.93, 3.06, 3.19, 3.32):
            c.cylinder("Insulator ceramic plates", (-1.58, y, z), .23, .045, "Concrete", vertices=24)
        c.beam("High voltage bus", (-1.58, y, 3.52), (-.43, y, 3.52), .040, "Steel")
    for y in (-1.7, 1.7):
        c.beam("Distribution gantry upright", (-2.08, y, .50), (-2.08, y, 4.85), .09, "Steel")
    c.beam("Distribution gantry crossmember", (-2.08, -1.7, 4.80), (-2.08, 1.7, 4.80), .09)
    panel("Power station team sign", (-2.10, -.12, 3.88), (.10, 1.05, .43), "Team", .035)
    rivet_row("Control roof bolt", (-1.87, -1.61, 2.43), 7, .25, normal="Z")


def barracks():
    service_pad(5.0, 4.0)
    panel("Barracks reinforced lower wall", (0, .18, .87), (4.42, 2.99, .85), "Dark", .10)
    panel("Barracks sloped upper block", (0, .18, 1.73), (4.10, 2.72, 1.53), "Armor", .13)
    panel("Roof overhang", (0, .18, 2.51), (4.46, 3.07, .22), "Steel", .065)
    panel("Reinforced roof centre", (0, .18, 2.68), (3.74, 2.34, .17), "Armor", .045)
    for x in (-1.66, 0, 1.66):
        panel("Exterior buttress", (x, -1.30, 1.45), (.34, .43, 2.01), "Steel", .085)
    for x in (-.81, .81):
        panel("Access door recess", (x, -1.224, 1.34), (1.14, .045, 1.71), "Dark", .025)
        for i in range(7):
            panel("Armored door slat", (x, -1.26, .65 + i * .205), (1.00, .055, .123), "Armor", .014)
        panel("Door team inset", (x, -1.309, 1.89), (.93, .022, .064), "Neon", .006)
        panel("Door perimeter floodlight", (x, -1.32, 2.32), (.58, .055, .12), "Lamp", .015)
        panel("Entrance step", (x, -1.57, .56), (1.31, .51, .22), "Concrete", .04)
    for x in (-2.12, 2.12):
        c.vent("Side filtered intake", (x, .13, 1.62), 1.72, .69, axis="X", count=8)
        panel("Side reinforcement strip", (x, .13, .88), (.08, 2.33, .17), "Steel", .015)
    for y in (-.30, .77):
        panel("Roof HVAC housing", (-.87, y, 2.89), (1.11, .69, .37), "Dark", .04)
        for i in range(7):
            panel("HVAC grille", (-1.32 + i * .15, y, 3.09), (.045, .53, .035), "Steel", .007)
    panel("Communications pedestal", (1.47, .85, 2.86), (.50, .48, .43), "Armor", .05)
    c.beam("Short radio mast", (1.47, .85, 3.02), (1.47, .85, 3.30), .027)
    c.beam("Radio horizontal element", (1.18, .85, 3.22), (1.76, .85, 3.22), .016)
    panel("Barracks team fascia", (0, -1.408, 2.56), (1.48, .033, .20), "Team", .012)
    rivet_row("Facade roof anchor", (-1.91, -1.386, 2.54), 12, .34)
    for x in (-1.64, 1.65):
        panel("Equipment wall crate", (x, 1.31, .86), (.67, .44, .83), "Dark", .045)
        panel("Crate face", (x, 1.55, .85), (.56, .025, .58), "Steel", .022)


def curved_dish(center, radius=1.4, tilt=math.pi * .33):
    vertices, faces = [], []
    rings, slices = 9, 48
    # Closed double shell avoids zero-thickness one-sided radar silhouettes.
    for layer in range(2):
        for ring in range(rings + 1):
            r = max(.02, radius * ring / rings)
            for index in range(slices):
                a = math.tau * index / slices
                p = Vector((r * math.cos(a), r * math.sin(a), .35 * (r / radius) ** 2 - layer * .055))
                vertices.append(tuple(Matrix.Rotation(tilt, 4, "Y") @ p + Vector(center)))
    layer_size = (rings + 1) * slices
    for layer in range(2):
        start = layer * layer_size
        for ring in range(rings):
            for index in range(slices):
                nxt = (index + 1) % slices
                face = (start + ring * slices + index, start + ring * slices + nxt,
                        start + (ring + 1) * slices + nxt, start + (ring + 1) * slices + index)
                faces.append(face if layer == 0 else tuple(reversed(face)))
    for ring in (0, rings):
        start = ring * slices
        for index in range(slices):
            nxt = (index + 1) % slices
            faces.append((start + index, start + nxt, layer_size + start + nxt, layer_size + start + index))
    data = bpy.data.meshes.new("Closed parabolic radar reflector")
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new("Closed parabolic radar reflector", data)
    bpy.context.collection.objects.link(obj)
    c.finish(obj, obj.name, "Steel")
    rim_point = lambda x, y, z: tuple(Matrix.Rotation(tilt, 4, "Y") @ Vector((x, y, z)) + Vector(center))
    for index in range(12):
        a = index * math.tau / 12
        c.beam("Dish radial supporting spar", rim_point(.14 * math.cos(a), .14 * math.sin(a), -.08),
               rim_point(radius * math.cos(a), radius * math.sin(a), .29), .023, "Dark")
    feed = rim_point(0, 0, .65)
    for a in (0, math.tau / 3, 2 * math.tau / 3):
        c.beam("Radar feed tripod", rim_point(radius * .85 * math.cos(a), radius * .85 * math.sin(a), .30), feed, .024)
    c.cylinder("Radar feed receiver", feed, .115, .26, "Dark", vertices=16)


def radar():
    service_pad(4.0, 4.0)
    panel("Radar control equipment bunker", (-.72, 0, 1.29), (1.91, 2.81, 1.67), "Armor", .11)
    panel("Control roof", (-.72, 0, 2.17), (2.10, 2.98, .19), "Steel", .06)
    panel("Radar console window", (-.72, -1.445, 1.49), (1.31, .05, .46), "Glass", .025)
    panel("Green console edge", (-.72, -1.48, 1.13), (1.34, .035, .038), "Neon", .008)
    c.vent("Radar equipment vent", (-1.72, .05, 1.38), 1.75, .73, axis="X", count=9)
    c.cylinder("Mast foundation", (.80, .0, .76), .87, .63, "Dark", vertices=24)
    c.cone("Tapered communications mast", (.80, 0, 2.55), .48, .23, 3.14, "Steel", 12)
    for side in (-1, 1):
        for z in (1.0, 1.6, 2.2, 2.8, 3.4):
            c.beam("Mast triangulated brace", (.80 + side * .50, 0, z), (.80, side * .34, z + .58), .026, "Dark")
    c.cylinder("Azimuth turntable", (.80, 0, 4.17), .47, .23, "Dark", vertices=32)
    panel("Elevation yoke", (.80, 0, 4.56), (.64, .33, .82), "Armor", .05)
    curved_dish((.80, 0, 4.65))
    c.beam("Tall communications aerial", (-1.22, 1.10, 2.27), (-1.22, 1.10, 5.97), .024)
    for z in (3.2, 4.0, 4.8, 5.6):
        c.beam("Aerial dipole", (-1.22, .88, z), (-1.22, 1.32, z), .012)
    c.cylinder("Aerial obstruction lamp", (-1.22, 1.10, 5.99), .056, .08, "Lamp", vertices=12)
    for i in range(11):
        c.beam("Maintenance ladder rung", (.30, -.44, .83 + i * .28), (.67, -.44, .83 + i * .28), .013)
    panel("Radar team sign", (-.72, -1.495, 1.88), (1.22, .03, .14), "Team", .012)


def defense():
    service_pad(3.0, 3.0)
    c.cone("Octagonal defense casemate", (-.23, 0, .89), 1.24, 1.02, .94, "Armor", 8)
    c.cylinder("Traversing ring", (-.23, 0, 1.46), .85, .24, "Dark", vertices=40)
    c.torus("Team illuminated turret bearing", (-.23, 0, 1.51), .84, .022, "Neon")
    c.cone("Angular automated turret", (-.34, 0, 1.85), .90, .68, .57, "Armor", 8)
    for side in (-1, 1):
        panel("Turret cheek armor", (-.04, side * .57, 1.88), (.79, .30, .35), "Steel", .05)
        panel("Turret cheek inset", (-.05, side * .741, 1.88), (.62, .025, .18), "Dark", .015)
        for i in range(4):
            c.cylinder("Defense smoke canister", (-.61 + i * .12, side * .58, 2.12), .038, .21,
                       "Dark", (side * .48, 0, 0), 12)
    panel("Twin cannon armored mantlet", (.31, 0, 1.90), (.35, .67, .40), "Steel", .06)
    for y in (-.18, .18):
        c.beam("Defense cannon heat sleeve", (.43, y, 1.93), (1.02, y, 1.93), .090, "Steel")
        c.beam("Defense cannon barrel", (.85, y, 1.93), (1.46, y, 1.93), .060, "Dark")
        c.cylinder("Cannon muzzle shroud", (1.40, y, 1.93), .093, .20, "Steel", (0, math.pi / 2, 0), 16)
        c.cylinder("Dark cannon muzzle", (1.505, y, 1.93), .059, .012, "Dark", (0, math.pi / 2, 0), 16)
    panel("Tracking sensor head", (-.31, 0, 2.27), (.48, .50, .30), "Dark", .055)
    panel("Tracking optic", (-.055, 0, 2.27), (.025, .36, .12), "Glass", .008)
    panel("Tracking optic strip", (-.033, 0, 2.37), (.016, .25, .027), "Neon", .004)
    for side in (-1, 1):
        c.vent("Base electronics intake", (-.27, side * .95, .91), .80, .31, count=5)
    rivet_row("Casemate bolts", (-.87, -.87, 1.30), 6, .24, normal="Z")


def ellipsoid(name, pos, scale, material="Armor", segments=24, rings=16):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, radius=1, location=pos)
    obj = bpy.context.object
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return c.finish(obj, name, material)


def infantry():
    # Weapon points +X, matching the prototype's movement and firing direction.
    for side in (-1, 1):
        y = side * .125
        panel("Combat boot", (.055, y, .085), (.28, .19, .17), "Rubber", .035)
        c.beam("Lower leg undersuit", (-.012, y, .20), (-.015, y, .62), .082, "Dark")
        panel("Shin armored plate", (.075, y, .43), (.080, .14, .32), "Armor", .024)
        ellipsoid("Articulated knee pad", (.027, y, .69), (.095, .102, .115), "Steel", 20, 12)
        c.beam("Thigh undersuit", (-.032, y, .76), (-.08, side * .13, 1.01), .102, "Dark")
        panel("Thigh armor", (.05, y, .89), (.075, .17, .24), "Armor", .025, (0, .12, 0))
        panel("Side ammunition pouch", (-.065, side * .207, 1.04), (.17, .075, .18), "Dark", .02)
    ellipsoid("Pelvis harness", (-.055, 0, 1.06), (.18, .20, .14), "Dark")
    panel("Body armor lower belt", (.037, 0, 1.12), (.23, .32, .11), "Steel", .026)
    ellipsoid("Torso undersuit", (-.043, 0, 1.30), (.19, .215, .29), "Dark")
    panel("Faceted ballistic chest plate", (.101, 0, 1.35), (.13, .37, .36), "Armor", .05, (0, -.10, 0))
    panel("Chest inset plate", (.174, 0, 1.37), (.027, .29, .24), "Steel", .025, (0, -.10, 0))
    panel("Chest IFF marker", (.194, 0, 1.46), (.015, .18, .032), "Neon", .006)
    for y in (-.13, 0, .13):
        panel("Chest ammunition magazine", (.18, y, 1.20), (.09, .092, .145), "Dark", .017)
    panel("Armored backpack", (-.23, 0, 1.34), (.21, .30, .38), "Armor", .044)
    panel("Backpack shield", (-.351, 0, 1.33), (.025, .24, .26), "Steel", .028)
    c.beam("Pack short aerial", (-.255, .12, 1.52), (-.27, .13, 1.73), .008)
    for side in (-1, 1):
        shoulder = (-.02, side * .239, 1.49)
        elbow = (.15, side * .22, 1.25)
        hand = (.37 if side == 1 else .47, side * .08, 1.24)
        ellipsoid("Layered shoulder pauldron", shoulder, (.145, .118, .143), "Armor")
        c.beam("Upper arm undersuit", shoulder, elbow, .067, "Dark")
        ellipsoid("Elbow joint", elbow, (.077, .077, .080), "Steel", 16, 12)
        c.beam("Armored forearm", elbow, hand, .067, "Armor")
        ellipsoid("Combat glove", hand, (.071, .057, .069), "Dark", 16, 12)
        panel("Shoulder faction patch", (-.025, side * .355, 1.51), (.11, .015, .071), "Team", .008)
    c.cylinder("Sealed collar", (-.04, 0, 1.575), .116, .077, "Dark", vertices=24)
    ellipsoid("Armored helmet shell", (-.026, 0, 1.674), (.17, .155, .137), "Armor", 32, 20)
    panel("Helmet brow reinforcement", (.126, 0, 1.704), (.046, .279, .063), "Steel", .022)
    panel("Helmet optical visor", (.138, 0, 1.657), (.032, .235, .074), "Glass", .016)
    panel("Visor IFF accent", (.158, 0, 1.701), (.014, .174, .014), "Neon", .003)
    panel("Respirator jaw armor", (.121, 0, 1.607), (.08, .20, .069), "Dark", .021)
    for side in (-1, 1):
        c.cylinder("Helmet comm earpiece", (-.005, side * .153, 1.658), .049, .037, "Dark", (math.pi / 2, 0, 0), 16)
    panel("Rifle lower receiver", (.41, 0, 1.285), (.29, .075, .105), "Dark", .016)
    panel("Rifle upper rail", (.43, 0, 1.35), (.31, .080, .035), "Steel", .008)
    panel("Rifle buttstock", (.20, 0, 1.282), (.17, .083, .098), "Armor", .023)
    panel("Rifle magazine", (.385, 0, 1.185), (.068, .065, .137), "Dark", .015, (0, .13, 0))
    c.beam("Rifle barrel", (.54, 0, 1.305), (.76, 0, 1.305), .021, "Steel")
    c.cylinder("Rifle muzzle", (.765, 0, 1.305), .031, .055, "Dark", (0, math.pi / 2, 0), 12)
    panel("Rifle optical sight", (.42, 0, 1.397), (.085, .054, .053), "Dark", .008)
    panel("Rifle sight lens", (.467, 0, 1.399), (.007, .032, .027), "Glass", .003)


def siege():
    panel("Artillery lower chassis", (-.25, 0, .54), (2.92, 1.66, .43), "Dark", .08)
    panel("Artillery upper sloped hull", (-.28, 0, .94), (2.82, 1.57, .58), "Armor", .13)
    for side in (-1, 1):
        y = side * .90
        panel("Continuous track outer band", (-.25, y, .43), (2.92, .33, .70), "Rubber", .18)
        panel("Track inset cavity", (-.25, side * 1.075, .43), (2.58, .025, .48), "Dark", .13)
        for x in (-1.34, -.80, -.25, .30, .83):
            c.cylinder("Armored artillery road wheel", (x, side * 1.10, .42), .241, .052, "Steel", (math.pi / 2, 0, 0), 24)
            c.cylinder("Road wheel hub", (x, side * 1.13, .42), .107, .029, "Armor", (math.pi / 2, 0, 0), 16)
        for index in range(22):
            x = -1.61 + index * .131
            for z in (.08, .78):
                panel("Individual track shoe", (x, y, z), (.09, .34, .045), "Steel", .008)
        panel("Artillery side skirts", (-.26, y, 1.00), (2.75, .20, .37), "Armor", .06)
        for i in range(5):
            panel("Skirt bolted armor panel", (-1.35 + i * .545, side * 1.02, 1.02), (.45, .075, .28), "Steel", .024)
        panel("Skirt IFF strip", (-.20, side * 1.07, .90), (1.91, .024, .035), "Neon", .005)
    panel("Rear engine cover", (-1.19, 0, 1.30), (.72, 1.29, .16), "Steel", .03)
    for i in range(8):
        panel("Engine deck louver", (-1.48 + i * .084, 0, 1.391), (.032, 1.08, .03), "Dark", .005)
    c.cylinder("Artillery azimuth ring", (-.21, 0, 1.30), .63, .18, "Dark", vertices=40)
    c.cone("Heavy artillery turret", (-.27, 0, 1.70), .88, .67, .69, "Armor", 8)
    panel("Turret armored rear magazine", (-.89, 0, 1.76), (.80, 1.19, .59), "Steel", .075)
    c.cylinder("Gun elevation trunnion", (.20, 0, 1.73), .28, .79, "Dark", (math.pi / 2, 0, 0), 24)
    gun_start, gun_end = Vector((.38, 0, 1.82)), Vector((2.05, 0, 2.23))
    c.beam("Artillery barrel", gun_start, gun_end, .078, "Dark")
    c.beam("Armored barrel heat jacket", gun_start, gun_start.lerp(gun_end, .62), .139, "Steel")
    for t in (.12, .28, .44, .60):
        point = gun_start.lerp(gun_end, t)
        collar = c.cylinder("Barrel jacket collar", point, .156, .070, "Armor", vertices=24)
        collar.rotation_euler = (gun_end - gun_start).to_track_quat("Z", "Y").to_euler()
    muzzle = panel("Artillery muzzle brake", gun_end, (.23, .22, .21), "Steel", .026,
                   (0, -math.atan2(gun_end.z - gun_start.z, gun_end.x - gun_start.x), 0))
    panel("Muzzle aperture", (2.168, 0, 2.258), (.016, .116, .103), "Dark", .008)
    for side in (-1, 1):
        c.beam("Recoil piston", (.19, side * .20, 1.94), (1.16, side * .20, 2.18), .041, "Steel")
        panel("Turret ammunition access", (-.79, side * .61, 1.73), (.58, .044, .31), "Dark", .025)
        panel("Front driving optic", (.94, side * .44, 1.23), (.034, .25, .091), "Glass", .012)
        panel("Driving lamp", (1.18, side * .52, .95), (.055, .18, .095), "Lamp", .012)
        c.beam("Rear deployment stabilizer", (-1.40, side * .57, .82), (-1.59, side * .90, .24), .069, "Steel")
    c.cylinder("Commander hatch", (-.23, .16, 2.10), .23, .046, "Steel", vertices=24)
    panel("Commander periscope", (-.05, .16, 2.19), (.16, .22, .13), "Dark", .017)
    c.beam("Artillery whip aerial", (-.85, .37, 2.02), (-.92, .37, 2.61), .012)


BUILDERS = {"PowerPlant": power_plant, "Barracks": barracks, "Radar": radar,
            "Defense": defense, "Infantry": infantry, "Siege": siege}


def export_and_verify(role):
    name = "SM_HR_" + role
    meshes = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    mesh = bpy.context.object
    mesh.name = name
    bpy.context.scene.cursor.location = (0, 0, 0)
    bpy.ops.object.origin_set(type="ORIGIN_CURSOR")
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    vertices = mesh.data.vertices
    low = Vector(tuple(min(v.co[axis] for v in vertices) for axis in range(3)))
    high = Vector(tuple(max(v.co[axis] for v in vertices) for axis in range(3)))
    size = high - low
    desired = DIMENSIONS[role]
    for vertex in vertices:
        vertex.co.x = (vertex.co.x - (low.x + high.x) * .5) * desired[0] / size.x
        vertex.co.y = (vertex.co.y - (low.y + high.y) * .5) * desired[1] / size.y
        vertex.co.z = (vertex.co.z - low.z) * desired[2] / size.z - .35
    mesh.data.update()
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=math.radians(66), island_margin=.008)
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.export_scene.fbx(filepath=str(OUT / (name + ".fbx")), use_selection=True,
        object_types={"MESH"}, axis_forward="X", axis_up="Z", global_scale=1,
        apply_unit_scale=False, add_leaf_bones=False, mesh_smooth_type="FACE")
    info = {"name": name, "role": role, "triangles": sum(len(p.vertices) - 2 for p in mesh.data.polygons),
        "vertices": len(vertices), "dimensions_m": list(desired), "min_z_m": -.35,
        "runtime_actor_z_cm": 35, "runtime_scale": 1,
        "materials": [m.name for m in mesh.data.materials], "original_geometry": True}
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (name + ".blend")))
    bpy.ops.object.select_all(action="DESELECT")
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.fbx(filepath=str(OUT / (name + ".fbx")))
    imported = [o for o in bpy.context.scene.objects if o not in before and o.type == "MESH"]
    assert len(imported) == 1, (name, "FBX did not return one mesh")
    copy = imported[0]
    points = [copy.matrix_world @ Vector(v) for v in copy.bound_box]
    measured = [max(p[axis] for p in points) - min(p[axis] for p in points) for axis in range(3)]
    for actual, expected in zip(measured, desired):
        assert abs(actual - expected) < .001, (name, measured, desired)
    assert abs(min(p.z for p in points) + .35) < .001, (name, "bad ground pivot")
    bm = bmesh.new()
    bm.from_mesh(copy.data)
    nonmanifold = sum(not e.is_manifold for e in bm.edges)
    bm.free()
    assert nonmanifold == 0, (name, "open or nonmanifold FBX geometry", nonmanifold)
    info["fbx_roundtrip"] = {"passed": True, "dimensions_m": measured,
        "min_z_m": min(p.z for p in points), "nonmanifold_edges": nonmanifold,
        "materials": [m.name.split(".")[0] for m in copy.data.materials]}
    bpy.data.objects.remove(copy, do_unlink=True)
    (OUT / (name + ".json")).write_text(json.dumps(info, indent=2), encoding="utf-8")
    print("FRONTLINES_V4_EXPORT_PASS", json.dumps(info), flush=True)
    return mesh, info


def render_portrait(role, mesh):
    scene = bpy.context.scene
    points = [mesh.matrix_world @ Vector(p) for p in mesh.bound_box]
    centre = sum(points, Vector()) / 8
    radius = max(DIMENSIONS[role])
    world = bpy.data.worlds.new("Frontline neutral portrait studio")
    scene.world = world
    world.use_nodes = True
    world.node_tree.nodes.clear()
    background = world.node_tree.nodes.new("ShaderNodeBackground")
    output = world.node_tree.nodes.new("ShaderNodeOutputWorld")
    background.inputs[0].default_value = (.25, .28, .30, 1)
    background.inputs[1].default_value = .65
    world.node_tree.links.new(background.outputs[0], output.inputs[0])
    for offset, power in (((1, -1.5, 2.1), 70), ((-1.5, -.7, 1.2), 46), ((.7, 1.4, 1.9), 95)):
        bpy.ops.object.light_add(type="AREA", location=centre + Vector(offset) * radius)
        light = bpy.context.object
        light.data.energy = power * radius * radius
        light.data.size = radius * .95
        light.rotation_euler = (centre - light.location).to_track_quat("-Z", "Y").to_euler()
    bpy.ops.object.camera_add(location=centre + Vector((1.3, -1.5, 1.0)) * radius)
    camera = bpy.context.object
    camera.rotation_euler = (centre - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.type = "ORTHO"
    camera.data.ortho_scale = radius * 1.45
    scene.camera = camera
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.film_transparent = True
    scene.render.resolution_x = 512
    scene.render.resolution_y = 512
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.image_settings.color_mode = "RGBA"
    scene.render.filepath = str(PORTRAITS / ("T_HR_" + role + ".png"))
    bpy.ops.render.render(write_still=True)
    import shutil
    shutil.copy2(scene.render.filepath, PREVIEWS / ("SM_HR_" + role + ".png"))
    print("FRONTLINES_V4_PORTRAIT_PASS", role, flush=True)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    PORTRAITS.mkdir(parents=True, exist_ok=True)
    PREVIEWS.mkdir(parents=True, exist_ok=True)
    roles = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else list(BUILDERS)
    records = []
    for role in roles:
        assert role in BUILDERS, role
        reset()
        BUILDERS[role]()
        mesh, info = export_and_verify(role)
        render_portrait(role, mesh)
        records.append(info)
    # A targeted rebuild must preserve the complete six-role manifest.
    complete_records = [json.loads((OUT / ("SM_HR_" + role + ".json")).read_text(encoding="utf-8"))
                        for role in BUILDERS if (OUT / ("SM_HR_" + role + ".json")).is_file()]
    (OUT / "FrontlinesV4-manifest.json").write_text(json.dumps(complete_records, indent=2), encoding="utf-8")
    print("FRONTLINES_V4_COMPLETE", len(records), flush=True)


if __name__ == "__main__":
    main()
