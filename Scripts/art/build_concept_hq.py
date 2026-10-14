"""Original faceted command headquarters derived from HERO concept silhouettes."""
import sys, math
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from concept_asset_common import *
reset()
N='SM_HERO_CommandHub'
# Octagonal concrete pad and stepped perimeter skirting.
cone('Octagonal fortified plinth',(0,0,.13),3.45,3.45,.26,'Concrete',8)
cone('Armor perimeter rail',(0,0,.31),3.28,3.22,.14,'Dark',8)
cone('Central broad lower glacis',(0,0,1.08),2.26,1.91,1.42,'Armor',8)
cone('Upper command armor',(0,0,2.18),1.91,1.52,.84,'Armor',8)
cone('Flat recessed roof',(0,0,2.68),1.53,1.53,.16,'Dark',8)
# Each exterior facet receives an independent bevelled sloped armor plate and inset light.
for i in range(8):
    a=math.pi/8+i*math.pi/4
    x,y=math.cos(a),math.sin(a)
    rot=(0,math.radians(13),a)
    box('Lower sloping facet %d'%i,(2.015*x,2.015*y,1.34),(.13,1.32,1.44),'Steel',.035,rot)
    box('Layered armored facet %d'%i,(2.08*x,2.08*y,1.37),(.075,1.16,1.25),'Armor',.025,rot)
    box('Inset team stripe %d'%i,(2.15*x,2.15*y,1.93),(.018,.87,.035),'Neon',.006,(0,0,a))
    box('Upper bevel inset %d'%i,(1.73*x,1.73*y,2.34),(.045,1.01,.30),'Steel',.018,(0,math.radians(25),a))
    box('Upper narrow identity light %d'%i,(1.78*x,1.78*y,2.35),(.03,.63,.032),'Neon',.004,(0,0,a))
    for side in [-1,1]:
        bx=2.13*x+side*.46*-y; by=2.13*y+side*.46*x
        cylinder('Armor captive screw',(bx,by,2.03),.035,.025,'Dark',vertices=6)
# Central roof communications assembly.
cylinder('Turret bearing',(0,0,2.84),1.19,.20,'Steel',vertices=32)
cylinder('Turret ring recess',(0,0,2.96),1.10,.10,'Dark',vertices=32)
torus('Continuous upper command light',(0,0,3.015),1.035,.035,'Neon')
cone('Command turret sloped drum',(0,0,3.20),1.02,.83,.36,'Armor',16)
cylinder('Turret cap',(0,0,3.43),.85,.11,'Steel',vertices=16)
cylinder('Recessed central roof',(0,0,3.49),.73,.045,'Dark',vertices=16)
for i in range(12):
    a=i*math.tau/12
    box('Turret inspection window',(.935*math.cos(a),.935*math.sin(a),3.22),(.035,.25,.13),'Glass',.008,(0,0,a))
    beam('Radial roof reinforcement',(.46*math.cos(a),.46*math.sin(a),3.53),(.78*math.cos(a),.78*math.sin(a),3.53),.03,'Armor')
# Tall central telemetry antenna: substantial base, segmented core and green slots.
cylinder('Antenna socket',(.05,.12,3.64),.29,.28,'Steel',vertices=12)
cone('Telemetry pylon',(.05,.12,4.45),.18,.115,1.44,'Armor',8)
for zz in [3.94,4.18,4.42,4.66,4.90]:
    box('Telemetry green slot',(.05,-.032,zz),(.085,.022,.105),'Neon',.004)
cylinder('Antenna upper rod',(.05,.12,5.48),.027,.78,'Steel',vertices=12)
cylinder('Antenna tip beacon',(.05,.12,5.92),.045,.07,'Lamp',vertices=12)
beam('Telemetry aerial',(.05,.12,5.34),(.47,.12,5.34),.018)
# Main entry facade faces -Y, with inclined portal armor and layered lintels.
box('Entry vestibule',(0,-1.94,.99),(1.14,.56,1.44),'Dark',.07)
box('Closed blast door',(0,-2.235,.98),(.73,.06,1.17),'Armor',.018)
for sx in [-1,1]:
    box('Armored portal column',(sx*.62,-2.13,1.20),(.23,.43,1.86),'Steel',.035,(0,sx*math.radians(-12),0))
    box('Column armor insert',(sx*.62,-2.36,1.24),(.10,.035,1.18),'Armor',.012)
    box('Door jamb light',(sx*.44,-2.281,1.26),(.025,.018,.58),'Neon',.003)
box('Overhead entry brow',(0,-2.18,2.07),(1.45,.48,.20),'Steel',.05)
box('Portal status strip',(0,-2.43,2.04),(.71,.03,.037),'Neon',.004)
box('Door seam',(0,-2.28,.96),(.014,.018,1.08),'Dark',0)
for sx in [-1,1]: box('Blast door panel',(sx*.19,-2.277,1.06),(.30,.027,.64),'Steel',.008)
for j in range(4):
    box('Approach step',(0,-2.43-j*.23,.31-j*.045),(1.36,.31,.10),'Concrete',.016)
    box('Step safety edge',(0,-2.58-j*.23,.36-j*.045),(1.30,.027,.015),'Dust',.004)
# Back technical towers asymmetry, articulated setbacks and service mounts.
for label,x,y,h in [('Left',-2.18,.89,3.63),('Right',2.13,1.13,3.04)]:
    box(label+' tower footing',(x,y,.47),(1.24,1.19,.40),'Dark',.065)
    box(label+' armored tower',(x,y,.60+h/2),(.91,.96,h),'Armor',.065)
    box(label+' recessed tower roof',(x,y,.65+h),(.80,.84,.12),'Steel',.025)
    cone(label+' tower taper',(x,y,.90+h),.52,.40,.40,'Armor',4)
    cylinder(label+' roof service dish',(x,y,1.15+h),.27,.11,'Dark',vertices=16)
    for zz in [.94,1.61,2.27,2.85]:
        if zz<h:
            box(label+' segmented facade',(x,y-.505,zz),(.74,.07,.48),'Steel',.012)
            box(label+' inset facade',(x,y-.549,zz),(.61,.024,.37),'Armor',.009)
            box(label+' tower green slot',(x+.21,y-.568,zz),(.04,.020,.23),'Neon',.004)
    vent(label+' tower cooling',(x,y-.554,1.61),.43,.30,count=6)
    for zz in [1.09,2.40]:
        box(label+' flank armored plate',(x+(.49 if x>0 else -.49),y,zz),(.055,.71,.55),'Steel',.02)
    cylinder(label+' rooftop antenna',(x,y+.19,1.70+h),.014,1.1,'Steel',vertices=8)
# Lower independent utilities, pipes, fuel containers and wall vents.
for x in [-2.55,2.55]:
    box('Ground service pod',(x,-.95,.69),(.61,.82,.77),'Armor',.06)
    box('Pod lid',(x,-.95,1.09),(.66,.86,.10),'Steel',.025)
    vent('Ground service pod vent',(x,-1.38,.72),.43,.46,count=8)
    box('Pod supply status',(x,-1.417,1.015),(.26,.021,.028),'Neon',.004)
    for yy in [-.70,-1.15]: beam('Pod protective bar',(x-.25,yy,1.14),(x+.25,yy,1.14),.025)
    beam('Visible service conduit',(x,-.55,.50),(x,0,.50),.065,'Dark')
    beam('Conduit elbow to wall',(x,0,.50),(x*.77,0,.78),.065,'Steel')
for x in [-.84,.84]:
    box('Rear HVAC module',(x,1.66,1.51),(.58,.47,.72),'Steel',.035)
    for j in range(5): box('HVAC fan ribs',(x,1.915,1.24+j*.12),(.50,.04,.03),'Dark',.006)
    beam('Rear vertical pipe',(x,2.02,.40),(x,2.02,2.03),.047)
# Perimeter service covers, edge lighting and parking markings (geometry).
for i in range(8):
    a=i*math.pi/4+math.pi/8
    x,y=2.92*math.cos(a),2.92*math.sin(a)
    box('Plinth light',(x,y,.275),(.24,.08,.025),'Neon',.008,(0,0,a+math.pi/2))
for x in [-1.5,1.5]:
    box('Service cover',(x,-2.37,.28),(.34,.50,.025),'Steel',.012)
    bolts('Cover screws',(x-.12,-2.53,.307),axis='Y',spacing=.11,count=4)
# Machined service access panels and captive fasteners break broad armor faces.
for i in range(8):
    a=math.pi/8+i*math.pi/4; x,y=math.cos(a),math.sin(a)
    for side in [-1,1]:
        px=2.10*x+side*.37*-y; py=2.10*y+side*.37*x
        box('Inset armor seam',(px,py,1.35),(.033,.018,.76),'Dark',.003,(0,math.radians(13),a))
        for zz in [.91,1.77]:
            beam('Captive armor bolt',(px,py,zz),(px+.035*x,py+.035*y,zz),.018,'Steel')
    box('Lower facet service plate',(2.19*x,2.19*y,.78),(.055,.64,.15),'Dark',.009,(0,0,a))
    box('Service inset metal',(2.223*x,2.223*y,.78),(.02,.49,.068),'Steel',.003,(0,0,a))
for label,x,y,h in [('Left',-2.18,.89,3.63),('Right',2.13,1.13,3.04)]:
    vent(label+' upper intake grille',(x,y-.54,h+.20),.65,.42,count=9)
    sign=1 if x>0 else -1
    for k in range(6):
        box(label+' flank radiator rib',(x+sign*.495,y-.26+k*.10,h-.17),(.05,.042,.49),'Dark',.006)
    beam(label+' outside service cable',(x+sign*.57,y+.28,.65),(x+sign*.57,y+.28,h+.47),.037,'Dark')
    for zz in [1.03,1.94,2.87]:
        box(label+' cable clamp',(x+sign*.57,y+.28,zz),(.12,.10,.065),'Steel',.008)
mesh=export(N,zoffset=-1.1)
preview(N,mesh)
