# HERO: erster gestalteter Spielabschnitt

Stand: 11. Oktober 2026. Visuelle Grundlage sind Art/Concepts/Factions/HERO.png, Land_Unit_Concept.png und UI_Inferface_Concept.png. Die vorhandenen Bilder bleiben Konzeptreferenzen; sie werden nicht als fertige 3D-Modelle ausgegeben.

Neu erstellte Originalmodelle unter Art/Models/Generated (jeweils .blend und .fbx):
- SM_HERO_Technical: improvisierter gepanzerter Pickup mit vier profilierten Rädern, Kabine, Windschutzscheibe, Ladefläche, Schutzbügel, Maschinengewehr, Munitionskasten und Auspuff.
- SM_HERO_BlackMarket: Betonhub mit Tor, Vordach, Büroaufbau, Lüftung, Antenne, Frachtkisten und Neonstreifen.
- SM_UrbanBlock, SM_CargoContainer, SM_OilDrums, SM_StreetLamp, SM_Rubble: Stadt- und Industrieausstattung.

Reproduzierbare Modellierung: Scripts/art/build_hero_models.py mit lokalem Blender 5.2. Import, Materialzuordnung und Stadtaufbau: Scripts/art/import_hero_scene.py. Nachbearbeitung: fix_exposure.py und polish_lights.py. Unreal-Assets unter Content/HERO, Karte Content/Maps/Prototype.umap.

Das Kartenlayout ist ein befahrbarer Straßenkorridor. Stadtbauten außen sind derzeit Kulisse. Fahrzeuge bleiben im freien Korridor und umgehen die Hauptquartiere; ein allgemeines NavMesh/Pathfinding für frei bebaubare Karten ist noch nicht implementiert.

Oberfläche: deutsche Kommandopanels mit Ressourcen, taktischer Übersicht, Schrottvorräten, Auswahl, Baufortschritt und Produktionswarteschlange. Keine passiven Einnahmen: Sammler liefern Schrott am Depot ab. Depot und Werkstatt sind platzierbar.

Zusätzliche Originalmodelle: SM_HERO_Salvager (Utility-Truck mit Schrottladung), SM_HERO_Workshop (Garage mit Servicebuchten, Generator und Kran), SM_HERO_Depot (Containerdepot mit Vordach). Spiel verwendet diese rollenabhängig.

Aktuelles Gameplay: Einzel-/Rechteck-/Gruppenauswahl, Kamerabewegung, Rückzug, Feinderfassung, Zielangriff, HP/Zerstörung, Sammlerwirtschaft, bezahlte Produktion, wirtschaftlich begrenzter Gegner, Sieg/Niederlage und Neustart. Gebäude werden während ihrer Bauzeit sichtbar aufgebaut. Lokale Hindernisumfahrung umfasst alle Gebäude.

Alle Modelle sind von uns parametrisch gebaut, keine Fremdassets. Das Ergebnis ist ein erster eigenständiger 3D-Stilversuch nach den Zeichnungen; es erreicht nicht deren finale Detail- und Texturqualität. Blender-Dateien erlauben weitere Modellierung statt Wegwerfplatzhalter.
