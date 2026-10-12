"""Run in Blender: retain only highest-detail source ATV geometry."""
from pathlib import Path
import bpy
import os

source = Path(os.environ.get("HEROHOUR_LEGACY_ROOT", "E:/HeroHour")) / "Art/Imported/RTSModern/ATV_N1_LE.fbx"
destination = Path(__file__).resolve().parents[2] / "Art/Models/Imported/ATV_N1_LOD0.fbx"
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(source))
meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
for obj in meshes:
    print(f"ATV_SOURCE_OBJECT name={obj.name} vertices={len(obj.data.vertices)} faces={len(obj.data.polygons)} materials={[slot.name for slot in obj.material_slots]}")
highest = [obj for obj in meshes if "LOD0" in obj.name.upper()]
if not highest:
    raise RuntimeError("No explicitly named LOD0 meshes found; refusing ambiguous cleanup")
for obj in meshes:
    if obj not in highest:
        bpy.data.objects.remove(obj, do_unlink=True)
bpy.ops.object.select_all(action="DESELECT")
for obj in highest:
    obj.name = "LOD0_" + obj.name.replace("LOD0_", "")
    obj.select_set(True)
destination.parent.mkdir(parents=True, exist_ok=True)
bpy.ops.export_scene.fbx(filepath=str(destination), use_selection=True, object_types={"MESH"}, bake_anim=False, mesh_smooth_type="FACE", path_mode="AUTO", axis_forward="-Z", axis_up="Y")
print(f"ATV_LOD0_EXPORT_RESULT=PASS meshes={len(highest)} polygons={sum(len(obj.data.polygons) for obj in highest)} file={destination}")
