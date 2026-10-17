# Tempest: begrenzte Sammler-Untersuchung V8

Stand: 16.10.2026. Der Folgeagent erreichte während der Untersuchung ein Nutzungslimit. Der Hauptagent hat die gespeicherten Belege geprüft, die beiden direkten Raten-Zielroutinen zusätzlich verifiziert und diesen Zwischenbericht abgeschlossen. Die REA-Sitzung ist geschlossen; kein Zielspiel wurde gestartet.

## Ergebnis

Die Shipping-Datei `Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe` aus der ausdrücklich genannten Installation besitzt SHA-256 `c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56`. Aussagen dieses Berichts gelten ausschließlich für diese Datei.

- **Beobachtet:** Ein begrenzter Decoder findet generierte Metadaten für `HarvesterData` mit 16 Byte Größe: `ResourceHarvestSpeed`, `ResourceDepositSpeed`, `ResourceMaxCapacity` und `AddResourceFrequency` an den relativen Offsets 0/4/8/12. Die ersten drei Typkennungen sind Int32-Kandidaten, die letzte ein Float-Kandidat. Typkennungen wurden gegen den bisherigen Decoder desselben Builds abgeglichen; das ist keine allgemeine Unreal-ABI-Freigabe. Konfigurierte Werte sind damit nicht bestimmt.
- **Beobachtet:** Für `HarvesterAlignmentData` ist ein 224 Byte großer, 16-Byte-ausgerichteter generierter Strukturdeskriptor mit einer reflektierten `Unit`-Referenz belegt. Daraus ergibt sich weder der Aufbau aller privaten Felder noch ein konkreter Dock-Transform.
- **Beobachtet:** Die zum Namen `SetResourceHarvestSpeed` gehörende untersuchte Exec-Kandidatenroutine ruft bei RVA `0x4B965FE` den Zielbereich `0x51B11D0` auf. Dort stehen exakt `mov dword ptr [rcx+0x2140], edx; ret`. Der entsprechende Deposit-Kandidat ruft bei `0x4B9656E` nach `0x51B11C0`: `mov dword ptr [rcx+0x2144], edx; ret`. Jeweils sieben Originalbytes stimmen mit dem installierten LLVM-Decoder überein; anschließend folgen neun INT3-Ausrichtungsbytes. Die kleinen A-Kandidaten aus den Namenspaaren sind Konstruktor-/Registrierungshelfer, keine Ernteschleife.
- **Ableitung:** Getrennte schreibbare Ernte- und Ablagewerte sind mit diesen Aufruf-/Speicheroperationen vereinbar. Die benachbarten Felder passen zur Metadatenreihenfolge. Vollständige Klassennestung, Wertebereiche, Einheiten oder ihre Verwendung in der Simulationsschleife sind nicht bewiesen.
- **Offen:** Codebelegte HarvestStart/Loop/Stop-Animationsübergänge, Clipdauer, tatsächlicher Abspielzeitpunkt, Dock-Ausrichtungsregeln und konfigurierte Abladeraten. Die bisherige Suche lieferte keine passend benannten nativen Editorfunktionen. Das schließt Blueprint- oder anders benannte Implementierungen nicht aus. Namen wie `QueueSpline`, `DepositSlots`, `HarvesterDistanceTolerance` und `HarvesterPositionBounds` sind zunächst Suchansätze, keine nachgewiesenen Laufzeitwerte. Unversionierte Animationsdateien werden ohne passendes Schema nicht nach beliebigen Floats als Sekunden interpretiert.

## Eigene Umsetzung daraus

HeroHour besitzt bereits getrennte autoritative Ernte-, Dock- und Abladezustände. Diese bleiben unabhängig von Animation und Bildrate. Nächster sichtbarer Schritt ist ein eigener Sammler mit Start-/Arbeits-/Stopdarstellung und separatem Entladen. Die konkrete Bewegung und Dauer werden für unsere eigenen Assets gestaltet; keine vermeintlichen Originalsekunden aus den gefundenen Namen übernehmen. Eine echte Aufnahme eines vollständigen Feld→Raffinerie→Feld-Zyklus ist dafür der relevante Nachweis.

## Reproduktion und Grenzen

`Scripts/research/verify_harvester_rate_targets_v8.py` prüft SHA, PE-Zuordnung, zwei tatsächliche CALL-Ziele, die beiden kurzen Store/RET-Körper und die dekodierten Bytes. Es liest die Originaldatei nur. Erfolgsmarker: `HARVESTER_RATE_TARGETS_V8_PASS 2 leaf stores, 14 bytes; no runtime-rate claim`.

Die kompakten Belege stehen unter `docs/verification/2026-10-16/harvester-v8/`. Vollständige lokale REA-/Editor-Snapshots und Assemblervorschauen bleiben unter `Saved/Research/TempestRising/V8/`. Lokale LLVM-/PE-Prüfungen besitzen Datei-/Bereichs-SHAs; ihnen werden keine erfundenen REA-Evidence-IDs zugewiesen. Byteübereinstimmung ist keine statische Rekompilierungsabnahme und keine Beobachtung laufenden Spielverhaltens.
