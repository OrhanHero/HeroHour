# HeroHour starten

Die Skripte verwenden standardmäßig Unreal Engine unter `F:\UE\UE_5.8`. Bei einer anderen Installation an jeden Aufruf `-EngineRoot 'D:\Pfad\UE_5.8'` anhängen. PowerShell im Projektordner öffnen.

## Geprüfter CityHero-Build vom 12. Oktober 2026

Das eigenständige Windows-Paket wurde um 23:38 Uhr (Berlin) erfolgreich erstellt. Start: `Builds\Windows\HeroHour.exe` oder Doppelklick auf `HeroHour-Starten.cmd` im Projektordner. Es enthält die aktuelle Karte `CityHero`, bereinigte texturierte ATV-Modelle und die übernommenen LocalAssets.

Im tatsächlichen Paket bestanden anschließend 15 Kampf-, 23 Wirtschafts- und 10 Navigationsprüfungen sowie drei vollständige simulierte Partien mit 3/3 Siegen, Ressourcenlieferungen und sicheren Pfaden. Alle vier Suiten meldeten PASS und Exitcode 0. Ihre Laufprotokolle enthalten keine Warnungen, Fehler oder fehlenden Spielassets. Diese Prüfungen liefen ohne Grafik; Bildqualität und echte Maus-/Tastaturbedienung werden separat im sichtbaren Paket geprüft.

Nachweise unter `Saved\Logs`:

- `Packaging-20261012-233719-924.log`: UAT `BUILD SUCCESSFUL`.
- `CombatTest-20261012-233808-738.log`: `COMBAT_TEST_RESULT=PASS`.
- `EconomyTest-20261012-233808-750.log`: `ECONOMY_TEST_RESULT=PASS`.
- `NavigationTest-20261012-233808-742.log`: `NAVIGATION_TEST_RESULT=PASS`.
- `MatchTest-20261012-233808-740.log`: `MATCH_TEST_RESULT=PASS wins=3/3`.
- `Builds\Windows\Build-Info.json`: Erstellungszeitpunkt und Paketierungslog.

## Spiel und Editor

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Start-HeroHour.ps1
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Start-HeroHour.ps1 -Mode Editor
```

Der erste Aufruf startet das Spiel sichtbar in einem separaten Fenster mit 1600 × 900 Pixeln. Der zweite öffnet das Projekt im Editor. Beide benötigen ein erfolgreich gebautes Editor-Modul und die Karte `Content\Maps\CityHero.umap`.

## Kompilieren und prüfen

Editor und Spiel schließen, bevor neu kompiliert wird.

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Build-HeroHour.ps1
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1 -Suite Economy
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1 -Suite Navigation
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1 -Suite Match
```

Der Kampftest startet Unreal ohne Grafik. Er prüft Schaden, Produktionskosten, Sieg/Niederlage und Neustart. Erfolg erfordert den Marker `COMBAT_TEST_RESULT=PASS` im neuen Log unter `Saved\Logs` sowie einen fehlerfreien Prozessabschluss. Ein solcher Test prüft weder Bildqualität noch Bedienbarkeit; diese müssen zusätzlich im sichtbaren Spiel geprüft werden.

Mit `-Suite Economy` wird die separate Wirtschaftssuite ausgeführt. Sie verlangt `ECONOMY_TEST_RESULT=PASS` und verwendet ein eigenes `EconomyTest-*.log`. Beide Suiten können mit `-Mode Packaged` dieselben Prüfungen im tatsächlich ausgelieferten Windows-Spiel ausführen.

`-Suite Navigation` verlangt `NAVIGATION_TEST_RESULT=PASS` und prüft Stadtgebäude, Wasser, Brückenrouten und alternative Wege. Die aktuellen Start-/Build-/Testskripte verwenden die Karte `CityHero`. Der oben dokumentierte Windows-Build enthält diese Karte.

`-Suite Match` startet den getrennten Partieablauftest mit `-PrototypeMatchTest` und verlangt `MATCH_TEST_RESULT=PASS`. Auch diese Suite hat im oben dokumentierten Paket bestanden.

Der Projektcache liegt unter `DerivedDataCache` im Projektordner. Die Projektkonfiguration verwendet hierfür einen lokalen Dateicache und den mitgelieferten Engine-Lesecache. Der fehlerhafte globale Cache-Verweis des Rechners wird dadurch für HeroHour umgangen, ohne ihn zu verändern. Cachedateien sind regenerierbar und werden nicht versioniert.

## Eigenständiges Windows-Paket

Nach erfolgreicher Paketierung kann `HeroHour-Starten.cmd` im Projektordner per Doppelklick gestartet werden.

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Build-HeroHour.ps1 -Package
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Economy
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Navigation
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Match
powershell -ExecutionPolicy Bypass -File .\Scripts\tools\Start-HeroHour.ps1 -Mode Packaged
```

Die Paketierung benötigt die von Unreal unterstützte Visual-Studio-C++-Toolchain und das Windows SDK. Sie baut den Game-Target, kocht die CityHero-Karte und alle HERO- und LocalAssets-Assets und schreibt nach `Builds\Windows`. Der Kochvorgang verwendet lokale Dateien statt des defekten globalen Zen-Speichers. Das erwartete Spiel ist `Builds\Windows\HeroHour.exe`. Die Existenz der Skripte bedeutet noch nicht, dass ein Paket erstellt oder getestet wurde. Ein fertiges Paket anschließend sichtbar starten und Gameplay, Materialien und Assets prüfen.

Die vollständigen Build- und UAT-Protokolle werden als `EditorBuild-*.log` und `Packaging-*.log` in `Saved\Logs` gespeichert. Der Build verlangt neben Exitcode 0 ausdrücklich den UAT-Erfolgsmarker und prüft ausführbares Spiel sowie Paketcontainer. `Build-Info.json` im Paket hält den Erstellungszeitpunkt und das zugehörige Log fest. Der anschließende Pakettest meldet fehlende HERO-Assets ebenfalls als Fehler, auch wenn die Kampfprüfungen erfolgreich waren.
