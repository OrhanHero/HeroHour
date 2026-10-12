"""Original CITY HERO environment geometry; source blends retained."""
from pathlib import Path
exec(Path(__file__).with_name('build_hero_models.py').read_text().split('# Salvaged armoured pickup')[0])
for style in range(3):
 clean()
 width=5.4; depth=5.4; height=(12,19,27)[style]
 box('Foundation',(0,0,.15),(6,6,.3),'Concrete')
 box('Building',(0,0,height/2),(width,depth,height),'Concrete',.08)
 for z in range(2,int(height),3):
  for side in (-1,1):
   for x in (-1.7,0,1.7):
    box('Recessed window',(x,side*2.72,z),(1.15,.035,1.65),'Glass',.015)
    box('Window sill',(x,side*2.78,z-.9),(1.3,.18,.13),'Paint',.01)
   for y in (-1.7,0,1.7): box('Side window',(side*2.72,y,z),(.035,1.15,1.65),'Glass',.015)
  box('Floor ledge',(0,0,z+1),(5.6,5.6,.13),'Paint',.01)
 box('Roof parapet',(0,0,height+.18),(5.7,5.7,.36),'Concrete')
 box('Roof inset',(0,0,height+.39),(4.8,4.8,.06),'Dark')
 for x in (-1.4,1.4): box('Roof HVAC',(x,.7,height+.7),(1,1.6,.7),'Steel')
 box('Entrance',(0,-2.76,1),(.95,.045,2),'Dark')
 box('Entrance canopy',(0,-3.1,2.2),(2.1,1,.16),'Steel')
 export('SM_CH_Tower_'+str(style))
clean()
box('House',(0,0,1.9),(5.8,4.2,3.8),'Concrete',.06)
box('Roof',(0,0,3.97),(6.1,4.5,.35),'Paint')
for x in (-1.8,1.8):
 box('House window',(x,-2.13,2.05),(1.2,.03,1.25),'Glass')
box('Door',(0,-2.13,1.05),(.8,.035,2.1),'Dark')
box('Front step',(0,-2.6,.12),(2.1,.9,.24),'Concrete')
box('Roof water tank',(-1,0,4.5),(1.2,1.2,.75),'Steel')
export('SM_CH_House')
clean()
box('Bridge deck',(0,0,-.25),(20,14,.5),'Concrete')
box('Road',(0,0,.03),(20,12.4,.06),'Dark',0)
for side in (-1,1):
 box('Bridge curb',(0,side*6.6,.25),(20,.45,.5),'Concrete')
 beam('Railing',(-10,side*6.65,1.2),(10,side*6.65,1.2),.065)
 for x in range(-10,11,2): beam('Railing post',(x,side*6.65,.45),(x,side*6.65,1.2),.05)
 for x in (-6,6): cylinder('Bridge support',(x,side*4.5,-3),.7,5,'Concrete')
for x in range(-9,10,3): box('Bridge dash',(x,0,.075),(1.4,.13,.02),'Paint',0)
export('SM_CH_Bridge')
clean()
box('Refinery apron',(0,0,.08),(18,15,.16),'Concrete')
for x,y in ((-5,-4),(-5,4),(0,4)):
 cylinder('Storage tank',(x,y,2),2.4,4,'Paint',vertices=32)
 cylinder('Tank cap',(x,y,4.1),2.5,.2,'Steel',vertices=32)
 for z in (.4,3.6): cylinder('Tank belt',(x,y,z),2.46,.12,'Steel',vertices=32)
for x,y,h in ((2,-4,11),(5,-3,14),(5,2,9)):
 cylinder('Process tower',(x,y,h/2),.7,h,'Steel',vertices=24)
 for z in range(2,h,2): cylinder('Tower walkway',(x,y,z),1,.08,'Rust')
 beam('Tower ladder',(x+.75,y,0),(x+.75,y,h),.06,'Paint')
for y in (-5,0,5):
 beam('Transfer pipe',(-7,y,1.1),(7,y,1.1),.17,'Rust')
for x in (-7,7): box('Pump house',(x,0,1.2),(2,3,2.4),'Armor')
export('SM_CH_Refinery')
clean()
# Elliptical stadium bowl, tiered spectator ring and floodlights.
for i in range(64):
 a=i*math.tau/64; x=12*math.cos(a); y=9*math.sin(a)
 box('Stadium exterior',(x,y,2.1),(1.2,1.6,4.2),'Concrete',.03,(0,0,a))
 for tier in range(4):
  r=1-tier*.065
  box('Seating tier',(x*r,y*r,2.8-tier*.5),(1.3,1.4,.18),'Paint',.015,(0,0,a))
box('Pitch',(0,0,.08),(17,11,.16),'Armor',0)
for y in (-5.3,5.3): box('Pitch line',(0,y,.18),(16,.06,.015),'Concrete',0)
for x in (-8,8,0): box('Pitch line',(x,0,.18),(.06,10.6,.015),'Concrete',0)
for x in (-11,11):
 for y in (-8,8):
  beam('Floodlight tower',(x,y,0),(x,y,9),.12,'Steel')
  box('Floodlight',(x,y,9),(1.8,.5,.7),'Lamp')
export('SM_CH_Stadium')
clean()
for x in (-5,5):
 verts=[]; faces=[]; rings=16; sides=40
 for ring in range(rings+1):
  z=ring*1.0; radius=2.2+1.6*((z-10)/10)**2
  for i in range(sides):
   a=i*math.tau/sides; verts.append((x+radius*math.cos(a),radius*math.sin(a),z))
 for j in range(rings):
  for i in range(sides):
   a=j*sides+i; b=j*sides+(i+1)%sides; faces.append((a,b,b+sides,a+sides))
 mesh=bpy.data.meshes.new('Cooling tower'); mesh.from_pydata(verts,[],faces); mesh.update()
 obj=bpy.data.objects.new('Cooling tower',mesh); bpy.context.collection.objects.link(obj); finish(obj,'Cooling tower','Concrete')
box('Turbine hall',(0,-6,2),(15,5,4),'Steel')
export('SM_CH_PowerPlant')
# Faceted eroded hills around a flat playable board; river bed is inset.
clean()
verts=[]; faces=[]; n=100
for j in range(n+1):
 y=-125+250*j/n
 for i in range(n+1):
  x=-155+310*i/n
  edge=max(abs(x)-119,abs(y)-89,0)
  peaks=sum(h*math.exp(-((x-px)**2/(sx*sx)+(y-py)**2/(sy*sy))) for px,py,h,sx,sy in [(-140,-55,19,23,27),(-132,72,16,27,25),(142,35,25,20,37),(110,-110,20,35,20),(-25,115,16,43,18)])
  blend=min(edge/10,1); blend=blend*blend*(3-2*blend)
  z=-.05 if edge<=0 else (edge*.16+peaks+math.sin(x*.13)*math.cos(y*.17)*min(edge*.1,1.2))*blend
  if abs(x+30)<7 and abs(y)<90: z=-3.5
  verts.append((x,y,z))
for j in range(n):
 for i in range(n):
  a=j*(n+1)+i; faces.extend(((a,a+1,a+n+2),(a,a+n+2,a+n+1)))
mesh=bpy.data.meshes.new('Desert'); mesh.from_pydata(verts,[],faces); mesh.update()
o=bpy.data.objects.new('Terrain',mesh); bpy.context.collection.objects.link(o); finish(o,'Terrain','Paint')
o.data.materials.append(materials['Concrete'])
for polygon in o.data.polygons:
 if sum(verts[i][2] for i in polygon.vertices)/len(polygon.vertices)>6: polygon.material_index=1
export('SM_CH_Terrain')
