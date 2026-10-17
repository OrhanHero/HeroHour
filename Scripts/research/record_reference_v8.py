"""Archive fresh package checks without changing historical V7 evidence."""
import datetime as dt
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'docs/verification/2026-10-16/reference-v8'
OUT.mkdir(parents=True, exist_ok=True)
build = json.loads((ROOT/'Builds/Windows/Build-Info.json').read_text(encoding='utf-8-sig'))
since = dt.datetime.fromisoformat(build['CreatedUtc'].replace('Z', '+00:00')).timestamp()
digest = lambda p: hashlib.sha256(p.read_bytes()).hexdigest()
records = json.loads((ROOT/'Saved/Research/ReferenceV8-PackagedRuns.json').read_text(encoding='utf-8-sig'))
assert len(records) == 19
tests = []
for record in records:
    suite, faction = record['Suite'], record['Faction']
    candidates = sorted((ROOT/'Saved/Logs').glob(f'{suite}Test-{faction}-*.log'), key=lambda p:p.stat().st_mtime)
    candidates = [p for p in candidates if p.stat().st_mtime >= since]
    # The required UI suite uses the default 1600 viewport; extra 1280 checks
    # are independently archived below rather than counted twice.
    if suite == 'UI':
        candidates = [p for p in candidates if '-ResX=1600' in p.read_text(encoding='utf-8-sig', errors='replace')]
    assert candidates, (suite, faction)
    log = candidates[-1]
    text = log.read_text(encoding='utf-8-sig', errors='replace')
    marker = 'LOGISTICS_STRESS' if suite == 'LogisticsStress' else suite.upper()
    assert marker+'_TEST_RESULT=PASS' in text
    assert marker+'_TEST_RESULT=FAIL' not in text
    assert 'HeroHour.exe' in text
    checks = len(re.findall(r'LogTemp: Display: '+marker+r'_TEST .*: PASS', text))
    tests.append(dict(suite=suite, faction=faction, result='PASS', checks=checks,
                      log=log.name, log_sha256=digest(log),
                      natural_rounds=re.findall(r'HEROHOUR_MATCH_RESULT=(-?\d+)', text)))
package = {}
for rel in ['HeroHour/Binaries/Win64/HeroHour.exe'] + [f'HeroHour/Content/Paks/HeroHour-Windows.{ext}' for ext in ('pak','utoc','ucas')]:
    p = ROOT/'Builds/Windows'/rel
    package[rel] = dict(bytes=p.stat().st_size, sha256=digest(p))
report = dict(schema='HeroHour.TransformationChecks.v1', package=build,
              executable_sha256=package['HeroHour/Binaries/Win64/HeroHour.exe']['sha256'],
              required_suites=19, result='PASS', tests=tests,
              assertion_checks=sum(t['checks'] for t in tests),
              observed_match_result_markers=sum(len(t['natural_rounds']) for t in tests),
              natural_rounds=sum(len(t['natural_rounds']) for t in tests if t['suite'] in ('Match','LogisticsStress')),
              round_count_scope='Natural round count includes Match and LogisticsStress only; Combat/FullMatch fixture result markers are excluded.')
(OUT/'Transformation-Checks.json').write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
(OUT/'Package-Integrity.json').write_text(json.dumps(package, indent=2)+'\n', encoding='utf-8')
extra = []
for faction in ('Aegis','Ember'):
    matches = [p for p in (ROOT/'Saved/Logs').glob(f'UITest-{faction}-*.log')
               if p.stat().st_mtime>=since and '-ResX=1280' in p.read_text(encoding='utf-8-sig', errors='replace')]
    log = max(matches, key=lambda p:p.stat().st_mtime)
    text = log.read_text(encoding='utf-8-sig', errors='replace')
    assert 'UI_TEST_RESULT=PASS' in text
    extra.append(dict(faction=faction, viewport=[1280,720], result='PASS', log=log.name,
                      checks=len(re.findall(r'LogTemp: Display: UI_TEST .*: PASS',text)), log_sha256=digest(log)))
(OUT/'UI-1280-Checks.json').write_text(json.dumps(extra,indent=2)+'\n',encoding='utf-8')
captures = []
for name, fixture in [('ReferenceV8-Package-Aegis-1600.png',False),
                      ('ReferenceV8-Package-InterfaceFixture-Aegis-1280.png',True)]:
    p = ROOT/'Art/Previews'/name
    assert p.stat().st_mtime>=since
    raw = p.read_bytes()
    captures.append(dict(file=p.relative_to(ROOT).as_posix(), sha256=digest(p),
                         dimensions=[int.from_bytes(raw[i:i+4],'big') for i in (16,20)],
                         source='packaged game', diagnostic_fixture=fixture,
                         limitation='Fixture injects selected rank2 suppressed infantry and doctrine tab; not natural battle evidence.' if fixture else 'Normal startup camera and fog; no concept parity claim.'))
(OUT/'Captures.json').write_text(json.dumps(dict(package=build,captures=captures),indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(result='PASS', suites=len(tests), checks=report['assertion_checks'],
                      natural_rounds=report['natural_rounds'], captures=len(captures))))
