"""Original graphite Ember HQ, authored from the supplied left-hand base concept.

Run Blender 5.2 --background --python this_file.py. No external game geometry.
The entrance is authored on Blender +Y, which imports as Unreal -Y.
"""
import hashlib
import json
import math
import sys
from pathlib import Path

import bpy
from mathutils import Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import concept_asset_common as c

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'Art/Models/ReferenceEmberHQV7'
OUT.mkdir(parents=True, exist_ok=True)
c.OUT = OUT
c.PREVIEW = OUT
c.PALETTE.update({
    'Armor': ((.085, .095, .105), .12, .64),
    'Steel': ((.21, .22, .23), .80, .40),
    'Dark': ((.024, .028, .032), .08, .79),
    'Concrete': ((.24, .23, .21), .0, .87),
    'Team': ((.025, .32, .13), .08, .61),
    'Neon': ((.025, .70, .28), .10, .30),
    'Glass': ((.012, .055, .032), .40, .24),
    'Dust': ((.25, .205, .15), .0, .90),
    'Lamp': ((.90, .32, .08), .0, .36),
})
c.reset()
NAME = 'SM_RV7_EmberHQ'


def outline(cx, cy, width, depth, z, cut):
    """A rectangular perimeter with clipped corners, never a circular core."""
    x, y = width / 2, depth / 2
    return [(cx + px, cy + py, z) for px, py in (
        (-x + cut, -y), (x - cut, -y), (x, -y + cut), (x, y - cut),
        (x - cut, y), (-x + cut, y), (-x, y - cut), (-x, -y + cut))]


def tapered_block(name, center, lower_size, upper_size, bottom, top,
                  material='Armor', cut=.13, bevel=.035, upper_shift=(0., 0.)):
    """Eight flat sloping wall panels, with independent upper footprint."""
    cx, cy = center
    vertices = outline(cx, cy, *lower_size, bottom, cut)
    vertices += outline(cx + upper_shift[0], cy + upper_shift[1],
                        *upper_size, top, min(cut, min(upper_size) / 4))
    faces = [tuple(range(7, -1, -1)), tuple(range(8, 16))]
    faces += [(i, (i + 1) % 8, (i + 1) % 8 + 8, i + 8) for i in range(8)]
    mesh = bpy.data.meshes.new(name + ' geometry')
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    c.finish(obj, name, material)
    return c.bevel(obj, bevel, 2)


def fitted_field(name, quad, u0, u1, v0, v1, material='Armor', offset=.03,
                 thickness=.035):
    """A fitted mechanical panel on a sloped wall, not a floating flat decal."""
    p0, p1, p2, p3 = [Vector(p) for p in quad]
    normal = (p1-p0).cross(p3-p0).normalized()
    def point(u, v):
        return (p0*(1-u)+p1*u)*(1-v)+(p3*(1-u)+p2*u)*v
    base = [point(u, v)+normal*offset
            for u, v in [(u0,v0),(u1,v0),(u1,v1),(u0,v1)]]
    upper = [p+normal*thickness for p in base]
    mesh = bpy.data.meshes.new(name+' geometry')
    mesh.from_pydata(base+upper, [], [(3,2,1,0),(4,5,6,7),(0,1,5,4),
                                    (1,2,6,5),(2,3,7,6),(3,0,4,7)])
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    c.finish(obj, name, material)
    c.bevel(obj, .008, 1)
    return point, normal


# A broad chamfered slab, not the old circular fortress foundation.
tapered_block('Chamfered square reinforced apron', (0, 0), (7.50, 7.60),
              (7.50, 7.60), .0, .22, 'Concrete', .38, .035)
tapered_block('Inset graphite ground deck', (0, -.05), (7.10, 7.10),
              (7.10, 7.10), .22, .30, 'Dark', .33, .015)

# The central hall is set back behind the portal. Sloping faceted shoulders
# and an offset command crown replace the Aegis stepped horizontal/dish layout.
tapered_block('Central sloped command hall', (0, -.80), (4.65, 3.70),
              (3.95, 3.18), .31, 3.25, 'Armor', .65, .07)
tapered_block('Main hall lower armored plinth', (0, -.80), (4.85, 3.90),
              (4.73, 3.79), .30, .60, 'Steel', .66, .035)
tapered_block('Main roof graphite rim', (0, -.80), (4.15, 3.39),
              (4.09, 3.34), 3.22, 3.42, 'Steel', .63, .025)
tapered_block('Setback command crown', (-.12, -.95), (2.20, 2.15),
              (1.94, 1.94), 3.42, 3.98, 'Armor', .65, .04)
tapered_block('Eight-sided command service cap', (-.12, -.95), (2.40, 2.35),
              (2.34, 2.29), 3.97, 4.10, 'Steel', .65, .018)

# Actual armor fields and fasteners break up the previously empty hall walls.
# Larger corner facets carry tall fields; straight walls expose a strip above
# the low side wings. The portal retains a separate, readable front silhouette.
hall_bottom = outline(0, -.80, 4.65, 3.70, .31, .65)
hall_top = outline(0, -.80, 3.95, 3.18, 3.25, .65)
for face_index in [0, 1, 2, 3, 5, 6, 7]:
    quad = [hall_bottom[face_index], hall_bottom[(face_index+1)%8],
            hall_top[(face_index+1)%8], hall_top[face_index]]
    v0, v1 = ((.13, .94) if face_index%2 else (.64, .94))
    columns = [(.06, .46), (.54, .94)] if face_index%2==0 else [(.10, .90)]
    for u0, u1 in columns:
        point, normal = fitted_field('Hall recessed armor border', quad, u0,u1,v0,v1,
                                     'Dark', .022, .025)
        fitted_field('Hall pressed armor insert', quad, u0+.035,u1-.035,v0+.020,v1-.020,
                     'Armor', .048, .028)
        rotation = normal.to_track_quat('Z', 'Y').to_euler()
        for u, v in [(u0+.025,v0+.025),(u1-.025,v0+.025),
                     (u0+.025,v1-.025),(u1-.025,v1-.025)]:
            c.cylinder('Hall captive fastener', point(u,v)+normal*.080, .014, .012,
                       'Steel', rot=rotation, vertices=6)
        if face_index%2:
            fitted_field('Corner inset green service line', quad, .44,.56,.31,.58,
                         'Team', .079,.014)

# Forward entrance: a genuine 1.3m deep recess, with the backing shutter
# separated from the forward support piers. The portal leans toward the crown.
c.box('Portal backing shadow', (0, 1.061, 1.42), (1.65, .045, 1.92), 'Dark', .014)
c.box('Inset armored entry shutter', (0, 1.092, 1.40), (1.36, .030, 1.69), 'Steel', .014)
for x in [-.45, -.15, .15, .45]:
    c.box('Vertical shutter panel split', (x, 1.111, 1.40), (.018, .010, 1.60), 'Dark', .001)
for z in [.66, 1.03, 1.40, 1.77, 2.14]:
    c.box('Armored shutter transverse bar', (0, 1.121, z), (1.31, .025, .045), 'Armor', .004)
for side in [-1, 1]:
    x = side * 1.09
    tapered_block('Forward portal sloped pier', (x, 1.74), (.64, 1.25),
                  (.53, 1.13), .40, 2.77, 'Steel', .07, .04,
                  upper_shift=(-side * .19, -.24))
    c.box('Portal pier armored foot', (x, 1.95, .50), (.94, .93, .28), 'Armor', .035)
    c.beam('Portal recessed signal strip', (side * .88, 2.335, .73),
           (side * .72, 2.09, 2.46), .025, 'Neon')
    c.beam('Outer pier machined frame', (side * 1.34, 2.30, .65),
           (side * 1.11, 2.04, 2.68), .042, 'Armor')
tapered_block('Forward portal angular lintel', (0, 1.62), (2.90, 1.49),
              (2.35, 1.10), 2.61, 2.97, 'Steel', .12, .045,
              upper_shift=(0, -.18))
lintel_bottom=outline(0,1.62,2.90,1.49,2.61,.12)
lintel_top=outline(0,1.44,2.35,1.10,2.97,.12)
fitted_field('Portal lintel inset graphite armor',
             [lintel_bottom[4],lintel_bottom[5],lintel_top[5],lintel_top[4]],
             .07,.93,.15,.84,'Armor',.035,.025)
c.box('Portal call sign plate', (0, 2.383, 2.73), (.85, .045, .15), 'Team', .012)
c.box('Recess ceiling worklight', (0, 1.65, 2.63), (.95, .68, .035), 'Lamp', .008)

# Two front shoulders articulate the hall rather than mirroring the other HQ.
for side in [-1, 1]:
    tapered_block('Faceted forward armored shoulder', (side * 1.82, .78),
                  (.95, 1.05), (.76, .83), .63, 2.81, 'Armor', .10, .035,
                  upper_shift=(-side * .10, -.30))
    c.beam('Shoulder stepped signal line', (side * 2.18, 1.24, .88),
           (side * 1.97, .94, 2.55), .020, 'Neon')
    c.box('Shoulder faction armor band', (side * 1.82, 1.322, 1.17),
          (.58, .035, .11), 'Team', .008)

# Low asymmetric wings with square faces: the right rear block carries the
# second antenna, while the left front wing stays lower and broader.
tapered_block('Low western communications wing', (-2.73, -.47), (1.65, 3.54),
              (1.49, 3.37), .31, 1.82, 'Armor', .17, .055)
tapered_block('Western wing roof armor', (-2.73, -.47), (1.78, 3.65),
              (1.72, 3.59), 1.79, 1.98, 'Steel', .19, .025)
tapered_block('Eastern operations wing', (2.69, -.90), (1.72, 3.08),
              (1.53, 2.88), .31, 2.05, 'Armor', .18, .055)
tapered_block('Eastern wing roof armor', (2.69, -.90), (1.85, 3.20),
              (1.77, 3.13), 2.01, 2.20, 'Steel', .20, .025)
tapered_block('Compact rear signals tower', (2.58, -1.80), (1.25, 1.32),
              (1.10, 1.17), 2.19, 3.20, 'Armor', .13, .045)
tapered_block('Signals tower cap', (2.58, -1.80), (1.23, 1.30),
              (1.18, 1.25), 3.18, 3.35, 'Steel', .14, .02)

for x, y, height in [(-2.73, 1.327, 1.26), (2.69, .661, 1.44)]:
    c.box('Wing front equipment inset', (x, y, 1.05), (1.18, .042, height), 'Dark', .018)
    c.box('Wing front armored panel', (x, y + .03, 1.05), (.98, .035, height * .80), 'Armor', .012)
    c.box('Wing rectangular identification bar', (x, y + .055, 1.53), (.88, .020, .08), 'Neon', .006)
    for offset in [-.38, .38]:
        c.box('Wing panel vertical frame', (x + offset, y + .065, 1.05), (.042, .035, .78), 'Steel', .004)

# Side equipment bays create a different rhythm on each wing.
for side, xs, ys in [(-1, -3.566, [-1.54, -.48, .58]),
                     (1, 3.559, [-1.79, -.76])]:
    for y in ys:
        c.box('Recessed side heat exchanger', (xs, y, 1.05), (.030, .73, .81), 'Dark', .012)
        for z in [.76, .89, 1.02, 1.15, 1.28, 1.41]:
            c.box('Heat exchanger louver', (xs + side * .014, y, z), (.040, .65, .030), 'Steel', .003)
        c.box('Side signal slot', (xs + side * .025, y, 1.61), (.022, .58, .055), 'Team', .004)

# The low faceted crown carries fitted windows, not floating rectangular panes.
crown_bottom=outline(-.12,-.95,2.20,2.15,3.42,.65)
crown_top=outline(-.12,-.95,1.94,1.94,3.98,.485)
for face_index in range(8):
    quad=[crown_bottom[face_index],crown_bottom[(face_index+1)%8],
          crown_top[(face_index+1)%8],crown_top[face_index]]
    fitted_field('Crown fitted green observation window',quad,.12,.88,.28,.74,
                 'Glass',.023,.025)
    fitted_field('Crown inset green signal sill',quad,.12,.88,.16,.23,
                 'Neon',.026,.025)
# Two slender antenna blades; no satellite dish or tall corner towers.
for x, y, roof_z, blade_height in [(-.20, -.90, 4.07, 1.22),
                                  (2.58, -1.80, 3.35, 1.06)]:
    tapered_block('Antenna rectangular bearing', (x, y), (.53, .50), (.40, .38),
                  roof_z, roof_z + .19, 'Steel', .06, .015)
    c.box('Thin armored antenna blade', (x, y, roof_z + .19 + blade_height / 2),
          (.19, .23, blade_height), 'Armor', .018)
    c.box('Antenna signal recess', (x, y + .124, roof_z + .22 + blade_height / 2),
          (.09, .015, blade_height * .76), 'Dark', .002)
    for step in range(4):
        c.box('Antenna segmented signal', (x, y + .135, roof_z + .35 + step * .18),
              (.056, .012, .095), 'Neon', .002)
    c.beam('Antenna whip', (x, y, roof_z + .19 + blade_height),
           (x, y, roof_z + .19 + blade_height + .51), .012, 'Steel')
    c.beam('Offset antenna rod', (x + .26, y, roof_z + .05),
           (x + .26, y, roof_z + blade_height + .10), .010, 'Steel')

# Visible roof servicing around the crown, in the gaps between the wings.
for x in [-1.50, 1.38]:
    for y in [-1.98, -.83]:
        c.box('Roof slatted service panel', (x, y, 3.45), (.48, .80, .08), 'Dark', .008)
        for index in range(5):
            c.box('Roof equipment slat', (x - .19 + index * .095, y, 3.51),
                  (.034, .68, .035), 'Steel', .003)
for x in [-1.5, 0, 1.5]:
    c.box('Rear main facade access plate', (x, -2.666, 1.48), (.91, .035, 1.47), 'Steel', .018)
    c.box('Rear access panel face', (x, -2.692, 1.48), (.75, .026, 1.27), 'Armor', .012)

# Broad entry stairs, compact equipment at the front corners, parking guides.
for index in range(4):
    y = 2.42 + index * .27
    top = .57 - index * .085
    c.box('Portal broad entry stair', (0, y, .30 + (top - .30) / 2),
          (1.90, .33, max(.035, top - .30)), 'Steel', .012)
    c.box('Stair edge wear strip', (0, y + .15, top + .006),
          (1.73, .025, .012), 'Dust', .002)
for side in [-1, 1]:
    x, y = side * 2.60, 2.72
    tapered_block('Forward protected utility cabinet', (x, y), (.90, .87),
                  (.77, .75), .30, 1.11, 'Armor', .10, .03)
    c.box('Forward cabinet steel cap', (x, y, 1.13), (.88, .86, .10), 'Steel', .018)
    c.vent('Utility cabinet front intake', (x, y + .45, .72), .56, .44, count=6)
    c.box('Cabinet signal edge', (x + side * .34, y + .456, .73), (.035, .025, .53), 'Neon', .004)
    c.box('Apron parking guide', (side * 3.36, .40, .312), (.030, 5.44, .012), 'Dust', .002)
for y in [-3.28, 3.28]:
    c.box('Apron transverse utility marking', (0, y, .312), (6.55, .030, .012), 'Dust', .002)
for x, y in [(-1.65, 2.91), (1.65, 2.91), (-2.40, -3.06), (2.40, -3.06)]:
    c.box('Recessed ground service hatch', (x, y, .317), (.53, .41, .020), 'Steel', .008)
    for index in range(4):
        c.box('Service hatch channels', (x - .18 + index * .12, y, .330),
              (.020, .31, .008), 'Dark', .001)

# Source origin convention is shared with runtime HQ actors, not changed here.
model = c.export(NAME, zoffset=-1.10)
source_points = [model.matrix_world @ vertex.co for vertex in model.data.vertices]
bounds = [[min(p[i] for p in source_points), max(p[i] for p in source_points)]
          for i in range(3)]
assert all(pair[1] - pair[0] <= 8.0 for pair in bounds[:2]), bounds
assert abs(bounds[2][0] + 1.10) < .001, bounds
assert Vector(model.rotation_euler).length < .001, list(model.rotation_euler)

# Roundtrip checks the exported artifact, not only the modelling parameters.
bpy.ops.object.select_all(action='DESELECT')
bpy.ops.import_scene.fbx(filepath=str(OUT / (NAME + '.fbx')))
imported = [obj for obj in bpy.context.selected_objects if obj.type == 'MESH']
roundtrip_points = [obj.matrix_world @ vertex.co for obj in imported
                    for vertex in obj.data.vertices]
roundtrip_bounds = [[min(p[i] for p in roundtrip_points), max(p[i] for p in roundtrip_points)]
                    for i in range(3)]
assert all(abs(value - other) < .001 for pair, second in zip(bounds, roundtrip_bounds)
           for value, other in zip(pair, second)), (bounds, roundtrip_bounds)
for obj in imported:
    bpy.data.objects.remove(obj, do_unlink=True)
metadata_path = OUT / (NAME + '.json')
meta = json.loads(metadata_path.read_text(encoding='utf-8'))
assert meta['triangles'] <= 35000, meta['triangles']
meta.update({
    'provenance': 'Original procedural HeroHour geometry; supplied concept reference only; no extracted game assets',
    'reference': 'Codex-Bild 1. Sept. 2026, 01_22_20.png, left graphite faction HQ',
    'validation': 'PASS',
    'bounds_m': bounds,
    'fbx_roundtrip_bounds_m': roundtrip_bounds,
    'fbx_roundtrip_pass': True,
    'fbx_sha256': hashlib.sha256((OUT / (NAME + '.fbx')).read_bytes()).hexdigest(),
    'unreal_expected_bounds_cm': [[a * 100, b * 100] if i != 1 else [-b * 100, -a * 100]
                                  for i, (a, b) in enumerate(bounds)],
    'runtime_scale': 1.5,
    'runtime_actor_z_cm': 110,
    'runtime_relative_z_cm': 55,
    'runtime_ground_z_cm': 110 + 55 + bounds[2][0] * 150,
    'blender_entrance_front_normal': [0, 1, 0],
    'unreal_entrance_front_normal': [0, -1, 0],
    'design': 'Sloping faceted portal hall; fitted mechanical armor fields; low asymmetric service wings; low eight-sided command crown and two slim antenna blades; no circular fortress core or roof dish',
    'color_decision': 'Graphite armor and green Team/Neon follow left concept; enemy health bars and gameplay identification remain unchanged',
    'limits': 'Static joined geometry; no moving antenna, destruction stages, unique grime atlas, authored LOD chain or traversable interior',
})
metadata_path.write_text(json.dumps(meta, indent=2) + '\n', encoding='utf-8')

# Actual exported geometry rendered from its entrance quarter, matching the
# gameplay-facing axis. The source blend is saved before preview-only objects.
scene = bpy.context.scene
zmin, zmax = bounds[2]
radius = max(model.dimensions)
c.box('Preview-only ground', (0, 0, zmin - .09), (radius * 4, radius * 4, .16), 'Concrete', 0)
world = bpy.data.worlds.new('Ember neutral studio')
scene.world = world
world.use_nodes = True
world.node_tree.nodes.clear()
background = world.node_tree.nodes.new('ShaderNodeBackground')
world_output = world.node_tree.nodes.new('ShaderNodeOutputWorld')
world.node_tree.links.new(background.outputs[0], world_output.inputs[0])
background.inputs[0].default_value = (.22, .25, .29, 1)
background.inputs[1].default_value = .5
for loc, power, size in [((-4, 6, 10), 1800, 7), ((5, 1, 6), 1500, 5), ((-2, -5, 8), 2100, 5)]:
    bpy.ops.object.light_add(type='AREA', location=loc)
    light = bpy.context.object
    light.data.energy = power
    light.data.shape = 'DISK'
    light.data.size = size
    light.rotation_euler = (Vector((0, 0, 2)) - light.location).to_track_quat('-Z', 'Y').to_euler()
target = Vector((0, 0, (zmin + zmax) * .47))
bpy.ops.object.camera_add(location=(-radius * 1.2, radius * 1.4, radius * 1.1))
cam = bpy.context.object
cam.rotation_euler = (target - cam.location).to_track_quat('-Z', 'Y').to_euler()
cam.data.type = 'ORTHO'
cam.data.ortho_scale = radius * 1.65
scene.camera = cam
scene.render.engine = 'CYCLES'
scene.cycles.samples = 48
scene.cycles.use_denoising = True
scene.render.resolution_x = 1200
scene.render.resolution_y = 1000
scene.render.resolution_percentage = 100
scene.render.image_settings.file_format = 'PNG'
scene.render.filepath = str(OUT / (NAME + '.png'))
bpy.ops.render.render(write_still=True)
print('REFERENCE_EMBER_HQ_V7_PASS ' + json.dumps(meta), flush=True)
