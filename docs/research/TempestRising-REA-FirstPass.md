# Tempest Rising: Untersuchung als HeroHour-Referenz

Untersuchung am 14. Oktober 2026. Vom Nutzer ausgewählter Einstieg: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\Tempest.exe`. Untersucht werden lokale Artefakte; eine Zuordnung dieser Installation zu einer unveränderten offiziellen Veröffentlichung oder einem bestimmten Patch ist nicht bestätigt. Es wurde kein Spiel gestartet und kein fremder Spielinhalt in HeroHour übernommen.

## Fragen und Belegarten

| Frage | Geeigneter Beleg | Stand |
|---|---|---|
| Welche Datei enthält den Spielcode? | PE-Ressourcen, Dateiinventar, native Analyse | Startziel in Bootstrap-Ressource identifiziert |
| Welche Sammler- und Produktionszustände sind vorgesehen? | Konfiguration, API-Handbuch, native Referenzen | Teilweise beantwortet; dokumentierte API, Editor-Platzhalter statt Shipping-Algorithmus |
| Welche Bedien- und Bauregeln sind vorgesehen? | Command-Tags, dokumentierte API | Dokumentierte Befehlsmodi beantwortet; konkrete Tasten und Refund offen |
| Wie werden Wege und Gelände abgebildet? | NavGrid-Konfiguration, dokumentierte Kartenregeln | Dokumentierte Flächen und Einschränkungen beantwortet; Suchalgorithmus offen |
| Welche Regeln eignen sich für HeroHour? | Eigene Ableitung aus belegten Befunden | Priorisierte Empfehlungen unten; keine Umsetzung in diesem Schritt |

## Zielidentität und Bootstrap

REA 6.4.0 mit Ghidra 12.1.4 erkennt beide ausführbaren Dateien als native PE/x86-64. Die originale `Tempest.exe` hat 235.520 Bytes und SHA-256 `932488fb015d21829637b00d931303f99251e3d44f3389bcd9723ad35c590af5`. Ghidra zählt dort 490 Funktionen und 328 definierte Strings; `.text` umfasst 66.560 Bytes. Übersicht: `ev_7f1f65b3e4579e8e96bf366b529df742ad26096a1a3317c6bbe65109d2dafb60`.

Die REA-PE-Ressourcenprüfung findet 13 Ressourcen: `ev_bc8314767f5c9442641ce0138bdc6dce7ef4e507fde6cc879009220619eb1f63`. Ressource RT_RCDATA/201/1033 liegt bei Dateioffset 230624, Länge 100 Bytes. Ergänzende UTF-16LE-Decodierung dieser unveränderten Bytes ergibt `Tempest\Binaries\Win64\Tempest-Win64-Shipping.exe`. RT_RCDATA/202/1033 bei Offset 230724, Länge 16, ergibt `Tempest`. Stringsuche findet außerdem `CreateProcessW` und `BootstrapPackagedGame-Win64-Shipping.exe`: `ev_ade8fbcb9ef943da0f319b2dcfd4504d22c45d2d68cd1678d15e0d6a62f4a784`.

Damit ist das deklarierte Startziel statisch belegt. Ein tatsächlich beobachteter Prozessstart ist damit nicht nachgewiesen. Die Zieldatei existiert mit 166.518.344 Bytes und SHA-256 `c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56`.

## Mitgelieferte Konfiguration und Dokumentation

Die Installation enthält `MapEditorProject` mit Config, Content, vorkompilierten Modulen, Plugins und vier PDF-Handbüchern. README nennt UE 5.4.4 für dieses Editorprojekt. Das ist kein sicherer Beleg für die genaue Engineversion des separat ausgelieferten Shipping-Programms.

- [Konfigurationsprüfung](TempestRising-ConfigReview.md): Command-/Ability-Tags, Bau- und Energiezustände, Eingabekonfiguration sowie NavGrid-Oberflächen mit Zeilenbelegen.
- [Karteneditor und API](TempestRising-MapEditorReview.md): dokumentierte Funktionen und Kartenregeln mit Seitenbelegen.

Es wurde kein offenliegendes C++-Source-Verzeichnis in der begrenzten Prüfung der Editorprojektwurzel und ihrer Plugins gefunden. Deklarationen, API-Dokumentation und native Implementierung bleiben getrennte Belegarten.

## Native Untersuchung: tatsächliche Grenze des Editor-Moduls

Die erste gezielte Stringsuche in der 166-MB-Shipping-EXE wurde nach 300 Sekunden vom Codex-MCP-Client mit `timed out awaiting tools/call after 300s` beendet. Dies ist eine Zeitgrenze der Verbindung, kein Beleg für fehlende Funktionen oder eine gescheiterte Formatprüfung. Es liegt daraus kein erfolgreiches Shipping-Funktionsresultat vor. Die Sitzung wurde geschlossen und die kleinere mitgelieferte Editor-DLL separat analysiert.

`MapEditorProject\Binaries\Win64\UnrealEditor-Tempest.dll` hat 3.106.304 Bytes, SHA-256 `2fff65ec4f3809e8f24010d5cbfa9ada2f2834b59c997569c219d6a494f0b4fa`, PE/x86-64. Ghidra findet Namen für Ressourcenfeldsuche, Enqueue und Sammelpunkte. Stringbeleg: `ev_2b09b977918989341734785b360f751e1c503d47ed5e3ce9c92597018c719f4a`.

Zwei anhand der tatsächlich gefundenen Funktionsnamen ausgewählte Funktionen wurden dekompiliert:

| Funktion und Adresse | Beobachteter Funktionskörper | Evidence |
|---|---|---|
| `ATempestFieldVolume::GetClosestUnclaimedResourceTile`, `0x1800e03f0` | 6 Bytes, Rückgabe konstant `-1`; kein Suchalgorithmus im geprüften Körper | `ev_66dd071601cc1a5303f2a09c5612e359ad804937a5d183c21949d9801ac9c498` |
| `ATedStructureBase::CanSpawnUnit`, `0x18003c550` | 3 Bytes, Rückgabe konstant `false`; keine Spawn-Prüfung im geprüften Körper | `ev_5688b9b8ea62625d2047c159e48d01bffb1a9abdccc7f58343cc821309e16f7c` |

Diese zwei Körper sind mit Platzhaltern für die Editor-Auslieferung vereinbar. Die Beobachtung gilt ausschließlich diesen Funktionen in dieser DLL. Weder ist damit die gesamte DLL als Platzhalter klassifiziert, noch ist daraus ein fehlerhaftes Verhalten des Shipping-Spiels ableitbar. Insbesondere darf das dokumentierte API-Verhalten nicht als erfolgreich rekonstruierter nativer Algorithmus ausgegeben werden.

## Paketgrenze

Das Spiel verwendet `.pak` und IoStore-Dateien `.utoc/.ucas`; das Haupt-UCAS hat 23.809.114.176 Bytes. Eine ausschließlich lesende Auflistung von `pakchunk0-Windows.pak` mit dem vorhandenen UnrealPak aus UE 5.8 scheiterte mit `Failed to find requested encryption key`. Es wurden weder ein Schlüssel gesucht noch Paketdaten extrahiert. Deshalb sind Shipping-Datenassetwerte, Balancezahlen und Blueprint-Implementierungen aus diesem Paket in dieser Untersuchung nicht belegt.

## Nachweise und Grenzen

Vollständige lokale REA-Antworten und ergänzende Inventare stehen unter `Saved/Research/TempestRising`; dieser Cache wird nicht ins Git-Repository aufgenommen. Native Strings belegen vorhandene Namen, nicht die Ausführung oder Semantik einer Spielregel. Karteneditor-Daten können vom Shipping-Build abweichen. Laufzeit-Bedienung, exakte Kosten, Erstattungen, Produktionszeiten und Leistungsmessungen benötigen zusätzliche Belege.

## Prioritäten für HeroHour nach dieser Untersuchung

1. **Sammlerstellen reservieren:** Ziel vor Annahme auf Erreichbarkeit prüfen; Reservierung bei Zielwechsel, Tod, Erschöpfung und Wegfehler freigeben. Grundlage: API S. 181–182. Konkurrenzauflösung und Suchkosten müssen wir selbst gestalten und testen.
2. **Befehle zentral validieren:** Unterstützung, aktuelle Ausführbarkeit und Ablehnungsgrund trennen; HUD-Vorschau und tatsächliche Ausführung verwenden dieselben Regeln. Grundlage: API S. 104–108, 116–118, 184.
3. **Sammelpunkte und klare Abbruchbedienung:** aktuellen Auftrag abbrechen und Warteschlange leeren separat behandeln. Produktionsstorno/Erstattung anhand eigener Regeln festlegen; Tempest-Refund ist nicht belegt.
4. **CityHero-Kartenprüfung:** Startpositionen, Ressourcenanschlüsse, Fabrikausgänge, Brücken und getrennte begehbare/bebaubare Flächen prüfen. Grundlage: manual S. 12–17, 28. Das vorhandene A*-Grid kann dafür zunächst bleiben.

Ein Stromsystem, Fog of War oder reparierbare Brücken wären spätere eigene Erweiterungen. Sie sind für diese vier Schritte keine Voraussetzung. Genaue Balance, Shipping-Sammlerzustandsmaschine, Produktionsrefund, KI-Buildorder und gemessene Laufzeitbedienung bleiben offen. Für einen nächsten nativen Shipping-Durchlauf ist ein längerer Client-Timeout oder der vorhandene CLI-Wrapper erforderlich; die verschlüsselten Paketwerte bleiben davon getrennt.

HeroHour-Code und Windows-Build wurden während dieser Referenzuntersuchung nicht geändert. Die Berichte und ihre Verlinkung liegen lokal; sie wurden nicht automatisch committed oder gepusht.
