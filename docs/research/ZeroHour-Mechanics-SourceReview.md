# Zero Hour: Quellcodeprüfung von Wirtschaft, Produktion und Wegfindung

Stand: 14. Oktober 2026. Zweck: belegte Mechaniken aus der offiziellen EA-Veröffentlichung verstehen und daraus eigene Entwicklungsaufgaben für HeroHour ableiten.

## Herkunft und Grenzen

Geprüft wurde ein lokaler Sparse-Checkout von [electronicarts/CnC_Generals_Zero_Hour](https://github.com/electronicarts/CnC_Generals_Zero_Hour) unter `Saved/Research/ZeroHourSource`. `git rev-parse HEAD` ergibt **0a05454d8574207440a5fb15241b98ad0b435590**. Alle nachfolgenden Belege beziehen sich auf diesen Commit und den Zweig des veröffentlichten Codes unter `GeneralsMD/Code/GameEngine`.

Dies ist eine gezielte, statische Quellcodeprüfung ausgewählter Funktionen. Es ist **keine vollständige Reverse-Engineering-Analyse** von Generals oder Zero Hour. Keine Spielbinärdatei wurde hier disassembliert, mit Ghidra untersucht, instrumentiert oder dynamisch abgeglichen. Ein nativer Installationspfad und eine entsprechende Analyseumgebung sind noch nicht Bestandteil dieser Prüfung. Der veröffentlichte Code allein belegt weder die genaue Version einer vorhandenen EXE noch deren Compilerkonfiguration, Patchstand, INI-Werte oder tatsächlich gemessene Laufzeitmechanik.

Im Referenzbaum und seinen lokalen Eltern `Saved/Research` und `Saved` wurde keine `AGENTS.md` gefunden. Die Referenzdateien wurden nicht verändert. HeroHour-C++ wurde für diesen Auftrag nicht geändert; es wurde kein EA-Code übernommen.

## Beobachtungen in der Quelle

### 1. Sammler besitzen einen eigenen Zustandsautomaten

**Beobachtet:** `SupplyTruckStateMachine::SupplyTruckStateMachine`, `SupplyTruckAIUpdate.cpp`, Zeilen 398–442, definiert Zustände für beschäftigt, untätig, Beschaffung/Lieferung, Rückzug zu einer Basis und Andocken. Übergänge berücksichtigen auch Spielerbefehle. Die autonome Wirtschaft ist dadurch nicht nur eine Entscheidung anhand von Füllstand und Entfernung. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate/SupplyTruckAIUpdate.cpp#L398-L442)

**Beobachtet:** `SupplyTruckWantsToPickUpOrDeliverBoxesState::update`, Zeilen 511–553, sucht bei vorhandenen Kisten ein Versorgungszentrum und ansonsten ein Rohstofflager. Ein gültiges Ziel erhält einen Andockauftrag. Ein fehlendes Ziel beendet diesen Zustand mit einem Fehler; `RegroupingState::onEnter`, ab Zeile 557, sucht anschließend unter anderem ein Versorgungsgebäude oder ein Kommandozentrum. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate/SupplyTruckAIUpdate.cpp#L511-L605)

**Einordnung für HeroHour:** explizite Zustände können Lade-, Liefer-, Warte- und Fehlerverhalten sichtbar und testbar machen. Das ist ein Vorschlag für unsere eigene Architektur, keine Aussage über untersuchte EXE-Zustände.

### 2. Zielwahl ist weniger komplex, als Kommentare vermuten lassen

**Beobachtet:** `computeRelativeCost`, `ResourceGatheringManager.cpp`, Zeilen 108–136, verwirft unzulässige Transfers und Ziele ohne freie Anfahrt und bewertet danach die quadrierte räumliche Entfernung. Pfadberechnung, Bedrohungen und andere Sammler werden an dieser Stelle als zukünftige Arbeit beschrieben; sie sind nicht Teil dieser berechneten Bewertung. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/Common/RTS/ResourceGatheringManager.cpp#L108-L136)

**Beobachtet:** `findBestSupplyWarehouse`, Zeilen 139–198, bevorzugt ein vom Sammler gespeichertes geeignetes Dock, berücksichtigt andernfalls eine Suchreichweite und räumt ungültige Lager-IDs auf. `findBestSupplyCenter`, ab Zeile 200, besitzt ebenfalls eine bevorzugte Dock-Auswahl. Es wäre falsch, daraus bereits eine optimierte Gesamtroute oder Lastverteilung abzuleiten. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/Common/RTS/ResourceGatheringManager.cpp#L139-L257)

### 3. Laden und Auszahlung sind getrennte Transaktionen

**Beobachtet:** `SupplyWarehouseDockUpdate::action`, Zeilen 93–139, prüft Vorrat und Nähe, reduziert den Lagerbestand vor dem Ladeversuch und stellt ihn bei fehlgeschlagener Übernahme wieder her. Ein konfigurierbares Verhalten kann ein erschöpftes Lager entfernen. Der Abschnitt enthält außerdem eine zufällige Positionskorrektur bei zu großer Andockentfernung; dies ist keine allgemeine robuste Navigationslösung. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/DockUpdate/SupplyWarehouseDockUpdate.cpp#L93-L139)

**Beobachtet:** `SupplyCenterDockUpdate::action`, Zeilen 88–114, entfernt transportierte Kisten, berechnet deren Wert einschließlich eines möglichen Bonus und schreibt einen positiven Betrag dem Eigentümer des Versorgungszentrums gut. Dieser Mechanikpfad zahlt bei Ablieferung und nicht schon beim Laden. Daraus folgt keine Behauptung, dass das gesamte Spiel keinerlei andere Einkommensquellen hat. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/DockUpdate/SupplyCenterDockUpdate.cpp#L88-L114)

### 4. Mehrere Sammler reservieren Anfahrplätze

**Beobachtet:** `DockUpdate::reserveApproachPosition`, Zeilen 136–188, verwendet Platzbesitzer-IDs. Ein Sammler erhält seinen vorhandenen oder einen freien Platz; dynamische Docks können weitere Plätze hinzufügen. `advanceApproachPosition`, ab Zeile 190, verschiebt einen Sammler erst in einen frei gewordenen vorherigen Platz. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/DockUpdate/DockUpdate.cpp#L136-L214)

**Einordnung für HeroHour:** das ist ein geeigneter Ausgangspunkt für ein eigenständig entworfenes Andocksystem. Unsere aktuell nur auf Gebäudeabstand basierende Lieferung bietet noch keine vergleichbare Platzreservierung.

### 5. Produktion trennt Zahlung, Warteschlange und tatsächlichen Austritt

**Beobachtet:** `ProductionUpdate::queueCreateUnit`, Zeilen 387–460, prüft die Bauberechtigung, behandelt gegebenenfalls Ausgangsreservierungen, begrenzt die Warteschlange, zieht die Baukosten ab und hängt einen identifizierbaren Auftrag an. Mengenmodifikatoren sind möglich; der geprüfte Code allein belegt keine konkreten Werte aus den Spieldaten. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/ProductionUpdate.cpp#L387-L460)

**Beobachtet:** `cancelUnitCreate`, Zeilen 467–489, sucht den Auftrag über seine Produktions-ID, erstattet die zu diesem Zeitpunkt berechneten Baukosten und entfernt ihn. Nicht behauptet wird, dass hierbei ein separat gespeicherter ursprünglicher Zahlungsbetrag erstattet wird. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/ProductionUpdate.cpp#L467-L489)

**Beobachtet:** `ProductionUpdate::update`, ab Zeile 611, verarbeitet den vordersten Auftrag. Ab Zeile 711 wird Fortschritt in Logikframes berechnet. Ab Zeile 742 beschreibt und behandelt der geprüfte Abschnitt die Freigabe eines fertigen Auftrags durch das Ausgangssystem; ein nicht verfügbarer Ausgang hält den Auftrag zurück, statt die Einheit blind zu platzieren. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/ProductionUpdate.cpp#L611-L758)

### 6. Wegfindung umfasst mehr als eine ungefilterte Rasterroute

**Beobachtet:** `Path::optimize`, `AIPathfind.cpp`, Zeilen 474–531, glättet Wege anhand von Passierbarkeitsprüfungen und behandelt Übergänge zwischen Boden- und Brückenebenen besonders. Brücken sind hier nicht lediglich eine Farbflecke auf einer zweidimensionalen Karte. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp#L474-L531)

**Beobachtet:** `PathfindCell::costToGoal` und `costSoFar`, Zeilen 1678–1740, berechnen Ziel- und bisherige Kosten; diagonale und orthogonale Schritte unterscheiden sich, enge Zellen und Richtungswechsel können zusätzliche Kosten erhalten. Diese Quellbeobachtung ersetzt keine Messung tatsächlich gefahrener Routen. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp#L1678-L1740)

**Beobachtet:** `Pathfinder::processPathfindQueue`, Zeilen 5881–5946, bearbeitet angeforderte Pfade über eine Warteschlange, begrenzt die kumulierten untersuchten Zellen pro Frame und kann zunächst Zonen aktualisieren. Es wird hier kein konkretes Millisekundenbudget oder plattformunabhängiger Durchsatz behauptet. [Beleg](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp#L5881-L5946)

**Beobachtet:** `changeBridgeState`, Zeilen 9686–9692, ändert den Zerstörungszustand einer Brückenebene und markiert die Zonenverwaltung als aktualisierungsbedürftig. `moveAllies`, ab Zeile 10112, besitzt Logik zum Freimachen von Wegen durch Verbündete, insbesondere auch für Sammler und Bauarbeiter. [Brückenzustand](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp#L9686-L9692), [Verbündete](https://github.com/electronicarts/CnC_Generals_Zero_Hour/blob/0a05454d8574207440a5fb15241b98ad0b435590/GeneralsMD/Code/GameEngine/Source/GameLogic/AI/AIPathfind.cpp#L10112-L10160).

## Konkreter HeroHour-Backlog: eigene Umsetzung

Die folgenden fünf Aufgaben sind Schlussfolgerungen für HeroHour, keine übernommenen EA-Implementierungen und keine bereits implementierten Funktionen.

1. **Sammlerzustände und Andockplätze.** Zustände für Anfahrt, Laden, Rückfahrt, Entladen, Warten und unterbrochenen Spielerbefehl; Plätze am Depot reservieren und bei Tod/Zerstörung freigeben. Abnahme: vier Sammler durchlaufen zehn Lieferzyklen ohne Doppelzahlung, Überlappungsstau oder verlorene Reservierung; Depotzerstörung mit voller Ladung bewahrt die Fracht und führt zu einem klaren Wartezustand.
2. **Erreichbare Ressourcen und manuelle Priorität.** Gültige erreichbare Ziele vor Entfernung bewerten, eine ausdrückliche Lager-/Depotwahl speichern und bei Erschöpfung oder Zerstörung kontrolliert wechseln. Abnahme: ein nahes Lager ohne Brückenverbindung wird verworfen; ein weiter entferntes erreichbares Lager bleibt nutzbar; eine manuelle Auswahl überschreibt die Automatik bis zur Ungültigkeit.
3. **Identifizierbare Produktionsaufträge und Abbruch.** Ursprünglich gezahlten Preis am Auftrag speichern, Abbruch mit eindeutiger ID und genau einmaliger Erstattung; fertig/blockiert als eigenen sichtbaren Status zeigen. Bestehendes HeroHour-Verhalten bei blockierten Ausgängen erhalten. Abnahme: wiederholter Abbruch erstattet nicht doppelt, ein blockierter Ausgang spawnt nicht in Hindernisse, und Freiräumen setzt den fertigen Auftrag ohne zweite Zahlung frei.
4. **Pfadwarteschlange mit Topologieversion.** Gemeinsame Anfragen, begrenzte Sucharbeit, Zwischenspeicherung und Invalidierung bei neuen/zerstörten Hindernissen; fehlgeschlagene Wege nicht unkontrolliert pro Einheit wiederholen. Abnahme: 40 gleichzeitige Querungsbefehle erzeugen messbare, begrenzte Sucharbeit; eine gesperrte Brücke invalidiert betroffene Wege und führt zu einer sicheren Alternativroute oder einem sichtbaren Fehler.
5. **Einheitenabstand und Vorrang für Logistik.** Dynamische Belegung oder lokale Ausweichlogik ergänzen; stehende verbündete Fahrzeuge müssen einen Sammler-/Produktionsausgang freigeben können. Abnahme: eine Kolonne und zwei entgegenkommende Sammler passieren eine Brücke ohne Wassereintritt; belegte Fabrikausgänge werden frei, ohne Fahrzeuge zu teleportieren oder Sammler dauerhaft anzuhalten.

## Noch ausstehender Abgleich mit dem Spiel

Für eine spätere Binär- und Laufzeitanalyse sind zunächst die tatsächlichen EXE-/DLL-Dateien, Versionsinformationen und Hashes sowie zugehörige Spieldaten zu erfassen. Erst danach können Funktionen/Strings mit dem veröffentlichten Stand abgeglichen und beobachtbare Tests zu Ladezeit, Andockreihenfolge, Produktionsabbruch und Brückensperrung durchgeführt werden. Bis dahin bleiben alle Aussagen oben **Quellcodebefunde** oder ausdrücklich gekennzeichnete **HeroHour-Vorschläge**.

## Erste eigene Umsetzung in HeroHour

Im anschließenden Entwicklungsauftrag wurde am 14. Oktober 2026 ein begrenzter Teil des ersten Backlogpunkts eigenständig umgesetzt: Ein erreichbarer manueller Bewegungsbefehl übernimmt bei einem Sammler die Kontrolle von der automatischen Logistik. Während der Bewegung bleiben Ladung und Lieferziel erhalten; nach Erreichen der Zielposition gibt der Sammler die manuelle Kontrolle frei und setzt Laden beziehungsweise Abliefern fort. Wasserziele, blockierte Zielpositionen und nicht erreichbare Wege werden vor Annahme geprüft. Ein später fehlgeschlagener Neuplanungsversuch gibt die manuelle Kontrolle ebenfalls wieder frei.

Die Umsetzung verwendet die vorhandene HeroHour-Navigation und neue eigene Kontrolllogik in `PrototypeGameMode.h/.cpp` sowie `PrototypeEconomy.cpp`. Sie kopiert keine EA-Funktion. Ein vollständiger Sammlerzustandsautomat, reservierte Andockplätze, Stauauflösung oder eine Übernahme von Zero-Hour-Werten sind damit ausdrücklich noch nicht implementiert.

Der Editor-Build war erfolgreich. `Saved/Logs/CollectorManualEconomyVerified.log` enthält bestandene Prüfungen für einen beladenen Sammler, mehrere unüberschriebene Bewegungsticks, tatsächliche Zielankunft, anschließende automatische Lieferung sowie Ablehnung von Wasser- und Gebäudezielen. Navigation und Kampf bestanden ebenfalls; `CollectorManualMatchVerified.log` meldet drei regulär abgeschlossene Partien mit `MATCH_TEST_RESULT=PASS wins=3/3`. Alle vier Testprozesse endeten mit Exitcode 0. Die Prüfungen liefen im Editor-Kommandozeilenmodus ohne Renderer und belegen deshalb noch keinen visuellen Mausklick-Test oder einen aktualisierten Windows-Paketstand.
