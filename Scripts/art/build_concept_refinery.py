"""Original HERO armored salvage processing depot, based on supplied visual concept."""
import sys, math
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parent))
from concept_asset_common import *

reset()
# Octagonal foundation, armored circular pressure vessel and heavy structural ribs.
cone('Octagonal concrete footing',(0,0,.16),1.79,1.79,.32,'Concrete')
cone('Raised steel perimeter',(0,0,.36),1.68,1.63,.13,'Steel')
cone('Pressure chamber',(0,.08,1.23),1.28,1.25,1.63,'Dark',16)
cone('Lower armored belt',(0,.08,.62),1.40,1.37,.31,'Armor',16)
cone('Upper armored belt',(0,.08,1.96),1.34,1.30,.31,'Armor',16)
cone('Sloping reactor crown',(0,.08,2.26),1.3,.82,.43,'Armor',16)
for i in range(12):
    ang=2*math.pi*i/12
    x,y=math.sin(ang),math.cos(ang)
    # Keep front clear for a real vehicle receiving bay.
    if y < -.75: continue
    rot=(0,0,-ang)
    box('Structural armored pilaster',(x*1.28,.08+y*1.28,1.3),(.19,.22,1.51),'Steel',.03,rot)
    box('Segmented facade armor',(x*1.29,.08+y*1.29,1.28),(.48,.07,.82),'Armor',.025,rot)
    box('Vertical green reactor strip',(x*1.335,.08+y*1.335,1.27),(.047,.025,.91),'Neon',.009,rot)
    box('Lower stripe guard',(x*1.36,.08+y*1.36,.76),(.38,.09,.095),'Dark',.015,rot)
    box('Upper stripe guard',(x*1.36,.08+y*1.36,1.77),(.38,.09,.095),'Dark',.015,rot)
    for zz in (.82,1.73):
        cylinder('Armor hex fastener',(x*1.385,.08+y*1.385,zz),.032,.028,'Steel',rot=(math.pi/2,0,-ang),vertices=6)
# Four visible processing columns with pipe manifolds and service platforms.
for i,(x,y) in enumerate([(-1.08,-.50),(1.08,-.50),(-.94,.93),(.94,.93)]):
    cylinder('Reservoir pressure cylinder',(x,y,1.37),.30,1.49,'Steel',vertices=24)
    cone('Reservoir armored cap',(x,y,2.16),.35,.20,.24,'Armor',12)
    for z in (.67,1.98):
        torus('Clamped reservoir flange',(x,y,z),.315,.042,'Armor')
    torus('Luminous reservoir seal',(x,y,1.75),.309,.028,'Neon')
    box('Reservoir shield',(x,y-.30,1.25),(.43,.10,.85),'Armor',.04)
    box('Reservoir status indicator',(x,y-.357,1.28),(.045,.025,.61),'Neon',.009)
    beam('Reservoir outlet',(x,y,2.27),(x,y,2.55),.073)
    beam('Manifold connection',(x,y,2.55),(x*.53,y*.53,2.55),.073)
    torus('Manifold outlet flange',(x,y,2.37),.105,.022)
    beam('Protective handrail',(x-.35,y-.38,2.24),(x+.35,y-.38,2.24),.025)
    for xx in (x-.35,x+.35): beam('Rail stanchion',(xx,y-.38,1.98),(xx,y-.38,2.25),.024)
# Offset cooling tower avoids a generic symmetric block silhouette.
box('Roof mechanical house',(0,.39,2.66),(1.31,1.12,.66),'Armor',.06)
vent('Roof intake',(0,-.18,2.72),1.02,.38)
box('Roof armored cover',(0,.39,3.02),(1.47,1.26,.13),'Steel',.035)
cylinder('Cooling riser',(.31,.55,3.46),.29,.78,'Dark',vertices=16)
for z in (3.10,3.22,3.68,3.84):
    cylinder('Cooling ring',(.31,.55,z),.35,.065,'Steel',vertices=16)
for i in range(8):
    a=2*math.pi*i/8
    box('Tower rib',(.31+math.sin(a)*.294,.55+math.cos(a)*.294,3.48),(.055,.09,.52),'Armor',.01,(0,0,-a))
    box('Cooling tower luminous slit',(.31+math.sin(a)*.304,.55+math.cos(a)*.304,3.46),(.025,.035,.34),'Neon',.004,(0,0,-a))
cylinder('Exhaust aperture',(.31,.55,3.89),.24,.016,'Dark',vertices=24)
for x in (-.60,-.33):
    beam('Secondary exhaust',(x,.75,3.04),(x,.75,3.48),.075)
    cylinder('Exhaust mouth',(x,.75,3.50),.105,.07,'Steel')
box('Maintenance roof hatch',(-.42,.21,3.10),(.46,.47,.09),'Dark',.02)
bolts('Hatch bolts',(-.60,.03,3.165),spacing=.12,count=4)
# Open intake tunnel, ramp, separate side walls: legible collection gameplay role.
box('Intake tunnel black interior',(0,-1.20,.98),(.95,.065,.95),'Dark',0)
for x in (-.62,.62):
    box('Intake armored wall',(x,-1.31,.96),(.27,.81,1.12),'Armor',.06)
    box('Intake wall raised panel',(x,-1.735,1.01),(.20,.045,.68),'Steel',.025)
    box('Intake vertical guide lamp',(x,-1.762,1.10),(.032,.022,.51),'Neon',.005)
    beam('Canopy hydraulic support',(x,-1.35,1.46),(x,-1.72,1.86),.045)
box('Receiving canopy',(0,-1.42,1.68),(1.53,1.13,.22),'Steel',.05)
box('Canopy armored roof',(0,-1.35,1.84),(1.40,.84,.15),'Armor',.035)
box('Intake status bar',(0,-2.00,1.68),(.71,.023,.047),'Neon',.006)
box('Receiving grated deck',(0,-1.35,.46),(1.12,.96,.10),'Dark',.016)
for i in range(10):
    box('Loading deck grate',(0,-1.79+i*.09,.521),(1.02,.024,.015),'Steel',.003)
box('Loading ramp',(0,-1.78,.32),(1.12,.50,.15),'Steel',.015,(-.32,0,0))
for x in (-.54,.54):
    box('Ramp safety curb',(x,-1.77,.40),(.055,.55,.075),'Lamp',.008,(-.32,0,0))
for i in range(8):
    x=-.59+i*.17
    box('Canopy yellow caution tab',(x,-1.99,1.54),(.09,.028,.068),'Lamp',.003,(0,-.35,0))
# Service pipes and access ladder on the rear.
for x in (-.53,.53):
    beam('Rear circulation vertical',(x,1.41,.64),(x,1.41,2.29),.065)
    for z in (.73,1.35,2.16): torus('Pipe bolted coupling',(x,1.41,z),.090,.017)
    beam('Horizontal return pipe',(x,1.41,2.29),(x*.4,1.01,2.29),.065)
    cylinder('Valve handwheel',(x,1.49,1.40),.12,.022,'Team',rot=(math.pi/2,0,0),vertices=12)
for x in (-.19,.19): beam('Ladder side',(x,1.43,.45),(x,1.43,3.06),.022)
for z in [ .55+i*.21 for i in range(12) ]: beam('Ladder rung',(-.19,1.45,z),(.19,1.45,z),.018)
for x in (-1.52,1.52):
    box('Perimeter utility cabinet',(x,.16,.75),(.25,.55,.61),'Armor',.03)
    vent('Cabinet cooling',(x,.16,.81),.42,.32,axis='X',count=5)
    box('Ground team insignia',(x,-.80,.444),(.12,.35,.008),'Team',.003)

mesh=export('SM_HERO_Refinery',-.35)
preview('SM_HERO_Refinery',mesh)
