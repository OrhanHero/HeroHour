# Referenzdurchlauf V8

V8 verbessert die vorhandene Basisumgebung und die Lesbarkeit des Konzept-HUDs. Zwei eigene dekorative Bodenmeshes ergänzen Betonplatten, Fugen, Entwässerung, Wartungsmarkierungen, Reifenspuren und Staubränder. Sie sind in der gespeicherten CityHero-Karte integriert, zusammen 25.426 Dreiecke und sechs gemeinsame Materialinstanzen. Die vorhandenen 1.298 anderen Map-Actors behalten ihre protokollierten Eigenschaften. Keine zusätzliche Kollision oder Navigationsblockade.

Die HUD-Schrift berücksichtigt eine Mindesthöhe von zwölf tatsächlichen Pixeln bei 1280×720. Beschreibende Texte werden bei Bedarf gekürzt, vollständige Namen bleiben im Tooltip. Der Granatenknopf heißt kurz „G / WURF“. Die Kamerahilfe besitzt einen dunklen Hintergrund. Bestehende Eingabebereiche bleiben erhalten.

Das Windows-Paket vom 16.10.2026, 14:03 Berlin besteht 19 Testläufe mit 775 einzelnen Prüfungen und 46 natürlichen Match-/Logistikrunden. Zusätzliche echte 1280-UI-Läufe bestehen für beide Fraktionen mit jeweils 43 Prüfungen. [Nachweise](verification/2026-10-16/reference-v8/Transformation-Checks.json) und [Paketprüfsummen](verification/2026-10-16/reference-v8/Package-Integrity.json).

![Tatsächliche Paketaufnahme](../Art/Previews/ReferenceV8-Package-Aegis-1600.png)

Die zusätzliche InterfaceFixture-Aufnahme injiziert ausdrücklich einen ausgewählten Veteranen mit Unterdrückung und öffnet den Doktrinreiter. Sie ist eine UI-Diagnoseansicht, keine natürlich entstandene Gefechtssituation. [Aufnahmenmanifest](verification/2026-10-16/reference-v8/Captures.json).

TRELLIS.2 wurde mit eigenen Bildreferenzen lokal getestet. Der erste texturierte HQ-Kandidat wurde nach tatsächlichen Vorder-/Rückansichten und Wireframes verworfen. Fehlerhafte Fassaden und die zerstörte Schüssel sind sichtbar; technische GLB-Prüfungen belegen keine Artqualität. [Diagnose](research/TrellisHQV8-Diagnosis.md). Kein Tempest-Spielasset wurde übernommen.

Die Tempest-REA belegt getrennte Ernte-/Ablageparameter und bestimmte Metadaten. Originale Animationszeiten sind damit nicht bekannt. Der nächste eigene Sammleranimationsdurchlauf verwendet unsere Zustände und autorisierten Quellen; diese Folgearbeit gehört noch nicht zu V8. Die Welt ist weiterhin zu leer und regelmäßig, ein großer Teil des Rosters zu vereinfacht. V8 erfüllt die Konzeptbilder noch nicht vollständig und enthält keine neue Langzeit-Leistungsabnahme.
