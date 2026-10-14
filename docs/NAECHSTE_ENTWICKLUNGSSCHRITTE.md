# Ablauf nach dem ersten HERO-Assetset

Ziel ist ein lesbarer, direkt bedienbarer C&C-artiger RTS-Prototyp in Unreal, dessen Präsentation den gelieferten Konzeptbildern näherkommt. Ein bestandener Headless-Test allein belegt dieses Ziel nicht. Als verbindliche UI-Referenz dient `Art/Concepts/CityHero/Codex-Bild 1. Sept. 2026, 01_22_00.png`; die dunkle grüne Basisreferenz bleibt die Form- und Materialvorlage für HERO.

## 1. UI nach Gameplay-Konzept

Schmale Ressourcenleiste oben links, Armeeübersicht oben mittig, Einsatzstatus oben rechts; Minimap unten links, kompakte Auswahlkarte unten mittig, Produktion unten rechts. Dunkle Metallrahmen, klare cyanfarbene Akzente und echte Porträts der eigenen 3D-Assets ersetzen die großen grauen Balken. Die Welt bleibt zwischen den Panels bedienbar. Minimap, Produktion, Sammelpunkte, Abbruch und Auswahl müssen mit den gleichen Koordinaten gezeichnet und angeklickt werden. Ungebaute Systeme wie Strom und Superwaffen erhalten keine erfundenen Werte oder scheinbar funktionierenden Schaltflächen.

Abnahme: sichtbare Prüfung im Windows-Spiel für mehrere Auflösungen, lesbare Preise/Status, funktionierende Klicks und keine ungewollten Weltbefehle unter den Panels.

## 2. Landschaft und Beleuchtung

Der Level ist derzeit der größte visuelle Abstand zum Konzept. Flussufer statt gerader türkisfarbener Fläche, natürliche Felsgruppen statt gleichförmiger Hügel, stärker gegliederte Straßen und Stadtblöcke, bessere Bodenmaterialien und ausgeglichene Tagesbeleuchtung. Die spielbare Fläche, Brücken und Wege müssen weiterhin mit der Navigation übereinstimmen.

Abnahme: reale Unreal-Screenshots aus Spielhöhe gegen die Karte und Gameplay-Referenz vergleichen; alle Navigationsprüfungen bestehen.

## 3. Asset-Oberflächen und Bewegung

Die vier gelieferten Meshes bilden eine erste gemeinsame Formensprache, noch keine finale Konzeptqualität. Als nächstes kommen kontrollierte Oberflächendetails, gebackene Materialkarten, Gebrauchsspuren und LODs. Danach bewegliche Räder/Türme, Torzustände und Produktions-/Treffereffekte. Silhouetten und Teamfarben müssen bei Spielkameraabstand erkennbar bleiben.

TRELLIS.2 wird nur eingesetzt, wenn ein gezielter Test bessere nutzbare Assets oder Materialien liefert. Es erzeugt laut offiziellem Projekt Bild-zu-3D und formgebundene PBR-Materialien sowie GLB-Exporte. Der Rechner besitzt eine RTX 4070 Ti SUPER mit 16 GB VRAM; README nennt Linux und mindestens 24 GB. Ubuntu/WSL2 sieht die GPU, aber CUDA-Compiler/Conda fehlen in der geprüften Shell. Der offizielle Pipelinecode enthält `low_vram=True` und CPU-Auslagerung; das ist kein Nachweis, dass eine vollständige Generierung auf dieser Karte gelingt. Deshalb wurde keine große Installation oder Modellgewichtsübertragung begonnen.

Quellen: [offizielles TRELLIS.2-Repository](https://github.com/microsoft/TRELLIS.2), [Speichermodus im Pipelinecode](https://github.com/microsoft/TRELLIS.2/blob/main/trellis2/pipelines/trellis2_image_to_3d.py), [Texturbeispiel](https://github.com/microsoft/TRELLIS.2/blob/main/example_texturing.py). Sinnvoller erster Kandidat wäre Materialverbesserung einer vorhandenen Fabrik oder ein einzeln dargestelltes Gebäude; Rückseiten, Geometrie, UVs, Kollisionsflächen und LODs brauchen anschließend Prüfung in Blender/Unreal.

## 4. Zusammenspiel und vollständige Partie

Ressourcenreservierung, klarere Rückmeldungen, Bewegung mehrerer Einheiten und Produktionsausgänge verbessern. Danach eine tatsächlich bediente Partie vom Basenbau bis Sieg/Niederlage sowie Grafik-/Performanceprüfung durchführen. Bestehende Kampf-, Wirtschafts-, Navigations- und Matchtests bleiben die technische Basis, ersetzen aber nicht die Bedienungsprüfung.

## 5. Veröffentlichung des überprüften Standes

Erst nach den jeweiligen Abnahmen Build-Nachweise, Bilder und Grenzen aktualisieren. Repository-Commits verwenden ausschließlich die bereits korrigierte OrhanHero-Identität. Der aktuelle Entwicklungsstand ist lokal; eine Veröffentlichung wird ausdrücklich als solche ausgewiesen.
