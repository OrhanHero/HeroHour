# HeroHour: Umbau zu einem RTS nach Tempest-Rising-Vorbild

Stand: **15.10.2026, Paketabnahme 13:45, Dauerprüfung läuft**. Ziel ist ein eigener spielbarer Unreal-Prototyp mit der gewünschten Wirtschafts-, Basisbau- und Kampfdynamik. Die Quellimplementierung umfasst inzwischen M1 bis M4; das bedeutet noch keine abgeschlossene Abnahme. Das aktuelle Windows-Paket besteht 19 Prüfläufe mit 519 Assertion-Prüfungen und 46 natürlichen Simulationspartien; die erneuten 20-Minuten-RHI-Dauerläufe laufen noch. Verbindliche Build- und Übergabebelege stehen in [CURRENT_HANDOVER.md](CURRENT_HANDOVER.md), WORKLOG und VERIFIED_BUILD.

Die früheren Aussagen „eine Zelle pro Feld“, „keine Strom-/Sichtsimulation“, „beide Teams teilen Aegis“ und „vier HUD-Aktionen“ beschreiben den historischen Stand vom 14.10. und gelten nicht für die neue Implementierung. Die acht bestandenen Paketsuiten des vorherigen Docking-Pakets ersetzen keine Prüfung des aktuellen Umbaus.

## 1. Tatsächliche Rolle von REA und Referenzbelege

[morluto/rea](https://github.com/morluto/rea) heißt **Reverse Engineer Anything**. Es ist kein RTS-Framework und liefert keine Unreal-Sammler-, Placement- oder Fog-of-War-Komponenten beziehungsweise Gameplay-Tags. Seine [Architektur](https://github.com/morluto/rea/blob/main/docs/architecture.mermaid) umfasst Analyse-Workflows, Provider-/Session-Routing, Analyseadapter und Evidence-/Unknown-Aufzeichnungen. In HeroHour dient es der Untersuchung: Referenzartefakt → belegte Eigenschaft oder offene Frage → eigene Implementierung → Abnahme. REA ist keine Laufzeitabhängigkeit des Spiels.

Zielartefakt: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\Tempest.exe`; der Bootstrap verweist auf `Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe`. Der installierte Shipping-Dateihash ist im [Produktions-/Docking-Pass](research/TempestRising-REA-Production-Docking.md) dokumentiert. Die Zuordnung der lokalen Installation zu einer unveränderten offiziellen Veröffentlichung bleibt unbestätigt.

- [Erster REA-Pass](research/TempestRising-REA-FirstPass.md), [Konfigurationsprüfung](research/TempestRising-ConfigReview.md) und [Editor/API-Prüfung](research/TempestRising-MapEditorReview.md): getrennte Belegarten; Deklarationen und Platzhalter sind keine rekonstruierte Mechanik.
- [Gezielte Systemprüfung](research/TempestRising-Systems-Evidence.md): zwei echte Ressourcen-Abfragekörper, zehn kurze Ressourcen-/Power-Platzhalter; Originalbereiche mit 200 beziehungsweise 102 Bytes verankert.
- [Feld-Pass 2](research/TempestRising-REA-Next-Pass.md): sechs echte Editor-Körper zu Wachstum, räumlichen Ressourcenindizes, Darstellungsstadien und Depletion; neun Bereiche mit 2.450 Originalbytes.
- [Shipping-Pass 3: Produktion und Docking](research/TempestRising-REA-Production-Docking.md): Refund-/Credit-Mutation, getrennte Queue-/Deposit-/Slot-Abfragen und Harvester-Datenstruktur. 13 Codebereiche beziehungsweise 21 Teilbereiche mit 2.637 Decoderbytes gegen Originalbytes geprüft. Produktions-Commit, Abbruchquote, Bezahlzeitpunkt, vollständiger Dockalgorithmus und konfigurierte Originalraten bleiben offen.

Diese Nachweise verankern analysierte Instruktionen in Originaldateien. Sie belegen weder eine bytegleiche Neukompilierung noch vollständige Verhaltensparität. Die nachfolgenden Zahlen und Abläufe sind eigene HeroHour-Regeln. Tempest wird in der [offiziellen Spielbeschreibung](https://tempestrising.com/) als elektrische Pflanze beschrieben; unsere kristalline Darstellung ist eine eigene Gestaltung. Eine globale Produktionsübersicht, wie in den [offiziellen Patchnotes](https://support.saber.games/hc/en/tempest-rising/articles/march-10-2026-patch-notes-21) beschrieben, ist von den darunter laufenden Gebäude-FIFOs zu unterscheiden.

## 2. Aktives Architektur-Mapping

`HeroHour.uproject` und seine Targets laden ausschließlich `HeroHourPrototype`. `Source/HeroHour` und der unvollständige F#-Altentwurf bleiben inaktiv. Die ursprünglich vorgeschlagenen Namen `UHRHarvesterComponent`, `UHRPlacementSubsystem`, `UHRTeamEconomyComponent` und `UHRVisibilitySubsystem` sind **keine implementierten Klassen**. Der aktuelle Umbau bindet eigenständige C++-Zustandskerne an den vorhandenen GameMode und die Einheiten. Eine spätere Auslagerung in Components/Subsystems muss diesen geprüften Vertrag bewahren und ist keine Voraussetzung für die erste Spielabnahme.

| Bereich | Aktive Implementierung | Verbleibende Abnahme |
|---|---|---|
| Felder und Sammler | `FHRResourceField`, `FPrototypeSalvage`, `TickCollector` in `PrototypeEconomy.cpp`; 48 räumliche Zellen, Reservierung und explizite Sammlerzustände | Mehrsammlerbilanz, Pause, blockierte/zerstörte Raffinerie und aktuelle Paketdarstellung erneut prüfen |
| Raffinerie | `FHRRefineryDocking`: getrennte exklusive FIFO-Docks, zeitliches Entladen, präzise Cargo-/Wallet-Buchung | Alle Docking-Gates gegen Mehrzellenfelder und neue Footprints wiederholen |
| Produktion | `FPrototypeProduction` mit Preis-/Dauer-Snapshot, bezahlte FIFO je zulässigem Produzenten; Fabrik, Kaserne und HQ können parallel arbeiten | Unabhängige Kategorien, Ausgangsblockade, Abbruch, Verkauf und Stromunterbrechung im Paket |
| Baugrid | `CanPlaceBuilding`, `SnapBuildPosition`, `GetBuildDimensions`; 200-cm-Raster, Vierteldrehung, verbundene Bauradien, sofort belegte Baustellen | Ghost, finale Platzierung und rechteckige Navigation müssen übereinstimmen |
| Navigation | `FPrototypeNavigation`: A*, Wasser/Brücken, rechteckige strukturbezogene Hindernisse mit Bewegungsfreiraum | Keine Eckdurchquerung; Spawn-, Kampf- und Dockanfahrten mit allen neuen Gebäuden |
| Strom | `FHRPowerGridState` und GameMode-Funktionen in `PrototypeSystems.cpp`; echte Verbraucher/Erzeuger, manuelles Abschalten | Ausfall, Lastabwurf und Wiederaufbau im vollständigen Match |
| Sicht | `FHRVisibilityGrid`, `UpdateVisibility`, Laufzeitmaske und Postprocess-Material | Gegner müssen bei Rendering, Auswahl, Zielsuche, Befehlen, Minimap und HUD konsistent verborgen bleiben |
| Kampf | `FHRCombatState`, `FHRDamageTable`, `APrototypeUnit`: typisierter Schaden, Rang, Deckung, Suppression und Fähigkeiten | Numerische Kern- und Live-Tests plus lesbares Gefechtsbild |
| Fraktionen/Doktrinen | Gespeicherte Aegis-/Ember-Definitionen; `FHRCommanderState`, zwei Powers je Fraktion | Beide Fraktionen sammeln bis Neustart unter denselben öffentlichen Matchbefehlen |
| Sidebar | `HeroHUD.cpp` und `PrototypeHUDLayout.h`: fünf Kategorien, Produzenten-FIFOs, Fähigkeiten, Doktrinzielwahl, Energie-/Radarstatus | Aktuelle Auflösungen, tatsächliche Klickpfade und Ähnlichkeit zu den Konzepten |
| Gegner/Spielrunde | `PrototypeBot.cpp`, `PrototypeAcceptance.cpp` und Match-Abnahme | Natürlicher Spielersieg, Niederlage, Wiederanlauf und echter Dauerlauf |

Das getrennte Beispiel `FHRProductionLane` in `HRProductionQueue.h/.cpp` bleibt ein Beispielkern mit `int64`-Wallet. Es ist **nicht** die Live-Produktionsquelle. Live führen Credits und Cargo `double`; der Umbau erzeugt kein zweites Konto und schneidet Bruchteile nicht über einen Integer-Adapter ab.

## 3. Daten, Umfang und eigene Assets

Die Schemas in `RTS/HRRTSDefinitions.h/.cpp` beschreiben:

- `UHRUnitDefinition`: Identität, Fraktion, Kategorie, Stats, Preis, Dauer, Voraussetzungen, Fähigkeiten, Veteranenstufen, Mesh und Icon.
- `UHRBuildingDefinition`: zusätzlich rechteckige Footprint-Zellen, Bauradius, Leistung, erlaubte Produktion und Radar-/Abwehrregeln.
- `UHRFactionDefinition`: Roster, Bau-/Wirtschaftsstil und aktive Bewegungs-/Panzerungsmultiplikatoren.
- `UHRDamageMatrix` / `FHRDamageRule`: exakte Schadens-/Rüstungspaare; fehlendes Paar hat Faktor 1, explizit 0 bedeutet Immunität.
- `FHRUnitStats`, `FHRVeterancyLevel`, `UHRTempestFieldDefinitionAsset`, `FHRTempestFieldDefinition` und `FHRTempestCellState`: Designwerte und getrennte Laufzeitzustände.

`create_rts_definitions_v4.py` erzeugt getrennte gespeicherte Rollendefinitionen und Roster für **Aegis und Ember** sowie die Schadensmatrix unter `/Game/HERO/DataV4`; der Feldvertrag stammt weiterhin aus dem gespeicherten Feldasset. Die aktiven Rollen sind **11**: Striker/Kampffahrzeug, Sammler, HQ, Raffinerie, Fabrik, Kraftwerk, Kaserne, Radar, Abwehr, Infanterie und Artillerie. Beide Fraktionen teilen die grundsätzlichen Rollen, unterscheiden sich durch Daten und Fähigkeiten. Aegis hat robustere Panzerung und Rauch; Ember höheres Tempo und Tempest-Waffen. Das ist der aktuelle asymmetrische Prototypumfang, kein vollständiges Original-Roster.

Native HeroHour-Tags in `HRRTSTags.h/.cpp` identifizieren Fraktionen, Rollen, `Production.*`, `Tech.Factory/Barracks`, `Resource.Tempest`, drei Schadens- und drei Rüstungsklassen, Status und Fähigkeiten. [Epics Gameplay-Tag-Dokumentation](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-tags-in-unreal-engine) beschreibt die Grundlage; Tags allein führen keine Mechanik aus.

Eigene Modelle, Materialien und UI-Grafiken ergänzen die Konzepte: bestehende ConceptV2-Hauptgebäude/Fahrzeuge, DistrictV3-Spielwelt und sechs FrontlinesV4-Rollenmodelle für Kraftwerk, Kaserne, Radar, Abwehr, Infanterie und Artillerie. Hinzu kommen eigene prozedurale Schuss-/Treffer-Audios, Waffen-/Raucheffekte sowie ein eigenes Infanterie-Skelett mit drei importierten Clips für Idle, Walk und Fire. Die Clip-/Importprüfung ersetzt keine visuelle Abnahme: vorhandene Bind-Pose-Warnungen müssen am gerenderten Modell geprüft werden. Es werden keine Tempest-Modelle, Texturen oder Audiodateien in HeroHour übernommen. TRELLIS und Assetkäufe sind für diesen Umfang keine Voraussetzung.

## 4. Aktueller Mechanikvertrag

### Wirtschaft und Logistik

Drei Felder besitzen je 16 unabhängige, erreichbare räumliche Zellen, insgesamt **48**. Anfangsbestand und Kapazität werden aus den bisherigen Feldsummen 4.000/4.000/6.000 auf die Zellen verteilt. Nachwachsen ist jetzt Standard; `-PrototypeFiniteFields` schaltet ausdrücklich auf endliche Vorräte. Das eigene Standardbudget beträgt 2 Einheiten je Sekunde **pro Feld**, auf dessen 16 Zellen verteilt, mit zellbezogener Ruhezeit von 10 Sekunden. Es wird nicht 16-fach vervielfacht.

`FHRResourceField` ist die Bestandsautorität; `Remaining` ist ein abgeleiteter Kompatibilitätswert. Die sichtbaren Pflanzen folgen der jeweiligen Zellmenge über Primitive-Daten und Wachstumsstadien. Die 17 Cluster eines Patch-Meshs sind Geometrie, keine zusätzlichen Simulationszellen.

Sammler durchlaufen `SeekingField`, `MovingToCell`, `Harvesting`, `MovingToRefinery`, `WaitingForDock` und `Unloading`. Manuelle Bewegung unterbricht die Automatik und gibt Reservierungen frei. Ernte wird durch Rate, freien Laderaum und entnehmbaren Zellbestand begrenzt. Feldreservierungen und Dockreservierungen bleiben getrennt. Eine Raffinerie besitzt im eigenen Prototyp einen exklusiven Dockslot und entlädt 40/s erst nach tatsächlicher Ankunft. Credits entstehen nur durch die entsprechende Cargo-Abbuchung. Gemeinsame darstellbare Buchungsschritte erhalten kleine Restmengen; nicht buchbares Restcargo wird in die nächste Ernte übernommen.

Die historische [M1-Abnahme](research/TempestRising-M1-Acceptance.md), der [Integrationsaudit vom 14.10.](research/Field-Integration-Audit-2026-10-14.md) und die [Docking-Abnahme](research/Refinery-Docking-Acceptance.md) bleiben Belege ihrer jeweiligen Stände. Die Mehrzellenintegration ist durch die aktuellen Economy-/Resource-/Docking-/FullMatch- und Logistik-Stressprüfungen belegt.

### Strom, Basisbau und Produktionskategorien

`CurrentPower` summiert fertiggestellte, eingeschaltete Erzeuger; `RequiredPower` zählt aktive Verbraucher. Verbraucher bleiben bei automatischem Strommangel in der Nachfrage, damit das Netz nicht oszilliert. Manuelles Abschalten entfernt die betreffende Last beziehungsweise Quelle und hält den zugehörigen Betrieb an.

Eigene Raten: ausreichend Leistung → 1; positive Unterversorgung → 0,5; keine Leistung bei positiver Nachfrage → 0. Radar und stromabhängige Abwehr benötigen volle Versorgung. Bezahlte Produktionsaufträge behalten bei Unterbrechung ihren Fortschritt. Kraftwerksbau besitzt einen unabhängigen Baukanal und kann mit Rate 1 fertig werden, damit der Netzwiederaufbau nicht an fehlendem Strom scheitert. Gebäude-, Abwehr- und Kraftwerksbaustellen werden getrennt fortgeschrieben; die übrigen Produktions-FIFOs sind an tatsächliche Produzenten gebunden.

Platzierung erfolgt über die Sidebar, ohne Bauarbeiter, im verbundenen Bauradius fertiggestellter eigener Gebäude. Gerasterte rechteckige Footprints, Rotation und belegte Baustellen fließen in Platzierungs- und Navigationsprüfung ein. Verkauf erstattet den definierten Gebäudewertanteil und die gespeicherten bezahlten Aufträge; Cargo anderer Actors darf dabei nicht verschwinden.

### Information und Kampf

Sicht und Erkundung sind je Team getrennt. Die Welt erhält eine Laufzeit-Fog-Maske; Gegner werden zusätzlich als Actors verborgen und aus Sicht-/Ziel-/HUD-Abfragen gefiltert. Ein ausgeschaltetes beziehungsweise unversorgtes Radar deaktiviert die taktische Karte. Rauch besitzt eine Simulationslebensdauer und beeinflusst gegnerische Sichtlinien; eigenes Rauchverhalten und Nahsicht sind explizite eigene Regeln.

Kinetisch, Explosiv und Tempest treffen Infanterie, Fahrzeuge und Strukturen nach einer gespeicherten Matrix. XP erzeugt drei Veteranenstufen mit getrennten Schadens-, Reichweiten- und Panzerungsboni. Infanterie erhält Deckung an Gebäudekanten und wird durch Treffer unterdrückt. Granate, Rauch und Deployment besitzen geprüfte Commit-/Cooldown-Zustände; Artillerie kann beim Deployment nicht fahren. Aegis-Doktrinen: Reparaturimpuls und Präzisionsschlag; Ember: Flammenzone und Übertaktung. Punkte, Zielvalidierung, Wirkung und globaler Cooldown gehören zur Simulationsautorität.

## 5. Meilensteine und noch erforderliche Abnahme

Die Mechanik- und Paketprüfungen für M1–M4 sind im aktuellen Bericht belegt; die vollständige M4-Langzeitabnahme läuft noch. Vorhandene Funktionen, importierte Assets oder einzelne bestandene Kernprüfungen rechtfertigen keine pauschale Fertigmeldung.

| Meilenstein | Implementierter Arbeitsumfang | Verbindlicher Abschlussnachweis |
|---|---|---|
| **M1: Daten und Wirtschaft** | Gespeicherte Definitionen, 48 Feldzellen, Nachwachsen, Sammlerzustände, erreichbare Reservierungen und zeitliches Docking | Vier Sammler ohne Doppelentnahme; Bilanz aus Bestand, Cargo und Lieferung; blockierte/zerstörte Raffinerie erholt sich; Pause/Neustart; Cook lädt aktuelle Daten |
| **M2: Basis und Information** | Stromnetz, manuelles Abschalten, Produzenten-FIFOs, fünf Sidebar-Kategorien, Baukanäle, verbundenes Grid, rechteckige Navigation und Fog | Unabhängige Kategorien; echter Low-Power-Ausfall und Wiederaufbau; Vorschau/Belegung konsistent; keine versteckten Gegner in Auswahl, Zielsuche oder Minimap; echte Klickprüfung |
| **M3: Kampf und Micro** | Schadensmatrix, Infanterie, Deckung/Suppression, Rangstufen, Granate/Rauch/Deployment | Numerische und Live-Abnahme; keine Doppelboni; gültiger Commit vor Verbrauch; Erholung/Cooldowns auf Simulationszeit; Sicht-/Pause-/Stromregeln werden eingehalten |
| **M4: Fraktionen und Spielrunde** | Zwei gespeicherte Roster, vier Commander-Powers, öffentliche Botbefehle, eigene Rollenassets, Effekte, Audio und drei Infanterieclips | Beide Fraktionen vom Sammeln bis Sieg/Niederlage/Neustart; mindestens ein natürlicher Spielersieg gegen unveränderten Bot; aktuelle Konzept-Captures; Animationsprüfung; 20 Minuten verpackt mit echtem RHI und gemessenen Framezeiten/Speicher ohne festhängende Logistik |

Die aktuellen Testgruppen umfassen Kernprüfungen und Live-Szenarien für Wirtschaft, Docking, Navigation, Architektur/Daten, Kampf, Sicht, Systeme, UI und vollständige Matches. Der geplante Dauerlauf muss reale verstrichene Zeit messen; eine schnelle manuelle Simulation ersetzt ihn nicht. Stalls, fehlende echte Spielausgänge, ungültige Geld-/Cargo-Werte sowie Log-/Assetfehler dürfen nicht durch Aufweichen der Assertions kaschiert werden.

Nach diesem Umfang folgen erst Kampagne, dritte Fraktion, Luft-/Marine-Roster, Multiplayer und große Techbäume. Eine spätere organisatorische Aufteilung des GameMode in Subsystems darf keine neue parallele Spielautorität schaffen.

## 6. Belegstandard und Übergabe

[awesome-rea](https://github.com/yibie/awesome-rea#inclusion-criteria) verlangt einen benannten Verifikationstest. Bytevergleich ist typisch für Matching-Decompilation; Ports/Recompilation können andere Verhaltens- oder Kompatibilitätstests vorweisen. Für HeroHour werden vier Aussagen getrennt protokolliert: Originalartefakt mit Hash; unabhängig rekonstruierte Funktion; fester verhaltensgleicher Original-/HeroHour-Ablauf; eigener bestandener HeroHour-Test. Derzeit besteht weder eine nachgewiesene bytegleiche Neukompilierung noch vollständige Original-Verhaltensparität.

Referenzabläufe bleiben Sammlerzyklus, Kraftwerkverlust/Wiederherstellung, Bauplatzgrenze, parallele Produktion, Storno, blockierter Ausgang und verlorener Sichtkontakt. Jeder Nachweis braucht Version/Hash, Startzustand, Aktion, Ergebnis und Messgrenze. Ein Renderbild belegt keine native Klickprüfung; funktionale HeroHour-Tests belegen keine Tempest-Parität. Die vom Nutzer aktive Tempest-Partie wird nicht ohne seinen Auftrag gesteuert.

Vor der Übergabe sind aktueller Build, Suite-Logs, Paketzeitpunkt, betrachtete Screenshots, Dauerlaufbericht, verbleibende Warnungen und Git-Identität ausdrücklich zu dokumentieren. Nur tatsächlich ausgeführte Prüfungen werden als bestanden eingetragen. Aktuell offene Gesamtabnahme wird nicht als fertiger Tempest-Umbau bezeichnet.
