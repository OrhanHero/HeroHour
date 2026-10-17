"""Archive exact fresh Zero Hour / Tempest HUD package tests, without launching UE.

Reuses the strict package-log validator; does not modify historical evidence.
All 19 required suite/faction results must validate before any PASS is emitted.
"""
import datetime as dt
import json
from pathlib import Path

from record_tempest_ui_package import (ROOT, CORE, EMBER, digest, select_test)

OUT = ROOT / "docs/verification/2026-10-17/zero-hour-tempest-ui-package"


def main():
    build_path = ROOT / "Builds/Windows/Build-Info.json"
    build = json.loads(build_path.read_text(encoding="utf-8-sig"))
    since = dt.datetime.fromisoformat(build["CreatedUtc"].replace("Z", "+00:00")).timestamp()
    assert since >= dt.datetime(2026, 10, 17, 0, 9, 58, tzinfo=dt.timezone.utc).timestamp(), build
    assert build["Platform"] == "Windows"
    packaging_path = Path(build["PackagingLog"])
    assert "BUILD SUCCESSFUL" in packaging_path.read_text(encoding="utf-8-sig", errors="replace")
    expected = [(suite, "Aegis") for suite in CORE] + [(suite, "Ember") for suite in EMBER]
    assert len(expected) == len(set(expected)) == 19
    tests = [select_test(suite, faction, since, (1600, 900) if suite == "UI" else None)
             for suite, faction in expected]
    package = {}
    for relative in ["HeroHour/Binaries/Win64/HeroHour.exe"] + [
            f"HeroHour/Content/Paks/HeroHour-Windows.{ext}" for ext in ("pak", "utoc", "ucas")]:
        path = ROOT / "Builds/Windows" / relative
        package[relative] = dict(bytes=path.stat().st_size, sha256=digest(path))
    report = dict(schema="HeroHour.ZeroHourTempestUI.PackageChecks.v1", package=build,
                  result="PASS", required_suites=19, tests=tests,
                  assertion_checks=sum(row["checks"] for row in tests),
                  natural_rounds=sum(len(row["natural_rounds"]) for row in tests
                                     if row["suite"] in ("Match", "LogisticsStress")),
                  executable_sha256=package["HeroHour/Binaries/Win64/HeroHour.exe"]["sha256"],
                  build_info_sha256=digest(build_path), packaging_log_sha256=digest(packaging_path),
                  scope="19 required fresh packaged suites. UI uses 1600x900; additional 1280 runs and captures are separate evidence. PASS demonstrates functional regression checks, not perfect visual parity or whole-game completion.",
                  rejected_earlier_editor_run="UITest-Aegis-20261017-020306-783-a2afaa31.log failed six new geometry checks because the credits panel overlapped the command panel by six reference pixels. Resource Y was corrected from H-332 to H-344. Tests remained unchanged; that failed editor run is not relabelled PASS.")
    OUT.mkdir(parents=True, exist_ok=True)
    for name, data in {"Transformation-Checks.json": report, "Package-Integrity.json": package}.items():
        pending = OUT / (name + ".pending")
        pending.write_text(json.dumps(data, indent=2) + "\n", encoding="utf-8")
        pending.replace(OUT / name)
    print(json.dumps(dict(result="PASS", suites=len(tests), checks=report["assertion_checks"],
                          natural_rounds=report["natural_rounds"], output=str(OUT))))


if __name__ == "__main__":
    main()
