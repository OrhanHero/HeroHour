"""Decode only two supplied MPCs with checked UE complete-type property frames.

No assets are loaded into Unreal. This is not a general uasset reader and does
not establish that the shipping HUD uses these editor defaults at runtime.
"""
import hashlib
import json
import math
import struct
from pathlib import Path

ROOT=Path('F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject/Content/Tempest/UI/Materials')
OUT=Path(__file__).resolve().parent

def decode(path):
 data=path.read_bytes()
 assert data[:4]==bytes.fromhex('c1832a9e')
 legacy,ue3,ue4,ue5,licensee=struct.unpack_from('<iiiii',data,4)
 assert legacy==-8 and ue4==522 and ue5==1012 and licensee==0
 count=struct.unpack_from('<i',data,24)[0]
 assert 0<=count<=32
 pos=28+20*count
 total_header=struct.unpack_from('<i',data,pos)[0];pos+=4
 folderlen=struct.unpack_from('<i',data,pos)[0];pos+=4
 assert 0<folderlen<256
 pos+=folderlen
 flags,namecount,nameoffset=struct.unpack_from('<Iii',data,pos)
 assert 0<namecount<256 and nameoffset<total_header<len(data)
 names=[];p=nameoffset
 for index in range(namecount):
  length=struct.unpack_from('<i',data,p)[0];p+=4
  assert 0<length<256
  raw=data[p:p+length];assert raw[-1]==0
  names.append(raw[:-1].decode('ascii'));p+=length+4
 assert p<=total_header
 def fname(name):return struct.pack('<ii',names.index(name),0)
 # Complete type tree: StructProperty<LinearColor</Script/CoreUObject>>;
 # each node is FName + uint32 child count. Size=16, tag flags=8.
 prefix=(fname('DefaultValue')+fname('StructProperty')+struct.pack('<I',1)
         +fname('LinearColor')+struct.pack('<I',1)
         +fname('/Script/CoreUObject')+struct.pack('<I',0)+struct.pack('<iB',16,8))
 nameprefix=fname('ParameterName')+fname('NameProperty')+struct.pack('<IiB',0,8,0)
 arrayprefix=(fname('VectorParameters')+fname('ArrayProperty')+struct.pack('<I',1)
              +fname('StructProperty')+struct.pack('<I',1)
              +fname('CollectionVectorParameter')+struct.pack('<I',1)
              +fname('/Script/Engine')+struct.pack('<I',0))
 arrayoff=data.find(arrayprefix,total_header)
 assert arrayoff>=total_header and data.find(arrayprefix,arrayoff+1)<0
 array_size,array_flags,vector_count=struct.unpack_from('<iBi',data,arrayoff+len(arrayprefix))
 assert array_flags==0 and 0<vector_count<=64
 records=[];pos=total_header
 while True:
  off=data.find(prefix,pos)
  if off<0:break
  valueoff=off+len(prefix)
  rgba=list(struct.unpack_from('<ffff',data,valueoff))
  assert all(math.isfinite(f) and 0<=f<=10 for f in rgba)
  nameoff=valueoff+16
  assert data[nameoff:nameoff+len(nameprefix)]==nameprefix
  ix,number=struct.unpack_from('<ii',data,nameoff+len(nameprefix))
  assert 0<=ix<len(names) and 0<=number<=64
  name=names[ix]+('_'+str(number-1) if number else '')
  records.append({'name':name,'name_map_value':names[ix],'fname_number':number,'linear_rgba':rgba,'property_tag_offset':off,'value_offset':valueoff,'parameter_name_value_offset':nameoff+len(nameprefix),'frame_sha256':hashlib.sha256(data[off:nameoff+len(nameprefix)+8]).hexdigest()})
  pos=nameoff+len(nameprefix)+8
 expected=[n for n in names if n.startswith(('ThemeColor_','SelectionColor_'))]
 assert set(expected).issubset(r['name'] for r in records),(expected,records)
 assert len(records)==vector_count and len(set(r['name'] for r in records))==vector_count
 assert all(r['name'].startswith(('ThemeColor_','SelectionColor_','PlayerColor')) for r in records)
 return {'path':str(path),'sha256':hashlib.sha256(data).hexdigest(),'legacy_version':legacy,'ue4_version':ue4,'ue5_version':ue5,'name_count':namecount,'name_map_offset':nameoffset,'total_header_size':total_header,'vector_array_offset':arrayoff,'declared_vector_count':vector_count,'records':records,'verification':'Every declared vector element has one checked LinearColor DefaultValue + NameProperty ParameterName frame; all names unique and expected semantic palette names present. FName number suffixes preserved. No nearest-string association.'}

results=[decode(ROOT/name) for name in ['MPC_UI.uasset','MPC_UI_Constant.uasset']]
out={'method':'bounded complete-type property frame decoder for two explicit editor MPC assets','results':results,'limitations':['Stored editor defaults only; dynamic runtime overrides and actual Shipping HUD bindings unobserved.','No general package parsing, graph evaluation, texture decoding or original shader/code reuse.','Linear float values are not sRGB display pixels.']}
(OUT/'ui-palette.json').write_text(json.dumps(out,indent=2),encoding='utf-8')
print('UI_PALETTE_FRAME_PASS',sum(len(r['records']) for r in results),'colors')
for item in results:
 for record in item['records']:
  if 'GDF' in record['name'] or record['name'].startswith('SelectionColor_'):
   print(record['name'],record['linear_rgba'])
