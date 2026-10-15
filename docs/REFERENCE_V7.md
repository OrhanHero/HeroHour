# Referenzdurchlauf V7

Dieser Durchlauf korrigiert drei konkrete Abweichungen von den gelieferten Konzepten: die runde Aegis-Kommandofestung, das pickupähnliche Kampffahrzeug und das zu weitläufige Produktionsmenü. Das vollständige Konzeptspiel bleibt in Entwicklung.

## Verbindliche Bildmerkmale

Referenz für die Oberfläche ist `Art/Concepts/CityHero/Codex-Bild 1. Sept. 2026, 01_22_00.png`: schmale Ressourcenleiste oben links, kompakte Armee oben mittig, Commander-Fähigkeiten oben rechts, Karte unten links, Auswahl unten mittig und fünf Produktionsspalten unten rechts. Es werden ausschließlich vorhandene Rollen und Fähigkeiten angezeigt.

Referenz für HQ und Fahrzeug ist `Art/Concepts/CityHero/Codex-Bild 1. Sept. 2026, 01_22_20.png`: weißblauer facettierter und gestufter Kommandobau mit Dachantenne und schrägen Panzerstützen, sandfarbener flacher Acht-Rad-Radpanzer sowie eine eigene graphitgrüne Basis und ein dunkler Radpanzer. Die neue Geometrie ist selbst erstellt. Sie enthält keine aus Tempest Rising extrahierten Modelle.

## Integration

- `Scripts/art/build_reference_hq_v7.py` erzeugt ein neues HQ mit facettiertem Turm, gestuftem achteckigem Dach, kleinem Eingang, schrägen Panzerstützen, niedrigem Seitenflügel und konkaver Parabolantenne. Sein Import aktualisiert ausschließlich die Aegis-HQ-Meshreferenz. Der zunächst erstellte hausförmige Entwurf wurde beim direkten Vergleich verworfen und vor dem finalen Paket ersetzt.
- `Scripts/art/build_reference_apc_v7.py` erzeugt einen neuen Acht-Rad-Panzer mit ausgeschnittenen Radkästen, Keilfront, Turm und Heckrampe. Sein Import aktualisiert ausschließlich die Aegis-Combat-Meshreferenz. Werte, Navigation und Produktionskosten bleiben erhalten.
- `build_reference_ember_hq_v7.py` und `build_reference_ember_apc_v7.py` erzeugen eine eigene dunkle Portalarchitektur und einen eigenen breiten Acht-Rad-Panzer. Grün folgt den sichtbaren Konzeptdetails; die Funktion von feindlichen Gesundheitsanzeigen und UI-Zuständen bleibt erhalten. Andere Rosterrollen sind noch nicht vollständig neu modelliert.
- `bake_reference_surfaces_v7.py` erzeugt aus den eigenen UV0-Geometrien modellbezogene Albedo-, ORM- und Emissionsmasken: HQs mit 2048², Fahrzeuge mit 1024² Pixeln. Die lokale Materialzuordnung erhält Lack, Stahl, Gummi, Staub und Fugen. Ein gespeicherter weißer Tint vermeidet eine zweite Fraktionseinfärbung; ein Schadensblitz kehrt zum ursprünglichen Tint zurück.
- `render_reference_portraits_v7.py` rendert die vier tatsächlich verwendeten Modelle mit identischen Atlanten erneut. Neun weitere Ember-Porträts übernehmen die grünen Markierungen des gesamten Laufzeitrosters. `import_reference_portraits_v7.py` aktualisiert diese dreizehn bestehenden UI-Texturen. Die anderen neun Aegis-Porträts bleiben unverändert. Das sind vier neue Modellgeometrien und neun Farbangleichungen, keine dreizehn neuen Modelle.
- Rendering und Klicksteuerung verwenden dieselben fünfspaltigen Kartenrechtecke. Zwei obere Commander-Karten rufen dieselbe geprüfte Zielbestätigung wie die Sidebar auf.
- Die Startkamera wird zur Beurteilung verwendet. Die Modelle orientieren ihren Eingang zur sichtbaren Basisfront. Zusätzlich erzeugt `Capture-HeroHour.ps1 -CloseUp -View Battle|Combat` echte nähere Kamerabilder mit normaler UI und Nebel; das ist eine gekennzeichnete Ansicht, keine veränderte Spielgeometrie.

Editor-Build, Unreal-Importe und Paketierung führt der Hauptagent seriell aus. Drei Fachagenten bearbeiten getrennte Dateien für Modell, UI und Abnahme. Die gespeicherten Importberichte und frischen Paketprüfungen stehen im zugehörigen Verifikationsordner; ältere V5-/V6-Nachweise bleiben historisch erhalten.

## Grenzen

Diese Änderungen verbessern Silhouette, Fraktionsfarbe und Bildaufteilung. Sie ersetzen keine visuelle Abnahme des gesamten Spiels. Die eigenen gebackenen Oberflächen sind ein erster Schritt; dicht belegte UVs, manuell platzierte Beschriftung und Schäden, weitere Normaldetails, bewegliche Räder/Türme/Tore, der übrige Fraktionsroster, weitere Infanteriedetails, dichteres Stadtleben und die weniger regelmäßige Kartenkomposition fehlen weiterhin. Automatische Funktionsprüfungen beweisen keine Gleichwertigkeit zur Konzeptgrafik oder zu Tempest Rising.
