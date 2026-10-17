"""Original eight-wheel armored vehicle derived from HERO green faction concept."""
import sys, math
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from concept_asset_common import *
reset()

def hull(name,rings,mat):
    verts=[]
    for x,w,low,high in rings:
        verts.extend([(x,-w,low),(x,w,low),(x,w*.80,high),(x,-w*.80,high)])
    faces=[(3,2,1,0)]
    for i in range(len(rings)-1):
        for j in range(4): faces.append((i*4+j,i*4+(j+1)%4,(i+1)*4+(j+1)%4,(i+1)*4+j))
    n=(len(rings)-1)*4; faces.append((n,n+1,n+2,n+3))
    data=bpy.data.meshes.new(name); data.from_pydata(verts,[],faces);data.update()
    o=bpy.data.objects.new(name,data);bpy.context.collection.objects.link(o)
    return bevel(finish(o,name,mat),.025,2)

hull('Welded lower hull',[(-1.65,.62,.38,.74),(-1.2,.72,.34,.85),(.85,.72,.34,.85),(1.62,.58,.45,.62)],'Dark')
hull('Faceted upper hull',[(-1.58,.67,.65,1.0),(-1.14,.68,.74,1.15),(.80,.66,.72,1.19),(1.56,.53,.52,.89)],'Armor')
box('Rear engine deck',(-1.10,0,1.15),(.85,1.07,.065),'Steel',.016)
for i in range(8): box('Engine grille louver',(-1.4+i*.075,0,1.195),(.037,.78,.025),'Dark',.006)
for side in (-1,1):
    for i,x in enumerate((-1.17,-.39,.40,1.15)):
        cylinder('Offroad tire',(x,side*.77,.36),.36,.24,'Rubber',(math.pi/2,0,0),28)
        cylinder('Armored wheel hub',(x,side*.903,.36),.19,.04,'Steel',(math.pi/2,0,0),16)
        cylinder('Hub cap',(x,side*.930,.36),.095,.02,'Armor',(math.pi/2,0,0),8)
        for t in range(16):
            a=t*math.tau/16
            box('Chevron tire block',(x+.351*math.sin(a),side*.77,.36+.351*math.cos(a)),(.10,.255,.032),'Rubber',.004,(0,a,side*.15))
        beam('Suspension swing arm',(x-.17,side*.55,.56),(x,side*.70,.36),.048,'Dark')
    box('Wheel protection rail',(0,side*.73,.79),(3.10,.10,.13),'Armor',.025)
    for i,x in enumerate((-.92,-.28,.36,.94)):
        box('Side applique armor',(x,side*.699,.99),(.54,.068,.29),'Steel',.014,(side*-.12,0,0))
        box('Inset side panel',(x,side*.741,.99),(.42,.026,.20),'Armor',.008)
        for dx in (-.17,.17):
            cylinder('Panel bolt',(x+dx,side*.765,1.06),.018,.02,'Steel',(math.pi/2,0,0),6)
    box('Faction strip',(.0,side*.736,.865),(1.88,.025,.035),'Neon',.004)
    box('Side glass',(.94,side*.592,1.125),(.29,.025,.10),'Glass',.009)
    beam('Tow rail',(-1.65,side*.43,.53),(-1.65,side*.43,.91),.03)
    box('Front headlamp',(1.55,side*.39,.72),(.035,.19,.085),'Lamp',.004)
    box('Tail lamp',(-1.635,side*.44,.78),(.025,.11,.06),'Lamp',.004)
    cylinder('Rear exhaust',(-1.36,side*.49,1.23),.07,.17,'Dark')
    torus('Exhaust rim',(-1.36,side*.49,1.315),.07,.014,'Steel')
box('Driver viewport',(1.01,0,1.073),(.43,.91,.033),'Glass',.012,(0,.39,0))
box('Forward glacis seam',(1.31,0,.86),(.04,.96,.028),'Steel',.006,(0,.39,0))
box('Tow bumper',(1.66,0,.47),(.12,1.27,.14),'Steel',.025)
for y in (-.43,.43): torus('Tow eye',(1.73,y,.49),.064,.018,'Dark',(0,math.pi/2,0))
cylinder('Turret race',(.12,0,1.22),.55,.12,'Dark',vertices=32)
cone('Low angular turret',(.14,0,1.42),.63,.48,.31,'Armor',8)
torus('Turret faction rim',(.14,0,1.30),.53,.013,'Neon')
box('Gun armored mantlet',(.68,0,1.44),(.39,.35,.29),'Steel',.032)
beam('Main cannon',(.76,0,1.45),(1.67,0,1.45),.055,'Dark')
beam('Barrel heat sleeve',(.86,0,1.45),(1.22,0,1.45),.077,'Steel')
box('Muzzle brake',(1.72,0,1.45),(.19,.12,.12),'Steel',.012)
box('Muzzle opening',(1.824,0,1.45),(.009,.063,.068),'Dark',.002)
cylinder('Commander hatch',(.02,0,1.63),.21,.035,'Steel',vertices=16)
box('Periscope',(.16,0,1.70),(.13,.21,.09),'Dark',.01)
box('Periscope glass',(.233,0,1.705),(.012,.15,.035),'Glass',.003)
for side in (-1,1):
    for i in range(3):
        cylinder('Smoke launcher',(.06-i*.11,side*.46,1.49),.045,.20,'Dark',(side*.60,0,0),12)
beam('Whip antenna',(-.3,.22,1.62),(-.42,.22,2.25),.008,'Steel')
mesh=export('SM_HERO_Striker',-.35)
preview('SM_HERO_Striker',mesh)
