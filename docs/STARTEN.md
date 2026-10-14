# HeroHour starten

Windows-Paket vom14.10.2026,22:33 Berlin, Unreal5.8.3. Alle acht Suiten PASS; aktuelle Nachweise und Grenzen: [VERIFIED_BUILD.md](VERIFIED_BUILD.md). Neuer Account: zuerst [CURRENT_HANDOVER.md](CURRENT_HANDOVER.md).

Doppelklick auf `HeroHour-Starten.cmd` oder `Builds/Windows/HeroHour.exe`. PowerShell im Projektordner:

```powershell
.\Scripts\tools\Start-HeroHour.ps1 -Mode Packaged
.\Scripts\tools\Build-HeroHour.ps1 -Package
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Combat
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Economy
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Navigation
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Match
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite UI
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Architecture
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Resource
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Docking
.\Scripts\tools\Capture-HeroHour.ps1 -Mode Packaged -View Harvester
.\Scripts\tools\Capture-HeroHour.ps1 -Mode Packaged -View Battle
```

Capture-Ansichten: Battle, Harvester, District und Overview. Standard1600×900; `-Width 2560 -Height 1440` angeben. Das Skript erzwingt die angeforderte Größe und prüft die PNG-Abmessungen. RenderOffscreen aktiviert echte Grafik ohne Bedienung der Nutzersitzung; die Ausgabe ist ein Spielrender. Vorschauen: `Art/Previews/ConceptV3-*.png`.

Engine-Standard `F:\UE\UE_5.8`, alternativ `-EngineRoot` verwenden. Editor vor Editor-Build schließen; laufendes Spiel vor Überschreiben des Pakets schließen. Neue Quelldateien gegebenenfalls mit Build.bat `-gather` erfassen. Logs `Saved/Logs`, Manifest `Builds/Windows/Build-Info.json`. Editor-Build aktualisiert das Windows-Paket nicht.

Steuerung: WASD Kamera, Mausrad Zoom, Klick/Boxauswahl, Doppelklick Typ, F Fokus, Strg+0…9 Gruppe speichern, 0…9 abrufen, doppelt Fokus. B Fabrikbau, V Raffineriebau, C Sammler am HQ, P Striker an Fabrik. Rechtsklick Boden: Move oder Rally am ausgewählten Produzenten; Rechtsklick Gegner: Angriff. Rücktaste storniert den letzten ausgewählten Produzentenauftrag mit einmaliger Erstattung. F10 Pause, R Neustart.

Sidebar: BASIS zeigt Raffinerie/Fabrik; FAHRZEUGE zeigt Striker/Sammler. Andere Kategorien besitzen noch keine spielbaren Rollen. Bauvorschau benötigt freien Standort im Hub-Radius. Minimap bewegt die Kamera; Auswahlpanel bietet Fokus.

Optionales Nachwachsen im tatsächlichen Paket:

```powershell
& .\Builds\Windows\HeroHour\Binaries\Win64\HeroHour.exe -PrototypeRegrowingFields -windowed -ResX=1600 -ResY=900
```

Standardfelder bleiben finite. Aktive Datenbindung: Kosten, Dauer, HP, Move/Waffe, Mesh, Porträt und Feldraten aus `/Game/HERO/DataV3`. Power/Grid/Fog/Veterancy und Mehrzellenfelder folgen dem [M1–M4-Plan](TEMPEST_TRANSFORMATION_PLAN.md).

Docking: FIFO und ein exklusiver Slot je Raffinerie; eigene Entladerate40/s. Restladung bleibt bei Unterbrechung erhalten. HUD zeigt Rückfahrt/Warten/Entladen. Paket und209 Prüfungen plus3 simulierte Partien sind abgenommen; vollständiger Tempest-Umbau bleibt offen.