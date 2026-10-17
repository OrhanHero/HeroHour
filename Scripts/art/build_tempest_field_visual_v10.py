"""Original low-cost organic root/pod geometry; CPU FBX export, no render or UE."""
import bpy,math,json,hashlib
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'Art/Models/TempestFieldV10';OUT.mkdir(exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True);bpy.context.preferences.filepaths.save_version=0
bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1
records=[]
def export(name,vertices,faces):
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(vertices,[],faces);mesh.update();obj=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(obj)
 mat=bpy.data.materials.new('Organic');mat.diffuse_color=(.16,.09,.035,1);mesh.materials.append(mat)
 for face in mesh.polygons:face.use_smooth=True
 bpy.ops.object.select_all(action='DESELECT');obj.select_set(True);bpy.context.view_layer.objects.active=obj
 bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
 records.append({'name':name,'vertices':len(mesh.vertices),'triangles':sum(len(p.vertices)-2 for p in mesh.polygons),'bounds_m':[[min(v.co[i] for v in mesh.vertices),max(v.co[i] for v in mesh.vertices)] for i in range(3)],'fbx_sha256':hashlib.sha256((OUT/(name+'.fbx')).read_bytes()).hexdigest()})
# Nominal X length100cm. Bulged, asymmetric and internally curved, not a uniform metal cylinder.
vertices=[];faces=[];rings=9;sides=8
for ring in range(rings):
 t=ring/(rings-1);radius=.052*(.74+.24*math.sin(t*math.pi)+.12*math.sin(t*13))
 center=Vector((t-.5,math.sin(t*math.pi*2)*.024,math.sin(t*math.pi)*.018))
 for side in range(sides):
  a=side*math.tau/sides;vertices.append(center+Vector((0,math.cos(a)*radius*(1+.10*math.sin(ring+side)),math.sin(a)*radius*.72)))
for ring in range(rings-1):
 for side in range(sides):
  a=ring*sides+side;b=ring*sides+(side+1)%sides;faces.append((a,b,b+sides,a+sides))
faces.extend([tuple(reversed(range(sides))),tuple((rings-1)*sides+i for i in range(sides))])
export('SM_TFV10_OrganicRootSegment',vertices,faces)
# A low asymmetric teardrop/bud, approximately90cm high before runtime downscale.
vertices=[];faces=[];rings=8;sides=9
for ring in range(rings):
 t=ring/(rings-1);r=max(.008,math.sin(t*math.pi)**.65*.27*(1-.28*t));z=t*.88
 for side in range(sides):
  a=side*math.tau/sides;vertices.append((math.cos(a)*r+.075*t*t,math.sin(a)*r*.79,z+.018*math.sin(a*3)*math.sin(t*math.pi)))
for ring in range(rings-1):
 for side in range(sides):
  a=ring*sides+side;b=ring*sides+(side+1)%sides;faces.append((a,b,b+sides,a+sides))
faces.extend([tuple(reversed(range(sides))),tuple((rings-1)*sides+i for i in range(sides))])
export('SM_TFV10_SeedPod',vertices,faces)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'TempestFieldV10.blend'))
report={'provenance':'Original mathematically authored organic forms; no copied/extracted Tempest Rising assets','runtime_contract':'Root X100cm; 3 curved segments per logical edge; Pod origin at ground; NoCollision/noNav set by presenter','records':records,'rendered':False,'visual_acceptance':False}
(OUT/'manifest.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print('TEMPEST_FIELD_V10_GEOMETRY_PASS',json.dumps(report),flush=True)
