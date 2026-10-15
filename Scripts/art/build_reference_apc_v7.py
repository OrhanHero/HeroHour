"""Original concept-matched Aegis 8x8 APC. Run Blender --background --python this.py."""
import sys,math,json
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
import concept_asset_common as c
from concept_asset_common import bpy,Vector
ROOT=Path(__file__).resolve().parents[2]; OUT=ROOT/'Art/Models/ReferenceAPCV7'; OUT.mkdir(parents=True,exist_ok=True)
c.OUT=OUT;c.PREVIEW=OUT
c.PALETTE.update({'Armor':((.39,.32,.21),.48,.63),'Steel':((.20,.22,.22),.75,.48),'Team':((.025,.18,.48),.55,.48),'Neon':((.025,.35,.8),.25,.3)})
c.reset()
def hull(name,rings,mat):
    vs=[]
    for x,w,lo,hi in rings: vs.extend([(x,-w,lo),(x,w,lo),(x,w*.82,hi),(x,-w*.82,hi)])
    fs=[(3,2,1,0)]
    for i in range(len(rings)-1):
        for j in range(4):fs.append((i*4+j,i*4+(j+1)%4,(i+1)*4+(j+1)%4,(i+1)*4+j))
    n=(len(rings)-1)*4;fs.append((n,n+1,n+2,n+3))
    me=bpy.data.meshes.new(name);me.from_pydata(vs,[],fs);me.update();o=bpy.data.objects.new(name,me);bpy.context.collection.objects.link(o)
    return c.bevel(c.finish(o,name,mat),.022,2)
# A single low armoured envelope with sloping nose; no exposed pickup cabin.
body=hull('Continuous low welded hull',[(-1.72,.70,.35,1.02),(-1.40,.78,.32,1.14),(.93,.77,.32,1.13),(1.63,.62,.50,.80)],'Armor')
# Boolean recesses extend into the hull, leaving genuine cavities behind the wheels.
for s in (-1,1):
    for x in (-1.21,-.42,.39,1.16):
        bpy.ops.mesh.primitive_cylinder_add(vertices=32,radius=.405,depth=.40,location=(x,s*.80,.365),rotation=(math.pi/2,0,0))
        cut=bpy.context.object;mod=body.modifiers.new('Recessed wheel arch','BOOLEAN');mod.operation='DIFFERENCE';mod.object=cut
        bpy.context.view_layer.objects.active=body;bpy.ops.object.modifier_apply(modifier=mod.name);bpy.data.objects.remove(cut,do_unlink=True)
        c.cylinder('All terrain tire',(x,s*.80,.35),.35,.255,'Rubber',(math.pi/2,0,0),32)
        c.cylinder('Inset hub',(x,s*.937,.35),.205,.025,'Dark',(math.pi/2,0,0),20)
        c.cylinder('Armored hub',(x,s*.953,.35),.153,.025,'Steel',(math.pi/2,0,0),12)
        c.cylinder('Hub axle cap',(x,s*.972,.35),.065,.018,'Armor',(math.pi/2,0,0),8)
        for j in range(22):
            a=j*math.tau/22
            c.box('Heavy tire tread',(x+.347*math.sin(a),s*.80,.35+.347*math.cos(a)),(.077,.27,.028),'Rubber',0,(0,a,s*.15))
        for j in range(6):
            a=j*math.tau/6;c.cylinder('Lug bolt',(x+.112*math.sin(a),s*.975,.35+.112*math.cos(a)),.013,.017,'Steel',(math.pi/2,0,0),6)
    for x in (-1.17,-.42,.34,1.02):
        c.box('Side applique panel',(x,s*.743,.94),(.62,.08,.23),'Armor',.025,(s*-.15,0,0))
        c.box('Panel seam',(x+.27,s*.79,.96),(.018,.012,.16),'Dark',.002)
    c.box('Blue identifier',(-.85,s*.791,1.00),(.24,.015,.065),'Team',.003)
    c.box('Step rail',(-.30,s*.83,.76),(1.68,.06,.04),'Steel',.008)
    c.box('Headlamp',(1.585,s*.45,.75),(.035,.17,.07),'Lamp',.005)
    c.box('Tail lamp',(-1.739,s*.48,.76),(.02,.09,.05),'Lamp',.002)
    c.beam('Rear grab handle',(-1.75,s*.35,.54),(-1.75,s*.35,.85),.018)
c.box('Rear troop ramp',(-1.745,0,.72),(.035,.94,.56),'Steel',.015)
c.box('Ramp recessed armor',(-1.77,0,.72),(.02,.79,.43),'Armor',.012)
c.box('Front bumper',(1.66,0,.49),(.13,1.2,.12),'Dark',.02)
for y in (-.42,.42):c.torus('Front tow eye',(1.74,y,.52),.047,.015,'Steel',(0,math.pi/2,0))
# Narrow vision prisms rather than the old large blue windscreen.
for y in (-.27,.27):c.box('Driver periscope',(1.04,y,1.064),(.19,.20,.035),'Glass',.005,(0,.37,0))
c.box('Rear cooling grille',(-1.20,0,1.16),(.65,.95,.045),'Dark',.012)
for j in range(9):c.box('Cooling louver',(-1.47+j*.065,0,1.19),(.032,.83,.018),'Steel',.003)
# Polygonal low turret with a broad rear bustle and slab cheeks, not a conical tank cap.
c.cylinder('Turret race',(.18,0,1.17),.46,.08,'Dark',vertices=24)
tur=hull('Faceted angular turret',[(-.50,.37,1.22,1.47),(-.24,.47,1.21,1.57),(.55,.42,1.23,1.49),(.73,.25,1.26,1.40)],'Armor')
for s in (-1,1):
    c.box('Turret cheek',(.23,s*.39,1.40),(.52,.09,.20),'Armor',.018,(s*-.17,0,0))
    c.box('Turret tactical marking',(-.22,s*.456,1.39),(.15,.017,.07),'Team',.002)
    for j in range(3):c.cylinder('Smoke discharger',(-.35+j*.10,s*.41,1.46),.033,.17,'Dark',(s*.65,0,0),10)
c.box('Cannon mantlet',(.69,0,1.38),(.26,.30,.23),'Steel',.02)
c.beam('Autocannon',(.76,0,1.39),(1.80,0,1.39),.038,'Dark')
c.beam('Cannon sleeve',(.80,0,1.39),(1.22,0,1.39),.060,'Steel')
c.box('Muzzle brake',(1.84,0,1.39),(.16,.10,.10),'Steel',.008)
c.box('Bore',(1.925,0,1.39),(.008,.052,.052),'Dark',.001)
c.cylinder('Commander hatch',(-.12,0,1.59),.18,.035,'Armor',vertices=12)
c.box('Roof optic',(.20,.10,1.61),(.15,.18,.10),'Dark',.008)
c.box('Optic lens',(.28,.10,1.61),(.012,.12,.045),'Glass',.003)
c.beam('Radio antenna',(-.37,-.20,1.52),(-.45,-.20,2.02),.006,'Steel')
for ob in [q for q in bpy.context.scene.objects if q.type=='MESH']:
    for slot in ob.material_slots:
        if slot.material is None: slot.material=bpy.data.materials['Armor']
# Keep visual width within the established navigation envelope.
for ob in [q for q in bpy.context.scene.objects if q.type=='MESH']:
    ob.location.y*=.955;ob.scale.y*=.955
for ob in [q for q in bpy.context.scene.objects if q.type=='MESH']: ob.location.z+=.011
mesh=c.export('SM_HR_Aegis_APC8',-.35)
meta=json.loads((OUT/'SM_HR_Aegis_APC8.json').read_text());meta['wheel_count']=8;meta['authoring_direction']='front +X, wheel axes Y';meta['reference']='Codex-Bild 1. Sept. 2026, 01_22_20.png, right tan eight-wheel AFV';meta['pivot']='ground at -0.35 m; actor +35cm places tire tread at ground'
assert 10000<=meta['triangles']<=25000,meta['triangles']
# Render actual exported mesh, then independently reload its FBX and compare every axis.
c.preview('SM_HR_Aegis_APC8',mesh)
expected=[(min((mesh.matrix_world@Vector(v)).__getitem__(i) for v in mesh.bound_box),max((mesh.matrix_world@Vector(v)).__getitem__(i) for v in mesh.bound_box)) for i in range(3)]
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.ops.import_scene.fbx(filepath=str(OUT/'SM_HR_Aegis_APC8.fbx'))
o=next(x for x in bpy.context.scene.objects if x.type=='MESH');actual=[(min((o.matrix_world@Vector(v))[i] for v in o.bound_box),max((o.matrix_world@Vector(v))[i] for v in o.bound_box)) for i in range(3)]
# X-forward/Z-up FBX export mirrors Y when reimported; symmetric bounds must remain equal.
assert all(abs(a-b)<.002 for p,q in zip(actual,expected) for a,b in zip(p,q)),(expected,actual)
meta['fbx_roundtrip']={'passed':True,'bounds_m':actual,'mirror_y_checked':True};meta['validation']='PASS'
(OUT/'SM_HR_Aegis_APC8.json').write_text(json.dumps(meta,indent=2));print('REFERENCE_APC_V7_PASS',json.dumps(meta))
