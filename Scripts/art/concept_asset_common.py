"""Shared modelling/export helpers for original HERO concept-derived meshes."""
import bpy, math, json
from pathlib import Path
from mathutils import Vector

PROJECT = Path(__file__).resolve().parents[2]
OUT = PROJECT / 'Art/Models/ConceptV2'
PREVIEW = PROJECT / 'Art/Previews/ConceptV2'
PALETTE = {
    'Armor': ((.105,.125,.135), .72,.52),
    'Steel': ((.24,.27,.28), .8,.42),
    'Dark': ((.026,.035,.039), .45,.66),
    'Concrete': ((.31,.30,.26), 0,.88),
    'Dust': ((.43,.37,.25), .15,.76),
    'Rubber': ((.018,.022,.024), 0,.85),
    'Glass': ((.02,.09,.09), .55,.24),
    'Team': ((.035,.24,.15), .55,.48),
    'Neon': ((.035,.6,.30), .25,.3),
    'Lamp': ((.8,.45,.12), .1,.36),
}

def reset():
    bpy.ops.object.select_all(action='SELECT')
    bpy.ops.object.delete(use_global=False)
    bpy.context.scene.unit_settings.system='METRIC'
    bpy.context.scene.unit_settings.scale_length=1
    for name,(color,metal,rough) in PALETTE.items():
        m=bpy.data.materials.get(name) or bpy.data.materials.new(name)
        m.use_nodes=True; m.diffuse_color=(*color,1)
        nodes=m.node_tree.nodes; links=m.node_tree.links; nodes.clear()
        p=nodes.new('ShaderNodeBsdfPrincipled'); out=nodes.new('ShaderNodeOutputMaterial')
        p.inputs['Base Color'].default_value=(*color,1)
        p.inputs['Metallic'].default_value=metal; p.inputs['Roughness'].default_value=rough
        links.new(p.outputs['BSDF'],out.inputs['Surface'])
        if name in ('Neon','Lamp'):
            p.inputs['Emission Color'].default_value=(*color,1)
            p.inputs['Emission Strength'].default_value=2.0 if name=='Neon' else 1.0
        else:
            tex=nodes.new('ShaderNodeTexNoise'); tex.inputs['Scale'].default_value=32
            tex.inputs['Detail'].default_value=3
            bump=nodes.new('ShaderNodeBump'); bump.inputs['Strength'].default_value=.16
            bump.inputs['Distance'].default_value=.018
            links.new(tex.outputs['Fac'],bump.inputs['Height']); links.new(bump.outputs['Normal'],p.inputs['Normal'])

def finish(obj,name,mat):
    obj.name=name; obj.data.materials.append(bpy.data.materials[mat]); return obj

def bevel(obj,width=.025,segments=2):
    if width:
        bpy.context.view_layer.objects.active=obj
        mod=obj.modifiers.new('Machined edge chamfer','BEVEL'); mod.width=width; mod.segments=segments
        bpy.ops.object.modifier_apply(modifier=mod.name)
    for poly in obj.data.polygons: poly.use_smooth=False
    return obj

def box(name,loc,size,mat='Armor',edge=.025,rot=None):
    bpy.ops.mesh.primitive_cube_add(size=1,location=loc)
    o=bpy.context.object; o.dimensions=size
    if rot: o.rotation_euler=rot
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    finish(o,name,mat); return bevel(o,edge)

def cylinder(name,loc,radius,depth,mat='Steel',rot=(0,0,0),vertices=24):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=radius,depth=depth,location=loc,rotation=rot)
    return bevel(finish(bpy.context.object,name,mat),.012,1)

def cone(name,loc,r1,r2,depth,mat='Armor',vertices=8):
    bpy.ops.mesh.primitive_cone_add(vertices=vertices,radius1=r1,radius2=r2,depth=depth,location=loc)
    return bevel(finish(bpy.context.object,name,mat),.025,2)

def beam(name,a,b,r=.025,mat='Steel'):
    a,b=Vector(a),Vector(b); delta=b-a
    o=cylinder(name,(a+b)/2,r,delta.length,mat,vertices=12)
    o.rotation_euler=delta.to_track_quat('Z','Y').to_euler(); return o

def torus(name,loc,major,minor=.04,mat='Steel',rot=(0,0,0)):
    bpy.ops.mesh.primitive_torus_add(major_radius=major,minor_radius=minor,major_segments=32,minor_segments=8,location=loc,rotation=rot)
    return finish(bpy.context.object,name,mat)

def bolts(name,loc,axis='X',spacing=.25,count=5):
    for i in range(count):
        p=list(loc); p['XYZ'.index(axis)]+=i*spacing
        cylinder(name+str(i),p,.024,.023,'Steel',vertices=6)

def vent(name,loc,width=.8,height=.45,axis='Y',count=7):
    x,y,z=loc
    size=(width,.045,height) if axis=='Y' else (.045,width,height)
    box(name+' recess',loc,size,'Dark',.008)
    for i in range(count):
        zz=z-height*.4+i*height*.8/(count-1)
        size=(width*.9,.065,.025) if axis=='Y' else (.065,width*.9,.025)
        box(name+' louver',(x,y,zz),size,'Steel',.005)

def export(name,zoffset=0):
    OUT.mkdir(parents=True,exist_ok=True); PREVIEW.mkdir(parents=True,exist_ok=True)
    meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
    for o in meshes: o.location.z+=zoffset
    bpy.ops.object.select_all(action='DESELECT')
    for o in meshes: o.select_set(True)
    bpy.context.view_layer.objects.active=meshes[0]; bpy.ops.object.join(); mesh=bpy.context.object; mesh.name=name
    bpy.context.scene.cursor.location=(0,0,0); bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    # Explicit UVs for later texture baking; original source geometry remains editable in Blender.
    bpy.ops.object.mode_set(mode='EDIT'); bpy.ops.mesh.select_all(action='SELECT')
    bpy.ops.uv.smart_project(angle_limit=math.radians(66),island_margin=.012)
    bpy.ops.object.mode_set(mode='OBJECT')
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},
        axis_forward='X',axis_up='Z',global_scale=1,apply_unit_scale=False,add_leaf_bones=False,mesh_smooth_type='FACE')
    info={'name':name,'vertices':len(mesh.data.vertices),'polygons':len(mesh.data.polygons),
        'triangles':sum(len(p.vertices)-2 for p in mesh.data.polygons),'dimensions_m':list(mesh.dimensions),
        'materials':[m.name for m in mesh.data.materials],'origin':'ground pivot with runtime Z compensation','zoffset_m':zoffset}
    (OUT/(name+'.json')).write_text(json.dumps(info,indent=2),encoding='utf-8')
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    print('CONCEPT_ASSET_EXPORTED',json.dumps(info))
    return mesh

def preview(name,mesh):
    # This is an actual render of the exported model, not an AI concept illustration.
    scene=bpy.context.scene
    points=[mesh.matrix_world@Vector(c) for c in mesh.bound_box]
    zmin=min(p.z for p in points); zmax=max(p.z for p in points)
    radius=max(mesh.dimensions.x,mesh.dimensions.y,mesh.dimensions.z)
    box('Preview ground',(0,0,zmin-.09),(radius*4,radius*4,.16),'Concrete',0)
    world=bpy.data.worlds.new('Warm studio') if not scene.world else scene.world; scene.world=world
    world.use_nodes=True; world.node_tree.nodes['Background'].inputs[0].default_value=(.22,.25,.29,1)
    world.node_tree.nodes['Background'].inputs[1].default_value=.5
    for loc,power,size in [((4,-6,10),1600,7),((-5,-1,6),1100,5),((2,5,8),1800,5)]:
        bpy.ops.object.light_add(type='AREA',location=loc); light=bpy.context.object
        light.data.energy=power; light.data.shape='DISK'; light.data.size=size
        light.rotation_euler=(Vector((0,0,2))-light.location).to_track_quat('-Z','Y').to_euler()
    target=Vector((0,0,(zmin+zmax)*.48))
    bpy.ops.object.camera_add(location=(radius*1.2,-radius*1.4,radius*1.15))
    cam=bpy.context.object; cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler()
    cam.data.type='ORTHO'; cam.data.ortho_scale=radius*1.65; scene.camera=cam
    scene.render.engine='CYCLES'; scene.cycles.samples=48; scene.cycles.use_denoising=True
    scene.render.resolution_x=1200; scene.render.resolution_y=1000; scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG'; scene.render.filepath=str(PREVIEW/(name+'.png'))
    bpy.ops.render.render(write_still=True)
