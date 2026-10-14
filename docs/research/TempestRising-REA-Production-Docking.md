# Tempest Rising: Produktion, Credit-Transaktionen und Sammler-Docking

Stand: 14. Oktober 2026. Dedizierter REA-Pass parallel zur HeroHour-Implementierung. Er ergänzt [M1-Abnahme](TempestRising-M1-Acceptance.md) und [Feld-Pass2](TempestRising-REA-Next-Pass.md), ohne dessen Ergebnisse zu wiederholen. Originalspiel, Editor und Benutzeroberfläche wurden nicht ausgeführt oder bedient.

## Ergebnis

**Erstmals in diesen HeroHour-Untersuchungen wurden gezielte echte Shipping-Codebereiche geprüft**, ohne die bekannte zeitüberschreitende Gesamtanalyse zu wiederholen. Ein Refund-Wrapper führt zu einer Credit-Mutation mit Cap-/Overflow-Behandlung. Außerdem wurden die getrennten Refinery-Abfragen für wartende und entladende Sammler sowie leere Entladeplätze gefunden. Drei konkrete Datenstrukturen wurden über ihre generierten Property-Pointerketten dekodiert.

13 ausgewählte Codebereiche mit 21 genauen Teilbereichen und **2.637 Bytes** wurden mit dem installierten LLVM-Decoder geprüft: seine Instruktionsbytes stimmen vollständig mit den Originaldateibereichen überein. Das ist Source-Byte-Verankerung, kein neu kompilierter Nachbau und kein Laufzeitvergleich.

Ein vollständiger Produktions-Commit, Abbruchprozentsatz, Bezahlzeitpunkt, Original-Dockreservierungs-/Freigabealgorithmus und konfigurierte Harvest-/Unload-Raten sind weiterhin offen.

## Identität, Methode und Grenzen

| Ziel | SHA-256 | Verwendung |
|---|---|---|
| `Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe` | `c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56` | Begrenzte Metadaten-/Zeiger-/Instruktionsanalyse; 166.518.344 Bytes |
| `MapEditorProject/Binaries/Win64/UnrealEditor-Tempest.dll` | `2fff65ec4f3809e8f24010d5cbfa9ada2f2834b59c997569c219d6a494f0b4fa` | REA/Ghidra-Namens- und Metadatensuche mit bestehendem Snapshot |
| `MapEditorProject/Plugins/GameplayCommands/Binaries/Win64/UnrealEditor-GameplayCommands.dll` | `531f6e6474b259e935ac84c1edb170f58cfc181bd32415c87aa35f24be0fb952` | Gezielte Command-/Lifecycle-Suche |

Wurzel aller Ziele: `F:/Tempest-Rising-SteamRIP.com/Tempest Rising/`. Die Zuordnung dieser lokalen Installation zu einer unveränderten offiziellen Veröffentlichung bleibt unbestätigt.

Die kleinen DLLs wurden mit REA6.4.0/Ghidra12.1.4 geöffnet, gezielt gesucht, exportiert und geschlossen. Ihre Treffer enthielten keine passenden benannten PayCredits-/Refund-/Dock-Schreibfunktionen. Es wurde daraufhin **nicht** eine weitere breite Ghidra-Shipping-Suche gestartet.

Stattdessen wurde das Shipping-Artefakt direkt und nur lesend untersucht:

1. Feste relevante ASCII-/UTF16-Suchnamen, höchstens16 Treffer pro Name und Encoding, mit vollständigem EXE-Hash und PE32+-Dateioffset/RVA-Zuordnung.
2. 13 ausdrücklich gewählte Textfenster mit je1.792 Bytes sowie höchstens8 ausgerichtete absolute Zeigerreferenzen pro Seed.
3. Funktionsnamenszeiger neben Codezeigern als Kandidaten für generierte Reflexions-/Exec-Tabellen. Die Kandidaten wurden anhand ihrer tatsächlichen Instruktionen weiterverfolgt.
4. PE-Exception-Directory für native Grenzen; **transitive `UNW_FLAG_CHAININFO`-Einträge** erfassen gesplittete Refund-/AddCredits-Körper. Drei kleine Leaf-Bereiche besitzen keine Unwind-Einträge und wurden gesondert mit ihren enthaltenen Verzweigungen und terminalen `RET`s begrenzt.
5. Der bereits installierte `llvm-objdump`22.1.3 dekodiert nur die ausgewählten Bereiche, maximal4.096 Bytes je Seed. Es wurde nichts zusätzlich installiert. Decoderbytes werden anschließend exakt gegen die Originalbereiche verglichen.

LLVM zeigt bei einer symbolarmen EXE teilweise einen weit entfernten Export als nächstgelegene Überschrift. Solche Beschriftungen wie `flecs_sparse_set_generation+…` wurden ausdrücklich **nicht** als Namen der hier untersuchten Funktionen übernommen. Die Bezeichnungen der Codebereiche sind Analystenzuordnungen aus Metadaten und Aufrufkanten; die Rohadresse bleibt der verlässliche Schlüssel.

## Echte Shipping-Credit-Mutation

Der Name `RefundCredits` liegt bei Datei136.787.056 / RVA`0x8274270`. Ein Name-/Code-Zeigerpaar bei Datei136.821.856 führt zum Exec-Kandidaten RVA`0x4B105D0`; dessen direkter Call bei RVA`0x4B10685` führt zum Körper RVA`0x4F98A20`. Ein zweites Paar verweist auf einen generierten Konstruktionskandidaten; es wurde nicht mit dem Exec-Wrapper verwechselt.

| Bereich | RVA / geprüfte Bytes | Beobachtung |
|---|---:|---|
| Refund-Exec-Kandidat | `0x4B105D0` / 202 | Liest zwei Parameter, stellt Integer und Bool für den nachfolgenden direkten Aufruf bereit. |
| Refund-Credit-Mutation | `0x4F98A20` / 257, fünf verkettete Unwind-Bereiche | Prüft ein Ressourcen-Flag, berechnet aktuellen Wert plus Delta, berücksichtigt Cap und Overflow-Bool und schreibt nur bei geändertem Wert. Setzt danach ein Änderungsflag. |
| AddCredits-Exec-Kandidat | `0x4B0C630` / 202 | Separater benannter Wrapper, ruft RVA`0x4F7EBE0`. |
| AddCredits-Mutation | `0x4F7EBE0` / 285, fünf verkettete Bereiche | Gleiche beobachtete Cap-/Overflow-Schreibstrecke; nachgelagerte Verarbeitung unterscheidet sich. |
| Rollenähnlicher Guard-Helper | `0x352EF00` / 21, manuell begrenzter Leaf | Liest ein Byte aus einem über Objekt-Offset`0x90` referenzierten Objekt; die Mutationen schreiben erst, wenn der Rückgabewert mindestens3 ist. |

Die Mutationskörper lesen einen Integer bei Offset`0xB0` und einen Cap bei`0xB4`; bei nichtnull Flag`0x1F8` kehren sie früh zurück. Für **positive** Delta ohne Overflow und bereits erreichtes Cap endet die Mutation ebenfalls früh. Bei nichtnegativem Delta ohne Overflow wird der neue Wert auf `[0, Cap]` begrenzt. Negatives Delta bzw. explizites Overflow läuft in diesen betrachteten Körpern durch eine andere, nicht capbegrenzte Strecke. Ein geänderter Wert wird zurückgeschrieben, zusätzlich Byte`0xBA1` gesetzt.

Die Zuordnung zu Credits/Cap ist durch die separaten Reflexionsnamen, Argumentform und die mitgelieferte `AddCredits`-API gestützt. Die Interpretation des Flags als Infinite-Resources und des Guardwerts als Authority ist mit der API vereinbar, aber in diesem Pass nicht für jedes private Feld vollständig typisiert. Die nachgelagerte Auswertungsfunktion wurde nicht rekonstruiert. Der sichtbare Unterschiede zwischen AddCredits und Refund darf deshalb nicht als vollständige Telemetrie-/Erstattungsregel ausgegeben werden.

**Was dieser Fund nicht sagt:** Der Handler erhält einen Betrag; er berechnet in den geprüften Instruktionen keinen sichtbaren Produktions-Abbruchprozentsatz. Der Queue-Aufrufer könnte diesen vorher festlegen. Es ist nicht bewiesen, dass ein aktiver oder wartender Produktionsauftrag100% erstattet wird. Die negative Strecke ersetzt außerdem keine eigene `CanAfford`-/Bezahlvalidierung.

Konkrete HeroHour-Ableitung: Produktion, Einkommen und Erstattung als ausdrückliche Wallet-Transaktionen führen. Gespeicherte `PaidCost`-Snapshots und Einmal-Storno beibehalten. Für ein späteres Credit-Cap bewusst festlegen, ob Refund ein Cap übersteigen darf oder ein gesondertes Guthaben benötigt; eine versteckte Cap-Klemme darf keine Erstattung verlieren. Vor und nach Commit tatsächlich gebuchte Beträge prüfen. Diese Politik bleibt eine HeroHour-Entscheidung.

## Echte Shipping-Dock-Abfragen

| Bereich | Namens-RVA → Code-RVA / Bytes | Tatsächliche Beobachtung |
|---|---|---|
| `AreAllDepositSlotsEmpty` | `0x82DE520` → Wrapper`0x4B4AB20` / 43 → Leaf`0x4FFCB90` / 56 | Iteriert ein Array von8-Byte-Actor-Pointern; null und nach einer Objektflagprüfung ungültige Einträge zählen nicht als belegte Slots. Bei einem gültigen Eintrag `false`, sonst `true`. |
| `IsDepositingResource` | `0x82DE628` → `0x4B4B0B0` / 187 | Der Wrapper enthält nach Parameterlesen selbst eine Mitgliedschaftssuche des übergebenen Actor-Pointers in diesem Slotarray. |
| `IsInQueue` | `0x82DE6D0` → `0x4B4B210` / 218 | Fragt über einen separaten virtuellen Zugriff ein Pointerarray ab und sucht darin den übergebenen Actor. Die Zugehörigkeit wird unabhängig vom Entlade-Slotarray beantwortet. |

Die Namens-/Code-Verknüpfung liegt bei Datei137.259.520,137.259.536 und137.259.552. Das Slotarray wird in diesen Instruktionen über Objekt-Offsets`0x20C8`/`0x20D0` gelesen; der Queue-Zugriff über einen anderen Teilzustand. Dies sind buildgebundene Beobachtungen, keine externe Runtime-ABI oder vorgeschlagene Speicherpatching-Schnittstelle.

Der Befund unterstützt **zwei getrennte Zustände: wartet in der Raffinerie-Queue und belegt einen Entladeplatz**. Er belegt weder die maximal konfigurierte Slotanzahl noch, wann Reservierungen geschrieben/freigegeben werden. Es wurde keine garantierte FIFO-Politik oder Mindestentladezeit rekonstruiert. Die Objektflagprüfung ist keine nachgewiesene Prüfung von HeroHours eigener `Health > 0`-Regel.

Shipping-Metadaten deklarieren außerdem `ERefineryEventType::ResourceDepositStarted` bei RVA`0x82DE3D8`, `OnResourceDepositFinished` bei`0x82DE408` und `HarvesterAligned` bei`0x82DE438`. Diese Namen sind Deklarationsbelege. Sie legen sinnvolle getrennte Ereignisse nahe, beweisen jedoch keine komplette Reihenfolge und keinen Auszahlungstakt.

Konkrete HeroHour-Ableitung: `WaitingForDock → ApproachingDock → Aligning → Unloading → Departing` als eigene Zustandsmaschine und separate Dock-Claims ergänzen. Feld-Claim und Dock-Claim sind unabhängig. Ein Ladebalken darf die getrennte Entladestufe anzeigen; tatsächliches Cargo wird ausschließlich durch deren autoritativen Tick abgebucht und dem Wallet gutgeschrieben. Tod, manueller Abbruch, Ownership-Wechsel und Raffinerieverlust brauchen ausdrückliche Freigabe-/Cargo-Erhaltungsregeln. Dauer und Rate konfigurieren wir selbst und messen sie später gegen das Original.

## Drei gekoppelte Shipping-Datenstrukturen

Hier werden Felder nicht bloß aus benachbarten Strings geraten. Der Prüfer verfolgt für jede ausdrücklich gewählte Struktur `Namepointer → Struct-Descriptor → begrenztes Propertypointer-Array → Propertyname/Typ`. Der bekannte Typ-ID-/Tabellenaufbau wurde mit dem lokal installierten Unreal-Public-Header korroboriert; dadurch wird nicht die gesamte Original-Engine-ABI als identisch mit UE5.8 behauptet.

### HarvesterData

Generierter Struct-Descriptor: Namepointer bei Datei155.606.760; Propertypointer-Array bei135.797.736; vier Tabellenfelder, deklarierte Größe16, Alignment4.

| Feld | Generierter Typ / Offset | Propertyrecord-RVA |
|---|---|---|
| `ResourceHarvestSpeed` | Int32 / 0 | `0x8182908` |
| `ResourceDepositSpeed` | Int32 / 4 | `0x8182940` |
| `ResourceMaxCapacity` | Int32 / 8 | `0x8182978` |
| `AddResourceFrequency` | Float / 12 | `0x81829B0` |

Dies sind Definitionstypen, **keine konfigurierten Zahlenwerte**. Ob die benannten Geschwindigkeiten Mengen pro Tick oder anders skalierte Werte sind, wurde nicht rekonstruiert. Die Float-Frequenz allein liefert keinen belegten Seconds-per-Tick-Wert. Für HeroHour sollten Cargo-Capacity, Erntequantum/-rate, Ablieferquantum/-rate und Takt getrennte Konfigurationsfelder sein.

### GameplayCommandDataPayload_EnterExitRefinery

Namepointer bei Datei155.784.872; zwei Propertyrecords, deklarierte Größe48, Alignment8. `Refinery` ist ein Object-Feld bei Offset32. `bExit` ist ein NativeBool. Dessen Propertysetterpointer führt zu RVA`0x1656F90`; die fünf Byte `MOV byte ptr [RCX+0x28],1; RET` bestätigen Bool-Speicherung bei Offset40.

Das unterstützt die Deklaration eines ausdrücklichen Ein-/Ausfahrt-Payloads mit referenzierter Raffinerie und Richtungsflag. Base-Payload-Felder, Wireformat, Replayserialisierung und Guard-/Dockaktionen wurden damit nicht vollständig rekonstruiert. HeroHour kann einen eigenen Befehl mit stabilem Refinery-Handle, Eintritt/Austritt und Ablehnungsgrund anbieten.

### TedConstructionQueue

Namepointer bei Datei155.719.920; Propertypointer-Array bei136.853.776; sieben Tabellenrecords, deklarierte Größe88, Alignment8. Zwei Records sind Array-Innertypen; sie sind keine zusätzlichen Top-Level-Felder.

| Top-Level-Feld | Generierter Typ / Offset | Bedeutung und Grenze |
|---|---|---|
| `QueueType` | Struct / 0 | Queueklassifikation; konkreter Structtyp hier nicht vollständig aufgelöst. |
| `Factory` | Object / 8 | Produzentenbezug. |
| `Order` | Array von Struct / 16 | Bestellliste; innere Struktur/PaidCost-Politik offen. |
| `Progresses` | Array von Struct / 32 | Getrennte Fortschrittsdaten; parallele Ausführung daraus nicht bewiesen. |
| `CompletedStructure` | Struct / 48 | Gesonderter fertiggestellter Strukturzustand; konkrete Place-/Spawn-Freigabe offen. |

Die echte native `CreateCommandData_ConstructionQueue`-Wrapperkette wurde ebenfalls geprüft: WrapperRVA`0x4AFB490`/414 Bytes, direkter Callee`0x4CB9650`/595 Bytes. Der Callee validiert mehrere Objekt-/Klassen-/Pointerbedingungen, erzeugt Daten und kopiert sie in den Rückgabeträger. Verschiedene indirekte Aufrufe sind nicht vollständig typisiert. Dieser Datenaufbau wird ausdrücklich **nicht** als rekonstruierter Queue-Enqueue-/Pay-Commit ausgegeben.

Konkrete HeroHour-Ableitung: Producer-Handle, Queuekategorie, Auftrags-ID, gespeicherter Preis, Zieldefinition und Fortschritt gehören in das Simulationsmodell. Ein fertig gebautes Gebäude kann als `ReadyForPlacement` gehalten werden, bevor Platzierung dieselbe autoritative Validierung nutzt. UI liest Queue-/Fortschrittsänderungen; sie besitzt weder Wallet noch Auftrag. Spawn-/Placement-Commit löscht einen fertigen Auftrag erst nach erfolgreicher Durchführung.

## Weitere Deklarationen und bewusst verworfene Treffer

Shipping deklariert `EConstructionQueueStatus` mit InProgress, Completed und Paused sowie `EQueueAction` mit Enqueue/Dequeue. Außerdem wurden Command-Replikationsereignisse SetCurrent, ClearCurrent, DequeueAndSetAsCurrent, Enqueue, EnqueueAtFront, ClearQueue, Reset und Invalid als Namen gefunden. Dies belegt unterschiedliche Operationen, keine originale Netzwerkserialisierung.

Ein UI-Textfenster enthält EnqueueCmdSingle/Multiple, DequeueCmdSingle/Multiple, PauseConstructionCmd, UnPauseConstructionCmd, PlaceStructureCmd und PlaceConstructionSiteCmd sowie SelectedFactoryTab/SelectedFactories. Diese Nähe ist ein begrenzter Metadatenhinweis auf getrennte Sidebar-Aktionen; die vollständige Widget-/Classbindung und deren tatsächliche Bedienung wurden nicht geprüft.

Die wörtlichen Suchnamen `QueueUnitProductionCommand`, `CancelUnitProduction` und `PayCredits` ergaben im definierten ASCII-/UTF16-Pass keine Treffer. Das ursprüngliche System nutzt zumindest die belegten ConstructionQueue-/Enqueue-Namen; das Fehlen der Suchwörter beweist keine fehlende Funktionalität. Allgemeine `Docking.*`-Treffer stammen erkennbar aus Unreal-Tab-/Fensterstyles und wurden **nicht** als Sammler-Andocklogik verwendet. Spectator-ProductionQueue-Widgetnamen werden ebenfalls nicht als aktive Spieler-Sidebar ausgegeben.

REA-DLL-Belege dieses Passes: `ev_a913d1a57a1f84ee24cafc403df516fb3663a261616a05de7d9e6aea01a2eaa3`, `ev_fa25ae25d7e7dec6941bb9461e390191755727e5014023aa5a165a979f826118`, `ev_4f547b5e3a19941bcc83a99d4a877c09987d0ab17f406042e235232e8fc823aa`, `ev_edc70fa9176bf52fe974debbf0ed6bfc318f8c6561d620d15a81eba0fa36a1aa`, `ev_981e200a1124432c6084034dbf5ff31c24db011016ad6e74a90cb6c99ba4032a`. Die nachfolgenden direkten Shipping-Decoder-Ergebnisse besitzen lokale JSON-/Hashbelege, keine erfundenen REA-Evidence-IDs.

## Reproduzierbare Prüfung und nächster Schritt

Lokale Rohbelege und Prüfer liegen unter `Saved/Research/rea-next/`:

- `editor-production-evidence.json`/`editor-production-snapshot.json` und entsprechende `commands-production-*`: neue begrenzte REA-DLL-Suchen, anschließend geschlossene Sitzungen.
- `scan_shipping_command_metadata.py` → `shipping-command-metadata.json`: feste Suchwörter, Treffergrenzen, Hash und textbezogene Dateioffsets/RVAs.
- `inspect_shipping_metadata_windows.py` → `shipping-command-windows.json`: begrenzte Text-/Zeigerfenster.
- `decode_shipping_struct_metadata.py` → `shipping-struct-metadata.json`: drei echte Property-Pointerketten, Typ-IDs, Größen und Quellbereich-Hashes.
- `disassemble_shipping_command_slices.py` → `shipping-command-slices.json` und einzelne `.asm.txt`: exakte Bereiche, Unwindketten, Decoder-/Originalbyte-SHA-256 und Gleichheitsprüfung. LLVM-Binary-SHA-256: `7e726d87fe123f10653998a78dc6a15492fb931f1e6de7265df7d923fc81a210`.

Die vier Pythonprüfer werden in der genannten Reihenfolge mit dem vorhandenen Bundled-Python ausgeführt. Sie lesen die explizite Original-EXE, prüfen deren Hash und schreiben nur in das Forschungsverzeichnis. Der Instruktionsprüfer gibt bei ungleichen Decoderbytes einen Fehler zurück. Keine dieser PASS-Aussagen ist ein akzeptierter kompletter Tempest-Rebuild.

Exakter erfolgreicher Abschlussmarker des Instruktionsprüfers:

```text
SHIPPING_BYTE_ANCHORING_PASS stage=commands-docking slices=13 ranges=21 bytes=2637
```

Reproduzieren:

```powershell
& 'C:\Users\Hero\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' 'C:\Users\Hero\HeroHour\Scripts\research\disassemble_shipping_command_slices.py'
```

Ein abschließender, zusätzlich begrenzter Direktcaller-Pass wurde lokal behalten: `find_shipping_direct_call_candidates.py`/`shipping-direct-call-candidates.json` enthält31 rohe E8-Kandidaten zu drei bereits identifizierten Zielen. Diese Kandidaten sind ausdrücklich noch keine vollständig validierte Callergraph-Abdeckung. Drei ausgewählte Kandidaten wurden anschließend mit `disassemble_shipping_command_slices.py --payment-callers` quellverankert und als `shipping-payment-callers.json`/`PaymentCaller_*.asm.txt` gespeichert. Deren vollständige Gameplay-Zuordnung wurde vor Abschluss dieser Stufe nicht ausformuliert; daraus wird hier kein ursprünglicher Produktions-Bezahlzeitpunkt behauptet. Die13 primären Codebereiche und deren Marker bleiben davon getrennt.

Nächste eigene Abnahme: zwei Sammler an einer Raffinerie, getrennte wartende/entladende Zustände, endliche Unload-Rate, genau einmal gebuchte Cargoabnahme, Slotfreigabe bei Unterbrechung und tatsächliche Abfahrt. Queue-Abnahme getrennt: Zahlungssnapshot, aktiver/wartender Storno, blockierter Spawn, ReadyForPlacement, Produzentenverlust und später Credit-Cap/Overflow. Originalvergleich weiterhin mit endlichen Credits und den vorab festgelegten M1-Runtime-Szenarien. Keine neue UI-Sitzung wurde in diesem Pass begonnen.

Die vier versionierten Prüfer unter Scripts/research wurden erneut erfolgreich ausgeführt; ROOT verweist auf Saved/Research/rea-next. JSON/ASM bleiben dort. Der komplette vierstufige Ablauf steht in Refinery-Docking-Acceptance.md.