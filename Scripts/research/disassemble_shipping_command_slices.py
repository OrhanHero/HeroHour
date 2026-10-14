"""Read explicit x64 Shipping PE function ranges through installed LLVM.

Uses PE exception-directory RUNTIME_FUNCTION ranges, not nearest export names.
No target execution, import-wide analysis, package extraction or key lookup.
"""
from pathlib import Path
import hashlib
import json
import re
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
scan = json.loads((ROOT / 'shipping-command-metadata.json').read_text(encoding='utf-8'))
target = Path(scan['target'])
data = target.read_bytes()
if hashlib.sha256(data).hexdigest() != scan['sha256']:
    raise SystemExit('Target identity changed.')
image_base = int(scan['image_base'], 16)
sections = scan['sections']
LLVM = Path('C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/llvm-objdump.exe')

def file_offset(rva):
    matches = [s for s in sections if s['rva'] <= rva < s['rva'] + s['raw_size']]
    if len(matches) != 1:
        raise ValueError('No unique initialized mapping for ' + hex(rva))
    s = matches[0]
    return s['raw_start'] + rva - s['rva']

pe = struct.unpack_from('<I', data, 0x3c)[0]
exception_rva, exception_bytes = struct.unpack_from('<II', data, pe + 24 + 112 + 3 * 8)
exception_offset = file_offset(exception_rva)
runtime_functions = [struct.unpack_from('<III', data, exception_offset + i) for i in range(0, exception_bytes - 11, 12)]
children = {}
parent_of = {}
for entry in runtime_functions:
    begin, end, unwind = entry
    unwind_offset = file_offset(unwind)
    version_flags, prologue, code_count, frame = struct.unpack_from('<BBBB', data, unwind_offset)
    if (version_flags >> 3) & 4:
        chain_offset = unwind_offset + 4 + ((code_count + 1) // 2) * 4
        parent = struct.unpack_from('<III', data, chain_offset)
        children.setdefault(parent, []).append(entry)
        parent_of[entry] = parent

def chained_ranges(root):
    seen_parents = set()
    while root in parent_of:
        if root in seen_parents or len(seen_parents) >= 64:
            raise ValueError('Invalid/cyclic parent unwind chain')
        seen_parents.add(root)
        root = parent_of[root]
    visited, pending = set(), [root]
    while pending:
        item = pending.pop()
        if item in visited:
            continue
        visited.add(item)
        pending.extend(children.get(item, []))
        if len(visited) > 64:
            raise ValueError('Chained unwind range budget exceeded')
    return sorted(visited)
SEEDS = [
    ('RefundCredits_ExecCandidate', 0x4b105d0),
    ('RefundCredits_DirectCallee', 0x4f98a20),
    ('AddCredits_ExecCandidate', 0x4b0c630),
    ('AddCredits_DirectCallee', 0x4f7ebe0),
    ('GetUnitQueueIdx_ExecCandidate', 0x4b7b080),
    ('CreateCommandData_ConstructionQueue_ExecCandidate', 0x4afb490),
    ('CreateCommandData_ConstructionQueue_DirectCallee', 0x4cb9650),
    ('AreAllDepositSlotsEmpty_ExecCandidate', 0x4b4ab20),
    ('AreAllDepositSlotsEmpty_DirectCallee', 0x4ffcb90),
    ('IsDepositingResource_ExecCandidate', 0x4b4b0b0),
    ('IsInQueue_ExecCandidate', 0x4b4b210),
    ('EnterExitRefinery_bExit_BoolSetter', 0x1656f90),
    ('CreditMutation_RoleLikeGuardHelper', 0x352ef00),
]
# A manually reviewed leaf has no unwind record: two RETs and all relative
# branches remain within this exact range, followed by INT3 alignment bytes.
REVIEWED_LEAF_ENDS = {0x4ffcb90: 0x4ffcbc8, 0x1656f90: 0x1656f95, 0x352ef00: 0x352ef15}
PAYMENT_CALLERS = '--payment-callers' in sys.argv
if PAYMENT_CALLERS:
    SEEDS = [('PaymentCaller_A', 0x4f9fbd8), ('PaymentCaller_B', 0x4fa00e0), ('PaymentCaller_C', 0x4f9652f)]
version = subprocess.run([str(LLVM), '--version'], capture_output=True, text=True, check=True).stdout.splitlines()[:2]
reports = []
for label, seed in SEEDS:
    ranges = [f for f in runtime_functions if f[0] <= seed < f[1]]
    leaf = seed in REVIEWED_LEAF_ENDS and not ranges
    if leaf:
        ranges = [(seed, REVIEWED_LEAF_ENDS[seed], 0)]
    elif len(ranges) != 1:
        reports.append({'label': label, 'seed_rva': hex(seed), 'status': 'unknown', 'reason': 'No unique exception-directory range'})
        continue
    body_ranges = [ranges[0]] if leaf else chained_ranges(ranges[0])
    begin, _, unwind = body_ranges[0]
    end = max(r[1] for r in body_ranges)
    total_bytes = sum(r[1] - r[0] for r in body_ranges)
    if total_bytes > 4096:
        reports.append({'label': label, 'seed_rva': hex(seed), 'status': 'unknown', 'reason': 'Range exceeds explicit 4096-byte budget', 'range_bytes': total_bytes})
        continue
    # Nearest-export captions are not function identities in a stripped PE.
    instructions = []
    source_bytes = bytearray()
    serialized_ranges = []
    for range_begin, range_end, range_unwind in body_ranges:
        command = [str(LLVM), '--disassemble', '--x86-asm-syntax=intel', '--start-address=' + hex(image_base + range_begin), '--stop-address=' + hex(image_base + range_end), str(target)]
        output = subprocess.run(command, capture_output=True, text=True, check=True).stdout
        for line in output.splitlines():
            if re.match(r'^[0-9a-fA-F]{8,16}:', line):
                instructions.append(re.sub(r'\s*<[^>]*>', '', line))
        range_offset = file_offset(range_begin)
        source_bytes.extend(data[range_offset:range_offset + range_end - range_begin])
        serialized_ranges.append({'rva_begin': hex(range_begin), 'rva_end_exclusive': hex(range_end), 'unwind_info_rva': hex(range_unwind), 'file_offset': range_offset, 'bytes': range_end - range_begin})
    text = '\n'.join(instructions) + '\n'
    destination = ROOT / (label + '.asm.txt')
    destination.write_text(text, encoding='utf-8')
    offset = file_offset(begin)
    disassembly_bytes = bytearray()
    for line in instructions:
        found = re.match(r'^[0-9a-fA-F]+:\s*((?:[0-9a-fA-F]{2}\s+)+)', line)
        if not found:
            raise ValueError('Instruction byte column missing')
        disassembly_bytes.extend(bytes.fromhex(found.group(1)))
    calls = [{'address': '0x' + m.group(1), 'target_va': m.group(2)} for line in instructions if (m := re.match(r'^([0-9a-fA-F]+):.*\bcall\s+(0x[0-9a-fA-F]+)\s*$', line))]
    reports.append({'label': label, 'seed_rva': hex(seed), 'status': 'observed', 'seed_is_range_start': seed == begin,
                    'range_source': 'Manually reviewed bounded leaf with contained branches and terminal RETs' if leaf else 'PE exception directory RUNTIME_FUNCTION plus transitive UNW_FLAG_CHAININFO children', 'rva_begin': hex(begin), 'rva_end_exclusive': hex(end), 'unwind_info_rva': None if leaf else hex(unwind), 'ranges': serialized_ranges,
                    'file_offset': offset, 'bytes': len(source_bytes), 'source_bytes_sha256': hashlib.sha256(source_bytes).hexdigest(),
                    'disassembly_bytes_sha256': hashlib.sha256(disassembly_bytes).hexdigest(), 'disassembly_bytes_equal': source_bytes == disassembly_bytes,
                    'direct_calls': calls, 'assembly_path': destination.name, 'assembly_sha256': hashlib.sha256(text.encode()).hexdigest()})
report = {'target': str(target), 'target_sha256': scan['sha256'], 'method': 'Explicit source-bound PE exception-directory ranges and installed LLVM instruction decoder',
          'tool': str(LLVM), 'tool_version': version, 'tool_sha256': hashlib.sha256(LLVM.read_bytes()).hexdigest(), 'functions': reports,
          'limitations': ['Labels are analyst identities derived from metadata pointers/calls, not recovered original symbol names', 'Exception-directory ranges may be unwind fragments rather than whole higher-level functions', 'External/direct callee semantics are not assumed', 'No runtime or recompiled reconstruction matching'],
          'passed_byte_anchoring': all(r.get('disassembly_bytes_equal', False) for r in reports)}
(ROOT / ('shipping-payment-callers.json' if PAYMENT_CALLERS else 'shipping-command-slices.json')).write_text(json.dumps(report, indent=2), encoding='utf-8')
print(json.dumps(report, indent=2))
if not report['passed_byte_anchoring']:
    raise SystemExit(1)
print('SHIPPING_BYTE_ANCHORING_PASS stage=' + ('payment-callers' if PAYMENT_CALLERS else 'commands-docking')
      + ' slices=' + str(len(reports))
      + ' ranges=' + str(sum(len(r.get('ranges', [])) for r in reports))
      + ' bytes=' + str(sum(r.get('bytes', 0) for r in reports)))
