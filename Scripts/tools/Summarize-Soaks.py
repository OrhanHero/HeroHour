"""Archive completed real-RHI soaks of the current package, never an older passing run."""
from pathlib import Path
import datetime as dt
import hashlib
import json
import argparse

ROOT = Path(__file__).resolve().parents[2]
build = json.loads((ROOT / 'Builds/Windows/Build-Info.json').read_text(encoding='utf-8-sig'))
created = dt.datetime.fromisoformat(build['CreatedUtc'].replace('Z', '+00:00'))
source = ROOT / 'Builds/Windows/HeroHour/Saved/Research'
parser = argparse.ArgumentParser()
parser.add_argument('--output-dir', type=Path, default=ROOT / 'docs/verification/2026-10-15')
output = parser.parse_args().output_dir
pending = []
for faction in ('Aegis', 'Ember'):
    reports = []
    for path in source.glob(f'Soak-{faction}-*.json'):
        data = json.loads(path.read_text(encoding='utf-8-sig'))
        started = dt.datetime.fromisoformat(data['started_utc'].replace('Z', '+00:00'))
        if started >= created:
            reports.append((started, path, data))
    assert reports, f'No {faction} soak for the current package'
    _, path, data = max(reports, key=lambda item: item[0])
    assert data['complete'] and data['real_rhi'] and data['result'] == 'PASS', f'{faction}: latest soak incomplete or failed'
    assert data['elapsed_wall_seconds'] >= 1200 and data['completed_rounds'] >= 2, f'{faction}: insufficient duration/rounds'
    pending.append((faction, path, data))

# Only publish artifacts after both gates pass.
output.mkdir(parents=True, exist_ok=True)
records = []
for faction, path, data in pending:
    target = output / f'Soak-{faction}.json'
    raw = path.read_bytes()
    target.write_bytes(raw)
    records.append(dict(faction=faction, source=path.name, artifact=target.name,
                        sha256=hashlib.sha256(raw).hexdigest(),
                        elapsed_wall_seconds=data['elapsed_wall_seconds'],
                        rounds=data['completed_rounds'], mean_fps=data['mean_fps_from_wall_frames'],
                        p95_frame_ms=data['p95_frame_ms'], worst_frame_ms=data['worst_frame_ms'],
                        final_memory_bytes=data['samples'][-1]['physical_memory_bytes'],
                        peak_memory_bytes=max(s['peak_physical_memory_bytes'] for s in data['samples'])))
exe = ROOT / 'Builds/Windows/HeroHour/Binaries/Win64/HeroHour.exe'
summary = dict(schema='HeroHour.SoakAcceptance.v1', result='PASS', package=build,
               executable_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),
               resolution=[1600, 900], frame_cap=60, concurrent_soak_instances=2,
               limits='Two concurrent capped instances; packaged acceptance workers also ran during the opening minutes. Not isolated maximum-performance or manual-play evidence.',
               reports=records)
(output / 'Soak-Acceptance.json').write_text(json.dumps(summary, indent=2)+'\n', encoding='utf-8')
print(json.dumps(summary, indent=2))
