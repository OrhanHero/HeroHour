# Konzeptabnahme HeroHour

Arbeitsziel des Nutzers: Die gelieferten Konzeptbilder sollen im eigenen spielbaren Projekt erkennbar werden, verbunden mit belegten Tempest-REA-Erkenntnissen. Die folgenden Kriterien verhindern, dass reine Dateierzeugung oder grüne Systemtests als fertige Grafik ausgegeben werden.

Die neueste Nutzerpriorität lautet: **zuerst die Tempest-Rising-UI vollständig für HeroHour umsetzen**. Deren beobachtetes Layout ersetzt damit die frühere HUD-Anordnung aus dem Konzept. Der implementierte UI-Stand und die noch laufende neue Paketabnahme stehen in [TEMPEST_UI](TEMPEST_UI.md). Historische Paketprüfungen werden nicht auf den neuesten Arbeitsstand übertragen.

| Bereich | Maßstab aus den Referenzen | Aktueller Nachweis / offene Abnahme |
|---|---|---|
| Fraktionsidentität | Helle abgenutzte Panzerplatten mit Blau gegenüber dunkler technischer Architektur mit Grün; Gebäude und Fahrzeuge auf Spielentfernung unterscheiden | V7 hat eigene HQ-/8×8-Modelle beider Seiten. Aegis-HQ V9 samt passendem Porträt wurde im historischen V9-Paket mit 19 bestandenen Läufen abgenommen. Das ist keine Abnahme des neuesten Pakets oder des gesamten Rosters; weitere Rollen bleiben zu ähnlich. |
| Gebäudequalität | Erkennbare Funktionen, kontrollierte Silhouette, mechanische Details, glaubhafte gebrauchte Oberflächen; keine beliebigen Bauklötze | Das reparierte HQ V9 ist tatsächlich integriert und historisch paketgeprüft. Fabrik V10 ist inzwischen ein echtes integriertes 3D-Mesh, keine bloße 2D-Referenz; die neue Gesamtpaketabnahme läuft. Eine vollständige Basis-Artabnahme fehlt weiterhin. |
| Einheiten | Eigene lesbare Silhouetten, passende Teammarkierungen, sichtbare Bewegung/Arbeit/Feuer | Zwei 8×8-Modelle vorhanden. Sammler V9 hat echte bewegliche Teile und isolierte Zustands-/Lebensdauertests. Infanterie und weitere Fahrzeugklassen brauchen Qualitätsarbeit. |
| Weltkomposition | Wüste, unregelmäßige Felsen, Fluss mit Brücken, dichter Stadtkern, Wohn- und Industriegebiete, funktionale Landmarken | Karte besitzt Grundgliederung. Große leere Parzellen, breite saubere Straßen und sichtbare Wiederholung weichen stark ab. Terrain V9 verbessert nur Teilbereiche und ersetzt keine Weltüberarbeitung. |
| UI | Neueste Priorität: Tempest-Layout mit Radar oben rechts, hoher dreispaltiger Sidebar, vertikalen Kategorien, BAUEN-/DOKTRINEN-Reitern und Kontrollgruppen unten links | Implementiert: echte Karten, Queue-Produzentenauswahl, Mitgliedsporträts, Kontrollgruppen und Pause-/Ergebnisfenster. Frühere Editorprüfungen belegen einen Zwischenstand; der zusätzliche Modal-Drag-Test und die finale Paketabnahme stehen noch unter Prüfung. Typografie, Zustandsbilder und visuelle Nähe sind separat zu bewerten; keine Pixelparität behauptet. |
| Tempest-orientierte Systeme | Sammeln→Transport→Dock→Entladen, nachwachsende Felder, Stromabhängigkeit, Produktion, Fog, Fähigkeiten und schnelle Gefechte | V8-Systemtests bestehen. REA belegt nur dokumentierte Bytes/Metadaten, keine unbekannten Originalwerte oder Animationszeiten. Eigene Animation darf keine Wirtschaftszustände steuern. |
| Tatsächliches Ergebnis | Bilder/Videos aus dem aktuellen Spiel und dem ausgelieferten Paket, mit unverfälschter Kennzeichnung von Fixtures/ArtReview | Historische V9-Paketbilder und 19 Testläufe sind vorhanden. Die jüngste UI-/V10-Paketabnahme läuft separat; Editorbilder sind dafür kein Ersatz. Diagnostische UI-Zustände sind als Fixtures markiert. Neutrale Blender- oder Imagegen-Bilder sind keine Spielaufnahmen. |

## Fertig-Kriterium

Das Ziel ist erst erfüllt, wenn die wesentlichen Regionen, Fraktionsbasen, verwendeten Einheiten und UI in aktuellen Spielaufnahmen gemeinsam die Referenzrichtung tragen, die zentralen Spielabläufe vollständig funktionieren und relevante Paketprüfungen bestehen. Ein einzelnes gutes HQ oder eine lange Prüfliste reicht nicht. Abweichungen werden ausdrücklich dokumentiert; gescheiterte Assets werden nicht als Fortschritt integriert. Der aktuelle Gesamtstatus ist **nicht erfüllt**.

## Reihenfolge der laufenden Arbeit

1. Die implementierte Tempest-UI einschließlich aller wichtigen Zustände und der neuesten Modalinteraktion im aktuellen Paket abnehmen; Fehler beheben und visuelle Differenzen ausdrücklich bewerten.
2. V10-Fabrik und Ressourcenfelder zusammen mit dem bereits abgenommenen V9-HQ und der Sammlerpräsentation im neuen Gesamtpaket prüfen, einschließlich Wirtschaft-, Dock- und Stressabläufen.
3. Die größten verbleibenden Einheiten-/Gebäude-Platzhalter ersetzen, mit tatsächlichen Mehransichten, Laufzeitporträts und Rollenerkennbarkeit.
4. Weltparzellen, Straßenränder und Stadtvariation zusammenhängend überarbeiten. Dekoration darf Bau- und Fahrwege nicht sichtbar widersprechen.
5. Gemeinsame Konzeptvergleichsbilder und Leistungsprüfung des tatsächlichen neuen Paketstands erstellen.
