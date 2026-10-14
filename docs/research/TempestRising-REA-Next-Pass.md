# Tempest Rising: zweiter gezielter REA-Pass

Stand: 14. Oktober 2026. Dieser Durchlauf ergänzt [Systems Evidence](TempestRising-Systems-Evidence.md) und den [M1-Prüfvertrag](TempestRising-M1-Acceptance.md). Er wurde parallel zur HeroHour-Implementierung durch einen eigenen REA-Agenten durchgeführt. Das Ziel wurde statisch gelesen; weder Tempest noch sein Editor wurden gestartet oder bedient.

## Ergebnis

**Neue echte Editor-Logik wurde gefunden:** eine Prüfung von Wachstumsquellen, räumliches Sammeln noch ressourcenhaltiger Zell-IDs, getrennte Getter für Darstellungsstadien und Übergangsfortschritt sowie ein autoritativer Wechsel des Erschöpfungszustands. Die Schreibfunktion `SetResourceDepleted` ist ein echter Körper; die ältere Aussage über kurze Platzhalter bezog sich auf die damals untersuchten Harvest-/Claim-/Power-Funktionen und darf nicht auf sämtliche Editor-Schreibpfade verallgemeinert werden.

Sechs ausgewählte Funktionen wurden zusätzlich anhand ihrer exakten analysierten Körperbereiche gegen die Originaldatei geprüft: **9 Bereiche, 2.450 Bytes, alle gleich**. Das bestätigt die Byte-Verankerung der statischen Analyse. Ein neu kompilierter Nachbau, Shipping-Parität und ein dynamischer Harvest-Zyklus sind damit weiterhin nicht nachgewiesen.

## Ziel und Methode

- Ziel: `F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject/Binaries/Win64/UnrealEditor-Tempest.dll`.
- SHA-256: `2fff65ec4f3809e8f24010d5cbfa9ada2f2834b59c997569c219d6a494f0b4fa`.
- REA 6.4.0, Ghidra 12.1.4, x86-64 PE, ImageBase `0x180000000`.
- Der passende vorhandene Analyse-Snapshot wurde importiert. Danach wurden fokussierte Namenssuchen und einzelne Funktionsdossiers verwendet, einschließlich Assembler und Kontrollfluss. Die neue Sitzung wurde mit Snapshot geschlossen.
- Die 166.518.344-Byte-Shipping-EXE wurde in diesem Durchlauf nicht erneut breit analysiert: der frühere Versuch hatte nach 300 Sekunden keinen Funktionsbeleg geliefert. Die fehlende Shipping-Abdeckung wird durch Editor-Ergebnisse nicht ersetzt.
- Keine Paketschlüssel, Schutzumgehung oder Übernahme fremder Modelle/Textures. Die Originaldateien blieben unverändert.

## Neue Funktionsbelege

Die Größen zählen nur die von Ghidra gemeldeten **inklusiven Körperbereiche**. Bei Lücken ist die umschließende Spanne größer und darf nicht als zusammenhängender Funktionscode ausgegeben werden.

| Funktion | RVA / analysierte Bytes | Tatsächliche Beobachtung | Evidence-ID |
|---|---:|---|---|
| `IsGrowthSourceTile` | `0xEA440` / 152 | Vergleicht zwei ganzzahlige Koordinaten zuerst mit einer separaten Liste von 8-Byte-Einträgen. Bei Treffer sofort `true`. Sonst sucht sie passende Koordinaten in 48-Byte-Zelldatensätzen und akzeptiert deren Byte bei Offset8 ab dem Wert2. Sonst `false`. | `ev_ce9b285f8f9cf0f0f8f20931168a40c35c1ed401b6357ae48229d3b0f1672686` |
| `GetResourceTileDataIndicesInRange` | `0xE5700` / 1.446; Spanne1.482 | Nutzt einen vorhandenen oder aus der Welt geholten Dienst über virtuelle Aufrufe, verarbeitet Koordinatenkandidaten und sucht passende Einträge im Zellarray mit 48-Byte-Abstand. Ein gefundener Zellindex wird nur aufgenommen, wenn dessen Float bei Offset32 positiv ist. Ausgabe ist eine Menge von ganzzahligen Indizes. | `ev_49a360ae1a058166605598981cd9b7feb4c25802b695a18a54d66514e653a328` |
| `GetElementCurrentStage` | `0xE13C0` / 135 | Prüft den Elementindex und liest ein Stage-Byte bei Offset128 aus einem separaten Elementarray mit 192-Byte-Abstand. Bei ungültigem Index wird `AL` nullgesetzt. | `ev_0fb7d74408a3c817c065c38b5534419114df5386094be4963e16b65355d62f43` |
| `GetElementNextStage` | `0xE14E0` / 135 | Gleiche Element-/Indexprüfung, liest das benachbarte Stage-Byte bei Offset129. Bei ungültigem Index wird `AL` nullgesetzt. | `ev_a8f50a931f79650af1ad4061e0e2760c6670399526acba26bff986cc0569aca0` |
| `GetElementTransitionProgress` | `0xE2180` / 137 | Gleiche Element-/Indexprüfung, liest einen Float bei Offset132; ungültiger Index liefert `0.0`. | `ev_4ec51bb6b38f22e31fb27ef64c51b0ce00720f5488d49e31c2681c1f03ea6963` |
| `SetResourceDepleted` | `0xFB5D0` / 445; Spanne450 | Prüft Authority, verlangt einen geänderten Depleted-Wert und schreibt dann das Flag. Ruft einen weiteren Dienst mit dem inversen Flag auf. Bei Wechsel zu nicht erschöpft und vorhandenem Timer/Welt wird `FTimerManager::UnPauseTimer` aufgerufen. Anschließend werden registrierte Listener mit Feld und neuem Flag benachrichtigt; ungültige Einträge werden bereinigt. | `ev_c4b229593837fde6588bf823dfe93ffdbc10f4844059d631c84b5e672362dd7e` |
| `IsResourceDepleted` | `0x992F0` / 8 | Liest ein gespeichertes Byteflag bei Objekt-Offset`0x658`. Die Abfrage summiert nicht selbst den aktuellen Bestand. | `ev_2e08f247b1ea10948e096ccb2dbc3e954a6ab574b0da115acaba13b15e923e25` |
| `IsHasCrater` | `0x992C0` / 8 | Liest ein separates Byteflag bei Objekt-Offset`0x5EC`. | `ev_a945f645abb112fd6b7f54a201a979c32a0671facd48d2be00ee6f0ec32cf89f` |

Die Actor-Offsets sind **buildgebundene Beobachtungen**, keine ABI für HeroHour und keine vorgeschlagene Speicherpatching-Schnittstelle. Die Namen stammen aus der untersuchten DLL. Pseudocode ist decompilererzeugt und nicht der ursprüngliche Quelltext.

`SetResourceDepleted` enthält die Assertions `HasAuthority()` und `bResourceDepleted != bDepleted` sowie den eingebetteten Quellpfad `Source/Tempest/Resource/TempestFieldVolume.cpp`, Zeilen791/793. Das stärkt die Interpretation des Guards und der Zustandsänderung. Die Bedeutung des zusätzlich aufgerufenen virtuellen Dienstes wurde nicht vollständig aufgelöst. Das konkrete Ereignis kann deshalb nicht als nachgewiesene komplette Sammler-Neuzuweisung bezeichnet werden.

## Gesicherte Beobachtung und begrenzte Ableitung

Die API (`MapEditorProject/Docs/api.pdf`, SHA-256 `591cef53057ae229fca5cf41f525bd26ee01fda79fcaf33a2333a78e76e79263`) nennt auf S.172–174 getrennte Element-Stage-/Transition-Abfragen. S.178 führt `Field Crater Coords` sowie Wachstumsanfang/-ziel auf; ein nicht voll ausgewachsenes Feld benötigt laut Beschreibung Krater, um wachsen zu können. S.190 deklariert die Wachstumsstadien Initial0, Vines1, SmallGrowth2, MediumGrowth3, MaxGrowth4 und zusätzliche Elementstadien Rotten5/Uninitialized6.

Die separate Koordinatenliste in `IsGrowthSourceTile` ist mit der dokumentierten Kraterliste vereinbar. Die Verbindung ist eine **Ableitung**, keine verifizierte Eigenschaftszuordnung für jedes private Objektfeld. Im nativen Vergleich des Zell-Bytes ist nur die untere Grenze2 sichtbar; es gibt dort keine obere Grenze4. Deshalb darf der Fund nicht in die stärkere Behauptung umformuliert werden, dass ausschließlich Small/Medium/MaxGrowth Quellen seien. Ob der Zell-Bytewert tatsächlich alle Element-Enumwerte annehmen kann, wurde nicht festgestellt.

Die räumliche Ressourcenabfrage prüft positives lokales Ressourcenquantum. Die virtuelle Dienstauflösung, die exakte Einheit des Integer-Radius, Claim-Eigentümer, Pawn-Erreichbarkeit und der verwendete Distanzbegriff wurden in diesem Dossier nicht vollständig rekonstruiert. Dieser Fund ersetzt insbesondere nicht den fehlenden echten `GetClosestUnclaimedResourceTile`-Algorithmus.

## Neue begrenzte Negativergebnisse

| Funktion | RVA / Bytes | Beobachteter Platzhalter | Evidence-ID |
|---|---:|---|---|
| `GetCurrentResourceGrowRate` | `0xE0710` / 4 | `XORPS XMM0, XMM0; RET`, liefert0. | `ev_0bd58d601a268ae1bfab808f5fff400741ba7e977c3a3a86a0850bb0fba9c7da` |
| `GetDepletedResourceGrowRate` | `0xE07D0` / 4 | Gleiche konstante0-Rückgabe. | `ev_fe9f88f6efc795ae538149ef7bc8f5e9a568409d408ad776e236977774fbff93` |
| `TickGrowth` | `0x1033B0` / 3 | Sofortige Rückkehr ohne Wachstumsschreibpfad. | `ev_9fa75bd33c83149bf8a078d9156fa336645ed6c0859bbac077ad8ee2525499e0` |
| `GetClosestUnclaimedResourceTile` | `0xE03F0` / 6 | Gibt konstant`−1` zurück. | `ev_66dd071601cc1a5303f2a09c5612e359ad804937a5d183c21949d9801ac9c498` |

Damit sind Regenerationsrate, Verzögerung und nächstes erreichbares Harvest-Ziel weiterhin unbekannt. Die Nullwerte der Editor-Stubs sind keine Tempest-Balancewerte. Die fokussierten Suchen fanden Deklarationen von `UTedAbility_EnqueueUnit`, `UTedUnitQueueInterface` und `UTedUnitCommand_EnterExitRefinery`; aus diesen Namen wurde keine Queue- oder Dock-Implementierung abgeleitet.

Suchbelege: `ev_627bd1c268c94bb053ffc51784788c032e05050c1dbaf14614618e012ef30ed4`, `ev_017a7f79da1a831adc4115f52cea6188c87738e86826f61811eda618c958572c`, `ev_6fb9737e8930802b1d51787996de089012519965b08a53d28c5cec554e6e6125`. Die Suchgrenze sind die analysierten Prozedurnamen dieser DLL; fehlende Namensmatches beweisen keine Abwesenheit anders benannter Shipping-Funktionen.

## Konkrete Anwendung in HeroHour

1. **Bestand und Darstellung trennen:** Der autoritative Feldkern liefert Bestand und Entnehmbarkeit. Ein visueller Adapter führt aktuelles Stadium, Zielstadium und Übergangsfortschritt. Ein eigener Schwellenplan kann aus Bestand abgeleitet werden, muss aber als HeroHour-Regel dokumentiert werden. Die aktuellen17 dekorativen Cluster des neuen Patch-Meshs sind noch keine17 unabhängig geernteten Simulationszellen.
2. **Erschöpfung als Wechsel behandeln:** Ein eigener `OnFieldAvailabilityChanged`-Hook wird nur ausgelöst, wenn Entnehmbarkeit von positiv zu null oder zurück wechselt. Nav-Hindernis, Sammler-Zielsuche und Visuals konsumieren denselben autoritativen Wechsel. Das vermeidet wiederholte Events pro Frame; eine erschöpfte nachwachsende Fläche bleibt als Feldbett sichtbar.
3. **Saat-/Quellzustand explizit machen:** Für spätere räumliche Ausbreitung eigene Quellen/Startstadien konfigurieren und getrennt vom erntbaren Quantum führen. Der vorhandene geschützte `SeedFloor` bleibt eine HeroHour-Regel und ist durch den Tempest-Fund nicht als ursprünglicher Algorithmus bestätigt.
4. **Zellkandidaten vor Auswahl filtern:** Positive Entnehmbarkeit, exklusive Reservierung und echte Erreichbarkeit separat prüfen. Eine räumliche Indexmenge ist noch kein gültiges Sammlerziel. Bei Saatrest muss Teilcargo weiterhin zurückfahren.
5. **Abnahme erweitern:** Visuals an voll→leer→wachsend prüfen, Erschöpfungs-/Wiederverfügbarkeitswechsel jeweils genau einmal zählen und sicherstellen, dass Rendering-Ticks keine Ressource erzeugen. Pause/Matchende stoppen nur die autoritative Simulation; visuelle Interpolation darf den Bestand nicht verändern.

Diese Anwendungen sind eigene Integrationsentscheidungen, keine kopierten DLL-Strukturen. Tatsächlicher Dockablauf, Stornobetrag, LowPower-Faktor und Originalwachstum benötigen weiterhin die im M1-Dokument festgelegten Laufzeit-Zeitreihen oder neue Shipping-Belege.

## Reproduzierbare Rohbelege

Alle lokalen Rohdateien liegen in `Saved/Research/rea-next/` und sind keine Git-Quelldateien:

- `editor-pass2-evidence.json`: 52 behaltene Records einschließlich wiederverwendeter alter Belege.
- `editor-pass2-snapshot.json`: abgeschlossene Sitzung mit passenden Artefakt-/Profilidentitäten.
- `verify_pass2_bytes.py`: liest das Originalartefakt und vergleicht ausschließlich die gemeldeten Funktionskörperbereiche; keine DLL-Ausführung.
- `editor-pass2-byte-comparison.json`: Quelle, SHA-256, genaue RVA-/Dateioffset-Bereiche, Evidence-IDs und Vergleichsergebnisse.

Ausführen im HeroHour-Ordner:

```powershell
& 'C:\Users\Hero\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe' 'Saved/Research/rea-next/verify_pass2_bytes.py'
```

Beispiel für die neue Wachstumsquellen-Prüfung: RVA`0xEA440`, Dateioffset956.480, inklusive152 Bytes. Analysebytes: `ev_64bd747e1261b9457d48173ab28dc0021b577f5894b223097acbca3a1a0394ac`; Dateizuordnung: `ev_d408ddc4ed7e3aca066dd4f8d68c4c0e1de219a7d46a967cfe823b86f7838f3e`. Der gespeicherte Bericht enthält entsprechende Angaben für alle9 Bereiche und überprüft vor jedem Vergleich den gesamten DLL-Hash.

REA ist nach diesem Pass geschlossen. Der nächste Original-Runtime-Pass bleibt eine koordinierte Aufgabe des Hauptagenten oder Nutzers, damit die aktive HeroHour-Sitzung nicht durch konkurrierende UI-Automation gestört wird.
