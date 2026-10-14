"""Original eroded riverbanks and sandstone outcrops, measured in metres."""
from pathlib import Path
import bpy, math, random
from mathutils import noise, Vector
ROOT=Path(__file__).resolve().parents[2]/'Art/Models/EnvironmentV2'
ROOT.mkdir(parents=True,exist_ok=True)
random.seed(81)
def export(name,verts,faces):
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
    obj=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(obj)
    obj.select_set(True);bpy.context.view_layer.objects.active=obj
    bpy.ops.wm.save_as_mainfile(filepath=str(ROOT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(ROOT/(name+'.fbx')),use_selection=True,
        object_types={'MESH'},axis_forward='X',axis_up='Z',mesh_smooth_type='FACE',apply_unit_scale=False)
    print('ENVIRONMENT_EXPORTED',name,len(faces))
ring_verts=[];ring_faces=[]
for i in range(64):
    a=i*math.tau/64
    for r in (.46,.5):ring_verts.append((r*math.cos(a),r*math.sin(a),0))
for i in range(64):
    a=i*2;b=((i+1)%64)*2;ring_faces.append((a,a+1,b+1,b))
export('SM_ENV_SelectionRing',ring_verts,ring_faces)
# Banks occupy only the existing impassable water band. Bridge corridors stay open.
for side in (-1,1):
    verts=[];faces=[];rows=360;cols=10
    for j in range(rows+1):
        y=-90+180*j/rows
        bridge=max(0,min(1,(min(abs(y-32),abs(y+32))-7)/3))
        width=(2.2+.8*math.sin(y*.13)+.5*math.sin(y*.43))*bridge
        for i in range(cols+1):
            t=i/cols;x=-30+side*(7-width*t)
            z=-.08-1.0*t-1.1*t*t+.16*math.sin(y*.8+t*3)*math.sin(t*math.pi)*bridge
            verts.append((x,y,z))
    for j in range(rows):
        for i in range(cols):
            a=j*(cols+1)+i;face=(a,a+1,a+cols+2,a+cols+1)
            faces.append(tuple(reversed(face)) if side==1 else face)
    export('SM_ENV_Bank_'+('East' if side==1 else 'West'),verts,faces)
for variant in range(4):
    verts=[];faces=[];rings=12;sides=32
    for j in range(rings+1):
        t=j/rings;z=t*(3.5+variant*.55)
        radius=(1-t**2.4)*(3.1+.3*variant)+.18
        for i in range(sides):
            a=i*math.tau/sides
            wobble=.84+.14*math.sin(a*3+variant)+.08*math.sin(a*7+j*.6)
            r=radius*wobble*(1+.055*math.sin(j*2.8))
            verts.append((math.cos(a)*r,math.sin(a)*r*.73,z+.08*math.sin(a*5+j)))
    for j in range(rings):
        for i in range(sides):
            a=j*sides+i;b=j*sides+(i+1)%sides;faces.append((a,b,b+sides,a+sides))
    faces.append(tuple(range(rings*sides,(rings+1)*sides)))
    export('SM_ENV_Sandstone_'+str(variant),verts,faces)
