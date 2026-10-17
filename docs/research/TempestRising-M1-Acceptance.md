# M1: Ressourcenfelder, Sammler und nachvollziehbare Abnahme

Stand: 14. Oktober 2026. Dieses Dokument verbindet die gezielte Tempest-Untersuchung mit der eigenen HeroHour-Integration. Es ist ein Prüfvertrag; ein formulierter Testfall ist noch kein bestandener Test. Aktuelle Build-Ergebnisse stehen in [VERIFIED_BUILD.md](../VERIFIED_BUILD.md), die Herkunft der Referenzbelege in [TempestRising-Systems-Evidence.md](TempestRising-Systems-Evidence.md).

## Was REA tatsächlich liefert

REA bedeutet **Reverse Engineer Anything**. Es untersucht Artefakte und hält Belege sowie offene Fragen fest. Es ist kein RTS-Laufzeitframework und bringt weder Harvester noch Unreal-Gameplay-Tags oder Fog of War mit. HeroHour konsumiert seine eigenen C++-Systeme; REA bleibt ein Entwicklungswerkzeug.

Die 21 behaltenen Records der jüngsten Prüfung beziehen sich auf zwei Editor-DLLs. Sie enthalten zwei echte untersuchte Ressourcen-Funktionskörper, zehn kurze Ressourcen-/Power-Platzhalter und einen UObject-Konstruktor. Daraus entsteht kein rekonstruierter Shipping-Sammler oder Produktionsalgorithmus.

| Aussage | Konkreter Beleg | Tragfähige HeroHour-Ableitung | Grenze |
|---|---|---|---|
| Feldbestand lässt sich über Zelleinträge summieren. | `GetResourceLeft`, 200 Bytes, `ev_9f4e0b2fa3db950dee672d90963c13f61c04751b833e3b985c9c70f4d3a841a7`. | Stabile Zell-IDs und eine autoritative Gesamtmenge. | Genaue Bedeutung der Wachstums-/Tabellenwerte unbekannt. |
| Eine Zell-ID kann in der Menge beanspruchter IDs vorkommen. | `IsFieldTileIdxClaimed`, 102 Bytes, `ev_dadc52f91f325b7310505ecd65eeb8b8a391a6c512af25aa046c11657c32a4af`. | Zellreservierung als eigener Zustand; konkrete Schreibpolitik separat testen. | Kein nativer Beweis für Claim-Eigentümer, Freigabe oder Synchronisation. |
| Die API kennt erreichbare freie Kacheln und `INDEX_NONE`. | Mitgeliefertes `api.pdf`, S. 181–182. | Jeder Kandidat muss erreichbar sein; fehlendes Ziel ist ein normaler Ausgang. | Deklaration ist kein ausgeführter Shipping-Ablauf. |
| Ressourcen besitzen Build-/Effects-/Gameplay-Konfiguration und Wachstumsstadien. | `api.pdf`, S. 175–178. | Definition, Bestand und Darstellung getrennt halten. | Regenerationsrate, exakte Saat-/Kraterregel und Tickfrequenz nicht rekonstruiert. |
| Die API trennt sichtbare und tatsächliche Ressourcenmenge. | `api.pdf`, S. 180–181. | M1 darf Gesamtbestand intern nutzen; M2 muss die Spieleranzeige durch Sichtwissen filtern. | Der aktuelle Prototyp besitzt noch kein echtes Sichtwissen. |
| Zahlung, Queue und Refund haben deklarierte Lebenszyklusmöglichkeiten. | GameplayCommands-Deklarationen und `api.pdf`, S. 6–11. | Zahlungspunkt, Preis-Snapshot und Stornoregel ausdrücklich festlegen. | Keine ursprüngliche Queue-Transaktion rekonstruiert. |

Die Originalbytes der beiden echten Editor-Funktionskörper wurden unabhängig gegen die unveränderten Dateibereiche verglichen. Das bestätigt die Bindung des Analysats an diese Dateien. Es bestätigt weder einen neu kompilierten Nachbau noch Verhalten der Shipping-EXE. Die Funktionen `ClaimFieldTileIdx`, `UnclaimFieldTile`, `HarvestTile` und `GetPowerStatus` sind im untersuchten Editor kurze konstante Platzhalter. Ihr Name darf nicht als Implementierungsbeleg benutzt werden.

Referenz-Shipping: `F:/Tempest-Rising-SteamRIP.com/Tempest Rising/Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe`, SHA-256 `c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56`. Die öffentliche Versionszuordnung dieser lokalen Installation bleibt unbestätigt. Die vorhandene Spielaufnahme hat unbegrenzte Credits; sie taugt für sichtbare UI/Umgebung, nicht zur Messung von Ernteerlös oder Erstattung. In diesem Durchlauf wurde Tempest weder gestartet noch bedient.

## Eigene Regeln und offene Originalwerte

`RTS/HRResourceField.h/.cpp` ist ein eigener Value-Type für Bestand, Reservierungen und Nachwachsen. `FHRTempestFieldDefinition` ist eine Konfiguration **pro Zelle**, keine Feld-Gesamtkapazität. Der Live-Adapter muss diese Einheit konsequent nutzen.

| Größe | Bestehende HeroHour-Basis / M1-Vertrag | Tempest-Status |
|---|---|---|
| Startgeld | 800 Credits pro Team im bisherigen Match. | Nicht aus dem Original abgeleitet. |
| Feldbestand | Seitenfelder je 4.000, Zentrum 6.000 insgesamt. Bei 16 Zellen: je 250 bzw. 375 pro Zelle. | Ursprüngliche Zell- und Feldkapazitäten unbekannt. |
| Sammlerladung | Bisher 100 Ressourceneinheiten pro Sammler. | Ursprüngliche Capacity unbekannt. |
| Ernterate | Bisher 10 je 0,5 Simulationssekunden = 20/s; Änderung ist eine eigene Balanceentscheidung. Der Schema-Default 10/s übernimmt diese Rate nicht automatisch. | Ursprüngliche Ernte-/Burst-/Upgrade-Raten unbekannt. |
| Auszahlung | Bisher 1 Ressourceneinheit = 1 Credit, erst bei tatsächlicher Ablieferung. | Wertumrechnung und ursprünglicher Commit-Zeitpunkt unbekannt. |
| Regeneration | Nur `RegrowingField`, mit explizit gesetzter Rate und Verzögerung. `FiniteSupply` regeneriert nie. | Rate, Verzögerung, Krater-/Saatbedingung und Schwierigkeitseinfluss offen. |
| Saatreserve | `SeedFloor` ist pro regenerierender Zelle geschützt. Entnehmbar ist `max(Amount − SeedFloor, 0)`. | Kein Nachweis, dass das Original dieselbe Regel nutzt. |
| Storno | Bestehender Prototyp erstattet den gespeicherten bezahlten Betrag genau einmal. | Aktiver/unge-starteter Auftrag und Refundprozentsätze nicht gemessen. |
| Strommangel | Eigenes Queue-Beispiel: volle Versorgung Rate1, positive Unterversorgung0,5, bei Bedarf ohne Erzeugung0. | Ursprüngliche Faktoren/Prioritäten unbekannt; Live-Anbindung folgt M2. |

Regrowth-Werte müssen mit den gespeicherten Definitions-Assets und dem getesteten Build dokumentiert werden. Ein CPP-Default oder die bloße Existenz eines `UPrimaryDataAsset` belegt keine Live-Anbindung. Ein neues Feld mit 16×4.000 Kapazität würde den Seitenbestand unbeabsichtigt vervielfachen. Ein regenerierendes Feld mit geschütztem Rest ist erschöpft, sobald **entnehmbarer** Bestand null ist, auch wenn sichtbare Pflanzen und Gesamtmenge größer null bleiben.

## M1-Integrationspflichten

1. **Ein Bestand als Quelle:** Der Sammler addiert ausschließlich die von `Harvest` tatsächlich zurückgegebene Menge. Eine alte `Remaining`-Variable darf nicht parallel unabhängig abbuchen. HUD und Feldvisuals lesen denselben Zustand. Ein Kompatibilitätsadapter darf Spiegelwerte ableiten, aber sie nicht zu einer zweiten Wahrheit machen.
2. **Exklusive Eigentümer:** Höchstens ein lebender, automatischer Sammler pro Zelle; höchstens ein Feld-Claim pro Sammler über **alle** Felder. `FHRResourceField::TryReserve` erzwingt bisher nur die Eindeutigkeit innerhalb eines Feldes. Das Spielmodell muss vor Feldwechsel alle alten Claims lösen.
3. **Ausdrückliche Freigaben:** Tod, manueller Befehl, volle Ladung, entnehmbarer Bestand null, zerstörtes Feld und unpassierbare Zielroute lösen den Feld-Claim. Ein noch gültiger Actor mit Gesundheit0 ist durch `IsValid` allein nicht als tot erkennbar; der Adapter übergibt einen Spielzustandsvalidator oder ruft `Release` auf.
4. **Teilcargo fährt zurück:** `Cargo > 0 && GetHarvestableAmount(Cell) == 0` führt zur Rückfahrt. Die alte Prüfung `Remaining <= 0` würde bei `SeedFloor > 0` einen Sammler am geschützten Rest festhalten.
5. **Erreichbare Raffinerie:** Nur lebende, fertige, gleichteameigene Raffinerien mit gültiger erreichbarer Annäherung dürfen liefern. Ist die nächstgelegene unpassierbar, wird eine erreichbare Alternative gewählt. Gibt es keine, bleibt Cargo erhalten; der Sammler versucht nach Zustandsänderung erneut. Reservierte Dockplätze sind ein eigenes System und nicht durch einen Feld-Claim abgedeckt.
6. **Auszahlung einmal:** Wallet erhöht sich um tatsächlich entladenes Cargo unter der eigenen Umrechnung; Cargo sinkt in derselben Simulationstransaktion. Kein Einkommen beim Ernten, Annähern, Warten, Tick wiederholen oder zerstörter/feindlicher Raffinerie.
7. **Simulation bestimmt Wachstum:** `Advance` genau einmal pro Simulationsschritt und Feld; Pause und Matchende stoppen Simulation. Nachwachsen erst für den Anteil einer Zeitspanne nach der Verzögerung, nie über Kapazität. Ein fehlgeschlagener Null-Ernteversuch setzt den Wachstumstimer nicht zurück.
8. **Darstellung und Navigation passen zusammen:** Ein Pflanzen-/Ressourcenfeld ersetzt die SupplyCache-Geometrie und deren Hindernisregel gemeinsam. Nur alte Crates zu verstecken lässt bei weiterhin positiven Salvage-Spiegelwerten unsichtbare Navigationshindernisse übrig. Ein Zellstandort muss für seinen tatsächlichen Sammler erreichbar bleiben; decorative Pflanzen dürfen keine eigene widersprüchliche Physikkollision hinzufügen.
9. **Neustart und Cook:** Reset entfernt Reservierungen/Timer/Bestand der alten Runde, bindet die gespeicherten Definitionen erneut und erzeugt genau den festgelegten Anfangsbestand. Das verpackte Spiel muss diese Assets laden; ein Editor-Test allein reicht nicht.

Der `OperationId`-Schutz des Value-Type ist eine eigene Idempotenzhilfe: akzeptierte nichtnull IDs müssen pro Zelle steigen. Er ist kein Tempest- oder Netzwerk-Nachweis. Ein trusted Game-Thread-Pulse darf ID0 benutzen. Persistenz/Replication sind noch nicht daraus abgeleitet.

## HeroHour-Abnahme des integrierten M1

Die Tests müssen den echten Match-Adapter prüfen, zusätzlich zu den isolierten Feldtests. Keine Teleportation in eine Raffinerie als einziger Nachweis einer erfolgreichen Fahrtroute. Die Kernsuite darf für Randfälle gezielt Zustände setzen; wenigstens ein kompletter Zyklus pro Team muss real über die Navigation fahren.

| Fall | Kontrollierter Start und Aktion | Erforderliches Ergebnis |
|---|---|---|
| H1 Bilanz | Regrowth aus; ein Feld, ein Sammler, eine erreichbare Raffinerie. Ernte, echte Rückfahrt, zwei Lieferungen. | `FieldStart = FieldNow + CargoAll + DeliveredAll` innerhalb festgelegter Float-Toleranz; Credits vor Entladen unverändert. |
| H2 Globale Claims | Zwei Felder; vier Sammler, einer wechselt A→B, einer manuell weg, einer stirbt. | Kein doppelter Zellbesitz; ein Sammler besitzt nie Claims in A und B zugleich; Übernahme freigegebener Zellen ohne Doppelentnahme. |
| H3 Saat und Regrowth | Kapazität100, Start30, Saat10; entnehmbare20 vollständig ernten bei Cargo noch frei. | Rest10 geschützt; Cargo20 fährt ab. Nach festgelegter Verzögerung steigt Bestand mit eigener Rate, gedeckelt100; Nullversuche verlängern Verzögerung nicht. |
| H4 Ersatzraffinerie | Nähere Raffinerie unpassierbar, weiter entfernte fertig/erreichbar; danach beide zerstören und eine neu bauen. | Erst weiter entfernte Lieferung; danach Cargo erhalten und keine Credits; nach Neubau automatische reale Lieferung. |
| H5 Visual/Route | Neues Feld startet voll, erschöpft und wächst wieder. Alte SupplyCache-Position gezielt anfahren; Route und sichtbare Zellmenge prüfen. | Keine unsichtbaren alten Cache-Hindernisse; Darstellung entspricht Bestand; befahrbare Zellen bleiben erreichbar. |
| H6 Reset/Paket | Während Ernte bzw. Rückfahrt neu starten, später verpacktes Spiel ohne Editor öffnen. | Anfangsbestand/Cargo/Wallet/Claims korrekt; Definitions-uassets geladen; kein doppelter Feldtick oder versteckter Altbestand. |

Zusatzkontrollen des Value-Type: ungültiger CellId/Owner/NaN/dt darf keinen Bestand verändern; falscher Eigentümer darf nicht ernten oder fremden Claim lösen; finite Felder wachsen auch nach großer Zeitspanne nicht; Reset räumt schwache Eigentümer und Operation-IDs auf. Prüfungen mit `SeedFloor > 0`, mehreren Feldern und blockierter näherer Raffinerie sind zwingend, weil nur ein Feld ohne Reserve diese Fehler verdeckt.

Bilanz mit Regrowth: `Initial + Generated = Remaining + Cargo + Delivered + LostCargo`. `LostCargo` ist nur nötig, wenn ein beladener Sammler stirbt und die Designregel seine Ladung vernichtet. Ernte zählt nicht noch einmal als Ausgabe. Liefer-Credits werden getrennt mit der gespeicherten Umrechnung geprüft; Wallet enthält außerdem Käufe und Erstattungen. Toleranz vor Testbeginn definieren, beispielsweise0,01 Ressourceneinheiten für diese kleinen Bilanzfälle. Eine Toleranz darf keine doppelte Lieferung verdecken.

## Sechs festgelegte Referenzszenarien

Diese Szenarien werden **nicht** als bereits gemessen ausgegeben. Die Originalaktionen führt nur der für native UI zuständige Hauptagent oder der Nutzer aus. REA-Static-Ergebnisse und UI-Zeitreihen werden getrennt protokolliert.

Gemeinsames Protokoll: dieselbe Referenzfraktion, dieselbe Karte, feste Startcredits und Startarmee, feste Bot-/Schwierigkeitsregeln, keine unendlichen Credits und keine Cheat-/Debug-Boni. Vor dem ersten Versuch EXE-Hash, im Spiel angezeigte Version, alle Regeln und den vollständigen Startzustand sichern. Ein einmal gewählter Zustand bleibt für die Wiederholungen fest; Fraktionen nicht innerhalb einer Messreihe wechseln. Aktionen mit Simulations-/Spielzeit und Messunsicherheit aufzeichnen. Je Szenario drei unabhängige Neustarts. Gegenmessungen in HeroHour benutzen die eigene ausdrücklich dokumentierte Konfiguration.

| Referenzfall | Festgelegter Ablauf | Messpunkte und Vergleichsgrenze |
|---|---|---|
| R1 Erntezyklus | Eine erreichbare Raffinerie und ein leerer Sammler an einem Feld; drei vollständige Füll-/Lieferzyklen ohne andere Käufe. | Vor Ernte, bei voller Ladung, unmittelbar vor/nach Entladen: Uhr, Credits, Cargo soweit sichtbar, Feldzustand und Position. Nur sichtbare Cargoanzeigen quantifizieren; fehlende interne Werte bleiben unbekannt. |
| R2 Konkurrenz/Unterbrechung | Vier Sammler auf dasselbe Feld. Einen manuell abfahren lassen; im getrennten Neustart einen Sammler und danach die Raffinerie verlieren. | Ziel-/Wartepositionen und Zeit bis Wiederaufnahme. Sichtbare Bewegung kann Konfliktvermeidung belegen, aber nicht allein exklusiven internen Claim-Eigentümer oder Freigabe-Tick. |
| R3 Queue/Refund | Drei identische Einheiten bezahlen. Einen noch wartenden und im unabhängigen Neustart den aktiven Auftrag bei festgelegtem Fortschritt abbrechen. | Credits und Queue vor Auftrag, nach Auftrag, vor und nach Storno; Refund separat pro Fall. Die richtige Originalbedienung erst anhand UI bestimmen, nicht rechte Maustaste unterstellen. |
| R4 Ausgang | Fabrikausgang mit zulässigen Einheiten blockieren, Produktionsende abwarten, Blocker entfernen. | Queue/Fortschritt/Credits/Einheitenzahl und Spawn-/Rallypositionen vor Ende, während Blockade und nach Öffnen. Eine Animation allein beweist keinen erhaltenen bezahlten Auftrag. |
| R5 Strom | Dieselbe Einheit bei voller Leistung, Unterversorgung und manuellem Shutdown produzieren; Versorgung dann wiederherstellen. | Supply/Demand, Produktionsfortschritt im Sekundenraster, Radar und energieabhängige Abwehr. Faktoren erst aus Zeitreihen ableiten; Nullstrom und LowPower getrennt behandeln. |
| R6 Bau/Sicht | Dasselbe rechteckige Gebäude in zwei erlaubten Rotationen an verbundener Grenze, isolierter Grenze, Brücke und verborgenem Gebiet prüfen. Gegner danach aus Sicht verlieren. | Ghostfarbe, bestätigte Bauentscheidung, Größe, Credits, Auswahl/Sidebar/Minimap/Feuerverhalten. Bauradius, Erkundung und Radarwissen nicht aus einer einzelnen dunklen Fläche ableiten. |

Datensatz pro Versuch: `case_id`, `attempt`, `target_sha256`, `version_visible`, `faction`, `map`, `rules`, `start_state`, `actions`, `samples`, `result`, `unknowns`, `measurement_tolerance`, `screenshots_or_video`. Eine eigene HeroHour-PASS-Suite beweist die eigene Regel. Verhaltensparität darf erst für den tatsächlich gemessenen gemeinsamen Fall behauptet werden. Hash-/Byte-Identität des Originals wird weiterhin separat geführt.

## Status dieses Dokuments

Der Feldkern wurde statisch gelesen; die Live-Integration und ihre neuen Ergebnisse werden durch den Hauptagenten geprüft und in den Build-/Übergabedokumenten vermerkt. In diesem Dokument sind keine neuen PASS-Ergebnisse eingetragen. M1 bleibt offen, solange echte Fahrten, globale Reservierungsinvarianten, Saatrest-Rückfahrt, erreichbare Ersatzraffinerie, Cook-Laden und Zustandsvisuals nicht gemeinsam im aktiven Match funktionieren.
