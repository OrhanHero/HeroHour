# HeroHour: aktueller CityHero-Prototyp

Stand: 12. Oktober 2026. Unreal Engine 5.8.3; aktives Modul HeroHourPrototype. Alte C++-/F#-Quellen bleiben erhalten.

Start, Build und Tests: [STARTEN.md](STARTEN.md). Aktuelle Karte: Content/Maps/CityHero.umap. Git LFS wird für Karten, Unreal-Assets, Modelle und neue Konzeptreferenzen benötigt.

WASD/Mausrad bewegt die Kamera. Linksklick oder Rechteck wählt eigene Fahrzeuge; Shift ergänzt, Leertaste wählt Kampfwagen. Rechtsklick bewegt oder greift an. B startet Werkstattplatzierung, V Depotplatzierung, Linksklick setzt das Gebäude; Escape/Rechtsklick bricht ab. P bestellt einen Panzer in einer fertigen Werkstatt, C einen Sammler im Hauptquartier. R startet neu. Die HUD-Schaltflächen bieten dieselben Bau- und Produktionsaktionen.

Startbudget 800 Credits, drei Panzer und ein Sammler. Sammler gewinnen endlichen Schrott und liefern ihn am Depot ab. Gebäude benötigen Geld und Bauzeit; Produktionswarteschlangen benötigen Geld, Zeit und einen freien Ausgang. Der Gegner nutzt dieselbe Wirtschaft und beginnt HQ-Angriffe nach 60 Sekunden Aufbauzeit. Sieg durch Zerstörung seines Hauptquartiers, Niederlage beim Verlust des eigenen.

A*-Wegfindung berücksichtigt den Fluss mit zwei befahrbaren Brücken, Stadtblöcke, Wohngebiet, Landmarken und dynamische Gebäude. Preise: Depot200, Werkstatt350, Panzer100, Sammler150. Depot benötigt8, Werkstatt12, Panzer5 und Sammler6 Sekunden. Bauradius2300; pro Produktionsgebäude höchstens5 Aufträge.

Die Windows-Pakettests für Kampf, Wirtschaft, Navigation und drei vollständige simulierte Partien sind bestanden. Nachweise: [VERIFIED_BUILD.md](VERIFIED_BUILD.md). Die drei identischen, deterministischen Partien prüfen den Ablauf und Neustart; sie belegen keine umfassende Balance oder unterschiedliche KI-Strategien.

Grenzen: eine Fraktion, zwei Fahrzeugrollen; vereinfachte Gebäude-/Geländeformen, keine finale Texturqualität, kein Multiplayer oder Weltnebel. Der Vergleich mit den Konzepten steht in [CITYHERO_ZIELKORREKTUR.md](CITYHERO_ZIELKORREKTUR.md). Stadion, Kraftwerk und Flugfeld sind Landmarken ohne Spezialproduktion.
