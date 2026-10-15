"""Original concept-led desert downtown architecture; run in Blender background.
All facade relief is real geometry. No original-game asset extraction or downloads.
"""
import bpy, bmesh, math, json
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'Art/Models/ConceptCityV6'; OUT.mkdir(parents=True,exist_ok=True)
COLORS={'Plaster':(.39,.36,.30),'Concrete':(.24,.25,.24),'Glass':(.024,.075,.085),'Metal':(.115,.14,.14),'Paint':(.24,.31,.29)}
bpy.context.preferences.filepaths.save_version=0
reports=[]

def box(p,s,mat='Concrete'):
    # Direct mesh buffering avoids thousands of Blender operators and retains
    # closed disconnected solids (architectural assembly, no fake thin planes).
    k=len(verts); x,y,z=p; a,b,c=[v/2 for v in s]
    verts.extend([(x+dx*a,y+dy*b,z+dz*c) for dx,dy,dz in [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]])
    for face in [(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)]:
        faces.append(tuple(k+i for i in face)); indices.append(list(COLORS).index(mat))

def parapet(z,w,d,cx=0,cy=0):
    for x in [-w/2,w/2]:box((cx+x,cy,z+.18),(.10,d,.36),'Plaster')
    for y in [-d/2,d/2]:box((cx,cy+y,z+.18),(w,.10,.36),'Plaster')
    box((cx,cy,z-.05),(w,d,.10),'Concrete')

def windows(z,w,d,cols=4,cx=0,cy=0,balcony=False):
    for axis,extent,along in [(1,d,w),(0,w,d)]:
        for side in [-1,1]:
            for i in range(cols):
                q=(i-(cols-1)/2)*along/cols; ww=along/cols*.62
                def item(u,out,h,su,so,sz,mat):
                    box((cx+u,cy+out,h) if axis==1 else (cx+out,cy+u,h),(su,so,sz) if axis==1 else (so,su,sz),mat)
                # A deep dark reveal sits behind projecting lintels/jambs/sills.
                outer=side*(extent/2+.025)
                item(q,outer,z,ww+.10,.05,1.32,'Metal')
                item(q,outer+side*.025,z,ww-.06,.035,1.15,'Glass')
                for u in [q-ww/2,q+ww/2]:item(u,outer+side*.055,z,.055,.14,1.42,'Concrete')
                for h in [z-.69,z+.69]:item(q,outer+side*.07,h,ww+.12,.18,.075,'Plaster')
                item(q,outer+side*.06,z,.035,.12,1.22,'Metal')
                if balcony and axis==1 and side==-1 and i%2==0:
                    item(q,outer-.18,z-.76,ww+.2,.50,.09,'Concrete')
                    item(q,outer-.40,z-.42,ww+.18,.065,.62,'Paint')

def hvac(x,y,z):
    box((x,y,z+.23),(.65,.55,.46),'Metal')
    box((x,y,z+.49),(.71,.61,.07),'Concrete')
    for j in range(6):box((x-.25+j*.10,y-.287,z+.24),(.035,.025,.31),'Concrete')
    # Two orthogonal ducts are highly visible at RTS camera angles.
    box((x+.45,y,z+.12),(.28,.20,.24),'Metal')
    box((x+.57,y+.25,z+.12),(.17,.50,.24),'Metal')

def create(kind):
    global verts,faces,indices
    verts=[]; faces=[];indices=[]
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    for name,col in COLORS.items():
        m=bpy.data.materials.get(name) or bpy.data.materials.new(name);m.diffuse_color=(*col,1);m.use_nodes=True
        bs=m.node_tree.nodes.get('Principled BSDF');bs.inputs['Base Color'].default_value=(*col,1);bs.inputs['Roughness'].default_value=.26 if name=='Glass' else .77
    floors=[4,6,9,5][kind];w=3.45;d=3.45; top=2.4+floors*1.75
    box((0,0,.10),(3.90,3.90,.20))
    box((0,0,1.3),(w,d,2.4),'Plaster')
    # Arcade supports and glazed shop fronts on all four sides.
    for x in [-1.5,0,1.5]:
        box((x,-1.77,1.23),(.18,.22,2.36),'Concrete')
    for x in [-.76,.76]:
        box((x,-1.74,1.23),(1.23,.065,1.90),'Glass')
        box((x,-1.88,2.18),(1.27,.20,.18),'Paint')
    box((0,-1.84,.20),(3.7,.18,.20),'Concrete')
    box((0,0,(2.4+top)/2),(w,d,top-2.4),'Plaster')
    for floor in range(floors):
        z=3.22+floor*1.75
        windows(z,w,d,4,balcony=kind in (0,3))
        box((0,0,z-.85),(w+.14,d+.14,.12),'Concrete')
    # Vertical corner piers and differently proportioned rooftop silhouettes.
    for x in [-1.75,1.75]:
        for y in [-1.75,1.75]:box((x,y,(2.4+top)/2),(.12,.12,top-2.4),'Concrete')
    parapet(top,w+.10,d+.10)
    if kind==2:
        box((.30,.25,top+1.1),(2.25,2.25,2.2),'Plaster');windows(top+1.1,2.25,2.25,3,.30,.25)
        parapet(top+2.25,2.30,2.30,.30,.25);roof=top+2.25
        hvac(.0,.0,roof);box((.85,.85,roof+1),(.055,.055,2.0),'Metal')
    else:
        box((-.65,.65,top+.65),(1.25,1.25,1.3),'Plaster');parapet(top+1.3,1.3,1.3,-.65,.65)
        hvac(.55,-.55,top);hvac(-.55,-.60,top)
        box((1.20,.85,top+.5),(.16,.16,1.0),'Metal')
    if kind==3:
        # Exposed service stair/buttress reads differently from repetitive slabs.
        for floor in range(floors):
            z=2.75+floor*1.75
            box((1.83,.30,z),(.18,1.6,.12),'Metal')
            box((1.85,.30,z+.31),(.055,1.6,.60),'Paint')
    name=['SM_CC6_Courtyard','SM_CC6_Office','SM_CC6_Tower','SM_CC6_Service'][kind]
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update();o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o)
    for mat in COLORS:mesh.materials.append(bpy.data.materials[mat])
    for p,i in zip(mesh.polygons,indices):p.material_index=i
    uv=mesh.uv_layers.new(name='PhysicalMetres')
    for p in mesh.polygons:
        axis=max(range(3),key=lambda j:abs(p.normal[j]));a,b=((1,2),(0,2),(0,1))[axis]
        for li in p.loop_indices:
            co=mesh.vertices[mesh.loops[li].vertex_index].co;uv.data[li].uv=(co[a],co[b])
    bm=bmesh.new();bm.from_mesh(mesh);bad=sum(not e.is_manifold for e in bm.edges);bm.free()
    assert bad==0
    tris=sum(len(p.vertices)-2 for p in mesh.polygons);assert tris<16000
    dims=list(o.dimensions); assert dims[0]<=4.15 and dims[1]<=4.15
    radius=max(math.hypot(v.co.x*200,v.co.y*200) for v in mesh.vertices)
    assert radius<600, radius
    bpy.context.view_layer.objects.active=o;o.select_set(True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False);bpy.ops.import_scene.fbx(filepath=str(OUT/(name+'.fbx')))
    imported=[x for x in bpy.context.scene.objects if x.type=='MESH'];assert len(imported)==1
    assert all(abs(a-b)<.001 for a,b in zip(dims,imported[0].dimensions))
    # Render the actual FBX round-trip geometry at the existing map's XY scale.
    imported[0].scale=(2,2,1)
    scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=16
    scene.render.resolution_x=512;scene.render.resolution_y=640;scene.render.resolution_percentage=100
    scene.world.color=(.25,.25,.25)
    bpy.ops.object.light_add(type='AREA',location=(-10,-14,25));bpy.context.object.data.energy=2800;bpy.context.object.data.shape='DISK';bpy.context.object.data.size=12
    bpy.ops.object.light_add(type='AREA',location=(10,4,18));bpy.context.object.data.energy=1800;bpy.context.object.data.size=10
    target=Vector((0,0,dims[2]*.48));bpy.ops.object.camera_add(location=target+Vector((20,-28,22)))
    camera=bpy.context.object;camera.rotation_euler=(target-camera.location).to_track_quat('-Z','Y').to_euler();camera.data.type='ORTHO';camera.data.ortho_scale=max(16,dims[2]*1.4);scene.camera=camera
    scene.render.image_settings.file_format='PNG';scene.render.film_transparent=True;scene.render.filepath=str(OUT/(name+'-Preview.png'));bpy.ops.render.render(write_still=True)
    report=dict(name=name,triangles=tris,dimensions_m=dims,non_manifold_edges=bad,materials=list(COLORS),validation='PASS',fbx_roundtrip=True,pivot='ground origin',floors=floors,footprint_radius_at_map_scale_cm=radius,uv='1 UV unit = 1 metre',source='original authored geometry; concept images observed')
    (OUT/(name+'.json')).write_text(json.dumps(report,indent=2));reports.append(report)
    print('CONCEPT_CITY_V6_ASSET_PASS',json.dumps(report))

for kind in range(4):create(kind)
(OUT/'manifest.json').write_text(json.dumps(dict(assets=reports,replacement_count=28,collision='keep existing gameplay city blocker radius 600 cm; visual meshes NoCollision'),indent=2))
print('CONCEPT_CITY_V6_BUILD_PASS',len(reports))
