# Tempest-Kartenanalyse ohne laufende Engine

17. Oktober 2026. Tatsächlich ausgeführte, schreibgeschützte Untersuchung der vorhandenen SetdressingExampleMap. Kein Editor-/Spielstart, keine Assetkopie, kein Export und keine Änderung der Tempest-Installation.

## Tatsächliches Ergebnis

`Scripts/research/inspect_tempest_map_offline.py` liest standardisierte Unreal-Package-Summaries, Name- und Importtabellen. Es prüft Magic, Version, Stringterminatoren, Tabellen-/Dateigrenzen, FName-Indizes und Importverknüpfungen. Die Implementierung wurde mit der lokal vorhandenen Unreal-Serialisierung in `PackageFileSummary.cpp`, `ObjectResource.cpp`, `ObjectVersion.h` und `ObjectMacros.h` abgeglichen. Sie ist kein vollständiger Unreal-Assetparser.

- **43 von 43** ausgewählten `.umap`/`.uasset`-Dateien wurden strukturiert gelesen. Darunter die vier Karten Persistent, Enviro, Foliage und Nav.
- Alle 43 haben UE4-Package-Version 522 und unterstützte versionierte UE5-Summaries; ihre tatsächlichen `PKG_Cooked`- und `PKG_FilterEditorOnly`-Flags sind nicht gesetzt.
- **205 eindeutige direkte Paketimporte**: 188 lokal auffindbare Projekt-/Mod-Assets, sechs Engine-Assets und elf native Scriptmodule. Kein Projekt-/Mod-Assetpfad fehlt innerhalb der untersuchten direkten Importmenge.
- Von den 188 lokalen direkten Referenzen sind **55 nicht cooked**, **133 cooked**. Bei den 133 unversionierten Dateien wurden ausschließlich validierte Legacy-`-8`-Summary-Präfixe ausgewertet; weder eine gespeicherte Engine-Version erfunden noch ihre Export-/Property-Layouts geraten.
- Konkrete native Klassenreferenzen umfassen `TedGameMode_MP`, `TedLevelScriptActor`, `TedMapInfo`, `TedPlayerStart`, `TedWorldSettings`, `TedNavMesh`, `TedNavPylon`, `TempestFieldVolume`, `WorldBoundsActor`, `FoliageClusterActor`, `FoliageInteractionManager`, `TedNavArea_NoGroundUnits`, `TedNavArea_NoStructures` sowie `NavGrid.SquareNavGrid` und `NavGrid.GridMeshComponent`.

Diese Zahlen beziehen sich auf direkte Packageimporte der 43 ausgewählten Dateien. Sie sind **kein vollständiger transitiver Abhängigkeitsabschluss**. Exportzahlen sind Objekt-Exportzahlen, keine gezählten Actorinstanzen. Ein Klassenimport beweist keine aktive Instanz und keine Laufzeitwirkung.

## Was diese Befunde ermöglichen

Die Beispielkarten sind nicht pauschal als ausschließlich cooked abzuschreiben. Es gibt eine reale, versionierte Kartenbasis. Die Übertragung lässt sich in Darstellung, allgemeine Unreal-Bestandteile und Tempest-spezifische Spiellogik zerlegen. Ein nicht gesetztes Cooked-Flag beweist allein aber noch keinen erfolgreichen Import unter HeroHours Engine.

| Referenzbereich | Konkreter nächster Transfer-Schritt | HeroHour-Anbindung |
|---|---|---|
| Landscape / Enviro | Im Editor einen kleinen tatsächlich vorhandenen Landschaftsabschnitt mit seinen Material-/Texturabhängigkeiten inventarisieren; Position und Beleuchtung festhalten | Zunächst nur visuelle, isolierte Testkarte; keine Änderung der aktiven CityHero-Karte |
| Static meshes / Materialien | Direkte und transitive Abhängigkeiten pro ausgewähltem Objekt prüfen; nicht cooked und cooked getrennt behandeln | Erst nach erfolgreichem Laden und Bildvergleich als verfügbar markieren |
| TedPlayerStart / WorldBoundsActor | Tatsächliche Instanztransformationen und Grenzen auslesen | Eigene Startpunkte und Kartenbegrenzung mit denselben Messdaten vorbereiten |
| TempestFieldVolume | Feldposition und Ausdehnung ermitteln | Bestehende HeroHour-Ressourcenfelder anpassen; keine Tempest-Simulation voraussetzen |
| NavGrid / TedNavMesh / Nav areas | Tatsächliche Laufwege, Blocker und Brücken prüfen | Eigene Navigation und Bebaubarkeit verwenden; keine fremden Nav-DLLs nach 5.8 kopieren |
| TedGameMode / LevelScript / WorldSettings | Abhängigkeit und benötigte Kartenparameter getrennt aufnehmen | HeroHour-Spielregeln beibehalten; fremden GameMode nicht als komplettes RTS-Framework behandeln |
| Foliage | Mesh-/Materialpfade, Platzierung und Instanzzahlen prüfen | Sichtbare Pflanzenverteilung erhalten; spezielle Interaktionslogik bei Bedarf separat ersetzen |

Die Ersetzung ist ein vorgeschlagener Integrationsweg und noch kein ausgeführter Transfer. Auf unversionierte/cooked Referenzen folgt formatgerechte Analyse, nicht die Behauptung, sie seien automatisch verwendbar oder grundsätzlich unmöglich.

## Bereits vorbereitete Anschlussarbeit

Der zweite Agent hat `Scripts/research/inspect_tempest_map_editor.py` vorbereitet. Es liest im vorgesehenen UE-5.4-Editor die tatsächlichen Actortransformationen, Meshes, Materialien, Instanzzahlen und geladenen Sublevels und schreibt ausschließlich JSON unter HeroHours `Saved`-Verzeichnis. **Noch nicht in Unreal ausgeführt**; API-Lücken werden im Bericht ausgewiesen. [Transfer-Test und konkrete Befehle](TempestMap-Transfer-Test.md).

Der erste technische Transfer-Test umfasst eine kleine sichtbare Partie der vorhandenen Karte. Erfolg bedeutet: richtige Geometrie, Position, Oberflächen und Beleuchtung, keine fehlenden Assets/Klassen, anschließend funktionierende HeroHour-Navigation. Eine neue erfundene Landschaft ist kein Ersatz für diesen Test.

## Reproduzieren und Nachweise

```powershell
python Scripts/research/inspect_tempest_map_offline.py --project "F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject" --output "C:/Users/Hero/HeroHour/Saved/Research/TempestMapTransfer"
```

Die Originaldateien werden nur lesend geöffnet. Berichte enthalten Dateihashes und Bytepositionen der Tabellen beziehungsweise Flags; keine Geometrie oder Texturpixel werden exportiert. [Package-Tabellen](../verification/2026-10-17/map-transfer/Package-Imports.json), [direkte Abhängigkeiten](../verification/2026-10-17/map-transfer/Direct-Dependencies.json).

Die Gestaltung bleibt: **Zero-Hour-Anordnung der UI, Tempest-Gestaltung und eine tatsächlich untersuchte Tempest-Umgebung als Ausgangspunkt.** Die vorhandene rechte Sidebar ist nicht die finale Zielanordnung.
