"""Original flat industrial courtyard dressing for the two existing start bases.

Blender 5.2 --background --python this_file.py. No Unreal imports occur here.
All positions are measured local Unreal metres before the FBX Y reflection.
"""
import hashlib
import json
import math
import random
from pathlib import Path

import bmesh
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'Art/Models/ReferenceBaseyardV8'
OUT.mkdir(parents=True, exist_ok=True)
NAME_PREFIX = 'SM_BY8_'
SEED = 8162406
MATERIALS = {
    'Concrete': dict(source='Concrete', tint=(.90,.88,.83), roughness=.93, metallic=0.),
    'ConcreteDark': dict(source='Concrete', tint=(.61,.61,.58), roughness=.95, metallic=0.),
    'Metal': dict(source='Metal', tint=(.68,.69,.66), roughness=.71, metallic=.72),
    'Dust': dict(source='CompactedDirt', tint=(1.06,1.01,.92), roughness=.97, metallic=0., feather_ground=True),
    'Track': dict(source='Asphalt', tint=(.70,.66,.58), roughness=.97, metallic=0.),
    'Paint': dict(source='Concrete', tint=(1.03,.83,.39), roughness=.88, metallic=0.),
}
TEXTURE_ROOT = ROOT / 'Art/Textures/DistrictV3'
TEXTURE_META = json.loads((TEXTURE_ROOT/'metadata.json').read_text(encoding='utf-8'))
AUDIT = ROOT/'Art/Models/RoadFinishV6/scene-input.json'
AUDIT_HASH = hashlib.sha256(AUDIT.read_bytes()).hexdigest()
audit = json.loads(AUDIT.read_text(encoding='utf-8'))
pad_input = [a for a in audit if any(s in a['mesh'] for s in
             ('SM_DV3_BaseApron.', 'SM_DV3_ServicePad.', 'SM_DV3_BaseApproach.'))]
assert sum('SM_DV3_BaseApron.' in a['mesh'] for a in pad_input) == 2
assert any(a['position'][:2] == [-7500.,1600.] for a in pad_input)
assert any(a['position'][:2] == [7500.,1600.] for a in pad_input)
bpy.context.preferences.filepaths.save_version = 0


def clear():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 1


def material(name, info):
    mat = bpy.data.materials.get('BY8_'+name) or bpy.data.materials.new('BY8_'+name)
    mat.use_nodes = True
    nodes = mat.node_tree.nodes; nodes.clear(); links = mat.node_tree.links
    shader = nodes.new('ShaderNodeBsdfPrincipled'); output = nodes.new('ShaderNodeOutputMaterial')
    links.new(shader.outputs['BSDF'], output.inputs['Surface'])
    shader.inputs['Metallic'].default_value = info['metallic']
    coords = nodes.new('ShaderNodeTexCoord'); scale = nodes.new('ShaderNodeVectorMath')
    scale.operation = 'SCALE'; scale.inputs[3].default_value = 1/TEXTURE_META['materials'][info['source']]['tile_meters']
    links.new(coords.outputs['UV'], scale.inputs[0])
    tex = {}
    for channel in ('Albedo','Normal','Roughness'):
        filename = TEXTURE_META['materials'][info['source']]['files'][channel]
        node = nodes.new('ShaderNodeTexImage')
        node.image = bpy.data.images.load(str(TEXTURE_ROOT/filename), check_existing=True)
        node.image.colorspace_settings.name = 'sRGB' if channel=='Albedo' else 'Non-Color'
        links.new(scale.outputs['Vector'], node.inputs['Vector']); tex[channel] = node
    tint = nodes.new('ShaderNodeMixRGB'); tint.blend_type = 'MULTIPLY'; tint.inputs[0].default_value = 1
    tint.inputs[2].default_value = (*info['tint'],1)
    links.new(tex['Albedo'].outputs['Color'], tint.inputs[1])
    base_color=tint.outputs[0]
    if info.get('feather_ground'):
        ground=nodes.new('ShaderNodeTexImage')
        ground.image=bpy.data.images.load(str(TEXTURE_ROOT/TEXTURE_META['materials']['Sand']['files']['Albedo']),check_existing=True)
        ground.image.colorspace_settings.name='sRGB'
        position=nodes.new('ShaderNodeNewGeometry');world_scale=nodes.new('ShaderNodeVectorMath');world_scale.operation='SCALE';world_scale.inputs[3].default_value=.25
        links.new(position.outputs['Position'],world_scale.inputs[0]);links.new(world_scale.outputs[0],ground.inputs['Vector'])
        ground_tint=nodes.new('ShaderNodeMixRGB');ground_tint.blend_type='MULTIPLY';ground_tint.inputs[0].default_value=1
        ground_tint.inputs[2].default_value=(1.,.9,.78,1);links.new(ground.outputs['Color'],ground_tint.inputs[1])
        vertex=nodes.new('ShaderNodeVertexColor');vertex.layer_name='BY8_EdgeBlend'
        blend=nodes.new('ShaderNodeMixRGB');links.new(vertex.outputs['Alpha'],blend.inputs[0])
        links.new(ground_tint.outputs[0],blend.inputs[1]);links.new(base_color,blend.inputs[2]);base_color=blend.outputs[0]
    links.new(base_color,shader.inputs['Base Color'])
    rough = nodes.new('ShaderNodeMath'); rough.operation = 'MULTIPLY'
    rough.inputs[1].default_value = info['roughness']
    links.new(tex['Roughness'].outputs['Color'],rough.inputs[0]); links.new(rough.outputs[0],shader.inputs['Roughness'])
    # Existing HeroHour maps are DirectX normals; Blender expects OpenGL +Y.
    split = nodes.new('ShaderNodeSeparateColor'); combine = nodes.new('ShaderNodeCombineColor')
    invert = nodes.new('ShaderNodeMath'); invert.operation='SUBTRACT'; invert.inputs[0].default_value=1
    links.new(tex['Normal'].outputs['Color'],split.inputs['Color'])
    links.new(split.outputs['Red'],combine.inputs['Red']);links.new(split.outputs['Blue'],combine.inputs['Blue'])
    links.new(split.outputs['Green'],invert.inputs[1]);links.new(invert.outputs[0],combine.inputs['Green'])
    normal = nodes.new('ShaderNodeNormalMap'); normal.inputs['Strength'].default_value=.28
    links.new(combine.outputs['Color'],normal.inputs['Color']);links.new(normal.outputs['Normal'],shader.inputs['Normal'])
    return mat


def bezier_approach():
    result=[]
    for i in range(65):
        t=i/64;u=1-t
        result.append((-12*u*t*t,32*u**3+69*u*u*t+45*u*t*t+9*t**3))
    return result


APPROACH = bezier_approach()


class Yard:
    def __init__(self, side):
        self.side=side;self.mirror=1 if side=='West' else -1
        self.rng=random.Random(SEED+(0 if side=='West' else 1))
        self.vertices=[];self.faces=[];self.face_mats=[];self.alpha=[];self.pieces={}
        self.max_relative_height=0.;self.protected_rejections=0

    def blocked(self,x,y,radius=.0):
        # The asymmetric eastern field is moved in PrototypeEconomy::Restart.
        fields=[(12.0,21.0,12.0,22.0)] if self.side=='West' else [(6.3,14.8,2.3,12.0)]
        if any(x+radius>a and x-radius<b and y+radius>c and y-radius<d for a,b,c,d in fields):return True
        inside_hq=abs(x)<9 and abs(y)<9
        inside_depot=abs(x-5)<4.5 and abs(y-16)<4.5
        if not inside_hq and not inside_depot:
            if abs(y)<2.50+radius:return True  # existing long east/west street
            if self.side=='East' and abs(x-5)<3.0+radius:return True
            if self.side=='West' and min(math.hypot(x-px,y-py) for px,py in APPROACH)<3.85+radius:return True
        return False

    def add(self,vertices,faces,kind,piece,alpha=None):
        k=len(self.vertices)
        self.vertices.extend((self.mirror*x,-y,z) for x,y,z in vertices)
        if self.mirror==1:faces=[tuple(reversed(face)) for face in faces]
        self.faces.extend(tuple(k+i for i in face) for face in faces)
        self.face_mats.extend([list(MATERIALS).index(kind)]*len(faces))
        self.alpha.extend(alpha if alpha is not None else [1.]*len(vertices))
        self.pieces[piece]=self.pieces.get(piece,0)+1

    def poly(self,poly,support,top,kind,piece):
        assert top>=support and top-support<=.03001,(piece,support,top)
        bottom=max(.0005,support+.0004)
        if top<=bottom:top=bottom+.0003
        # Winding flips with the authored Y reflection; East additionally mirrors X.
        points=[(x,y,bottom) for x,y in poly]
        points += [(x,y,top) for x,y in poly]
        n=len(poly)
        local=[tuple(range(n-1,-1,-1)),tuple(range(n,2*n))]
        local += [(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
        self.add(points,local,kind,piece)
        self.max_relative_height=max(self.max_relative_height,top-support)

    def rect(self,x,y,w,h,support,top,kind,piece,angle=0,cut=0):
        if cut:
            xh,yh=w/2,h/2;cut=min(cut,w*.25,h*.25)
            poly=[(-xh+cut,-yh),(xh-cut,-yh),(xh,-yh+cut),(xh,yh-cut),
                  (xh-cut,yh),(-xh+cut,yh),(-xh,yh-cut),(-xh,-yh+cut)]
        else:poly=[(-w/2,-h/2),(w/2,-h/2),(w/2,h/2),(-w/2,h/2)]
        co,si=math.cos(angle),math.sin(angle)
        self.poly([(x+px*co-py*si,y+px*si+py*co) for px,py in poly],support,top,kind,piece)

    def slab(self,x,y,w,h,support):
        if self.blocked(x,y,min(w,h)/2):self.protected_rejections+=1;return
        top=support+self.rng.uniform(.007,.011)
        self.rect(x,y,w,h,support,top,'Concrete' if self.rng.random()>.23 else 'ConcreteDark',
                  'concrete_repair_slab',self.rng.uniform(-.004,.004),self.rng.uniform(.055,.13))
        # Sparse irregular cracks are geometric dark seams, a few millimetres high.
        if self.rng.random()<.43 and min(w,h)>.7:
            px=x-w*.34;py=y+self.rng.uniform(-.22,.22)*h
            for i in range(3):
                nx=px+w*.22;ny=py+self.rng.uniform(-.12,.12)
                length=math.hypot(nx-px,ny-py)
                self.rect((px+nx)/2,(py+ny)/2,length,.012,top,top+.001,
                          'Track','concrete_crack',math.atan2(ny-py,nx-px))
                px,py=nx,ny
        if self.rng.random()<.25 and w>1.3 and h>1.3:
            self.rect(x+w*.18,y-h*.17,w*.24,h*.31,top,top+.0015,'ConcreteDark','small_repair_patch',.04,.03)

    def slab_ring(self,cx,cy,outer_x,outer_y,inner_x,inner_y,floor):
        for sign in [-1,1]:
            width=outer_x*2/7
            for col in range(7):
                self.slab(cx-outer_x+(col+.5)*width,cy+sign*(outer_y+inner_y)/2,
                          width-.055,outer_y-inner_y-.06,floor)
            height=inner_y*2/4
            for row in range(4):
                self.slab(cx+sign*(outer_x+inner_x)/2,cy-inner_y+(row+.5)*height,
                          outer_x-inner_x-.06,height-.06,floor)

    def drain(self,x,y,length,angle=0):
        if self.blocked(x,y,.2):return
        floor=.05
        self.rect(x,y,length,.29,floor,.063,'Track','drain_channel',angle)
        for side in [-1,1]:
            co,si=math.cos(angle),math.sin(angle)
            self.rect(x-side*.15*si,y+side*.15*co,length,.025,floor,.071,'Metal','drain_rim',angle)
        for i in range(max(1,int(length/.14))):
            offset=-length/2+.07+i*.14
            self.rect(x+offset*math.cos(angle),y+offset*math.sin(angle),.037,.24,
                      floor,.078,'Metal','drain_crossbar',angle)

    def dust(self,x,y,rx,ry,floor=0):
        if self.blocked(x,y,max(rx,ry)*.45):self.protected_rejections+=1;return
        poly=[]
        for i in range(10):
            angle=i*math.tau/10;factor=self.rng.uniform(.74,1.12)
            poly.append((x+math.cos(angle)*rx*factor,y+math.sin(angle)*ry*factor))
        # Closed low-profile mesh, but the edge color blends into real sand.
        # Vertex alpha is a PBR blend weight, not material transparency.
        points=[];alpha=[];n=len(poly);peak=self.rng.uniform(.004,.008)
        for radius,z,weight in [(1.,floor+.0005,0.),(.74,floor+peak*.45,.50),(.42,floor+peak,1.)]:
            points.extend((x+(px-x)*radius,y+(py-y)*radius,z) for px,py in poly)
            alpha.extend([weight]*n)
        bottom_index=len(points);points.extend((px,py,floor+.0002) for px,py in poly);alpha.extend([0.]*n)
        center_index=len(points);points.append((x,y,floor+peak));alpha.append(1.)
        faces=[tuple(bottom_index+i for i in range(n-1,-1,-1))]
        for ring in range(2):
            for i in range(n):faces.append((ring*n+i,ring*n+(i+1)%n,(ring+1)*n+(i+1)%n,(ring+1)*n+i))
        for i in range(n):
            faces.append((2*n+i,2*n+(i+1)%n,center_index))
            faces.append((bottom_index+i,bottom_index+(i+1)%n,(i+1)%n,i))
        self.add(points,faces,'Dust','irregular_dust_drift',alpha)
        self.max_relative_height=max(self.max_relative_height,peak)

    def track(self,points,ground):
        for i in range(len(points)-1):
            a,b=points[i],points[i+1];dx,dy=b[0]-a[0],b[1]-a[1];length=math.hypot(dx,dy)
            nx,ny=-dy/length,dx/length
            for side in [-1,1]:
                x=(a[0]+b[0])/2+nx*side*.77;y=(a[1]+b[1])/2+ny*side*.77
                if self.blocked(x,y,.22):continue
                floor=ground(x,y)
                first=len(self.alpha)
                exterior=floor<.02
                self.rect(x,y,length*.87,self.rng.uniform(.11,.17),floor,floor+.0015,
                          'Dust' if exterior else 'Track','worn_tire_segment',math.atan2(dy,dx))
                if exterior:
                    # Dust displacement on sand is lighter and fades at the ends;
                    # only hard-surface rubber wear uses the dark track material.
                    weight=.48+.35*math.sin(math.pi*(i+.5)/(len(points)-1))
                    self.alpha[first:]=[weight]*(len(self.alpha)-first)

    def pebble(self,x,y):
        if self.blocked(x,y,.15):return
        # Stone chips are at most 2.8cm above ground, never tall blockers.
        count=6;rad=self.rng.uniform(.035,.10);height=self.rng.uniform(.009,.028)
        poly=[(x+math.cos(i*math.tau/count)*rad*self.rng.uniform(.75,1.08),
               y+math.sin(i*math.tau/count)*rad*self.rng.uniform(.65,1.05)) for i in range(count)]
        self.poly(poly,0,height,'ConcreteDark' if self.rng.random()<.40 else 'Dust','small_stone_chip')

    def build(self):
        # Ring surrounds the 10.65/11.25m HQ foundations without covering them.
        self.slab_ring(0,0,8.60,8.60,5.85,5.85,.05)
        self.slab_ring(5,16,3.70,3.70,3.05,3.30,.05)
        # A small separate work surface around the existing power-plant site.
        self.slab_ring(-10,-16,4.40,4.40,3.20,3.20,0)
        self.drain(-8.27,-2.25,3.75,math.pi/2)
        self.drain(8.27,-2.25,3.75,math.pi/2)
        self.drain(-1.50,8.30,3.40)
        self.drain(5,12.48,2.70)
        # Restrained worn maintenance hatching in front of the real HQ entrance.
        for side in [-1,1]:
            self.rect(side*1.65,-7.13,.045,1.35,.062,.064,'Paint','maintenance_edge')
        self.rect(0,-7.80,3.34,.045,.062,.064,'Paint','maintenance_edge')
        for i in range(8):
            self.rect(-1.33+i*.38,-7.22,.055,.77,.062,.064,'Paint','maintenance_hatch',-.52)
        # Dark rubber wear follows actual collector and vehicle travel space.
        self.track([(0+.20*math.sin(i*.19),-14.2+i*.27) for i in range(25)],
                   lambda x,y:.063 if abs(x)<9 and abs(y)<8.6 else .004)
        for lane in [-1,1]:
            self.track([(9.3+i*.25,lane*(3.4+.14*math.sin(i*.25))) for i in range(24)],lambda x,y:.004)
        # Deposited dirt drifts and flattened gravel just outside hard surfaces.
        for side in [-1,1]:
            for i in range(7):
                t=-7.9+i*2.6
                self.dust(side*self.rng.uniform(9.03,9.62),t,.75,.94)
                self.dust(t,side*self.rng.uniform(9.06,9.67),.94,.63)
        for i in range(9):
            angle=i*math.tau/9
            self.dust(5+math.cos(angle)*4.92,16+math.sin(angle)*4.86,.65,.49)
        for i in range(340):
            side=self.rng.choice([-1,1]);t=self.rng.uniform(-9.0,9.0);outer=self.rng.uniform(9.08,10.25)
            x,y=(side*outer,t) if i%2 else (t,side*outer)
            self.pebble(x,y)
        for i in range(100):
            angle=self.rng.uniform(0,math.tau);radius=self.rng.uniform(4.65,5.55)
            self.pebble(5+math.cos(angle)*radius,16+math.sin(angle)*radius)
        assert self.max_relative_height<=.03001


def physical_uv(mesh):
    layer=mesh.uv_layers.new(name='PhysicalMetres')
    for face in mesh.polygons:
        axis=max(range(3),key=lambda j:abs(face.normal[j]));coords=((1,2),(0,2),(0,1))[axis]
        for index in face.loop_indices:
            point=mesh.vertices[mesh.loops[index].vertex_index].co
            layer.data[index].uv=(point[coords[0]],point[coords[1]])
    assert all(math.isfinite(v) for item in layer.data for v in item.uv)


def export_yard(yard):
    clear();name=NAME_PREFIX+yard.side+'Baseyard'
    mesh=bpy.data.meshes.new(name);mesh.from_pydata(yard.vertices,[],yard.faces);mesh.update()
    obj=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(obj)
    for key,info in MATERIALS.items():mesh.materials.append(material(key,info))
    for face,index in zip(mesh.polygons,yard.face_mats):face.material_index=index
    colors=mesh.color_attributes.new(name='BY8_EdgeBlend',type='FLOAT_COLOR',domain='CORNER')
    for loop in mesh.loops:colors.data[loop.index].color=(1.,1.,1.,yard.alpha[loop.vertex_index])
    physical_uv(mesh)
    bm=bmesh.new();bm.from_mesh(mesh);nonmanifold=sum(not e.is_manifold for e in bm.edges);bm.free()
    degenerate=sum(face.area<1e-10 for face in mesh.polygons)
    assert nonmanifold==0 and degenerate==0,(nonmanifold,degenerate)
    bounds=[[min(v[i] for v in yard.vertices),max(v[i] for v in yard.vertices)] for i in range(3)]
    tris=sum(len(face.vertices)-2 for face in mesh.polygons)
    assert tris<30000 and len(mesh.materials)==6,(tris,len(mesh.materials))
    bpy.context.view_layer.objects.active=obj;obj.select_set(True)
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},
        axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    bpy.ops.object.select_all(action='DESELECT');bpy.ops.import_scene.fbx(filepath=str(OUT/(name+'.fbx')))
    imported=[o for o in bpy.context.selected_objects if o.type=='MESH'];assert len(imported)==1
    imported_colors=imported[0].data.color_attributes
    assert len(imported_colors)>0,'FBX lost dust vertex falloff'
    imported_alpha=[entry.color[3] for entry in imported_colors[0].data]
    assert min(imported_alpha)<.02 and max(imported_alpha)>.98
    points=[o.matrix_world@v.co for o in imported for v in o.data.vertices]
    rb=[[min(p[i] for p in points),max(p[i] for p in points)] for i in range(3)]
    assert all(abs(a-b)<.001 for pair,other in zip(bounds,rb) for a,b in zip(pair,other)),(bounds,rb)
    for old in imported:bpy.data.objects.remove(old,do_unlink=True)
    record=dict(name=name,side=yard.side,triangles=tris,vertices=len(mesh.vertices),materials=list(MATERIALS),
        pieces=yard.pieces,piece_count=sum(yard.pieces.values()),source_bounds_m=bounds,fbx_roundtrip_bounds_m=rb,
        fbx_roundtrip_pass=True,non_manifold_edges=nonmanifold,degenerate_faces=degenerate,
        expected_ue_bounds_cm=[[a*100,b*100] if i!=1 else [-b*100,-a*100] for i,(a,b) in enumerate(bounds)],
        actor_location_cm=[-8000 if yard.side=='West' else 8000,0,0],actor_rotation_degrees=[0,0,0],
        actor_scale=[1,1,1],maximum_relative_relief_cm=yard.max_relative_height*100,
        height_convention='HQ and depot old deck top=5cm; added details <=3cm above their contact surface; exterior stones <=2.8cm',
        uv_mapping='UV0 dominant-plane physical metres, one UV unit per metre',vertex_alpha='BY8_EdgeBlend: opaque sand/dust PBR falloff, FBX roundtrip checked',protected_rejections=yard.protected_rejections,
        fbx_sha256=hashlib.sha256((OUT/(name+'.fbx')).read_bytes()).hexdigest(),collision=False,
        casts_shadow=False,affects_navigation=False,validation='PASS',
        provenance='Original procedural HeroHour ground dressing; own existing PBR texture library; no commercial game assets')
    (OUT/(name+'.json')).write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
    preview(obj,record)
    return record


def append_context(folder,name,location,scale=1):
    file=ROOT/'Art/Models'/folder/(name+'.blend');before=hashlib.sha256(file.read_bytes()).hexdigest()
    with bpy.data.libraries.load(str(file),link=False) as (_,data):data.objects=[name]
    obj=data.objects[0];assert obj;bpy.context.collection.objects.link(obj)
    obj.location=location;obj.scale=(scale,scale,scale)
    assert hashlib.sha256(file.read_bytes()).hexdigest()==before
    return obj


def preview(obj,record):
    # These are actual mesh renders with read-only copies of the existing apron
    # and HQ as scale/context. They are explicitly not screenshots from Unreal.
    side=record['side'];mirror=1 if side=='West' else -1
    append_context('DistrictV3','SM_DV3_BaseApron',(0,0,.02))
    append_context('DistrictV3','SM_DV3_ServicePad',(mirror*5,-16,.01))
    folder,name=('ReferenceHQV7','SM_RV7_AegisHQ') if side=='West' else ('ReferenceEmberHQV7','SM_RV7_EmberHQ')
    append_context(folder,name,(0,0,1.65),1.5)
    append_context('MilitaryDetailV6','SM_MDV6_Refinery',(mirror*5,-16,.525),1.5)
    append_context('FrontlinesV4','SM_HR_PowerPlant',(-mirror*10,16,.35))
    for old in bpy.context.scene.objects:
        if old==obj or old.type!='MESH':continue
        if old.name.startswith(('SM_DV3_BaseApron','SM_DV3_ServicePad')):
            for slot in old.material_slots:
                source=slot.material.name.split('.')[0] if slot.material else ''
                target='Track' if source=='Asphalt' else 'Concrete' if source=='Concrete' else 'Paint' if source=='Paint' else 'Metal'
                slot.material=bpy.data.materials['BY8_'+target]
    # A plain desert contact plane supplies ground under the flat dust/stone islands.
    bpy.ops.mesh.primitive_plane_add(size=110,location=(0,0,-.001));ground=bpy.context.object
    physical_uv(ground.data)
    ground.data.materials.append(material('PreviewSand',dict(source='Sand',tint=(1,.9,.78),roughness=1,metallic=0)))
    scene=bpy.context.scene;world=bpy.data.worlds.new('Baseyard neutral daylight');scene.world=world;world.use_nodes=True
    world.node_tree.nodes.clear();bg=world.node_tree.nodes.new('ShaderNodeBackground');out=world.node_tree.nodes.new('ShaderNodeOutputWorld')
    world.node_tree.links.new(bg.outputs[0],out.inputs[0]);bg.inputs[0].default_value=(.32,.36,.40,1);bg.inputs[1].default_value=.6
    for pos,power,size in [((-15,18,25),5500,15),((18,-8,18),3200,12)]:
        bpy.ops.object.light_add(type='AREA',location=pos);light=bpy.context.object;light.data.energy=power;light.data.size=size
        light.rotation_euler=(Vector((0,-3,0))-light.location).to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.camera_add(location=(-24,31,31));cam=bpy.context.object
    cam.rotation_euler=(Vector((0,-4,0))-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';cam.data.ortho_scale=43;scene.camera=cam
    scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True
    scene.render.resolution_x=1600;scene.render.resolution_y=1400;scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG';scene.render.filepath=str(OUT/(record['name']+'-Preview.png'))
    bpy.ops.render.render(write_still=True)
    cam.location=(-14,20,23);cam.rotation_euler=(Vector((0,0,.1))-cam.location).to_track_quat('-Z','Y').to_euler()
    cam.data.ortho_scale=25;scene.render.filepath=str(OUT/(record['name']+'-HQCloseUp.png'));bpy.ops.render.render(write_still=True)


reports=[]
for side in ('West','East'):
    yard=Yard(side);yard.build();reports.append(export_yard(yard))
manifest=dict(generator='build_reference_baseyard_v8.py',seed=SEED,assets=reports,materials=MATERIALS,
    cached_layout_audit_sha256=AUDIT_HASH,source_layout_inputs=pad_input,
    spawn_basis=dict(HQ=[[-8000,0],[8000,0]],Depot=[[-7500,1600],[7500,1600]],PowerPlant=[[-9000,-1600],[9000,-1600]]),
    reference='User concept 01_22_20 and actual V7 runtime base captures',
    intended_actors=2,shared_material_instances=6,unreal_import_executed=False,
    unchanged='Global roads, existing map actors, C++ navigation, actor footprints, collisions and gameplay',
    preview_limits='Blender mesh renders with actual source HQ/pad context; source HQ palette is not the final V7 runtime atlas; root must inspect current Unreal captures')
(OUT/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n',encoding='utf-8')
print('REFERENCE_BASEYARD_V8_BUILD_PASS '+json.dumps(dict(actors=2,total_triangles=sum(r['triangles'] for r in reports),
    total_pieces=sum(r['piece_count'] for r in reports),shared_materials=6)),flush=True)
