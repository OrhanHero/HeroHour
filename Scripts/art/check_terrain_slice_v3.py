"""Round-trip the delivered FBX and verify gameplay-critical geometry, in Blender."""
from pathlib import Path
import json
import bpy

PROJECT = Path(__file__).resolve().parents[2]
SOURCE = PROJECT / 'Art/Models/DistrictV3/SM_DV3_Terrain.fbx'
REPORT = PROJECT / 'Saved/Research/terrain-v3-fbx-roundtrip.json'
bpy.ops.object.select_all(action='SELECT')
bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.fbx(filepath=str(SOURCE))
obj = next(o for o in bpy.context.scene.objects if o.type == 'MESH')
vertices = [obj.matrix_world @ v.co for v in obj.data.vertices]
lo = [min(v[i] for v in vertices) for i in range(3)]
hi = [max(v[i] for v in vertices) for i in range(3)]
assert abs(lo[0] + 170) < 1e-4 and abs(hi[0] - 170) < 1e-4
assert abs(lo[1] + 135) < 1e-4 and abs(hi[1] - 135) < 1e-4
ground = [v for v in vertices if abs(v.x) <= 120 and abs(v.y) <= 90
          and (v.x < -37 - 1e-4 or v.x > -23 + 1e-4)]
bed = [v for v in vertices if -37 + 1e-4 < v.x < -23 - 1e-4 and abs(v.y) <= 90]
assert ground and max(abs(v.z + .05) for v in ground) < 1e-5
assert bed and max(abs(v.z + 3.5) for v in bed) < 1e-5
for x in (-37, -23):
    heights = [v.z for v in vertices if abs(v.x - x) < 1e-4 and abs(v.y) < 89]
    assert any(abs(z + .05) < 1e-5 for z in heights)
    assert any(abs(z + 3.5) < 1e-5 for z in heights)
assert len(obj.data.polygons) == 119232
assert all(p.area > 1e-7 for p in obj.data.polygons)
assert [m.name for m in obj.data.materials] == ['Sand', 'Sandstone']
assert obj.data.uv_layers
flat = [p for p in obj.data.polygons if p.center.z > -.051 and p.center.z < -.049]
assert flat and all((obj.matrix_world.to_3x3() @ p.normal).z > .999 for p in flat)
report = {'source': str(SOURCE), 'result': 'PASS', 'bounds_min_m': lo,
          'bounds_max_m': hi, 'triangles': len(obj.data.polygons),
          'flat_ground_vertices_checked': len(ground),
          'river_bed_vertices_checked': len(bed),
          'checks': ['FBX metre scale and X/Y bounds', 'flat playable ground Z',
                     'flat river bed Z', 'vertical cut-bank boundary columns',
                     'nondegenerate triangles', 'two expected material slots',
                     'UV layer', 'upward flat ground normals']}
REPORT.parent.mkdir(parents=True, exist_ok=True)
REPORT.write_text(json.dumps(report, indent=2), encoding='utf-8')
print('TERRAIN_FBX_ROUNDTRIP', json.dumps(report))
