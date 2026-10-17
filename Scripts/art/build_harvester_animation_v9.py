"""Split original HeroHour harvester geometry into genuine moving parts.

CPU-only Blender authoring. Never executes the original script's export/render
tail and never modifies its source or previously exported assets.
"""
import ast
import hashlib
import json
import math
import sys
from pathlib import Path

import bmesh
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / 'Scripts/art/build_concept_harvester.py'
OUT = ROOT / 'Art/Models/HarvesterAnimationV9'
sys.path.insert(0, str(SOURCE.parent))
import concept_asset_common as common


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def facts(objects):
    points = [obj.matrix_world @ vertex.co for obj in objects for vertex in obj.data.vertices]
    return dict(triangles=sum(len(poly.vertices)-2 for obj in objects for poly in obj.data.polygons),
                vertices=sum(len(obj.data.vertices) for obj in objects),
                bounds_min_m=[min(p[i] for p in points) for i in range(3)],
                bounds_max_m=[max(p[i] for p in points) for i in range(3)])


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    source_sha = sha(SOURCE)
    # Execute imports, declarations and function definitions only. The original
    # module's first common.reset() starts its mutating/exporting main program.
    tree = ast.parse(SOURCE.read_text(encoding='utf-8'))
    safe = []
    for node in tree.body:
        if isinstance(node, ast.Expr) and isinstance(node.value, ast.Call):
            target = node.value.func
            if isinstance(target, ast.Attribute) and isinstance(target.value, ast.Name) and target.value.id == 'common' and target.attr == 'reset':
                break
        safe.append(node)
    namespace = {'__file__': str(SOURCE), '__name__': 'harvester_source_definitions'}
    exec(compile(ast.Module(body=safe, type_ignores=[]), str(SOURCE), 'exec'), namespace)
    common.reset()
    bpy.context.preferences.filepaths.save_version = 0
    namespace['create_geometry']()
    objects = [o for o in bpy.context.scene.objects if o.type == 'MESH']
    floor_z = min((o.matrix_world @ v.co).z for o in objects for v in o.data.vertices)
    for obj in objects:
        obj.location.z -= floor_z + .35
        bpy.context.view_layer.objects.active = obj
        bm = bmesh.new()
        bm.from_mesh(obj.data)
        bmesh.ops.dissolve_degenerate(bm, dist=1e-7, edges=list(bm.edges))
        bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
        bm.to_mesh(obj.data)
        bm.free()
        mod = obj.modifiers.new('Explicit triangles', 'TRIANGULATE')
        bpy.ops.object.modifier_apply(modifier=mod.name)
    full = facts(objects)
    assert abs(full['bounds_min_m'][2] + .35) < 1e-5
    height_offset = -floor_z - .35
    arm_pivot = Vector((1.15, 0, .88 + height_offset))
    drum_pivot = Vector((1.77, 0, .495 + height_offset))
    coupler_pivot = Vector((-1.839, 0, 1.18 + height_offset))
    groups = {key: [] for key in ['Base', 'ArmAssembly', 'CutterDrum', 'RearCoupler'] + [f'Gauge{i}' for i in range(7)]}
    arm_names = {'Mining lift arm', 'Lift hydraulic cylinder', 'Lift hydraulic piston',
                 'Drum drive shroud', 'Cutter bearing hub', 'Cutter armored debris visor',
                 'Debris visor identification stripe'}
    drum_names = {'Rotary harvesting drum', 'Drum serrated collar', 'Replaceable cutter tooth'}
    coupler_names = {'Unload sealing boot', 'Unload port interior', 'Unload indexing rim'}
    assignments = []
    for obj in objects:
        label = obj.name.split('.')[0]
        key = 'Base'
        if label in arm_names:
            key = 'ArmAssembly'
        elif label in drum_names:
            key = 'CutterDrum'
        elif label in coupler_names:
            key = 'RearCoupler'
        elif label == 'Resource fill gauge segment':
            key = f'Gauge{round((obj.location.x + .935) / .068)}'
        assert key in groups, (obj.name, key)
        groups[key].append(obj)
        assignments.append(dict(source_object=obj.name, part=key, triangles=len(obj.data.polygons)))
    assert sum(len(group) for group in groups.values()) == len(objects)
    assert all(groups.values())
    manifest = dict(schema=1, provenance='Original HeroHour geometry; no external game meshes',
                    source=str(SOURCE.relative_to(ROOT)), source_sha256=source_sha,
                    material_source_sha256=sha(Path(common.__file__)),
                    full_original=full, assignments=assignments, parts=[],
                    units='metres in FBX; centimetres in Unreal', forward='+X',
                    unreal_y_reflection=True, original_anchor_offset_m=-.35,
                    animation='Own presentation only; no Tempest timing claim')
    combined_min = [float('inf')]*3
    combined_max = [-float('inf')]*3
    for key, group in groups.items():
        before = facts(group)
        for i in range(3):
            combined_min[i] = min(combined_min[i], before['bounds_min_m'][i])
            combined_max[i] = max(combined_max[i], before['bounds_max_m'][i])
        pivot = arm_pivot if key == 'ArmAssembly' else drum_pivot if key == 'CutterDrum' else coupler_pivot if key == 'RearCoupler' else Vector((0,0,0))
        bpy.ops.object.select_all(action='DESELECT')
        for obj in group:
            obj.select_set(True)
        bpy.context.view_layer.objects.active = group[0]
        bpy.ops.object.join()
        obj = bpy.context.object
        name = 'SM_HA9_' + key
        obj.name = name
        bpy.context.scene.cursor.location = pivot
        bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
        # Bake the active source object's rotation as well: a beam or drum can
        # be the join's active object, but exported part axes must remain +X/Z.
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
        obj.location = (0,0,0)
        bpy.ops.object.mode_set(mode='EDIT')
        bpy.ops.mesh.select_all(action='SELECT')
        bpy.ops.uv.smart_project(angle_limit=math.radians(66), island_margin=.012)
        bpy.ops.object.mode_set(mode='OBJECT')
        assert all(len(poly.vertices)==3 for poly in obj.data.polygons)
        assert all(math.isfinite(x) for value in obj.data.uv_layers.active.data for x in value.uv)
        path = OUT / (name + '.fbx')
        bpy.ops.export_scene.fbx(filepath=str(path), use_selection=True, object_types={'MESH'},
            axis_forward='X', axis_up='Z', global_scale=1, apply_unit_scale=False,
            add_leaf_bones=False, mesh_smooth_type='FACE')
        entry = dict(name=name, key=key, pivot_actor_m=list(pivot),
                     parent='ArmAssembly' if key=='CutterDrum' else 'Actor',
                     pivot_parent_m=list(pivot-arm_pivot) if key=='CutterDrum' else list(pivot),
                     material_slots=[m.name for m in obj.data.materials],
                     fbx_sha256=sha(path), **facts([obj]))
        # Roundtrip each FBX before accepting the part. Restore original named
        # material references later in Unreal, never rely on FBX auto materials.
        bpy.ops.import_scene.fbx(filepath=str(path))
        imported=[o for o in bpy.context.selected_objects if o.type=='MESH']
        roundtrip=facts(imported)
        assert roundtrip['triangles']==entry['triangles'], key
        for bound in ('bounds_min_m','bounds_max_m'):
            assert max(abs(a-b) for a,b in zip(entry[bound],roundtrip[bound]))<1e-5, (key,bound)
        entry['fbx_roundtrip']='PASS'
        for imported_obj in imported:
            bpy.data.objects.remove(imported_obj, do_unlink=True)
        obj.location=pivot
        manifest['parts'].append(entry)
    assert sum(p['triangles'] for p in manifest['parts'])==full['triangles']
    assert max(abs(a-b) for a,b in zip(combined_min,full['bounds_min_m']))<1e-6
    assert max(abs(a-b) for a,b in zip(combined_max,full['bounds_max_m']))<1e-6
    assert sha(SOURCE)==source_sha
    manifest['validation']='PASS: partition, triangle conservation, bounds, UV, FBX roundtrip'
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'HarvesterAnimationV9.blend'))
    (OUT/'manifest.json').write_text(json.dumps(manifest,indent=2),encoding='utf-8')
    print('HARVESTER_ANIMATION_V9_SOURCE_PASS',full['triangles'],len(manifest['parts']))


if __name__ == '__main__':
    main()
