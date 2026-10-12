# HeroHour: Entwicklungsplan zum spielbaren Prototypen

Stand: 11. Oktober 2026. Planungsgrundlage: lokale Quellen und GitHub-main, Commit e6bc3bcde3809c4b25b37d9cb3e884d59971fc81. Die nachfolgenden Architekturentscheidungen sind Vorschläge für den Prototypen, keine bereits implementierten Funktionen.

## Ziel und Definition von fertig

Ein eigenständig startbarer Windows-Prototyp: Der Spieler sammelt Ressourcen, baut eine Basis, produziert Einheiten und zerstört das gegnerische Hauptquartier. Eine Partie dauert als Designziel 10–15 Minuten. Eine kleine Karte, eine Fraktion mit zwei Teamfarben, ein einfacher Computergegner und Platzhaltergrafik genügen. Der Computergegner dient zum Testen des späteren PvP-Spielablaufs; eine Kampagne ist nicht vorgesehen.

Abnahme:
- Windows-Build startet ohne Unreal Editor und ohne Online-Anmeldung.
- Kamera, Einzel- und Rechteckauswahl sowie Bewegungs- und Angriffsbefehle funktionieren.
- Ressourcen werden gesammelt und beim Bauen/Produzieren korrekt abgezogen.
- Gebäude entstehen nur an gültigen Positionen; Einheiten entstehen nach Ablauf einer Produktionszeit.
- Einheiten umgehen die vorgesehenen Kartenhindernisse und können Gebäude angreifen.
- Gegner produziert Einheiten und greift an; beide Seiten können gewinnen.
- Sieg/Niederlage stoppt die Partie und erlaubt einen sauberen Neustart.
- Drei aufeinanderfolgende vollständige Partien ohne Absturz oder blockierten Spielablauf.
- Belastungstest mit insgesamt 100 Einheiten; Ziel 60 FPS bei 1080p auf einem vorab dokumentierten Referenz-PC. Messung und Qualitätsstufe gehören zum Ergebnis, nicht nur eine Behauptung.

## Bestandsaufnahme und erste Blocker

| Befund | Konsequenz | Erste Maßnahme |
|---|---|---|
| Lokaler Ordner hat keinen eigenen Git-Root; Git findet C:/Users/Hero und einen Remote eines anderen Projekts | Gefahr falscher Commits und unklarer Versionsstände | HeroHour in sauberem, separatem Checkout bearbeiten; bestehende lokale Dateien zunächst vergleichen und erhalten |
| .uproject referenziert HeroHourGame und HeroHourEditor, deren Module im geprüften Baum fehlen; Target.cs fehlen | Unreal-Build ist nicht als funktionsfähig nachgewiesen | Minimalen Runtime-Modulaufbau und Game-/Editor-Targets herstellen |
| F#-Projekt referenziert 24 fehlende Quellen; Testprojekt referenziert fehlende Tests | Kein vollständiger Simulationskern oder belastbarer Testnachweis | Für Prototyp vom Build entkoppeln und für spätere Prüfung erhalten |
| Keine .umap/.uasset im geprüften Repositorybaum | Keine nachgewiesene spielbare Szene | Kleine Testkarte mit Engine-Grundformen anlegen |
| Header deklarieren zahlreiche Gameplay-Funktionen ohne entsprechende Implementierungsdateien | Schnittstellen sind noch kein Gameplay | Nur tatsächlich benötigte Klassen implementieren |
| README fordert Multiplayer-only; Gameplay-Dokument fordert Kampagne | Unklare Produktpriorität | Dieser Prototyp-Scope wird Arbeitsgrundlage; widersprüchliche Dokumente anschließend kennzeichnen |
| CI enthält umfangreiche Tool-/Engine-Annahmen | Grünes README-Badge belegt keinen erfolgreichen Build | Tatsächliche Builds/Tests und Laufprotokolle als Nachweis verwenden |
| .uproject nennt UE 5.8; keine Engine im geprüften Standardinstallationsordner gefunden | Lokal verfügbarer, kompatibler Build noch offen | Engine-Installation und C++-Toolchain prüfen; konkrete verfügbare Version festschreiben |

Es wurde kein erfolgreicher Unreal-Build oder Spieltest durchgeführt. Ein fehlender Standardinstallationspfad beweist nicht, dass Unreal nirgendwo installiert ist. Die Word-Spezifikationen sind in dieser ersten Bestandsaufnahme noch nicht vollständig ausgewertet; vor inhaltlichen Änderungen relevante Abschnitte mit dem begrenzten Scope abgleichen.

## Umfang

### Verpflichtend

- Windows, Maus/Tastatur, RTS-Kamera mit Verschieben und Zoom.
- Eine kompakte, gut navigierbare Karte, zwei Startpositionen.
- Eine Fraktion; identische Regeln für Spieler und Gegner.
- Eine Ressource: Credits. Sammler pendelt zwischen Depot und Ressourcenknoten.
- Drei Gebäude: Hauptquartier, Ressourcendepot und Fahrzeugfabrik.
- Drei Einheitentypen: Sammler, leichter Kampfwagen, schwerer Panzer.
- Bauen durch Hauptquartier-Baumenü innerhalb eines Bauradius; kein eigener Bauarbeiter notwendig.
- Gebäudekosten, Bauzeit, Produktionswarteschlange und Einheitenlimit.
- Bewegung, automatische Zielerfassung, gezielter Angriff, Lebenspunkte, Zerstörung.
- Sichtbare Ressourcen, Auswahlstatus, Lebenspunkte, Bau-/Produktionsfortschritt und Fehlermeldungen.
- Einfacher Gegner: Wirtschaft aufbauen, Armee produzieren, in Gruppen angreifen.
- Sieg durch Zerstörung des Hauptquartiers; Neustart.

### Optional nach bestandenem Kernspiel

Kontrollgruppen, Sammelpunkt, Attack-Move, Minimap, einfacher Fog of War, Grundsound und ein erkennbarer HERO-Look. Optionales wird nicht zum Grund, einen fertigen Prototypen zurückzuhalten.

### Verschoben

Zehn Fraktionen, komplexe Asymmetrie, Helden, Superwaffen, Luft-/Marineeinheiten, Wetter mit Statuseffekten, zerstörbare Städte, Garnisonen, Map-Editor, Modding, Kampagne/AoD, Lern-KI, Matchanalyse, Replays, Ranked-Matchmaking, Accounts, Cloud-Saves, Steam/EOS-Integration, 1000+-Einheiten-Versprechen und AAA-Grafik.

## Technischer Weg

Unreal bleibt zunächst die Engine. Gameplay entsteht in einem kleinen C++-Modul; Blueprints/UMG dienen Szene und Oberfläche. Die endgültige Engine-Version wird nach Prüfung der vorhandenen Installation und Kompilierbarkeit festgelegt; nicht blind die Versionsangabe übernehmen.

Die F#/.NET/Fable-Brücke wird für die erste Version aus dem kritischen Pfad entfernt. Zwei Laufzeiten plus Sprachbrücke, deterministische Simulation und Netcode gleichzeitig aufzubauen ist ein eigenes Projekt. Vorhandene Quellen bleiben erhalten. Nach dem Prototypen entscheiden wir anhand eines begrenzten Integrationsexperiments, ob der zusätzliche Aufwand einen messbaren Vorteil bringt.

Gameplay erhält klare Komponenten für Team, Lebenspunkte, Befehle, Bewegung, Angriff, Wirtschaft und Produktion. Balancingwerte liegen zentral in einfachen Datenstrukturen/Data Assets. Navigation zunächst mit Unreal-Navigation auf ebener Karte; einfache Abstände und versetzte Gruppenziele. Schaden zunächst über Reichweite und Abklingzeit, Projektile nur visuell. Kein Physikschaden und keine komplexe Formation.

Spielzustand und Spielerbefehle werden getrennt gehalten; Besitz und Ressourcen werden zentral geprüft. Das erleichtert eine spätere autoritative Multiplayer-Version, garantiert aber keine automatische Netzwerkfähigkeit. Multiplayer und Lockstep werden nicht parallel implementiert.

## Meilensteine und Abnahmen

Aufwände sind grobe aktive Entwicklungstage bei regelmäßiger Mitarbeit und funktionierender Toolchain, keine Lieferzusage. Nach M0 und M2 neu schätzen.

| Schritt | Inhalt | Abnahme | Aufwand |
|---|---|---|---|
| M0: Startfähigkeit | Sauberer Checkout, lokale Änderungen vergleichen, Engine/Compiler prüfen, Module/Targets bereinigen, unnötige Plugins entkoppeln, Root-Config, leere Karte, erster Windows-Build | Editor öffnet; Paket startet auf Windows; reproduzierbarer Buildbefehl dokumentiert | 1–3 Tage |
| M1: Steuern | Kamera, Cursor-Hit-Test, Teamzuordnung, Auswahl, Rechtsklickbewegung, Navigation | 20 Einheiten auswählbar und um Hindernisse steuerbar | 2–4 Tage |
| M2: Kampfspiel | HP, Reichweite, Cooldown, Feinderkennung, Angriffe auf Einheiten/HQ, Matchende, Neustart | Kleine Partie mit vorplatzierten Armeen vollständig gewinn-/verlierbar | 2–4 Tage |
| M3: Wirtschaft/Basis | Credits, Sammlerroute, Depot, Bauradius, Platzierungsprüfung, Kosten, Bauzeit, Fabrik, Produktion | Vom Startzustand aus Ressourcen sammeln, Fabrik bauen und beide Kampfeinheiten produzieren | 3–5 Tage |
| M4: Gegner/Spieloberfläche | Regelbasierter Bot, Produktionsstrategie, Angriffswellen, HUD, verständliche Rückmeldung | 10–15-Minuten-Partie von Start bis Ergebnis ohne Entwicklerkonsole | 3–5 Tage |
| M5: Stabilisierung/Lieferung | Kosten-/Kampfbalancing, Navigation, Fehlerbehebung, Profiling, Windows-Paket, Anleitung | Gesamte Definition von fertig erfüllt; bekannte Restprobleme dokumentiert | 2–4 Tage |

Gesamtbudget als erste Planung: 13–25 aktive Entwicklungstage plus Puffer für Toolchain, Assetarbeit und unerwartete Integrationsprobleme. Ein zuverlässiger Kalendertermin ist erst nach M0 sinnvoll.

M2 ist die erste spielbare Rückfallversion. M3–M5 machen daraus den gewünschten RTS-Prototypen mit Wirtschaft und Basisbau.

## Streich- und Wechselregeln

1. Jedes neue Feature muss einen verpflichtenden Spielablauf verbessern oder einen reproduzierbaren Fehler beseitigen. Andere Wünsche kommen ins spätere Backlog.
2. Ein Meilenstein darf höchstens zwei aktive Entwicklungstage über seine obere Schätzung hinauslaufen, bevor Umfang oder Ansatz überprüft wird. Kein unbegrenztes Weiterbauen an Infrastruktur.
3. Beim Zeitdruck zuerst Minimap, Fog of War, Kontrollgruppen, Spezialeffekte und zusätzliche Assets verschieben.
4. Bei Navigationsproblemen Karte vereinfachen, Engstellen verbreitern, Einheitengröße vereinheitlichen und Einheitenlimit reduzieren. Erst danach komplexere Navigation erwägen.
5. Bei Wirtschaftsproblemen als vereinfachten Prototypen periodisches Depot-Einkommen verwenden; die Änderung ausdrücklich im Scope dokumentieren. Es bleibt die Schleife Einkommen → Bauen → Produzieren → Kämpfen erhalten.
6. Bei Botproblemen festen regelkonformen Bauplan und Angriffswellen verwenden; kein strategisches KI-Framework hinzufügen. Ein scripted Bot genügt.
7. Wenn M0 nach drei aktiven Arbeitstagen keinen lauffähigen Minimalbuild liefert: Fehlerursache festhalten und einen frischen minimalen Unreal-Projektaufbau mit kompatibler installierter Version versuchen. Vor einem Engine-Wechsel Aufwand neu beurteilen; ein leichter 2D-Prototyp ist eine mögliche Rückfallentscheidung, kein paralleles Zweitprojekt.
8. Fehlen Engine/Compiler oder Zugriff auf notwendige Entwicklungswerkzeuge, wird dies als externe Voraussetzung protokolliert; Wartezeit wird nicht als Implementierungsfortschritt gezählt.

## Qualitäts- und Arbeitsablauf

Kleine Änderungen auf codex/-Branches im richtigen HeroHour-Checkout. Vorhandene lokale Arbeit zuerst sichern/abgleichen; keine Änderungen am übergeordneten Benutzerprofil-Repository. Jeder Meilenstein endet mit einer vorführbaren Version, kurzen Startanweisungen und einer Liste bekannter Fehler.

Fehler nach Bedeutung behandeln: P0 Absturz/Buildausfall/Datenverlust; P1 verhindert Start, Befehle oder Partieende; P2 beeinträchtigt Spielablauf, hat aber Umgehung; P3 optischer Fehler. Vor zusätzlichen Features P0/P1 lösen.

Gezielte automatisierte Prüfungen für Kostenabbuchung, fehlende Ressourcen, Produktionsabschluss, Schaden/Tod, Sieg und vollständigen Match-Reset. Manuelle Prüfung für Auswahl, Kamera, Platzierung, Hindernisse und HUD. Ein Paket-Smoke-Test gehört schon zu M0 und wird vor jedem Meilensteinabschluss wiederholt.

CI zuerst ehrlich und klein: tatsächlich vorhandene Quellen prüfen; Unreal-Build nur auf Runner mit nachgewiesener Engine/Toolchain. Fehlende Tests oder übersprungene Builds nicht als erfolgreiche Validierung darstellen. Keine komplexe Release-Pipeline vor funktionierendem Paket.

## Erstes Umsetzungspaket

- [ ] HeroHour-Checkout eindeutig von C:/Users/Hero trennen und lokale/Remote-Unterschiede prüfen.
- [ ] Relevante Word-Spezifikationen gegen den Prototyp-Scope abgleichen.
- [ ] Engine-Installation, Version, Visual-Studio-C++-Toolchain und Windows SDK prüfen.
- [ ] Fehlende Game-/Editor-Targets ergänzen und .uproject auf vorhandene Module begrenzen.
- [ ] EOS/Steam und F# für den Offline-Minimalbuild entkoppeln.
- [ ] Minimale GameMode-/Controller-/Kamera-Implementierung und Testkarte herstellen.
- [ ] Editorbuild und Windows-Paket tatsächlich starten; Buildschritte und Fehler dokumentieren.

## Nach dem Prototypen

Zuerst durch mehrere Testpartien klären, ob Steuerung, Wirtschaft und Kampf Spaß machen. Danach kleine PvP-Version mit zwei Clients und autoritativem Host/Server; Befehlsvalidierung, Replikation und Verbindungsabbrüche separat testen. Erst anschließend zweite Fraktion, Fog of War für Wettbewerb und Lobby/Online-Dienste. Eine deterministische F#-Architektur wird nur nach erfolgreichem Versuch mit klaren Mess- und Korrektheitskriterien wieder aufgenommen. Weitere Inhalte werden einzeln aus dem Backlog ausgewählt.
