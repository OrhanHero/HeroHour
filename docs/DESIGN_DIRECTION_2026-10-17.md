# Verbindliche Gestaltungsvorgabe – 17. Oktober 2026

**Fortschritt nach dieser Entscheidung:** Die Zero-Hour-Anordnung wurde inzwischen im Canvas-HUD umgesetzt, paketiert und geprüft. Ein originaler Abschnitt mit 38 Actors/16 Mesh-Typen wurde in das separate UE5.8-Prüfprojekt importiert. Die vollständige Spielwelt, Originalshader und UMG-Migration sind nicht fertig. [Aktueller Plan](PROJECT_ROADMAP.md), [Übergabe](CURRENT_HANDOVER.md). Der Nutzer hat die Aufteilung Radar links, Befehle Mitte, Auswahl rechts, Ressourcen darüber bei der UMG-Folgeklärung erneut bestätigt. Die nachfolgende ursprüngliche Prüfreihenfolge beschreibt den Ausgangspunkt; aktuelle Erfolge und offene Arbeiten stehen im Plan.

Der Nutzer hat die zuvor falsch interpretierte UI-Richtung ausdrücklich korrigiert:

**C&C Generals / Zero Hour liefert die UI-Anordnung. Tempest Rising liefert die angestrebte Gestaltung und dient als konkrete Karten-/Umgebungsreferenz.**

Diese Vorgabe ersetzt die vorherige Entscheidung, Tempests hohe rechte Produktionssidebar und Radar oben rechts als HeroHours finales Layout zu verwenden. Der geprüfte Commit `b2847d2` bleibt ein funktionierender Zwischenstand, ist aber gestalterisch nicht die neue Zielanordnung.

## UI

Die vorhandene Nutzerreferenz `Art/Concepts/CityHero/zh_widescreen.jpg` ist maßgeblich für die räumliche Aufteilung: bodengebundene Kommandoleiste, Radar links unten, kontextabhängige Befehls-/Baukarten im unteren Mittelbereich und ausgewähltes Objekt samt Porträt/Information rechts unten. Ressourcen und Spezialfunktionen werden in diese Hierarchie integriert; keine frei hinzugefügte Tempest-Rechtssidebar.

Tempests Referenz bestimmt Materialwirkung, Rahmenausarbeitung, Typografiehierarchie, Zustandsdarstellung und visuelle Qualität. Einheiten-/Gebäudeporträts müssen innerhalb dieser Zero-Hour-Anordnung konsistent wirken. Die Mischung wird als eine vollständige Spielansicht geprüft, nicht anhand isolierter Panels. Die bestehenden funktionalen Befehle, Radarregeln, Erstattungen, Cooldowns und Pausenmodalität sollen bei der Layoutkorrektur erhalten bleiben.

## Karten: Vorhandenes zuerst untersuchen

Vom Nutzer ausdrücklich ausgewählter Editorordner:

`F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject`

Die lokale `README.txt` verlangt Unreal Engine **5.4.4**. `Tempest.uproject` enthält native Module `Tempest` und `TempestEditor`. HeroHour verwendet dagegen Unreal **5.8.3**. Deshalb wird der Originalordner nicht durch Öffnen/Speichern unter 5.8 konvertiert. Die reine Übereinstimmung „beide Unreal“ beweist keine funktionierende Migration.

Die offiziellen Entwickler-Patchnotes nennen `ZooExampleMap` und `SetdressingExampleMap`; letztere wird als vollständige Beispielkarte mit Tempest-Set-Dressing-Techniken beschrieben. Sie ist der erste Kandidat für die Prüfung einer bestehenden visuell überzeugenden Umgebung. Eine direkte Verwendung im unabhängigen HeroHour-Projekt ist noch nicht nachgewiesen. Kartenabhängigkeiten, bearbeitbare Assets, native Klassen und eine passende Editorinstallation sind getrennt zu prüfen. Das Vorhandensein des Karteneditors belegt einen Tempest-Modding-Workflow, keine pauschale Freigabe zur Veröffentlichung sämtlicher Inhalte in einem eigenständigen Spiel.

Offizielle Quelle: https://support.saber.games/hc/de/tempest-rising/articles/7-juli-2026-patchnotes-27

## Nächste Entscheidung anhand von Befunden

1. Vorhandene Beispielkarte und Editorabhängigkeiten inventarisieren; keinen neuen Landschaftsentwurf als Ersatz für ungeprüfte vorhandene Inhalte erfinden.
2. Klären, ob wir eine bearbeitbare Umgebung technisch in HeroHour nutzen können oder die vorhandene Tempest-Karte zunächst innerhalb ihres vorgesehenen Editors als Vergleichs-/Entwicklungsumgebung verwenden.
3. Einen vollständigen UI-Entwurf mit Zero-Hour-Anordnung und Tempest-Gestaltung an dieser konkreten Umgebung abnehmen.
4. Erst danach die bisherige Sidebar im HeroHour-Code umbauen und den ausgewählten Umgebungsweg umsetzen. Keine neuen Gameplay-Systeme während dieser Klärung priorisieren.

Dieser Eintrag dokumentiert die Entscheidung und die Prüfung. Er behauptet weder eine bereits importierte Tempest-Karte noch eine abgeschlossene neue UI.
