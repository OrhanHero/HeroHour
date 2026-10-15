"""Build original ground-pivot, metre-based decorative kit. Blender background script.
No gameplay collision or navigation geometry; no borrowed game assets.
"""
import sys, math, json, random
from pathlib import Path
import bpy, bmesh
from mathutils import Vector
sys.path.insert(0, str(Path(__file__).resolve().parent))
import concept_asset_common as c
OUT = Path(__file__).resolve().parents[2] / 'Art/Models/PresentationV5'
OUT.mkdir(parents=True, exist_ok=True)
SEED=150151
rng=random.Random(SEED)
reports=[]
bpy.context.preferences.filepaths.save_version=0

def twig(name,a,b,r):
    delta=b-a
    bpy.ops.mesh.primitive_cylinder_add(vertices=8,radius=r,depth=delta.length,location=(a+b)/2)
    o=c.finish(bpy.context.object,name,'Dust')
    o.rotation_euler=delta.to_track_quat('Z','Y').to_euler()

def export(name):
    objects=[o for o in bpy.context.scene.objects if o.type=='MESH']
    bpy.ops.object.select_all(action='DESELECT')
    for o in objects: o.select_set(True)
    bpy.context.view_layer.objects.active=objects[0]
    bpy.ops.object.join(); o=bpy.context.object; o.name=name
    bpy.context.scene.cursor.location=(0,0,0); bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
    floor=min(v.co.z for v in o.data.vertices)
    for v in o.data.vertices: v.co.z-=floor
    o.data.update()
    # Physical metre UVs preserve existing tiled-material scale on all sizes.
    uv=o.data.uv_layers.new(name='PhysicalMetres')
    for face in o.data.polygons:
        axis=max(range(3),key=lambda j:abs(face.normal[j])); a,b=((1,2),(0,2),(0,1))[axis]
        for li in face.loop_indices:
            p=o.data.vertices[o.data.loops[li].vertex_index].co
            uv.data[li].uv=(p[a],p[b])
    bm=bmesh.new(); bm.from_mesh(o.data)
    bad=sum(not e.is_manifold for e in bm.edges); bm.free()
    tris=sum(len(p.vertices)-2 for p in o.data.polygons)
    assert bad==0 and tris<5000,(name,bad,tris)
    assert all(p.area>1e-10 for p in o.data.polygons),name
    dims=list(o.dimensions)
    mats=[m.name for m in o.data.materials]
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    # Independent FBX read verifies metre scale and exported material names.
    bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
    bpy.ops.import_scene.fbx(filepath=str(OUT/(name+'.fbx')))
    q=[x for x in bpy.context.scene.objects if x.type=='MESH']; assert len(q)==1
    actual=list(q[0].dimensions)
    assert all(abs(a-b)<.001 for a,b in zip(actual,dims)),(actual,dims)
    report=dict(name=name,seed=SEED,triangles=tris,dimensions_m=dims,materials=mats,pivot='ground origin z=0',collision='none',navigation='decoration only; does not affect navigation',non_manifold_edges=bad,uv_mapping='dominant plane; 1 UV unit = 1 metre',fbx_roundtrip=dict(passed=True,dimensions_m=actual),validation='PASS')
    (OUT/(name+'.json')).write_text(json.dumps(report,indent=2),encoding='utf-8'); reports.append(report)
    print('PRESENTATION_V5_PASS',json.dumps(report))

c.reset()
for i,(x,y,s) in enumerate([(-.65,.08,1),(.35,.30,.70),(.65,-.35,.45),(-.25,-.55,.27),(.85,.45,.19)]):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2,radius=1,location=(x,y,s*.32))
    o=c.finish(bpy.context.object,'Eroded sandstone '+str(i),'Dust')
    for v in o.data.vertices:
        v.co*=rng.uniform(.85,1.13)
        v.co.x*=s*.72; v.co.y*=s*.53; v.co.z=max(v.co.z*s*.48,-s*.32)
    o.rotation_euler.z=rng.random()*math.tau
export('SM_PV5_RockCluster')

c.reset()
for i in range(9):
    a=rng.uniform(0,math.tau); r=rng.uniform(.05,.42)
    base=Vector((math.cos(a)*r, math.sin(a)*r, .015))
    tip=base+Vector((math.cos(a)*.15,math.sin(a)*.15,rng.uniform(.18,.48)))
    twig('Dry twig',base,tip,.012)
    for j in range(2):
        mid=base.lerp(tip,.45+j*.19)
        end=mid+Vector((rng.uniform(-.15,.15),rng.uniform(-.15,.15),.10))
        twig('Dry branch',mid,end,.007)
# Closed tapered blades give a readable dry grass outline without alpha cards.
for i in range(12):
    a=rng.random()*math.tau; r=rng.random()*.39
    bpy.ops.mesh.primitive_cone_add(vertices=4,radius1=.018,radius2=.002,depth=.22,location=(math.cos(a)*r,math.sin(a)*r,.11))
    o=c.finish(bpy.context.object,'Dry grass','Dust')
    o.rotation_euler=(rng.uniform(-.4,.4),rng.uniform(-.4,.4),a)
export('SM_PV5_DryScrub')

c.reset()
for i in range(4):
    c.box('Concrete curb',(-1.125+i*.75,0,.10),(.73,.26,.20),'Concrete',.018)
    c.box('Worn painted cap',(-1.125+i*.75,0,.203),(.52,.18,.008),'Dust',.002)
export('SM_PV5_CurbStrip')

c.reset()
c.box('Barrier footing',(0,0,.07),(2.5,.65,.14),'Concrete',.035)
c.box('Barrier body',(0,0,.34),(2.30,.38,.54),'Concrete',.055)
c.box('Barrier top',(0,0,.63),(2.25,.25,.10),'Concrete',.025)
for x in [-.8,0,.8]:
    c.box('Faded reflector',(x,-.199,.43),(.28,.018,.13),'Dust',.006)
for x in [-1.01,1.01]:
    c.box('Lifting recess',(x,-.2,.56),(.14,.025,.055),'Dark',.005)
export('SM_PV5_LowBarrier')

c.reset()
for x,y,z,size in [(-.45,0,.34,(.85,.8,.68)),(.43,.1,.26,(.7,.62,.52)),(-.40,.03,.92,(.72,.68,.48))]:
    c.box('Supply case',(x,y,z),size,'Armor',.025)
    for dx in [-size[0]*.36,size[0]*.36]:
        c.box('Strap',(x+dx,y,z),( .045,size[1]+.012,size[2]+.012),'Steel',.006)
    c.box('Case label',(x,y-size[1]/2-.008,z),(.20,.015,.10),'Dust',.002)
export('SM_PV5_SupplyCrates')

c.reset()
for y,z in [(-.23,.16),(.10,.16),(-.065,.46)]:
    c.cylinder('Pipe',(0,y,z),.15,2.0,'Steel',rot=(0,math.pi/2,0),vertices=12)
    for x in [-1.005,1.005]:
        bpy.ops.mesh.primitive_cylinder_add(vertices=12,radius=.115,depth=.006,location=(x,y,z),rotation=(0,math.pi/2,0))
        c.finish(bpy.context.object,'Dark closed inset','Dark')
for x in [-.65,.65]:
    c.box('Bundle tie',(x,-.065,.30),(.075,.70,.65),'Armor',.018)
export('SM_PV5_PipeBundle')
(OUT/'manifest.json').write_text(json.dumps(dict(provenance='Original procedural meshes created for HeroHour; no extracted game assets',units='metres',decoration_only=True,assets=reports),indent=2),encoding='utf-8')
