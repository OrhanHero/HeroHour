# Raffinerie-Docking: Abnahme und reproduzierbare REA-Belege

Stand: 14. Oktober 2026. Diese Stufe ergänzt die [Shipping-Untersuchung](TempestRising-REA-Production-Docking.md) und die [M1-Abnahme](TempestRising-M1-Acceptance.md). Sie definiert die Abnahme der eigenen HeroHour-Implementierung. Die unten stehenden D1–D6 sind Abnahmeverträge; die tatsächlich belegte Abdeckung steht im Paketnachtrag. D5 bleibt für einen aktiv entladenden Sammler offen.

## Gesicherte Grundlage und eigene Entscheidungen

Die Original-Shipping-Bereiche unterscheiden `IsInQueue`, `IsDepositingResource` und `AreAllDepositSlotsEmpty`. Die HarvesterData-Metadaten deklarieren drei Int32-Felder für HarvestSpeed, DepositSpeed und MaxCapacity sowie einen Float für AddResourceFrequency. Separat wurden Credit-Mutationskörper mit Cap-/Overflow-Strecken quellverankert. Exakte Adressen, Hashes, Grenzen und REA-Evidence-IDs stehen im verlinkten Shipping-Bericht.

Nicht rekonstruiert sind ursprüngliche FIFO-Politik, Slotanzahl, konfigurierte Entladerate, Auszahlungsfrequenz, vollständige Dock-Schreib-/Freigabestrecken und Produktions-Bezahlzeitpunkt. HeroHours **FIFO, ein Entladeplatz pro Raffinerie, 40 Cargo pro Simulationssekunde und Double-Wallet** sind eigene Entwurfsentscheidungen. Sie dürfen nicht als gemessene Tempest-Rising-Werte beschrieben werden.

Der eigene Zyklus soll Feldreservierung, Rückfahrt, Dock-Warteschlange, exklusive Entladestufe und Abfahrt getrennt behandeln. Wartende Sammler behalten Cargo und erhalten keine Credits. Nur die tatsächlich gebuchte Entlademenge verändert beide Größen. Ein verlorenes oder abgebrochenes Dock gibt Claims frei; verbleibende Fracht bleibt bei einem lebenden Sammler.

## Abnahme-Gates D1–D6

| Gate | Reproduzierbare Ausgangslage und Aktion | Erwartung / Nachweis |
|---|---|---|
| D1: FIFO und exklusiver Slot | Zwei lebende Sammler A/B mit je100 Cargo melden sich in dieser Reihenfolge an einer fertigen eigenen Raffinerie. Anmeldung von A wiederholen. Beide in Dockreichweite bringen; zwei bzw. mehrere kleine Simulationsticks ausführen. | Keine doppelte Queue-Mitgliedschaft. Höchstens ein aktiver Slot. A entlädt zuerst; B wartet ohne Cargo-/Walletänderung. Nach Freigabe erhält B den Slot. Zusätzlich drei Sammler prüfen, damit ein zufälliges Wechseln zweier Einträge nicht als FIFO gilt. |
| D2: Nur tatsächliche Entladung bezahlt | Walletstart separat erfassen, aktive Produktion/passives Einkommen deaktivieren. Sammler mit100 Cargo erst wartend, dann aktiv testen. Tick0, Tick0,25s und ausreichend großer Tick. | Wartend und Tick0: keine Buchung. Bei40/s und0,25s höchstens10 Cargo/10 Credits. Große Zeitschritte buchen maximal die verbleibende Fracht. Wiederholung nach leerem Cargo zahlt nichts. Walletdelta entspricht exakt Cargoabnahme innerhalb der vereinbarten Double-Toleranz. |
| D3: Unterbrechung und Claims | Getrennte Fälle: manueller Bewegungsbefehl, Sammlertod, Rollen-/Ownership-Wechsel, Raffinerietod/-verlust; jeweils wartenden und aktiven Sammler unterbrechen. | Queue und Slot verlieren ungültige Claims spätestens im nächsten autoritativen Tick. Kein weiterer Unload/Pay. Lebende Sammler behalten die unverladene Fracht. Tote Sammler lösen keine spätere Gutschrift aus. Feld-Claim darf einen Dock-Claim nicht ersetzen. Nachfolger kann den frei gewordenen Slot erhalten. |
| D4: Recovery zu neuer Raffinerie | Sammler mit Restcargo nach Depotverlust ohne erreichbares Depot stehen lassen. Dann neues eigenes fertiges Depot bereitstellen; näher gelegenes, aber unerreichbares Depot ebenfalls anbieten. | Ohne erreichbares Depot weder Frachtverlust noch Gutschrift. Neue Route wählt tatsächlich erreichbares Depot und erreicht dessen Dock. Dort neuer Claim/Unload, danach echte Abfahrt und Rückkehr zum Feld. Veraltete Depot-/Routenzeiger werden nicht wiederverwendet. |
| D5: Pause und Reset | Während aktiver Entladung F10-Pause setzen; Weltzeit weiter verstreichen lassen. Wieder fortsetzen. Danach Matchreset bei besetztem Slot und nichtleerer Queue. | Während Pause: Cargo, Wallet und Queue-Zustand unverändert. Fortsetzen zahlt nur tatsächlich simulierte Entladezeit; keine Nachholzahlung für pausierte Wandzeit. Reset leert Claims/Queue/Slots und stellt Startzustand her; alte Actor-Handles können keinen neuen Matchauftrag beeinflussen. |
| D6: Bruchteil-Konservierung | Nichtganzzahliges Cargo und Wallet sowie nichtganzzahlige Rate/Dt testen: z.B.Cargo0,3, Rate0,1/s und mehrere Dt0,07s; getrennt große Wallets und kleine Teilbeträge. Gleiche Gesamtsimulationszeit mit unterschiedlichen Tickteilungen prüfen. | Jede Buchung erfüllt0≤ActualUnload≤CargoVorher und≤Rate×Dt. Kein negatives Cargo; kein Double-Booking; Summe gebuchter Credits entspricht Summe Cargoabnahme. Fehlergrenze ausdrücklich dokumentieren, z.B.1e−9 für kleine Double-Fixtures und ULP-bewusste Grenze bei großer Wallet. UI-Rundung ist kein Simulationsbeweis. Keine versteckte Konvertierung nach Int/Float darf Kleinstbeträge vernichten. |

Für jedes Gate Log mit Fixture, Startwerten, Tickfolge, tatsächlichem Unload, Cargo/Wallettdelta, aktiver Slot-ID und wartenden IDs behalten. Core-Fixtures prüfen Reservierungs-/Buchungsregeln; mindestens D1, D3, D4 und D5 benötigen zusätzlich den echten HeroHour-Weltpfad. Ein Core-PASS allein beweist weder Navigation noch GameMode-Pauseintegration.

## REA-Prüfer direkt aus einem frischen Checkout

Die vier versionierten Prüfer liegen unter `Scripts/research`. Alle schreiben nach `Saved/Research/rea-next`, das beim ersten Aufruf angelegt wird. Man benötigt keine bereits vorhandenen Pythonkopien oder JSON-Dateien unter Saved. Die Stufen haben Datenabhängigkeiten und laufen nacheinander:

```powershell
# Aus der HeroHour-Repo-Wurzel ausführen; Python3 genügt (nur Standardbibliothek).
$researchPython = 'C:/Users/Hero/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe'
$researchStages = @(
 'scan_shipping_command_metadata.py',
 'inspect_shipping_metadata_windows.py',
 'decode_shipping_struct_metadata.py',
 'disassemble_shipping_command_slices.py'
)
foreach ($researchStage in $researchStages) {
 & $researchPython (Join-Path 'Scripts/research' $researchStage)
 if ($LASTEXITCODE -ne 0) { throw "REA stage failed: $researchStage" }
}
```

Explizite lokale Voraussetzungen: die vom Benutzer genannte Shipping-EXE unter `F:/Tempest-Rising-SteamRIP.com/Tempest Rising/Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe`, der öffentliche UHT-Header unter `F:/UE/UE_5.8/Engine/Source/Runtime/CoreUObject/Public/UObject/UObjectGlobals.h` und bereits installiertes `C:/Program Files/Microsoft Visual Studio/18/Community/VC/Tools/Llvm/x64/bin/llvm-objdump.exe`. Auf anderem Rechner die Konstanten bewusst anpassen und die Zielidentität neu bestätigen; die Prüfer installieren nichts. Die Skripte führen das Originalspiel nicht aus.

Der Shipping-Hash muss `c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56` entsprechen. Jede Folgestufe prüft ihn erneut. Bei Hashwechsel abbrechen und feste Seeds/Offsets neu untersuchen; nicht stillschweigend alte Befunde übernehmen.

Am14.Oktober2026 wurden alle vier Repo-Aufrufe nach der Pfadkorrektur erfolgreich erneut ausgeführt. JSON-/ASM-Artefakte entstanden unter Saved. Abschlussmarker:

```text
SHIPPING_BYTE_ANCHORING_PASS stage=commands-docking slices=13 ranges=21 bytes=2637
```

Das PASS bestätigt Instruktionsbytes gegen Originaldateibereiche. Es ist **kein** rekonstruierter Tempest-Rebuild, keine Verhaltensparität, kein vollständiger Callergraph und kein bereits bestandener HeroHour-Docking-Test.

## Übergabe und nächste Abnahme

Core-Queue und Live-Sammlerzustände sind integriert und im Paket geprüft; siehe folgenden Nachtrag. Weitere Tests über `Scripts/tools/Test-HeroHour.ps1` ausführen und neue Abdeckung mit Testname und Log belegen. Editor- und Windows-Package-Nachweis getrennt führen; ein altes Package enthält die neue Stufe nicht automatisch.

M1 bleibt teilweise umgesetzt: die Docking-Abnahme ersetzt weder mehrere echte harvestbare Feldzellen, vollständige Feldvisual-Depletion, Grid/Power/Fog noch den gemessenen Original-Laufzeitvergleich. Original-Unloadraten und vollständiger Enter/Exit-Lifecycle bleiben gezielte offene REA-Fragen.


## Paketabnahme 22:33–22:35 Berlin

Das neue Paket enthält die eigene Docking-Stufe. `Packaging-20261014-223219-417.log` ist erfolgreich; CreatedUtc `2026-10-14T20:33:13.3558656Z` entspricht 22:33 Berlin. Die Docking- und Economy-Marker wurden direkt aus den tatsächlichen Paketlogs gelesen: **Docking 28 PASS**, **Economy 80 PASS**. Dies erweitert den älteren Stand mit sofortiger Lieferung; Paket 18:03 bleibt historisch.

| Gate | Belegte Abdeckung im neuen Paket | Verbleibende Grenze |
|---|---|---|
| D1 | Core: idempotente FIFO-Anmeldung, Head-Promotion und exklusive Slots. Live: zweiter Sammler wartet physisch außerhalb der Raffinerie; freigegebener Slot geht an Nachfolger. | Keine große Drei-/Mehrsammler-Stressabnahme und keine Original-FIFO-Parität. |
| D2 | Core und Live: eigene Rate 40/s, partielle tatsächliche Cargoabnahme und Walletbuchung; wartende Sammler zahlen nichts. Replay-ID und leeres Cargo erzeugen keine Doppelzahlung. | Originalrate und Original-Auszahlungstakt unbekannt. |
| D3 | Core: manueller/predicate-invalidierter Claim, Sammlertod und Raffinerieverlust. Live: erreichbarer manueller Abgang gibt Slot frei und behält Cargo; zerstörtes Depot stoppt weitere Buchungen. | Vollständige Live-Matrix für alle Rollen-/Ownership-Wechsel nicht separat abgearbeitet. |
| D4 | Live: verbliebenes Cargo wird zur erreichbaren Ersatzraffinerie umgeleitet und genau einmal geliefert; frühere Depot-/Routengates bleiben bestanden. | Keine umfassende wechselnde Depot-/Traffic-Stressreihe. |
| D5 | Core: Reset entfernt alte Claims und Replayhistorie. UI-Suite: Welt-Simulationsuhr bleibt bei echter Pause eingefroren. | **Offen:** gezielt aktiv entladenden Sammler pausieren und Cargo/Wallet/Slot vor und nach Pause vergleichen. Die Uhrprüfung ersetzt das nicht. |
| D6 | Sechs Core-Präzisionsgates: große Wallet, nichtbinäre Bruchteile, Restschutz, Nullwallet, Subnormal und MaxFinite. Live: kleiner als Wallet-ULP darstellbarer Rest bleibt Cargo, gibt Dock frei und setzt erreichbare Ernte fort. | Keine vollständige Teilungsinvarianz für beliebige Dt-Folgen behauptet; Rest kann bis zu späterer darstellbarer Buchung bestehen bleiben. |

Die gefundene Rundungslücke wurde vor diesem Paket behoben: tatsächlich dargestelltes Walletdelta und Cargodelta müssen dem gebuchten Transfer entsprechen, der das Zeitbudget nicht überschreitet. Nicht darstellbare Beträge werden nicht vernichtet oder als zusätzliche Credits erzeugt. Der Live-Wallet ist double; UI-Rundung bleibt reine Darstellung.

| Tatsächliche Paketsuite | Ergebnis | Log unter Saved/Logs |
|---|---|---|
| Docking | 28 PASS | `DockingTest-20261014-223509-021.log` |
| Economy | 80 PASS | `EconomyTest-20261014-223508-935.log` |
| UI | 15 PASS, einschließlich Simulationsuhr bei Pause | `UITest-20261014-223508-974.log` |
| Combat | 17 PASS | `CombatTest-20261014-223508-941.log` |
| Navigation | 10 PASS | `NavigationTest-20261014-223508-959.log` |
| Match | 3/3 PASS | `MatchTest-20261014-223508-959.log` |
| Architecture | 41 PASS | `ArchitectureTest-20261014-223508-983.log` |
| Resource | 18 PASS | `ResourceTest-20261014-223508-983.log` |

M1 bleibt teilweise umgesetzt: mehrere räumliche Erntezellen und vollständiger Originalvergleich fehlen weiterhin. Neue RHI-Captures waren bei diesem Nachtrag noch in Prüfung; hier wird kein neuer Bild-/Klicknachweis behauptet.
