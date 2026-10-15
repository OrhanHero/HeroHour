"""Summarize only fresh tests of the current packaged executable. Standard library only."""
from pathlib import Path
import datetime as dt
import hashlib
import json
import re

ROOT = Path(__file__).resolve().parents[2]
build = json.loads((ROOT / 'Builds/Windows/Build-Info.json').read_text(encoding='utf-8-sig'))
since = dt.datetime.fromisoformat(build['CreatedUtc'].replace('Z', '+00:00')).timestamp()
suites = ('Combat','Economy','Navigation','Match','UI','Architecture','Resource','Docking',
          'Tactical','Visibility','Systems','FullMatch','Presentation','LogisticsStress')
records = {}
for suite in suites:
    for path in sorted((ROOT/'Saved/Logs').glob(suite+'Test-*.log'), key=lambda p: p.stat().st_mtime):
        if path.stat().st_mtime < since:
            continue
        text = path.read_text(encoding='utf-8-sig', errors='replace')
        if 'HeroHour.exe' not in text:
            continue
        marker = 'LOGISTICS_STRESS_TEST' if suite == 'LogisticsStress' else suite.upper()+'_TEST'
        matches = re.findall(marker+r'_RESULT=(PASS|FAIL)', text)
        if not matches:
            continue
        faction = 'Ember' if '-FactionEmber' in text else 'Aegis'
        count = re.search(marker+r'_COUNT=(\d+)', text)
        checks = int(count[1]) if count else len(re.findall(marker+r' [^\n]*: PASS', text))
        records[(suite, faction)] = dict(suite=suite, faction=faction, result=matches[-1],
            checks=checks, log=path.name,
            natural_rounds=re.findall(r'(?:MATCH_TEST|LOGISTICS_STRESS_TEST) round=\d+ outcome=(-?\d+)', text))
required = [(suite,'Aegis') for suite in suites] + [(suite,'Ember') for suite in ('FullMatch','Match','UI','Presentation','LogisticsStress')]
assert all(key in records and records[key]['result']=='PASS' for key in required), 'Current package checks missing or failed'
exe = ROOT/'Builds/Windows/HeroHour/Binaries/Win64/HeroHour.exe'
report = dict(schema='HeroHour.TransformationChecks.v1', package=build,
    executable_sha256=hashlib.sha256(exe.read_bytes()).hexdigest(),
    required_suites=len(required), result='PASS',
    aegis_assertion_checks=sum(records[(s,'Aegis')]['checks'] for s in suites),
    tests=[records[key] for key in required])
output=ROOT/'docs/verification/2026-10-15/Transformation-Checks.json'
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
print(json.dumps(report, indent=2))
