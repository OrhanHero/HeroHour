# Konzept-Assets im Unreal-Prototyp

Am 14. Oktober 2026 wurden vier eigene Static Meshes aus der dunklen grünen HERO-Basisreferenz erstellt: Hauptquartier, Fahrzeugfabrik, Ressourcenraffinerie und ein gepanzertes Kampffahrzeug mit vier Achsen und acht Rädern.

![Tatsächlich modelliertes Set, Blender-Render](../Art/Previews/ConceptV2/AssetSet.png)

Diese Abbildung ist ein Blender-Render derselben gelieferten Geometrie, kein Unreal-Screenshot. Die Dateien und Polygonzahlen stehen in [der Asset-Dokumentation](../Art/Models/ConceptV2/README.md).

Die importierten Unreal-Assets liegen unter `/Game/HERO/ConceptV2/Meshes`, die zehn gemeinsamen Oberflächenmaterialien unter `/Game/HERO/ConceptV2/Materials`. Der Import prüft Materialslots und Größe in Zentimetern und meldete nach dem Glättungsgruppen-Export null Fehler und null Warnungen. Die Modelle ersetzen die bisherigen Darstellungen für HQ, Fabrik, Depot und Kampfeinheit. Das Depot behält seine bestehende Ressourcenfunktion; das Raffineriemodell allein führt kein neues Wirtschaftssystem ein.

Gebäude-Bauvorschauen verwenden ebenfalls das neue Set. Mausauswahl und Rechtsklickbefehle prüfen die sichtbare Mesh-Geometrie. Zwei zusätzliche Laufzeitprüfungen decken Auswahlstrahlen auf HQ und Fabrik ab. Die Startkamera blickt auf die eigene Basis mit einer Höhe von 4500 cm; Mausrad und WASD bleiben verfügbar.

Silhouetten, Schichten, Leuchtstreifen, technische Aufbauten, Rohrleitungen und offenes Fabriktor orientieren sich an der Referenz. Finale gebackene Gebrauchsspuren, LODs, Animationen, zusätzliche Einheiten und die komplette Landschaft fehlen weiterhin. Die Basis hat nun ein zusammengehöriges 3D-Set, während der restliche CityHero-Level Prototypqualität behält.

Quellen wurden eigenständig in Blender erzeugt; keine fremden Tempest-/C&C-Meshes wurden übernommen. Alle Quellen, Exporte und Render bleiben im Projekt. Der erzeugte Windows-Build liegt lokal unter `Builds/Windows`; GitHub wurde in diesem Schritt nicht automatisch aktualisiert.
