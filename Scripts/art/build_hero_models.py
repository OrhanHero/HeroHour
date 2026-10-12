"""Original procedural HERO prototype models. Centimetre export; Blender source retained."""
import bpy, math, random, os
from pathlib import Path
from mathutils import Vector
ROOT = str(Path(__file__).resolve().parents[2] / 'Art/Models/Generated')
os.makedirs(ROOT, exist_ok=True)
random.seed(42)
materials = {}
for name, color, metal, rough in [
 ('Armor',(0.17,0.20,0.13),.65,.68),('Rust',(.30,.12,.055),.6,.8),
 ('Rubber',(.016,.019,.018),.0,.87),('Steel',(.12,.14,.15),.8,.42),
 ('Glass',(.025,.105,.115),.65,.25),('Concrete',(.24,.25,.23),0,.94),
 ('Dark',(.025,.032,.035),.35,.8),('Neon',(.04,.85,.57),.1,.3),
 ('Lamp',(.95,.62,.21),.0,.25),('Paint',(.60,.55,.33),.35,.7),
 ('Team',(.15,.34,.26),.5,.5)]:
 m=bpy.data.materials.new(name); m.diffuse_color=(*color,1); m.use_nodes=True
 p=m.node_tree.nodes.get('Principled BSDF'); p.inputs['Base Color'].default_value=(*color,1); p.inputs['Metallic'].default_value=metal; p.inputs['Roughness'].default_value=rough
 materials[name]=m

def clean():
 bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
def finish(obj,name,mat):
 obj.name=name; obj.data.materials.append(materials[mat]); return obj
def box(name,loc,size,mat='Steel',bevel=.025,rotation=None):
 bpy.ops.mesh.primitive_cube_add(size=1,location=loc); o=bpy.context.object; o.dimensions=size
 if rotation: o.rotation_euler=rotation
 bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
 if bevel:
  mod=o.modifiers.new('Machined edges','BEVEL'); mod.width=bevel; mod.segments=2
  bpy.context.view_layer.objects.active=o; bpy.ops.object.modifier_apply(modifier=mod.name)
 return finish(o,name,mat)
def cylinder(name,loc,radius,depth,mat='Steel',rot=(0,0,0),vertices=20):
 bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=radius,depth=depth,location=loc,rotation=rot)
 return finish(bpy.context.object,name,mat)
def beam(name,a,b,r=.025,mat='Steel'):
 delta=Vector(b)-Vector(a); o=cylinder(name,(Vector(a)+Vector(b))/2,r,delta.length,mat)
 o.rotation_euler=delta.to_track_quat('Z','Y').to_euler(); return o
def export(name,zoffset=0):
 meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
 for o in meshes: o.location.z+=zoffset
 bpy.ops.object.select_all(action='DESELECT')
 for o in meshes: o.select_set(True)
 bpy.context.view_layer.objects.active=meshes[0]; bpy.ops.object.join(); o=bpy.context.object; o.name=name
 bpy.context.scene.cursor.location=(0,0,0); bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
 bpy.ops.wm.save_as_mainfile(filepath=os.path.join(ROOT,name+'.blend'))
 bpy.ops.export_scene.fbx(filepath=os.path.join(ROOT,name+'.fbx'),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False)
 print('HERO_MODEL_EXPORTED',name,len(o.data.polygons))

# Salvaged armoured pickup / technical. Front is +X.
clean()
box('Chassis',(0,0,.44),(2.75,1.40,.22),'Dark')
box('Armored body',(.05,0,.70),(2.7,1.48,.48),'Armor',.07)
box('Engine hood',(.93,0,1.0),(.90,1.40,.23),'Armor',.055)
box('Cabin',(.12,0,1.22),(1.03,1.33,.80),'Armor',.07)
box('Windshield',(.655,0,1.35),(.04,1.12,.43),'Glass',.015,(0,-.12,0))
for side in (-1,1):
 box('Side glass',(.17,side*.682,1.38),(.75,.025,.38),'Glass',.012)
 box('Door',(.10,side*.73,.95),(.86,.065,.34),'Rust',.025)
 box('Door handle',(.0,side*.775,1.12),(.16,.035,.03),'Steel',.006)
 box('Side armor rear',(-.97,side*.71,.96),(.62,.1,.42),'Armor',.025)
 box('Runner',(.0,side*.83,.53),(1.4,.16,.10),'Steel')
 for x in (-.92,.95):
  cylinder('Offroad tire',(x,side*.80,.40),.40,.24,'Rubber',(math.pi/2,0,0),24)
  cylinder('Wheel hub',(x,side*.93,.40),.22,.02,'Steel',(math.pi/2,0,0))
  for i in range(12):
   a=i*math.tau/12
   box('Tire tread',(x+.38*math.sin(a),side*.80,.40+.38*math.cos(a)),(.12,.27,.065),'Rubber',.008,(0,a,0))
box('Truck bed',(-.94,0,.83),(.83,1.17,.08),'Rust')
box('Grille',(1.43,0,.82),(.07,.92,.28),'Dark')
for y in (-.35,-.21,-.07,.07,.21,.35): box('Grille bars',(1.47,y,.82),(.035,.035,.23),'Steel',.004)
box('Front bumper',(1.53,0,.51),(.18,1.65,.16),'Rust')
for y in (-.55,.55):
 box('Headlights',(1.44,y,1.0),(.035,.23,.13),'Lamp',.005)
 box('Team patch',(.34,y*1.39,.94),(.18,.02,.17),'Team',.005)
for side in (-1,1):
 beam('Roll cage rear',(-1.21,side*.51,.92),(-1.21,side*.51,1.52),.035)
 beam('Roll cage top',(-1.21,side*.51,1.52),(-.55,side*.51,1.52),.035)
beam('Roll crossbar',(-1.21,-.51,1.52),(-1.21,.51,1.52),.035)
cylinder('Gun pedestal',(-.88,0,1.03),.12,.35)
box('Gun receiver',(-.7,0,1.34),(.46,.17,.19),'Dark')
beam('Gun barrel',(-.52,0,1.35),(.17,0,1.35),.035)
box('Ammo box',(-.76,-.24,1.22),(.29,.24,.27),'Paint')
for x in (-.20,.12,.40): box('Roof armor ridge',(x,0,1.68),(.08,1.1,.055),'Rust',.008)
cylinder('Exhaust',(-1.2,.72,1.19),.045,.65,'Steel')
export('SM_HERO_Technical',-.35)

# Black market headquarters, concrete bunker, metal awning, neon signage.
clean()
box('Foundation',(0,0,.20),(6.8,6.0,.4),'Concrete',.04)
box('Lower bunker',(0,0,1.45),(5.9,5.1,2.5),'Concrete',.06)
box('Upper office',(-.55,0,3.10),(4.6,4.8,1.35),'Concrete',.05)
box('Roof cap',(-.55,0,3.84),(4.85,5.0,.20),'Steel')
box('Entrance recess',(3.01,0,1.4),(.08,2.8,2.4),'Dark')
box('Door shutter',(3.07,0,1.27),(.055,2.05,2.15),'Steel')
for z in (.3,.5,.7,.9,1.1,1.3,1.5,1.7,1.9,2.1): box('Shutter slat',(3.12,0,z),(.04,2.02,.04),'Dark',.004)
box('Entrance canopy',(3.13,0,2.76),(1.3,3.4,.2),'Steel')
for y in (-1.56,1.56):
 box('Entry buttress',(3.35,y,1.3),(.46,.42,2.6),'Concrete',.03,(0,-.1,0))
 box('Neon door strip',(3.60,y,1.9),(.045,.055,1.0),'Neon',.005)
for y in (-2.57,2.57):
 for x in (-1.75,-.6,.55,1.7):
  box('Office window',(x,y,3.10),(.78,.04,.72),'Glass')
  box('Window frame',(x,y,3.5),(.94,.07,.055),'Steel',.005)
  box('Security grille',(x,y*1.01,3.1),(.04,.045,.76),'Steel',.004)
box('Sign board',(2.0,0,3.35),(.1,3.9,.77),'Dark')
box('Sign underline',(2.07,0,3.03),(.035,3.4,.05),'Neon',.005)
for y in (-2.0,2.0): box('Roof plant',(-1.1,y,4.16),(1.4,.7,.45),'Dark')
for x in (-2.3,-.8):
 cylinder('Roof ventilator',(x,-.5,4.1),.3,.5)
 box('Vent cover',(x,-.5,4.43),(.75,.75,.14),'Steel')
beam('Antenna',(-1.6,1.1,3.9),(-1.6,1.1,5.4),.045)
for z in (4.55,4.9,5.2): beam('Antenna arm',(-1.6,.6,z),(-1.6,1.6,z),.025)
for y in (-3.05,3.05):
 box('Cargo crate',(-1.8,y,.55),(1.4,.9,1.1),'Rust')
 for x in (-2.3,-1.8,-1.3): box('Crate braces',(x,y,.55),(.065,.94,1.13),'Steel',.005)
export('SM_HERO_BlackMarket',-1.1)

# City building modular prop, repeated with variation.
clean()
box('Building',(0,0,3),(7,6,6),'Concrete',.03)
box('Roof',(0,0,6.15),(7.35,6.35,.3),'Steel')
for z in (1.6,3.4,5.2):
 for x in (-2.5,-.85,.85,2.5):
  for y in (-3.025,3.025):
   box('Window',(x,y,z),(1.15,.07,1.15),'Glass')
   box('Lintel',(x,y,z+.62),(1.38,.13,.13),'Concrete')
for x in (-3.55,3.55): box('Corner pillar',(x,0,3),(.22,6.2,6),'Dark')
box('Shopfront',(0,-3.08,.72),(5.3,.08,1.3),'Dark')
box('Shop sign',(0,-3.12,1.7),(5.8,.12,.45),'Neon')
box('Roof service',(0,.8,6.6),(2.2,2.5,.8),'Dark')
for x in (-2.4,-1.1,1.4): cylinder('Air vent',(x,-1.5,6.4),.34,.45)
export('SM_UrbanBlock')

clean()
box('Container',(0,0,1.25),(6.0,2.4,2.5),'Rust',.03)
for x in [i*.30-2.85 for i in range(20)]:
 for y in (-1.225,1.225): box('Corrugated rib',(x,y,1.25),(.055,.055,2.4),'Steel',.005)
for y in (-.6,.6): box('Cargo door',(3.035,y,1.25),(.045,1.1,2.34),'Armor')
export('SM_CargoContainer')

clean()
for x,y in ((0,0),(.7,.15),(.4,.7)):
 cylinder('Oil drum',(x,y,.45),.28,.90,'Rust')
 for z in (.1,.8): cylinder('Drum rim',(x,y,z),.30,.05,'Steel')
export('SM_OilDrums')

clean()
box('Foot',(0,0,.15),(.8,.8,.3),'Concrete')
beam('Lamp pole',(0,0,.25),(0,0,5),.075)
beam('Lamp arm',(0,0,5),(.9,0,5),.07)
box('Lamp housing',(.95,0,4.96),(.55,.3,.18),'Steel')
box('Lamp glow',(.95,0,4.86),(.45,.23,.025),'Lamp',.005)
export('SM_StreetLamp')

clean()
for i in range(16):
 box('Broken concrete',(random.uniform(-1.3,1.3),random.uniform(-1,1),random.uniform(.05,.25)),
     (random.uniform(.25,.7),random.uniform(.2,.6),random.uniform(.12,.3)),random.choice(['Concrete','Dark','Rust']),.01,
     (random.uniform(-.3,.3),random.uniform(-.3,.3),random.uniform(0,3)))
export('SM_Rubble')


# Salvage truck: utility cab, exposed cargo deck and scrap bins, no weapon.
clean()
box('Truck chassis',(0,0,.43),(3.25,1.6,.24),'Dark')
box('Cargo deck',(-.5,0,.69),(2.2,1.65,.14),'Rust')
box('Cab',(1.0,0,1.15),(1.08,1.55,1.2),'Paint',.06)
box('Front window',(1.56,0,1.43),(.035,1.31,.53),'Glass',.01)
box('Hood',(1.7,0,.9),(.40,1.47,.29),'Armor')
for side in (-1,1):
 box('Side window',(1.05,side*.795,1.42),(.76,.02,.49),'Glass',.01)
 box('Cargo rail',(-.51,side*.80,1.1),(2.12,.10,.45),'Steel')
 for x in (-1.06,1.10):
  cylinder('Utility tire',(x,side*.88,.42),.42,.25,'Rubber',(math.pi/2,0,0),24)
  cylinder('Wheel hub',(x,side*1.015,.42),.23,.022,'Steel',(math.pi/2,0,0))
for x,y in ((-.4,-.36),(-1.05,.36),(-1.15,-.36)):
 cylinder('Salvage drum',(x,y,1.0),.23,.54,'Rust')
box('Salvage crate',(-.2,.38,1.02),(.62,.65,.58),'Paint')
for i in range(8): box('Mixed scrap',(-.4+i*.11,0,1.37+random.uniform(-.08,.08)),(.42,.14,.12),'Steel',.008,(0,0,random.random()*3))
box('Rear tow rack',(-1.62,0,1.0),(.13,1.7,.5),'Rust')
for side in (-1,1): box('Hazard lamps',(1.57,side*.6,1.85),(.14,.14,.10),'Lamp',.006)
box('Team insignia',(1,0,1.79),(.65,1.10,.04),'Team')
export('SM_HERO_Salvager',-.35)

# Vehicle workshop with visible service bays and machinery.
clean()
box('Workshop platform',(0,0,.12),(4.8,4.4,.24),'Concrete')
box('Workshop shell',(0,0,1.45),(4.25,3.7,2.7),'Concrete')
box('Ribbed roof',(0,0,2.92),(4.65,4.1,.23),'Steel')
for y in (-.95,.95):
 box('Bay recess',(2.17,y,1.14),(.07,1.42,1.94),'Dark')
 box('Garage slats',(2.22,y,1.35),(.04,1.32,1.55),'Steel')
 for z in (.62,.92,1.22,1.52,1.82): box('Door line',(2.25,y,z),(.025,1.3,.035),'Dark',.003)
 box('Bay light',(2.3,y,2.28),(.05,1.18,.08),'Neon',.005)
for x in (-1.7,-.6,.6,1.7): box('Roof seams',(x,0,3.06),(.045,3.9,.06),'Rust',.006)
box('Workshop sign',(2.18,0,2.69),(.10,3.5,.34),'Paint')
cylinder('Roof vent',(-1.1,1.0,3.17),.34,.41)
box('Generator',(-2.40,0,.65),(.6,1.7,1.3),'Armor')
for y in (-.63,0,.63): box('Generator vent',(-2.72,y,.70),(.045,.40,.05),'Dark')
beam('Crane support',(1.4,-2.05,0),(1.4,-2.05,3.7),.06,'Paint')
beam('Crane arm',(1.4,-2.05,3.7),(2.8,-2.05,3.7),.06,'Paint')
beam('Hook',(2.8,-2.05,3.7),(2.8,-2.05,2.5),.025)
export('SM_HERO_Workshop',-.35)

# Compact scrap depot, canopy, storage cages, ramp.
clean()
box('Depot platform',(0,0,.10),(3.5,3.5,.2),'Concrete')
box('Depot container',(-.4,0,.95),(2.25,2.8,1.7),'Rust')
for y in (-1.42,1.42):
 for x in (-1.35,-.95,-.55,-.15,.25,.65): box('Depot rib',(x,y,.95),(.045,.05,1.65),'Steel',.004)
box('Roller door',(.77,0,.91),(.035,2.2,1.55),'Dark')
box('Canopy',(.65,0,1.98),(2.3,3.0,.13),'Steel')
for y in (-1.3,1.3): beam('Awning pillar',(1.75,y,0),(1.75,y,1.95),.045,'Paint')
box('Depot sign',(.79,0,1.72),(.04,2.1,.22),'Neon',.004)
for x,y in ((-1.55,1.66),(-1.55,-1.66),(.2,1.66)):
 box('Metal crate',(x,y,.45),(.6,.48,.9),'Armor')
 for z in (.25,.6): box('Storage straps',(x,y,z),(.65,.54,.05),'Steel',.005)
export('SM_HERO_Depot',-.35)
