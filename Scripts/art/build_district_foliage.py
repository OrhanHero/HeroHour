"""Sparse original desert groundcover; no extracted textures or meshes."""
import bpy,math,json,random
from pathlib import Path
from mathutils import Vector
OUT=Path(__file__).resolve().parents[2]/'Art/Models/DistrictV3'
def reset():
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1
    for name,col in [('Foliage',(.25,.28,.12)),('Dust',(.38,.28,.13))]:
        m=bpy.data.materials.get(name) or bpy.data.materials.new(name);m.use_nodes=True;m.diffuse_color=(*col,1)
        p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*col,1);p.inputs['Roughness'].default_value=.92
def cone(a,b,r1,r2):
    a,b=Vector(a),Vector(b);d=b-a;bpy.ops.mesh.primitive_cone_add(vertices=6,radius1=r1,radius2=r2,depth=d.length,location=(a+b)/2)
    o=bpy.context.object;o.rotation_euler=d.to_track_quat('Z','Y').to_euler();o.data.materials.append(bpy.data.materials['Dust'])
def leaves(center,theta,scale):
    # Each cluster is a star of tapered narrow leaves, actual geometry rather than a solid blob.
    c=Vector(center);verts=[];faces=[]
    for k in range(7):
        ang=theta+k*2.399;v=Vector((math.cos(ang),math.sin(ang),.25+(.2 if k%2 else -.12)))
        side=Vector((-math.sin(ang),math.cos(ang),0))*scale*.11
        mid=c+v*scale*.5;tip=c+v*scale
        n=len(verts);verts.extend([tuple(c),tuple(mid+side),tuple(tip),tuple(mid-side),tuple(mid+Vector((0,0,.025)))])
        faces.extend([(n,n+1,n+4),(n+1,n+2,n+4),(n+2,n+3,n+4),(n+3,n,n+4)])
    me=bpy.data.meshes.new('Tapered sparse leaf cluster');me.from_pydata(verts,[],faces);me.materials.append(bpy.data.materials['Foliage']);o=bpy.data.objects.new('Leaves',me);bpy.context.collection.objects.link(o)
def export(name):
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];bpy.ops.object.select_all(action='DESELECT')
    for o in meshes:o.select_set(True)
    bpy.context.view_layer.objects.active=meshes[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
    bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR');bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.cube_project(cube_size=1);bpy.ops.object.mode_set(mode='OBJECT')
    old=[m.name for m in o.data.materials];names=['Foliage','Dust'];idx=[names.index(old[p.material_index]) for p in o.data.polygons];o.data.materials.clear()
    for n in names:o.data.materials.append(bpy.data.materials[n])
    for p,i in zip(o.data.polygons,idx):p.material_index=i
    OUT.mkdir(parents=True,exist_ok=True);bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    r={'name':name,'dimensions_m':list(o.dimensions),'triangles':sum(len(p.vertices)-2 for p in o.data.polygons),'materials':names,'pivot':'ground origin','bounds_min_m':[min(v.co[j] for v in o.data.vertices) for j in range(3)],'bounds_max_m':[max(v.co[j] for v in o.data.vertices) for j in range(3)]}
    (OUT/(name+'.json')).write_text(json.dumps(r,indent=2));print('FOLIAGE_EXPORTED',json.dumps(r))
reset();rng=random.Random(437)
cone((0,0,0),(0,0,.30),.026,.014)
for k in range(9):
    ang=k*2.399;radius=.29+rng.random()*.10;tip=(math.cos(ang)*radius,math.sin(ang)*radius,.37+rng.random()*.15)
    cone((0,0,.15),tip,.012,.004);leaves(tip,ang,.19+rng.random()*.06)
    mid=(tip[0]*.58,tip[1]*.58,tip[2]*.77);leaves(mid,ang+.9,.14)
leaves((0,0,.59),.4,.17);export('SM_DV3_DesertShrub')
reset();verts=[];faces=[];rng=random.Random(938)
for k in range(42):
    ang=rng.random()*math.tau;r=rng.random()*.13;base=Vector((math.cos(ang)*r,math.sin(ang)*r,0))
    height=.23+rng.random()*.22;bend=.08+rng.random()*.14;side=Vector((-math.sin(ang),math.cos(ang),0))*(.008+rng.random()*.012)
    centers=[base,base+Vector((math.cos(ang)*bend*.2,math.sin(ang)*bend*.2,height*.45)),base+Vector((math.cos(ang)*bend*.65,math.sin(ang)*bend*.65,height*.82)),base+Vector((math.cos(ang)*bend,math.sin(ang)*bend,height))]
    n=len(verts)
    for i,c in enumerate(centers[:3]):verts.extend([tuple(c+side*(1-i*.27)),tuple(c-side*(1-i*.27))])
    verts.append(tuple(centers[3]));faces.extend([(n,n+1,n+3,n+2),(n+2,n+3,n+5,n+4),(n+4,n+5,n+6)])
me=bpy.data.meshes.new('Curved tapered grass blades');me.from_pydata(verts,[],faces);me.materials.append(bpy.data.materials['Foliage']);o=bpy.data.objects.new('Grass',me);bpy.context.collection.objects.link(o);export('SM_DV3_DryGrass')
