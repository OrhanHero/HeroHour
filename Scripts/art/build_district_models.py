"""Original, compact desert-city kit. Run using Blender --background --python."""
import bpy, math, json
from pathlib import Path
from mathutils import Vector

OUT=Path(__file__).resolve().parents[2]/'Art/Models/DistrictV3'
COLORS={'Plaster':(.57,.49,.37),'Concrete':(.33,.32,.29),'Asphalt':(.075,.083,.085),'Glass':(.055,.12,.15),'Metal':(.17,.20,.21),'Dust':(.48,.36,.22),'Paint':(.24,.35,.34)}
def reset():
    bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system='METRIC'; bpy.context.scene.unit_settings.scale_length=1
    for name,col in COLORS.items():
        m=bpy.data.materials.get(name) or bpy.data.materials.new(name); m.use_nodes=True; m.diffuse_color=(*col,1)
        p=m.node_tree.nodes.get('Principled BSDF'); p.inputs['Base Color'].default_value=(*col,1); p.inputs['Roughness'].default_value=.82
        if name=='Glass':p.inputs['Roughness'].default_value=.23;p.inputs['Metallic'].default_value=.5
        if name=='Metal':p.inputs['Metallic'].default_value=.65
def box(loc,size,mat='Concrete',edge=.025):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc);o=bpy.context.object;o.dimensions=size
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True);o.data.materials.append(bpy.data.materials[mat])
    if edge:
        mod=o.modifiers.new('Edge catching light','BEVEL');mod.width=edge;mod.segments=1;bpy.ops.object.modifier_apply(modifier=mod.name)
    return o
def cyl(loc,r,h,mat='Metal',vertices=12):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=h,location=loc);o=bpy.context.object;o.data.materials.append(bpy.data.materials[mat]);return o
def roof(z,w,d):
    box((0,0,z),(w+.12,d+.12,.18),'Concrete')
    for x in [-w/2,w/2]:box((x,0,z+.24),(.1,d,.48),'Plaster')
    for y in [-d/2,d/2]:box((0,y,z+.24),(w,.1,.48),'Plaster')
    for x,y in [(-.85,.65),(.75,.5)]:
        box((x,y,z+.35),(.8,.7,.55),'Metal');cyl((x,y,z+.65),.22,.04)
        for i in range(5):box((x-.31+i*.15,y-.36,z+.38),(.045,.025,.32),'Concrete',0)
    cyl((-.7,-.4,z+.52),.14,.7);cyl((-.7,-.4,z+.9),.23,.08)
def facade(z,w,d,columns=3,balcony=False):
    for y in [-d/2-.035,d/2+.035]:
        for i in range(columns):
            x=(i-(columns-1)/2)*w/columns;ww=w/columns*.63
            box((x,y,z),(ww,.065,1.3),'Glass',.008)
            for dx in [-ww/2,ww/2]:box((x+dx,y,z),(.06,.10,1.45),'Concrete',.006)
            for zz in [z-.72,z+.72]:box((x,y,zz),(ww+.12,.13,.07),'Concrete',.006)
            box((x,y,z),(.045,.1,1.3),'Metal',0)
            if balcony and y<0:
                box((x,y-.18,z-.8),(ww+.25,.45,.12),'Concrete')
                box((x,y-.39,z-.45),(ww+.2,.035,.6),'Paint',.01)
    for x in [-w/2-.035,w/2+.035]:
        for yy in [-d*.23,d*.23]:
            box((x,yy,z),(.06,.72,1.25),'Glass',.008)
            box((x,yy,z-.69),(.14,.85,.08),'Concrete',.008)
def storefront(w,d):
    for x in [-1.1,1.1]:
        box((x,-d/2-.035,1.28),(1.32,.08,1.9),'Glass')
        box((x,-d/2-.15,2.4),(1.6,.28,.3),'Paint')
        for xx in [x-.69,x+.69]:box((xx,-d/2-.1,1.25),(.09,.17,2.15),'Concrete')
    box((0,-d/2-.12,.16),(.9,.30,.32),'Concrete')
    box((0,-d/2-.05,1.2),(.72,.08,1.85),'Metal')
def building(kind):
    reset();w=3.6;d=3.6
    box((0,0,.12),(3.9,3.9,.24),'Concrete')
    box((0,0,1.5),(w,d,2.76),'Plaster');storefront(w,d)
    if kind==0:
        box((-.25,.15,5.15),(3.05,3.05,4.4),'Plaster')
        for z in [3.95,6.05]:facade(z,3.05,3.05,3,True)
        box((-.25,.15,7.5),(3.2,3.2,.18),'Concrete');roof(7.65,2.65,2.65)
        box((.8,.65,8.34),(.65,.65,.92),'Concrete');cyl((.8,.65,8.89),.06,.20)
    elif kind==1:
        box((0,0,7.35),(3.4,3.4,8.7),'Plaster')
        for z in [4,6.2,8.4,10.6]:
            facade(z,3.4,3.4,3,z in [6.2,10.6]);box((0,0,z-1.0),(3.55,3.55,.12),'Concrete')
        box((.2,.2,12.8),(2.7,2.7,2.0),'Plaster');facade(12.8,2.7,2.7,2)
        roof(13.9,2.7,2.7);cyl((.6,.6,14.67),.025,.66)
    else:
        box((0,0,9.4),(3.5,3.5,12.8),'Concrete')
        for z in [4,6.2,8.4,10.6,12.8,15]:facade(z,3.5,3.5,3)
        for x in [-1.8,1.8]:box((x,0,9.5),(.12,3.7,13),'Paint')
        box((0,.3,18),(2.8,2.7,4.3),'Plaster')
        for z in [17,19.2]:facade(z,2.8,2.7,2)
        roof(20.3,2.8,2.7);cyl((.7,.5,21.25),.035,1.5)
    return export(['SM_DV3_Civic_0','SM_DV3_Civic_1','SM_DV3_Civic_2'][kind])
def export(name):
    OUT.mkdir(parents=True,exist_ok=True);meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
    bpy.ops.object.select_all(action='DESELECT')
    for o in meshes:o.select_set(True)
    bpy.context.view_layer.objects.active=meshes[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
    bpy.context.scene.cursor.location=(0,0,0);bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.mode_set(mode='EDIT');bpy.ops.mesh.select_all(action='SELECT');bpy.ops.uv.cube_project(cube_size=1);bpy.ops.object.mode_set(mode='OBJECT')
    # Unified stable slots avoid duplicate slots left by joining component meshes.
    old=[m.name for m in o.data.materials];indices=[list(COLORS).index(old[p.material_index]) for p in o.data.polygons]
    o.data.materials.clear()
    for mat in COLORS:o.data.materials.append(bpy.data.materials[mat])
    for p,i in zip(o.data.polygons,indices):p.material_index=i
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    report={'name':name,'dimensions_m':list(o.dimensions),'triangles':sum(len(p.vertices)-2 for p in o.data.polygons),'materials':list(COLORS),'pivot':'ground origin','bounds_min_m':[min(v.co[j] for v in o.data.vertices) for j in range(3)],'bounds_max_m':[max(v.co[j] for v in o.data.vertices) for j in range(3)]}
    (OUT/(name+'.json')).write_text(json.dumps(report,indent=2));print('DISTRICT_EXPORTED',json.dumps(report));return report
reports=[building(i) for i in range(3)]
reset();box((0,0,.055),(6,6,.11),'Concrete',.01)
for x in [-2.98,2.98]:box((x,0,.12),(.04,6,.15),'Dust',0)
for y in [-2.98,2.98]:box((0,y,.12),(6,.04,.15),'Dust',0)
for k in [-2,-1,0,1,2]:
    box((k,0,.114),(.013,5.9,.004),'Dust',0);box((0,k,.114),(5.9,.013,.004),'Dust',0)
reports.append(export('SM_DV3_Pavement'))
reset();box((0,0,.13),(.35,.35,.26),'Concrete');cyl((0,0,2.4),.055,4.6);box((.34,0,4.64),(.76,.055,.07),'Metal');box((.65,0,4.60),(.42,.22,.09),'Paint');box((.65,0,4.54),(.32,.16,.025),'Dust')
reports.append(export('SM_DV3_Lamp'))
reset();box((0,0,.15),(1.7,.55,.3),'Concrete');box((0,0,.52),(1.7,.34,.65),'Concrete');box((0,0,.86),(1.7,.23,.10),'Concrete')
for x in [-.52,.52]:box((x,-.18,.62),(.26,.025,.20),'Paint',.008)
reports.append(export('SM_DV3_Barrier'))
(OUT/'district-kit-report.json').write_text(json.dumps(reports,indent=2))
