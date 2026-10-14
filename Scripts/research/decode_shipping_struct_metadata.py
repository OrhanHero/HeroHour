"""Decode three selected generated UHT-style struct/property tables.

The primitive type IDs are compared with the installed Unreal public header.
This yields definition metadata, not configured asset values or serialization.
"""
from pathlib import Path
import hashlib
import json
import struct

ROOT = Path(__file__).resolve().parent
scan = json.loads((ROOT / 'shipping-command-metadata.json').read_text(encoding='utf-8'))
data = Path(scan['target']).read_bytes()
if hashlib.sha256(data).hexdigest() != scan['sha256']:
    raise SystemExit('Target identity changed.')
base = int(scan['image_base'], 16)
sections = scan['sections']
HEADER = Path('F:/UE/UE_5.8/Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h')
TYPES = {3: 'Int32', 10: 'Float', 12: 'Bool', 18: 'Object', 22: 'Array', 25: 'Struct'}

def off(va):
    candidates = [s['raw_start'] + va - base - s['rva'] for s in sections if s['rva'] <= va - base < s['rva'] + s['raw_size']]
    if len(candidates) != 1:
        raise ValueError('No unique file mapping for ' + hex(va))
    return candidates[0]

def ascii_at(va):
    start = off(va)
    raw = data[start:start + 128].split(b'\0', 1)[0]
    if not raw or not all(0x20 <= b <= 0x7e for b in raw):
        raise ValueError('Invalid bounded property name')
    return raw.decode('ascii')

# These record-name-pointer locations are obtained from the prior exact name
# references, not a scan for arbitrary unowned objects.
SEEDS = [('HarvesterData', 155606760), ('GameplayCommandDataPayload_EnterExitRefinery', 155784872), ('TedConstructionQueue', 155719920)]
result = []
for expected_name, name_pointer_offset in SEEDS:
    actual_name = ascii_at(struct.unpack_from('<Q', data, name_pointer_offset)[0])
    if actual_name != expected_name:
        raise ValueError('Struct name mismatch')
    array_va = struct.unpack_from('<Q', data, name_pointer_offset + 8)[0]
    array_offset = off(array_va)
    count, size = struct.unpack_from('<HH', data, name_pointer_offset + 16)
    alignment = data[name_pointer_offset + 20]
    if not 0 < count <= 16 or alignment not in (1, 2, 4, 8, 16):
        raise ValueError('Descriptor bounds/type mismatch')
    props = []
    for index in range(count):
        prop_va = struct.unpack_from('<Q', data, array_offset + 8 * index)[0]
        prop_offset = off(prop_va)
        name = ascii_at(struct.unpack_from('<Q', data, prop_offset)[0])
        gen_flags = struct.unpack_from('<I', data, prop_offset + 24)[0]
        array_dim = struct.unpack_from('<H', data, prop_offset + 48)[0]
        offset = struct.unpack_from('<H', data, prop_offset + 50)[0]
        type_id = gen_flags & 0x3f
        props.append({'name': name, 'property_record_file_offset': prop_offset,
                      'property_record_rva': hex(prop_va - base), 'type_id': hex(type_id),
                      'type_name_from_header': TYPES.get(type_id, 'Unknown'), 'gen_flags': hex(gen_flags),
                      'array_dim': array_dim, 'field_offset_candidate': None if type_id == 12 else offset,
                      'bool_setter_candidate_va': hex(struct.unpack_from('<Q', data, prop_offset + 56)[0]) if type_id == 12 else None,
                      'definition_record_prefix_sha256': hashlib.sha256(data[prop_offset:prop_offset + 56]).hexdigest()})
    result.append({'name': actual_name, 'name_pointer_file_offset': name_pointer_offset,
                   'property_array_file_offset': array_offset, 'property_table_entries': count,
                   'declared_struct_size': size, 'declared_alignment': alignment, 'properties': props})
report = {'target_sha256': scan['sha256'], 'method': 'Source-mapped name-pointer -> bounded generated struct descriptor -> property-pointer array -> name/type/offset fields',
          'decoder_reference_header': str(HEADER), 'decoder_reference_sha256': hashlib.sha256(HEADER.read_bytes()).hexdigest(),
          'structs': result,
          'limitations': ['Generated metadata fields, not configured per-unit data-asset values', 'Array inner entries are included in the table and do not have separate outer offsets', 'Bool storage needs its setter, not the normal offset field', 'Unreal5.8 header used only to corroborate compatible type IDs/table shape, not to assert complete original engine ABI', 'No original wire or save serialization reconstructed']}
(ROOT / 'shipping-struct-metadata.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
print(json.dumps(report, indent=2))
