"""Promote the reviewed real-model factory portrait without touching other icons."""
from pathlib import Path
import hashlib
import json
import shutil
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'Art/UI/PortraitsV5'
PROOF=ROOT/'docs/verification/2026-10-16/reference-v10/factory-portrait'
record=json.loads((PROOF/'ProposedRecord.json').read_text(encoding='utf-8'))
preserved=json.loads((PROOF/'Preserved21.json').read_text(encoding='utf-8'))
metadata=json.loads((OUT/'metadata.json').read_text(encoding='utf-8'))
digest=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
for current in metadata['records']:
    if current['file']==record['file']:continue
    evidence=preserved[current['file']]
    assert current==evidence['metadata']
    assert digest(OUT/current['file'])==evidence['sha256']
pending=ROOT/'Art/Models/TrellisFactoryV10/Portrait-Pending.png'
assert digest(pending)==record['file_sha256']
assert digest(ROOT/record['source'])==record['source_sha256']
shutil.copyfile(pending,OUT/record['file'])
metadata['records']=[record if r['file']==record['file'] else r for r in metadata['records']]
(OUT/'metadata.json').write_text(json.dumps(metadata,indent=2)+'\n',encoding='utf-8')
print('FACTORY_PORTRAIT_V10_SOURCE_ACTIVATION_PASS preserved21')
