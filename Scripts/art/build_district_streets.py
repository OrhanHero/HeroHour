"""Original road, base apron and approach modules; Blender metres, UE centimetres."""
from pathlib import Path
import bpy,math,json
ROOT=Path(__file__).resolve().parents[2]/'Art/Models/DistrictV3';ROOT.mkdir(parents=True,exist_ok=True)
MATS={}
for name,col in [('Asphalt',(.12,.13,.125)),('Concrete',(.37,.35,.30)),('Paint',(.65,.58,.38)),('Metal',(.1,.12,.13)),('Glass',(.1,.22,.2))]:
    m=bpy.data.materials.new(name);m.diffuse_color=(*col,1);MATS[name]=m
def clear():bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
def box(name,pos,size,mat,bevel=0):
    bpy.ops.mesh.primitive_cube_add(size=1,location=pos);o=bpy.context.object;o.name=name;o.dimensions=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(MATS[mat])
    if bevel:
        b=o.modifiers.new('Edge wear profile','BEVEL');b.width=bevel;b.segments=2;bpy.ops.object.modifier_apply(modifier=b.name)
    return o
def export(name):
    objects=[o for o in bpy.context.scene.objects if o.type=='MESH'];bpy.ops.object.select_all(action='DESELECT')
    for o in objects:o.select_set(True)
    bpy.context.view_layer.objects.active=objects[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
    bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.cube_project(cube_size=2);bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(ROOT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',apply_unit_scale=False,mesh_smooth_type='FACE')
    (ROOT/(name+'.json')).write_text(json.dumps({'dimensions_m':list(o.dimensions),'material_slots':[m.name for m in o.data.materials],'triangles':sum(len(p.vertices)-2 for p in o.data.polygons)},indent=2))
clear()
# An 18m apron has open approaches, shallow curbs and visual service markings.
box('Base asphalt',(0,0,.015),(18,18,.03),'Asphalt')
for side in (-1,1):
    for end in (-1,1):
        box('Apron raised edge',(side*8.85,end*6.0,.13),(.28,5.8,.26),'Concrete',.045)
        box('Apron raised edge',(end*6.0,side*8.85,.13),(5.8,.28,.26),'Concrete',.045)
    for x in (-7,-5,-3,3,5,7):box('Parking bay stripe',(x,side*7.2,.04),(.09,2.6,.02),'Paint')
    box('Service lane line',(0,side*5.7,.04),(16,.075,.02),'Paint')
for x in (-8,8):
    for y in (-8,8):
        box('Concrete service pocket',(x,y,.07),(1.1,1.1,.14),'Concrete',.04)
        box('Access bollard',(x,y,.5),(.16,.16,.85),'Metal',.03)
export('SM_DV3_BaseApron')
clear()
box('Industrial service pad',(0,0,.02),(9,9,.04),'Asphalt')
for x in (-3.9,3.9):box('Pad rim',(x,0,.07),(.14,8,.14),'Concrete',.025)
for y in (-3.9,3.9):box('Pad rim',(0,y,.07),(8,.14,.14),'Concrete',.025)
for x in (-3.3,3.3):
    for y in (-3.3,3.3):box('Warning corner',(x,y,.045),(.7,.08,.02),'Paint')
export('SM_DV3_ServicePad')
clear()
box('Road surface',(0,0,.02),(12,9,.04),'Asphalt')
for side in (-1,1):
    box('Footpath',(0,side*5.15,.09),(12,1.25,.18),'Concrete',.025)
    box('Kerb',(0,side*4.56,.15),(12,.12,.30),'Concrete',.025)
    box('Roadside stripe',(0,side*4.0,.045),(12,.08,.015),'Paint')
for x in (-4,0,4):box('Centre dash',(x,0,.045),(1.8,.095,.015),'Paint')
export('SM_DV3_RoadStraight')
clear()
# Small sidewalk module: expansion joints and curb, no collidable game obstruction.
for x in (-2,-.8,.4,1.6):box('Crosswalk stripe',(x,0,.035),(.6,5.6,.025),'Paint')
export('SM_DV3_Crosswalk')
clear()
box('Concrete cable trench',(0,0,.045),(2.2,.6,.09),'Concrete',.015)
for x in (-.9,-.6,-.3,0,.3,.6,.9):box('Drain grate',(x,0,.1),(.045,.4,.025),'Metal')
export('SM_DV3_Drain')
clear()
# 15m city pitch minus the 3.8m junction gives an 11.2m street segment.
box('Urban asphalt',(0,0,.02),(11.2,3.8,.04),'Asphalt')
for side in (-1,1):
    box('Urban sidewalk',(0,side*2.62,.09),(11.2,1.28,.18),'Concrete',.025)
    box('Urban kerb',(0,side*1.97,.13),(11.2,.13,.26),'Concrete',.025)
    box('Urban edge line',(0,side*1.63,.04),(11.2,.055,.012),'Paint')
for x in (-3.8,0,3.8):box('Urban lane dash',(x,0,.04),(1.25,.07,.012),'Paint')
export('SM_DV3_UrbanStreet')
clear()
# Curved service approach, local to the western HQ. The broad playable ground stays flat.
points=[]
p0=(0,-32);p1=(0,-23);p2=(-4,-15);p3=(0,-9)
for i in range(81):
    t=i/80;u=1-t
    points.append((u**3*p0[0]+3*u*u*t*p1[0]+3*u*t*t*p2[0]+t**3*p3[0],u**3*p0[1]+3*u*u*t*p1[1]+3*u*t*t*p2[1]+t**3*p3[1]))
def ribbon(offset,width,z,mat):
    verts=[];faces=[]
    for i,(x,y) in enumerate(points):
        a=points[max(0,i-1)];b=points[min(len(points)-1,i+1)]
        dx=b[0]-a[0];dy=b[1]-a[1];length=math.hypot(dx,dy);nx=dy/length;ny=-dx/length
        for w in (offset-width/2,offset+width/2):verts.append((x+nx*w,y+ny*w,z))
    for i in range(len(points)-1):faces.append((i*2,i*2+1,i*2+3,i*2+2))
    mesh=bpy.data.meshes.new('Curved '+mat);mesh.from_pydata(verts,[],faces);mesh.update()
    o=bpy.data.objects.new('Curved '+mat,mesh);bpy.context.collection.objects.link(o);o.data.materials.append(MATS[mat])
ribbon(0,6,.025,'Asphalt')
for side in (-1,1):
    ribbon(side*3.6,1.05,.12,'Concrete');ribbon(side*2.7,.065,.04,'Paint')
for i in range(0,80):
    a=points[i];b=points[i+1];dx=b[0]-a[0];dy=b[1]-a[1];length=math.hypot(dx,dy);nx=dy/length;ny=-dx/length
    for side in (-1,1):
        o=box('Curve kerb',((a[0]+b[0])/2+nx*side*3.06,(a[1]+b[1])/2+ny*side*3.06,.14),(length+.015,.13,.25),'Concrete',.02)
        o.rotation_euler.z=math.atan2(dy,dx)
    if i%12==0:
        o=box('Curve centre dash',((a[0]+b[0])/2,(a[1]+b[1])/2,.04),(1.0,.08,.015),'Paint')
        o.rotation_euler.z=math.atan2(dy,dx)
export('SM_DV3_BaseApproach')
print('DISTRICT_STREETS_EXPORTED')
