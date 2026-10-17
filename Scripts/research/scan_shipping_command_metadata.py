"""Bounded read-only name scan in the explicitly selected Tempest Shipping PE.

Locations and names are observations; no instruction/control-flow analysis or
runtime behavior is claimed. Only exact question-related text is retained.
"""
from pathlib import Path
import hashlib
import json
import struct

ROOT = Path(__file__).resolve().parents[2] / 'Saved' / 'Research' / 'rea-next'
ROOT.mkdir(parents=True, exist_ok=True)
TARGET = Path('F:/Tempest-Rising-SteamRIP.com/Tempest Rising/Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe')
EXPECTED_SHA = 'c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56'
PATTERNS = ('QueueUnitProductionCommand', 'CancelUnitProduction', 'PayCredits', 'Refund', 'DockState', 'Docking', 'EnterExitRefinery', 'HarvesterRefinery', 'UnitQueue', 'ProductionQueue', 'RefineryDock', 'HarvesterState', 'QueueProduction', 'EnqueueUnit', 'CancelProduction', 'Harvester', 'Refinery', 'SyncCommand_Queue', 'PayCost', 'ProductionRequestHandle', 'ProductionRequest', 'RequestUnit', 'QueueUnit', 'Dequeue', 'UnitProduction', 'ProductionData', 'ProductionProgress', 'ResourceDeposit', 'EHarvester', 'EStructureHarvesterStage', 'ERefineryEventType', 'EQueueAction', 'EGameplayReplicationCommandExecutionEvent', 'EnqueueCmd', 'QueuedDefinition', 'ProductionHandle', 'ConstructionQueue', 'RefundCost')
data = TARGET.read_bytes()
digest = hashlib.sha256(data).hexdigest()
if digest != EXPECTED_SHA:
    raise SystemExit('Target identity changed; do not reuse prior results.')
pe_offset = struct.unpack_from('<I', data, 0x3c)[0]
if data[:2] != b'MZ' or data[pe_offset:pe_offset+4] != b'PE\0\0':
    raise SystemExit('Not the expected PE target.')
num_sections, optional_size = struct.unpack_from('<H', data, pe_offset + 6)[0], struct.unpack_from('<H', data, pe_offset + 20)[0]
optional = pe_offset + 24
magic = struct.unpack_from('<H', data, optional)[0]
if magic != 0x20b:
    raise SystemExit('Expected PE32+ only.')
image_base = struct.unpack_from('<Q', data, optional + 24)[0]
sections = []
table = optional + optional_size
for index in range(num_sections):
    offset = table + 40 * index
    name = data[offset:offset+8].rstrip(b'\0').decode('ascii', errors='replace')
    virtual_size, rva, raw_size, raw_start = struct.unpack_from('<IIII', data, offset + 8)
    sections.append({'name': name, 'rva': rva, 'virtual_size': virtual_size, 'raw_size': raw_size, 'raw_start': raw_start})

def location(offset):
    matches = [s for s in sections if s['raw_start'] <= offset < s['raw_start'] + s['raw_size']]
    if len(matches) != 1:
        return {'file_offset': offset, 'section': None, 'rva': None, 'preferred_va': None}
    section = matches[0]
    rva = section['rva'] + offset - section['raw_start']
    return {'file_offset': offset, 'section': section['name'], 'rva': hex(rva), 'preferred_va': hex(image_base + rva)}

matches = []
for pattern in PATTERNS:
    for encoding in ('ascii', 'utf-16-le'):
        needle = pattern.encode(encoding)
        width = 1 if encoding == 'ascii' else 2
        cursor = 0
        found = 0
        while found < 16:
            hit = data.find(needle, cursor)
            if hit < 0:
                break
            cursor = hit + len(needle)
            left, right = hit, hit + len(needle)
            def printable(at):
                return 0x20 <= data[at] <= 0x7e and (width == 1 or data[at + 1] == 0)
            while left >= width and hit - left < 160 and printable(left - width):
                left -= width
            while right + width <= len(data) and right - hit < 256 and printable(right):
                right += width
            text = data[left:right].decode(encoding, errors='replace')
            matches.append({'pattern': pattern, 'encoding': encoding, 'text': text,
                            'string_location': location(left), 'match_location': location(hit)})
            found += 1
        if found == 16:
            matches.append({'pattern': pattern, 'encoding': encoding, 'limit_reached': True})
report = {
    'method': 'Fixed literal ASCII/UTF16 scans with 16-hit per-pattern/per-encoding cap, plus PE32+ file-to-RVA mapping',
    'target': str(TARGET), 'sha256': digest, 'bytes': len(data), 'image_base': hex(image_base),
    'patterns': list(PATTERNS), 'sections': sections, 'matches': matches,
    'limitations': ['No default Ghidra analysis was launched', 'Strings are names/metadata, not executed behavior', 'No proven cross-reference or function boundary', 'RVA means mapped text location, not callable entry point', 'No native execution or package decoding'],
}
(ROOT / 'shipping-command-metadata.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print(json.dumps({'sha256': digest, 'bytes': len(data), 'matches': [{k: m[k] for k in ('pattern', 'encoding', 'text', 'string_location') if k in m} for m in matches if m['pattern'] in ('ERefineryEventType', 'EQueueAction', 'EGameplayReplicationCommandExecutionEvent', 'EnqueueCmd', 'QueuedDefinition', 'ProductionHandle', 'ConstructionQueue', 'RefundCost')]}, indent=2))
