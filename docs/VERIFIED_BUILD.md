# Nachgewiesener Windows-Build

Aktuell:14.10.2026,22:33 Berlin, Unreal5.8.3, Win64Development, `/Game/Maps/CityHero`. CreatedUtc`2026-10-14T20:33:13.3558656Z`, `Packaging-20261014-223219-417.log`: BUILD SUCCESSFUL/Exit0. Paketcontainer, HERO-Asset-Cook und Materialprüfung bestanden. Paket liegt lokal unter `Builds/Windows`.

| Tatsächliches Paket,22:35 | Ergebnis | Log unter Saved/Logs |
| --- | --- | --- |
| Combat |17PASS|CombatTest-20261014-223508-941.log|
| Economy |80PASS|EconomyTest-20261014-223508-935.log|
| Navigation |10PASS|NavigationTest-20261014-223508-959.log|
| Match |3/3Siege, sichere Pfade, reale Lieferung|MatchTest-20261014-223508-959.log|
| UI, echte Grafik |15PASS|UITest-20261014-223508-974.log|
| Architecture |41PASS|ArchitectureTest-20261014-223508-983.log|
| Resource |18PASS|ResourceTest-20261014-223508-983.log|
| Docking |28PASS|DockingTest-20261014-223509-021.log|

Alle acht Prozesse Exit0. Testwrapper verlangt PASS und lehnt fehlende HERO-/LocalAssets sowie Materialfehler ab. UI verwendet reales Grafik-Backend mit ForceRes1600×900; andere Suiten NullRHI.

## Enthalten und tatsächlich abgenommen

Fünf originale ConceptV2-Meshes samt Porträts; DistrictV3-Gelände/Stadt/Straßen/PBR/Groundcover, warmes Wüstenmaterial und passend gefärbte taktische Karte. ConceptV3-HUD mit Metallrahmen, Glas, Kategorien, Fokus, echten Produktions-/Stornoaktionen. Acht gespeicherte DataV3-Assets sind gekocht; Rollen lesen Kosten, Dauer, HP, Bewegung, Waffenwerte, Mesh und Porträt. Feldraten kommen aus ihrer gespeicherten Definition.

Autoritativer Ressourcenfeldkern mit exklusiven Claims, optionaler Saatreserve und verzögertem Nachwachsen. Standardfinite; RegrowingFields ist opt-in. Depletion-Wechselevents und CustomPrimitiveData steuern Emission. Erschöpfter regenerierender Boden bleibt sichtbar und für Nav/Bau reserviert; finite Erschöpfung gibt die Fläche frei.

Neu: explizite Rückfahrt, FIFO-Warten, ein Dockslot und zeitliche Entladung. **Eigene Regel40/s, keine gemessene Originalrate.** Nur tatsächlicher Slotbesitzer am erreichbaren Anfahrpunkt entlädt. Manuelle Abfahrt, Tod oder Raffinerieverlust lösen Claims; Restcargo bleibt erhalten und kann zur Ersatzraffinerie fahren. Die hintere Kupplung wird zur Raffinerie gedreht; HUD zeigt RUECKFAHRT/DOCK WARTEN/ENTLADEN.

Cargo/Wallet sind double. Core nutzt monotone Transaktions-IDs und gemeinsame ULP-Schritte; tatsächlicher Debit und Credit müssen identisch und innerhalb des Budgets sein. Nicht buchbarer Rest bleibt Cargo, gibt den Dockslot frei und wird mit der nächsten Ernte zusammengeführt. Tests prüfen normale/fraktionale Bilanz, große Wallet, Subnormal/NaN/Overflow sowie manuelle Unterbrechung und zerstörte/reparierte Logistik.

Im ersten Live-Test scheiterten zwei Gates: FIFO-Kopf fuhr trotz belegtem Slot in die Entladelinie;30cm Anfahrtoleranz stoppte Sammler zu früh. Beide echten Abläufe korrigiert, alle80 Wirtschafts-Gates bestehen jetzt. Sie wurden nicht abgeschaltet. Gezielt pausiertes Entladen bleibt zusätzlich offen; bestehende UI-Suite prüft Pausenstopp der Matchzeit.

## Grafik und Grenzen

Aktueller tatsächlicher Paket-Render1600×900: [Sammler beim Entladen](../Art/Previews/ConceptV3-Harvester.png), `Capture-Harvester-20261014-223518-464.log`, CAPTURE_PASS, tatsächlich angesehen. Sichtbar860Credits/40Cargo beim laufenden Entladen. `Capture-HeroHour.ps1` prüft frischen Zeitstempel und PNG-Abmessungen, nutzt den korrekten Paket-Saved-Ordner und beendet seinen eigenen Hilfsprozess. Kopflose888×500-Ausgaben wurden verworfen und ForceRes ergänzt. Battle2560×1440 ist die frühere18:03-Ansicht; die neue Sammleransicht zeigt aktuelle Docking-Logik/Minimap.

RenderOffscreen ist reale Spielgrafik; keine manuelle20-Minuten-Partie oder Tempest-Grafikparität. M1 bleibt teilweise umgesetzt: je sichtbarem Feld derzeit eine autoritative Zelle, noch keine räumliche Mehrzellen-Ernte. Kein Live-Stromnetz, Fog of War, zwei asymmetrische Fraktionen, Veterancy oder aktive Fähigkeiten. Native Original-Vergleichsszenarien fehlen; REA-Originalbyteprüfung belegt keinen kompilierten Nachbau.

Reproduzieren: `Build-HeroHour.ps1 -Package`, danach `Test-HeroHour.ps1 -Mode Packaged -Suite` für Combat/Economy/Navigation/Match/UI/Architecture/Resource/Docking. Start: `HeroHour-Starten.cmd`. [Übergabe](CURRENT_HANDOVER.md), [Docking-Abnahme](research/Refinery-Docking-Acceptance.md).
