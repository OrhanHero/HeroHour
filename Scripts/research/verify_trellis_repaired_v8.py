"""Validate own repaired GLB and copy embedded texture bytes without image editing."""
import hashlib,json,struct,io,shutil
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[2];BASE=ROOT/'Saved/Research/TrellisV8/repaired-texture';OUT=ROOT/'Art/Models/TrellisV8-Repaired';EVID=ROOT/'docs/verification/2026-10-16/reference-v8/trellis'
source=OUT/'AegisHQ-Remeshed512-PBR2048_00001_.glb';raw=source.read_bytes();sha=hashlib.sha256(raw).hexdigest()
assert sha=='802747bb8124a02ad0c0f899f4a2dc5b9578f23bff12ea534c28d3b5cd201f03'
chunks={};offset=12
while offset<len(raw):
 size,kind=struct.unpack_from('<I4s',raw,offset);offset+=8;chunks[kind]=raw[offset:offset+size];offset+=size
g=json.loads(chunks[b'JSON']);binary=chunks[b'BIN\0'];p=g['meshes'][0]['primitives'][0]
attrs=p['attributes'];assert 'NORMAL' in attrs and 'TEXCOORD_0' in attrs
assert g['accessors'][attrs['NORMAL']]['count']==g['accessors'][attrs['POSITION']]['count']
material=g['materials'][p['material']];pbr=material['pbrMetallicRoughness'];images=[]
for key,label in [('baseColorTexture','BaseColor'),('metallicRoughnessTexture','MetallicRoughness')]:
 index=g['textures'][pbr[key]['index']]['source'];image=g['images'][index];view=g['bufferViews'][image['bufferView']];start=view.get('byteOffset',0);data=binary[start:start+view['byteLength']]
 assert image['mimeType']=='image/png';target=OUT/(label+'.png')
 if target.exists():assert target.read_bytes()==data
 else:target.write_bytes(data)
 with Image.open(io.BytesIO(data)) as im:
  assert im.size==(2048,2048);images.append({'semantic':label,'file':target.name,'size':list(im.size),'sha256':hashlib.sha256(data).hexdigest()})
report={'glb_sha256':sha,'triangles':g['accessors'][p['indices']]['count']//3,'uv_vertices':g['accessors'][attrs['POSITION']]['count'],'normal_attributes':True,'textures':images,'pbr':pbr,'no_normalmap':'normalTexture' not in material,'no_emissivemap':'emissiveTexture' not in material,'source_unchanged':hashlib.sha256(source.read_bytes()).hexdigest()==sha,'not_yet_unreal_imported':True,'provenance':'Own project-derived generated input; original same-seed TRELLIS voxel texture sampled onto remeshed, freshly unwrapped surface; no Tempest game art'}
(EVID/'repaired-pbr-verification.json').write_text(json.dumps(report,indent=2),encoding='utf-8');(OUT/'verification.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
for src,name in [('status.json','repaired-run-status.json'),('schema-validation.json','repaired-schema-validation.json'),('workflow-api.json','repaired-workflow-executed.json')]:shutil.copyfile(BASE/src,EVID/name)
shutil.copyfile(BASE/'workflow-api.json',OUT/'generation-workflow-api.json')
print('REPAIRED_PBR_VERIFIED',json.dumps(report))
