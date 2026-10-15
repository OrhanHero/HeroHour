"""Refresh six existing portrait paths from original MilitaryDetailV6 blends.
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
 'Team': ((.48,.045,.018),.08,.61), 'Glass': ((.085,.025,.015),.40,.24),
 'Neon': ((.90,.12,.025),.10,.30), 'Concrete': ((.24,.23,.21),0.,.87),
 'Dust': ((.25,.205,.15),0.,.90), 'Lamp': ((.90,.32,.08),0.,.36)}}
metadata_path=OUT/'metadata.json'
metadata=json.loads(metadata_path.read_text(encoding='utf-8'))
assert len(metadata['records'])==22
preserved={r['file']:dict(r) for r in metadata['records'] if r['role'] not in ('HQ','Factory','Depot')}
preserved_images={name:hashlib.sha256((OUT/name).read_bytes()).hexdigest() for name in preserved}
manifest=json.loads((ROOT/'Art/Models/MilitaryDetailV6/manifest.json').read_text())
assert len(manifest['assets'])==3
records=[]
for item in manifest['assets']:
    assert item['validation']=='PASS' and item['fbx_roundtrip']
    role,name=item['role'],item['name']
    assert role in ('HQ','Factory','Depot')
    source=ROOT/'Art/Models/MilitaryDetailV6'/(name+'.blend')
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    for faction,palette in PALETTES.items():
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
        bpy.ops.object.camera_add(location=center+Vector((1.2,-1.65,1.1))*radius)
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
        records.append(dict(role=role,faction=faction,file=dest.name,source=str(source.relative_to(ROOT)),
                            source_sha256=digest,original_geometry=True,source_unchanged=True))
        records[-1]['file_sha256']=hashlib.sha256(dest.read_bytes()).hexdigest()
        print('MILITARY_PORTRAIT_V6_PASS',faction,role,flush=True)
assert len(records)==6
updates={r['file']:r for r in records}
metadata['records']=[updates.get(r['file'],r) for r in metadata['records']]
assert len(metadata['records'])==22
for record in metadata['records']:
    if record['file'] in preserved:
        assert record==preserved[record['file']]
        assert hashlib.sha256((OUT/record['file']).read_bytes()).hexdigest()==preserved_images[record['file']]
metadata_path.write_text(json.dumps(metadata,indent=2)+'\n',encoding='utf-8')
print('MILITARY_PORTRAITS_V6_COMPLETE 6; remaining 16 images and records unchanged',flush=True)
