# Reference HUD V8 — Mindestlesbarkeit

## Ausgangsbefund und Umfang

Die tatsächlich geprüfte V7-Paketaufnahme `Art/Previews/ReferenceV7-UI-1280.png` passt geometrisch auf 1280×720, aber Hilfstexte und Kartenbeschriftungen sind zu klein. Diese Änderung betrifft ausschließlich die Darstellung in `HeroHUD.cpp` und die Schriftmetrik-Hilfe in `PrototypeHUDLayout.h`. Panelpositionen, Kartenreihenfolge, Klickrechtecke, Produktions-/Commander-Kommandos und `HandleClick()` bleiben unverändert. Keine Tests wurden geändert oder abgeschwächt.

## Umsetzung

- `GetTextSize("Ag", ..., 1.0)` misst die Höhe des tatsächlich verwendeten Unreal-Canvas-Fonts. `FPrototypeHUDLayout::TextScale()` hält anschließend mindestens **12 physische Pixel gemessene Zeilenhöhe** ein. Das ist die Fontmetrik inklusive Ober-/Unterlängen, keine Behauptung einer 12-Pixel-Versalhöhe. Die Vorzugsgrößen größerer Überschriften bleiben erhalten.
- Breitenmessung, Zeilenumbruch und Zeichnung verwenden denselben effektiven Fontmaßstab. `FitText()` unterschreitet die Mindestgröße nicht mehr; zu lange beschreibende Texte erhalten gegebenenfalls `...`. Vollständige Rollennamen stehen weiterhin im Hover-Tooltip. Produktionspreise, Befehlspunktekosten und Hotkeys werden vollständig gezeichnet und nicht elidiert.
- Kartenbeschriftungen sind kürzer: unter anderem `STROM`, `INFANT.`, `BELAG.` sowie `REPAR.`/`PRÄZIS.`. Kartenporträts sind intern 54×54 Referenzpixel groß, damit zwei lesbare Textzeilen unter das Bild passen. Der Karten-/Klickbereich bleibt 74×94 Referenzpixel.
- Das redundant kleine dritte Textfeld auf den Sidebar-Commander-Karten entfällt. Name und Punktpreis bleiben; vollständiger Name, Cooldown und Zielbeschreibung stehen im Tooltip beziehungsweise im oberen Commander-Bereich.
- Die permanente untere Hilfe nennt nur Kamera, Zoom und Pause und wird anhand ihrer tatsächlichen Breite zentriert. Fähigkeiten und Doktrin-Hotkeys bleiben an ihren Schaltflächen sichtbar. Gruppensteuerung/Rechtsklick-Hilfe erscheint beim Hover über dem ausgewählten Objektporträt. Dadurch wird im Auswahlpanel Platz für Rang, Status und Unterdrückungsbalken frei.
- Kategorie-Glyphen und Text werden innerhalb derselben Tabs neu verteilt. Der Produktionshinweis und Radar-Hinweis sind kürzer. Weltbeschriftungen oberhalb der Lebensbalken berücksichtigen jetzt die tatsächlich vergrößerte Fontzeilenhöhe.

## Geplante Zeilenabstände

Die Darstellung verwendet weiterhin den bisherigen Referenzmaßstab `S=min(Breite/1672, Höhe/941)`. Geänderte relevante Positionen sind relativ zum bisherigen Panel beziehungsweise zur bisherigen Karte:

| Inhalt | Zeilentops in Referenzpixeln | Mindestabstand bei 1280×720 |
|---|---|---|
| Produktionskarte: Name / Preis | 57 / 76 | ca. 14,54 physische Pixel |
| Auswahl: zweite Statistikzeile / Rang / Status | 145 / 164 / 183 | jeweils ca. 14,54 Pixel |
| Oberer Commander: Name / Status oder Kosten | 42 / 61 | ca. 14,54 Pixel |
| Sidebar-Commander: Name / Punktpreis | 52 / 75 | ca. 17,60 Pixel |
| Umgebrochene Ereignis-/Tooltip-Zeilen | Schritt 18,5 | ca. 14,15 Pixel |

Damit wird mindestens 12 Pixel Zeilenhöhe plus ungefähr 2 Pixel Abstand eingeplant. Bei 1600/1920 wächst der verfügbare Abstand weiter. Kompakte Kartenzeilen begrenzen zusätzlich ihre Vorzugsgröße anhand der gemessenen Höhe, ohne die 12-Pixel-Untergrenze zu verletzen. Der Unterdrückungsbalken liegt nun bei Auswahl-Y203; die alte zusätzliche Hilfszeile Y199 entfällt, damit der Status nicht über den Balken oder die Aktionsschaltflächen läuft.

## Prüfstand und Grenzen

Der lokale Unreal-Quelltext `Engine/Source/Runtime/Engine/Private/HUD.cpp` wurde gelesen: `AHUD::GetTextSize()` und `DrawText()` verwenden bei `Font=nullptr` beide `GEngine->GetMediumFont()`. Messung und Zeichnung beziehen sich somit auf denselben tatsächlichen Font. `git diff --check` wurde ausgeführt; keine Builds, Unreal-Imports oder neuen Aufnahmen wurden durch diesen Agenten gestartet.

**Editor-Bildprüfung nach Integration, 16. Oktober 2026:** Der Integrationsverantwortliche meldet einen erfolgreichen Editor-Build. `Art/Previews/ReferenceV8-UI-1280.png` (Dateizeit 07:19:50 UTC / 09:19 Uhr Berlin) und `ReferenceV8-UI-1600.png` (07:20:09 UTC / 09:20 Uhr Berlin) wurden anschließend tatsächlich geöffnet. Beide zeigen merklich größere Beschriftungen; Name und Preis der fünf Karten, Ereignistext und Commander-Kosten liegen in getrennten Zeilen ohne offensichtliche Überläufe. 1280 ist gegenüber der V7-Aufnahme sichtbar besser lesbar. Die Kategorie-Glyphen bleiben klein, und die untere Kamerahilfe besitzt über hellem Boden nur mäßigen Kontrast. Diese Bilder sind Editor-Aufnahmen, keine neue Paketabnahme.

**Noch erforderlich:** vorhandene 43 UI-Prüfungen pro Fraktion mit dem finalen Build; gezielte Bilder der Spezialkategorie, ausgewählter Infanterie mit Rang/Unterdrückung sowie langer Tooltip-/Ereignistexte. Rechenabstände und die beiden Startbilder decken diese Zustände nicht ab. Eine vollständige visuelle Abnahme oder Konzeptparität wird damit nicht behauptet.

Die größeren Mindesttexte sind eine gezielte Lesbarkeitsverbesserung im vorhandenen Konzeptlayout. Sie ersetzen noch keine unabhängige UI-Skalierungsoption, hochwertigere Glyphen oder vollständige Konzepttreue.

## Nachprüfung der Infanterie-/Doktrinansicht

Die tatsächlichen Editor-Aufnahmen `Art/Previews/ReferenceV8-InterfaceFixture-Aegis-1280.png` und `ReferenceV8-InterfaceFixture-Ember-1280.png` wurden geöffnet. Rang, Unterdrückungsstatus und Balken sowie die zwei Doktrinkarten sind getrennt lesbar. Die Ansicht ist eine ausdrücklich künstlich gesetzte UI-Prüfsituation, kein Nachweis eines natürlich entstandenen Gefechtszustands. Auf beiden Bildern wurde der Granatenknopf jedoch als `G / GRANA...` gekürzt und die Kamerahilfe verschwand teilweise über hellem Terrain.

Die Granatenaktion erhält deshalb die kurze Schaltflächenbeschriftung `G / WURF`; der Hover-Titel nennt vollständig `G / GRANATENWURF`. Die untere Kamerahilfe erhält eine schmale, dunkle Fläche mit 88 Prozent Deckkraft und hellen Text. Die Fläche folgt der gemessenen Textbreite und belegt ausschließlich den bisherigen Footerbereich. Panel- und Klickrechtecke sowie die Mindestschriftgröße bleiben unverändert. Diese beiden abschließenden Quelltextkorrekturen benötigen noch einen neuen Build und eine Bildnachprüfung; die oben genannten Bilder zeigen den Befund davor.
