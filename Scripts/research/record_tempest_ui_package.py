"""Archive only fresh final-package evidence. Run after all tests/captures finish.

Does not build, launch tests, edit historic evidence or update handover files.
Rejects pre-rebuild Architecture failure, editor logs, incomplete run sets and
unmatched captures. A historical fixture failure is not silently relabelled PASS.
"""
import datetime as dt
import hashlib
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'docs/verification/2026-10-16/tempest-ui-package'
RUNFILE = ROOT / 'Saved/Research/TempestUI-PackagedRuns.json'
EXE = ROOT / 'Builds/Windows/HeroHour/Binaries/Win64/HeroHour.exe'
CORE = ['Combat', 'Economy', 'Navigation', 'Match', 'UI', 'Architecture',
        'Resource', 'Docking', 'Tactical', 'Visibility', 'Systems', 'FullMatch',
        'Presentation', 'LogisticsStress']
EMBER = ['FullMatch', 'Match', 'UI', 'Presentation', 'LogisticsStress']
CAPTURES = [('Radar', 'Aegis', 1600, 900), ('LowPower', 'Aegis', 1280, 720),
            ('Multi', 'Ember', 1280, 720), ('Production', 'Aegis', 1600, 900),
            ('Tooltip', 'Ember', 1600, 900), ('Pause', 'Aegis', 1600, 900),
            ('Victory', 'Ember', 1280, 720)]
UTC = dt.timezone.utc


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def timestamp(text):
    found = re.search(r'\[(\d{4}\.\d{2}\.\d{2}-\d{2}\.\d{2}\.\d{2}):(\d{3})\]', text)
    assert found, 'No Unreal UTC timestamp in log'
    return dt.datetime.strptime(found[1], '%Y.%m.%d-%H.%M.%S').replace(tzinfo=UTC).timestamp() + int(found[2]) / 1000


def command_line(text):
    found = re.search(r'^LogInit: Command Line:\s*(.+)$', text, re.M)
    assert found, 'No command line recorded'
    return found[1]


def packaged_log(path, since):
    assert path.stat().st_mtime >= since, f'Old log: {path.name}'
    text = path.read_text(encoding='utf-8-sig', errors='replace')
    assert timestamp(text) >= since, f'Log started before final build: {path.name}'
    assert re.search(r'^LogInit: ExecutableName: HeroHour\.exe\s*$', text, re.M), path.name
    base = re.search(r'^LogInit: Base Directory:\s*(.+)$', text, re.M)
    assert base, path.name
    assert Path(base[1].strip()).resolve() == EXE.parent.resolve(), (path.name, base[1])
    assert 'UnrealEditor-Cmd.exe' not in text, path.name
    assert not re.search(r'Fatal error:|Assertion failed:|Unhandled Exception:|Failed to compile Material|Cooking a material resource.*valid ShaderMap|(?:SkipPackage|Failed to find object).*\/Game\/(?:HERO|LocalAssets)\/', text), path.name
    return text, command_line(text)


def faction_matches(command, faction):
    return ('-FactionEmber' in command) == (faction == 'Ember')


def viewport_matches(command, width, height):
    return f'-ResX={width}' in command.split() and f'-ResY={height}' in command.split()


def marker_for(suite):
    return {'LogisticsStress': 'LOGISTICS_STRESS', 'FieldVisual': 'FIELD_VISUAL'}.get(suite, suite.upper())


def select_test(suite, faction, since, viewport=None):
    candidates = []
    for path in (ROOT / 'Saved/Logs').glob(f'{suite}Test-{faction}-*.log'):
        if path.stat().st_mtime < since:
            continue
        raw = path.read_text(encoding='utf-8-sig', errors='replace')
        # Do not confuse default-viewport suite with extra 1280 checks.
        if viewport and not viewport_matches(command_line(raw), *viewport):
            continue
        candidates.append(path)
    assert candidates, ('No fresh test', suite, faction, viewport)
    path = max(candidates, key=lambda candidate: candidate.stat().st_mtime)
    text, command = packaged_log(path, since)
    assert faction_matches(command, faction), path.name
    assert f'-Prototype{suite}Test' in command.split(), path.name
    marker = marker_for(suite)
    assert marker + '_TEST_RESULT=PASS' in text, path.name
    assert marker + '_TEST_RESULT=FAIL' not in text, path.name
    checks = len(re.findall(r'LogTemp: Display: ' + marker + r'_TEST .*: PASS', text))
    if suite in ('Match', 'LogisticsStress'):
        expected_rounds = 3 if suite == 'Match' else 20
        rounds = re.findall(marker + r'_TEST round=(\d+) .*result=PASS', text)
        assert [int(number) for number in rounds] == list(range(1, expected_rounds + 1)), path.name
    else:
        assert checks > 0, ('No assertion checks', path.name)
    result = dict(suite=suite, faction=faction, result='PASS', checks=checks,
                  log=path.name, log_sha256=digest(path), command_line=command,
                  log_first_utc=dt.datetime.fromtimestamp(timestamp(text), UTC).isoformat(),
                  natural_rounds=re.findall(r'HEROHOUR_MATCH_RESULT=(-?\d+)', text))
    if viewport:
        result['viewport'] = list(viewport)
    return result


def select_capture(state, faction, width, height, since):
    path = ROOT / f'Art/Previews/TempestUI-Package-{state}-{faction}-{width}.png'
    assert path.is_file() and path.stat().st_mtime >= since, ('Missing/stale image', path)
    raw = path.read_bytes()
    assert raw[:8] == b'\x89PNG\r\n\x1a\n', path
    dimensions = [int.from_bytes(raw[i:i+4], 'big') for i in (16, 20)]
    assert dimensions == [width, height], (path, dimensions)
    candidates = []
    for log in (ROOT / 'Saved/Logs').glob('Capture-InterfaceFixture-*.log'):
        if log.stat().st_mtime < since:
            continue
        text = log.read_text(encoding='utf-8-sig', errors='replace')
        command = command_line(text)
        if (f'-PrototypeUIState={state}' not in command.split() or
                not faction_matches(command, faction) or
                not viewport_matches(command, width, height)):
            continue
        # A copied preview preserves the actual source screenshot timestamp.
        shots = [line for line in text.splitlines() if 'Tracing Screenshot' in line and
                 re.search(rf'size:\s*{width}\s*x\s*{height}\b', line)]
        # Shipping-style packaged logging omits the renderer trace. The capture
        # helper independently rejects stale PNGs and verifies their dimensions;
        # associate that fresh image with its immediately preceding state marker.
        if not shots:
            shots = [line for line in text.splitlines() if
                     re.search(rf'UI_CAPTURE_FIXTURE state={state}\s', line) and 'result=PASS' in line]
        if not shots:
            continue
        shot_utc = timestamp(shots[-1])
        if abs(path.stat().st_mtime - shot_utc) > 10:
            continue
        candidates.append((log, shots[-1]))
    assert candidates, ('No matching package/state/faction/viewport capture log', path.name)
    log, screenshot_marker = max(candidates, key=lambda row: row[0].stat().st_mtime)
    text, command = packaged_log(log, since)
    assert '-PrototypeInterfaceCapture' in command.split(), log.name
    assert '-RenderOffscreen' in command.split(), log.name
    markers = [line for line in text.splitlines() if re.search(rf'UI_CAPTURE_FIXTURE state={state}\s', line)]
    assert markers and all('result=FAIL' not in line for line in markers), log.name
    assert any('unnatural_fixture=1' in line and 'result=PASS' in line for line in markers), log.name
    return dict(file=path.relative_to(ROOT).as_posix(), sha256=digest(path), bytes=len(raw),
                dimensions=dimensions, state=state, faction=faction,
                source='packaged game', diagnostic_fixture=True,
                capture_log=log.name, capture_log_sha256=digest(log), command_line=command,
                fixture_markers=markers, screenshot_marker=screenshot_marker,
                limitation='Deterministic diagnostic setup, not a naturally played match; a still does not prove interaction, timing or complete visual parity.')


def main():
    build_path = ROOT / 'Builds/Windows/Build-Info.json'
    build = json.loads(build_path.read_text(encoding='utf-8-sig'))
    since = dt.datetime.fromisoformat(build['CreatedUtc'].replace('Z', '+00:00')).timestamp()
    # Fail closed if accidentally pointed at an older package rather than the
    # final rebuild after both the obsolete Factory expectation and the exact
    # suppression-threshold runtime guard were corrected.
    assert since >= dt.datetime(2026, 10, 16, 19, 31, tzinfo=UTC).timestamp(), build
    assert build['Platform'] == 'Windows'
    package_log = Path(build['PackagingLog'])
    packaging = package_log.read_text(encoding='utf-8-sig', errors='replace')
    assert 'BUILD SUCCESSFUL' in packaging, package_log
    expected = {(suite, 'Aegis') for suite in CORE} | {(suite, 'Ember') for suite in EMBER}
    records = json.loads(RUNFILE.read_text(encoding='utf-8-sig'))
    actual = [(row['Suite'], row['Faction']) for row in records]
    assert len(actual) == 19 and len(set(actual)) == 19 and set(actual) == expected, actual
    tests = [select_test(suite, faction, since, (1600, 900) if suite == 'UI' else None)
             for suite, faction in actual]
    extra = [select_test('UI', faction, since, (1280, 720)) for faction in ('Aegis', 'Ember')]
    field = select_test('FieldVisual', 'Aegis', since)
    captures = [select_capture(*entry, since) for entry in CAPTURES]
    package = {}
    for relative in ['HeroHour/Binaries/Win64/HeroHour.exe'] + [f'HeroHour/Content/Paks/HeroHour-Windows.{ext}' for ext in ('pak', 'utoc', 'ucas')]:
        path = ROOT / 'Builds/Windows' / relative
        package[relative] = dict(bytes=path.stat().st_size, sha256=digest(path))
    checks = dict(schema='HeroHour.TransformationChecks.v2', package=build,
                  build_info_sha256=digest(build_path), packaging_log_sha256=digest(package_log),
                  runfile_sha256=digest(RUNFILE),
                  executable_sha256=package['HeroHour/Binaries/Win64/HeroHour.exe']['sha256'],
                  required_suites=19, result='PASS', tests=tests,
                  assertion_checks=sum(test['checks'] for test in tests),
                  natural_rounds=sum(len(test['natural_rounds']) for test in tests if test['suite'] in ('Match', 'LogisticsStress')),
                  observed_match_result_markers=sum(len(test['natural_rounds']) for test in tests),
                  round_count_scope='Natural rounds include Match and LogisticsStress only; Combat/FullMatch fixture markers excluded.',
                  earlier_failed_run='Earlier package passes found two failures: Architecture used an obsolete expected Factory mesh, and Tactical exposed a floating-point suppression-threshold boundary requiring a precise runtime guard fix. Both are excluded from these fresh final-package results. The final package was rebuilt after 2026-10-16T19:31:00Z and all required suites rerun; no earlier failure is relabelled PASS.',
                  scope='19 required suites; UI1280 and FieldVisual are separate extra checks; diagnostic captures do not establish naturally played states.')
    reports = {'Transformation-Checks.json': checks, 'Package-Integrity.json': package,
               'UI-1280-Checks.json': extra, 'FieldVisual-Checks.json': field,
               'Captures.json': dict(package=build, captures=captures)}
    # No PASS files are emitted until every required check and capture validates.
    OUT.mkdir(parents=True, exist_ok=True)
    for name, report in reports.items():
        pending = OUT / (name + '.pending')
        pending.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        pending.replace(OUT / name)
    print(json.dumps(dict(result='PASS', suites=len(tests), checks=checks['assertion_checks'],
                          natural_rounds=checks['natural_rounds'], extra_ui=len(extra),
                          field_visual_checks=field['checks'], captures=len(captures))))


if __name__ == '__main__':
    main()
