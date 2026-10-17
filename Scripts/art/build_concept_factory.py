"""Original HERO military vehicle plant, modelled from faction concept language."""
import sys, math
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from concept_asset_common import *

reset()
box('Chamfered reinforced foundation',(0,0,.08),(4.4,4,.16),'Concrete',.09)
box('Foundation armor plinth',(0,.08,.22),(3.85,3.48,.22),'Dark',.08)
# The garage is a real empty chamber: independent walls, roof and recessed back wall.
box('Workshop rear wall',(0,1.38,1.25),(2.65,.24,2.05),'Armor',.06)
box('Workshop left shell',(-1.18,.18,1.25),(.34,2.55,2.05),'Armor',.05)
box('Workshop right shell',(1.18,.18,1.25),(.34,2.55,2.05),'Armor',.05)
box('Workshop ceiling',(0,.18,2.26),(2.72,2.75,.30),'Armor',.07)
box('Vehicle bay dark floor',(0,-.05,.36),(2.02,2.95,.12),'Dark',.015)
for x in [-.59,.59]:
    box('Service track',(x,-.15,.435),(.12,2.6,.03),'Steel',.004)
    for y in [-1.3,-.95,-.6,-.25,.1,.45,.8,1.15]:
        box('Track tread',(x,y,.459),(.20,.05,.022),'Steel',.002)
box('Recessed armored bulkhead',(0,1.215,1.14),(2.0,.08,1.34),'Dark',.025)
for x in [-.55,.55]:
    box('Inner maintenance bay',(x,1.155,1.10),(.95,.09,1.18),'Armor',.03)
    box('Inner machinery access',(x,1.09,1.03),(.66,.04,.78),'Steel',.02)
    vent('Inner machinery grille',(x,1.055,1.16),.57,.30)
# Thick sculpted portal with angled buttress armor.
for x in [-1.11,1.11]:
    box('Portal steel edge',(x,-1.22,1.19),(.22,.34,1.68),'Steel',.04)
    box('Portal armored upright',(x*1.13,-1.29,1.15),(.36,.42,1.80),'Armor',.05)
    box('Vertical team inset',(x*1.13,-1.516,1.28),(.12,.025,.99),'Dark',.007)
    box('Portal guidance light',(x*1.13,-1.536,1.29),(.035,.012,.85),'Neon',.006)
    box('Sloped portal shoulder',(x*.94,-1.19,2.04),(.40,.49,.57),'Armor',.04,rot=(0,(-1 if x<0 else 1)*.44,0))
    for z in [.57,.9,1.8]:
        cylinder('Portal fastening',(x*1.13,-1.528,z),.035,.025,'Steel',rot=(math.pi/2,0,0),vertices=6)
box('Portal lintel',(0,-1.23,2.10),(2.25,.45,.34),'Steel',.04)
box('Roller door recessed header',(0,-1.03,1.95),(1.88,.19,.16),'Dark',.02)
for i in range(6):
    box('Retracted shutter segment',(0,-1.16,1.955+i*.023),(1.90,.04,.014),'Armor',.002)
box('Front ID recess',(0,-1.472,2.12),(1.0,.025,.15),'Dark',.01)
for x in [-.34,-.20,-.06,.08,.22,.36]:
    box('Plant identifier bars',(x,-1.49,2.12),(.05,.01,.085),'Team',.002)
box('Garage approach ramp',(0,-1.60,.28),(2.02,.86,.16),'Steel',.012,rot=(.15,0,0))
for x in [-.94,.94]:
    box('Approach rail',(x,-1.60,.386),(.025,.76,.025),'Steel',.004,rot=(.15,0,0))
for x in [i*.17-.85 for i in range(11)]:
    box('Threshold warning stripe',(x,-1.95,.264),(.075,.19,.012),'Lamp',.002,rot=(0,0,-.36))
for y in [-1.8,-1.58,-1.36]:
    box('Ramp grip ribs',(0,y,.30+(y+1.95)*.15),(1.78,.026,.018),'Dark',.003)
# Broad sloped side armor panels have overlapping steel frames and protected light channels.
for side in [-1,1]:
    x=side*1.39
    box('Slope armor structural rib',(x,.22,1.40),(.33,2.4,1.85),'Steel',.035,rot=(0,side*.18,0))
    for y in [-.70,.10,.90]:
        box('Slope individual armor plate',(x+side*.10,y,1.39),(.20,.71,1.64),'Armor',.035,rot=(0,side*.18,0))
        box('Panel recessed light groove',(x+side*.265,y,1.58),(.028,.52,.18),'Dark',.006)
        box('Panel green guidance',(x+side*.282,y,1.60),(.016,.41,.035),'Neon',.005)
        box('Panel lower abrasion guard',(x+side*.18,y,.66),(.08,.55,.14),'Steel',.015)
        for yy in [y-.25,y+.25]:
            cylinder('Panel bolt',(x+side*.25,yy,1.12),.028,.025,'Steel',rot=(0,math.pi/2,0),vertices=6)
# Multi-level roof and radiator assembly.
box('Roof raised armored spine',(0,.18,2.49),(1.9,2.18,.20),'Steel',.06)
box('Roof dark thermal deck',(0,.18,2.61),(1.77,1.94,.14),'Dark',.035)
for y in [-.40,.39,.90]:
    box('Thermal panel',(0,y,2.70),(1.55,.42,.08),'Armor',.035)
    for x in [-.57,-.38,-.19,0,.19,.38,.57]:
        box('Radiator fin',(x,y,2.762),(.07,.32,.06),'Steel',.008)
for x in [-.96,.96]:
    box('Roof team stripe',(x,.12,2.609),(.04,1.88,.018),'Neon',.004)
for y in [-.88,1.19]:
    bolts('Roof bolts',(-.68,y,2.71),spacing=.34,count=5)
cylinder('Roof reactor mounting',(0,.25,2.83),.40,.20,'Armor',vertices=12)
torus('Reactor neon ring',(0,.25,2.915),.34,.025,'Neon')
cylinder('Reactor exhaust',(0,.25,3.08),.24,.30,'Steel',vertices=12)
cone('Armored reactor crown',(0,.25,3.25),.32,.20,.15,'Armor',vertices=12)
for x in [-.15,.15]:
    beam('Antenna',(x,1.12,2.69),(x,1.12,3.33),.018,'Steel')
# A compact asymmetric power annex distinguishes this from a generic hangar.
box('Power annex foot',(1.90,.58,.35),(.51,1.78,.25),'Steel',.045)
box('Power annex armored shell',(1.92,.60,1.14),(.48,1.50,1.37),'Armor',.045)
box('Power annex upper shoulder',(1.90,.60,1.89),(.44,1.33,.27),'Steel',.04)
for y in [.1,.6,1.1]:
    box('Annex inset plate',(2.165,y,1.15),(.03,.36,.86),'Dark',.01)
    box('Annex vertical green slit',(2.185,y,1.23),(.013,.038,.60),'Neon',.005)
vent('Annex front radiator',(1.92,-.165,1.45),.36,.41)
for y in [.13,.59,1.05]:
    cylinder('Annex roof power cell',(1.92,y,2.09),.13,.20,'Dark',vertices=16)
    torus('Power cell rim',(1.92,y,2.19),.12,.02,'Steel')
for y in [.15,.88]:
    beam('Power connection',(1.55,y,1.05),(1.72,y,1.05),.055,'Dark')
    torus('Power flange',(1.70,y,1.05),.07,.012,'Steel',rot=(0,math.pi/2,0))
# External crane on opposite flank, with real triangular lattice and pulley cable.
for y in [-.67,1.04]:
    box('Crane mounting shoe',(-1.98,y,.37),(.34,.37,.17),'Steel',.025)
    beam('Crane column',(-1.98,y,.40),(-1.98,y,2.45),.055,'Steel')
    for z in [.52,1.14,1.76]:
        beam('Crane lattice diagonal',(-2.04,y,z),(-1.82,y,z+.58),.022,'Armor')
        beam('Crane lattice opposing',(-1.82,y,z),(-2.04,y,z+.58),.022,'Armor')
box('Gantry upper rail',(-1.95,.18,2.48),(.22,2.10,.17),'Armor',.025)
box('Gantry inset rail',(-1.95,.18,2.53),(.08,1.89,.035),'Steel',.008)
box('Gantry trolley',(-1.95,-.3,2.45),(.30,.34,.20),'Steel',.025)
beam('Suspended hoist cable',(-1.95,-.3,2.33),(-1.95,-.3,1.49),.011,'Dark')
torus('Hoist hook',(-1.95,-.3,1.44),.065,.022,'Steel',rot=(math.pi/2,0,0))
# Ground utility items, conduit and foundation seams.
for x,y in [(-1.74,-1.40),(1.72,-1.52),(-1.75,1.64),(1.8,1.6)]:
    box('Equipment crate',(x,y,.35),(.33,.28,.30),'Armor',.025)
    box('Crate steel band',(x,y,.41),(.345,.10,.15),'Steel',.01)
for x in [-2.12,2.12]:
    box('Foundation outline',(x,0,.172),(.016,3.80,.012),'Dust',.002)
for y in [-1.88,1.88]:
    box('Foundation outline',(0,y,.172),(4.2,.016,.012),'Dust',.002)
for x in [-1.8,-.9,0,.9,1.8]:
    box('Concrete expansion joint',(x,0,.169),(.008,3.75,.009),'Dark',0)
mesh=export('SM_HERO_VehiclePlant',-.35)
preview('SM_HERO_VehiclePlant',mesh)
