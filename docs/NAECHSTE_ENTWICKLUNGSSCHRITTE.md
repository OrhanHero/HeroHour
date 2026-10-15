# Weitere Entwicklung nach dem Tempest-Umbau

Stand: 15.10.2026. Der M1–M4-Kern ist implementiert und durch 19 aktuelle Windows-Paketläufe geprüft: 14 Aegis-Suiten und fünf zusätzliche Ember-Läufe, insgesamt 519 Assertion-Prüfungen und 46 natürliche Simulationspartien. Zwei 20-Minuten-Dauerläufe mit echtem RHI laufen noch; ihr Ergebnis gehört separat in [VERIFIED_BUILD.md](VERIFIED_BUILD.md) und [CURRENT_HANDOVER.md](CURRENT_HANDOVER.md). Die Gesamtabnahme bleibt bis dahin offen.

## 1. Laufende Abnahme abschließen

Aktuelle Prüfpunkte: Presentation 34, Economy 102, Systems 54, Architecture 63, Tactical 46, Visibility 25, Combat 17, UI 22, Navigation 15, Resource 18, Docking 28 und FullMatch 17. Natürliche Matchläufe enthalten drei Runden je Fraktionsstart. Maßgeblich sind die zugeordneten Logs und das Paketmanifest; diese Tests beweisen keine Tempest-Originalparität.

Die Dauerläufe müssen je Fraktion 1.200 echte Sekunden erreichen, Framezeiten/Speicher messen, natürliche Spielausgänge wiederholen und festhängende Sammler erkennen. Währenddessen keine Paketdateien durch Build oder Import ersetzen. Schnelle Simulationszeit ersetzt keine reale RHI-Laufzeit. Danach eine bediente Partie mit Sidebar, Rotation, Radarverlust, Fähigkeiten, Doktrinzielwahl, Verkauf/Storno, Pause und Neustart prüfen.

## 2. Visuelle Konzeptnähe

Verbindliche Referenz bleibt Art/Concepts/CityHero/Codex-Bild 1. Sept. 2026, 01_22_00.png: Ressourcen und Armee oben, Minimap unten links, Auswahl unten mittig, Produktion unten rechts. Dunkle Metallrahmen und eigene Porträts sind vorhanden. Tatsächliche Spielrender gegen die Referenz vergleichen; lesbare Zustände und frei bedienbare Weltfläche erhalten.

Elf Rollen, zwei Fraktionsdatensätze, Ressourcenfelder, Strom und Weltnebel sind spielbar. Weitere Art-Arbeit betrifft insbesondere Geländeübergänge, Gebäudeoberflächen, Animationen, Effekte und getrennte Fraktionssilhouetten. Viele Meshes sind noch gemeinsam. Es handelt sich um eigene Prototypassets, keine fertige AAA-Grafik oder Übernahme von Tempest-Modellen.

Das eigene Infanterierigg besitzt Idle-/Walk-/Fire-Clips. Importdaten und Clipdauer sind technisch geprüft; Bind-Pose-Warnungen und Gelenkbewegungen brauchen weiterhin visuelle Prüfung am gerenderten Modell. Räder, Türme, Tore, Ladezustände und differenzierte Todesanimationen sind nächste Art-Schritte. Sichtbare Geometrie, Grid und Navigation müssen übereinstimmen.

## 3. Rebuild und Reimport

Arbeitskopie: C:/Users/Hero/HeroHour; Engine: F:/UE/UE_5.8; Originalkonzepte zusätzlich E:/HeroHour. Neuer Account liest zuerst CURRENT_HANDOVER. Git-Identität bleibt ausschließlich OrhanHero. Nach Checkout Git LFS installieren und git lfs pull ausführen. Builds/Windows ist ignoriert und wird lokal erzeugt.

Normale Codeänderungen brauchen keinen pauschalen Asset-Reimport. Build-HeroHour.ps1 verwendet bereits -gather für neue Quellen. Zuerst das Editor-Modul bauen, erst danach Unreal-Python-Skripte ausführen, die neue reflektierte Klassen benötigen. Editor/Game vor Binärersetzung schließen. Import, Materialerzeugung und Packaging sequenziell ausführen.

Vollständige V4-Neuerzeugung in dieser Reihenfolge:

1. Bestehende ConceptV2-Meshes/Materialien und DistrictV3 bereitstellen. Eigene Quellen mit build_frontline_assets_v4.py, build_infantry_rig_v4.py und build_audio_v4.py in den vorgesehenen Blender-/Python-Laufzeiten erzeugen.
2. import_frontline_assets_v4.py importiert sechs statische Rollenmeshes und benötigt ConceptV2-Materialien.
3. import_infantry_rig_v4.py importiert Skelett und drei Clips. Skeletal-Materialnutzung setzen, Materialzuordnung ändern und Material sowie Mesh ausdrücklich mit save_loaded_asset(..., only_if_is_dirty=False) speichern. Ein nur im laufenden Importprozess korrektes Materialarray reicht für das nächste Cook nicht.
4. import_audio_portraits_v4.py importiert eigene WAVs und Porträts. Geänderte UI-Rahmen/Minimap über ihre eigenen Imports erneuern.
5. create_fog_effects_v4.py erzeugt Fog-/Smoke-/Impact-Materialien. Danach create_rts_definitions_v4.py: die Definitionen referenzieren bereits vorhandene Meshes/Icons und setzen Werte, Matrix und Fraktionen.
6. Build-HeroHour.ps1 -Package erzeugt das aktuelle Windows-Paket. Zuerst Presentation/Architecture, anschließend betroffene Gameplay-Suiten und beide Fraktionen gegen genau dieses Paket prüfen. Editor-Erfolg ersetzt keinen Paketest.

Importberichte unter Saved/Research sind lokale Arbeitsbelege; gespeicherte uassets bilden die Cook-Grundlage. Nach Rigg-/Materialimport die separate Paketpräsentation prüfen. Build-Logs und vertrauliche lokale Konfiguration nicht ungeprüft veröffentlichen.

## 4. Balance und Pflege

Sammlerwege, Stromreserve, Produzenten, Infanterie/Artillerie, Veterancy und Commander-Powers in echten Partien bewerten. Tests nicht durch Geld-/HP-Geschenke oder abgeschwächte Gegner grün machen. KI-Stalemates mit Befehlen, Positionen und Spielzeit dokumentieren; natürliche Siege und Niederlagen bleiben notwendig.

Aktive Zustandskerne sind FHRResourceField, FHRRefineryDocking, FHRVisibilityGrid, FHRCombatState und FHRCommanderState; Produktion/Strom sind an den GameMode gebunden. Früher geplante UHR-Subsystemnamen sind keine bereits eingebauten Klassen. Spätere Auslagerung erhält dieselbe Autorität und Testverträge statt einer zweiten Wirtschaft.

## 5. REA und optionale Werkzeuge

REA dient gezielter Referenzanalyse. Originalbyte-Anker beweisen keine neu kompilierte Rekonstruktion oder vollständige Gameplay-Parität. Originalraten und vollständige Queue-/Dockalgorithmen bleiben offen. Details: [Transformationsplan](TEMPEST_TRANSFORMATION_PLAN.md).

TRELLIS.2 ist optional und nur für einen begrenzten Versuch sinnvoll, wenn er bessere nutzbare eigene Assets liefert. Referenzen: [Repository](https://github.com/microsoft/TRELLIS.2), [Pipeline](https://github.com/microsoft/TRELLIS.2/blob/main/trellis2/pipelines/trellis2_image_to_3d.py), [Texturbeispiel](https://github.com/microsoft/TRELLIS.2/blob/main/example_texturing.py). Frühere Speicher-/WSL-Befunde sind historische Prüfungen und vor einer Installation neu zu prüfen. Keine große Installation ist für den aktuellen Prototyp nötig.

Übergabe und Veröffentlichung nennen überprüften Commit, Paketzeitpunkt, Bilder, tatsächliche Testläufe und Grenzen. Kampagne, Multiplayer, weitere Fraktionen und Luft-/Marine-Roster folgen nach der Abnahme dieses Umfangs.
