# Sammleranzeige statt unpassender Kampffähigkeiten

Am 16. Oktober 2026 wurde die tatsächliche Aufnahme `Art/Previews/HarvesterV9-Aegis-Natural-08.png` geprüft. Der ausgewählte Sammler zeigte unten die drei deaktivierten Kampffähigkeiten Granate, Rauch und Modus. Das vermittelte ein falsches Bedienangebot, obwohl der Sammler diese Aktionen nicht unterstützt.

`HeroHUD.cpp` zeigt bei einem einzeln ausgewählten Sammler nun in derselben Zone eine kurze Erklärung des automatischen Tempest-Kreislaufs. Bei einem manuellen Fahrbefehl wird stattdessen dessen Status erklärt. Der tatsächliche Ladestand und Wirtschaftsstatus bleiben in der bestehenden Detailzeile sichtbar.

Kampffähigkeitskarten werden nur noch gezeichnet, wenn mindestens eine gültige ausgewählte Kampfeinheit die jeweilige Fähigkeit unterstützt. Dieselbe Eignungsfunktion gilt für Klicks: Eine unpassende Zone löst weder `TryActivateAbility`, Fehlermeldung noch Abbruch einer Doktrin-Zielwahl aus. Die bestehende Gebäudeauswahl verwendet weiterhin ihre Strom-/Verkaufsschaltflächen. Kartenpositionen und unterstützte Infanterieaktionen bleiben unverändert.

Die Quelltextänderung wurde auf Whitespacefehler geprüft. Build, tatsächliche UI-Prüfungen und erneute gerenderte Sammleraufnahme stehen bei dieser Übergabe noch aus. Die oben genannte Aufnahme zeigt den Befund vor der Korrektur.
