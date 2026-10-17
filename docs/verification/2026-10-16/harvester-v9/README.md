# Sammler V9: Editor-Abnahme

Am 16. Oktober 2026 wurden acht tatsächliche, getrennte Unreal-Editor-Prozesse mit `-game -nullrhi` ausgeführt. Alle acht Prozesse beendeten sich erfolgreich und lieferten den jeweiligen PASS-Marker:

| Suite | Aegis | Ember |
|---|---:|---:|
| Presentation | 165 Prüfungen PASS | 165 Prüfungen PASS |
| Resource | 18 Prüfungen PASS | 18 Prüfungen PASS |
| Docking | 28 Prüfungen PASS | 28 Prüfungen PASS |
| Economy | PASS | PASS |

Economy besitzt keinen expliziten COUNT-Marker; deshalb wird hier keine erfundene Gesamtzahl angegeben. Presentation enthält weiterhin alle bisherigen 141 Prüfungen sowie 24 neue Sammlerprüfungen. Jede Presentation-Ausführung prüft beide Teamzuordnungen; Aegis und Ember als Startfraktion testen zusätzlich deren vertauschte Fraktionsauflösung.

Die neuen Prüfungen verwenden tatsächlich geladene und registrierte Teilmeshes. Sie prüfen Geometriegrenzen, Fraktionsmaterialien und Tintwerte, nicht interaktive Zusatzkomponenten, echte Transformänderungen von Arbeitsarm/Trommel/Kupplung, leere und volle Füllstandsanzeigen, unveränderte Simulationswerte sowie Rollenwechsel und Actor-Zerstörung. Die Darstellung wird dazu über `TickVisual` direkt angesteuert. Diese künstlich gesetzten Zustände werden nicht als natürlich durchlaufener Wirtschaftszyklus dargestellt. Die getrennten Resource-, Docking- und Economy-Suites prüfen weiterhin die Wirtschaft.

`Editor-Checks.json` enthält die ursprünglichen vollständigen Logpfade und deren SHA-256, die kompakten Logauszüge samt Hashes sowie den Hash der getesteten Editor-DLL und der relevanten Quellen. Die vollständigen Logs bleiben unter `Saved/Logs`. Die acht daneben gespeicherten Logs enthalten ausschließlich die jeweiligen Testzeilen.

**Keine Paketabnahme:** Diese Prüfung bestätigt weder ein neu gekochtes Windows-Paket noch die Bildqualität in der Spielkamera. Eine gerenderte Aufnahme und ein vollständiger natürlicher Feld→Raffinerie→Feld-Zyklus stehen separat an.
