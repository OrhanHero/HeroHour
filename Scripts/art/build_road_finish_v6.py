"""Measured low-profile legacy street finish, original merged geometry.
Uses the actual saved actor audit, not guessed road coordinates. Blender background.
"""
import bpy,bmesh,json,math,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'Art/Models/RoadFinishV6';OUT.mkdir(parents=True,exist_ok=True)
AUDIT=OUT/'scene-input.json';raw=AUDIT.read_bytes();actors=json.loads(raw)
roads=[];exclusions=[]
for a in actors:
    if a['label'] in ('North south road','West road','East road'):
        assert a['mesh']=='/Engine/BasicShapes/Cube.Cube'
        x,y,z=[v/100 for v in a['position']];sx,sy,sz=a['scale']
        roads.append(dict(label=a['label'],x=x,y=y,z=z+sz/2,hx=sx/2,hy=sy/2,axis=1 if sy>sx else 0))
    if any(s in a['mesh'] for s in ('SM_DV3_RoadStraight.','SM_DV3_UrbanStreet.','SM_CH_Bridge.','SM_DV3_BaseApron.','SM_DV3_ServicePad.','SM_DV3_BaseApproach.')):
        # Conservative axis-aligned clearance avoids any paint over dressed roads.
        x,y,_=[v/100 for v in a['position']];scale=a['scale'];lo=a['local_min'];hi=a['local_max']
        hx=max(abs(lo[0]),abs(hi[0]))*scale[0]/100;hy=max(abs(lo[1]),abs(hi[1]))*scale[1]/100
        # Corrected actor yaw can swap the bounds. Audit now includes rotations.
        yaw=a.get('rotation',[0,0,0])[1];c=abs(math.cos(math.radians(yaw)));s=abs(math.sin(math.radians(yaw)))
        exclusions.append((x,y,hx*c+hy*s+.20,hx*s+hy*c+.20))
assert len(roads)==9,len(roads)
COLORS={'Yellow':(.48,.40,.16),'White':(.58,.58,.51),'Dust':(.40,.29,.17)}
bpy.context.preferences.filepaths.save_version=0
reports=[]

def blocked(x,y,road):
    # Do not embellish within the downtown road/module grid, resource pickups,
    # bridge corridors, or perpendicular legacy junctions.
    if 5<x<71 and abs(y)<56:return True
    if -40<x<-20 and min(abs(y-32),abs(y+32))<9:return True
    if any(abs(x-ex)<hx and abs(y-ey)<hy for ex,ey,hx,hy in exclusions):return True
    for other in roads:
        if other is road or other['axis']==road['axis']:continue
        if abs(x-other['x'])<other['hx']+.30 and abs(y-other['y'])<other['hy']+.30:return True
    for px,py in [(-65,16),(70,7.5),(0,-32)]:
        if math.hypot(x-px,y-py)<5.5:return True
    return False

def box(x,y,z,sx,sy,sz):
    k=len(verts)
    # Blender FBX -> UE reflects Y in this project's axis convention. Compensate
    # world-layout geometry (unlike local building meshes) before export. Reverse
    # winding as well so the reflection does not turn the solids inside out.
    verts.extend([(x+dx*sx/2,-(y+dy*sy/2),z+dz*sz/2) for dx,dy,dz in [(-1,-1,-1),(1,-1,-1),(1,1,-1),(-1,1,-1),(-1,-1,1),(1,-1,1),(1,1,1),(-1,1,1)]])
    faces.extend(tuple(k+i for i in reversed(f)) for f in [(0,3,2,1),(4,5,6,7),(0,1,5,4),(1,2,6,5),(2,3,7,6),(3,0,4,7)])

for kind in COLORS:
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
    verts=[];faces=[];pieces=0
    for road in roads:
        axis=road['axis'];half=road['hy'] if axis else road['hx'];width=road['hx'] if axis else road['hy'];mid=road['y'] if axis else road['x']
        stride=3.5 if kind=='Yellow' else 1.0
        for i in range(math.ceil((half*2-1)/stride)):
            t=mid-half+.6+i*stride
            offsets=[0] if kind=='Yellow' else [-width+.22,width-.22] if kind=='White' else [-width-.13,width+.13]
            for offset in offsets:
                x,y=(road['x']+offset,t) if axis else (t,road['y']+offset)
                length=1.5 if kind=='Yellow' else .93
                # Reject entire piece if its endpoints touch protected areas.
                ends=[(x,y),(x+(0 if axis else length/2),y+(length/2 if axis else 0)),(x-(0 if axis else length/2),y-(length/2 if axis else 0))]
                if any(blocked(px,py,road) for px,py in ends):continue
                thick=.065 if kind=='Yellow' else .055 if kind=='White' else .22+(.04*math.sin(i*.73))
                z=road['z']+.012 if kind!='Dust' else .005
                box(x,y,z,thick if axis else length,length if axis else thick,.004 if kind!='Dust' else .008);pieces+=1
    name='SM_RF6_'+kind
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update();obj=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(obj);bpy.context.view_layer.objects.active=obj;obj.select_set(True)
    mat=bpy.data.materials.new(kind);mat.diffuse_color=(*COLORS[kind],1);mesh.materials.append(mat)
    uv=mesh.uv_layers.new(name='PhysicalMetres')
    for p in mesh.polygons:
        for li in p.loop_indices:
            v=mesh.vertices[mesh.loops[li].vertex_index].co;uv.data[li].uv=(v.x,v.y)
    bm=bmesh.new();bm.from_mesh(mesh);bad=sum(not e.is_manifold for e in bm.edges);bm.free();assert bad==0
    dims=list(obj.dimensions);tris=sum(len(p.vertices)-2 for p in mesh.polygons)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,mesh_smooth_type='FACE',add_leaf_bones=False)
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False);bpy.ops.import_scene.fbx(filepath=str(OUT/(name+'.fbx')))
    q=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(q)==1
    assert all(abs(a-b)<.002 for a,b in zip(dims,q[0].dimensions))
    low=[min(v[j] for v in verts) for j in range(3)];high=[max(v[j] for v in verts) for j in range(3)]
    report=dict(name=name,material=kind,pieces=pieces,triangles=tris,dimensions_m=dims,expected_ue_bounds_cm=dict(min=[low[0]*100,-high[1]*100,low[2]*100],max=[high[0]*100,-low[1]*100,high[2]*100]),non_manifold_edges=bad,fbx_roundtrip=True,validation='PASS',pivot='world origin; source Y reflected to compensate FBX -> Unreal conversion')
    (OUT/(name+'.json')).write_text(json.dumps(report,indent=2));reports.append(report)
(OUT/'manifest.json').write_text(json.dumps(dict(assets=reports,roads=roads,audit_sha256=hashlib.sha256(raw).hexdigest(),excluded_detail_rectangles=len(exclusions),unchanged='road slabs, collision, navigation, all city actors',bridges='not embellished; original paint and detailed southern arterial retained',materials=COLORS),indent=2))
print('ROAD_FINISH_V6_BUILD_PASS',len(reports),sum(r['pieces'] for r in reports))
