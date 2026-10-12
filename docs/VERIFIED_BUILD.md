# Nachgewiesener Windows-Build

Stand: 12. Oktober 2026. Unreal Engine5.8.3, Win64 Development, Karte `/Game/Maps/CityHero`. Paket erstellt um23:38Uhr (Europe/Berlin); UAT meldete BUILD SUCCESSFUL mit Exitcode0. Das Paket liegt lokal unter Builds/Windows und wird als erzeugtes Build-Ergebnis nicht in Git eingecheckt.

| Prüfung im ausgelieferten Paket | Ergebnis | Lokales Log unter Saved/Logs |
| --- | --- | --- |
| Paketierung | PASS | Packaging-20261012-233719-924.log |
| Kampf | 15 Prüfungen PASS | CombatTest-20261012-233808-738.log |
| Wirtschaft | 23 Prüfungen PASS | EconomyTest-20261012-233808-750.log |
| Navigation | 10 Prüfungen PASS | NavigationTest-20261012-233808-742.log |
| Drei vollständige simulierte Partien | 3/3 Siege, reguläre Produktion und Lieferungen, sichere Bewegung | MatchTest-20261012-233808-740.log |

Alle vier Testprozesse endeten mit Exitcode0. Keine fehlenden HERO-/LocalAssets-Pakete. Navigation prüft auch echte Einheitenbewegung über die Brücke. Die Partieprüfung nutzt denselben Gameplaycode mit beschleunigten Simulationsticks; sie ersetzt keine verschiedenen manuellen Spielstrategien oder Performance-Messung.

Reproduzieren: Build-HeroHour.ps1 -Package, anschließend Test-HeroHour.ps1 -Mode Packaged mit -Suite Combat, Economy, Navigation und Match. EngineRoot lässt sich als Parameter angeben. Visualisierter tatsächlicher Spielstand: Art/Previews/CityHero-Prototype.png. Die Grafik ist weiterhin Prototypqualität.
