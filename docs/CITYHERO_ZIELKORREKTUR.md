# CITY HERO — Zielkorrektur anhand der Originalreferenzen

Stand: 12. Oktober 2026. Die Referenzen liegen im alten Projekt unter `E:\HeroHour\Art\Concepts\HH Art`. Dieser Ordner bleibt als Quelle erhalten; sein altes Runtime-Modul ersetzt nicht die aktuelle Wirtschaft.

## Verbindliche gestalterische Richtung

- `zh_widescreen.jpg`: lesbare RTS-Kamera, sichtbare Basisgebäude und Einheiten, dichtes unteres Kommandopanel.
- `CITYHERO_SAMPLE_0..3.png`: breite Wüstenkarte, West Heights, Downtown Core und Refinery District, zwei Brücken, äußere Straßen und markante Anlagen.
- `Codex-Bild 1. Sept. 2026, 01_22_00.png`: sandige Erdtöne, türkisfarbener Fluss, detaillierte Stadt und militärische Fahrzeuge; eingebettete dunkle Kommandopanels.
- `01_22_12.png`: räumlicher Überblick mit Stadt, Industrie, Flugfeld und getrennten Basen.
- `01_22_20.png`: differenzierte grüne HERO- und blaue Militärarchitektur, mechanische Details, glaubwürdige Größenverhältnisse.
- `HERO_CITYHERO_MAP*.png`: mehrere Wege, Stadt als spielbarer Raum, Fluss als strategisches Hindernis statt schmalem Straßenkorridor.

## Tatsächlich umgesetzt

`/Game/Maps/CityHero` nutzt eine befahrbare Fläche von 240 × 180 Metern. Ein gesperrter Fluss trennt die Ufer; zwei Brücken verbinden sie. Straßen, 112 Stadtgebäude in 28 Blöcken, 64 Wohnhäuser, Raffinerie, Stadion, Kraftwerk und Flugfeld sind als echte 3D-Geometrie vorhanden. Dynamische Gebäude und die statischen Stadt-/Anlagenflächen werden von A*-Navigation und Bauplatzprüfung berücksichtigt. Schrottsammler, Bauzeit, bezahlte Produktion und Gegnerwirtschaft bleiben aktiv.

Ein echter vorhandener ATV aus dem alten Projekt ist mit Tarn-, Waffen-, Rad- und Emissive-Texturen importiert. Überlappende LOD1-Geometrie wurde entfernt. Die fünf CitySample-Verknüpfungen des alten Projekts zeigen auf fehlende Verzeichnisse und liefern keine Stadtassets.

## Noch sichtbare Abweichungen

Die Karte trifft jetzt die grundlegende Geografie, nicht die finale Bildqualität. Der Fluss ist gerade; Ufer, Berge und Stadtmaterialien sind vereinfacht. Stadtmodelle sind parametrisch erzeugt und wiederholen sich. Militärbasen verwenden weiterhin die ersten HERO-Modelle; die differenzierte Architektur aus `01_22_20.png` ist noch nicht modelliert. Wasseranimation, Vegetationsdetails, Straßenschäden, realistische Fassadentexturen und finale Gebäudeeffekte fehlen. Flugfeld, Stadion und Kraftwerk sind Landmarken ohne Spezialfähigkeiten. HUD zeigt ausschließlich vorhandene Funktionen, keine vorgetäuschten Superwaffen oder Flugzeugproduktion.

## Nächste gestalterische Abnahme

1. Nahansicht an einer Brücke: beide Ufer, Straße, Stadt und echte Militärfahrzeuge müssen zugleich lesbar sein.
2. Gebäude aus der Fraktionsreferenz als eigene Modelle ersetzen; zuerst HQ und Fahrzeugfabrik.
3. Fassaden, Sand, Fels und Wasser mit zusammenpassenden Texturen und Größenmaßstäben ausarbeiten.
4. Unregelmäßige Stadtblöcke und Erosionsformen statt wiederholter Rasterbilder.
5. Erst nach Vergleich tatsächlicher Spiel-Screenshots mit den Originalreferenzen die Gestaltung als abgeschlossen bezeichnen.

Die automatisierten Funktionsprüfungen ersetzen diese visuelle Abnahme nicht.
