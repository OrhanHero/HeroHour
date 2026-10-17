"""CPU-only Blender LOD candidates from stable remeshed geometry, no source edit."""
import bpy,hashlib,json,time
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'Art/Models/TrellisV8-Diagnostic'
SOURCE=OUT/'REMESHED_00001_.glb';before=hashlib.sha256(SOURCE.read_bytes()).hexdigest();start=time.monotonic();results=[]
bpy.ops.wm.read_factory_settings(use_empty=True);bpy.ops.import_scene.gltf(filepath=str(SOURCE))
source=next(o for o in bpy.context.scene.objects if o.type=='MESH');source.hide_render=True
for target in [150000,100000]:
 obj=source.copy();obj.data=source.data.copy();bpy.context.collection.objects.link(obj);obj.hide_render=False
 bpy.context.view_layer.objects.active=obj;obj.select_set(True);source.select_set(False)
 modifier=obj.modifiers.new('Independent CPU LOD trial','DECIMATE');modifier.ratio=target/len(source.data.polygons);modifier.use_collapse_triangulate=True
 bpy.ops.object.modifier_apply(modifier=modifier.name)
 path=OUT/f'LOD-Blender-{target}.glb';bpy.ops.export_scene.gltf(filepath=str(path),use_selection=True,export_format='GLB',export_normals=True,export_materials='NONE')
 results.append({'file':path.name,'vertices':len(obj.data.vertices),'faces':len(obj.data.polygons),'sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'algorithm':'Blender Collapse Decimate on already remeshed input, not native Comfy QEM'})
 bpy.data.objects.remove(obj,do_unlink=True)
assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==before
report={'source_sha256':before,'source_unchanged':True,'results':results,'cpu_only_no_render':True,'elapsed_seconds':time.monotonic()-start,'visual_acceptance':False}
(ROOT/'docs/verification/2026-10-16/reference-v8/trellis/cpu-lod-candidates.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print('TRELLIS_CPU_LOD_PASS',json.dumps(report),flush=True)
