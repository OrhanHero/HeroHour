"""Read bounded shipped editor metadata; never deserialize/copy asset payloads."""
import hashlib
import json
import re
import struct
from pathlib import Path

ROOT=Path('F:/Tempest-Rising-SteamRIP.com/Tempest Rising')
OUT=Path(__file__).resolve().parent
CONTENT=ROOT/'MapEditorProject/Content/Tempest'
files=[
 CONTENT/'UI/Materials/MPC_UI.uasset',
 CONTENT/'UI/Materials/MPC_UI_Constant.uasset',
 CONTENT/'MaterialLibrary/Vehicles/M_MASTER_Vehicles.uasset',
]
for folder in [CONTENT/'GDF/Vehicles/GDF_Harvester',CONTENT/'GDF/Vehicles/GDF_RiotVan',CONTENT/'GDF/Structures/BD_GDF_ConYard']:
 files.extend(p for p in folder.rglob('*.uasset') if p.name.startswith(('MI_','SK_','AM_','Anim_','BD_')))

records=[]
for path in sorted(set(files)):
 data=path.read_bytes()
 assert len(data)<=1_000_000,(path,len(data))
 strings=[]
 for match in re.finditer(rb'[\x20-\x7e]{5,}',data):
  value=match.group().decode('ascii')
  if any(word in value.lower() for word in ('/game/','/script/','material','texture','normal','rough','color','tint','team','skin','damage','destroy','harvest','unload','move','idle','rmo','animation','skeleton','scalar','vector','selected','alpha','scale','contrast','opacity','metal','emiss','highlight')):
   strings.append({'file_offset':match.start(),'encoding':'ascii','value':value})
 records.append({'path':str(path),'size':len(data),'sha256':hashlib.sha256(data).hexdigest(),'package_tag_hex':data[:4].hex(),'legacy_file_version':struct.unpack_from('<i',data,4)[0],'strings':strings,'coverage':'ASCII sequences >=5 bytes in uasset header/metadata only; no uexp or ubulk payload decoding; string presence does not prove property assignment or runtime use'})

inventory=[]
for folder in [CONTENT/'GDF/Vehicles/GDF_Harvester',CONTENT/'GDF/Vehicles/GDF_RiotVan',CONTENT/'GDF/Structures/BD_GDF_ConYard']:
 inventory.append({'root':str(folder),'files':[{'relative_path':str(p.relative_to(folder)),'size':p.stat().st_size} for p in sorted(folder.rglob('*')) if p.is_file()]})

result={'method':'read-only, explicit three directory inventories and selected bounded uasset metadata scans','records':records,'inventory':inventory,'limits':['No material graph evaluation, pixel decoding, animation playback or geometry extraction.','Names do not establish channel packing, animation duration, actual material assignment or shipping runtime usage.','Editor content is a separately shipped artifact, not proven identical to the shipping IoStore package.']}
output=OUT/'visual-metadata.json'
output.write_text(json.dumps(result,indent=2),encoding='utf-8')
print('VISUAL_METADATA_READ_PASS',len(records),'records',sum(len(r['strings']) for r in records),'strings')
for record in records:
 if record['path'].endswith(('MPC_UI.uasset','MPC_UI_Constant.uasset','M_MASTER_Vehicles.uasset')):
  print(record['path'],record['sha256'])
  print(json.dumps(record['strings'],ensure_ascii=True)[:7000])
