# Tempest-UI: tatsächliche Editor-Bildprüfung

16. Oktober 2026. Der REA-Referenzagent hat die beiden neuen HeroHour-Bilder und die historische Tempest-Aufnahme selbst vollständig visuell gelesen. Keine Sourceänderung, kein neuer REA-Sitzungsstart, keine Bildmanipulation. Bildidentität und Maße stehen in `docs/verification/2026-10-16/tempest-ui/visual-qa-images.json`.

## Geprüfte Bilder und Abdeckungsgrenze

| Aufnahme | Zustand | Abnahmeumfang |
| --- | --- | --- |
| `Art/Previews/TempestUI-Editor-Aegis-1600.png` | 1600×900, natürlicher Spielstart, HQ ausgewählt, Bauen/Gebäude, Radar noch nicht gebaut | Startlayout und Gebäudekarten; keine aktive Produktion |
| `Art/Previews/TempestUI-Editor-Ember-1280.png` | 1280×720, ausdrücklich injiziertes UI-Fixture mit Rang-2-Infanterie, Unterdrückung und Doktrinenseite | Kleine Auflösung, Infanteriepanel, Doktrinen; kein natürlich erspielter Rang-/Kampfzustand |
| `Saved/Research/TempestRising/reference-live-base.png` | 2560×1440, historische Veti-Partie 14.10., Zeit 2:08, unbegrenzte Credits, keine Einheit ausgewählt | Sichtbare Tempest-Anordnung und Rahmensprache; keine Queue-/Tooltip-/SelectedUnit-Transitionsprüfung |

## Ergebnis: Grundstruktur erreicht, vollständige UI-Abnahme noch offen

Die neue Anordnung ist deutlich näher an der tatsächlichen Tempest-Referenz: Radar oben rechts, eine hohe schmale Produktionssidebar direkt darunter, zwei Haupttabs Bauen/Doktrinen, fünf vertikale Produktionstabs und zehn Kontrollgruppenslots unten links. Aegis-Gebäudekarten stehen tatsächlich in **drei Spalten**. Die früheren fünfspaltigen unteren Produktionsfelder sind in diesen Aufnahmen nicht mehr die bestimmende Struktur.

Vertikale Kategorien sind korrekt gedreht und innerhalb ihrer eigenen Spalten sichtbar. Es gibt in den beiden geprüften Zuständen kein erkennbares Überzeichnen der Nachbarkategorie. Der ausgegraute Luftwaffe-Tab ist als gesperrter Bereich erkennbar; das kleine Sperrzeichen liest sich allerdings eher als zwei Striche als als deutliches Schloss. Die oberen Bauen/Doktrinen-Tabs sind getrennt und der aktive Zustand ist goldfarben gefüllt. Der Rahmen besteht aus mehreren dünnen Metallkanten mit schrägen Ecken; keine größeren abgeschnittenen Rahmenstücke oder unbeschrifteten alten Hitregionen können aus einem Standbild belegt werden.

Die Porträts füllen ihre vorgesehenen Felder sinnvoller als vor dem UV-Crop-Helper; sie sind nicht verzerrt und besitzen passende Transparenz. Die Aegis-Raffinerie ist hochkant, die Fabrik ist breiter; beides bleibt innerhalb des jeweiligen Porträtbereichs. Der Infanteriekörper im Ember-Auswahlpanel wird vollständig sichtbar dargestellt. Der Radarbereich meldet ausdrücklich Offline und die fehlende Voraussetzung Radarzentrum. Aus diesen Bildern folgt jedoch keine überprüfte Radar-Klickumrechnung oder Fog-/Enemy-Marker-Unterdrückung im Onlinezustand.

## Sichtbare Korrekturen vor erneuter Abnahme

1. **Preis auf Kartenrahmen:** Aegis 1600 zeigt `200`, `350`, `220`, `180` und `250` direkt auf dem unteren Metallrahmen. Bei Ember-Doktrinen liegen `50 BP`/`40 BP` ebenfalls sehr tief am Rahmen. Preise benötigen eine echte freie Zeile mit Innenabstand; Rahmen zuletzt darüber zu zeichnen darf keine Textpixel schneiden. Das ist ein sichtbarer Layoutfehler, unabhängig von funktionierenden Klicktests.
2. **Radartext durch Linie geschnitten:** In Ember 1280 läuft eine horizontale Scan-/Dekorlinie durch `RADAR OFFLINE`. Für Zustandstext einen freien Streifen aussparen oder die Scanlinien hinter einer opaken Textfläche beenden. Die Aegis-Zeile ist deutlicher, kleine Auflösung muss separat geprüft werden.
3. **Ereignis-Unterrand:** Ember 1280 bricht den Starttext auf drei Zeilen um; `produzieren.` liegt nahezu am unteren Innenrand. Panelhöhe aus Zeilenanzahl plus Innenabstand berechnen oder Text in diesem Zustand bewusster kürzen.
4. **Visuelle Gewichtung:** HeroHours Schrift und Zwischenräume wirken feiner/luftiger als die kräftige kondensierte Schrift der Referenz. Besonders der lange Doktrinen-Untertext wird bei 1280 mit Auslassung abgeschnitten. Die Auslassung ist funktional vertretbar, ein kürzerer verständlicher Titel wäre ruhiger. Eine genaue Font-Pixelhöhe oder Fontgleichheit wird aus diesen Bildern nicht behauptet.

Die Sidebar enthält bei Aegis fünf vorhandene Gebäudekarten, bei Ember zwei Doktrinenkarten. Entsprechend entsteht im unteren Bereich viel Leerraum, während Tempests Bild acht befüllte Karten zeigt. Diese Differenz darf nicht durch erfundene funktionslose Karten kaschiert werden. Eine kompaktere Aufteilung von Queue und leeren, tatsächlich verfügbaren Kartenplätzen kann später geprüft werden. Der große freie Bereich allein ist kein Interaktionsfehler.

## Offene Zustandsabnahme

Nicht in diesen zwei Aufnahmen vorhanden: echte aktive Produktionsqueue und Abbruch, bereites Gebäude/Platziermodus, Hovertooltip mit Sperrgrund, LowPower nach vorher aktivem Radar, Online-Minimap mit Camera-Footprint, Fahrzeugcargo, Mehrfachauswahl, aktiver Commander-Cooldown/Zielmodus. Diese Zustände benötigen weitere tatsächlich aufgenommene Bilder und die passenden Klicktests. Ebenso wurde durch diesen Agenten noch kein neues Windows-Paket geprüft. Deshalb ist der Stand **neue Struktur bildlich bestätigt, sichtbare Feinkorrekturen und Zustands-/Paketabnahme offen**, nicht „UI vollständig fertig“.

## Nachprüfung nach Build 4: fünf weitere tatsächliche Editor-Zustände

Die oben genannten zwei Aufnahmen bleiben historische Vor-Korrektur-Belege. Nach Root-Korrektur wurden **alle fünf** folgenden neuen Bilder selbst vollständig angesehen; ihre Hashes sind separat in `visual-qa-state-images.json` erhalten. Alle sind ausdrücklich deterministische UI-Fixtures, keine natürlich erspielten Zustände. In den zugehörigen Logs steht `UI_CAPTURE_FIXTURE ... unnatural_fixture=1 ... result=PASS`; die Screenshot-Zeilen bestätigen die tatsächlichen Auflösungen.

| Neue Aufnahme unter Art/Previews | Bildlich bestätigt | Zugehöriger Log unter Saved/Logs |
| --- | --- | --- |
| `TempestUI-Editor-Radar-Aegis-1600.png` | Radar mit dunklem unbekanntem Gelände, eigenen Markern und hellem Kameraumriss; Infanterieauswahl vollständig im Panel | `Capture-InterfaceFixture-20261016-210233-396.log` |
| `TempestUI-Editor-LowPower-Aegis-1280.png` | Leistung `60/170`, gelbe Warnfarbe, Radar offline ohne Linie durch Zustandstext; neue linke Energieleiste | `Capture-InterfaceFixture-20261016-210252-518.log` |
| `TempestUI-Editor-Multi-Ember-1280.png` | Vier Mitgliederporträts mit Einzel-HP-Balken, Einsatzgruppe /04, Gesamtzustand 370/370, Gruppenfeld 1 trägt Anzahl 4 | `Capture-InterfaceFixture-20261016-210311-517.log` |
| `TempestUI-Editor-Production-Aegis-1600.png` | Drei wartende Panzerporträts, erste Fortschrittsanzeige, Queue 3/5 sowohl Auswahl als Sidebar, aktive Abbruchfläche | `Capture-InterfaceFixture-20261016-210330-681.log` |
| `TempestUI-Editor-Tooltip-Ember-1600.png` | Raffinerietooltip vollständig links der Sidebar und im Bildschirm, Titel/200Credits/8Sekunden/Energie/Erklärung lesbar | `Capture-InterfaceFixture-20261016-210350-128.log` |

**Die drei zuvor benannten konkreten Bildfehler sind korrigiert:** Kartenpreise stehen innerhalb einer freien Zeile oberhalb des Metallrandes; der Radar-Offline-Text liegt nicht mehr auf einer Scanlinie; das höhere Ereignispanel hat ausreichenden unteren Innenabstand. Gedrehte Kategorien und Metallrahmen zeigen auch in den neuen Zuständen keine wesentlichen Überschneidungen. Die drei Spalten bleiben bei 1280 und 1600 erhalten.

Das Produktionsfixture deklariert im Log ausdrücklich `explicit_wallet_seed=1600 real_paid_orders=3`; das Bild belegt die resultierende Queueanzeige. Ein Klick auf Stornieren oder eine dadurch ausgelöste Erstattung wurde durch diese reine Bild-QA nicht ausgeführt. Das LowPower-Fixture meldet `production_rate=0.50`; eine Halbierung der tatsächlichen Produktionszeit ist aus dem Standbild allein nicht messbar.

Noch sinnvolle kleine Gestaltungskorrekturen, keine beobachteten Blocker: Der Tooltip hat bei einer einzeiligen Beschreibung etwa 85 Pixel ungenutzten unteren Raum; dynamische Inhaltshöhe würde weniger Spielwelt verdecken. Die Ereignisnachricht `Group 1 saved: 4 selected.` ist englisch, während der Großteil des HUDs deutsch ist. Der LowPower-Handlungshinweis ist bei 1280 verständlich, aber mit Auslassung abgeschnitten; `STROM ERHÖHEN` wäre eine kürzere eigene Formulierung. Die Radar-/Commander- und Produktionswerte in diesen Aufnahmen sind HeroHours Regeln, keine verifizierten Original-Tempest-Zahlen.

Zusätzlich noch offen gegenüber einer vollständigen UI-Abnahme: bereites Gebäude mit Platziermodus, gesperrte Karte samt konkretem Sperrtooltip, Cargoanzeige, aktiver Commander-Cooldown/Zielmodus, tatsächliche Klick-/Rückerstattungsprüfungen und Windows-Paketstand. Die Radar-Klickkoordinaten und Vermeidung versteckter Enemy-Informationslecks müssen über Funktionsprüfungen erfolgen; diese fünf Standbilder ersetzen sie nicht. Stand nach dieser Nachprüfung: **Layout und fünf weitere Zustandsdarstellungen visuell bestätigt; vollständige UI-/Paketabnahme weiterhin Root-Aufgabe.**

## Nachprüfung Build 5: Pause, Sieg und kompakter Tooltip

Erneut alle drei tatsächlich neuen Bilder selbst vollständig angesehen. Bild-/Log-SHAs und echte `UI_CAPTURE_FIXTURE`-/Screenshot-Marker stehen in `visual-qa-build5-images.json`. Alle drei Zustände sind deterministische Fixtures; die Aufnahmen belegen keine natürlich gewonnene Partie und keinen durch manuelle Eingabe ausgelösten Pausezustand.

| Aufnahme | Visuelles Ergebnis |
| --- | --- |
| `TempestUI-Editor-Pause-Aegis-1600.png` | Mittiges dunkles Modal mit Goldtitel `EINSATZ PAUSIERT`, klarer Erklärung und zwei getrennten Flächen `FORTSETZEN` / `NEUER EINSATZ`. Spielfeld und übriges HUD sind gleichmäßig abgedunkelt. Titel, Text, Buttons und Außenrahmen sind vollständig im Viewport; keine erkennbare Überlagerung oder abgeschnittene Zeile. Log bestätigt `paused=1 outcome=0`. |
| `TempestUI-Editor-Victory-Ember-1280.png` | Auch bei 1280 vollständig sichtbares mittiges Siegmodal `SEKTOR GESICHERT`, lesbare Erklärung und eine getrennte Neustartfläche. Kontrast ausreichend; keine Zeile läuft in einen Rahmen. Log bestätigt `paused=0 outcome=1`. |
| `TempestUI-Editor-Tooltip-Ember-1600.png` | Der Tooltip ist jetzt ungefähr 110 statt 178 Pixel hoch. Titel, Kosten/Zeit/Energie und Erklärung bleiben vollständig sichtbar, der zuvor große leere Unterbereich ist entfernt. Er liegt weiterhin links der Sidebar und überdeckt das Auswahlpanel nicht. Keine sichtbare Beschneidung oder neue Rahmenüberlagerung. |

**Keine neuen visuellen Blocker in diesen drei Zuständen gefunden.** Der Tooltip-Maßhinweis aus Build 4 ist damit erledigt. Dieselbe Tooltip-Datei wurde absichtlich neu aufgenommen; ihr alter Build-4-Hash in `visual-qa-state-images.json` ist historische Evidenz und entspricht nicht mehr den aktuellen Dateibytes. Der Build-5-Manifest enthält die aktuelle Identität. Der frühere englische Gruppenereignistext und der kurze LowPower-Hinweis wurden in diesen drei Zuständen nicht erneut geprüft.

Diese Nachprüfung beurteilt Darstellung. Ob Weiter/Neustart tatsächlich reagieren und Hintergrund-HitTests blockieren, muss die zugehörige UI-Funktionssuite bestätigen; ein Standbild beweist das nicht. Windows-Paketstand, Cargo, bereite Bauplatzierung, gesperrter Tooltip und aktiver Commander-Cooldown/Zielmodus bleiben gegenüber diesem Agentenbericht offen.

## Finale Paket-Bildprüfung: sieben Zustände am frischen Windows-Build

Paketidentität laut `Builds/Windows/Build-Info.json`: `CreatedUtc=2026-10-16T19:36:08.6233213Z`, Packaginglog `Packaging-20261016-213531-028.log`. Dieser Abschnitt ersetzt die frühere Aussage, dass ausschließlich Editorbilder geprüft seien. Die historischen Editorprüfungen bleiben als Entwicklungsschritte erhalten.

**Alle sieben neuen Paketbilder wurden vom Referenz-/QA-Agenten persönlich und vollständig angesehen**, nicht aus Dateinamen oder Editorbildern abgeleitet. Ihre individuellen SHAs und Maße sind in `docs/verification/2026-10-16/tempest-ui/visual-qa-package-images.json` gespeichert. Root führt die davon getrennte genaue Paket-/Testlog-/Capturelog-Zuordnung mit `Scripts/research/record_tempest_ui_package.py` aus; dieser Agent hat den Recorder nicht ausgeführt und keine UE-Jobs gestartet.

| Paketbild unter Art/Previews | Tatsächlich sichtbarer Zustand / Urteil |
| --- | --- |
| `TempestUI-Package-Radar-Aegis-1600.png` | Online-Radar oben rechts mit Fog, eigenen Markern und Kameraumriss; drei Kartenkolumnen, gedrehte Kategorien und Energiebalken sauber; Infanteriepanel samt Befehlen ohne sichtbares Clipping. |
| `TempestUI-Package-LowPower-Aegis-1280.png` | Leistung 60/170, gelbe Anzeige, deaktiviertes Radar mit freiem Offline-Text; Preise vollständig oberhalb Kartenunterrand, kein Rückfall des früheren Rahmenfehlers. Hinweistext noch abgekürzt, aber kein überlagerter oder abgeschnittener Rest außerhalb der Fläche. |
| `TempestUI-Package-Multi-Ember-1280.png` | Vier Einzelporträts und HP-Balken, Gesamtzustand 370/370, Gruppe 1 mit vier Objekten. Die zuvor englische Nachricht ist jetzt deutsch: `Gruppe 1 gespeichert: 4 Objekte.` Ereignispanel mit genügend Platz. |
| `TempestUI-Package-Production-Aegis-1600.png` | Fahrzeuge aktiv, drei Baubilder, drei Queueporträts, aktiver erster Fortschritt, Queue 3/5 und sichtbar aktive Stornofläche. Kein Bildüberlauf in Nachbarspalten und keine Preise auf Rahmenlinien. |
| `TempestUI-Package-Tooltip-Ember-1600.png` | Kompakter Raffinerietooltip mit vollständig sichtbarem Titel, Kosten/Zeit/Energie und Erklärung; innerhalb des Viewports links der Sidebar, ohne den früheren übergroßen Unterrand. |
| `TempestUI-Package-Pause-Aegis-1600.png` | Mittiges Pausemodal, Goldtitel, zwei lesbare getrennte Buttons; gleichmäßiger dunkler Hintergrund über Welt und HUD, keine erkennbare Text-/Rahmenüberschneidung. |
| `TempestUI-Package-Victory-Ember-1280.png` | Mittiges Siegmodal und Neustartfläche auch in kleiner Auflösung vollständig sichtbar; gut abgesetzter Titel und Erklärung, kein sichtbares Clipping. |

Für diese **sieben geprüften Paket-Zustandsdarstellungen wurden keine wesentlichen neuen visuellen Blocker gefunden**. Die großen Strukturentscheidungen entsprechen der tatsächlich beobachteten Tempest-Anordnung: Radar oben rechts, hohe rechte Produktionssidebar mit drei Spalten, Bauen/Doktrinen, vertikale Kategorien und Kontrollgruppen unten links. Fonts, eigene Modellporträts und Projektfarben sind HeroHour-Anpassungen; eine pixelgenaue Kopie des Originalspiels oder vollständig rekonstruierte Original-Widgets werden weiterhin nicht behauptet.

Alle sieben Aufnahmen sind ausdrücklich `diagnostic_fixture=true`. Sie zeigen weder natürlich erspielte Mehrfachauswahl-/Siegzustände noch gemessene Produktionsdauer oder laufende Benutzereingaben. Interaktionsfunktion und Testzahlen sind separat in den finalen Root-Paketnachweisen zu belegen. Cargo, bereites Gebäude/Platzierung, gesperrte Karte samt Sperrtooltip und aktiver Commander-Zielmodus/Cooldown wurden in dieser finalen siebenteiligen Bildserie nicht zusätzlich aufgenommen. Die Vollständigkeit des gesamten Spiels und die Konzepttreue der Spielwelt sind nicht Gegenstand dieser UI-Bildfreigabe.

### Abgleich mit den abgeschlossenen Root-Paketnachweisen

Die inzwischen vorhandenen finalen JSON-Nachweise unter `docs/verification/2026-10-16/tempest-ui-package/` wurden zusätzlich gelesen: **19 Pflichtläufe PASS, 911 Checks, 46 natürliche Match-/Stressrunden**; separat zwei UI-1280-Läufe mit je 73 Checks und FieldVisual mit 14 Checks. Diese Zahlen stammen aus Roots Recorderlauf und sind keine von diesem Bildagenten neu ausgeführten Spieltests. Match-/Stress-Prüfungen verwenden ihre tatsächlichen `result=PASS`-Roundmarker statt des bei anderen Suiten vorhandenen `: PASS`-Schemas.

Die sieben persönlich betrachteten Bild-SHAs wurden programmatisch gegen Roots `Captures.json` verglichen: `QA_PACKAGE_HASH_ALIGNMENT 7 911 46`. Alle sieben stimmen exakt überein. In den Paket-Capturelogs fehlt die Editor-Trace-Screenshotzeile; Root ordnet daher explizite Fixturezustände mit frischen PNG-Dimensionen/-Zeitstempeln und Capturehelper-Validierung innerhalb zehn Sekunden zu. Eine nicht vorhandene Screenshot-Trace wird nicht behauptet. Der Recorder wurde nach diesen Root-Korrekturen durch diesen Agenten nicht verändert.

Damit sind die oben benannten sieben UI-Paketdarstellungen und die getrennten vorliegenden Testnachweise zusammengeführt. Die ausdrücklich nicht aufgenommenen weiteren Zustände und die Grenzen gegenüber einer vollständigen Tempest-/Spielrekonstruktion bleiben bestehen.
