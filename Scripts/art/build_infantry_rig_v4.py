"""Rig and animate original FrontlinesV4 infantry; no external models or motion.

Blender 5.2 --background --python-exit-code 1 --python this_file.py
Produces a rigid-armour skeletal mesh, 14-bone skeleton, Idle/Walk/Fire clips,
an editable source blend, FBX roundtrip evidence and a manifest. The rigid bone
assignment deliberately preserves separate armour silhouettes. It is a basic
in-place prototype rig, not a motion-capture or production character claim.
"""
import json
import math
import sys
from collections import Counter
from pathlib import Path

import bpy
from mathutils import Quaternion, Vector

sys.path.insert(0, str(Path(__file__).resolve().parent))
import build_frontline_assets_v4 as front

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Art/Models/InfantryRigV4"
NAME = "SK_HR_Infantry"
FPS = 30
CLIPS = {"Idle": 2.0, "Walk": .8, "Fire": .3}
MIN_Z = -.35


def part_bone(obj):
    name = obj.name.split(".")[0]
    side = "l" if obj.location.y < 0 else "r"
    if name.startswith("Combat boot"):
        return "foot_" + side
    if name.startswith(("Lower leg", "Shin armored", "Articulated knee")):
        return "shin_" + side
    if name.startswith(("Thigh undersuit", "Thigh armor")):
        return "thigh_" + side
    if name.startswith(("Layered shoulder", "Upper arm", "Shoulder faction")):
        return "upperarm_" + side
    if name.startswith(("Elbow joint", "Armored forearm", "Combat glove")):
        return "forearm_" + side
    if name.startswith("Rifle"):
        return "forearm_r"
    if name.startswith(("Helmet", "Armored helmet", "Visor", "Respirator", "Sealed collar")):
        return "head"
    if name.startswith(("Pelvis", "Body armor lower belt", "Side ammunition pouch")):
        return "hip"
    return "spine"


def normalize_geometry(parts):
    points = [obj.matrix_world @ vertex.co for obj in parts for vertex in obj.data.vertices]
    low = Vector(tuple(min(p[axis] for p in points) for axis in range(3)))
    high = Vector(tuple(max(p[axis] for p in points) for axis in range(3)))
    size = high - low
    desired = Vector((.9, .7, 1.8))

    def transform(point):
        return Vector(((point.x - (low.x + high.x) * .5) * desired.x / size.x,
                       (point.y - (low.y + high.y) * .5) * desired.y / size.y,
                       (point.z - low.z) * desired.z / size.z + MIN_Z))

    assignments = {}
    for obj in parts:
        # Capture side from authoring coordinates before all object pivots become origin.
        assignments[obj.name] = part_bone(obj)
        world_matrix = obj.matrix_world.copy()
        for vertex in obj.data.vertices:
            vertex.co = transform(world_matrix @ vertex.co)
        obj.location = (0, 0, 0)
        obj.rotation_euler = (0, 0, 0)
        obj.scale = (1, 1, 1)
        obj.data.update()
    return transform, assignments


def create_armature(transform):
    data = bpy.data.armatures.new("SK_HR_Infantry_Skeleton")
    arm = bpy.data.objects.new("Armature", data)
    bpy.context.collection.objects.link(arm)
    bpy.context.view_layer.objects.active = arm
    arm.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    bones = {}

    def add(name, head, tail, parent=None, source_coords=True):
        bone = data.edit_bones.new(name)
        bone.head = transform(Vector(head)) if source_coords else Vector(head)
        bone.tail = transform(Vector(tail)) if source_coords else Vector(tail)
        bone.use_deform = True
        if parent:
            bone.parent = bones[parent]
        bones[name] = bone

    add("root", (0, 0, 0), (0, 0, .15), source_coords=False)
    add("hip", (-.055, 0, 1.03), (-.043, 0, 1.19), "root")
    add("spine", (-.043, 0, 1.19), (-.040, 0, 1.57), "hip")
    add("head", (-.040, 0, 1.58), (-.026, 0, 1.80), "spine")
    for side, y in (("l", -.125), ("r", .125)):
        add("thigh_" + side, (-.080, y, 1.01), (.027, y, .69), "hip")
        add("shin_" + side, (.027, y, .69), (-.012, y, .20), "thigh_" + side)
        add("foot_" + side, (-.012, y, .20), (.17, y, .085), "shin_" + side)
        shoulder_y = -.239 if side == "l" else .239
        elbow_y = -.220 if side == "l" else .220
        hand_x = .47 if side == "l" else .37
        hand_y = -.08 if side == "l" else .08
        add("upperarm_" + side, (-.02, shoulder_y, 1.49), (.15, elbow_y, 1.25), "spine")
        add("forearm_" + side, (.15, elbow_y, 1.25), (hand_x, hand_y, 1.24), "upperarm_" + side)
    bpy.ops.object.mode_set(mode="OBJECT")
    for bone in arm.pose.bones:
        bone.rotation_mode = "QUATERNION"
    arm.show_in_front = True
    return arm


def join_weighted_mesh(parts, assignments, arm):
    counts = Counter()
    bpy.ops.object.select_all(action="DESELECT")
    for obj in parts:
        name = assignments[obj.name]
        group = obj.vertex_groups.new(name=name)
        group.add(list(range(len(obj.data.vertices))), 1.0, "REPLACE")
        counts[name] += len(obj.data.vertices)
        obj.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    mesh = bpy.context.object
    mesh.name = NAME
    mesh.parent = arm
    modifier = mesh.modifiers.new("HeroHour original armour rig", "ARMATURE")
    modifier.object = arm
    bpy.context.view_layer.objects.active = mesh
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=math.radians(66), island_margin=.008)
    bpy.ops.object.mode_set(mode="OBJECT")
    invalid = []
    for vertex in mesh.data.vertices:
        weights = [group.weight for group in vertex.groups]
        if len(weights) != 1 or abs(sum(weights) - 1.0) > .000001:
            invalid.append(vertex.index)
    assert not invalid, ("bad bone weights", invalid[:20])
    assert all(mesh.vertex_groups[g.group].name in arm.data.bones for v in mesh.data.vertices for g in v.groups)
    return mesh, dict(counts)


def select_rig(mesh, arm, include_mesh=True):
    bpy.ops.object.select_all(action="DESELECT")
    arm.select_set(True)
    if include_mesh:
        mesh.select_set(True)
    bpy.context.view_layer.objects.active = arm


def rotate_world_y(pose_bone, angle):
    local_axis = pose_bone.bone.matrix_local.to_3x3().inverted() @ Vector((0, 1, 0))
    pose_bone.rotation_quaternion = Quaternion(local_axis.normalized(), angle)


def create_clip(arm, clip, duration):
    scene = bpy.context.scene
    action = bpy.data.actions.new("AN_HR_Infantry_" + clip)
    action.use_fake_user = True
    arm.animation_data_create()
    arm.animation_data.action = action
    end = int(round(duration * FPS)) + 1
    scene.frame_start, scene.frame_end = 1, end
    for frame in range(1, end + 1):
        t = (frame - 1) / (end - 1)
        scene.frame_set(frame)
        for bone in arm.pose.bones:
            bone.location = (0, 0, 0)
            bone.rotation_quaternion = Quaternion((1, 0, 0, 0))
            bone.scale = (1, 1, 1)
        if clip == "Idle":
            breath = math.sin(t * math.tau)
            arm.pose.bones["spine"].scale.y = 1 + .012 * breath
            rotate_world_y(arm.pose.bones["spine"], .009 * breath)
            rotate_world_y(arm.pose.bones["head"], -.006 * breath)
        elif clip == "Walk":
            phase = t * math.tau
            arm.pose.bones["hip"].location.z = .012 * (1 - math.cos(phase * 2))
            for side, shift in (("l", 0), ("r", math.pi)):
                swing = math.sin(phase + shift)
                thigh = .39 * swing
                shin = .62 * max(0.0, -swing)
                rotate_world_y(arm.pose.bones["thigh_" + side], thigh)
                rotate_world_y(arm.pose.bones["shin_" + side], shin)
                rotate_world_y(arm.pose.bones["foot_" + side], -thigh - shin)
                # Both hands keep the weapon; shoulder movement stays restrained.
                rotate_world_y(arm.pose.bones["upperarm_" + side], .026 * math.sin(phase))
                rotate_world_y(arm.pose.bones["forearm_" + side], -.020 * math.sin(phase))
            rotate_world_y(arm.pose.bones["spine"], .026 * math.sin(phase))
        else:
            recoil = math.sin(math.pi * min(t / .30, 1.0)) if t < .30 else 0.0
            rotate_world_y(arm.pose.bones["spine"], -.040 * recoil)
            for side in ("l", "r"):
                rotate_world_y(arm.pose.bones["upperarm_" + side], -.025 * recoil)
                rotate_world_y(arm.pose.bones["forearm_" + side], -.065 * recoil)
            rotate_world_y(arm.pose.bones["head"], .025 * recoil)
        for bone in arm.pose.bones:
            bone.keyframe_insert("location", frame=frame)
            bone.keyframe_insert("rotation_quaternion", frame=frame)
            bone.keyframe_insert("scale", frame=frame)
    return action


def export_fbx(path, with_animation):
    bpy.ops.export_scene.fbx(filepath=str(path), use_selection=True,
        object_types={"MESH", "ARMATURE"} if not with_animation else {"ARMATURE"},
        axis_forward="X", axis_up="Z", global_scale=1, apply_unit_scale=False,
        add_leaf_bones=False, use_armature_deform_only=True, armature_nodetype="NULL",
        primary_bone_axis="Y", secondary_bone_axis="X", mesh_smooth_type="FACE",
        bake_anim=with_animation, bake_anim_use_all_actions=False,
        bake_anim_use_nla_strips=False, bake_anim_force_startend_keying=True,
        bake_anim_step=1, bake_anim_simplify_factor=0)


def roundtrip(mesh, arm, clips):
    before = set(bpy.context.scene.objects)
    bpy.ops.import_scene.fbx(filepath=str(OUT / (NAME + ".fbx")))
    imported = [obj for obj in bpy.context.scene.objects if obj not in before]
    meshes = [obj for obj in imported if obj.type == "MESH"]
    arms = [obj for obj in imported if obj.type == "ARMATURE"]
    assert len(meshes) == len(arms) == 1, "roundtrip missing mesh/armature"
    other, skeleton = meshes[0], arms[0]
    expected_bones = sorted(b.name for b in arm.data.bones)
    actual_bones = sorted(b.name for b in skeleton.data.bones)
    assert actual_bones == expected_bones, (actual_bones, expected_bones)
    points = [other.matrix_world @ Vector(v) for v in other.bound_box]
    sizes = [max(p[a] for p in points) - min(p[a] for p in points) for a in range(3)]
    for actual, expected in zip(sizes, (.9, .7, 1.8)):
        assert abs(actual - expected) < .001, (sizes, expected)
    assert abs(min(p.z for p in points) + .35) < .001
    assert len(other.data.vertices) > 0
    for vertex in other.data.vertices:
        total = sum(group.weight for group in vertex.groups)
        assert abs(total - 1.0) < .00001, ("roundtrip weight", vertex.index, total)
        assert all(other.vertex_groups[g.group].name in expected_bones for g in vertex.groups)
    for obj in imported:
        bpy.data.objects.remove(obj, do_unlink=True)
    clip_results = []
    for clip, duration in clips.items():
        before_objects = set(bpy.context.scene.objects)
        before_actions = set(bpy.data.actions)
        bpy.ops.import_scene.fbx(filepath=str(OUT / ("AN_HR_Infantry_" + clip + ".fbx")))
        imported_objects = [obj for obj in bpy.context.scene.objects if obj not in before_objects]
        imported_actions = [action for action in bpy.data.actions if action not in before_actions]
        assert imported_actions, "No imported animation: " + clip
        ranges = [(float(action.frame_range[0]), float(action.frame_range[1])) for action in imported_actions]
        measured = max((end - start) / FPS for start, end in ranges)
        assert abs(measured - duration) < .001, (clip, measured, duration, ranges)
        # Compare actual deformed limb/bone transforms, not just metadata or clip names.
        imported_arm = next(obj for obj in imported_objects if obj.type == "ARMATURE")
        bpy.context.scene.frame_set(round(ranges[0][0]))
        start_matrix = imported_arm.pose.bones["thigh_l" if clip == "Walk" else "spine"].matrix.copy()
        bpy.context.scene.frame_set(round(ranges[0][0] + (ranges[0][1] - ranges[0][0]) * .25))
        middle_matrix = imported_arm.pose.bones["thigh_l" if clip == "Walk" else "spine"].matrix.copy()
        difference = max(abs(start_matrix[i][j] - middle_matrix[i][j]) for i in range(4) for j in range(4))
        assert difference > .0001, (clip, "animation is static", difference)
        clip_results.append(dict(name="AN_HR_Infantry_" + clip, seconds=measured,
                                 animated_transform_delta=difference, roundtrip_passed=True))
        for obj in imported_objects:
            bpy.data.objects.remove(obj, do_unlink=True)
        for action in imported_actions:
            bpy.data.actions.remove(action)
    return dict(passed=True, dimensions_m=sizes, min_z_m=min(p.z for p in points),
                bones=actual_bones, weights_normalized=True, clips=clip_results)


def main():
    OUT.mkdir(parents=True, exist_ok=True)
    front.reset()
    front.infantry()
    parts = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    transform, assignments = normalize_geometry(parts)
    arm = create_armature(transform)
    mesh, weights = join_weighted_mesh(parts, assignments, arm)
    scene = bpy.context.scene
    scene.render.fps = FPS
    scene.render.fps_base = 1
    select_rig(mesh, arm)
    export_fbx(OUT / (NAME + ".fbx"), False)
    for clip, duration in CLIPS.items():
        create_clip(arm, clip, duration)
        select_rig(mesh, arm, False)
        export_fbx(OUT / ("AN_HR_Infantry_" + clip + ".fbx"), True)
    arm.animation_data.action = bpy.data.actions["AN_HR_Infantry_Idle"]
    scene.frame_start, scene.frame_end = 1, 61
    scene.frame_set(1)
    select_rig(mesh, arm)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT / (NAME + ".blend")))
    result = roundtrip(mesh, arm, CLIPS)
    metadata = dict(name=NAME, dimensions_m=[.9, .7, 1.8], min_z_m=MIN_Z,
        runtime_actor_z_cm=35, runtime_scale=1, fps=FPS, bone_count=len(arm.data.bones),
        triangles=sum(len(p.vertices) - 2 for p in mesh.data.polygons),
        material_slots=[m.name for m in mesh.data.materials], vertices_per_bone=weights,
        original_geometry=True, original_keyframes=True, rigid_armour_weights=True,
        clips=CLIPS, fbx_roundtrip=result)
    (OUT / "InfantryRigV4-manifest.json").write_text(json.dumps(metadata, indent=2), encoding="utf-8")
    print("INFANTRY_RIG_V4_COMPLETE", json.dumps(metadata), flush=True)


if __name__ == "__main__":
    main()
