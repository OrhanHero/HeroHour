"""Refresh four atlas portraits plus nine legacy Ember roles with green markings.
Run Blender --background --python this_file.py. Source blends are read only.
"""
import bpy
import hashlib
import json
from pathlib import Path
from mathutils import Vector
ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'Art/UI/PortraitsV5'
OUT.mkdir(parents=True, exist_ok=True)
ROLES = {'Combat': 'Striker', 'Collector': 'Harvester', 'HQ': 'CommandHub',
         'Depot': 'Refinery', 'Factory': 'VehiclePlant', 'PowerPlant': 'PowerPlant',
         'Barracks': 'Barracks', 'Radar': 'Radar', 'Defense': 'Defense',
         'Infantry': 'Infantry', 'Siege': 'Siege'}
# Literal linear palettes match polish_faction_materials_v5.py; no Unreal imports.
PALETTES = {
 'Aegis': {'Armor': ((.48,.42,.30),.08,.66), 'Steel': ((.24,.30,.34),.78,.43),
 'Dark': ((.045,.060,.070),.08,.77), 'Rubber': ((.018,.022,.025),0.,.90),
 'Team': ((.025,.18,.48),.10,.58), 'Glass': ((.018,.075,.13),.40,.22),
 'Neon': ((.025,.35,.80),.10,.30), 'Concrete': ((.34,.32,.27),0.,.86),
 'Dust': ((.30,.255,.18),0.,.90), 'Lamp': ((.85,.57,.22),0.,.36)},
 'Ember': {'Armor': ((.085,.095,.105),.12,.64), 'Steel': ((.21,.22,.23),.80,.40),
 'Dark': ((.024,.028,.032),.08,.79), 'Rubber': ((.014,.017,.019),0.,.91),
 'Team': ((.025,.32,.13),.08,.61), 'Glass': ((.012,.055,.032),.40,.24),
 'Neon': ((.025,.70,.28),.10,.30), 'Concrete': ((.24,.23,.21),0.,.87),
 'Dust': ((.25,.205,.15),0.,.90), 'Lamp': ((.90,.32,.08),0.,.36)}}
metadata_path=OUT/'metadata.json'
metadata=json.loads(metadata_path.read_text(encoding='utf-8'))
assert len(metadata['records'])==22
surface_root=ROOT/'Art/Textures/ReferenceV7'
surface_records={r['key']:r for r in json.loads((surface_root/'metadata.json').read_text(encoding='utf-8'))['records']}
targets={f'T_HR_{faction}_{role}.png' for faction in ('Aegis','Ember') for role in ('HQ','Combat')}
legacy_ember=[r for r in metadata['records'] if r['faction']=='Ember' and r['role'] not in ('HQ','Combat')]
assert len(legacy_ember)==9
targets.update(r['file'] for r in legacy_ember)
assert len(targets)==13
preserved={r['file']:dict(r) for r in metadata['records'] if r['file'] not in targets}
assert len(preserved)==9 and all(r['faction']=='Aegis' for r in preserved.values())
preserved_images={name:hashlib.sha256((OUT/name).read_bytes()).hexdigest() for name in preserved}
items=[('Aegis','HQ','SM_RV7_AegisHQ','ReferenceHQV7'),
       ('Aegis','Combat','SM_HR_Aegis_APC8','ReferenceAPCV7'),
       ('Ember','HQ','SM_RV7_EmberHQ','ReferenceEmberHQV7'),
       ('Ember','Combat','SM_HR_Ember_APC8','ReferenceEmberAPCV7')]
for record in legacy_ember:
    source=Path(record['source'])
    folder=str(source.parent.relative_to(Path('Art/Models')))
    items.append(('Ember',record['role'],source.stem,folder))
assert len(items)==13
assert 'REFERENCE_APC_V7_PASS' in (ROOT/'Saved/Logs/ReferenceAPCV7-Blender.log').read_text(encoding='utf-8',errors='replace')
records=[]
for selected_faction,role,name,folder in items:
    source=ROOT/'Art/Models'/folder/(name+'.blend')
    digest=hashlib.sha256(source.read_bytes()).hexdigest()
    for faction in [selected_faction]:
        palette=dict(PALETTES[faction])
        if role=='HQ':
            palette['Armor']=((.60,.64,.64),.08,.66)
            palette['Concrete']=((.45,.47,.45),0.,.86)
        bpy.ops.wm.open_mainfile(filepath=str(source))
        mesh=bpy.data.objects[name]
        for obj in list(bpy.data.objects):
            if obj != mesh:
                bpy.data.objects.remove(obj,do_unlink=True)
        for material in mesh.data.materials:
            surface=material.name.split('.')[0].removeprefix('M_CV2_')
            assert surface in palette, (name,material.name)
            color,metal,rough=palette[surface]
            material.use_nodes=True
            nodes=material.node_tree.nodes;nodes.clear()
            shader=nodes.new('ShaderNodeBsdfPrincipled');output=nodes.new('ShaderNodeOutputMaterial')
            shader.inputs['Base Color'].default_value=(*color,1)
            shader.inputs['Metallic'].default_value=metal
            shader.inputs['Roughness'].default_value=rough
            if surface in ('Neon','Lamp'):
                shader.inputs['Emission Color'].default_value=(*color,1)
                shader.inputs['Emission Strength'].default_value=2. if surface=='Neon' else 1.
            material.node_tree.links.new(shader.outputs['BSDF'],output.inputs['Surface'])
        # Use the same baked UV0 surfaces as the actual runtime mesh, including
        # dirt, roughness, metal and the restricted lamp mask. Never save to the
        # source .blend; portrait rendering is a read-only consumer of it.
        is_reference=role in ('HQ','Combat')
        if is_reference:
            surface_key=faction+('_HQ' if role=='HQ' else '_APC8')
            surface_record=surface_records[surface_key]
            assert surface_record['source_sha256']==digest,'Atlas does not match final geometry'
            baked=bpy.data.materials.new('Portrait_'+surface_key);baked.use_nodes=True
            nodes=baked.node_tree.nodes;nodes.clear();links=baked.node_tree.links
            shader=nodes.new('ShaderNodeBsdfPrincipled');output=nodes.new('ShaderNodeOutputMaterial')
            links.new(shader.outputs['BSDF'],output.inputs['Surface'])
            images={}
            for channel in ('Albedo','ORM','EmissionMask'):
                info=surface_record['textures'][channel];path=surface_root/info['file']
                assert hashlib.sha256(path.read_bytes()).hexdigest()==info['sha256']
                tex=nodes.new('ShaderNodeTexImage');tex.image=bpy.data.images.load(str(path),check_existing=False)
                tex.image.colorspace_settings.name='sRGB' if channel=='Albedo' else 'Non-Color'
                images[channel]=tex
            links.new(images['Albedo'].outputs['Color'],shader.inputs['Base Color'])
            split=nodes.new('ShaderNodeSeparateColor');links.new(images['ORM'].outputs['Color'],split.inputs['Color'])
            links.new(split.outputs['Green'],shader.inputs['Roughness']);links.new(split.outputs['Blue'],shader.inputs['Metallic'])
            links.new(images['Albedo'].outputs['Color'],shader.inputs['Emission Color'])
            strength=nodes.new('ShaderNodeMath');strength.operation='MULTIPLY';strength.inputs[1].default_value=2.
            links.new(images['EmissionMask'].outputs['Color'],strength.inputs[0]);links.new(strength.outputs[0],shader.inputs['Emission Strength'])
            for slot in mesh.material_slots:slot.material=baked
        scene=bpy.context.scene
        points=[mesh.matrix_world@Vector(p) for p in mesh.bound_box]
        center=sum(points,Vector())/8; radius=max(mesh.dimensions)
        world=bpy.data.worlds.new('V5 neutral studio');scene.world=world;world.use_nodes=True
        bg=world.node_tree.nodes.new('ShaderNodeBackground');wo=world.node_tree.nodes.new('ShaderNodeOutputWorld')
        world.node_tree.links.new(bg.outputs[0],wo.inputs[0])
        bg.inputs[0].default_value=(.27,.31,.36,1)
        bg.inputs[1].default_value=.30
        # Scale energy with subject area: infantry and HQ get equivalent exposure.
        for offset,power,color in [((1.,-1.8,2.),90,(1.,.88,.72)),
                                   ((-1.6,-.4,1.),60,(.62,.78,1.)),
                                   ((.4,1.5,1.8),130,(.92,.96,1.))]:
            bpy.ops.object.light_add(type='AREA',location=center+Vector(offset)*radius)
            lamp=bpy.context.object;lamp.data.energy=power*radius*radius
            lamp.data.size=radius*1.3;lamp.data.color=color
            lamp.rotation_euler=(center-lamp.location).to_track_quat('-Z','Y').to_euler()
        # HQ geometry bakes a 180-degree yaw so its garage faces the game camera.
        camera_offset=(Vector((-1.2,1.65,1.1)) if role=='HQ' else Vector((1.65,-1.2,1.0))) if is_reference else Vector((1.2,-1.65,1.1))
        bpy.ops.object.camera_add(location=center+camera_offset*radius)
        cam=bpy.context.object;cam.rotation_euler=(center-cam.location).to_track_quat('-Z','Y').to_euler()
        cam.data.type='ORTHO';scene.camera=cam
        # Fit projected geometry with consistent 9% safety margin, not bounding sphere.
        bpy.context.view_layer.update()
        inverse=cam.matrix_world.inverted()
        projected=[inverse@p for p in points]
        cam.data.ortho_scale=max(max(p.x for p in projected)-min(p.x for p in projected),
                                  max(p.y for p in projected)-min(p.y for p in projected))*1.18
        scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True
        scene.render.film_transparent=True
        scene.render.resolution_x=512;scene.render.resolution_y=512;scene.render.resolution_percentage=100
        scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA'
        scene.view_settings.view_transform='AgX';scene.view_settings.exposure=0.
        dest=OUT/('T_HR_'+faction+'_'+role+'.png');scene.render.filepath=str(dest)
        bpy.ops.render.render(write_still=True)
        assert hashlib.sha256(source.read_bytes()).hexdigest()==digest, 'Source blend changed'
        rendered=bpy.data.images.load(str(dest),check_existing=False)
        pixels=list(rendered.pixels); occupied=[i//4 for i in range(3,len(pixels),4) if pixels[i]>.01]
        assert occupied,'Empty portrait'
        xs=[i%512 for i in occupied];ys=[i//512 for i in occupied]
        alpha_bounds=[min(xs),min(ys),max(xs),max(ys)]
        assert min(alpha_bounds[:2])>8 and max(alpha_bounds[2:])<503,alpha_bounds
        bpy.data.images.remove(rendered)

        records.append(dict(role=role,faction=faction,file=dest.name,source=str(source.relative_to(ROOT)),
                            source_sha256=digest,original_geometry=True,source_unchanged=True))
        records[-1]['alpha_bounds']=alpha_bounds
        if is_reference:
            records[-1]['surface_key']=surface_key
            records[-1]['albedo_sha256']=surface_record['textures']['Albedo']['sha256']
        else:
            records[-1]['palette_variant']='EmberGreenV7'
            records[-1]['team_tint_linear']=list(PALETTES['Ember']['Team'][0])
            records[-1]['neon_tint_linear']=list(PALETTES['Ember']['Neon'][0])
        records[-1]['file_sha256']=hashlib.sha256(dest.read_bytes()).hexdigest()
        print('REFERENCE_PORTRAIT_V7_PASS',faction,role,flush=True)
assert len(records)==13
updates={r['file']:r for r in records}
metadata['records']=[updates.get(r['file'],r) for r in metadata['records']]
assert len(metadata['records'])==22
metadata['render']='Cycles 24 samples denoised: four ReferenceV7 UV0 atlas portraits, nine green Ember legacy portraits, nine preserved Aegis V5 portraits'
for record in metadata['records']:
    if record['file'] in preserved:
        assert record==preserved[record['file']]
        assert hashlib.sha256((OUT/record['file']).read_bytes()).hexdigest()==preserved_images[record['file']]
metadata_path.write_text(json.dumps(metadata,indent=2)+'\n',encoding='utf-8')
print('REFERENCE_PORTRAITS_V7_COMPLETE 13; remaining 9 Aegis images and records unchanged',flush=True)
