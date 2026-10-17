# HeroHour starten

Stand: 17.10.2026. Unreal Engine 5.8.3, aktives Modul HeroHourPrototype. Das aktuelle Zero-Hour-HUD-Paket besteht 19 Pflichtläufe mit 917 Einzelprüfungen und 46 natürlichen Match-/Logistikrunden. Ältere Dauerläufe gelten nicht als erneute Langzeitabnahme dieses Pakets. Belege: [VERIFIED_BUILD.md](VERIFIED_BUILD.md). Neuer Account: zuerst [CURRENT_HANDOVER.md](CURRENT_HANDOVER.md), [Dokumentationsindex](README.md) und [Entwicklungsplan](PROJECT_ROADMAP.md).

HeroHour-Starten.cmd startet Aegis, HeroHour-Ember-Starten.cmd startet Ember. PowerShell im Projektordner:

```powershell
.\Scripts\tools\Start-HeroHour.ps1 -Mode Packaged -Faction Aegis
.\Scripts\tools\Start-HeroHour.ps1 -Mode Packaged -Faction Ember
```

Ziel: Tempest sammeln, an der Raffinerie entladen, Strom erzeugen, Basis und Armee ausbauen, aufklären und das gegnerische HQ zerstören. Das eigene HQ muss überleben. Der Gegner finanziert sich über dieselben Regeln und beginnt HQ-Angriffe frühestens nach 60 Sekunden.

| Eingabe | Funktion |
|---|---|
| WASD / Mausrad | Kamera bewegen / zoomen |
| Klick / Ziehrechteck / Shift | Eigene Einheiten wählen / Auswahl ergänzen |
| Doppelklick / Leertaste | Gleichen Einheitentyp / eigene Kampfwagen wählen |
| F / Strg+0…9 / 0…9 | Fokus / Gruppe speichern / abrufen; doppelter Abruf fokussiert |
| Rechtsklick Boden / sichtbarer Gegner | Bewegung / Angriff; am Produzenten Rally setzen |
| B / V / Q | Fabrik / Raffinerie platzieren; Bauvorschau um 90° drehen |
| Linksklick / Escape oder Rechtsklick | Gültige Bauvorschau bestätigen / abbrechen |
| P / C | Striker an Fabrik / Sammler am HQ bestellen; gewählter Produzent muss passen |
| Rücktaste | Letzten Auftrag des ausgewählten Produzenten mit Erstattung stornieren |
| G / T / Y | Infanteriegranate / Aegis-Rauch / Artillerie entfalten oder zusammenpacken |
| Z / X | Fraktionsdoktrin zum Zielen wählen, danach gültiges Weltziel anklicken |
| K / Delete | Gebäude ein-/ausschalten / verkaufen; HQ ist nicht verkäuflich |
| F10 oder Pause / R | Pause / neue Partie |

Die Sidebar bietet BASIS, ABWEHR, INFANTERIE, FAHRZEUGE und SPEZIAL. Elf Rollen sind spielbar: HQ, Raffinerie, Fabrik, Kraftwerk, Kaserne, Radar, Abwehr, Sammler, Striker, Infanterie und Artillerie. Aegis-Doktrinen: Reparaturimpuls/Präzisionsschlag; Ember: Flammenzone/Übertaktung. Fähigkeiten sind auch über das HUD erreichbar.

Baustellen benötigen Credits, freien Rasterplatz und Basisverbindung. HQ, Kaserne und Fabrik führen parallele bezahlte FIFOs mit höchstens fünf Aufträgen je Produzent. Blockierte Ausgänge halten fertige Aufträge fest. Unterversorgung halbiert normale Produktion/Bau, vollständiger Stromausfall stoppt sie; Kraftwerksbau stellt das Netz unabhängig wieder her. Radar und Abwehr benötigen volle Versorgung. Die Minimap wird durch ein betriebsbereites Radar aktiviert. Weltnebel verbirgt Gegner und unerlaubte Ziele.

Drei Felder besitzen 48 Zellen und wachsen standardmäßig nach. Sammler tragen höchstens 100 und entladen zeitlich mit 40/s. Manuelle Bewegung unterbricht vorübergehend die Automatik. Für endliche Felder beim direkten EXE-Start -PrototypeFiniteFields verwenden; das frühere Regrowth-Flag entfällt.

## Build, Tests und Bilder

```powershell
.\Scripts\tools\Build-HeroHour.ps1 -Package
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite Systems
.\Scripts\tools\Test-HeroHour.ps1 -Mode Packaged -Suite FullMatch -Faction Ember
.\Scripts\tools\Capture-HeroHour.ps1 -Mode Packaged -View Harvester
.\Scripts\tools\Capture-HeroHour.ps1 -Mode Packaged -View Battle -Width 2560 -Height 1440
```

Weitere Suiten: Combat, Economy, Navigation, Match, UI, Architecture, Resource, Docking, Tactical, Visibility und Presentation. Captures bieten Battle, Harvester, District und Overview. Bildgröße und Aktualität werden geprüft; Bilder unter Art/Previews ersetzen keine Klickprüfung.

Engine: F:\UE\UE_5.8, alternativ -EngineRoot. Editor vor Modul-Build schließen, Spiel vor Paketüberschreiben beenden. Build-HeroHour verwendet bereits -gather für neue C++-Dateien. Ein Editor-Build aktualisiert das Windows-Paket nicht. Logs: Saved/Logs; Manifest: Builds/Windows/Build-Info.json; Paketstart: Builds/Windows/HeroHour.exe.

Git LFS ist für Unreal-Assets, Karten und Modelle erforderlich: nach Checkout git lfs install und git lfs pull. Das ignorierte Windows-Paket lokal neu bauen. Reimport-Reihenfolge: [NAECHSTE_ENTWICKLUNGSSCHRITTE.md](NAECHSTE_ENTWICKLUNGSSCHRITTE.md).

Art und Balance bleiben Prototypqualität; die Fraktionen teilen viele eigene Meshes. Tempest-Originalparität und grafische Gleichwertigkeit sind nicht belegt.

Logistik-Stresstest: Scripts/tools/Test-HeroHour.ps1 -Mode Packaged -Suite LogisticsStress -Faction Aegis beziehungsweise Ember. Jeweils 20 natürliche Partien mit 1/60-Sekunden-Simulationsschritten, 120 Sekunden Stillstandsgrenze, maximal 600 Simulationssekunden je Partie. Der Wrapper erlaubt dafür standardmäßig 1.800 echte Sekunden; das ersetzt keinen RHI-Dauerlauf.
