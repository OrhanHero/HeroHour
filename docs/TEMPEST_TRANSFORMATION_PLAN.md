# HeroHour: Umbau zu einem RTS nach Tempest-Rising-Vorbild

Stand: 14.10.2026. Ziel ist ein eigener spielbarer Unreal-Prototyp mit den gewünschten Regeln, zunächst offline. Dieser Plan ersetzt keine behauptete Rekonstruktion des Shipping-Spiels. Für den aktuellen technischen Übergabestand siehe [CURRENT_HANDOVER.md](CURRENT_HANDOVER.md).

## 1. Tatsächliche Rolle von REA

Das verlinkte [morluto/rea](https://github.com/morluto/rea) heißt **Reverse Engineer Anything**. Es enthält keine RTS-Sammlerkomponente, Baugrid-Komponente, Fog-of-War-Komponente oder entsprechende Unreal-Gameplay-Tags. Deshalb gibt es auch kein REA-Spielmodul, das wir für Tempest-Mining erweitern könnten.

Die [veröffentlichte Architektur](https://github.com/morluto/rea/blob/main/docs/architecture.mermaid) gliedert REA in CLI/MCP-Einstieg, gemeinsame Analyse-Workflows, Provider-/Session-Routing, Ghidra/Hopper/weitere Analyseadapter und Evidence-/Unknown-Aufzeichnungen. In HeroHour verwenden wir es im Entwicklungsprozess: Referenzartefakt → gezielte Untersuchung → belegte Regel oder offene Frage → eigene Implementierung → Abnahmetest. REA wird keine Laufzeitabhängigkeit des ausgelieferten Spiels.

Untersuchungsziel: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\Tempest.exe`. Der Bootstrap verweist auf `Tempest\Binaries\Win64\Tempest-Win64-Shipping.exe`. Bereits vorhandene [REA-Belege](research/TempestRising-REA-FirstPass.md), [Konfigurationsbelege](research/TempestRising-ConfigReview.md) und [Editor/API-Belege](research/TempestRising-MapEditorReview.md) bleiben getrennt. Die erste Editorprüfung traf Platzhalter. Die spätere [gezielte Systemprüfung](research/TempestRising-Systems-Evidence.md) belegt zwei echte Ressourcen-Abfragekörper und zehn kurze Ressourcen-/Power-Platzhalter; 21 Records sind gespeichert. Byte-Anker 200/102 Bytes binden die analysierten Originalbereiche an die Editor-DLL, sind keine neu kompilierte Rekonstruktion. Die Shipping-Analyse hatte einen Client-Timeout. Native Sammler-/Queue-Algorithmen und Original-Balancewerte sind damit nicht rekonstruiert.

Der [zweite gezielte Pass](research/TempestRising-REA-Next-Pass.md) fand danach echte Editor-Logik für Wachstumsquellen, räumliche Ressourcenindizes, Darstellungsstadien und `SetResourceDepleted`. Sechs ausgewählte Körper sind über 9 Originalbereiche/2.450 Bytes verankert. Dies belegt statischen Kontrollfluss, keine neu kompilierte oder dynamische Originalparität. Ein erster Shipping-Refundkörper in Pass 3 wird noch abschließend dokumentiert; bis dahin keine vollständige Queue-/Stornoregel als rekonstruiert ausgeben.

Die gewünschten Mechaniken werden als HeroHour-Design behandelt, bis ein konkreter Referenzablauf verifiziert ist. Tempest ist laut [offizieller Spielbeschreibung](https://tempestrising.com/) eine elektrische Pflanze; das gewünschte flüssige/kristalline Aussehen ist eine mögliche eigene Darstellung. Kraftwerksleistung ist kein auszugebendes Geldkonto. Die aktuellen [offiziellen Patchnotes](https://support.saber.games/hc/en/tempest-rising/articles/march-10-2026-patch-notes-21) belegen eine globale Produktionsübersicht; ihre Anzeige und die darunter laufenden Produzentenwarteschlangen müssen als unterschiedliche Dinge modelliert werden.

## 2. Mapping auf die aktive Codebasis

`HeroHour.uproject` und die beiden Targets laden nur `HeroHourPrototype`. `Source/HeroHour` ist ein inaktiver Altentwurf; das F#-Projekt referenziert zahlreiche fehlende Dateien. Diese Systeme werden nicht neben dem funktionierenden Prototyp aktiviert.

| Bedarf | Vorhandener Baustein | Umbau |
|---|---|---|
| Sammler | `PrototypeEconomy.cpp`: Vorrat, schwache exklusive Reservierung, echte Fahrt, Cargo und Lieferung | In `UHRHarvesterComponent` überführen; explizite Zustände und getrennte Abhol-/Dockreservierungen |
| Tempest-Felder | `FHRResourceField` aktiv; gespeichertes Feldasset; eigenes Patch-Mesh, Depletion-Events/Primitive-Activity, eine Zelle je Node | Mehrere erreichbare räumliche Zellen, Wachstumsdarstellung und vollständiger Bestand als einzige Quelle; optionaler Component besitzt den Kern |
| Produktion | Bezahlte FIFO je Gebäude, Preis-Snapshot, einmalige Erstattung, geblockter Ausgang, Rally | Team-Lanes pro Kategorie plus Registry zulässiger Produzenten; bestehende Methoden zunächst als Adapter behalten |
| Bauplatz | Ghost und freie Kreisprüfung | `UHRPlacementSubsystem`: gerasterte rechteckige Footprints, Rotation, verbundener Bauradius, atomare Belegung |
| Navigation | Begrenztes A*, Wasser-/Brücken-/Gebäudeprüfung | Weiterverwenden; Baugrid und Sichtgrid dürfen andere Auflösungen haben |
| Strom | Keine aktive Stromsimulation | `UHRTeamEconomyComponent`: Wallet und Leistung getrennt; Zustand beeinflusst tatsächliche Produktion, Radar und Abwehr |
| Fog of War | Keine aktive Sichtsimulation | `UHRVisibilitySubsystem`: sichtbar/erkundet je Team; Zielsuche, Befehle, Minimap und Renderansicht filtern |
| Kampf | Gespeicherte HP/Move/Weapon-Werte aktiv; bisherige Zielsuche und direkte HP-Schadensanwendung | Aktive Schadens-/Rüstungsmatrix, Veterancy, Deckung/Suppression und Fähigkeitszustände |
| Sidebar | Canvas-HUD mit vier echten Aktionen | Ansicht über Teamzustand, Kategorien und verfügbare Definitionen; Widgets dürfen weder Geld abbuchen noch selbst produzieren |

Alle `UHR...Component/Subsystem`-Namen dieser Tabelle sind geplante Komponenten. Acht Definitions-uassets unter `/Game/HERO/DataV3` sind inzwischen gespeichert und im Windows-Paket geladen: fünf Rollen, TempestField, Aegis und DamageMatrix. Live lesen Rollen Kosten/Dauer/HP/Move/Weapon/Mesh/Icon, Felder lesen Rates/Delay/Seed. Beide Teams teilen vorläufig Aegis; Roster/Fraktionsasymmetrie, Baugrid/Power, Matrix/Abilities/Veterancy sind weiterhin teilweise Metadaten oder Beispiele. Neue Team-Lanes bleiben unangekoppelt. Die [M1-Abnahme](research/TempestRising-M1-Acceptance.md) und der [Integrationsaudit](research/Field-Integration-Audit-2026-10-14.md) trennen Kernprüfung, Live-Match und Referenzvergleich.

## 3. Datenmodell und Tags

In `Source/HeroHourPrototype/RTS/HRRTSDefinitions.h/.cpp` liegen die kompilierbaren Schemas:

- `UHRUnitDefinition`: Identität, Fraktion, Kategorie, Preis/Zeit, Mesh/Icon, Voraussetzungen, Fähigkeiten und Veteranenstufen.
- `UHRBuildingDefinition`: zusätzlich Footprint-Zellen, Bauradius, erzeugte/benötigte Leistung, erlaubte Produktion, Radar-/Abwehrregeln.
- `UHRFactionDefinition`: Fraktions-ID, Roster, Wirtschafts- und Baustil.
- `UHRDamageMatrix` / `FHRDamageRule`: exakte Schadens-/Rüstungspaare; nicht definierte Paare haben Faktor 1, explizit 0 bedeutet Immunität.
- `FHRUnitStats`, `FHRVeterancyLevel`: Grundwerte und kumulative XP-Schwellen mit separat ausgewerteten Boni.
- `UHRTempestFieldDefinitionAsset` / `FHRTempestFieldDefinition`: Kapazität, Anfangsmenge, Ernterate, Regrowth-Rate/-Verzögerung und optionale Saatreserve.
- `FHRTempestCellState`: stabile Zell-ID, aktuelle Menge und schwacher Reservierungsbesitzer.

Definitionen bleiben gemeinsam genutzte, unveränderliche Designwerte. Gesundheit, XP, Cargo, reservierte Zellen, Cooldowns und Queue-Fortschritt gehören auf Instanzen. Preise und Dauer werden bei Auftragsannahme gespeichert. Ein später veränderter Assetpreis darf die Rückerstattung nicht ändern.

Native Tags in `HRRTSTags.h/.cpp` gehören HeroHour: `HeroHour.Faction.*`, `Production.Buildings/Defense/Infantry/Vehicles/Specialists`, `Tech.Factory`, `Resource.Tempest`, `Damage.Kinetic/Explosive/Tempest`, `Armor.Infantry/Vehicle/Structure`, `Status.LowPower/PoweredDown/Suppressed`, `Ability.Smoke/Grenade/Deploy`. Sie identifizieren Regeln; ein Tag allein implementiert keine Fähigkeit. [Epic dokumentiert](https://dev.epicgames.com/documentation/en-us/unreal-engine/using-gameplay-tags-in-unreal-engine) native Tags und die erforderliche `GameplayTags`-Modulabhängigkeit. GAS wird für M1/M2 nicht benötigt.

## 4. Kernmechaniken

### Harvester und Nachwachsen

Zustände: `Idle → SeekField → MovingToCell → Harvesting → Returning → WaitingForDock → Unloading`. `Manual` unterbricht die Automatik; nach Erreichen des Befehlsziels setzt sie fort. Reservierungen werden bei Tod, manueller Abfahrt, erschöpftem Ziel, unpassierbarer Route und Neustart freigegeben. Eine Raffinerie reserviert Dockplätze getrennt von den Feldzellen.

Ernte pro Simulationsschritt: `min(Rate × dt, freie Cargo-Kapazität, entnehmbare Zellmenge)`. Entnahme und Cargo-Zuwachs sind eine Transaktion. Credits entstehen erst beim Entladen. Nachwachsen beginnt nach einer definierten Ruhezeit, endet an der Kapazität und läuft ausschließlich in der Simulationsautorität. Die Regeln für vollständig erschöpfte Zellen und Saatreserve werden vor M1-Abnahme festgelegt. Erst eine funktionierende Raffinerie mit gültigem Dock und passendem Team darf liefern.

**Aktiver geprüfter Stand 17:54:** `FHRResourceField` besitzt stabile Zellindizes, schwache exklusive Actor-Eigentümer, begrenzte Entnahme und verzögertes Nachwachsen. Optionale monotone Operations-IDs verhindern erneute Entnahme. `Advance` läuft einmal pro Economy-Schritt. Vorläufig eine Zelle pro Punkt, 4.000/4.000/6.000 Gesamtbestand, 20/s; Raten kommen aus dem gespeicherten Feldasset. `-PrototypeRegrowingFields` aktiviert 2/s nach 10s, SeedFloor 0; ohne Flag kein Wachstum. Cargo/Wallet sind double. Dockzustand, fraktionale echte Lieferung mit 1e−8, Depletion-Events, Primitive-Daten und Regrowth-Nullbestand-Nav/Bau sind in Economy 70 geprüft. Windows-Paket 17:55: Combat 17, Economy 70, Navigation 10, Match 3/3, UI 15, Architecture 41, Resource 18 PASS. Logs stehen in WORKLOG/VERIFIED_BUILD.

M1 bleibt offen: `Remaining` schreibt als Kompatibilitäts-Restore noch in den Kern; mehrere räumliche Zellen, explizite Harvester-Zustandsmaschine, exklusive Dockplätze/Entladezeit und größere Mehrsammlerabnahme müssen folgen. Die 17 Cluster des neuen Meshs sind keine 17 unabhängigen Zellen. Gespeicherte Daten und erste echte FieldActivity-Darstellung sind inzwischen vorhanden. Ein vorgeschlagener 16-Zellen-Schnitt teilt den bisherigen Feldbestand durch 16.

Die konkret behandelten Seed-/Ownership-/Depotrouten-/Float-/Visual-Fälle sind mit gezielten Live-Gates behoben. Der ursprüngliche I1–I8-Review bleibt ein größerer Prüfvertrag; nicht jeden künstlich korrumpierten Zustand als bereits getestet ausgeben. Regrowth behält eine permanente strukturelle Sperre, FiniteSupply gibt sie frei; tatsächliche Actor-Sichtbarkeit und einmalige Events werden geprüft. Das eigene Patch ist importiert, seine Emission liest FieldActivity. Die reale RHI-Ansicht 17:42 wurde betrachtet; neue Paket-Captures laufen getrennt. Ein Renderbild belegt kein natives Klickverhalten oder Originalparität.

### Stromnetz

`CurrentPower = Summe betriebsbereiter Erzeuger`; `RequiredPower = Summe aktiver Verbraucher`. Fertigstellung, Zerstörung, Verkauf, manuelles Abschalten und Anschlussänderungen lösen Neuberechnung aus. Baustellen liefern keinen Strom. Ein Verbraucher, der durch Strommangel abgeschaltet wird, bleibt in der Nachfrage enthalten; sonst würde das Netz zwischen Ein/Aus pendeln. Manuelles Abschalten darf Nachfrage entfernen.

Im getesteten Beispiel gilt die **eigene Prototypregel**: genug Leistung → Rate 1; positive Unterversorgung → Rate 0,5; keine Leistung bei positiver Nachfrage → Rate 0. Radar und energieabhängige Abwehr laufen nur mit ausreichender Leistung. Strommangel hält bezahlte Aufträge und Fortschritt fest. Diese Faktoren sind keine aus Tempest rekonstruierten Werte. Bauverbindung und Stromverbindung sind getrennte Regeln; der Prototyp beginnt mit einem Team-Strombudget und benötigt keine simulierten Stromkabel.

### Sidebar-Queue: vorhandenes Header-/Source-Beispiel

`HRProductionQueue.h/.cpp` implementiert `FHRProductionLane` und `FHRPowerGridState`. Ein UObject-Teamcomponent besitzt die Lanes als `UPROPERTY`, damit Definitionen GC-sicher referenziert bleiben. Der autoritative Command-Service prüft davor Fraktionsroster, fertige lebende Produzenten, Tech und Befehlsrechte. Der HUD-Click sendet nur die Anfrage.

```cpp
FHRProductionLane Vehicles;
Vehicles.Initialize(FactionId, HRRTSTags::Production_Vehicles);
FGuid Order;
if (Vehicles.TryEnqueue(UnitDefinition, CompletedTech, TeamCredits, Order))
{
    // Der gespeicherte Preis wurde genau einmal abgebucht.
}

Vehicles.Advance(SimulationDelta, Power.GetProductionRate());
if (const FHRProductionOrder* Ready = Vehicles.PeekReady())
{
    const FGuid Id = Ready->OrderId; // Keine Zeiger über Mutationen behalten.
    if (TrySpawnAtValidatedExit(Ready->Definition))
        Vehicles.CommitSpawn(Id);
    // Ein blockierter Ausgang hält den fertigen, bezahlten Auftrag fest.
}
Vehicles.Cancel(Order, TeamCredits); // Einmalige Erstattung des Preis-Snapshots.
```

`TrySpawnAtValidatedExit` ist hier ein Integrationspunkt, keine bereits vorhandene neue Funktion. Das vollständige Queue-Beispiel kompiliert. Es ist noch nicht mit dem Live-Match verbunden. Zunächst eine aktive Lane pro Produktionstyp; zusätzliche Fabriken können später einen ausdrücklich definierten Bonus oder parallele Unterlanes liefern. Anzeige aller Aufträge ist keine zusätzliche Warteschlange. Vorläufige Rategrenze des Beispiels: 1.

Vor M2 muss der Walletvertrag vereinheitlicht werden: das Queue-Beispiel erwartet `int64& TeamCredits`, Live-Wirtschaft führt inzwischen double für fraktionale Erträge. Der Adapter darf Bruchteile nicht still abschneiden oder eine zweite Geldquelle führen. Eine gemeinsame feste Schrittweite oder durchgängige präzise Buchung ist vorher festzulegen.

## 5. Priorisierte Meilensteine

| Meilenstein | Implementierungsreihenfolge | Verbindliche Abnahme |
|---|---|---|
| **M1: Daten und Wirtschaft** | Definitionen an echte Actors/HUD binden; Credits-Wallet; Feldzellen; Harvester-Zustände; erreichbare Pickup-/Dockreservierung; Regrowth | Vier Sammler verdienen reales Geld ohne Doppelentnahme; zerstörte/blockierte Raffinerie erholt sich; Menge+Cargo+Lieferung sind bilanziert; Neustart und Cook laden die Definitionen |
| **M2: Basis und Information** | Kraftwerk/Leistung; Queue-Service; Konzept-Sidebar; verbundenes Baugrid und gedrehte Footprints; Sicht-/Erkundungsmaske | Zwei Kategorien produzieren unabhängig; Low Power wirkt wirklich; Vorschau und endgültige Belegung stimmen überein; versteckte Gegner sind für Auswahl, Zielsuche und Minimap unsichtbar |
| **M3: Kampf und Micro** | Drei Schadensarten und Rüstungspaare; schnelle TTK-Balance; Infantry; Deckung/Suppression; drei Veteranenstufen; Rauch, Granate, Deployment | Unabhängige Schadenstabelle; Cooldowns/Kosten erst bei definiertem Commit; Suppression erholt sich; Rangbonus gilt genau einmal; keine Angriffe durch Sicht-/Stromregeln hindurch |
| **M4: Fraktionen und Spielrunde** | Zwei eigene asymmetrische Roster; je zwei Doktrinen/Commander-Powers; Bot mit denselben Regeln; VFX/Animation/Audio; Balance und Performance | Jede Fraktion schafft sammeln→bauen→versorgen→produzieren→aufklären→kämpfen→Sieg/Niederlage→Neustart; 20 Minuten verpackt ohne festhängende Logistik; gemessene Framezeiten und Speicher |

Gemeinsame Basis zuerst, asymmetrische Spezialregeln danach. Aegis als Arbeitsname: robustere Panzerung, Aufklärung, defensiver Rauch. Ember: schnellere Einheiten, Flammen-/Energieangriffe, mehr Positionswechsel. Unterschiede betreffen später Kosten, Lebenspunkte, Reichweite, Tempo, Fähigkeiten und Wirtschaft. Namen und Werte sind eigene Arbeitsentscheidungen, keine aus dem Original extrahierten Daten.

Erster Umfang: eine CityHero-Karte, zwei Fraktionen, je Sammler und drei Kampfeinheitentypen; HQ, Kraftwerk, Raffinerie, Kaserne, Fabrik, Radar und eine Abwehr. Je Fraktion zwei Doktrinen und wenige aktive Fähigkeiten. Kampagne, dritte Fraktion, Luft-/Marine-Roster, Multiplayer, große Techbäume und alle Original-Spezialfälle folgen erst nach M4. Assets müssen zunächst genau diesen Umfang abdecken: erkennbare Meshes, Icons, Bau-/Schuss-/Tod-Effekte, Sammlerentladung und grundlegende Animationen.

## 6. Belege und Abnahme nach awesome-rea

[awesome-rea](https://github.com/yibie/awesome-rea#inclusion-criteria) verlangt einen benannten Verifikationstest. Bytevergleich ist typisch für Matching-Decompilation; Recompilation/Ports können auch Verhaltens- und Kompatibilitätstests verwenden. Das ist keine pauschale ROM-Bytepflicht für alle Einträge.

Für uns werden vier Aussagen getrennt protokolliert: (1) welches Originalartefakt durch SHA-256 identifiziert wurde; (2) welche konkrete Funktion unabhängig bytegenau rekonstruiert wurde; (3) welcher feste Ablauf in Original und HeroHour verhaltensgleich war; (4) welche eigene HeroHour-Regel bestanden hat. Aktuell gibt es keinen Nachweis von (2) oder vollständiger (3).

Referenzszenarien: feste Fraktion/Karte/Startcredits, Cheat-Einstellungen aus, ein Sammlerzyklus, Kraftwerkverlust und Wiederherstellung, Bauplatzgrenze, zwei Produktionstypen, Auftrag abbrechen, Ausgang blockieren und freigeben, Sichtkontakt verlieren. Pro Versuch Uhrzeit, Spielversion/Dateihash, Startzustand, Aktion, Ergebnis und Messunsicherheit sichern. Die momentan aktiv vom Nutzer gespielte Tempest-Partie wird nicht durch Agenten gesteuert. Funktionale HeroHour-Tests beweisen keine Tempest-Parität.

Für den nächsten Schritt sind Projekt, Unreal, Referenzinstallation und Konzepte vorhanden. Externe Assetkäufe oder TRELLIS sind keine Voraussetzung. Der Engpass ist die Integration der Systeme und eine konkrete visuelle/spielerische Abnahme.

## Geprüfter Nachtrag22:35: Docking integriert

Acht PaketsuitenPASS, Economy80/Docking28; Logs in VERIFIED_BUILD. Eigene FIFO je Raffinerie, ein exklusiver Dockslot,40/s Entladung, echte Ankunft, unterbrechbare Transfers mit Restcargo-Erhalt und neueSammler-HUD-Zustände. Gemeinsame ULP-Schritte verhindern Credit/Cargo-Rundungsdifferenz; nicht buchbarer Rest wird zur nächsten Ernte mitgenommen. Shipping-REA belegte getrennte Queue-/Slot-/Deposit-Abfragen und HarvesterData, nicht diese konkrete Rate oder Slotzahl. M1 bleibt offen für räumliche Mehrzellenfelder, größere Mehrsammlerabnahme und gezielt pausiertes Entladen. Historische Abschnitte ohne Docking beziehen sich auf frühere Paketstände.
