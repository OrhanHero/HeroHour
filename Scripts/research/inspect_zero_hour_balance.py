"""Read bounded BIG archive directory/INI facts; never extract game assets."""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

TARGETS = {'AmericaVehicleDozer', 'AmericaVehicleChinook', 'ChinaVehicleSupplyTruck',
           'GLAInfantryWorker', 'AmericaSupplyCenter', 'ChinaSupplyCenter',
           'GLASupplyStash', 'AmericaWarFactory', 'ChinaWarFactory', 'GLAArmsDealer'}

def inspect_archive(path):
    data = path.read_bytes()
    if len(data) < 16 or data[:4] not in (b'BIGF', b'BIG4'):
        raise ValueError(f'Unsupported BIG header: {path.name}')
    count = struct.unpack_from('>I', data, 8)[0]
    if count > 100000:
        raise ValueError('Archive inventory exceeds bounded entry limit')
    position = 16
    entries = []
    for _ in range(count):
        if position + 8 > len(data):
            raise ValueError('Truncated directory')
        offset, size = struct.unpack_from('>II', data, position)
        end = data.find(b'\0', position + 8, min(len(data), position + 4104))
        if end == -1 or offset + size > len(data):
            raise ValueError('Invalid archive entry')
        name = data[position + 8:end].decode('latin-1')
        entries.append((name, offset, size))
        position = end + 1
    records = []
    compressed = []
    for name, offset, size in entries:
        if not name.lower().endswith('.ini'):
            continue
        if size > 4 * 1024 * 1024:
            raise ValueError('INI exceeds bounded member limit')
        payload = data[offset:offset + size]
        if b'\0' in payload[:100]:
            compressed.append(name)
            continue
        text = payload.decode('latin-1')
        starts = list(re.finditer(r'^Object\s+(\S+)', text, re.MULTILINE))
        for index, start in enumerate(starts):
            object_name = start.group(1)
            if object_name not in TARGETS:
                continue
            end = starts[index + 1].start() if index + 1 < len(starts) else len(text)
            block = text[start.end():end]
            fields = {}
            for field in ('BuildCost', 'BuildTime', 'MaxBoxes', 'SupplyGatheringSpeed',
                          'SupplyWarehouseActionDelay', 'SupplyCenterActionDelay'):
                found = re.search(r'^\s*' + field + r'\s*=\s*([0-9.]+)', block, re.MULTILINE)
                if found:
                    fields[field] = float(found.group(1))
            records.append({'object': object_name, 'member': name, 'member_offset': offset,
                            'member_bytes': size, 'member_sha256': hashlib.sha256(payload).hexdigest(),
                            'fields': fields})
    return {'archive': path.name, 'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest(),
            'directory_entries': count, 'selected_objects': records,
            'unparsed_binary_ini_members': compressed,
            'limitations': ['Raw per-archive fields only; inheritance, patch precedence and runtime modifiers are not resolved.',
                            'This is an independent BIG parser, not an REA native BIG capability.']}

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--game-dir', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    report = {'archives': [inspect_archive(args.game_dir / name)
                           for name in ('INIZH.big', 'PatchINI.big') if (args.game_dir / name).is_file()]}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2), encoding='utf-8')
    for archive in report['archives']:
        print(archive['archive'], archive['directory_entries'], 'entries')
        for record in archive['selected_objects']:
            print(record['object'], record['fields'])
