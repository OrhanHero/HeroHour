"""Prepare proportional, grounded FBX from own verified PBR GLB. No UE edits."""
import bpy,hashlib,json,math,shutil
from pathlib import Path
from mathutils import Matrix,Vector
ROOT=Path(__file__).resolve().parents[2];SOURCE=ROOT/'Art/Models/TrellisV8-Repaired/AegisHQ-Remeshed512-PBR2048_00001_.glb'
EXPECTED='802747bb8124a02ad0c0f899f4a2dc5b9578f23bff12ea534c28d3b5cd201f03'
OUT=ROOT/'Art/Models/TrellisHQV9';TEX=ROOT/'Art/Textures/TrellisHQV9';OUT.mkdir(exist_ok=True);TEX.mkdir(exist_ok=True)
NAME='SM_HQ9_AegisHQ';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();assert sha(SOURCE)==EXPECTED
bpy.ops.wm.read_factory_settings(use_empty=True);bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1.
bpy.ops.import_scene.gltf(filepath=str(SOURCE));meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(meshes)==1
obj=meshes[0];obj.name=NAME;obj.data.name=NAME
points=[obj.matrix_world@v.co for v in obj.data.vertices];low=Vector([min(p[i] for p in points) for i in range(3)]);high=Vector([max(p[i] for p in points) for i in range(3)])
center=(low+high)*.5;scale=7.8/max((high-low).x,(high-low).y)
# Source door faces Blender -Y. Bake 180deg yaw to +Y; FBX->UE reflects Y, yielding UE -Y.
transform=Matrix.Translation((0,0,-1.1))@Matrix.Scale(scale,4)@Matrix.Rotation(math.pi,4,'Z')@Matrix.Translation((-center.x,-center.y,-low.z))
obj.matrix_world=transform@obj.matrix_world
bpy.context.view_layer.objects.active=obj;obj.select_set(True);bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
assert len(obj.data.materials)==1;material=obj.data.materials[0];material.name='Team'
textures={}
for semantic,source_name,dest_name,srgb in [('Albedo','BaseColor.png','T_HQ9_AegisHQ_Albedo.png',True),('MetallicRoughness','MetallicRoughness.png','T_HQ9_AegisHQ_MR.png',False)]:
 src=SOURCE.parent/source_name;target=TEX/dest_name
 if target.exists():assert target.read_bytes()==src.read_bytes()
 else:shutil.copyfile(src,target)
 textures[semantic]={'file':str(target.relative_to(ROOT)),'sha256':sha(target),'size':[2048,2048],'srgb':srgb}
for node in material.node_tree.nodes:
 if node.bl_idname=='ShaderNodeTexImage' and node.image:
  srgb=node.image.colorspace_settings.name=='sRGB'
  image=bpy.data.images.load(str(TEX/('T_HQ9_AegisHQ_Albedo.png' if srgb else 'T_HQ9_AegisHQ_MR.png')),check_existing=True)
  image.colorspace_settings.name='sRGB' if srgb else 'Non-Color';node.image=image
points=[v.co for v in obj.data.vertices];bounds=[[min(p[i] for p in points),max(p[i] for p in points)] for i in range(3)]
assert abs(bounds[2][0]+1.1)<1e-5;assert abs(max(b-a for a,b in bounds[:2])-7.8)<1e-4
assert len(obj.data.uv_layers)==1 and obj.data.has_custom_normals
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(NAME+'.blend')))
bpy.ops.export_scene.fbx(filepath=str(OUT/(NAME+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE',use_tspace=True)
bpy.ops.object.select_all(action='DESELECT');bpy.ops.import_scene.fbx(filepath=str(OUT/(NAME+'.fbx')))
round_objects=[o for o in bpy.context.selected_objects if o.type=='MESH'];assert len(round_objects)==1
points=[o.matrix_world@v.co for o in round_objects for v in o.data.vertices];round_bounds=[[min(p[i] for p in points),max(p[i] for p in points)] for i in range(3)]
assert all(abs(a-b)<.001 for pair,other in zip(bounds,round_bounds) for a,b in zip(pair,other)),(bounds,round_bounds)
assert [m.name.split('.')[0] for m in round_objects[0].data.materials]==['Team']
meta={'name':NAME,'source':str(SOURCE.relative_to(ROOT)),'source_sha256':EXPECTED,'source_unchanged':sha(SOURCE)==EXPECTED,'source_bounds_blender_units':[list(low),list(high)],'uniform_scale_m_per_source_unit':scale,'baked_yaw_degrees':180,'transform_matrix_rows':[list(row) for row in transform],'bounds_m':bounds,'dimensions_m':[b-a for a,b in bounds],'zoffset_m':-1.1,'unreal_expected_bounds_cm':[[a*100,b*100] if i!=1 else [-b*100,-a*100] for i,(a,b) in enumerate(bounds)],'fbx_roundtrip_bounds_m':round_bounds,'fbx_roundtrip_pass':True,'fbx_sha256':sha(OUT/(NAME+'.fbx')),'blend_sha256':sha(OUT/(NAME+'.blend')),'vertices':len(obj.data.vertices),'triangles':len(obj.data.polygons),'material_slots':['Team'],'uv_channels':1,'custom_normals_preserved':True,'runtime_scale':1.5,'runtime_ground_z_cm':110+55-110*1.5,'blender_front_normal':[0,1,0],'unreal_front_normal':[0,-1,0],'textures':textures,'provenance':'Original HeroHour concept-derived generated image + native TRELLIS2 + proven remesh repair; no extracted Tempest art','limits':['No new LOD acceptance','No emissive mask, normalmap or measured AO','No UnitDefinition, runtime source or map change in preparation']}
(OUT/(NAME+'.json')).write_text(json.dumps(meta,indent=2),encoding='utf-8');print('TRELLIS_HQ_V9_PREPARE_PASS',json.dumps(meta),flush=True)
