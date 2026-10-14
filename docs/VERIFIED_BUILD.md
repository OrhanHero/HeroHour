# Nachgewiesener Windows-Build

Stand: 14. Oktober 2026, Unreal 5.8.3, Win64 Development, `/Game/Maps/CityHero`. Paket vom 18:03 Berlin (`CreatedUtc2026-10-14T16:03:36.9731133Z`). UAT `BUILD SUCCESSFUL`, Exitcode0; ausführbare Dateien, Paketcontainer und Materialfehlerprüfung bestanden. Paket lokal unter `Builds/Windows`.

| Prüfung im tatsächlichen Paket | Ergebnis | Log unter Saved/Logs |
| --- | --- | --- |
| Paketierung | PASS | Packaging-20261014-180303-705.log |
| Kampf und Selektion | 17 PASS | CombatTest-20261014-180732-263.log |
| Wirtschaft, Produktion, Logistik, Feldvisuals | 70 PASS | EconomyTest-20261014-180732-272.log |
| Navigation einschließlich echter Brückenfahrt | 10 PASS | NavigationTest-20261014-180732-262.log |
| Vollständig simulierte Partien | 3/3 PASS, 3 Siege | MatchTest-20261014-180732-262.log |
| Datenbindung, Queue-/Power-Beispiele | 41 PASS | ArchitectureTest-20261014-180732-284.log |
| UI mit realer Grafik | 15 PASS | UITest-20261014-180559-868.log |
| Autoritativer Ressourcenfeldkern | 18 PASS | ResourceTest-20261014-180732-278.log |

Alle sieben Prozesse Exitcode0. Der Testwrapper verlangt Erfolgsmarker und lehnt fehlende HERO-/LocalAssets sowie fehlerhafte Materialien ab. UI läuft mit realem Grafik-Backend, die anderen Suiten mit NullRHI. UI nach ForceRes-Korrektur erneut15PASS: `UITest-20261014-180559-868.log`.

## Tatsächlich enthalten

Fünf eigene ConceptV2-Rollenmodelle einschließlich neuem Sammler mit Frontfräse, Tank und Entladekupplung. DistrictV3 mit eigenem Gelände, Stadtmodulen, Straßen, warmem PBR-Wüstenboden, Vegetation und drei ursprünglichen Energieressourcen-Patches. Größere Gebäudemodelle besitzen abgestimmte Navigations-Footprints. ConceptV3-HUD mit Metallrahmen, Glas, fünf Kategorien, echten Porträts, taktischer Karte, Auswahl/Fokus und fünf Auftrags-Slots mit Storno.

Acht gespeicherte `DataV3`-Assets sind im Paket vorhanden. Rollen lesen Kosten, Bau-/Produktionsdauer, Gesundheit, Bewegung, Waffenwerte, Mesh und UI-Porträt; Feldraten lesen die gespeicherte Definition. Fraktionsroster, rechteckige Grid-Footprints, Schadenstypen, Power-Grid und Fähigkeiten sind noch keine vollständige Live-Migration.

Der Feldkern läuft im echten Sammler-Adapter: exklusive Claims, Saatrest-Rückfahrt, verzögertes opt-in Nachwachsen, reale Lieferung und einmalige Auszahlung. Cargo/Wallet verwenden double. Neue Live-Gates prüfen fraktionale Bilanz (Toleranz1e-8), blockierte Direktzufahrt innerhalb des Lieferradius und nicht mehr erreichbare alte Dockpunkte. Depletion-Wechselevents und Primitive-Daten steuern die eigene Feldemission; erschöpfter regenerierender Boden bleibt sichtbar und für Bau/Nav reserviert, finite Erschöpfung gibt diese Flächen frei.

## Grafiknachweise und Grenzen

`Capture-HeroHour.ps1` rendert das tatsächliche Spiel mit `RenderOffscreen` und `ForceRes`, beendet den eigenen Hilfsprozess und prüft Screenshot-Zeitstempel sowie PNG-Auflösung. Paket-Ausgaben liegen unter `Builds/Windows/HeroHour/Saved/Screenshots`, Editor-Ausgaben unter `Saved/Screenshots`; Vorschauen werden nach `Art/Previews/ConceptV3-*.png` kopiert. Eine frühere Kopflose-Ausgabe888×500 wurde durch den neuen Auflösungsgate verworfen. Die vorher angesehenen District-Bilder dokumentieren die Farbkorrektur, keine1600×900-Abnahme.

Aktuelle Paketansichten: Battle2560×1440 (`Capture-Battle-20261014-180559-871.log`) und Harvester1600×900 (`Capture-Harvester-20261014-180559-870.log`), jeweils `CAPTURE_PASS`; tatsächlich geöffnet und visuell geprüft. District1600×900 stammt aus dem geometrisch gleichen Vorgängerpaket (`Capture-District-20261014-180131-299.log`); die letzte Paketänderung betraf UI-Text. Native Bedienung wurde nach ausdrücklicher Fortsetzungsanweisung wieder aufgenommen; der frühere Escape-Abbruch war eine Toolmeldung und wurde vom Nutzer bestritten. Native Beobachtungen des alten Pakets sind getrennte Belege; gerenderte Vorschauen sind keine menschliche Langzeitpartie.

M1 ist weiter offen: pro sichtbarem Feld derzeit eine autoritative Zelle, keine räumliche Mehrzellen-Ernte und keine reservierten Dockslots/zeitliche Entladung. Standardwirtschaft weiterhin finite; Nachwachsen mit `-PrototypeRegrowingFields`. Kein Live-Stromnetz, Fog of War, zwei asymmetrische Fraktionen, Veterancy oder aktive Fähigkeiten. Kein nachgewiesener Tempest-Nachbau, keine Grafikparität und keine20-Minuten-Performanceabnahme.

Reproduzieren: `Build-HeroHour.ps1 -Package`, anschließend `Test-HeroHour.ps1 -Mode Packaged -Suite` mit Combat, Economy, Navigation, Match, UI, Architecture und Resource. Start: `HeroHour-Starten.cmd`. Übergabe: [CURRENT_HANDOVER.md](CURRENT_HANDOVER.md).
