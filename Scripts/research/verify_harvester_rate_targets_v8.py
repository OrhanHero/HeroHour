"""Verify two bounded shipped-code leaf targets; never execute the target game.

Names come from the separately saved reflection/name-code investigation. This
check proves exact wrapper-to-leaf bytes and field writes, not a live harvest
rate, animation sequence, configured value or whole-object layout.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import struct
import subprocess

ROOT=Path(__file__).resolve().parents[2]
parser=argparse.ArgumentParser()
parser.add_argument('--target',type=Path,default=Path('F:/Tempest-Rising-SteamRIP.com/Tempest Rising/Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe'))
parser.add_argument('--output',type=Path,default=ROOT/'Saved/Research/TempestRising/V8/rate-target-verification.json')
args=parser.parse_args()
data=args.target.read_bytes()
expected='c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56'
assert hashlib.sha256(data).hexdigest()==expected,'Target identity changed'
assert data[:2]==b'MZ'
pe=struct.unpack_from('<I',data,0x3c)[0]
assert data[pe:pe+4]==b'PE\0\0'
count=struct.unpack_from('<H',data,pe+6)[0]
optional_size=struct.unpack_from('<H',data,pe+20)[0]
optional=pe+24
assert struct.unpack_from('<H',data,optional)[0]==0x20b
base=struct.unpack_from('<Q',data,optional+24)[0]
assert base==0x140000000
sections=[]
for i in range(count):
    pos=optional+optional_size+i*40
    virtual_size,rva,size,offset=struct.unpack_from('<IIII',data,pos+8)
    sections.append((rva,size,offset))
def locate(rva,length):
    found=[offset+rva-start for start,size,offset in sections if start<=rva and rva+length<=start+size]
    assert len(found)==1,'No unique initialized section'
    return found[0]

llvm=Path('C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/llvm-objdump.exe')
records=[]
for label,caller,target,field in (
    ('HarvestRateTarget',0x4b965fe,0x51b11d0,0x2140),
    ('DepositRateTarget',0x4b9656e,0x51b11c0,0x2144)):
    call=data[locate(caller,5):locate(caller,5)+5]
    assert call[0]==0xe8 and caller+5+struct.unpack_from('<i',call,1)[0]==target
    start=locate(target,16)
    code=data[start:start+7]
    assert code==b'\x89\x91'+struct.pack('<I',field)+b'\xc3'
    # Reviewed leaf body: exactly one store and RET, then nine INT3 alignment
    # bytes. Do not infer an endpoint from the next unrelated export name.
    assert data[start+7:start+16]==b'\xcc'*9
    command=[str(llvm),'--disassemble','--x86-asm-syntax=intel',
             '--start-address='+hex(base+target),'--stop-address='+hex(base+target+7),str(args.target)]
    output=subprocess.run(command,capture_output=True,text=True,check=True).stdout
    instructions=[];decoded=bytearray()
    for line in output.splitlines():
        match=re.match(r'\s*([0-9a-f]+):\s*((?:[0-9a-f]{2}\s+)+)\s*(.*)',line)
        if match:
            decoded.extend(bytes.fromhex(match[2]));instructions.append(match[3].strip())
    assert bytes(decoded)==code,(label,output)
    records.append(dict(label=label,wrapper_call_rva=hex(caller),wrapper_call_hex=call.hex(),
        target_rva=hex(target),file_offset=start,bytes=7,code_hex=code.hex(),
        original_sha256=hashlib.sha256(code).hexdigest(),decoded_bytes_equal=True,
        instructions=instructions,write_width_bits=32,destination_offset=hex(field),
        source_register='edx',object_register='rcx',boundary='Reviewed store/RET leaf followed by 9 INT3 alignment bytes'))
report=dict(target=str(args.target),target_sha256=expected,llvm_sha256=hashlib.sha256(llvm.read_bytes()).hexdigest(),
    result='PASS',records=records,
    limits=['No target execution. No configured rates, durations, range checks or animation transitions established.',
            'Function labels inherit separately recorded name/code candidates; the byte check proves call targets and stores.',
            'Two object offsets do not prove the complete private class layout or public structure nesting.'])
args.output.parent.mkdir(parents=True,exist_ok=True)
args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print('HARVESTER_RATE_TARGETS_V8_PASS 2 leaf stores, 14 bytes; no runtime-rate claim')
