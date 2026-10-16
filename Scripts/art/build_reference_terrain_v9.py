"""Original stratified talus kit, CPU geometry only; no Unreal or rendering.

All substantial geometry stays outside authoritative walkable land. Existing
terrain is ray sampled, not modified. Root owns subsequent Unreal integration.
"""
import hashlib
import json
import math
import random
from pathlib import Path
import bpy
import bmesh
from mathutils import Vector
from mathutils.bvhtree import BVHTree

ROOT=Path(__file__).resolve().parents[2]
# Reuse audited material/UV functions without executing the V8 model/render loop.
helper=ROOT/'Scripts/art/build_reference_baseyard_v8.py'
scope={'__file__':str(helper),'__name__':'terrain_v9_shared_helpers'}
exec(compile(helper.read_text(encoding='utf-8').split('\nreports=[]')[0],str(helper),'exec'),scope)
OUT=ROOT/'Art/Models/ReferenceTerrainV9';OUT.mkdir(parents=True,exist_ok=True)
MATERIALS={
 'Concrete':dict(source='Sand',tint=(1.02,.95,.83),roughness=1.,metallic=0),
 'ConcreteDark':dict(source='Sand',tint=(.92,.86,.75),roughness=1.,metallic=0),
 'Metal':dict(source='Sand',tint=(.98,.91,.80),roughness=1.,metallic=0),
 'Dust':dict(source='CompactedDirt',tint=(1.06,1.01,.92),roughness=.97,metallic=0.,feather_ground=True),
 'Track':dict(source='Sand',tint=(1.04,.97,.86),roughness=1.,metallic=0),
 'Paint':dict(source='Sand',tint=(1.01,.95,.82),roughness=1.,metallic=0),
}
RNG=random.Random(9162406)
sources=[]
scope['clear']()
for folder,name in [('DistrictV3','SM_DV3_Terrain')]+[
 ('ConceptTerrainV6','SM_CTV6_'+suffix) for suffix in
 ('CanyonWest','CanyonEast','RiverMarginWest','RiverMarginEast')]:
 path=ROOT/'Art/Models'/folder/(name+'.blend')
 with bpy.data.libraries.load(str(path),link=False) as (_,data):data.objects=[name]
 o=data.objects[0];assert o;bpy.context.collection.objects.link(o)
 sources.append((o,hashlib.sha256(path.read_bytes()).hexdigest(),str(path.relative_to(ROOT))))
bpy.context.view_layer.update()
terrain_trees=[BVHTree.FromPolygons([v.co for v in o.data.vertices],[list(p.vertices) for p in o.data.polygons]) for o,_,_ in sources]

def height(x,y):
 hits=[]
 for tree in terrain_trees:
  p,_,_,_=tree.ray_cast(Vector((x,-y,100)),Vector((0,0,-1)),200)
  if p is not None:hits.append(p.z)
 assert hits,(x,y)
 return max(hits)

def safe(x,y):
 return abs(x)>=120.6 or abs(y)>=90.6 or (-36.8<=x<=-23.2 and min(abs(y-32),abs(y+32))>=10.6)

def make_group(name,centers,origin,side):
 verts=[];faces=[];mats=[];alphas=[];pieces=[]
 def rock(cx,cy,width,depth,h,angle,kind):
  # Four irregular terraces give sedimentary ledges rather than smooth spheres.
  n=24;start=len(verts);rings=[(-.12,1.02),(.19,.95),(.43,.83),(.69,.65),(.88,.34)]
  ground=height(cx,cy)
  offsets=[RNG.uniform(.78,1.13) for _ in range(n)]
  erosion=[RNG.uniform(-.12,.12) for _ in range(n)]
  for layer,(z,r) in enumerate(rings):
   for j in range(n):
    a=j*math.tau/n;rx=math.cos(a)*width*.5*r*offsets[j];ry=math.sin(a)*depth*.5*r*offsets[j]
    x=cx+rx*math.cos(angle)-ry*math.sin(angle);y=cy+rx*math.sin(angle)+ry*math.cos(angle)
    assert safe(x,y),(name,x,y)
    # Ground-contact ring follows actual underlying terrain at each vertex.
    zz=height(x,y)-.10 if layer==0 else height(x,y)+(z+erosion[j])*h
    verts.append((x-origin[0],-(y-origin[1]),zz));alphas.append(1.)
  faces.append(tuple(start+j for j in reversed(range(n))));mats.append(1)
  for layer in range(len(rings)-1):
   for j in range(n):
    a=start+layer*n+j;b=start+layer*n+(j+1)%n
    faces.append((a,b,b+n,a+n));mats.append(RNG.choice([0,0,0,1,2,5]))
  faces.append(tuple(start+(len(rings)-1)*n+j for j in range(n)));mats.append(4)
  pieces.append(dict(kind=kind,center_ue_m=[cx,cy,ground],dimensions_m=[width,depth,h],angle=angle))
 for cx,cy,angle in centers:
  river=side.startswith('River')
  for j in range(3):
   rock(cx+math.cos(angle)*j*1.15,cy+math.sin(angle)*j*1.15,3.8-j*.45,1.1 if river else 1.8,1.15-j*.16,angle,'stratified_embankment')
  for j in range(9):
   a=RNG.random()*math.tau;r=RNG.uniform(.9,2.2);size=RNG.uniform(.35,.95)
   if river:
    size=RNG.uniform(.3,.6)
    rock(cx+RNG.uniform(-.2,.2),cy+math.sin(a)*r,size,size*.7,size*.6,a,'talus_boulder')
   else:rock(cx+math.cos(a)*r,cy+math.sin(a)*r,size,size*.7,size*.6,a,'talus_boulder')
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
 obj=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(obj)
 for key,info in MATERIALS.items():mesh.materials.append(scope['material'](key,info))
 for p,index in zip(mesh.polygons,mats):p.material_index=index
 colors=mesh.color_attributes.new(name='BY8_EdgeBlend',type='FLOAT_COLOR',domain='CORNER')
 for c in colors.data:c.color=(1,1,1,1)
 scope['physical_uv'](mesh)
 bm=bmesh.new();bm.from_mesh(mesh);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(mesh)
 nonmanifold=sum(not e.is_manifold for e in bm.edges);bm.free()
 degenerate=sum(p.area<1e-10 for p in mesh.polygons);assert not nonmanifold and not degenerate
 tri=sum(len(p.vertices)-2 for p in mesh.polygons)
 bounds=[[min(v.co[i] for v in mesh.vertices),max(v.co[i] for v in mesh.vertices)] for i in range(3)]
 for o in bpy.context.selected_objects:o.select_set(False)
 obj.select_set(True);bpy.context.view_layer.objects.active=obj
 # Save sources without the read-only raycast context objects.
 for o,_,_ in sources:o.hide_viewport=True;o.hide_render=True
 bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
 bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,mesh_smooth_type='FACE')
 obj.select_set(False);bpy.ops.import_scene.fbx(filepath=str(OUT/(name+'.fbx')))
 loaded=[o for o in bpy.context.selected_objects if o.type=='MESH'];assert len(loaded)==1
 rb=[[min(v.co[i] for v in loaded[0].data.vertices),max(v.co[i] for v in loaded[0].data.vertices)] for i in range(3)]
 assert all(abs(a-b)<.002 for pair,other in zip(bounds,rb) for a,b in zip(pair,other))
 for o in loaded:bpy.data.objects.remove(o,do_unlink=True)
 record=dict(name=name,side=side,triangles=tri,pieces=pieces,materials=list(MATERIALS),
  validation='PASS',fbx_roundtrip_pass=True,non_manifold_edges=0,degenerate_faces=0,
  source_bounds_m=bounds,expected_ue_bounds_cm=[[a*100,b*100] if i!=1 else [-b*100,-a*100] for i,(a,b) in enumerate(bounds)],
  actor_location_cm=[origin[0]*100,origin[1]*100,0],fbx_sha256=hashlib.sha256((OUT/(name+'.fbx')).read_bytes()).hexdigest(),
  collision=False,casts_shadow=False,affects_navigation=False,all_vertices_outside_walkable_land=True)
 (OUT/(name+'.json')).write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
 bpy.data.objects.remove(obj,do_unlink=True)
 return record

reports=[]
for name,centers,origin,label in [
 ('SM_TRV9_RiverNorth',[(-36,8,math.pi/2),(-36,14,math.pi/2),(-24,8,math.pi/2),(-24,14,math.pi/2)],(-30,11),'RiverNorth'),
 ('SM_TRV9_RiverSouth',[(-36,-8,-math.pi/2),(-36,-14,-math.pi/2),(-24,-8,-math.pi/2),(-24,-14,-math.pi/2)],(-30,-11),'RiverSouth'),
 ('SM_TRV9_WestTalus',[(-124,-18,math.pi/2),(-124,-10,math.pi/2),(-124,-2,math.pi/2),(-124,6,math.pi/2)],(-124,0),'WestTalus'),
 ('SM_TRV9_EastTalus',[(124,-18,math.pi/2),(124,-10,math.pi/2),(124,-2,math.pi/2),(124,6,math.pi/2)],(124,0),'EastTalus')]:
 reports.append(make_group(name,centers,origin,label))
assert sum(r['triangles'] for r in reports)<=50000
manifest=dict(generator='build_reference_terrain_v9.py',assets=reports,materials=MATERIALS,
 intended_actors=4,shared_material_instances=6,source_terrain=[dict(path=p,sha256=h) for _,h,p in sources],
 layout_sha256=hashlib.sha256((ROOT/'Source/HeroHourPrototype/BattlefieldLayout.h').read_bytes()).hexdigest(),
 cached_layout_audit_sha256=scope['AUDIT_HASH'],
 provenance='Original layered procedural HeroHour rock geometry and own existing PBR textures; no commercial game assets',
 limits='Significant rocks only outside walkable land; no cure for empty buildable interior sand; source build does not execute previews or Unreal imports')
(OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
print('REFERENCE_TERRAIN_V9_BUILD_PASS '+json.dumps(dict(actors=4,triangles=sum(r['triangles'] for r in reports))),flush=True)
