# Zero Hour: erster belegter REA-Durchlauf

Stand: 14. Oktober 2026. Dies ist eine begrenzte statische Untersuchung der vom Nutzer ausgewählten Installation, ergänzt durch die offizielle EA-Quelle. Die Vollrekonstruktion des Spiels und Messungen einer laufenden Partie sind nicht Bestandteil dieses Durchlaufs.

## Identität der untersuchten Dateien

| Datei | SHA256 | REA/Ghidra-Beobachtung |
| --- | --- | --- |
| Generals.exe | `ff6f78211a014100d8ef6b08bc2f8edd3d55e99e872dfdb5371776fc5a5d02ce` | x86 PE; 34 erkannte Prozeduren, 179 definierte Strings |
| game.dat | `253feba0a5503cb4d49fd07463b17d3cc84731e583f9625cb90fcd8b5cac0221` | x86 PE; 35.300 erkannte Prozeduren, 11.900 definierte Strings |

Die DAT-Datei besitzt einen Codeabschnitt von 5.513.216 Bytes; die EXE einen von 77.824 Bytes. Daraus folgt, dass die EXE allein die Spielanalyse stark begrenzt. Eine konkrete Start-/Ladekette zwischen den Dateien wurde nicht rekonstruiert. Funktions- und Stringzahlen sind Ghidra-Inventarbeobachtungen, keine garantierte Gesamtanzahl aller logischen Methoden oder möglichen Texte.

Belege:

- EXE-Übersicht: `ev_e35f7512216f95ee2d7342b207597e03c99781f11074185eeaa846c878089aec`, `Saved/Research/ZeroHour/rea-launcher-overview.json`.
- DAT-Übersicht: `ev_e278aec7dfb66f923de0a6e319fa555427a11ed57a8a2b6d1e538390891a5996`, `rea-game-overview.json` im selben Verzeichnis.
- Unabhängige REA-PE-Ressourcenprüfung bestätigt die Dateihashes und PE32-Kennungen.

## Gezielte native Belege

Die Suche nach sechs thematischen Begriffen liefert unter anderem folgende definierte Strings im tatsächlich installierten `game.dat`:

| Adresse | Befund |
| --- | --- |
| `0x9e12d4` | ResourceGatheringManager |
| `0x9e9868` | ProductionUpdate |
| `0x9e9ae4` | SupplyTruckAIUpdate |
| `0xa0c054` | SupplyTruckAIUpdateMachine |
| `0x9e22dc` | Data\INI\CommandSet.ini |
| `0x9e22f4` | Data\INI\CommandButton.ini |

Suchbeleg: `ev_e12b9477521d3ad9c9458e9e4b97cf273e7280e6492e52678aa9b5d1454c58e5`, `mcp-02-search_strings.json`. Damit ist die Anwesenheit dieser Namen beobachtet. Eine vollständige Übereinstimmung mit den gleichnamigen Klassen in der veröffentlichten EA-Quelle ist daraus allein nicht abzuleiten.

Eine Referenz auf `ResourceGatheringManager` bei `0x452e50` liegt innerhalb der von Ghidra beobachteten Funktion `0x452e30` bis `0x452e6f`. REA liefert deren vollständigen 64-Byte-Funktionskörper, Assembler, Pseudocode, Referenzen und Einschränkungen. Der Pseudocode zeigt einen durch ein Datenflag geschützten Initialisierungspfad, der den beobachteten Namen an eine weitere Routine übergibt. Dieser Befund identifiziert noch nicht den Algorithmus zur Auswahl einer Rohstoffroute.

Funktionsbeleg: `ev_c42ece78701c6b7a034411d2247621445e22c62556349fef6d920a02754d2f04`, `mcp-09-analyze_function.json`. Referenzbeleg: `ev_5fb7ae1c3fc9be8b7eeb612b7d3dcfe70b452f204e09d932220eea29428f8e05`.

Bei der Sammler-Zustandsmaschine reichen die beobachteten Referenzen noch nicht zur zuverlässigen Zuordnung eines vollständigen Funktionskörpers. Ein zunächst untersuchter benachbarter Funktionskandidat enthält die Referenzadressen nicht in seinem deklarierten Körper und wird deshalb nicht als Sammlermechanik gewertet. Zusätzlich wurde ein Aufruf mit einer Referenzadresse statt einem Funktionseinstieg korrekt vom Provider abgelehnt. Diese Grenzen bleiben als Befunde erhalten.

Ein erster MCP-Stringaufruf endete am SDK-Requesttimeout. Der Projektadapter wurde auf die tatsächlich installierte SDK-2-Aufrufsignatur und ein begrenztes 600-Sekunden-Timeout korrigiert. Die erneute Suche lieferte anschließend vollständige Evidence. Ghidra benötigt eine abgeschlossene Standardanalyse, bevor diese Inventare ausgegeben werden.

## Ergänzende installierte Konfiguration

Ein eigener begrenzter BIG-Parser liest die Verzeichnisse von `INIZH.big` und `PatchINI.big`; er schreibt numerische Fakten mit Archiv- und Mitgliederhashes nach `installed-balance-facts.json`. Beispielhafte Rohwerte aus dem Basisarchiv:

| Objekt | BuildCost | BuildTime | MaxBoxes |
| --- | ---: | ---: | ---: |
| AmericaVehicleDozer | 1000 | 5 | — |
| AmericaVehicleChinook | 1200 | 10 | 8 |
| ChinaVehicleSupplyTruck | 600 | 10 | 4 |
| GLAInfantryWorker | 200 | 3 | 1 |
| AmericaSupplyCenter | 2000 | 10 | — |
| ChinaSupplyCenter | 1500 | 10 | — |
| AmericaWarFactory | 2000 | 15 | — |
| GLAArmsDealer | 2500 | 15 | — |

Dies sind gelesene INI-Felder, keine Laufzeitmessungen. Vererbung, Patchpriorität, Generalvarianten und Spielerboni sind nicht aufgelöst. Die Werte sind Referenzdaten und wurden nicht als HeroHour-Balancing übernommen.

## Anwendung auf HeroHour

Die offizielle Quellenprüfung belegt Spielerbefehle als Teil des Sammler-Zustandsmodells. Darauf basiert eine eigenständige HeroHour-Verbesserung: manuell befohlene Fahrten behalten Vorrang, bis ihr gültiges Ziel erreicht ist; danach läuft die Logistik weiter. Ungültige Ziele zerstören den bisherigen Auftrag nicht. Native Stringbelege dienen als zusätzlicher Untersuchungsanker; sie werden nicht als Beweis für das vollständige Laufzeitverhalten ausgegeben.

Der aktualisierte Windows-Build enthält die Änderung. Paketierung `Packaging-20261014-003521-077.log`: BUILD SUCCESSFUL, Exitcode 0. Pakettests: 15 Kampf-, 28 Wirtschafts-, 10 Navigationsprüfungen und drei vollständige simulierte Siege bestanden. Die Wirtschaftsprüfungen enthalten gezielte Tests für manuelle Sammlerbefehle. Quelle, Binärbelege und Tests liegen getrennt vor.

Als nächste abgegrenzte Aufgaben folgen kontextbezogene Befehlsmenüs, Produktionsabbruch/Erstattung, Sammelpunkte und Kontrollgruppen. Sie werden jeweils als eigener HeroHour-Entwurf mit Tests umgesetzt. Verknüpfte Berichte: [Mechaniken](ZeroHour-Mechanics-SourceReview.md), [Bedienung](ZeroHour-Controls-SourceReview.md), [REA-Einrichtung](REA_WORKFLOW.md).
