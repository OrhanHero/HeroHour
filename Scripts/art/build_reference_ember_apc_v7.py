"""Original graphite Ember 8x8: Blender 5.2 --background --python this.py.

Independent hull/turret geometry inspired by the supplied left-hand concept;
no extracted game geometry and no dependency on the Aegis APC build script.
"""
import hashlib
import json
import math
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import concept_asset_common as c
from concept_asset_common import bpy, Vector

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'Art/Models/ReferenceEmberAPCV7'
OUT.mkdir(parents=True, exist_ok=True)
NAME = 'SM_HR_Ember_APC8'
c.OUT = OUT
c.PREVIEW = OUT
c.PALETTE.update({
    'Armor': ((.085, .095, .105), .12, .64),
    'Steel': ((.21, .22, .23), .80, .40),
    'Glass': ((.045, .075, .08), .5, .24),
    'Team': ((.025, .32, .13), .55, .48),
    'Neon': ((.025, .70, .28), .25, .3),
})
c.reset()


def faceted_shell(name, rings, material='Armor'):
    """Six-sided sections: beveled belly, deep shoulder, canted roof edge."""
    vertices = []
    for x, width, low, high in rings:
        vertices.extend([
            (x, -width * .78, low), (x, width * .78, low),
            (x, width, low + .17), (x, width * .83, high),
            (x, -width * .83, high), (x, -width, low + .17),
        ])
    faces = [(5, 4, 3, 2, 1, 0)]
    for ring in range(len(rings) - 1):
        for side in range(6):
            faces.append((ring * 6 + side, ring * 6 + (side + 1) % 6,
                          (ring + 1) * 6 + (side + 1) % 6,
                          (ring + 1) * 6 + side))
    end = (len(rings) - 1) * 6
    faces.append(tuple(end + n for n in range(6)))
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    return c.bevel(c.finish(obj, name, material), .018, 2)


# Unlike the Aegis flat-sided hull, Ember uses a continuous hexagonal hull with
# a narrow sloping prow, raised rear engine shoulders and a dropped belly.
hull = faceted_shell('Ember six sided stealth hull', [
    (-1.66, .65, .38, .94), (-1.41, .79, .28, 1.08),
    (.82, .79, .28, 1.03), (1.40, .68, .39, .87),
    (1.69, .46, .49, .66),
])
wheel_positions = (-1.18, -.39, .40, 1.19)
wheel_objects = []
arch_count = 0
for side in (-1, 1):
    for index, x in enumerate(wheel_positions):
        bpy.ops.mesh.primitive_cylinder_add(
            vertices=32, radius=.38, depth=.50,
            location=(x, side * .77, .33), rotation=(math.pi / 2, 0, 0))
        cutter = bpy.context.object
        boolean = hull.modifiers.new('Actual recessed wheel arch', 'BOOLEAN')
        boolean.operation = 'DIFFERENCE'
        boolean.object = cutter
        bpy.context.view_layer.objects.active = hull
        bpy.ops.object.modifier_apply(modifier=boolean.name)
        bpy.data.objects.remove(cutter, do_unlink=True)
        arch_count += 1
        wheel_objects.append(c.cylinder(
            'Ember road tire %s %d' % (side, index), (x, side * .78, .33),
            .33, .24, 'Rubber', (math.pi / 2, 0, 0), 32))
        c.cylinder('Deep wheel inset', (x, side * .906, .33), .21, .022,
                   'Dark', (math.pi / 2, 0, 0), 20)
        c.cylinder('Octagonal protected wheel hub', (x, side * .919, .33),
                   .145, .021, 'Armor', (math.pi / 2, 0, 0), 8)
        c.cylinder('Hub center', (x, side * .932, .33), .052, .012,
                   'Steel', (math.pi / 2, 0, 0), 8)
        for j in range(22):
            angle = math.tau * j / 22
            c.box('Diagonal tire tread',
                  (x + .328 * math.sin(angle), side * .78,
                   .33 + .328 * math.cos(angle)),
                  (.074, .25, .026), 'Rubber', 0,
                  (0, angle, side * .20))
        for j in range(5):
            angle = math.tau * j / 5
            c.cylinder('Wheel locking bolt',
                       (x + .101 * math.sin(angle), side * .934,
                        .33 + .101 * math.cos(angle)),
                       .012, .010, 'Steel', (math.pi / 2, 0, 0), 6)

    # Separate sloping shoulder blocks leave the wheel arches visible, with no
    # running board or cab opening that could read as a pickup truck.
    for x in (-1.11, -.47, .17, .81):
        c.box('Canted modular shoulder armor', (x, side * .70, .925),
              (.53, .13, .20), 'Armor', .018, (side * -.24, 0, 0))
        c.box('Shoulder panel seam', (x + .24, side * .764, .94),
              (.016, .018, .135), 'Dark', .002, (side * -.24, 0, 0))
    c.box('Faction identifier rear shoulder', (-1.05, side * .773, .97),
          (.18, .014, .055), 'Team', .003)
    c.box('Narrow front running light', (1.493, side * .42, .77),
          (.033, .145, .042), 'Lamp', .003, (0, -.20, 0))
    c.box('Recessed tail light', (-1.681, side * .48, .70),
          (.020, .077, .045), 'Team', .002)
    c.beam('Rear ramp grab handle', (-1.69, side * .37, .46),
           (-1.69, side * .37, .78), .014, 'Steel')
    # Small rear exhaust ports, distinct from the Aegis broad single top grille.
    c.box('Protected rear exhaust housing', (-1.24, side * .44, 1.10),
          (.51, .29, .08), 'Dark', .012)
    for j in range(5):
        c.box('Engine exhaust slat', (-1.43 + j * .09, side * .44, 1.15),
              (.027, .25, .018), 'Steel', .002)

c.box('Rear troop access frame', (-1.685, 0, .65), (.045, .84, .52),
      'Dark', .012)
c.box('Angular troop ramp', (-1.714, 0, .65), (.030, .71, .43),
      'Armor', .018)
c.box('Ramp upper seam', (-1.736, 0, .81), (.012, .58, .015),
      'Steel', .002)
for y in (-.28, .28):
    c.box('Rear access hinge', (-1.741, y, .49), (.028, .15, .055),
          'Steel', .004)
    c.torus('Front armored towing eye', (1.707, y, .54), .041, .012,
            'Steel', (0, math.pi / 2, 0))
c.box('Lower prow anti impact strip', (1.64, 0, .50), (.13, .91, .07),
      'Dark', .012)

# Driver sits under a flush wedge deck, observing only through small prisms.
for y in (-.25, .25):
    c.box('Flush driver prism housing', (1.02, y, .998),
          (.24, .16, .040), 'Dark', .006, (0, .23, 0))
    c.box('Driver vision slit', (1.116, y, 1.014),
          (.018, .12, .024), 'Glass', .002, (0, .23, 0))
c.box('Broad front deck plate', (1.18, 0, .916),
      (.47, .54, .035), 'Armor', .008, (0, .30, 0))
for y in (-.36, .36):
    c.box('Fender recessed service strip', (.90, y, 1.04),
          (.22, .025, .016), 'Steel', .002)

# Low remote weapon station is wide and rectangular, with two rear sensor pods
# instead of Aegis' taller manned polygonal turret and circular command hatch.
c.cylinder('Remote weapon turret bearing', (-.21, 0, 1.085), .43, .070,
           'Dark', vertices=20)
faceted_shell('Wide flattened unmanned turret', [
    (-.90, .38, 1.12, 1.31), (-.61, .49, 1.10, 1.40),
    (.18, .45, 1.10, 1.35), (.43, .27, 1.15, 1.27),
])
for side in (-1, 1):
    c.box('Separate broad turret slab', (-.30, side * .412, 1.28),
          (.62, .13, .16), 'Armor', .016, (side * -.18, 0, 0))
    c.box('Concept green turret identifier', (-.65, side * .463, 1.25),
          (.15, .012, .047), 'Team', .002)
    for j in range(3):
        c.cylinder('Angled smoke projector', (-.67 + j * .103, side * .43, 1.37),
                   .028, .14, 'Dark', (side * .62, -.17, 0), 10)
c.box('Offset autocannon armored mantlet', (.42, -.055, 1.235),
      (.26, .24, .18), 'Steel', .018)
c.beam('Short Ember autocannon sleeve', (.48, -.055, 1.25),
       (.99, -.055, 1.25), .050, 'Steel')
c.beam('Ember autocannon barrel', (.96, -.055, 1.25),
       (1.53, -.055, 1.25), .030, 'Dark')
c.box('Slotted squared muzzle', (1.57, -.055, 1.25),
      (.14, .084, .08), 'Steel', .005)
c.box('Cannon bore', (1.644, -.055, 1.25),
      (.007, .040, .040), 'Dark', .001)
c.beam('Coaxial light cannon', (.40, .16, 1.22),
       (1.01, .16, 1.22), .016, 'Dark')

for side in (-1, 1):
    c.box('Independent rear sensor pod', (-.57, side * .23, 1.435),
          (.28, .17, .13), 'Dark', .012)
    c.box('Sensor forward glass', (-.422, side * .23, 1.435),
          (.012, .11, .065), 'Glass', .003)
    c.box('Green powered sensor status', (-.414, side * .23, 1.468),
          (.014, .065, .012), 'Neon', .001)
    c.box('Sensor armored visor', (-.47, side * .23, 1.510),
          (.19, .21, .020), 'Armor', .005)
c.box('Flush rectangular turret service hatch', (-.22, 0, 1.408),
      (.35, .32, .025), 'Armor', .007)
for y in (-.27, .27):
    c.beam('Rear antenna mount', (-.85, y, 1.29), (-.85, y, 1.44),
           .018, 'Dark')
    c.beam('Flexible radio antenna', (-.85, y, 1.44),
           (-.92, y, 1.89 if y < 0 else 1.74), .005, 'Steel')

assert len(wheel_objects) == arch_count == 8
objects = [o for o in bpy.context.scene.objects if o.type == 'MESH']
for obj in objects:
    for slot in obj.material_slots:
        if slot.material is None:
            slot.material = bpy.data.materials['Armor']

# Compute ground compensation from the actual tread vertices, not nominal tire
# radius. This also catches a future tread or bevel adjustment.
bpy.context.view_layer.update()
minimum_z = min((obj.matrix_world @ Vector(v)).z
                for obj in objects for v in obj.bound_box)
for obj in objects:
    obj.location.z -= minimum_z
mesh = c.export(NAME, -.35)
metadata = json.loads((OUT / (NAME + '.json')).read_text(encoding='utf-8'))
expected = [
    [min((mesh.matrix_world @ Vector(v))[axis] for v in mesh.bound_box),
     max((mesh.matrix_world @ Vector(v))[axis] for v in mesh.bound_box)]
    for axis in range(3)
]
assert abs(expected[2][0] + .35) < .0001, expected
assert metadata['dimensions_m'][1] <= 1.88, metadata['dimensions_m']
assert 10000 <= metadata['triangles'] <= 35000, metadata['triangles']
metadata.update({
    'wheel_count': 8,
    'actual_boolean_arch_count': arch_count,
    'authoring_direction': 'front +X, wheel axles Y',
    'reference': 'Codex-Bild 1. Sept. 2026, 01_22_20.png, left graphite 8x8',
    'provenance': 'Original procedural geometry; no extracted or downloaded models',
    'geometry_distinction': 'Hexagonal stealth hull, clipped prow, wide remote turret, dual sensors, offset cannon',
    'team_color_decision': 'Concept graphite/green visual identity; enemy red health bars, selection rings and UI remain gameplay identification',
    'concept_surface_tints': {'Team': [.025, .32, .13], 'Neon': [.025, .70, .28]},
    'pivot': 'Actual tire tread Z=-0.35m; runtime actor Z=+35cm and scale=1',
    'ground_correction_m': -minimum_z,
    'bounds_m': expected,
    'build_script_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
    'common_helpers_sha256': hashlib.sha256(Path(c.__file__).read_bytes()).hexdigest(),
})
reference = Path('E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_20.png')
if reference.exists():
    metadata['reference_sha256'] = hashlib.sha256(reference.read_bytes()).hexdigest()

# Preview the exact joined/exported mesh. Validate the FBX independently after
# rendering; the saved .blend is untouched by the temporary studio/reimport.
c.preview(NAME, mesh)
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.fbx(filepath=str(OUT / (NAME + '.fbx')))
imported = [obj for obj in bpy.context.scene.objects if obj.type == 'MESH']
assert len(imported) == 1
obj = imported[0]
actual = [
    [min((obj.matrix_world @ Vector(v))[axis] for v in obj.bound_box),
     max((obj.matrix_world @ Vector(v))[axis] for v in obj.bound_box)]
    for axis in range(3)
]
# The export convention mirrors Y. The outer wheel envelope is symmetrical;
# all extrema must therefore agree even though the upper gun is offset.
assert all(abs(a - b) < .002 for p, q in zip(actual, expected)
           for a, b in zip(p, q)), (expected, actual)
metadata['fbx_roundtrip'] = {'passed': True, 'bounds_m': actual,
                             'mirror_y_checked': True}
metadata['fbx_sha256'] = hashlib.sha256((OUT / (NAME + '.fbx')).read_bytes()).hexdigest()
metadata['validation'] = 'PASS'
(OUT / (NAME + '.json')).write_text(json.dumps(metadata, indent=2), encoding='utf-8')
print('REFERENCE_EMBER_APC_V7_PASS', json.dumps(metadata))
