"""Replace only the existing Aegis Factory portrait from the actual FV10 PBR mesh."""
import bpy
import hashlib
import json
from pathlib import Path
from mathutils import Vector
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'Art/UI/PortraitsV5'
TARGET='T_HR_Aegis_Factory.png';SOURCE=ROOT/'Art/Models/TrellisFactoryV10/SM_FV10_AegisFactory.blend'
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
metadata_path=OUT/'metadata.json';metadata_hash=sha(metadata_path);metadata=json.loads(metadata_path.read_text(encoding='utf-8'));active_target_hash=sha(OUT/TARGET)
PENDING=ROOT/'Art/Models/TrellisFactoryV10/Portrait-Pending.png'
assert len(metadata['records'])==22
record=next(r for r in metadata['records'] if r['file']==TARGET)
assert record['faction']=='Aegis' and record['role']=='Factory'
preserved={r['file']:dict(metadata=r,sha256=sha(OUT/r['file'])) for r in metadata['records'] if r['file']!=TARGET}
assert len(preserved)==21
audit=ROOT/'Saved/Research/FV10Portrait';audit.mkdir(parents=True,exist_ok=True)
(audit/'PreservedBefore.json').write_text(json.dumps(preserved,indent=2)+'\n',encoding='utf-8')
proof=ROOT/'docs/verification/2026-10-16/reference-v10/factory-portrait';proof.mkdir(parents=True,exist_ok=True)
(proof/'Preserved21.json').write_text(json.dumps(preserved,indent=2)+'\n',encoding='utf-8')
if not (audit/TARGET).exists():(audit/TARGET).write_bytes((OUT/TARGET).read_bytes())
if not (audit/'metadata-before.json').exists():(audit/'metadata-before.json').write_bytes(metadata_path.read_bytes())
source_hash=sha(SOURCE)
bpy.ops.wm.open_mainfile(filepath=str(SOURCE));mesh=bpy.data.objects['SM_FV10_AegisFactory']
for obj in list(bpy.data.objects):
 if obj!=mesh:bpy.data.objects.remove(obj,do_unlink=True)
assert len(mesh.data.materials)==1
material=mesh.data.materials[0];material.use_nodes=True;nodes=material.node_tree.nodes;nodes.clear();links=material.node_tree.links
shader=nodes.new('ShaderNodeBsdfPrincipled');output=nodes.new('ShaderNodeOutputMaterial');links.new(shader.outputs['BSDF'],output.inputs['Surface'])
textures={}
for key,filename,srgb in [('Albedo','T_FV10_AegisFactory_Albedo.png',True),('MetallicRoughness','T_FV10_AegisFactory_MR.png',False)]:
 path=ROOT/'Art/Textures/TrellisFactoryV10'/filename;texture=nodes.new('ShaderNodeTexImage')
 texture.image=bpy.data.images.load(str(path),check_existing=False);texture.image.colorspace_settings.name='sRGB' if srgb else 'Non-Color'
 textures[key]=dict(file=str(path.relative_to(ROOT)),sha256=sha(path));textures[key]['node']=texture
links.new(textures['Albedo']['node'].outputs['Color'],shader.inputs['Base Color'])
split=nodes.new('ShaderNodeSeparateColor');links.new(textures['MetallicRoughness']['node'].outputs['Color'],split.inputs['Color'])
links.new(split.outputs['Green'],shader.inputs['Roughness']);links.new(split.outputs['Blue'],shader.inputs['Metallic'])
scene=bpy.context.scene;points=[mesh.matrix_world@Vector(p) for p in mesh.bound_box];center=sum(points,Vector())/8;radius=max(mesh.dimensions)
world=bpy.data.worlds.new('V5 neutral studio');scene.world=world;world.use_nodes=True;world.node_tree.nodes.clear()
bg=world.node_tree.nodes.new('ShaderNodeBackground');wo=world.node_tree.nodes.new('ShaderNodeOutputWorld');world.node_tree.links.new(bg.outputs[0],wo.inputs[0])
bg.inputs[0].default_value=(.27,.31,.36,1);bg.inputs[1].default_value=.30
for offset,power,color in [((1.,-1.8,2.),90,(1.,.88,.72)),((-1.6,-.4,1.),60,(.62,.78,1.)),((.4,1.5,1.8),130,(.92,.96,1.))]:
 bpy.ops.object.light_add(type='AREA',location=center+Vector(offset)*radius);lamp=bpy.context.object;lamp.data.energy=power*radius*radius;lamp.data.size=radius*1.3;lamp.data.color=color
 lamp.rotation_euler=(center-lamp.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=center+Vector((-1.2,1.65,1.1))*radius);cam=bpy.context.object
cam.rotation_euler=(center-cam.location).to_track_quat('-Z','Y').to_euler();cam.data.type='ORTHO';scene.camera=cam
bpy.context.view_layer.update();projected=[cam.matrix_world.inverted()@p for p in points]
cam.data.ortho_scale=max(max(p.x for p in projected)-min(p.x for p in projected),max(p.y for p in projected)-min(p.y for p in projected))*1.18
scene.render.engine='CYCLES';scene.cycles.samples=24;scene.cycles.use_denoising=True
prefs=bpy.context.preferences.addons['cycles'].preferences;prefs.compute_device_type='OPTIX';prefs.get_devices()
for device in prefs.devices:device.use=device.type=='OPTIX'
scene.cycles.device='GPU';scene.render.film_transparent=True
scene.render.resolution_x=512;scene.render.resolution_y=512;scene.render.resolution_percentage=100
scene.render.image_settings.file_format='PNG';scene.render.image_settings.color_mode='RGBA'
scene.view_settings.view_transform='AgX';scene.view_settings.exposure=0.;scene.render.filepath=str(PENDING)
bpy.ops.render.render(write_still=True)
rendered=bpy.data.images.load(str(PENDING),check_existing=False);assert list(rendered.size)==[512,512] and rendered.channels==4
pixels=list(rendered.pixels);occupied=[i//4 for i in range(3,len(pixels),4) if pixels[i]>.01];assert occupied
xs=[i%512 for i in occupied];ys=[i//512 for i in occupied];bounds=[min(xs),min(ys),max(xs),max(ys)]
assert min(bounds[:2])>8 and max(bounds[2:])<503,bounds
assert sha(SOURCE)==source_hash
record.clear();record.update(role='Factory',faction='Aegis',file=TARGET,source=str(SOURCE.relative_to(ROOT)),source_sha256=source_hash,original_geometry=True,
 ai_assisted_geometry=True,provenance='Own concept-conditioned TRELLIS.2 geometry; actual FV10 mesh and PBR atlases; no commercial game art',
 source_unchanged=True,alpha_bounds=bounds,surface_key='Aegis_Factory_TrellisV10',albedo_sha256=textures['Albedo']['sha256'],
 file_sha256=sha(PENDING),metallic_roughness_sha256=textures['MetallicRoughness']['sha256'])
for r in metadata['records']:
 if r['file'] in preserved:
  assert r==preserved[r['file']]['metadata'];assert sha(OUT/r['file'])==preserved[r['file']]['sha256']
metadata['render']='FV10 Aegis Factory from actual TRELLIS mesh/PBR; other 21 existing portraits and metadata records preserved'
assert sha(metadata_path)==metadata_hash and sha(OUT/TARGET)==active_target_hash
(proof/'ProposedRecord.json').write_text(json.dumps(record,indent=2)+'\n',encoding='utf-8')
report=dict(result='PASS_PENDING_NOT_ACTIVATED',target=record,pending_image=str(PENDING.relative_to(ROOT)),active_factory_image_unchanged=True,whole_active_metadata_unchanged=True,preserved_image_and_record_count=21,source_blend_unchanged=True)
(audit/'Result.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
(proof/'Render.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('TRELLIS_FACTORY_PORTRAIT_V10_PASS '+json.dumps(report),flush=True)
