"""Describe a real Unreal CSV frame capture; never infer a soak or 60-FPS pass."""
from pathlib import Path
import argparse,csv,json,hashlib,math,statistics

parser=argparse.ArgumentParser()
parser.add_argument('--csv',type=Path,required=True)
parser.add_argument('--output',type=Path,required=True)
parser.add_argument('--warmup-frames',type=int,default=300)
parser.add_argument('--executable-sha256',required=True)
parser.add_argument('--workload',required=True)
args=parser.parse_args()
csv.field_size_limit(10_000_000)
raw=args.csv.read_bytes()
with args.csv.open(encoding='utf-8-sig',newline='') as stream:
    reader=csv.reader(stream);header=next(reader);index=header.index('FrameTime')
    frames=[]
    for row in reader:
        try:value=float(row[index])
        except (ValueError,IndexError):continue
        if math.isfinite(value) and value>0:frames.append(value)
assert len(frames)>args.warmup_frames+600,'Insufficient real frame samples'
steady=frames[args.warmup_frames:];ordered=sorted(steady)
report=dict(schema='HeroHour.FrameProfile.v1',result='CAPTURED',csv=args.csv.name,
    csv_sha256=hashlib.sha256(raw).hexdigest(),executable_sha256=args.executable_sha256,
    captured_frames=len(frames),discarded_warmup_frames=args.warmup_frames,measured_frames=len(steady),
    measured_seconds=sum(steady)/1000,mean_frame_ms=statistics.mean(steady),
    mean_fps=1000/statistics.mean(steady),median_frame_ms=statistics.median(steady),
    p95_frame_ms=ordered[math.ceil(len(ordered)*.95)-1],worst_frame_ms=max(steady),
    frames_over_33ms=sum(v>33.333 for v in steady),workload=args.workload,
    limits='Short fixed-camera capped frame capture. No active combat, full-map stress, long soak, or visual-parity acceptance implied.')
args.output.parent.mkdir(parents=True,exist_ok=True)
args.output.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
print(json.dumps(report,indent=2))
