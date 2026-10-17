"""Verify source-byte anchoring of the selected REA function ranges.

This reads the supplied editor DLL; it never loads or executes it. PASS is
limited to analysis bytes equalling their mapped original file bytes.
"""
from pathlib import Path
import hashlib
import json

ROOT = Path(__file__).resolve().parent
REFERENCE = Path('F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject/Binaries/Win64/UnrealEditor-Tempest.dll')
EXPECTED_SHA = '2fff65ec4f3809e8f24010d5cbfa9ada2f2834b59c997569c219d6a494f0b4fa'
ADDRESSES = ('0x1800ea440', '0x1800e5700', '0x1800e13c0', '0x1800e14e0', '0x1800e2180', '0x1800fb5d0')
records = json.loads((ROOT / 'editor-pass2-evidence.json').read_text(encoding='utf-8'))['records']
source = REFERENCE.read_bytes()
source_sha = hashlib.sha256(source).hexdigest()
if source_sha != EXPECTED_SHA:
    raise SystemExit('Reference identity changed: recorded offsets are not reusable.')
checks = []
for address in ADDRESSES:
    dossier = next(r for r in records if r['operation'] == 'analyze_function' and r['normalized_result']['procedure']['address'] == address)
    procedure = dossier['normalized_result']['procedure']
    for body_range in procedure['body']['ranges']:
        start, end = body_range['start'], body_range['end']
        length = int(end, 16) - int(start, 16) + 1
        memory_record = next(r for r in records if r['operation'] == 'read_bytes' and r['parameters']['address'] == start and r['normalized_result']['returned_bytes'] == length)
        mapping_record = next(r for r in records if r['operation'] == 'address_to_file_offset' and r['parameters']['address'] == start)
        memory = memory_record['normalized_result']
        loaded = bytes.fromhex(memory['bytes_hex'])
        offset = mapping_record['normalized_result']['file_offset']
        original = source[offset:offset + length]
        checks.append({
            'name': procedure['name'], 'entry_address': address,
            'rva': hex(int(start, 16) - 0x180000000), 'start': start, 'inclusive_end': end,
            'file_offset': offset, 'bytes': length,
            'complete': memory['complete'], 'equal': memory['complete'] and loaded == original,
            'loaded_sha256': hashlib.sha256(loaded).hexdigest(),
            'source_range_sha256': hashlib.sha256(original).hexdigest(),
            'function_evidence': dossier['evidence_id'],
            'memory_evidence': memory_record['evidence_id'], 'mapping_evidence': mapping_record['evidence_id'],
        })
report = {
    'claim': 'Initialized REA analysis bytes equal the mapped original editor DLL bytes in these exact inclusive function-body ranges',
    'does_not_claim': ['Recompiled source byte matching', 'Shipping implementation parity', 'Runtime equivalence', 'Complete gameplay reconstruction'],
    'reference_path': str(REFERENCE), 'reference_sha256': source_sha,
    'bundle_records': len(records), 'functions': len(ADDRESSES),
    'ranges': len(checks), 'total_checked_bytes': sum(c['bytes'] for c in checks),
    'checks': checks, 'passed': all(c['equal'] for c in checks),
}
(ROOT / 'editor-pass2-byte-comparison.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print(json.dumps({k: report[k] for k in ('passed', 'functions', 'ranges', 'total_checked_bytes', 'bundle_records', 'reference_sha256')}, indent=2))
if not report['passed']:
    raise SystemExit(1)
