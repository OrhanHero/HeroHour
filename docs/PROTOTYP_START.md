# HeroHour: aktueller CityHero-Prototyp

Stand: 15.10.2026. Unreal Engine 5.8.3, aktives Modul HeroHourPrototype, Karte /Game/Maps/CityHero. Start und vollständige Steuerung: [STARTEN.md](STARTEN.md). Übergabe: [CURRENT_HANDOVER.md](CURRENT_HANDOVER.md). Git LFS ist für Karten, Unreal-Assets und Modelle erforderlich; das ignorierte Paket lokal neu bauen.

HeroHour-Starten.cmd startet Aegis, HeroHour-Ember-Starten.cmd Ember. Beide Fraktionen besitzen elf Rollen: HQ, Raffinerie, Fabrik, Kraftwerk, Kaserne, Radar, Abwehr, Sammler, Striker, Infanterie und Artillerie. Aegis besitzt robustere Panzerung und Rauch, Ember höheres Tempo und Tempest-Waffen. Getrennte gespeicherte Fraktionsdefinitionen steuern die Regeln; viele eigene Meshes werden noch geteilt.

Startbudget: 800 Credits. Sammler ernten räumliche Tempest-Zellen und entladen zeitlich an der Raffinerie. Drei Felder mit insgesamt 48 Zellen wachsen standardmäßig nach. Credits entstehen erst durch Cargo-Transfer. Strom beeinflusst Produktion, Bau, Radar und Abwehr; Kraftwerksbau stellt ein ausgefallenes Netz unabhängig wieder her. Manuelles Abschalten pausiert die Funktionen des Gebäudes.

Fünf Sidebar-Kategorien erschließen Gebäude, Abwehr, Infanterie, Fahrzeuge und Spezialaktionen. Baustellen werden ohne Bauarbeiter auf einem 200-cm-Raster innerhalb verbundener Bauradien platziert und mit Q gedreht. HQ, Kaserne und Fabrik führen echte bezahlte Produzentenwarteschlangen. Ausgänge müssen frei bleiben. Die Wegfindung berücksichtigt Wasser, Brücken, Stadt und rechteckige Gebäude samt Baustellen.

Einheiten klären den Weltnebel auf. Radar aktiviert bei ausreichender Versorgung die taktische Karte. Sicht regelt Darstellung und zulässige Ziele. Kampf enthält drei Schadensarten, Rüstungsmatrix, Veteranenstufen, Infanteriedeckung und Unterdrückung. G: Granate; T: Aegis-Rauch; Y: Artillerie-Deployment. Z/X: Doktrin mit anschließendem Zielklick. K: Gebäude schalten; Delete: verkaufen; F10: Pause; R: Neustart.

Sieg durch Zerstörung des gegnerischen HQ, Niederlage durch Verlust des eigenen. Der Gegner baut und finanziert sich nach denselben öffentlichen Regeln und greift das HQ frühestens nach 60 Sekunden an. Der aktuelle M1–M4-Kern umfasst beide Wirtschaften, Aufklärung, Basisbau, gemischte Armeen, Commander-Powers, Effekte, eigene Audios und grundlegende Infanterieanimationen.

19 aktuelle Paketläufe sind bestanden: 14 Aegis-Suiten und fünf zusätzliche Ember-Läufe. Dazu gehören drei Matchrunden und 20 Logistik-Stressrunden bei 60-Hz-Simulationsschritten je Fraktionsstart, insgesamt 46 natürliche Simulationspartien. Nachweise und Grenzen: [VERIFIED_BUILD.md](VERIFIED_BUILD.md). Zwei 20-Minuten-RHI-Dauerläufe sind zum Zeitpunkt dieses Dokuments noch in Arbeit und werden hier nicht als bestanden ausgewiesen.

Modelle, Oberflächen, Animationen und Balance bleiben Prototypqualität. Grafische Gleichwertigkeit mit Tempest Rising und Original-Verhaltensparität sind nicht belegt. Kampagne, Multiplayer, Luft-/Marine-Roster und große Techbäume gehören nicht zum aktuellen Umfang. Weitere Schritte: [NAECHSTE_ENTWICKLUNGSSCHRITTE.md](NAECHSTE_ENTWICKLUNGSSCHRITTE.md), Architektur: [TEMPEST_TRANSFORMATION_PLAN.md](TEMPEST_TRANSFORMATION_PLAN.md), Konzeptvergleich: [CITYHERO_ZIELKORREKTUR.md](CITYHERO_ZIELKORREKTUR.md).
