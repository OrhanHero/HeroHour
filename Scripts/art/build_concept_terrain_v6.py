"""Original stratified canyon escarpments and eroded river margins; Blender 5.2.
All vertices stay outside the navigable board or inside the blocked river strip.
No third-party meshes, no changed gameplay heights, no generated collision.
"""
from pathlib import Path
import bpy,bmesh,json,math,random,hashlib,struct
from mathutils import Vector,noise
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'Art/Models/ConceptTerrainV6';OUT.mkdir(parents=True,exist_ok=True)
bpy.context.preferences.filepaths.save_version=0
records=[]
def normal_preview(obj,suffix):
    scene=bpy.context.scene;scene.render.engine='BLENDER_WORKBENCH'
    scene.display.shading.light='STUDIO';scene.display.shading.color_type='MATERIAL'
    scene.display.shading.show_shadows=True;scene.display.shading.show_cavity=False
    scene.render.resolution_x=1024;scene.render.resolution_y=640;scene.render.resolution_percentage=100
    scene.render.film_transparent=False;scene.world.color=(.08,.10,.12)
    target=Vector((65,108,12))
    bpy.ops.object.camera_add(location=target+Vector((35,-70,45)))
    camera=bpy.context.object;camera.rotation_euler=(target-camera.location).to_track_quat('-Z','Y').to_euler()
    camera.data.type='ORTHO';camera.data.ortho_scale=112;scene.camera=camera
    scene.render.image_settings.file_format='PNG';scene.render.filepath=str(OUT/('CanyonNorth-Normals'+suffix+'.png'))
    bpy.ops.render.render(write_still=True)
    bpy.data.objects.remove(camera,do_unlink=True)
    bpy.context.view_layer.objects.active=obj;obj.select_set(True)

def weathered_normals(mesh):
    # Smooth erosion surfaces without rounding over a genuine rock ledge.
    adjacency={}
    for p in mesh.polygons:
        p.use_smooth=True
        for key in p.edge_keys:adjacency.setdefault(tuple(sorted(key)),[]).append(p)
    sharp=0
    for edge in mesh.edges:
        faces=adjacency.get(tuple(sorted(edge.vertices)),[])
        edge.use_edge_sharp=len(faces)!=2 or faces[0].normal.angle(faces[1].normal)>math.radians(55)
        sharp+=int(edge.use_edge_sharp)
    mesh.update();return sharp
def n(x,y,scale):return noise.noise(Vector((x*scale+19.7,y*scale-42.9,61.3)),noise_basis='PERLIN_ORIGINAL')
def export(name,verts,faces,zone):
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
    o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o)
    o.select_set(True);bpy.context.view_layer.objects.active=o
    mat=bpy.data.materials.get('Sandstone') or bpy.data.materials.new('Sandstone');mat.diffuse_color=(.44,.29,.15,1);o.data.materials.append(mat)
    uv=mesh.uv_layers.new(name='PhysicalMetres')
    for poly in mesh.polygons:
        # Side-facing UVs carry stone strata around the eroded faces.
        axis=max(range(3),key=lambda k:abs(poly.normal[k]));a,b=((1,2),(0,2),(0,1))[axis]
        for li in poly.loop_indices:
            p=mesh.vertices[mesh.loops[li].vertex_index].co;uv.data[li].uv=(p[a]/3,p[b]/3)
    tris=sum(len(p.vertices)-2 for p in mesh.polygons)
    assert tris<18000 and all(p.area>1e-10 for p in mesh.polygons)
    assert all(p.normal.z>0 for p in mesh.polygons),'Upward-facing terrain surfaces required'
    for v in mesh.vertices:
        x,y,z=v.co
        if zone=='border':assert abs(x)>=120.5 or abs(y)>=90.5,(name,tuple(v.co))
        else:
            assert -36.9<=x<=-23.1,(name,x)
            assert min(abs(y-32),abs(y+32))>=10.5,(name,y)
    if name=='SM_CTV6_CanyonNorth':normal_preview(o,'Before')
    sharp_edges=weathered_normals(mesh)
    if name=='SM_CTV6_CanyonNorth':normal_preview(o,'After')
    geometry=hashlib.sha256()
    for v in mesh.vertices:geometry.update(struct.pack('<3f',*v.co))
    for p in mesh.polygons:
        geometry.update(struct.pack('<I',len(p.vertices)))
        for i in p.vertices:geometry.update(struct.pack('<I',i))
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,mesh_smooth_type='EDGE',use_tspace=True)
    bounds=[[min(v.co[k] for v in mesh.vertices) for k in range(3)],[max(v.co[k] for v in mesh.vertices) for k in range(3)]]
    dims=list(o.dimensions)
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False);bpy.ops.import_scene.fbx(filepath=str(OUT/(name+'.fbx')))
    loaded=[a for a in bpy.context.scene.objects if a.type=='MESH'];assert len(loaded)==1
    assert all(abs(a-b)<.002 for a,b in zip(dims,loaded[0].dimensions))
    record=dict(name=name,triangles=tris,vertices=len(verts),bounds_m=bounds,dimensions_m=dims,zone=zone,validation='PASS',fbx_roundtrip=True,normal_policy='smooth with sharp splits above 55 degrees',sharp_edges=sharp_edges,geometry_sha256=geometry.hexdigest())
    records.append(record);print('CONCEPT_TERRAIN_V6_ASSET_PASS',json.dumps(record))

# Continuous four canyon walls, broken into strata, buttresses and drainage cuts.
# Inner foot is at least 0.5m outside the authoritative playable rectangle.
for axis,side,label in [('y',-1,'South'),('y',1,'North'),('x',-1,'West'),('x',1,'East')]:
    verts=[];faces=[];rows=180;cols=24
    length=170 if axis=='y' else 90;inner=90.5 if axis=='y' else 120.5
    for j in range(rows+1):
        along=-length+2*length*j/rows
        for i in range(cols+1):
            t=i/cols;out=inner+37*t
            x,y=(along,side*out) if axis=='y' else (side*out,along)
            crest=(.50+.29*math.sin(along*.055+side)+.21*n(x,y,.073))
            height=18+13*crest
            # A steep exposed inner face and a broad irregular mesa crest.
            profile=min(1,t/.42)*(1-.19*max(0,(t-.8)/.2))
            gulley=max(0,1-abs(n(x+7*n(x,y,.08),y,.16))/.10)*4*math.sin(math.pi*t)
            z=max(-.15,height*profile-gulley+1.6*n(x,y,.25)*profile)
            z+=.42*math.sin(z*math.tau/2.1)*profile
            # River exits remain open through northern/southern geology.
            if axis=='y':z*=min(1,max(0,(abs(x+30)-8)/8))
            verts.append((x,y,z-.15))
    for j in range(rows):
        for i in range(cols):
            a=j*(cols+1)+i;face=(a,a+1,a+cols+2,a+cols+1)
            if (axis=='y' and side>0) or (axis=='x' and side<0):face=tuple(reversed(face))
            faces.append(face)
    export('SM_CTV6_Canyon'+label,verts,faces,'border')

# Two long, irregular banks. Hard gaps preserve the existing two bridge corridors.
for side,label in [(-1,'West'),(1,'East')]:
    verts=[];faces=[]
    for low,high in [(-89,-42.6),(-21.4,21.4),(42.6,89)]:
        rows=math.ceil((high-low)/.5);cols=9;start=len(verts)
        for j in range(rows+1):
            y=low+(high-low)*j/rows
            width=1.45+.55*math.sin(y*.19)+.25*math.sin(y*.73)
            for i in range(cols+1):
                t=i/cols;x=-30+side*(6.85-width*t)
                z=-.06-1.82*t+.27*math.sin(y*.88+t*5)*math.sin(math.pi*t)
                z+=.12*math.sin(t*math.pi*8)*math.sin(math.pi*t)
                verts.append((x,y,z))
        for j in range(rows):
            for i in range(cols):
                a=start+j*(cols+1)+i;face=(a,a+1,a+cols+2,a+cols+1)
                faces.append(tuple(reversed(face)) if side>0 else face)
    export('SM_CTV6_RiverMargin'+label,verts,faces,'river')
(OUT/'manifest.json').write_text(json.dumps(dict(version=6,original_geometry=True,units='metres',navigation_unchanged=True,assets=records,total_triangles=sum(a['triangles'] for a in records)),indent=2),encoding='utf-8')
print('CONCEPT_TERRAIN_V6_BUILD_PASS',len(records))
