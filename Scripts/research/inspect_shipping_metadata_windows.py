"""Inspect bounded strings/pointer references around explicit name-scan seeds.

Adjacent text is not assigned to a class without separate structural evidence.
Pointers to .text are candidates, not proven function starts.
"""
from pathlib import Path
import hashlib
import json
import re
import struct

ROOT = Path(__file__).resolve().parent
scan = json.loads((ROOT / 'shipping-command-metadata.json').read_text(encoding='utf-8'))
data = Path(scan['target']).read_bytes()
if hashlib.sha256(data).hexdigest() != scan['sha256']:
    raise SystemExit('Target identity changed.')
image_base = int(scan['image_base'], 16)
seeds = [('HarvesterData', 135799848), ('EnterExitPayload', 137560320), ('RefundCredits', 136787056), ('UnitQueueIdx', 137621912), ('ProductionQueue', 137431168), ('ConstructionQueue', 136854448), ('CreateCommandData', 136657536), ('EnqueueCmdSingle', 134966472), ('DockEvents', 137221592), ('AddCredits', 136786304), ('AreAllDepositSlotsEmpty', 137221920), ('IsDepositingResource', 137222184), ('IsInQueue', 137222352)]
sections = scan['sections']

def map_va(value):
    rva = value - image_base
    for s in sections:
        if s['rva'] <= rva < s['rva'] + s['raw_size']:
            return {'va': hex(value), 'rva': hex(rva), 'section': s['name'], 'file_offset': s['raw_start'] + rva - s['rva']}
    return None

def offset_to_va(offset):
    for s in sections:
        if s['raw_start'] <= offset < s['raw_start'] + s['raw_size']:
            return image_base + s['rva'] + offset - s['raw_start']
    raise ValueError('Unmapped seed')

result = []
for name, center in seeds:
    start, end = max(center - 768, 0), min(center + 1024, len(data))
    strings = [{'file_offset': start + m.start(), 'text': m.group().decode('ascii')} for m in re.finditer(rb'[\x20-\x7e]{4,160}', data[start:end])]
    seed_va = offset_to_va(center)
    pointer = struct.pack('<Q', seed_va)
    cursor, refs = 0, []
    while len(refs) < 8:
        hit = data.find(pointer, cursor)
        if hit < 0:
            break
        cursor = hit + 8
        if hit % 8 != 0:
            continue
        neighbors = []
        for displacement in range(-16, 49, 8):
            pos = hit + displacement
            if pos < 0 or pos + 8 > len(data):
                continue
            value = struct.unpack_from('<Q', data, pos)[0]
            mapped = map_va(value)
            text = None
            if mapped and mapped['section'] == '.rdata':
                raw = data[mapped['file_offset']:mapped['file_offset'] + 96].split(b'\0', 1)[0]
                if len(raw) >= 3 and all(0x20 <= b <= 0x7e for b in raw):
                    text = raw.decode('ascii')
            neighbors.append({'relative_bytes': displacement, 'value': hex(value), 'mapped': mapped, 'ascii': text})
        refs.append({'file_offset': hit, 'neighbors': neighbors})
    result.append({'seed': name, 'seed_file_offset': center, 'seed_va': hex(seed_va), 'window_start': start, 'window_end_exclusive': end, 'strings': strings, 'aligned_pointer_references': refs})
report = {'target_sha256': scan['sha256'], 'method': 'Thirteen explicit 1792-byte text windows; at most eight aligned absolute pointer references per seed', 'results': result,
          'limitations': ['Adjacency does not prove field/class ownership', 'Pointer neighbors do not prove a UHT record type', 'No disassembly or native execution']}
(ROOT / 'shipping-command-windows.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
for item in result:
    print(json.dumps({'seed': item['seed'], 'strings': item['strings'], 'references': [{'file_offset': ref['file_offset'], 'next': ref['neighbors'][3]} for ref in item['aligned_pointer_references']]}, ensure_ascii=False))
