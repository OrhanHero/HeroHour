"""Re-export retained source meshes with explicit FBX smoothing groups."""
import bpy
from pathlib import Path
root=Path(__file__).resolve().parents[2]/'Art/Models/ConceptV2'
for source in sorted(root.glob('SM_HERO_*.blend')):
    bpy.ops.wm.open_mainfile(filepath=str(source))
    mesh=bpy.data.objects.get(source.stem)
    assert mesh and mesh.type=='MESH',str(source)
    bpy.ops.object.select_all(action='DESELECT');mesh.select_set(True);bpy.context.view_layer.objects.active=mesh
    bpy.ops.export_scene.fbx(filepath=str(source.with_suffix('.fbx')),use_selection=True,object_types={'MESH'},
        axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    print('SMOOTHING_EXPORT_PASS',source.stem)
