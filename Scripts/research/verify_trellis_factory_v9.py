"""Validate GLB structure and copy embedded PBR bytes unchanged."""
import hashlib
import io
import json
import struct
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[2]
BASE=ROOT/'Saved/Research/TrellisFactoryV9'
OUT=ROOT/'Art/Models/TrellisFactoryV9-Candidate'
status=json.loads((BASE/'status.json').read_text(encoding='utf-8'))
assert status['status']=='success' and status['user_db_unchanged']
assert len(status['outputs'])==1
source=ROOT/status['outputs'][0]['file'];raw=source.read_bytes()
sha=lambda data:hashlib.sha256(data).hexdigest()
assert sha(raw)==status['outputs'][0]['sha256']
magic,version,length=struct.unpack_from('<III',raw,0)
assert magic==0x46546c67 and version==2 and length==len(raw)
chunks={};offset=12
while offset<len(raw):
    size,kind=struct.unpack_from('<I4s',raw,offset);offset+=8
    chunks[kind]=raw[offset:offset+size];offset+=size
g=json.loads(chunks[b'JSON']);binary=chunks[b'BIN\0']
assert len(g['meshes'])==1 and len(g['meshes'][0]['primitives'])==1
p=g['meshes'][0]['primitives'][0];attrs=p['attributes']
assert 'NORMAL' in attrs and 'TEXCOORD_0' in attrs
vertices=g['accessors'][attrs['POSITION']]['count']
assert g['accessors'][attrs['NORMAL']]['count']==vertices
assert g['accessors'][attrs['TEXCOORD_0']]['count']==vertices
material=g['materials'][p['material']];pbr=material['pbrMetallicRoughness'];images=[]
for key,label in [('baseColorTexture','BaseColor'),('metallicRoughnessTexture','MetallicRoughness')]:
    image=g['images'][g['textures'][pbr[key]['index']]['source']]
    view=g['bufferViews'][image['bufferView']];start=view.get('byteOffset',0)
    data=binary[start:start+view['byteLength']]
    assert image['mimeType']=='image/png'
    with Image.open(io.BytesIO(data)) as im:
        assert im.size==(2048,2048)
    target=OUT/(label+'.png')
    if target.exists():assert target.read_bytes()==data
    else:target.write_bytes(data)
    images.append(dict(semantic=label,file=target.name,size=[2048,2048],sha256=sha(data)))
report=dict(glb=source.name,glb_sha256=sha(raw),triangles=g['accessors'][p['indices']]['count']//3,
    uv_vertices=vertices,normal_attributes=True,textures=images,no_normalmap='normalTexture' not in material,
    no_emissivemap='emissiveTexture' not in material,not_unreal_imported=True,
    provenance='Original HeroHour concept-derived factory input; one repaired TRELLIS workflow, fresh voxel PBR bake')
(OUT/'verification.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
(OUT/'generation-workflow-api.json').write_bytes((BASE/'workflow-api.json').read_bytes())
print('FACTORY_GLB_VERIFIED',json.dumps(report))
