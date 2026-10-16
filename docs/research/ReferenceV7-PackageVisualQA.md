# Reference V7 — visuelle Paketprüfung, 16. Oktober 2026

## Nachweis und Prüfumfang

Alle sechs unten genannten Bilder wurden nach Prüfung ihrer Frische tatsächlich mit `view_image` geöffnet. Grundlage ist das Windows-Development-Paket mit `CreatedUtc=2026-10-15T22:34:24.1236961Z`, also **16. Oktober 2026, 00:34 Uhr Berlin**. Die ausführbare Datei hat SHA-256 `3e15769e38e921c7a8f12d37c5acac73d8e8b14ce356e7de620c2bc9987de7cb`.

Für jedes Bild wurden Datei-SHA gegen `docs/verification/2026-10-16/reference-v7/Captures.json`, tatsächliche PNG-Abmessungen und Änderungszeit nach Paketerstellung geprüft. Die zugehörigen Capture-Logs nennen als Base Directory `Builds/Windows/HeroHour/Binaries/Win64/`; es sind Paketaufnahmen. Alle verwenden regulären Spielnebel und HUD, keine ArtReview-Ansicht. Die Nahansichten ändern nur die Kameradistanz.

| Tatsächlich geprüftes Bild | Auflösung | Dateiänderung UTC |
|---|---|---|
| [Aegis-Startansicht](../../Art/Previews/ReferenceV7-Aegis.png) | 1600×900 | 15.10.2026 22:36:48 |
| [Ember-Startansicht](../../Art/Previews/ReferenceV7-Ember.png) | 1600×900 | 15.10.2026 22:37:02 |
| [Aegis-APC näher](../../Art/Previews/ReferenceV7-Aegis-APC-CloseUp.png) | 1600×900 | 15.10.2026 22:37:16 |
| [Ember-APC näher](../../Art/Previews/ReferenceV7-Ember-APC-CloseUp.png) | 1600×900 | 15.10.2026 22:37:30 |
| [Aegis-HQ näher](../../Art/Previews/ReferenceV7-Aegis-HQ-CloseUp.png) | 1600×900 | 15.10.2026 22:37:43 |
| [HUD bei 1280×720](../../Art/Previews/ReferenceV7-UI-1280.png) | 1280×720 | 15.10.2026 22:37:57 |

Referenz sind weiterhin die gelieferten `Codex-Bild 1. Sept. 2026, 01_22_00.png` und `01_22_20.png`. Diese Prüfung bestätigt konkrete Fortschritte und benennt verbleibende Unterschiede; sie erteilt **keine vollständige Konzeptabnahme**.

## Materialien und grünes Ensemble

In den sichtbaren Vorder-/Oberseiten gibt es keine offensichtlichen verdrehten Atlasflächen, Schachbrett-Ersatzmaterialien oder flächige Fehl-Emission. Aegis-HQ bleibt hell mit blauen Fenstern und getrennten grauen Dachflächen. Fugen, Lüftungsgitter und Stützen bleiben auch unter direktem Spiellicht erkennbar; die helle Fassade verliert ihre Gliederung nicht vollständig durch Überbelichtung.

Die APCs unterscheiden sich sichtbar in Silhouette und Farbe. Aegis besitzt den sandfarbenen Rumpf, Ember den dunklen Rumpf mit flacherem Turm. Gummi ist dunkler als Lack und Stahl. Feiner Abrieb und Staub sind bei der tatsächlichen Objektgröße jedoch stark zurückhaltend: besonders Aegis liest weiterhin überwiegend als sauberes, cremefarbenes Fahrzeug. Die gebackenen Oberflächen ersetzen noch keine gezielt platzierten Kratzer, Laufspuren, Kennnummern oder Warnmarkierungen.

Die vorherige Rot/Grün-Inkonsistenz ist in der aktuellen Ember-Startansicht behoben: HQ, Raffinerie, Sammler und die fünf sichtbaren Gebäude-Porträts tragen jetzt grüne Akzente. Die Emission sitzt auf begrenzten Leuchtleisten; der übrige Graphitrumpf leuchtet nicht. Die dunkle Fahrzeugseite im Schatten verliert allerdings etwas Detailtrennung. Auf den HQ-Panzerfeldern wirkt ein Teil der Schmutzvariation eher wolkig als durch Nutzung oder Wasserlauf begründet. Mehr Zufallsrauschen würde diese Schwäche nicht lösen.

Diese Bilder zeigen keine sichtbaren Gegner oder aktiven Treffer. Gegnerkennzeichnung, Schadensflash und Rücksetzen werden deshalb aus diesen Stillbildern nicht visuell abgenommen. Unsichtbare Rückseiten, Infanterieanimationen und andere Produktionskategorien sind ebenfalls außerhalb dieses Bildumfangs.

## HUD und Lesbarkeit

Bei 1600×900 sind Haupttitel, Auswahlname, Lebenspunkte, Ressourcen und Produktionspreise erkennbar. Alle fünf Gebäude-Karten stehen in derselben Zeile; Kurzbezeichnungen bleiben innerhalb ihrer Karten. Porträts überschreiten die Rahmen nicht. Ressourcen-, Armee- und Commander-Bereiche überdecken einander nicht. Die grünen Ember-Porträts passen jetzt zu den sichtbaren Gebäuden.

Die tatsächliche 1280×720-Aufnahme wurde zusätzlich geöffnet: Panels und fünf Karten passen weiterhin ohne sichtbare Überlagerung. Auswahlname, Credits und die großen Zustandsangaben bleiben erkennbar. Kartenbeschriftungen, Kategorie-Kurztexte, Commander-Erklärungen und die unteren Hilfstexte sind jedoch sehr fein. Die globale Tastatur-Hilfe am unteren Bildrand ist für bequemes Lesen zu klein. Daher ist 1280 **geometrisch passend, aber keine uneingeschränkte Lesbarkeitsabnahme**. Eine spätere größere Mindestschrift oder eine unabhängig einstellbare UI-Skalierung ist sinnvoller als weiteres Verkleinern langer Texte.

Die Glyphen funktionieren als Platzhalter, erreichen aber nicht die eigene Bildsprache des Konzepts. Besonders die schlichten Commander-Symbole und das große leere Radar-Offline-Feld verändern die Bildwirkung. Radar offline ist im gezeigten Startzustand absichtlich korrekt; die Konzeptansicht mit gefüllter taktischer Karte wird dadurch trotzdem nicht erreicht.

## Code- und Klickabdeckung

`PrototypeHUDLayout.h` liefert dieselben `ProductionCard()`- und `DoctrineCard()`-Rechtecke an Zeichnung und `HandleClick()`. `HeroHUD.cpp` misst Text mit dem tatsächlich verwendeten Canvas-Font und Viewport-Maßstab; vollständige Rollenbezeichnungen bleiben in Hover-Tooltips. Eine 74-Referenzpixel-Karte ist bei 1280×720 nur etwa 56,6 physische Pixel breit — Überlaufvermeidung allein beweist daher keine gute Lesbarkeit.

Die frischen Paket-UI-Tests in `Transformation-Checks.json` enthalten **43 bestandene Prüfungen pro Fraktion**. Die Quellprüfung bestätigt echte `HandleClick()`-Aufrufe für alle fünf Gebäudekarten und Produktionskategorien, Zahlung erst nach gültiger Aktion, Sammlerauftrag/Stornierung, minimapabhängige Kamerabewegung, Fokus und beide Commander-Karten an beiden UI-Positionen einschließlich Zielbestätigung und exaktem Punkteverbrauch. Layout-Überlappung wird rechnerisch für 1280×720, 1600×900 und 1920×1080 geprüft.

Die tatsächlichen automatisierten Klicktests laufen laut `Test-HeroHour.ps1` in einem 1600×900-Viewport. Die Geometrieprüfung anderer Auflösungen ist kein Ersatz für tatsächliche Klicktests bei 1280 oder 1920. Hover-Tooltips, volle Warteschlangen, lange Meldungen, Kampf-/Siegzustände und aktive Commander-Zielwahl sind in diesen sechs Stillbildern nicht dargestellt. Die gesamten 19 Paket-Suites sind separat im Verifikationsbericht dokumentiert; deren Erfolg ist keine automatische visuelle Konzeptabnahme.

## Priorisierte Restarbeit

1. Mindestlesbarkeit bei 1280 verbessern, besonders Footer und Karten-/Commander-Kleindruck; echte Klick- und Tooltip-Sitzung bei dieser Auflösung ergänzen.
2. Gezielte Oberflächendetails und Kennzeichnung auf HQ/APC hinzufügen, ohne das ganze Modell mit mehr Noise zu überziehen. Dunkle Ember-Seiten besser vom Schatten trennen.
3. Straßen-/Basisumgebung mit Randdetails, Bodenübergängen und zweckgebundenen Requisiten verdichten. Die breiten, leeren Kreuzungen und repetitiven Stadthäuser dominieren noch stark und unterscheiden sich deutlich von der belebten, kleinteiligen Konzeptwelt.
4. Eigene konsistente Glyphen und differenzierte Commander-/Produktionsgrafiken erstellen; funktionalen Zustand mit aktiver Karte und Gefecht zusätzlich visuell prüfen.

Für diese Prüfung wurden keine Quellen, Assets, Renderparameter oder Spielzustände verändert. Die einzige neue Datei ist dieser Bericht.
