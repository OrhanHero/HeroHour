"""Bake original ReferenceV7 geometry into bounded, geometry-aware UV0 atlases.

Blender 5.2 --background --python this.py -- --only Aegis_APC8 Ember_APC8
HQ sources are opt-in: do not select them until the integration owner releases them.
Source .blend/.fbx files are read only; all output stays in Art/Textures/ReferenceV7.
"""
import argparse
import hashlib
import json
import math
import sys
import time
from pathlib import Path
import bpy
import numpy as np
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'Art/Textures/ReferenceV7'
OUT.mkdir(parents=True, exist_ok=True)
ITEMS = {
    'Aegis_APC8': ('ReferenceAPCV7', 'SM_HR_Aegis_APC8', 'Aegis', 'Combat', 1024),
    'Ember_APC8': ('ReferenceEmberAPCV7', 'SM_HR_Ember_APC8', 'Ember', 'Combat', 1024),
    'Aegis_HQ': ('ReferenceHQV7', 'SM_RV7_AegisHQ', 'Aegis', 'HQ', 2048),
    'Ember_HQ': ('ReferenceEmberHQV7', 'SM_RV7_EmberHQ', 'Ember', 'HQ', 2048),
}
PALETTE = {
    'Aegis': {'Armor': ((.48,.42,.30),.67), 'Steel': ((.24,.30,.34),.43),
              'Dark': ((.045,.060,.070),.78), 'Rubber': ((.018,.022,.025),.90),
              'Team': ((.025,.18,.48),.58), 'Glass': ((.018,.075,.13),.22),
              'Neon': ((.025,.35,.80),.30), 'Concrete': ((.45,.47,.45),.87),
              'Dust': ((.30,.255,.18),.92), 'Lamp': ((.85,.57,.22),.36)},
    'Ember': {'Armor': ((.085,.095,.105),.67), 'Steel': ((.21,.22,.23),.43),
              'Dark': ((.024,.028,.032),.80), 'Rubber': ((.014,.017,.019),.92),
              'Team': ((.025,.32,.13),.61), 'Glass': ((.012,.055,.032),.24),
              'Neon': ((.025,.70,.28),.30), 'Concrete': ((.24,.23,.21),.88),
              'Dust': ((.25,.205,.15),.92), 'Lamp': ((.90,.32,.08),.36)},
}


def digest(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def configure_cycles(scene):
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 16
    scene.render.threads_mode = 'FIXED'
    scene.render.threads = 8
    backend = 'CPU'
    preferences = bpy.context.preferences.addons['cycles'].preferences
    for candidate in ('OPTIX', 'CUDA', 'HIP', 'ONEAPI'):
        try:
            preferences.compute_device_type = candidate
            preferences.get_devices()
            devices = [d for d in preferences.devices if d.type == candidate]
            if devices:
                for device in preferences.devices:
                    device.use = device.type == candidate
                scene.cycles.device = 'GPU'
                backend = candidate
                break
        except (TypeError, RuntimeError):
            continue
    if backend == 'CPU':
        scene.cycles.device = 'CPU'
    scene.render.bake.use_selected_to_active = False
    scene.render.bake.use_clear = True
    scene.render.bake.use_pass_direct = False
    scene.render.bake.use_pass_indirect = False
    scene.view_settings.view_transform = 'Standard'
    scene.view_settings.look = 'None'
    scene.view_settings.exposure = 0
    scene.view_settings.gamma = 1
    return backend


def procedural_surface(material, surface, faction, role, zmin):
    """A quiet paint variation, convex-only wear, contact AO and low dirt mask."""
    color, roughness = PALETTE[faction][surface]
    if faction == 'Aegis' and role == 'HQ' and surface == 'Armor':
        color = (.60,.64,.64)
    material.use_nodes = True
    nodes = material.node_tree.nodes
    nodes.clear()
    links = material.node_tree.links

    def node(kind):
        return nodes.new(kind)

    def feed(value, socket):
        if isinstance(value, (int,float)):
            socket.default_value = value
        elif isinstance(value, (tuple,list)):
            socket.default_value = (*value[:3],1)
        else:
            links.new(value, socket)

    def math_node(operation, a, b=None, clamp=False):
        n = node('ShaderNodeMath'); n.operation = operation; n.use_clamp = clamp
        feed(a,n.inputs[0])
        if b is not None: feed(b,n.inputs[1])
        return n.outputs[0]

    def mix(a,b,amount):
        n = node('ShaderNodeMixRGB'); n.blend_type = 'MIX'
        feed(amount,n.inputs[0]);feed(a,n.inputs[1]);feed(b,n.inputs[2])
        return n.outputs[0]

    geometry = node('ShaderNodeNewGeometry')
    noise = node('ShaderNodeTexNoise')
    noise.inputs['Scale'].default_value = 24 if role == 'Combat' else 7
    noise.inputs['Detail'].default_value = 2
    noise.inputs['Roughness'].default_value = .65
    links.new(geometry.outputs['Position'],noise.inputs['Vector'])
    macro = node('ShaderNodeTexNoise')
    macro.inputs['Scale'].default_value = 3.5 if role == 'Combat' else 1.4
    macro.inputs['Detail'].default_value = 2
    links.new(geometry.outputs['Position'],macro.inputs['Vector'])
    variation = math_node('ADD',.94,math_node('MULTIPLY',noise.outputs['Fac'],.10))
    varied = node('ShaderNodeMixRGB');varied.blend_type='MULTIPLY'
    varied.inputs[0].default_value=1;feed(color,varied.inputs[1]);feed(variation,varied.inputs[2])

    ao = node('ShaderNodeAmbientOcclusion')
    ao.inputs['Distance'].default_value = .17 if role == 'Combat' else .42
    ao.samples = 8
    ao.only_local = True
    # Pointiness is topology derived; noise only breaks up its convex edge mask.
    edge = math_node('MULTIPLY',math_node('SUBTRACT',geometry.outputs['Pointiness'],.51),9,True)
    flecks = math_node('MULTIPLY',math_node('SUBTRACT',noise.outputs['Fac'],.38),2,True)
    edge = math_node('MULTIPLY',edge,flecks)
    wear_strength = .72 if surface in ('Armor','Steel','Dark') else .24 if surface=='Team' else .12 if surface=='Concrete' else 0
    edge = math_node('MULTIPLY',edge,wear_strength)
    exposed = (.31,.32,.29) if faction == 'Aegis' else (.24,.26,.27)
    worn = mix(varied.outputs[0],exposed,edge)

    height = node('ShaderNodeSeparateXYZ')
    links.new(geometry.outputs['Position'],height.inputs[0])
    low = math_node('SUBTRACT',height.outputs['Z'],zmin)
    low = math_node('DIVIDE',low,.62 if role=='Combat' else .95)
    low = math_node('SUBTRACT',1,low,True)
    irregular = math_node('ADD',.40,math_node('MULTIPLY',macro.outputs['Fac'],.60))
    dirt_strength = .46 if surface=='Rubber' else .34 if surface in ('Armor','Concrete','Dark','Dust') else .20 if surface=='Steel' else .06
    dirt = math_node('MULTIPLY',math_node('MULTIPLY',low,irregular),dirt_strength)
    dusty = mix(worn,(.24,.185,.115),dirt)
    # Only a 10% contact term is embedded in albedo; full AO remains separate.
    contact = math_node('ADD',.90,math_node('MULTIPLY',ao.outputs['AO'],.10))
    shaded = node('ShaderNodeMixRGB');shaded.blend_type='MULTIPLY'
    shaded.inputs[0].default_value=1
    links.new(dusty,shaded.inputs[1]);feed(contact,shaded.inputs[2])

    rough = math_node('ADD',roughness-.025,math_node('MULTIPLY',noise.outputs['Fac'],.05))
    rough = math_node('ADD',rough,math_node('MULTIPLY',dirt,.16),True)
    orm = node('ShaderNodeCombineColor');orm.mode='RGB'
    links.new(ao.outputs['AO'],orm.inputs[0]);links.new(rough,orm.inputs[1])
    orm.inputs[2].default_value=.78 if surface=='Steel' else 0
    emission_mask = node('ShaderNodeValue')
    emission_mask.outputs[0].default_value=1 if surface=='Neon' else .35 if surface=='Lamp' else 0
    emission = node('ShaderNodeEmission')
    output = node('ShaderNodeOutputMaterial')
    links.new(emission.outputs[0],output.inputs['Surface'])
    target = node('ShaderNodeTexImage');target.name='RV7_BAKE_TARGET'
    channels = {'Albedo': shaded.outputs[0], 'ORM':orm.outputs[0], 'EmissionMask':emission_mask.outputs[0]}
    return material,emission,target,channels


def render_baked_preview(mesh,images,key,role):
    material=bpy.data.materials.new('RV7 rendered atlas proof');material.use_nodes=True
    n=material.node_tree.nodes;n.clear();l=material.node_tree.links
    p=n.new('ShaderNodeBsdfPrincipled');o=n.new('ShaderNodeOutputMaterial')
    l.new(p.outputs['BSDF'],o.inputs['Surface'])
    base=n.new('ShaderNodeTexImage');base.image=images['Albedo']
    orm=n.new('ShaderNodeTexImage');orm.image=images['ORM']
    mask=n.new('ShaderNodeTexImage');mask.image=images['EmissionMask']
    separate=n.new('ShaderNodeSeparateColor');separate.mode='RGB';l.new(orm.outputs['Color'],separate.inputs[0])
    l.new(base.outputs['Color'],p.inputs['Base Color'])
    l.new(separate.outputs['Green'],p.inputs['Roughness']);l.new(separate.outputs['Blue'],p.inputs['Metallic'])
    l.new(base.outputs['Color'],p.inputs['Emission Color'])
    strength=n.new('ShaderNodeMath');strength.operation='MULTIPLY';strength.inputs[1].default_value=2
    l.new(mask.outputs['Color'],strength.inputs[0]);l.new(strength.outputs[0],p.inputs['Emission Strength'])
    for index in range(len(mesh.data.materials)):mesh.data.materials[index]=material
    scene=bpy.context.scene
    radius=max(mesh.dimensions);points=[mesh.matrix_world@Vector(v) for v in mesh.bound_box]
    center=sum(points,Vector())/8
    ground_material=bpy.data.materials.new('Neutral preview floor');ground_material.diffuse_color=(.19,.17,.14,1)
    bpy.ops.mesh.primitive_cube_add(size=1,location=(0,0,min(v.z for v in points)-.04))
    ground=bpy.context.object;ground.dimensions=(radius*4,radius*4,.05);ground.data.materials.append(ground_material)
    world=bpy.data.worlds.new('Reference surface studio');world.use_nodes=True;scene.world=world
    background=world.node_tree.nodes.new('ShaderNodeBackground')
    world_output=world.node_tree.nodes.new('ShaderNodeOutputWorld')
    world.node_tree.links.new(background.outputs[0],world_output.inputs[0])
    background.inputs[0].default_value=(.22,.25,.29,1)
    background.inputs[1].default_value=.35
    for offset,power in [((1,-1.8,2),95),((-1.6,-.4,1),65),((.4,1.5,1.8),110)]:
        bpy.ops.object.light_add(type='AREA',location=center+Vector(offset)*radius)
        light=bpy.context.object;light.data.energy=power*radius*radius;light.data.size=radius*1.4
        light.rotation_euler=(center-light.location).to_track_quat('-Z','Y').to_euler()
    offset=Vector((-1.2,1.65,1.1)) if role=='HQ' else Vector((1.65,-1.2,1.0))
    bpy.ops.object.camera_add(location=center+offset*radius)
    camera=bpy.context.object;camera.rotation_euler=(center-camera.location).to_track_quat('-Z','Y').to_euler()
    camera.data.type='ORTHO';camera.data.ortho_scale=radius*1.48;scene.camera=camera
    scene.cycles.samples=32;scene.cycles.use_denoising=True
    scene.view_settings.view_transform='AgX'
    scene.render.resolution_x=1200;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
    scene.render.image_settings.file_format='PNG';scene.render.filepath=str(OUT/('Preview_'+key+'.png'))
    bpy.ops.render.render(write_still=True)


def bake(key,size_override=None):
    folder,name,faction,role,default_size=ITEMS[key]
    source=ROOT/'Art/Models'/folder/(name+'.blend')
    fbx=source.with_suffix('.fbx')
    before=digest(source);fbx_before=digest(fbx)
    started=time.monotonic();bpy.ops.wm.open_mainfile(filepath=str(source))
    mesh=bpy.data.objects[name]
    for obj in list(bpy.data.objects):
        if obj!=mesh:bpy.data.objects.remove(obj,do_unlink=True)
    assert mesh.type=='MESH' and len(mesh.data.uv_layers)>0
    mesh.data.uv_layers.active_index=0;mesh.data.uv_layers[0].active_render=True
    bpy.context.view_layer.objects.active=mesh;mesh.select_set(True)
    bpy.context.view_layer.update()
    zmin=min((mesh.matrix_world@Vector(v)).z for v in mesh.bound_box)
    size=size_override or default_size
    scene=bpy.context.scene;backend=configure_cycles(scene)
    scene.render.bake.margin=4 if size<=1024 else 8
    surfaces=[]
    for material in mesh.data.materials:
        surface=material.name.split('.')[0].removeprefix('M_CV2_')
        assert surface in PALETTE[faction],(name,material.name)
        surfaces.append(procedural_surface(material,surface,faction,role,zmin))
    images={};textures={}
    for channel in ('Albedo','ORM','EmissionMask'):
        filename='T_RV7_'+key+'_'+channel+'.png'
        image=bpy.data.images.new('RV7_'+key+'_'+channel,width=size,height=size,alpha=False)
        image.colorspace_settings.name='sRGB' if channel=='Albedo' else 'Non-Color'
        image.file_format='PNG';image.filepath_raw=str(OUT/filename)
        for material,emission,target,channels in surfaces:
            links=material.node_tree.links
            for link in list(emission.inputs['Color'].links):links.remove(link)
            links.new(channels[channel],emission.inputs['Color'])
            target.image=image;material.node_tree.nodes.active=target
        print('REFERENCE_SURFACE_BAKE_START',key,channel,size,backend,flush=True)
        bpy.ops.object.bake(type='EMIT',uv_layer=mesh.data.uv_layers[0].name)
        image.save();images[channel]=image
        pixels=np.array(image.pixels[:],dtype=np.float32).reshape((-1,4))[:,:3]
        textures[channel]={'file':filename,'sha256':digest(OUT/filename),'size':[size,size],
                           'srgb':channel=='Albedo','min_rgb':pixels.min(axis=0).tolist(),
                           'max_rgb':pixels.max(axis=0).tolist()}
    orm=np.array(images['ORM'].pixels[:],dtype=np.float32).reshape((-1,4))
    coverage=orm[:,0]>.01
    assert coverage.mean()>.05,'Empty/inadequate bake coverage'
    assert orm[coverage,1].std()>.015,'Missing roughness differentiation'
    assert orm[coverage,2].max()>.70,'Missing steel metallic mask'
    assert orm[coverage,0].min()<.97 and orm[coverage,0].max()>.8,'Missing contact AO'
    render_baked_preview(mesh,images,key,role)
    assert digest(source)==before and digest(fbx)==fbx_before,'Read-only source was changed'
    record={'key':key,'name':name,'faction':faction,'role':role,
            'source':str(source.relative_to(ROOT)),'source_sha256':before,'source_fbx_sha256':fbx_before,
            'bake_script_sha256':digest(Path(__file__)),
            'source_unchanged':True,'uv_channel':0,'textures':textures,'backend':backend,
            'atlas_coverage':float(coverage.mean()),'elapsed_seconds':round(time.monotonic()-started,2),
            'ao_range_covered':[float(orm[coverage,0].min()),float(orm[coverage,0].max())],
            'roughness_range_covered':[float(orm[coverage,1].min()),float(orm[coverage,1].max())],
            'metallic_policy':'Only named Steel surfaces; armor/rubber/concrete/glass/team are dielectric',
            'provenance':'Original geometry-derived AO/pointiness/position and bounded procedural noise; no game textures',
            'preview':'Preview_'+key+'.png','validation':'PASS'}
    print('REFERENCE_SURFACE_BAKE_PASS',json.dumps(record),flush=True)
    return record


arguments=sys.argv[sys.argv.index('--')+1:] if '--' in sys.argv else []
parser=argparse.ArgumentParser()
parser.add_argument('--only',nargs='+',choices=ITEMS,default=['Aegis_APC8','Ember_APC8'])
parser.add_argument('--size',type=int,choices=(512,1024,2048))
args=parser.parse_args(arguments)
manifest=OUT/'metadata.json'
records=json.loads(manifest.read_text(encoding='utf-8')).get('records',[]) if manifest.exists() else []
for key in args.only:
    record=bake(key,args.size)
    records=[r for r in records if r['key']!=key]+[record]
    manifest.write_text(json.dumps({'kind':'Original ReferenceV7 UV baked surfaces','records':records},indent=2)+'\n',encoding='utf-8')
print('REFERENCE_SURFACES_V7_COMPLETE',len(args.only),flush=True)
