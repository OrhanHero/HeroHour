# Tempest Rising → HeroHour: Gestaltung und Spielsysteme

Stand: 14.10.2026. Fortsetzung der lokalen Referenzuntersuchung. Die REA-Verbindung ist erreichbar; aktuell ist kein natives Ziel geöffnet. Die bestehenden nativen Befunde werden wiederverwendet. Dieser Schritt untersucht zusätzliche mitgelieferte Dokumentation und ein begrenztes Ordnerinventar, keine Shipping-Algorithmen und keine exportierten Modelle.

## Neuer Beleg: Environment Art Guide

Quelle: `F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject/Docs/setdressing.pdf`, 61 Seiten, SHA-256 `94e60801404640c8027500d78f431729240dd7cd2ea091db6d52a2cddfeb961d`. Vollständiger seitenweiser Text im ignorierten `Saved/Research/TempestRising/setdressing-pages.json`; Seite 58 zusätzlich gerendert und visuell geprüft. Das Dokument beschreibt den Editor-Workflow, nicht die tatsächliche Ausführung des Shipping-Spiels.

Beobachtet:

- S. 3–10: Landschaft verwendet einen eigenen Material-Master, Materialinstanzen, bemalbare Ebenen und Runtime Virtual Textures.
- S. 17–23: Felsen und Klippen verwenden einen getrennten Environment-Master; Texturvariation, Normalstärke und Rauheit werden gezielt gesteuert.
- S. 26–38: Gebäude/Props verwenden einen eigenen Master mit UV- bzw. Weltkoordinaten, Masken, Materialvariation und Schmutzparametern.
- S. 58–59: dekorative Meshes und Navigationsblocker werden getrennt behandelt; einfache Blocker mit ausreichendem Freiraum werden empfohlen. Das Handbuch benennt dafür NavModifierVolume und deaktivierte Kollision dekorativer Meshes.
- S. 59–61: Vertexfarben und RVT helfen beim Übergang von Felsen/Props zum Gelände. Diese Übergänge sind ein eigener Arbeitsschritt.

Das begrenzte Verzeichnisinventar bestätigt im Editorprojekt getrennte Bereiche für Environments, Props und MaterialLibrary. Props enthält unter anderem RoadAsphalt, RoadBridge, Sidewalk, HighRise_WestEU, ApartmentsMiddleEast, Industrial_Pipes und Trimsheets. Das sind Dateisystembeobachtungen; daraus folgen keine geprüften Modellabmessungen, Polygonzahlen, Materialqualität oder Nutzungsrechte.

## Verbindung mit C&C

Die öffentliche Zero-Hour-Quellprüfung belegt getrennte Lade-/Liefertransaktionen, reservierte Andockplätze und eine Produktion mit Zahlung, Warteschlange und Ausgangsfreigabe. Details und Quellstellen: [Mechanikprüfung](ZeroHour-Mechanics-SourceReview.md). Diese Struktur dient als Referenz für unsere eigene Implementierung; daraus wird kein fremder Code kopiert.

HeroHour besitzt bereits endliche Ressourcen, Auszahlung bei Lieferung, gebäudebezogene Produktion, gespeicherte Auftragskosten, einmalige Stornoerstattung und validierte Sammelpunkte. Konkurrenz mehrerer Sammler, Andockplätze und dynamischer Abstand sind noch auszubauen. Es wäre falsch, diese Teile bereits als nachgebildetes C&C-Verhalten auszugeben.

## Konkreter nächster Bauabschnitt

Ein spielbarer Abschnitt aus HERO-Basis, südlicher Brücke und angrenzendem Stadtblock wird zuerst ausgearbeitet. Die eigenen Konzeptbilder bestimmen Silhouetten, Wüstenfarbe und Fraktionsstil; Tempest liefert einen belegten Arbeitsablauf für Materialien, Übergänge und die Trennung von Dekoration und Navigation.

1. Eigener modularer Baukasten: Straßenstück, Bordstein/Gehweg, Gebäudesockel, zwei Fassaden und Dachvarianten; Maße und Durchfahrtsbreiten vor Detailmodellierung festlegen.
2. Eigene Materialbasis mit Albedo, Normal und Rauheit für Sand, Asphalt, Beton und Metall. Schmutz und Materialübergänge gezielt einsetzen statt bloßer Noise-Farbe. RVT erst integrieren, wenn die Geländegrundlage dafür geeignet ist; das aktuelle Terrain ist ein Static Mesh.
3. Stadtblock und Basiszufahrten aus diesem Baukasten setzen. Dekoration erhält keine Wegsperre; tatsächliche Gebäudegrundrisse müssen mit der Navigation übereinstimmen.
4. Vier Sammler mit reservierten Anfahr-/Andockplätzen sowie Fabrikproduktion durch die Brücke prüfen. Visuelle Qualität und Spielablauf gemeinsam abnehmen.
5. Echte Unreal-Bilder aus festgelegter Spielkamera mit den Konzepten vergleichen. Erst dann auf weitere Kartenteile ausweiten.

## Grenzen

Die frühere UnrealPak-Auflistung des Shipping-Pakets scheiterte an einem fehlenden Verschlüsselungsschlüssel. Daraus wurden keine Modelle extrahiert. Native Editorfunktionen waren in zwei geprüften Fällen Platzhalter; die große Shipping-Analyse endete am Client-Timeout. Siehe [REA-Erstprüfung](TempestRising-REA-FirstPass.md). Weitere EXE-Decompilation würde diese konkreten Gestaltungsfragen nicht automatisch beantworten. Nächste sinnvolle Evidenz wäre eine dokumentierte visuelle Prüfung ausgewählter Editor-Modelle bzw. einer Spielszene, mit gemessenen Größenverhältnissen. Diese Messung wurde in diesem Schritt noch nicht durchgeführt.
