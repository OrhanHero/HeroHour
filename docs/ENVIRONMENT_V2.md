# CityHero: erster Landschaftspass

Originale Blender-Geometrie unter `Art/Models/EnvironmentV2`: zwei unregelmäßige, abfallende Flussufer und vier geschichtete Sandsteinvarianten. Die Ufer liegen innerhalb des vorhandenen gesperrten Flussbands; beide Brückenkorridore bleiben frei. Felsen stehen außerhalb der spielbaren Fläche oder im gesperrten Wasser und haben keine Spielkollision.

Sand, Asphalt, Sandstein und Wasser verwenden zurückhaltende Materialvariation statt des groben, wiederholten Rauschmusters. Sonnenlicht und Umgebungslicht wurden ausgeglichen. Das Stadtzentrum erhält stärker gestaffelte Gebäudehöhen; Straßenenden ragen nicht mehr in den Fluss. Ein dünner grüner Auswahlring ersetzt die undurchsichtige blaue Auswahlfläche.

Reproduzierbar mit `Scripts/art/build_environment_v2.py` in Blender, danach `Scripts/art/polish_cityhero_environment.py` als Unreal-Python-Skript. Der zweite Schritt aktualisiert die vorhandene CityHero-Karte, erhält die Landmarken und ersetzt nur die eigenen ENV2-Akteure. `Saved/Research/environment-v2.json` enthält die Importdaten. Beide Schritte dürfen wiederholt werden.

Dies ist ein erster Landschaftspass. Die Karte besitzt weiterhin ein sehr gerades Straßennetz, einfache Stadthäuser und einen geraden Flussverlauf. Natürliche Flussmäander erfordern eine gemeinsame Änderung der sichtbaren Geometrie, Navigation und Minimap. Boden-Normalkarten, Straßenränder, Vegetation und ausgearbeitete Stadtblöcke stehen noch aus. TRELLIS wurde für diesen Schritt nicht benötigt.
