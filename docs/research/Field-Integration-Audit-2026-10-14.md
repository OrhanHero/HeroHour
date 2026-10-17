# Live-Ressourcenfelder: unabhängiger Integrationsaudit

Stand: 14.10.2026, aktualisiert 17:55 Europe/Berlin nach tatsächlicher Paketabnahme. Aktives Modul: `HeroHourPrototype`. Gelesen wurden `PrototypeEconomy.cpp`, `PrototypeEconomyTypes.h`, `PrototypeGameMode.cpp/.h`, `PrototypeNavigation.cpp`, `BattlefieldLayout.h`, `RTS/HRResourceField.*`, die zugehörigen Kern-/Live-Tests und vorhandene Logs. Keine Unreal-Prozesse oder native Bedienung wurden durch diese Rolle ausgeführt; ausgeführt hat der Hauptagent, diese Rolle las die Ergebnisse unabhängig.

Der Auftrag begann als Read-only-Audit. Nach Meldung zweier konkreter Depotfehler autorisierte der Hauptagent eng begrenzte Änderungen: `FindDeliveryPoint`, der Returning-Branch von `TickCollector` und vier neue Economy-Gates. Später kamen auf ausdrücklichen Integrationsauftrag vier Live-Bilanz- und vier Visual-/Nav-/Baugates im Economy-Testtail hinzu. Nur diese C++-Abschnitte wurden durch diese Rolle geändert. Der Hauptagent bindet parallel gespeicherte Definitions-Assets an, stellt Cargo/Wallet auf double um und führt Builds/Tests aus. Dies ist eine Prüfung eigener HeroHour-Regeln, kein Nachweis von Tempest-Rising-Parität.

## Ergebnis

Die bisherige Feld-/Sammlerbrücke ist wesentlich belastbarer: Entnahme gehört dem reservierten Actor, der Sammler übernimmt den Rückgabewert, Feldzeit läuft nur in `TickEconomy`, geschützter Rest führt zur Rückfahrt und erreichbare Raffineriealternativen werden gesucht. Globale Freigabe und Eligibility-Abgleich verhindern im normalen Befehls-/Todes-/Wechselpfad mehrere gültige Feldansprüche desselben Sammlers. Nachwachsendes Supply hat nun eine dauerhafte strukturelle Nav-/Bauausschlussregel.

Zwei echte Depot-Kontrollflussfehler wurden korrigiert und ihre vier neuen Live-Gates bestanden. Die aufgedeckte Floatgrenze führte zur Double-Umstellung; vier fraktionale Live-Gates sichern tatsächliche Ernte, Rückfahrt und Zahlung. Weitere vier Gates prüfen echte Depletion-Events, Primitive-Daten und Nav-/Bauausschluss. **Alle 70 Economy-Gates bestehen nun im frisch gebauten Windows-Paket 17:54**, nicht nur im Editor. Vollständige Dockreservierung und räumliche Mehrzellenfelder fehlen weiterhin.

## 1. Anspruch und Freigabe

`ReleaseSalvageReservation` gibt den Actor in **allen** `SalvageNodes[].Field` frei, löst seinen Legacy-Zeiger und leert `HarvestNode`, `HarvestTimer` und Route-Check. Der normale Wechsel ruft dies vor einer neuen Reservierung auf. `TickCollector` bereinigt Feldansprüche mit dem Prädikat Alive, Collector-Rolle, kein manueller Befehl, erwarteter Node-Index und identischer Legacy-Besitzer. Damit kann derselbe Actor wegen seines einzelnen `HarvestNode` nicht in zwei Feldern gleichzeitig diese Eligibility erfüllen.

Die Ressourcenschicht erzwingt selbst nur einen Anspruch pro Actor **innerhalb eines Feldes**. Die globale Regel bleibt Verantwortung des Live-Adapters. Es wurde im aktuellen normalen Kontrollfluss kein zusätzlicher Doppelentnahmefehler gefunden. Eine künftige Mehrzellen-/Mehrfeld-Anbindung darf diese globale Freigabe nicht verlieren. Die bestehende Role-Change-Liveprüfung belegt einen Wechsel; eine ausdrücklich künstlich korrumpierte Dreifachansicht ist noch kein vorhandener Live-Gate.

## 2. Geschützter Rest, Nachwachsen und Blockflächen

Rückfahrt, Erschöpfung, Kandidatenauswahl und Anspruchsfreigabe verwenden `GetHarvestableAmount`, nicht nur den sichtbaren Gesamtbestand. Bei 50 Bestand / 20 Saat / Cargo0 entstehen 30 Cargo, Claim wird gelöst und die Liveprüfung liefert genau 30 Credits durch tatsächliche Bewegung.

`GetWorldObstacles` behält die Cache-Blockfläche bei `RegrowingField` auch bei Bestand0. `CanPlaceBuilding` behält Pickup-/Cache-Ausschluss ebenfalls unabhängig vom momentanen Bestand. Das verhindert den zuvor gefundenen normalen Ablauf: Fahrzeug fährt nach Erschöpfung in die leere Fläche, Nachwachsen stellt den Navkreis wieder her und sperrt den Startpunkt. Es verhindert auch Bau während Feldleere. Endliche Vorräte geben die Fläche weiterhin frei.

`TickEconomy` lässt einen nachwachsenden Cache derzeit bei 0 sichtbar. Das ist eine bewusste strukturelle Zwischenregel, keine fertig dargestellte Pflanzen-/Kristall-Wachstumsstufe. Die Darstellung sollte später den Bestand auslesen, während Navigation/Bauausschluss getrennt bleiben. Ein fehlender VisualActor nimmt den visuellen Navkreis weg, die Pickup-Bausperre bleibt; die neue Feldgeometrie muss deshalb mit dem tatsächlichen Actor und seiner Größe abgeglichen werden.

## 3. Korrigierte Depotfehler

### Zahlung verlor den zuvor geprüften Dockzustand

Vor der Korrektur berechnete die Auswahl `AlreadyDocked` mit einem freien Direktsegment zum Depot. Sie akzeptierte alternativ aber auch einen erreichbaren anderen Annäherungspunkt. Die nachfolgende Zahlung prüfte nur Entfernung≤Footprint+100. Damit führte ein blockiertes Direktsegment bei einer Entfernung unter 500cm und einem gültigen Umweg trotzdem unmittelbar zur Zahlung.

Jetzt wird der Zustand des ausgewählten Depots als `bSelectedAlreadyDocked` gespeichert. Nur dieser Zustand zahlt aus. Ein erreichbarer Umweg erhält Cargo und erzeugt einen echten Bewegungsauftrag. Sämtliche Ringkandidaten brauchen zusätzlich ein freies Entladesegment zum Depot, wobei dessen eigener Footprint ignoriert wird. Andernfalls könnte der Sammler am erreichbaren, aber abgeschirmten Ringpunkt dauerhaft warten.

Neue Gates:

- `blocked direct dock segment inside delivery radius preserves cargo and orders reachable detour`: Sammler 490cm vom Depot, 250cm-Cachekreis zwischen Sammler und Depot; direkte Entladung gesperrt, andere Seite erreichbar; keine sofortige Zahlung.
- `screened refinery pays once only after real movement reaches an unobstructed unloading side`: tatsächliche Unit-Ticks liefern erst nach Bewegung genau 100 Credits.

Die deterministische Cache-Fixture nutzt einen Actor als `VisualActor`-Blocker; dessen Modell ist für diese Nav-/Transaktionsprüfung unerheblich. Dies behauptet keine legal vom Spieler baubare Überlappung und keine Original-Dockregel.

### Freier alter Dockpunkt wurde ohne Route beibehalten

Vorher genügte für einen bereits gewählten Ringpunkt `IsWalkable(Destination)`. Wird die Route inzwischen abgeschnitten, konnte der nahe Depotkandidat deshalb eine tatsächlich erreichbare weiter entfernte Raffinerie verdrängen. Die normale Bewegung schlug dann wiederholt fehl.

Jetzt wird auch der alte Ringpunkt mit `FindWorldPath` erneut vom aktuellen Sammlerstandort geprüft. Ein freier Punkt jenseits einer abgeschnittenen Route zählt nicht als erreichbares Depot.

Neue Gates:

- `walkable cached dock with lost route cannot hide a reachable alternative refinery`: Zielpunkt östlich des Flusses bleibt frei, beide Brücken sind durch deterministische Struktur-Fixtures abgeschnitten; westliche Raffinerie muss gewählt werden.
- `cached dock route invalidation recovers through real alternate refinery delivery`: echte Bewegungs-Ticks liefern dort genau 100 Credits.

Auch diese Fixtures sind gezielte Nav-Anordnungen, keine behauptete zulässige Spielerplatzierung im Wasser. Die Funktionen bleiben ein Proximity-Entlademodell ohne exklusive Dockslots oder Entladezeit.

## 4. Aufgedeckte Floatbilanz und anschließende Umstellung

`FHRResourceField::Harvest` stellt eine repräsentierbare, durch Rate/Stock/Cargo begrenzte Entnahme sicher. Die eigene `many cargo and delivery cycles`-Kernprüfung bilanziert anschließend jedoch mit **double Cargo und Delivered**. Bei Beginn dieses Audits waren Live-Cargo, Credits und EnemyCredits **float**. Ein exakt repräsentierbarer Felddebit war deshalb nicht automatisch ein exakter Cargo- oder Wallet-Zuwachs.

Die float32-Operationen wurden unabhängig mit Python `struct.pack/unpack('<f')` reproduziert:

| Operation | Ergebnis |
|---|---|
| Feldrest `1.123456f` vollständig entnehmen und an `80.f` Cargo addieren | Vorherige Summe 81.12345600128174; nachher 81.12345886230469; Differenz +0.00000286102294921875. |
| `0.1f` Cargo an `800.f` Wallet auszahlen | Debit 0.10000000149011612; Walletdelta 0.0999755859375; Differenz −0.000024415552616119385. |

Dies sind arithmetische Reproduktionen der konkreten Live-Operationen, keine neu ausgeführte Unreal-Partie. Die finite 4k/6k-Basis mit 10er-Pulsen bleibt bei diesen normalen Werten exakt. Das ausdrücklich aktivierbare Nachwachsen erzeugt dagegen fraktionale Bestände; die bisherige Aussage „jede Live-Menge exakt erhalten“ ist dafür nicht abgesichert.

Der Hauptagent hat anschließend `APrototypeUnit::Cargo`, `Credits`, `EnemyCredits` und `Wallet` auf double umgestellt; der Editor kompiliert laut Integrationsmeldung. Die alten Floatbeispiele beschreiben den gefundenen Vorzustand, keinen weiter behaupteten Float-Walletfehler im aktuellen Source. Vier neue Economy-Gates prüfen Rest 1.123456f mit Cargo 80, Mengenbilanz mit Toleranz 1e−8 über echte Rückfahrt/Entladung, .1f-Payout an einem gültigen Dock und Verhinderung einer zweiten Auszahlung. Ein Double-Kerntest ersetzt diese Live-Gates nicht. Große Langzeit-/Mehrsammlerbilanz und festgelegte Preis-/Anzeige-Rundung bleiben zusätzliche M1-Arbeit.

## 5. Neue visuelle Zustands-Gates

Der Hauptagent hat `UpdateResourceVisuals`, `Node.bVisualDepleted` und `OnResourceDepletionChanged(Index,bDepleted)` integriert. Der Zustand richtet sich nach entnehmbarem Bestand. Ein echter `AStaticMeshActor` erhält CustomPrimitiveDataFloat0 als Activity 0…1; der eigene TempestEnergy-Shader verwendet dies für die Emission. Diese Implementierung wurde durch die belegte REA-Deklaration `SetResourceDepleted` angeregt; sie ist ein eigener Entwurf.

Vier neue Testfälle sind im Economy-Testtail:

1. Bestand 5/Saat 5 → entnehmbar 0: genau ein Depleted-Event trotz zweimaligem Update, Actor bleibt bei Regrowth sichtbar, sein tatsächlicher Primitive-Dateneintrag 0 ist 0.
2. Bestand anschließend 0: weiterhin keine zweite Depletion-Meldung; Actor sichtbar, Navkreis und Bauausschluss bleiben bestehen. Der Sammler wird vorher entfernt, damit der Bauvalidator nicht wegen eines anderen Blockers ablehnt.
3. `TickEconomy(6)` bei Regrowth 2/s: Bestand 12/entnehmbar 7, genau ein nicht-erschöpftes Event trotz weiteren Updates und Primitive-Activity 7/45.
4. Derselbe Ort als `FiniteSupply` mit 0: tatsächlicher Actor verborgen, Navkreis frei und derselbe Fabrikbauplatz zulässig.

Die Tests lesen `GetCustomPrimitiveData().Data` des tatsächlichen Meshcomponents; sie sind kein GPU-Pixel-/Shaderaussehen-Nachweis. Sie sichern jedoch die Zustandsdaten, aus denen der Shader seine Darstellung erhält. Ereignishandler wird vor Fixture-Reset entfernt. Alle vier Gates bestehen im Editor 17:51 und im Paket 17:55.

## 6. Vorhandene und fehlende Gates

| Gate | Status dieses Audits |
|---|---|
| Feldkern, Actor-Ownership, Duplikat-IDs, Saat, Wachstumstimer, Quantisierung | 18 Editor-Prüfungen PASS: `ResourceTest-20261014-171459-821.log`. |
| Bestehende Wirtschaft plus Saat-/Wachstums-/Rollenwechsel-/Raffineriealternative | 58 Editor-Prüfungen PASS: `EconomyTest-20261014-171459-829.log`; dieser Stand liegt **vor** den vier Depot-Gates dieses Durchlaufs. |
| HUD mit tatsächlicher Grafik | 15 Editor-Prüfungen PASS: `UITest-20261014-171459-873.log`; kein transaktionaler Wirtschaftsbeweis. |
| Vier Depot-Abnahmen dieses Durchlaufs |Editor 17:36 PASS in Economy 62; anschließend im frisch gebauten Paket 17:55 ebenfalls PASS, enthalten in Economy 70. Die Lognamen stehen in Abschnitt 7. |
| Gespeicherte Definitionen und aktuelle übrige Regeln |Architecture 41, Combat 17 und Match 3/3 Editor 17:36 PASS; passende Logs `ArchitectureTest-20261014-173603-101.log`, `CombatTest-20261014-173603-110.log`, `MatchTest-20261014-173603-123.log`. |
| Permanenter Regrowth-Nav-/Bauausschluss und Depletion-Ereignisse |Vier tatsächliche Actor-/Primitive-/Nav-/Baugates im Editor 17:51 und Paket 17:55 PASS, enthalten in Economy 70. |
| Fraktionale Live-Feld+Cargo+Wallet-Bilanz |Double-Umstellung und vier echte Live-Gates mit 1e−8 im Editor 17:51 und Paket 17:55 PASS, enthalten in Economy 70. |
| Gespeicherte Definitions-uassets |Acht `Content/HERO/DataV3/DA_HR_*.uasset` gekocht und im tatsächlichen Paket geladen; Architecture 41 inkl. Datenmutation/Restore PASS. |
| Mehrere räumliche Feldzellen / vier Sammler / Dockreservierung / Entladezeit |Weiterhin vollständiger M1-Umfang, durch Ein-Zellen-Adapter nicht erfüllt. |
| Tempest Shipping-Verhaltensparität |Nicht belegt. REA-Originalbyteanker sind kein Recompile-/Live-Paritätsnachweis. |

## 7. Abschließende echte Paketabnahme

`Packaging-20261014-175357-811.log`: BUILD SUCCESSFUL, AutomationTool ExitCode 0. `Builds/Windows/Build-Info.json`: CreatedUtc `2026-10-14T15:54:41.7217707Z` = 17:54 Berlin. Die vorherige 16:07-Basis ist historisch. Keine Materialkompilierungsfehler im Packaging-Markercheck. Alle folgenden Logs stammen aus diesem tatsächlichen neuen Paket:

| Suite | Ergebnis | Log |
|---|---|---|
| Combat | 17 PASS |`CombatTest-20261014-175530-614.log` |
| Economy | 70 PASS inkl. vier Depot-, vier fraktionaler Bilanz- und vier Visual-Gates |`EconomyTest-20261014-175530-623.log` |
| Navigation | 9 Pfadprüfungen + 1 echte Unit-Fahrt = 10 PASS |`NavigationTest-20261014-175530-614.log` |
| Match | 3/3 Siege, sichere Pfade, 1.300 reale Lieferung je Runde |`MatchTest-20261014-175530-633.log` |
| UI | 15 PASS mit Grafik |`UITest-20261014-175530-632.log` |
| Architecture | 41 PASS |`ArchitectureTest-20261014-175530-641.log` |
| Resource | 18 PASS |`ResourceTest-20261014-175530-663.log` |

Die Zeilen/Marker/Anzahlen wurden unabhängig gelesen. Sie bestätigen diesen eigenen begrenzten Prototypstand; kein vollständiges M1, keine Original-Byte-/Verhaltensparität. Neue RHI-Captures und native Bedienung bleiben gesonderte Nachweisarten. Die bestätigte native Bedienungsbeobachtung 17:07 betraf das ältere Paket und wird nicht auf diesen Build übertragen.

## Quellenstand dieser statischen Prüfung

SHA-256 um 17:38 **vor** den anschließend hinzugefügten acht Float-/Visual-Gates und Double-/Visual-Integration; kein veröffentlichter Commit:

- `PrototypeEconomy.cpp`: `91875c6ad25ee459fd38fd03f41aee9b3171c9aa287a5c66c9f032b5c3e1d0a7`
- `PrototypeNavigation.cpp`: `2f99ba05ae09ae276d0f3f19102acfacedd71add121d9630eaa7875749f73def`
- `RTS/HRResourceField.cpp`: `facaebade82087459ad43d1871072397d25f8846fae2a4e9e9189b261f2946c2`
- `RTS/HRResourceFieldTests.cpp`: `9080463173e4bf08f99faacd1e916f4cc6839d7ed9139444d61266e47a30e6a6`
