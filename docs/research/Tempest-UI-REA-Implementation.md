# Tempest Rising: UI-Referenz und konkrete HeroHour-Umsetzung

Stand: 16. Oktober 2026. Auftrag: zuerst das vollständige UI-Design umsetzen. REA-Skill: `C:/Users/Hero/.agents/skills/reverse-engineer-anything/SKILL.md`. Keine HeroHour-C++-Änderung durch diesen Referenzagenten.

## Belastbarer Befund

Die tatsächlich gespeicherte Tempest-Spielaufnahme zeigt eine **rechte, hohe Produktionssidebar mit drei Porträtspalten und Radar darüber**. Sie zeigt nicht die fünfspaltige Produktionsbox unten rechts aus den HeroHour-Konzeptbildern. Der jüngste ausdrückliche Nutzerwunsch nach Tempests UI ist deshalb eine echte Layoutänderung und nicht durch Umfärben des bisherigen HUDs erfüllt.

Die Aufnahme `Saved/Research/TempestRising/reference-live-base.png` wurde vollständig visuell geprüft. Sie ist trotz Dateiendung tatsächlich JPEG, 2560×1440; die erste Headerprüfung als PNG wurde verworfen und korrekt durch eine Formatprüfung ersetzt. Sie stammt aus der bereits am 14.10. dokumentierten Veti-Referenzpartie, Zeit 2:08, unbegrenzte Credits. Herkunft: `docs/WORKLOG_2026-10-14.md` und `docs/research/TempestRising-Systems-Evidence.md`. Kein neuer Spielstart und kein neuer dynamischer Vergleich werden behauptet.

## Beobachtete Anordnung und nachgemessene Näherungen

Die Grenzen wurden am sichtbaren Bild geschätzt; Unsicherheit etwa 8–12 Pixel an Rahmen und Schatten. Sie sind **keine dekodierten UMG-Anker oder Originalkonstanten**. Normierte Angaben beziehen sich auf 2560×1440.

| Region | Ungefähre Bildschirmgrenze x/y/w/h | Sichtbarer Inhalt |
| --- | --- | --- |
| Radar samt Unterleiste | 2170 / 25 / 360 / 395 | Kantiger Metallrahmen, grüner vertikaler Energiebalken links, eingeschränktes Radar, darunter drei Werkzeug-/Statusfelder |
| Produktion | 2210 / 440 / 325 / 690 | Bauen/Doktrinen oben; fünf vertikale Kategorien darunter; kleine Queuezeile; drei Spalten großer Porträts, vier sichtbare Reihen |
| Ressourcen | ca. 1700 / 37 / 450 / 30 | Mehrere kleine farbige Symbole mit Zahlen; keine große Resourcebox links oben |
| Zeit | ca. 1150 / 0 / 260 / 72 | Mittiger schwarzer Timer mit goldener Einfassung und kleinem Dreieck darunter |
| Kontrollgruppen | ca. 16 / 1290 / 355 / 145 | Zehn schmale hochkantige Felder mit 1–9/0, dekorativer unterer Rahmen |
| Linke Werkzeugleiste | ca. 0 / 510 / 75 / 310 | Fünf gestapelte schmale Symbolflächen; konkrete Aktionen aus diesem Bild nicht sicher identifiziert |

Normiert: Radar rechts bei x≈0,848, Breite≈0,141, oben y≈0,017; Produktion rechts bei x≈0,863, oben y≈0,306, Breite≈0,127, Höhe≈0,479. Diese schmale Sidebar ist wesentlich für die erhaltene freie Spielfläche.

Die sichtbaren Produktionskategorien heißen **Gebäude, Defensiv, Infanterie, Fahrzeuge, Luftwaffe**. Die Kategorien stehen senkrecht. Die erste ist goldfarben aktiv, übrige grau/dunkel. Oberhalb ist **Bauen** aktiv und **Doktrinen** inaktiv. Die erste Baudarstellung trägt `F1`. Eine kleine `1` steht in der Queuezeile; aus diesem einzelnen Bild folgt weder ihre Bedeutung als Fabrikanzahl noch eine geprüfte Queuebedienung. Die leeren Felder der Queuezeile sind als dunkle X-Kästchen sichtbar.

Das Radar zeigt **EINGESCHRÄNKTER RADAR** und **NUR LOKALE ANSICHT**, mit Warnblitzen. Das ist ein ausdrücklich angezeigter Funktionszustand. Der sichtbare grüne Balken kann aus diesem Bild allein nicht zuverlässig als Current/Required-Verhältnis erklärt werden.

## Originalpalette: vorhandene statische REA-Belege

`docs/verification/2026-10-16/reference-v7/REA/ui-palette.json` enthält 35 über vollständige Propertyframes geprüfte Linear-RGBA-Werte, darunter sieben Themezustände je GDF, Dynasty und VETI. Herkunft und Metadaten-Digests stehen in `Tempest-REA-V7.md`.

Für GDF sind Default/Hovered/Pressed/Selected/Active identisch `(0.099899, 0.376262, 0.955974, 1)`, Disabled/Empty identisch `(0.955974, 0.955974, 0.955974, 1)`. **Daraus folgen keine sieben verschiedenen sichtbaren UI-Farben.** Die Laufzeitaufnahme zeigt gold/orange Rahmen der Veti-Fraktion. Runtime-Overrides, Texturmasken, Shader und Alphaeffekte sind nicht rekonstruiert. Eine HeroHour-Palette mit Blau für Aegis und Grün für Ember bleibt unsere Projektanpassung.

Neu ausgeführt: `Scripts/research/inspect_tempest_ui_reference.py`. Marker `TEMPEST_UI_REFERENCE_PASS 2560x1440; 3 UI files; 18 bounded searches`. Ergebnis `docs/verification/2026-10-16/tempest-ui/reference-inventory.json` enthält Screenshotformat, Hash, vollständiges Inventar des ausgewählten Editor-UI-Ordners und begrenzte Shipping-Stringfundstellen. Dieser Ordner enthält **nur zwei MPCs und eine Materialfunktion**, keine Widget-Bäume. Die Literalnamen in der EXE belegen vorhandene Bytes, nicht deren Verwendung.

REA `binary_session` wurde geprüft: kein Ziel geöffnet. Native UI-Szenarioaufnahme ist auf diesem Windows-Host laut tatsächlichem Server mit `unsupported_host` nicht verfügbar (macOS-Tool). Es wurde deshalb keine untaugliche Runtime-Aktion wiederholt, keine neue breite Shipping-Analyse begonnen und keine Sitzung offen gelassen. Das frühere verschlüsselte Paket bleibt eine getrennte Grenze; keine Schlüsselbeschaffung oder fremde Art-Übernahme.

## Ausführbare HeroHour-Designspezifikation

Die folgenden Werte sind **unsere Implementierungsentscheidungen**, aus dem beobachteten Aufbau abgeleitet. Sie werden nicht als Original-Tempest-Parameter ausgegeben.

1. **Ein gemeinsames Layoutobjekt** für Draw und HitTest. Rechte Kante fixiert; adaptive UI-Skalierung. Radar oben rechts, Baupanel direkt darunter. Drei Porträtspalten. Kein altes fünftes unsichtbares Klickfeld; keine Weltbefehle durch transparente Teile der belegten Panels.
2. **Bauen/Doktrinen als erste Ebene.** Darunter fünf vertikale Produktionstypen. HeroHours tatsächlich unterstützte Kategorien übernehmen; falls Luftproduktion nicht existiert, letzte Kategorie mit erkennbarem Sperrgrund statt funktionslosem Symbol. Bestehende Spezialeinheiten können vorerst in einem vorhandenen Fahrzeug-/Infanterietyp bleiben. Kein erfundener Air-Queue-Backend.
3. **Karten:** großes eigenes echtes Modellporträt, leichter schwarzer Untergrund, dünne kontrastierende Kante, Hotkey oben rechts, Preis in eigener unterer Leiste. Verfügbar: helles Porträt; Hover: aufgehellter Rahmen; ausgewählt/aktiv: Fraktionskante; fehlende Credits: rote Kosten; Voraussetzung fehlt: abgedunkelt und Schloss; Produktion: sichtbare Fortschrittsmaske und Anzahl. Fertiges Gebäude: eigener Platzieren-Zustand. Kein rein dekorativer Fortschritt.
4. **Queueleiste:** die echten wartenden Aufträge bzw. echten Kategorien mit Anzahl und aktivem Fortschritt anzeigen. Abbruchbedienung und gespeicherte Erstattung aus HeroHours bestehenden Regeln beibehalten. Tooltip sagt konkret, welcher Auftrag beendet wird und was rückerstattet wird. Ein belegtes Original-Refundverhalten wird nicht behauptet.
5. **Radar:** Map-/Fog-/Markerberechnung wiederverwenden und in neue Position transformieren. Korrekte Umkehrung der Klickkoordinaten und Camera-Footprint prüfen. Strommangel verdeckt feindliche Radarinfos und zeigt ausdrücklich RADAR OFFLINE samt Blitz; keine stale Enemy-Marker. Energiebalken plus lesbare aktuelle/benötigte Werte. Farbe und Blinkwarnung aus tatsächlichem Powerstatus.
6. **Auswahlpanel unten:** die Aufnahme hat keine ausgewählte Einheit und belegt daher dessen Originalposition nicht. Für HeroHour eigenes schmales Panel unten mittig: Modellporträt, Name, HP, Rang, Cargo/Unterdrückung abhängig vom Typ, 2×3 Befehle und echte Cooldowns. Mehrfachauswahl als kleine Typ-/Anzahlporträts, Gebäudeselektion als Queue-/Rally-/Reparaturbefehle. Leere Auswahl nimmt keinen breiten Raum ein.
7. **Kontrollgruppen unten links:** zehn schmale nummerierte Slots. Gruppenbelegung, Auswahl und Fokus aus tatsächlichen Gruppen; keine erfundenen Einheitenzahlen. Damit bleibt der Platz der früheren Minimap frei und der Tempest-Eindruck entsteht.
8. **Werkzeuge links:** nur tatsächliche Aktionen (AttackMove/Stop/Hold/Repair/Sell soweit vorhanden), mit klaren Glyphen und Hovertext. Unbekannte Tempest-Symbole nicht mit geratenen Originalnamen beschriften.
9. **Typografie:** klare kondensierte Großbuchstaben für Titel, normale lesbare Schrift für Erklärungen, mindestens 12 physische Pixel bei 1280×720. Keine dünnen blaugrünen Mikroglyphen. Metallrahmen mehrstufig, dunkle nahezu opake Füllung, ein heller Innenstrich, kurze verschraubte/winklige Eckstücke; keine übergroßen weichen Neonflächen.
10. **Tooltip:** fester max. Breite mit Titel, Kosten/Zeit/Energie, Beschreibung und konkretem Sperrgrund; seitlich links von Sidebar, vollständig im Viewport. Verzögerung und Maße sind eigene UX-Entscheidungen. Commander-/Doktrinziele zeigen echte Reichweite, Cooldown, Punkte und Abbruchhinweis.

## Abnahme vor der Bezeichnung „vollständig umgesetzt“

Erforderlich sind tatsächliche Unreal-Bilder bei 1600×900 und 1280×720, je Aegis und Ember, plus UI-Klicktests mit neuer Geometrie. Zustandsbilder: Standard/Bauen, Infanterieauswahl, Fahrzeug mit Cargo, Gebäude mit aktiver Queue, Doktrinen/Cooldown, LowPower/RadarOffline, gesperrte Karte samt Tooltip und Mehrfachauswahl. Wo ein Zustand nur als deterministisches Fixture hergestellt wird, Aufnahme klar so kennzeichnen. Die Vorlage selbst bleibt historische Einzelaufnahme; das zeigt keinen beobachteten Tempest-Zustandswechsel.

Nicht durch diese Untersuchung beantwortet: Original-Fontfamilie und exakte Punktgrößen, UMG-Anker/Paddings, Original-SelectedUnit-/Tooltip-Positionen, Hover-/Pressedanimationen und Übergangsdauern, Original-Queue-Storno/Refund und Original-Commander-Punktkosten. Das vollständige HeroHour-UI kann trotzdem mit eigenen ausdrücklich geprüften Regeln gebaut werden.
