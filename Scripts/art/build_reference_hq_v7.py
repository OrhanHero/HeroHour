"""Original faceted Aegis command tower based on the supplied right-hand HQ.
Run Blender 5.2 --background --python this_file.py. Entrance Blender +Y -> UE -Y.
"""
import hashlib, json, math, sys
from pathlib import Path
import bpy
from mathutils import Vector
sys.path.insert(0, str(Path(__file__).resolve().parent))
import concept_asset_common as c
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'Art/Models/ReferenceHQV7'; OUT.mkdir(parents=True, exist_ok=True)
c.OUT = OUT; c.PREVIEW = OUT
c.PALETTE.update(Armor=((.60,.64,.64),.08,.66), Steel=((.25,.29,.31),.65,.46),
    Concrete=((.45,.47,.45),0,.86), Team=((.025,.15,.42),.35,.44),
    Neon=((.015,.30,.85),.10,.32), Glass=((.015,.045,.07),.50,.22))
c.reset(); NAME = 'SM_RV7_AegisHQ'; CX, CY = .60, -.35

def outline(cx, cy, width, depth, z, cut):
    x, y = width/2, depth/2
    return [Vector((cx+px,cy+py,z)) for px,py in ((-x+cut,-y),(x-cut,-y),
        (x,-y+cut),(x,y-cut),(x-cut,y),(-x+cut,y),(-x,y-cut),(-x,-y+cut))]

def solid(name, vertices, faces, material, bevel=.02):
    mesh=bpy.data.meshes.new(name+' geometry');mesh.from_pydata(vertices,[],faces);mesh.update()
    obj=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(obj)
    c.finish(obj,name,material);c.bevel(obj,bevel,1);return obj

def tier(name, lower, upper, material, bevel=.025):
    sides=[(i,(i+1)%8,(i+1)%8+8,i+8) for i in range(8)]
    solid(name,lower+upper,[tuple(range(7,-1,-1)),tuple(range(8,16))]+sides,material,bevel)
    return [[lower[i],lower[(i+1)%8],upper[(i+1)%8],upper[i]] for i in range(8)]

def panel(name, quad, u0=.04, u1=.96, v0=.04, v1=.96,
          material='Armor', offset=.015, thickness=.030, bevel=.008):
    """Fitted solid armor following an actual sloping face, with visible joints."""
    p0,p1,p2,p3=quad;normal=(p1-p0).cross(p3-p0).normalized()
    def point(u,v):return (p0*(1-u)+p1*u)*(1-v)+(p3*(1-u)+p2*u)*v
    base=[point(u,v)+normal*offset for u,v in [(u0,v0),(u1,v0),(u1,v1),(u0,v1)]]
    top=[p+normal*thickness for p in base]
    return solid(name,base+top,[(3,2,1,0),(4,5,6,7),(0,1,5,4),(1,2,6,5),
                              (2,3,7,6),(3,0,4,7)],material,bevel)

def plate_fasteners(quad,u0,u1,v0,v1):
    p0,p1,p2,p3=quad;normal=(p1-p0).cross(p3-p0).normalized()
    rotation=normal.to_track_quat('Z','Y').to_euler()
    for u,v in [(u0+.035,v0+.045),(u1-.035,v0+.045),
                (u0+.035,v1-.045),(u1-.035,v1-.045)]:
        point=((p0*(1-u)+p1*u)*(1-v)+(p3*(1-u)+p2*u)*v)+normal*.084
        c.cylinder('Captive armor plate fastener',point,.012,.012,'Steel',rot=rotation,vertices=6)

def cheek(name,x,width,front_y,back_y,bottom,top):
    # Wide diagonal support, around 45 degrees, not a thin corner column.
    q=[Vector((x+width/2,front_y,bottom)),Vector((x-width/2,front_y,bottom)),
       Vector((x-width/2,back_y,top)),Vector((x+width/2,back_y,top))]
    assert (q[1]-q[0]).cross(q[3]-q[0]).normalized().y>0
    behind=[p+Vector((0,-.45,0)) for p in q]
    solid(name,q+behind,[(0,1,2,3),(7,6,5,4),(0,4,5,1),(1,5,6,2),
                        (2,6,7,3),(3,7,4,0)],'Steel',.035)
    for i in range(3):
        panel(name+' fitted slope plate',q,.07,.93,.045+i*.315,.325+i*.315,
              'Armor',.018,.055,.014)
    for edge in [.045,.940]:
        panel(name+' protective edge rail',q,edge,edge+.015,.025,.98,'Steel',.08,.018,.003)
    c.box(name+' wide armored foot',(x,front_y-.20,.43),(width+.23,.89,.24),'Armor',.025)

# Existing footprint and ground compensation are retained. The flat house and
# giant garage are replaced by a compact eight-faceted command tower.
c.box('Reinforced rectangular apron',(0,0,.12),(7.10,6.80,.24),'Concrete',.07)
tier('Inset clipped apron deck',outline(0,0,6.75,6.35,.24,.40),
     outline(0,0,6.75,6.35,.30,.40),'Dark',.012)
walls=tier('Eight faceted main tower shell',outline(CX,CY,4.65,4.50,.37,.98),
           outline(CX,CY,3.98,3.80,2.92,.83),'Steel',.045)
tier('Lower tower armored belt',outline(CX,CY,4.80,4.65,.32,1.00),
     outline(CX,CY,4.70,4.54,.61,.98),'Armor',.028)
for face_index,quad in enumerate(walls):
    if face_index==4:
        for u0,u1 in [(.045,.24),(.76,.955)]:
            panel('Front narrow facade armor',quad,u0,u1,.10,.76,'Armor',.018,.045)
        panel('Entry overhead fitted armor',quad,.25,.75,.73,.94,'Armor',.02,.045)
    else:
        for u0,u1 in [(.035,.485),(.515,.965)]:
            for v0,v1 in [(.12,.485),(.515,.94)]:
                panel('Fitted tower facade panel',quad,u0,u1,v0,v1,'Armor',.018,.042,.011)
                panel('Stamped armor inset border',quad,u0+.055,u1-.055,v0+.065,v1-.065,
                      'Dark',.062,.008,.003)
                panel('Pressed inner armor field',quad,u0+.068,u1-.068,v0+.079,v1-.079,
                      'Armor',.069,.012,.004)
                plate_fasteners(quad,u0,u1,v0,v1)
        panel('Horizontal recessed facade channel',quad,.04,.96,.487,.513,'Dark',.017,.012,.001)

# Sloped shoulders, blue-window observation tier and stacked polygonal plates.
tier('Octagonal command shoulder',outline(CX,CY,4.13,3.95,2.88,.88),
     outline(CX,CY,3.82,3.66,3.09,.83),'Armor',.023)
windows=tier('Inset polygonal blue command level',outline(CX,CY,3.84,3.68,3.07,.84),
             outline(CX,CY,3.40,3.28,3.56,.75),'Steel',.015)
for quad in windows:
    for u0,u1 in [(.07,.46),(.54,.93)]:
        panel('Blue command window inset',quad,u0,u1,.20,.70,'Glass',.012,.020,.006)
        panel('Command blue lower sill',quad,u0,u1,.16,.22,'Team',.040,.018,.003)
    panel('Window level upper armor brow',quad,.025,.975,.76,.98,'Armor',.015,.040,.006)
tier('Wide octagonal dish roof plate',outline(CX,CY,3.71,3.59,3.53,.81),
     outline(CX,CY,3.65,3.53,3.68,.80),'Steel',.018)
roof=tier('Stepped polygonal roof shoulder',outline(CX,CY,3.60,3.48,3.68,.79),
          outline(CX,CY,3.10,3.01,3.88,.70),'Armor',.018)
tier('Upper dish service plate',outline(CX,CY,3.13,3.04,3.87,.71),
     outline(CX,CY,3.08,2.99,3.98,.70),'Steel',.015)
for quad in roof:panel('Roof shoulder inset armor',quad,.08,.92,.15,.87,'Armor',.010,.025,.006)

# The reference's broad diagonal cheek armor and small central personnel door.
for x in [CX-1.34,CX+1.34]:cheek('Diagonal front tower cheek',x,.73,2.81,1.08,.50,2.65)
for side in [-1,1]:cheek('Outer short side reinforcement',CX+side*1.90,.46,1.75,.59,.47,1.92)
door_x=CX
c.box('Pedestrian entrance shadow pocket',(door_x,1.948,1.20),(1.20,.32,1.73),'Dark',.025)
c.box('Inset double personnel door',(door_x,2.115,1.18),(.88,.032,1.46),'Steel',.012)
for side in [-1,1]:
    c.box('Door inset armor leaf',(door_x+side*.22,2.141,1.18),(.38,.020,1.34),'Armor',.008)
    c.box('Personnel entrance frame',(door_x+side*.57,2.25,1.20),(.13,.42,1.78),'Armor',.018)
    c.box('Entrance blue identification light',(door_x+side*.47,2.48,1.45),(.025,.025,.36),'Neon',.004)
c.box('Personnel door armored header',(door_x,2.23,2.06),(1.28,.43,.17),'Armor',.022)
c.box('Door blue number plate',(door_x,2.465,2.07),(.62,.026,.07),'Team',.004)
c.box('Door centre seam',(door_x,2.159,1.18),(.016,.009,1.38),'Dark',.001)
for x in [door_x-.07,door_x+.07]:c.box('Door handle',(x,2.172,1.15),(.021,.025,.17),'Steel',.003)
for i in range(3):
    c.box('Personnel approach step',(door_x,2.43+i*.23,.38-i*.035),(1.22,.27,.13),'Concrete',.012)
    c.box('Step wear edge',(door_x,2.55+i*.23,.45-i*.035),(1.13,.022,.010),'Dust',.002)

# Lower rectangular wing: equipment bays, inset panels, rooftop pipes and rails.
wing_x,wing_y=-2.18,-.35
wing_walls=tier('Low attached rectangular service wing',outline(wing_x,wing_y,1.93,3.38,.31,.15),
                outline(wing_x,wing_y,1.81,3.25,1.80,.14),'Steel',.03)
for index,quad in enumerate(wing_walls):
    if index in [0,2,6]:
        for u0,u1 in [(.05,.47),(.53,.95)]:
            panel('Service wing fitted wall panel',quad,u0,u1,.10,.88,'Armor',.015,.04,.008)
    elif index!=4:panel('Service wing corner armor',quad,.06,.94,.10,.88,'Armor',.015,.04,.008)
c.box('Wing roof armored overhang',(wing_x,wing_y,1.88),(2.04,3.50,.16),'Armor',.035)
c.box('Inset roof servicing deck',(wing_x,wing_y,1.979),(1.75,3.18,.045),'Steel',.010)
for y in [-1.41,-.35,.71]:
    c.box('Wing side equipment bay shadow',(-3.174,y,1.05),(.027,.78,.87),'Dark',.008)
    c.box('Wing side bay inset plate',(-3.194,y,1.05),(.027,.65,.74),'Armor',.006)
    for z in [.78,.91,1.04,1.17,1.30]:
        c.box('Wing side pressed armor rib',(-3.220,y,z),(.025,.57,.033),'Steel',.003)
    c.box('Wing side blue inset strip',(-3.23,y,1.48),(.020,.58,.045),'Team',.003)
c.box('Wing personnel entrance recess',(wing_x,1.354,.99),(.74,.045,1.25),'Dark',.015)
c.box('Wing service door leaf',(wing_x,1.385,.99),(.61,.025,1.12),'Armor',.010)
c.box('Wing entry inset frame',(wing_x,1.405,1.65),(.84,.065,.09),'Steel',.010)
c.box('Wing entrance worklight',(wing_x,1.445,1.65),(.46,.023,.035),'Neon',.003)
for y in [-1.31,-.56]:
    c.box('Wing roof ventilation unit',(-2.55,y,2.13),(.61,.58,.24),'Armor',.018)
    c.cylinder('Roof fan opening',(-2.55,y,2.266),.21,.024,'Dark',vertices=20)
    for j in range(5):c.box('Roof fan grille',(-2.71+j*.08,y,2.285),(.018,.35,.014),'Steel',.002)
for x in [-2.17,-1.99,-1.81]:
    c.beam('Roof bundled conduit',(x,-1.72,2.10),(x,.65,2.10),.035,'Steel')
    c.beam('Roof conduit down bend',(x,.65,2.10),(x,.77,1.99),.035,'Steel')
for y in [-1.32,-.50,.31]:c.box('Conduit retaining band',(-1.99,y,2.14),(.51,.047,.037),'Dark',.003)
for x in [-2.99,-1.42]:
    for y in [-1.76,-.68,.53]:c.beam('Guardrail post',(x,y,2.00),(x,y,2.30),.018,'Steel')
    c.beam('Guardrail top',(x,-1.76,2.30),(x,.53,2.30),.020,'Steel')
    c.beam('Guardrail middle',(x,-1.76,2.16),(x,.53,2.16),.014,'Steel')
for x in [CX-.89,CX-.58,CX-.27]:
    c.beam('Rear vertical pipe bundle',(x,-2.65,.51),(x,-2.65,2.55),.037,'Steel')
    c.beam('Rear upper pipe elbow',(x,-2.65,2.55),(x,-2.34,2.74),.037,'Steel')
for z in [.75,1.55,2.30]:c.box('Rear pipe strap',(CX-.58,-2.69,z),(.87,.048,.067),'Dark',.003)
c.vent('Rear electrical louver',(CX+1.1,-2.62,1.43),.59,.81,count=9)

# Shallow parabolic dish above a low bearing, not a circular building core.
c.cylinder('Dish azimuth bearing',(CX,CY,4.075),.57,.18,'Steel',vertices=32)
c.cylinder('Dish bearing seam',(CX,CY,4.18),.49,.045,'Dark',vertices=32)
c.box('Dish tilt pedestal',(CX,CY,4.37),(.34,.42,.37),'Armor',.025)
center=Vector((CX,CY,4.64));rotation=Vector((0,.74,.67)).to_track_quat('Z','Y')
vertices,faces=[],[];rings,segments,dish_radius=8,56,1.08
for side in [0,1]:
    for ring in range(rings+1):
        rr=max(.001,ring*dish_radius/rings)
        for k in range(segments):
            angle=k*math.tau/segments
            vertices.append(center+rotation@Vector((rr*math.cos(angle),rr*math.sin(angle),
                                                    .23*(rr/dish_radius)**2-side*.035)))
stride=(rings+1)*segments
for side in [0,1]:
    for ring in range(rings):
        for k in range(segments):
            a=side*stride+ring*segments+k;b=side*stride+ring*segments+(k+1)%segments
            face=(a,b,b+segments,a+segments);faces.append(face if side==0 else tuple(reversed(face)))
    faces.append(tuple(side*stride+j for j in (range(segments-1,-1,-1) if side==0 else range(segments))))
for k in range(segments):
    a=rings*segments+k;b=rings*segments+(k+1)%segments;faces.append((a,a+stride,b+stride,b))
solid('Parabolic command dish',vertices,faces,'Armor',0)
for k in range(12):
    angle=k*math.tau/12
    c.beam('Dish radial reinforcing rib',center+rotation@Vector((.19*math.cos(angle),.19*math.sin(angle),.010)),
           center+rotation@Vector((1.06*math.cos(angle),1.06*math.sin(angle),.23)),.012,'Steel')
for k in range(3):
    angle=k*math.tau/3
    c.beam('Dish feedhorn tripod',center+rotation@Vector((.91*math.cos(angle),.91*math.sin(angle),.19)),
           center+rotation@Vector((0,0,.66)),.016,'Steel')
c.beam('Dish receiver',center+rotation@Vector((0,0,.63)),center+rotation@Vector((0,0,.77)),.055,'Dark')

for x,y in [(-2.70,2.52),(2.74,-2.76)]:
    c.box('Ground equipment cabinet',(x,y,.65),(.63,.65,.71),'Armor',.026)
    c.box('Ground cabinet cap',(x,y,1.025),(.68,.70,.07),'Steel',.010)
    c.vent('Ground cabinet intake',(x,y+.342,.65),.43,.42,count=6)
for x in [-3.30,3.30]:c.box('Apron long edge',(x,0,.311),(.025,5.90,.010),'Dust',.002)
for y in [-2.93,2.93]:c.box('Apron short edge',(0,y,.311),(6.60,.025,.010),'Dust',.002)
for x,y in [(-1.10,-2.80),(1.34,-2.80),(-2.60,1.91)]:
    c.box('Apron utility hatch',(x,y,.317),(.42,.44,.023),'Steel',.006)
    for i in range(4):c.box('Hatch channel',(x-.14+i*.095,y,.331),(.012,.33,.008),'Dark',.001)

model=c.export(NAME,zoffset=-1.10)
points=[model.matrix_world@vertex.co for vertex in model.data.vertices]
bounds=[[min(p[i] for p in points),max(p[i] for p in points)] for i in range(3)]
assert abs(bounds[2][0]+1.10)<.001,bounds
assert all(pair[1]-pair[0]<=8.0 for pair in bounds[:2]),bounds
assert Vector(model.rotation_euler).length<.001
bpy.ops.object.select_all(action='DESELECT');bpy.ops.import_scene.fbx(filepath=str(OUT/(NAME+'.fbx')))
imported=[obj for obj in bpy.context.selected_objects if obj.type=='MESH']
round_points=[obj.matrix_world@vertex.co for obj in imported for vertex in obj.data.vertices]
round_bounds=[[min(p[i] for p in round_points),max(p[i] for p in round_points)] for i in range(3)]
assert all(abs(a-b)<.001 for pair,other in zip(bounds,round_bounds) for a,b in zip(pair,other)),(bounds,round_bounds)
for obj in imported:bpy.data.objects.remove(obj,do_unlink=True)
metadata_path=OUT/(NAME+'.json');meta=json.loads(metadata_path.read_text(encoding='utf-8'))
assert meta['triangles']<40000,meta['triangles']
meta.update(dict(provenance='Original HeroHour mesh; supplied concept reference only; no extracted game assets',
    reference='Codex-Bild 1. Sept. 2026, 01_22_20.png, right white/blue command HQ',
    design_revision='Faceted tower replaces rejected flat house and giant garage',
    bounds_m=bounds,fbx_roundtrip_bounds_m=round_bounds,fbx_roundtrip_pass=True,
    fbx_sha256=hashlib.sha256((OUT/(NAME+'.fbx')).read_bytes()).hexdigest(),
    unreal_expected_bounds_cm=[[a*100,b*100] if i!=1 else [-b*100,-a*100] for i,(a,b) in enumerate(bounds)],
    runtime_scale=1.5,runtime_actor_z_cm=110,runtime_relative_z_cm=55,
    runtime_ground_z_cm=110+55+bounds[2][0]*150,baked_authoring_yaw_degrees=0,
    blender_entrance_front_normal=[0,1,0],unreal_entrance_front_normal=[0,-1,0],
    design='Faceted command tower; stepped octagonal dish roof; broad diagonal cheek armor; small doors; fitted armor panels; blue windows; low service wing; pipe bundles and roof rails',
    remaining_differences='Cleaner procedural surfaces than reference; original approximation of panel/pipe layout; static dish; destruction and authored LODs absent; runtime capture required'))
metadata_path.write_text(json.dumps(meta,indent=2)+'\n',encoding='utf-8')

# Genuine exported-geometry render from the gameplay-facing front quarter.
scene=bpy.context.scene;zmin,zmax=bounds[2];radius=max(model.dimensions)
c.box('Preview-only ground',(0,0,zmin-.09),(radius*4,radius*4,.16),'Concrete',0)
world=bpy.data.worlds.new('Faceted HQ neutral studio');scene.world=world;world.use_nodes=True
world.node_tree.nodes.clear();bg=world.node_tree.nodes.new('ShaderNodeBackground')
wo=world.node_tree.nodes.new('ShaderNodeOutputWorld');world.node_tree.links.new(bg.outputs[0],wo.inputs[0])
bg.inputs[0].default_value=(.22,.25,.29,1);bg.inputs[1].default_value=.5
for loc,power,size in [((-4,6,10),1600,7),((5,1,6),1100,5),((-2,-5,8),1800,5)]:
    bpy.ops.object.light_add(type='AREA',location=loc);light=bpy.context.object
    light.data.energy=power;light.data.shape='DISK';light.data.size=size
    light.rotation_euler=(Vector((0,0,2))-light.location).to_track_quat('-Z','Y').to_euler()
target=Vector((0,0,(zmin+zmax)*.48))
bpy.ops.object.camera_add(location=(-radius*1.2,radius*1.4,radius*1.15));cam=bpy.context.object
cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler()
cam.data.type='ORTHO';cam.data.ortho_scale=radius*1.65;scene.camera=cam
scene.render.engine='CYCLES';scene.cycles.samples=48;scene.cycles.use_denoising=True
scene.render.resolution_x=1200;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG';scene.render.filepath=str(OUT/(NAME+'.png'))
bpy.ops.render.render(write_still=True)
print('REFERENCE_HQ_V7_PASS '+json.dumps(meta),flush=True)
