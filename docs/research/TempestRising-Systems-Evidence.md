# Tempest Rising: gezielte Systembelege für HeroHour

Stand: 14. Oktober 2026. Diese Untersuchung verwendet den Skill `reverse-engineer-anything`, REA 6.4.0 und Ghidra 12.1.4. Sie ergänzt die [Erstprüfung](TempestRising-REA-FirstPass.md), [Konfiguration](TempestRising-ConfigReview.md) und [API-Prüfung](TempestRising-MapEditorReview.md). Die Originaldateien wurden nicht geändert. Das laufende Spiel des Nutzers wurde nicht bedient. Es wurden weder Paket-Schlüssel gesucht noch fremde Spielassets übernommen.

## Ergebnis und Beleggrenzen

Neue native Belege bestätigen zwei echte Editor-Funktionskörper: die Ressourcen-Gesamtabfrage über Zelleneinträge und eine Mitgliedschaftsprüfung für beanspruchte Zell-IDs. Die untersuchten Funktionen zum Beanspruchen, Freigeben, Ernten und Ändern des Stromzustands sind dagegen kurze Platzhalter. Für diese Schreiboperationen wurde kein Shipping-Algorithmus rekonstruiert.

Die kleine GameplayCommands-DLL enthält aussagekräftige Befehls- und Lebenszyklusdeklarationen. Die Suche und ein Konstruktor-Dossier liefern jedoch keine rekonstruierte Produktionsqueue. Kosten, Erstattung, Andocken, Freigabe bei Wegfehlern, Regenerationsraten und Strommangel-Multiplikatoren bleiben offen.

Vier Belegarten werden getrennt geführt:

1. **Deklaration:** mitgelieferte Konfiguration, API-Handbuch und eingebettete Reflexionsmetadaten.
2. **Native Beobachtung:** untersuchte Maschinenbefehle und dekompilierte Kontroll-/Datenflüsse dieser konkreten DLL.
3. **Laufzeitbild:** bereits vorhandene Spielaufnahme; sie belegt nur den sichtbaren Zustand.
4. **Eigene Ableitung:** vorgeschlagene HeroHour-Regel mit eigenem Abnahmetest.

## Identität und Umfang

| Artefakt | SHA-256 | Rolle in diesem Durchlauf |
|---|---|---|
| `MapEditorProject/Binaries/Win64/UnrealEditor-Tempest.dll` | `2fff65ec4f3809e8f24010d5cbfa9ada2f2834b59c997569c219d6a494f0b4fa` | Gezielte Ressourcen-/Strom-Funktionsanalyse |
| `MapEditorProject/Plugins/GameplayCommands/Binaries/Win64/UnrealEditor-GameplayCommands.dll` | `531f6e6474b259e935ac84c1edb170f58cfc181bd32415c87aa35f24be0fb952` | Befehlsdeklarationen und Konstruktor |
| `MapEditorProject/Docs/api.pdf` | `591cef53057ae229fca5cf41f525bd26ee01fda79fcaf33a2333a78e76e79263` | Seitenbezogene öffentliche API |
| `MapEditorProject/Docs/manual.pdf` | `8f8b8ce9c1725ea3fce3d4296de93ce94c347eba3e8f88f9e600c4e2b2a66417` | Karten-/Rasterregeln |

Referenzwurzel: `F:/Tempest-Rising-SteamRIP.com/Tempest Rising`. Die bereits identifizierte Shipping-EXE ist unverändert: 166.518.344 Bytes, SHA-256 `c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56`. Eine Zuordnung der lokalen Installation zu einer unveränderten offiziellen Veröffentlichung ist nicht bestätigt. Die frühere 300-Sekunden-Zeitüberschreitung wird nicht durch eine erneute breite Shipping-Analyse wiederholt. Die Editorbelege werden nicht als Shipping-Verhalten ausgegeben.

REA hatte zu Beginn kein geöffnetes Ziel und keine behaltenen alten Records. Bestehende lokale JSON-Ergebnisse und Seitenextraktionen wurden wiederverwendet; für die neuen Fragen wurden die beiden kleinen DLLs gezielt geöffnet und danach einschließlich Snapshot geschlossen.

## Neue native Ressourcenbelege

Gezielte Namenssuche: `ev_48210348c04d799872e8f213f96a36a272b46337d953475ef022302cba04354c`. Die folgende Tabelle betrifft ausschließlich `UnrealEditor-Tempest.dll`.

| Funktion | Adresse / Körpergröße | Beobachtung | Evidence |
|---|---|---|---|
| `GetResourceLeft` | `0x1800e53f0` / 200 Bytes | Iteriert Datensätze mit 48-Byte-Abstand; summiert einen Float aus jedem Datensatz und einen tabellarischen Beitrag mit Index `max(Bytewert−2,0)`. Enthält eine Prüfung gegen Änderung der Arraygröße während der Iteration. | `ev_9f4e0b2fa3db950dee672d90963c13f61c04751b833e3b985c9c70f4d3a841a7` |
| `IsFieldTileIdxClaimed` | `0x180099250` / 102 Bytes | Prüft eine ganzzahlige Zell-ID über Bucket-Auswahl und verkettete Einträge; endet bei Treffer oder `−1`. | `ev_dadc52f91f325b7310505ecd65eeb8b8a391a6c512af25aa046c11657c32a4af` |
| `ClaimFieldTileIdx` | `0x1800ca660` / 3 Bytes | Gibt konstant `true` zurück; kein beobachteter Schreibvorgang. | `ev_9c8ac0cec6769f3b8be93b4f8a8a684e70c2c2da83b5c39a68a63a53e3ac97c5` |
| `UnclaimFieldTile` | `0x180106bc0` / 3 Bytes | Sofortige Rückkehr ohne beobachtete Freigabe. | `ev_11c4ed09e2b6bd1cc988f7c513635a4249f941a63b7a8f1d7ac51368c45704b8` |
| `UnclaimFieldTileSet` | `0x180106bd0` / 3 Bytes | Sofortige Rückkehr ohne beobachtete Freigabe. | `ev_ccddcd2276688b522265feee3f50b07817eb2192073ae3e69dbcfb6e15495ca7` |
| `HarvestTile` | `0x1800e6940` / 4 Bytes | Gibt konstant `0.0` zurück. | `ev_fb6fd3a879b5c460e33a0b68c3961f5a4a4caed6dff4ee3ecd0b0a29f267a308` |
| `GetResourceAtWorldLocation` | `0x1800e53b0` / 4 Bytes | Gibt konstant `0.0` zurück. | `ev_c0ebc52e3b2dd646008790e6df862d89163d917e173c457dc1de49d502cea5e8` |
| `GetVisibleResourceLeft` | `0x1800e6220` / 4 Bytes | Gibt konstant `0.0` zurück. | `ev_681080815a13e39fb56e7e0c4c1bb0d002762625b9d3abed5ff96b9f34f0e6d3` |
| `NumClaimedTiles` | `0x1800eae40` / 3 Bytes | Gibt konstant `0` zurück. Die widersprüchliche Handbuchbeschreibung wird dadurch nicht geklärt. | `ev_76debcc8b692206d68e4e5bc82afdc63dad0e13237dad4332dfb2851ca2c85d8` |

**Ableitung, keine zusätzliche Beobachtung:** Der Zell-ID-Lookup ist mit einer Hashmenge ganzzahliger Schlüssel vereinbar. Die Ressourcen-Summe passt zum dokumentierten Zell-/Wachstumsmodell. Benennung und exakte Bedeutung aller rekonstruierten Felder sind damit nicht bestätigt; insbesondere fehlen Claim-Eigentümer, Synchronisation, Schreibpfade und Wachstumstakt. Keine dieser Funktionen wurde in diesem Durchlauf ausgeführt.

Die API ergänzt: Ressourcenfelder haben getrennte Build-, Effects- und Gameplay-Config-Assets sowie Schwierigkeitsüberschreibungen (S. 175–177). Ein Startstadium vor MaxGrowth benötigt laut Dokumentation aktivierte Krater, damit Wachstum stattfindet (S. 178). Die Abfrage der nächstgelegenen freien Kachel kann Pawn-Erreichbarkeit berücksichtigen und `INDEX_NONE` liefern (S. 181–182). Sichtbare und tatsächliche Restmenge sind getrennte APIs (S. 180–181). Eine konkrete Regenerationsrate oder Freigabepolitik wird dadurch nicht belegt.

## Befehle, Queue und Definitionen

Neue GameplayCommands-Namenssuche: `ev_72eb0bcb7d908924b141fb80b3bc67a54a5085c115551ae00803b8bece766a3a`. Eingebettete Enum-/Kommentarstrings: `ev_87c0fbbcefa986751ffc45468485c143c4efd6f5f4a20be89459373f3d66d428`.

| Deklarierter Bereich | Beobachtete Varianten | Bedeutung für einen eigenen Entwurf |
|---|---|---|
| Zieltyp | Actor, Location, LocationOrActor, None | Zielauflösung und Befehlsprüfung typisieren. |
| Gruppenverteilung | All, MainSelected, MostSuitable, SelectedSubGroup | Nicht jeden Befehl blind auf alle ausgewählten Einheiten anwenden. |
| Verarbeitung | CanBeCanceled, CannotBeCanceled, Instant | Abbruchfähigkeit und sofortige Aktionen getrennt festlegen. |
| Cooldown | Custom, OnExecute, OnFinished | Zeitpunkt einer Cooldown-Buchung ausdrücklich definieren. |
| Allgemeine Anwendung | Manual, None, OnExecute, OnFinished, OnFinishedSuccessfully | Lebenszykluszeitpunkte unterscheiden; die Suche identifiziert nicht, welche konkrete Mutation jede Spielaktion daran bindet. |
| Ergebnis | Canceled, Failed, Succeeded | Ein Abbruch ist ein eigener Ausgang. |
| Netz-Ausführung | LocalPredicted, ServerOnly | Deklarierte Autoritätsmodelle; kein Laufzeitnachweis von Prediction oder Replikation. |

Der untersuchte `UGameplayCommand`-Konstruktor bei `0x180001c60` (34 Bytes) ruft den UObject-Konstruktor auf und setzt eine VTable: `ev_a4dd9b527dc8fefaa599a6545fe7dc380570d929136672fcbe7f508bd667a3ae`. Daraus folgt kein Produktions- oder Queue-Algorithmus. Die vollständige fokussierte Suche wurde behalten; das Fehlen eines passenden Namens beweist nicht die Abwesenheit unbekannter oder anders benannter Implementierung.

Die API beschreibt Definitionen konkreter Pawn-Klassen mit Faction, Bauzeit, Kosten, Voraussetzungen, Eigentümer-Tags, Sicht-/Kampfattributen, Icon und unterstützten Befehlen (S. 6–10). `InfluenceRange` wird in **Baurasterzellen** angegeben (S. 8). Strukturdefinitionen unterscheiden produzierte Strukturen und Einheiten, erlaubte Mehrfach-Bauqueue, eindeutige Gebäude und optionalen Baustellen-Vorauspreis (S. 11). Größe und gedrehte Größe werden getrennt abgefragt (S. 154). Keine dieser Deklarationen liefert den vollständigen Zahlungs-/Refundalgorithmus.

`PlayerDataTags.ini` unterscheidet Buildings, Defenses, Infantry, Vehicles und AerialAndNaval (Z. 197–204). Die Kommentare zu Bauverbot, gleichzeitiger Konstruktion und Bau im eigenen Fabrikradius stehen in Z. 158–167. Diese Klassifikation lässt sich für Sidebar-Kategorien und Validatoren verwenden. HeroHours bisherige FIFO-Queue und volle Stornoerstattung bleiben eigene Regeln, bis ein konkreter Vergleich etwas anderes belegt.

## Strom, Bauraster und Sicht

Die API führt FullPower, LowPower und NoPower auf (S. 182); manuelles Abschalten ist zusätzlich über Flags/Tags ausdrückbar. Die untersuchten nativen Editorfunktionen liefern dazu keine echte Zustandstransition:

| Funktion | Adresse / Größe | Beobachtung / Evidence |
|---|---|---|
| `GetPowerStatus` | `0x18003d860` / 3 Bytes | `XOR AL, AL; RET`; kein Auslesen von Gebäudestatus. `ev_53b263d27c621364d7c2589b9cc13b319d785ec24a50e4b4f3360a562421b038` |
| `HasPowerStateFlag` | `0x180040080` / 3 Bytes | Konstant `false`. `ev_da793b3a1b5a7877f243ec107d972e92aa0a307f05a383a19736bc561115c114` |
| `SetPowerStateFlag` | `0x180041120` / 3 Bytes | Rückkehr ohne beobachtete Änderung. `ev_4798ff809a0d528c6d49f34d7d2c5a9ce1366b11f3bacdc8050fb955d110710e` |

Deklariert sind getrennte Stromerzeugung, Verbrauch, freie Leistung und LowPower-Abfragen sowie autoritative Mutationen (api S. 134–137, 146). Definitions-Power ist vorzeichenbehaftet: positive Erzeugung, negativer Verbrauch (S. 8). `AllowInsufficientPower`, `AllowPowerDown` und `InsufficientPowerSlowDown` haben aussagekräftige Entwicklerkommentare (Config Z. 155–159); Zahlenwerte für Verlangsamung und Prioritätsverteilung fehlen. Abschalt- und LowPower-Zustände werden im `PowerDown`-Tag zusammengeführt (Z. 208), aber nicht gleichbedeutend als Ursache erklärt.

Das Handbuch trennt Infantry/Medium/Large-Navigation, Square-Platzierung und Air-Navigation (manual S. 12). NoStructures und weitere Blockerflächen sind dokumentiert. Daraus folgt keine Pflicht, HeroHours funktionierendes A* durch NavGrid zu ersetzen. Bebaubarkeit und Begehbarkeit müssen dennoch getrennt geprüft werden. Fog-of-War-Zustände müssen außerdem Befehle, Zielsuche, Minimap und bekannte Ressourcenmengen beeinflussen; ein dunkles Overlay allein erfüllt diese Systemanforderung nicht.

## Byte-Anker und Aussagekraft

Die beiden echten Funktionskörper wurden zusätzlich über REA `address_to_file_offset` und `read_bytes` auf Originaldateibereiche bezogen und unabhängig mit Python erneut verglichen:

| Funktion | Dateioffset / Länge | Vergleich |
|---|---|---|
| `GetResourceLeft` | 935.920 / 200 Bytes | Identisch; Bereich-SHA-256 `e5da3cf119344f8d95e714099c2b126fdc85adffd3a8cbdb1e5cf34b605402ef` |
| `IsFieldTileIdxClaimed` | 624.208 / 102 Bytes | Identisch; Bereich-SHA-256 `028b2527985fce2191b750f6ddd4831f89aadee2cd077b0515250558e1b259c5` |

Reproduzierbar: `Saved/Research/TempestRising/verify_editor_loaded_bytes.py`; Ergebnis: `editor-loaded-byte-comparison-20261014.json`. Dies ist ausschließlich ein Nachweis, dass die analysierten Speicherbytes diesen unveränderten Originaldateibereichen entsprechen. **Es ist kein Vergleich neu kompilierten Quellcodes, kein Shipping-Match und kein Laufzeit-Paritätsnachweis.**

Die [awesome-rea-Aufnahmeregeln](https://github.com/yibie/awesome-rea#inclusion-criteria) verlangen ein benanntes Prüfverfahren. [CONTRIBUTING](https://github.com/yibie/awesome-rea/blob/main/CONTRIBUTING.md) unterscheidet bytegenaue Matching-Decompilation von verhaltensbezogenen Ports. Ein Originalhash bestätigt Identität; er bestätigt keinen Nachbau. Das verlinkte [decomp-compat-kit](https://github.com/LinuxJessi/decomp-compat-kit) richtet sich an PS2/ee-gcc2.96 und ist kein fertiger Win64/Unreal-Prüfer.

## Vorhandene Laufzeitaufnahme

`Saved/Research/TempestRising/reference-live-base.png` wurde visuell gelesen. Die Aufnahme zeigt bei 2:08 einen großen Gebäudekomplex, ein Ressourcengebiet, Einheiten, eine rechte Sidebar mit Bauen/Doktrinen und Produktionskategorien sowie eingeschränktes Radar. Sie zeigt **999.999 Credits**; zusammen mit den vorher erfassten Einstellungen ist diese Aufnahme für Ernte-, Preis- oder Refundmessungen ungeeignet. Sie belegt weder Claim-Eigentümer noch eine Queue-Transaktion. Es wurde keine neue Szene bedient und kein neuer dynamischer Vergleich durchgeführt.

## Konkrete eigene Integrationsregeln

1. **M1 Ressourcenfeld:** `ResourceDefinition` beschreibt Zellkapazität, eigenen Wachstumstakt und Erntemenge; Instanzzustand trägt Restmenge, Wachstumsstadium und Reservierung. Gesamtsumme und sichtbare Summe sind getrennte Abfragen. Claim-Eigentümer und Freigabegründe gestalten und testen wir selbst.
2. **M1 Andocken:** Pickup- und Raffinerie-Dockreservierungen sind getrennt. Nur tatsächliche Lieferung zahlt aus; Tod, manuelle Abfahrt, Gebäudezerstörung und Wegfehler lösen Reservierungen. Diese Regeln stammen aus unserem Entwurf und C&C-Quellvergleich, nicht aus rekonstruierter Tempest-Dockinglogik.
3. **M1/M2 Befehle:** Unterstützt, aktuell erlaubt und erfolgreich ausgeführt unterscheiden. Vorschau, Sidebar und Commit nutzen denselben Validator. Gruppenbefehle brauchen eine deklarierte Verteilung und ein Ergebnis je betroffener Einheit.
4. **M2 Produktion/Strom:** Queue-Kategorien und Produzentenfähigkeit sind Daten. Preis wird beim festgelegten Zahlungspunkt gespeichert. Strommangel verändert einen eigenen, ausdrücklich getesteten Produktionsfaktor; manueller Shutdown hält Fortschritt. Ein blockierter Ausgang darf weder neu bezahlen noch einen Auftrag verlieren.
5. **M2 Baufläche/Sicht:** Rastergröße, gedrehter Footprint, Begehbarkeit, Bebaubarkeit und Konnektivität getrennt modellieren. Nebel verhindert Informationslecks in Minimap, Auswahl und Zielsuche. Die aktuelle A*-Navigation bleibt zunächst nutzbar.

Diese Regeln sind keine Behauptung, HeroHour sei bereits gleichwertig mit Tempest. Die aktiven HeroHour-Systeme müssen die neuen Definitionen tatsächlich konsumieren. Das alte HeroHour-Modul und die unvollständige F#-Simulation sind weiterhin inaktiv.

## Wiederholbares Referenz-Abnahmeprotokoll

Das Protokoll ist vorgeschlagen und noch nicht ausgeführt. Die aktuelle Installation und Pfade sind bekannt; zum Weiterbauen werden keine zusätzlichen Dateien des Nutzers benötigt. Die späteren Vergleichsszenarien benötigen eine festgehaltene Fraktion, Karte, Startcredits, Startarmee, Schwierigkeitsstufe und aktivierte Regeln. Für Wirtschaftsprüfung **unbegrenzte Credits ausschalten**; Einstellungen und Referenz-EXE-Hash im Protokoll speichern.

| Szenario | Referenzaktion / Aufzeichnung | Eigene HeroHour-Abnahme | Noch offen |
|---|---|---|---|
| R1 Einzel-Sammler | Ein Feld, eine Raffinerie; Cargo-/Creditstand und Zeit vor Ernte, nach Füllung und nach Lieferung festhalten; drei Runden. | Keine Gutschrift vor Lieferung; Ressourcen-/Cargo-/Credit-Erhaltung unter eigener Umrechnung. | Tempest-Raten, Capacity, Wertumrechnung und exakter Zahlungszeitpunkt. |
| R2 Konkurrenz und Unterbrechung | Vier Sammler auf dasselbe Feld; einen manuell wegfahren lassen; getrennt Sammler/Raffinerie verlieren. Positionen, Stillstand und Wiederaufnahme protokollieren. | Keine doppelte Reservierung oder Ressource; Claim-/Dockfreigabe und automatisches Fortsetzen. | Interner Eigentümer, Wartepolitik und Freigabezeit; sichtbarer Ablauf beweist sie nur teilweise. |
| R3 Queue und Refund | Drei identische Aufträge; Startcredits/Queue/Fortschritt notieren; zuletzt eingereihten und aktiven Auftrag getrennt abbrechen. | Jeweils genau dokumentierte Zahlung und Erstattung, keine doppelte Gutschrift. | Tempest-Abbruchbedienung und tatsächlicher Erstattungssatz. |
| R4 Geblockter Ausgang | Fabrikausgang belegen, Produktionsende abwarten, Ausgang wieder frei machen. | Bezahlter Auftrag erhalten, freier Spawn und validierter Rally-Befehl; kein Kostenverlust. | Tempest-Spawn-/Warteverhalten. |
| R5 Strommangel/Shutdown | Gleiche Produktion unter voller Versorgung, zu wenig Versorgung und manuellem Abschalten; Fortschritt alle Sekunde festhalten. | Eigene Zustandsübergänge, Faktor und Fortschrittserhalt; Radar/Defense reagieren konsistent. | Tatsächliche Tempest-Faktoren und Prioritätsregeln. |
| R6 Baufläche/Fog | Rechteckiges Gebäude drehen; verbundenen/isolierten Rand, Brücke und verborgenes Gebiet prüfen; Gegner aus Sicht verlieren. | Identische Ghost-/Commitprüfung, atomare Belegung, kein verborgenes Ziel in HUD/Minimap/Autofeuer. | Sicht-/Radarregeln und Bauradius dieser konkreten Fraktion. |

Für einen behaupteten Verhaltensvergleich werden Version/Hash, Startzustand, Aktionen, Ergebnisse und Toleranzen **vorher** definiert. Nicht gemessene Fälle bleiben ausdrücklich offen. Screenshots belegen sichtbare Zustände; für Transaktionen sind mehrere Messpunkte nötig. Der eigene Pakettest bleibt ein separater Nachweis und darf nicht als Byte-Matching des Originals bezeichnet werden.

## Behaltene Ergebnisse

- `Saved/Research/TempestRising/commands-systems-evidence-20261014.json`: 3 vollständige Records.
- `Saved/Research/TempestRising/editor-systems-evidence-20261014.json`: 18 vollständige Records.
- Entsprechende `commands-systems-snapshot-20261014.json` und `editor-systems-snapshot-20261014.json` erlauben Wiederverwendung bei passender Identität und Analyseprofil.
- Originale Seitenextraktionen `api-pages.json` und `manual-pages.json` wurden weiterverwendet.
- Neue native Sitzungen wurden geschlossen. Shipping-Gameplay, Paketwerte und Runtime-Parität sind weiterhin unbestätigt.
