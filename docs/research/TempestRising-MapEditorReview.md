# Tempest Rising: lokale Map-Editor-Dokumentation

Stand: 14.10.2026. Read-only Untersuchung der mitgelieferten Dokumentation. Kein Spielstart, keine Änderung der Installation, keine Übernahme von Spielcode oder Assets. Die folgenden Befunde stammen aus Dokumentation, nicht aus einer Beobachtung des Shipping-Runtimes.

## Quellen und Beleggrenzen

Quellordner: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Docs`.

| Datei | PDF-Seiten | SHA-256 |
| --- | ---: | --- |
| api.pdf | 190 | `591cef53057ae229fca5cf41f525bd26ee01fda79fcaf33a2333a78e76e79263` |
| manual.pdf | 33 | `8f8b8ce9c1725ea3fce3d4296de93ce94c347eba3e8f88f9e600c4e2b2a66417` |

Seitenangaben sind die einbasierten PDF-Seiten; auf den visuell geprüften Seiten entsprechen sie den gedruckten Seitenzahlen. Vollständige seitenweise Textauszüge liegen ausschließlich im ignorierten `Saved/Research/TempestRising/{api,manual}-pages.json` und den zugehörigen `*-text.txt`. Die Seiten manual 12/17 und api 181 wurden zusätzlich gerendert und visuell geprüft. `beginner.pdf` und `setdressing.pdf` waren für diese gezielte Mechanikprüfung nicht erforderlich und wurden nicht ausgewertet.

Die PDFs beschreiben Editor-/Mod-APIs. Sie beweisen weder tatsächliche Balancewerte dieser Installation noch interne Algorithmen, Tickfrequenzen, Multiplayer-Determinismus oder das Verhalten einer konkreten Shipping-Version. Generierte API-Texte enthalten erkennbare Unschärfen: Beispielsweise beschreibt `Num Claimed Tiles` auf api S. 179 entgegen dem Namen eine Zahl unbeanspruchter Kacheln. Diese Funktion darf ohne weiteren Nachweis nicht als zuverlässige Spezifikation übernommen werden.

## Ressourcen und Sammler

Das Handbuch beschreibt Ressourcenfelder als Volumen mit einem eigenen Ressourcenwert je überdeckter Nav-Grid-Zelle; die Visualisierung zeigt hexagonale Zellen (manual S. 17). Die API dokumentiert für `TempestFieldVolume` Gesamtmenge, Menge an Weltposition, Erschöpfungszustand sowie getrennte Abfragen für lokal sichtbare Restmenge und Anteil (api S. 177-181). Der Grid-Befund ist nicht gleichbedeutend mit einem hexagonalen allgemeinen Pathfinding-Algorithmus.

`Get Closest Unclaimed Resource Tile` sucht eine nicht beanspruchte Kachel nahe einer Position. Optional werden nur auf dem Navmesh für einen angegebenen Pawn erreichbare Kacheln berücksichtigt; das Ergebnis kann `INDEX_NONE (-1)` sein (api S. 181-182). Das dokumentiert eine Erreichbarkeitsoption und das Konzept beanspruchter Ressourcenstellen. Es zeigt nicht, wie Reservierung, Freigabe, Konkurrenzauflösung oder Distanzberechnung intern implementiert sind.

Die dokumentierten Befehlstags unterscheiden Bewegung zur Raffinerie, Ernten und automatisches Ernten; Status-Tags unterscheiden Idle und Autoharvesting (api S. 56-57, 79-80, 82). Das belegt die öffentliche Begriffstrennung, nicht eine vollständig rekonstruierte Sammler-Zustandsmaschine. Wachstumsstadien und Feldkonfiguration existieren (api S. 178); daraus lassen sich keine Regenerationsraten ableiten.

**Eigene HeroHour-Anwendung:** Sammlerzielwahl sollte Erreichbarkeit vor Annahme prüfen und Ressourcenstellen zeitlich reservieren. Reservierungen müssen bei Zielwechsel, Tod, Erschöpfung und fehlgeschlagenem Weg freigegeben werden. Die eigene finite Ressourcenwirtschaft kann beibehalten werden. Für späteres Fog of War sind tatsächliche Ressourcenmenge und dem Spieler bekannte Menge getrennte Daten. Hex-Zellen sind hierfür nicht erforderlich.

## Befehle, Produktion und Bauplatzierung

Die Mod-API trennt grundsätzlich unterstützte Befehle, aktuelle Ausführbarkeit und tatsächliche Ausführung. Einzelbefehle und Gruppenbefehle können Actor- oder Positionsziele haben; Gruppenrückgaben besagen nur, dass mindestens ein Pawn den Befehl ausführen oder einreihen konnte (api S. 104-108, 116-118). Bei gemischten Gruppen folgt daraus keine Garantie für alle Einheiten.

Der Ausführungsmodus ist explizit: sofort versuchen und andernfalls scheitern, ausschließlich einreihen oder sofort versuchen und bei Bedarf einreihen. Diese Modi sind auf api S. 184 dokumentiert. Warteschlange leeren und aktuellen Befehl abbrechen sind getrennte Operationen (api S. 116, 118). Ein Sammelpunkt-Befehlstag ist vorhanden (api S. 78). Das ist keine Beschreibung einer Produktionsrückerstattung.

Produktionszeit- und Kostenmultiplikatoren sind öffentliche Attribute einer Fabrik (api S. 6; manual S. 24). `TedStructureBase::Can Spawn Unit` dokumentiert eine aktuelle Spawn-Möglichkeitsabfrage (api S. 156). Die Dokumente liefern jedoch keine vollständige Produktionsqueue, Zahlungszeitpunkte, Stornierungsregeln oder blockierte Ausgangsbehandlung. Diese Fragen bleiben offen.

Für Bauplatzierung beschreibt das Handbuch ein eigenes quadratisches Navigationsraster und NoStructures-Flächen (manual S. 12-13). Die API enthält BuildRadius, Baustellenfortschritt und Strukturausrichtung; daraus ergibt sich keine vollständige Platzierungsprüfung (api S. 5-6, 151-159). Die Eigenschaft `No Placement Collision` auf api S. 170 betrifft dagegen visuelle Elemente eines Ressourcenfeldes, nicht die allgemeine Gebäudeplatzierung.

**Eigene HeroHour-Anwendung:** Ein zentraler Command-Validator sollte Vorschau/HUD und tatsächliche Ausführung mit demselben Regelwerk versorgen. Ein `Supported`-Zustand kann von `AllowedNow` mit konkretem Ablehnungsgrund getrennt werden. Queue-Abbruch und aktueller Befehlsabbruch brauchen unterschiedliche Bedienung. Bei Produktionsstorno müssen HeroHours eigene, explizit festgelegte Zahlungs- und Refundregeln gelten. Sammelpunkte sollten beim Ausbringen einer Einheit einen validierten Befehl erzeugen. Ein Spawn-Check sollte Fläche und anschließende Wegverbindung berücksichtigen.

## Navigation, Brücken und Kartenvalidierung

Das Handbuch beschreibt fünf Navmeshes: Infantry, Medium, Large, Square und Air. Medium/Large dienen Bodenfahrzeugen, Square der Bauplatzierung. Die Flächen werden innerhalb eines NavMeshBoundsVolume auf geeignetem Terrain generiert (manual S. 12). APIs für gültige Bodenpositionen können Navmesh/Queryfilter und Ausdehnung berücksichtigen; Einheitenprüfungen verwenden bei fehlender expliziter Ausdehnung den Einheitenradius (api S. 103-104, 111, 113). `TedNavMesh` ist als RecastNavMesh-Unterklasse mit Glättungsparametern dokumentiert (api S. 119-120).

Die dokumentierte Navigation ist vertikal projiziert. Ein Szenario, in dem Einheiten dieselbe Brücke sowohl überqueren als auch darunter hindurchgehen, wird ausdrücklich als nicht unterstützt beschrieben (manual S. 12). Getrennte Luftnavigation folgt standardmäßig dem Gelände und kann über zusätzliche unsichtbare Navigationsflächen geglättet werden (manual S. 14-15). Das rechtfertigt keine Annahme frei beweglicher dreidimensionaler Flugpfade.

NavModifier-Flächen unterscheiden komplette Sperre, zu meidende Bereiche, Bauverbote sowie Boden- und Luftverbote. TedNavPylons markieren verbundene erreichbare Bereiche; isolierte Inseln werden funktional deaktiviert (manual S. 13-14). Ein erfolgreicher Pawn-Spawn garantiert keine Navigation: Das Handbuch warnt vor ohne gültige Navmesh feststehenden Einheiten (manual S. 15).

Reparierbare Brücken bestehen dokumentiert aus visueller/spielmechanischer Brücke und einer zusätzlich angelegten begehbaren Fläche. Zerstörung erzeugt einen Navigationsblocker, Reparatur entfernt ihn; Reparaturaktivierungspunkte liegen an beiden Enden (manual S. 16-17). Es handelt sich nicht um automatisch aus dem Brückenmesh abgeleitete Navigation.

Karten-Build und Packaging validieren häufige Fehler (manual S. 28). Weltgrenzen betreffen Kamera und Fog of War; das Handbuch fordert TedNavigationSystem für leere Kartenvorlagen (manual S. 8). Startarmeen, Spielerbeziehungen und ein Sandbox-Modus sind Kartenmetadaten; Sandbox braucht eigene Sieg-/Niederlageauslösung (manual S. 10).

**Eigene HeroHour-Anwendung:** Zunächst vorhandenes A*-Grid behalten, aber Walkability, Buildability und gegebenenfalls Bewegungsgrößen getrennt prüfen. Navigationstestfälle sollten isolierte Ressourcenstellen, unpassierbare Fabrikausgänge und jede Brückenroute abdecken. Reparierbare Brücken wären später ein eigener dynamischer Blockerzustand. Die CityHero-Karte benötigt für diesen Prototyp keine zweite Verkehrsebene unter Brücken. Ein Editor-/Build-Validator kann Startpunkte, Ressourcenanschlüsse, Ausgänge und erreichbare Gegenbasis prüfen, bevor eine Karte ausgeliefert wird.

## Spielerwirtschaft, Autorität und KI

`TedPlayerData` bündelt dokumentiert Ressourcen und Eigentum sowie Ereignisse für veränderte Einheiten, Gebäude und Upgrades (api S. 133-134). Credits, Credit-Cap und Anteil werden separat abgefragt. Credit-Mutation ist als server-/Singleplayerseitig dokumentiert, mit expliziter Overflow-Option (api S. 145-147). Stromstatus und Stromverbrauch sind ebenfalls öffentlich abgefragt bzw. autoritativ geändert (api S. 134-137, 146). Diese Dokumentation ist kein Laufzeitnachweis der Replikation.

Spielerzustand exponiert Bot-Strategie und Schwierigkeit (api S. 150); Bot-/Mensch-/unkontrollierbare Spieler werden unterschieden (api S. 187). Die untersuchten Seiten erklären keine KI-Entscheidungskosten, Buildorders, Angriffsplanung oder Schwierigkeitsboni.

**Eigene HeroHour-Anwendung:** HUD auf wirtschaftliche Ereignisse reagieren lassen, Änderungen im GameMode/Economy ausführen und Bot-Strategie getrennt von Bot-Schwierigkeit modellieren. Für einen späteren Multiplayer-Port können autoritative Transaktionen vorbereitet werden. Das macht den aktuellen Prototyp nicht bereits multiplayerfähig. KI-Buildorder oder Balance müssen anhand eigener Tests und zusätzlicher Evidenz entworfen werden.

## Empfohlene Reihenfolge nach der Untersuchung

1. Sammler-Zielreservierung und Freigabe mit Tests für mehrere konkurrierende Sammler.
2. Gemeinsame Befehlsvalidierung mit sichtbaren Gründen für verweigerte Aktionen.
3. Sammelpunkte und getrennte Queue-/Befehlsabbruchbedienung.
4. Kartenvalidator für Startpunkte, Ressourcenwege, Fabrikausgänge und Brücken.
5. Erst anschließend Fog of War, Stromsystem und reparierbare Brücken nach eigenem Prototypbedarf.

Diese Liste ist ein Vorschlag, keine in diesem Untersuchungsschritt implementierte Änderung. Exakte Tempest-Rising-Balance, Sammler-Docking, Produktionsrefund und KI-Strategie bleiben ungeprüft.
