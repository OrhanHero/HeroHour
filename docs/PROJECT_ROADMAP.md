# Verbindlicher Entwicklungsplan – 17.10.2026

## Ziel und Abgrenzung

HeroHour soll spielerisch ein schnelles klassisches RTS und visuell den gelieferten Konzepten deutlich ähnlicher werden. **Zero Hour bestimmt die UI-Anordnung; Tempest Rising bestimmt die angestrebte Material-, Umgebungs- und Präsentationsqualität.** Der aktuelle Prototyp erfüllt das Gesamtziel noch nicht.

Bestätigte Anordnung: Radar links unten, Befehle und Baukarten mittig, Auswahl rechts unten, Ressourcen darüber. Die abweichende Anordnung der eingefügten allgemeinen UMG-Vorlage wurde vom Nutzer ausdrücklich verworfen. [Gestaltungsvorgabe](DESIGN_DIRECTION_2026-10-17.md).

## Bereits ausgeführt

| Bereich | Belegter Stand | Offene Grenze |
| --- | --- | --- |
| Spielbarer Prototyp | Zwei Fraktionen, Wirtschaft, Strom, Produktion, Kampf, Sicht und Fähigkeiten | Kein vollständiges Spiel; Multiplayer und Kampagne fehlen |
| UI-Anordnung | Canvas-HUD umgebaut und paketiert | Materialwirkung und gesamte Konzeptqualität fehlen noch |
| Neueste Paketprüfung | 19 Suiten, 917 Einzelprüfungen, 46 natürliche Match-/Logistikrunden | Keine neue Langzeit- oder Leistungsabnahme |
| UI-Editorprüfung | Beide Fraktionen bei 1600×900 und 1280×720: je 76 Prüfungen | Kein getesteter 4K-UMG-Modus |
| Originalsektor | 16 Mesh-Typen, 38 Actors im getrennten UE5.8-Projekt; Import Exit 0 | Nicht in HeroHours Spielkarte integriert |
| Materialanalyse | 13 Quellen/31 Texturen über Parentketten; 506 Container-Mastergraph-Exports analysiert | UE-Import verwendet Albedo/Normal-Baseline, keinen vollständigen Originalshader |
| Terrain | 64 Componentbounds geprüft, zwei passende Komponenten identifiziert | Landscape-Geometrie noch nicht übertragen |
| UMG | Konkrete Hierarchie, C++-Vorlagen und Materialrezept dokumentiert | Noch nicht als Runtime-Migration eingebaut |

## M1 – Eine vollständige HUD-Ansicht gestalterisch fertigstellen

1. Bestehendes Layout beibehalten und Rahmen, Schrift, Bildsprache und Zustände gemeinsam ausarbeiten.
2. Wiederverwendbare Befehlskarte und parametrierbaren abgeschrägten Rahmen umsetzen; sichtbare Hover-, Auswahl-, Sperr- und Fortschrittszustände.
3. UMG schrittweise als alternative Darstellung einführen. Canvas und UMG dürfen dieselbe Mausaktion nicht doppelt auslösen. Bestehende semantische Befehlswege weiterverwenden.
4. Produktionspanel, Radar, Auswahl, Ressourcen, Tooltips und Modalrouting übernehmen. Blur zunächst nur begrenzt im mittleren Panel, mit Qualitätsfallback.

Abnahme: tatsächliche Spielaufnahmen und Interaktionstests beider Fraktionen bei 720p, 1080p, 1440p und 4K; keine Überlappung, abgeschnittene Kosten oder doppelte Käufe. Eine bestätigte Gesamtansicht ist Voraussetzung für weitere UI-Verbreiterung. [Konkrete Implementierung](UI_REFACTOR_IMPLEMENTATION.md).

## M2 – Den vorhandenen Kartenabschnitt vollständig nachvollziehbar darstellen

1. Ausgehend von `/Game/TransferProbe/Maps/OriginalSector02` Original-Materialvarianten korrekt erhalten; gleichnamige Materialien immer über vollständigen Assetpfad zuordnen.
2. Farbmasken, Rauheit, Metall und AO anhand belegter Graphverbindungen ergänzen. Grunge/Wetter und Material-Parameter-Collections getrennt prüfen; keine Dateinameninterpretation als Shadernachweis ausgeben.
3. Die beiden passenden Landscape-Komponenten gezielt exportieren/importieren oder die tatsächliche technische Grenze dokumentieren. Keine erfundene Ebene als Originalterrain ausgeben.
4. Einen beleuchteten Abschnitt in Unreal rendern und am Original prüfen. Blender-Prüfansichten bleiben separat gekennzeichnet.
5. Erst nach diesem Vergleich einen abgegrenzten Abschnitt in die HeroHour-Entwicklungswelt integrieren; Sicht, Navigation, Kollision und Maßstab prüfen. Originalinhalte bleiben während der Untersuchung im lokalen Prüfbereich, keine unbemerkte Aufnahme in Git.

Abnahme: UE-Aufnahmen, Originaltransform-/Boundsvergleich, dokumentierte Materialabweichungen und tatsächliche Wegfindung. Ein Containerhof ersetzt noch keine Gesamtkarte. [Sektornachweise](research/Tempest-Sector-Selection.md).

## M3 – Gebäude, Einheiten und Porträts konsistent machen

1. Einen vollständigen Fraktionshof mit HQ, Raffinerie, Kraftwerk, Fabrik und Abwehr als gemeinsame Ansicht abstimmen.
2. Sammler und Kampffahrzeuge bei Spielkamera und Nahansicht auf Silhouette, Maßstab, Teamfarbe und Lesbarkeit prüfen.
3. Passende Porträts aus den tatsächlich eingebauten Modellen erzeugen; keine abweichenden Symbolmodelle.
4. V11-Wohnhauskandidaten bleiben bis zur expliziten Qualitätsabnahme pausiert. TRELLIS nur nutzen, wenn es für eine konkrete Assetlücke hilfreich ist; kein pauschaler Austausch funktionierender Assets.

Abnahme: direkter Bildvergleich mit den gelieferten Konzepten und eine zusammenhängende Spielszene. Technisch importiert bedeutet nicht gestalterisch akzeptiert.

## M4 – Integration und überprüfbares Prototyp-Paket

1. Nach stabilen Änderungen neu bauen und paketieren.
2. Passende Verhaltens- und UI-Suiten ausführen; vor Abschluss erneut vollständige Pflichtsuiten am neuen Paket.
3. Sammlerfahrt, Entladung, Nachwachsen, Stromausfall, Produktion, Gefecht, Fähigkeiten und Sieg/Niederlage im realen Ablauf prüfen.
4. Isoliert GPU-/Framezeiten messen; anschließend längeren Grafiklauf durchführen. Alte Messungen nicht auf neue Assets oder UMG übertragen.
5. README, Übergabe, Buildnachweise, bekannte Fehler und PR-Beschreibung aktualisieren.

Abnahme: reproduzierbarer Start, neue Paket-/Log-Hashes, geprüfte Bilder und konkrete Liste verbleibender Konzeptlücken. Keine vollständige Tempest-Parität behaupten.

## Arbeitsaufteilung und Regeln

Bei paralleler Arbeit sinnvoll: UI/Slate, Materialanalyse, Karten/Terrain und Root für Integration, Enginejobs, Regression und Sichtprüfung. Nur tatsächlich laufende Agenten als aktiv melden. Owner-Dateien pro Aufgabe festlegen; keine konkurrierenden Enginejobs oder widersprüchlichen Dateiänderungen.

REA bedeutet Reverse Engineer Anything, keine RTS-Laufzeitarchitektur. Originalbeobachtungen, eigene Implementierung und Akzeptanztests getrennt dokumentieren. Hauptprojekt UE5.8.3; ModKit-Prüfkopie UE5.4.4. Original-ModKit nicht unter UE5.8 speichern. Git-Autor und GitHub-Konto ausschließlich OrhanHero.

## Wiederaufnahme

1. [Aktuelle Übergabe](CURRENT_HANDOVER.md), [Dokumentationsindex](README.md) und diesen Plan lesen.
2. `git status` prüfen; fremde lokale Dateien und pausierte Kandidaten nicht in einen pauschalen Commit aufnehmen.
3. [Verifizierten Build](VERIFIED_BUILD.md) und dessen Hashes prüfen, bevor neue Tests behauptet werden.
4. Nächste konkrete Aufgabe: M1-Befehlskarte/Paneldarstellung tatsächlich implementieren; M2-Material-/Terrainprüfung kann unabhängig vorbereitet werden. Die Anleitung allein ist kein implementierter Fortschritt.
