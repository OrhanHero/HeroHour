import bpy,json
from pathlib import Path
ROOT=Path(__file__).resolve().parent
for name in ['SM_DV3_Civic_0','SM_DV3_Civic_1','SM_DV3_Civic_2']:
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    bpy.ops.import_scene.fbx(filepath=str(ROOT/(name+'.fbx')))
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
    for o in meshes:
        counts=[0,0,0];backwards=0
        for p in o.data.polygons:
            axis=max(range(3),key=lambda j:abs(p.normal[j]));counts[axis]+=1
            if len(p.vertices)==4 and p.area>1 and p.normal.dot(p.center-__import__('mathutils').Vector((0,0,11)))<0:backwards+=1
        print('FBX_ROUNDTRIP',json.dumps({'name':o.name,'dimensions':list(o.dimensions),'polygons':len(o.data.polygons),'face_normal_major_axis_counts_XYZ':counts,'hide_render':o.hide_render,'hide_viewport':o.hide_viewport,'negative_scale':list(o.scale),'material_slots':[m.name for m in o.data.materials],'shapekeys':bool(o.data.shape_keys),'mesh_valid':not o.data.validate()}))
