"""Original military salvage supply cache, separate from the civic generator."""
import bpy,json,math
from pathlib import Path
OUT=Path(__file__).resolve().parents[2]/'Art/Models/DistrictV3'
PALETTE={'Metal':(.16,.19,.20),'Concrete':(.30,.29,.26),'Paint':(.20,.29,.24),'Dust':(.47,.36,.23)}
bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False)
bpy.context.scene.unit_settings.system='METRIC';bpy.context.scene.unit_settings.scale_length=1
for name,color in PALETTE.items():
    m=bpy.data.materials.get(name) or bpy.data.materials.new(name);m.use_nodes=True;m.diffuse_color=(*color,1)
    p=m.node_tree.nodes.get('Principled BSDF');p.inputs['Base Color'].default_value=(*color,1);p.inputs['Roughness'].default_value=.7;p.inputs['Metallic'].default_value=.65 if name=='Metal' else .05
def box(loc,size,mat='Metal',edge=.025,rot=None):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.dimensions=size
    if rot:o.rotation_euler=rot
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(bpy.data.materials[mat])
    if edge:
        mod=o.modifiers.new('Fabricated edges','BEVEL');mod.width=edge;mod.segments=2;bpy.ops.object.modifier_apply(modifier=mod.name)
    return o
def cylinder(loc,r,h,mat='Metal',rot=(0,0,0),vertices=20):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=h,location=loc,rotation=rot);o=bpy.context.object;o.data.materials.append(bpy.data.materials[mat]);return o
def crate(x,y,z,w=1.25,d=.85,h=.75):
    box((x,y,z+h/2),(w,d,h),'Paint',.045)
    box((x,y,z+h-.03),(w+.045,d+.045,.10),'Metal')
    box((x,y,z+.06),(w+.02,d+.02,.12),'Metal')
    for xx in [x-w*.36,x+w*.36]:
        for yy in [y-d/2-.012,y+d/2+.012]:box((xx,yy,z+h*.5),(.095,.055,h*.82),'Metal',.008)
        box((xx,y,z+h+.035),(.075,d*.9,.025),'Dust',.005)
    for yy in [y-d/2-.035,y+d/2+.035]:
        for i in range(4):box((x-w*.2+i*w*.13,yy,z+h*.38),(.04,.025,h*.39),'Metal',.005)
        box((x,yy,z+h*.73),(.30,.04,.16),'Dust',.008)
        for i in range(3):box((x-.09+i*.085,yy-.021 if yy<y else yy+.021,z+h*.73),(.025,.01,.065),'Metal',0)
    for xx in [x-w/2-.035,x+w/2+.035]:
        box((xx,y,z+h*.60),(.035,.28,.06),'Metal',.005)
        for yy in [y-.11,y+.11]:box((xx,yy,z+h*.64),(.035,.035,.12),'Metal',.004)
def barrel(x,y):
    cylinder((x,y,.48),.27,.9,'Paint')
    for z in [.08,.28,.68,.90]:cylinder((x,y,z),.285,.035)
    cylinder((x,y,.947),.255,.025,'Metal');cylinder((x+.1,y,.968),.04,.016,'Dust',vertices=8)
box((0,0,.06),(3.9,2.8,.12),'Concrete',.02)
for y in [-.75,.0,.75]:box((-.35,y,.18),(2.65,.15,.20),'Dust',.015)
crate(-.93,-.56,.3);crate(.48,-.56,.3);crate(-.93,.45,.3);crate(.48,.45,.3)
crate(-.30,-.05,1.15,w=1.35,d=.92,h=.8)
box((-.30,-.05,2.00),(1.15,.80,.22),'Metal',.025)
for x in [-.65,.05]:box((x,-.05,2.14),(.05,.76,.065),'Dust',.01)
barrel(1.52,-.52);barrel(1.52,.22)
# Restrained salvage items on the apron, with recognizable axle and flanged ends.
cylinder((-.15,1.1,.30),.065,1.50,rot=(0,math.pi/2,0))
for x in [-.82,.52]:cylinder((x,1.1,.30),.19,.13,rot=(0,math.pi/2,0));cylinder((x,1.1,.30),.08,.17,'Dust',rot=(0,math.pi/2,0))
box((.15,1.11,.17),(.85,.17,.06),'Metal',.01,rot=(0,0,.23));box((-.96,1.04,.17),(.43,.23,.05),'Paint',.01,rot=(.05,.06,-.20))
meshes=[o for o in bpy.context.scene.objects if o.type=='MESH'];bpy.ops.object.select_all(action='DESELECT')
for o in meshes:o.select_set(True)
bpy.context.view_layer.objects.active=meshes[0];bpy.ops.object.join();o=bpy.context.object;o.name='SM_DV3_SupplyCache'
bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.cube_project(cube_size=1);bpy.ops.object.mode_set(mode='OBJECT')
old=[m.name for m in o.data.materials];indices=[list(PALETTE).index(old[p.material_index]) for p in o.data.polygons];o.data.materials.clear()
for mat in PALETTE:o.data.materials.append(bpy.data.materials[mat])
for p,i in zip(o.data.polygons,indices):p.material_index=i
OUT.mkdir(parents=True,exist_ok=True);bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(o.name+'.blend')))
bpy.ops.export_scene.fbx(filepath=str(OUT/(o.name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
report={'name':o.name,'dimensions_m':list(o.dimensions),'triangles':sum(len(p.vertices)-2 for p in o.data.polygons),'materials':list(PALETTE),'pivot':'ground origin','bounds_min_m':[min(v.co[j] for v in o.data.vertices) for j in range(3)],'bounds_max_m':[max(v.co[j] for v in o.data.vertices) for j in range(3)]}
(OUT/(o.name+'.json')).write_text(json.dumps(report,indent=2));print('SUPPLY_CACHE_EXPORTED',json.dumps(report))
