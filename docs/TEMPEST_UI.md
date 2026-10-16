# Tempest-orientierte HeroHour-UI

Stand: 16. Oktober 2026. Die neue UI ist implementiert und im frischen Windows-Paket geprüft: **19 erforderliche Suiten, 911 Einzelprüfungen und 46 natürliche Match-/Logistikrunden PASS.** Die Oberfläche rekonstruiert die beobachtete Anordnung und Bedienhierarchie von Tempest Rising für HeroHours vorhandene Systeme. Originale Tempest-Texturen, Porträts oder andere Spielassets wurden dafür nicht übernommen; Pixelparität ist nicht erreicht oder behauptet.

## Layout und Bedienung

- Radar oben rechts mit Stromanzeige; bei fehlendem betriebsbereitem Radar bleibt die Karte gesperrt. Eine aktive Karte verschiebt per Klick die tatsächliche Kamera.
- Hohe rechte Sidebar mit drei Porträtspalten, vertikalen Kategorien und den Primärreitern **BAUEN / DOKTRINEN**. Gebäude, Abwehr, Infanterie und Fahrzeuge sind funktional; Luftwaffe bleibt sichtbar deaktiviert.
- Produktionskarten starten echte Platzierung oder bezahlte Produktion. Queue-Slots wählen den verantwortlichen Produzenten, ohne Aufträge oder Geld zu verändern. Stornieren entfernt dessen **letzten** offenen Auftrag und erstattet dessen tatsächlich bezahlten Preis.
- Mitgliedsporträts zeigen individuelle Lebenspunkte. Klick wählt dieses Mitglied allein; Umschalt-Klick entfernt es aus dem Verband. Gruppenfähigkeiten berücksichtigen weiterhin die gesamte Auswahl und passende Einheitentypen.
- Kontrollgruppen unten links: Strg und Zahl beziehungsweise Strg-Klick speichert; Zahl oder Klick ruft ab; Umschalt erweitert die Auswahl. Zweifaches schnelles Abrufen fokussiert die Kamera. Tote und fremde Mitglieder werden gefiltert.
- Doktrinen benötigen Punkte und eine gültige Zielbestätigung. Auswahl allein kostet nichts; der globale Cooldown verhindert sofortige Wiederholung. Einheitenfähigkeiten zeigen Bereitschaft beziehungsweise Cooldown.
- Das Pausefenster setzt die Simulation tatsächlich aus und sperrt Welt-/Produktionsklicks. Fortsetzen und Neustart sind bedienbar; nach Spielende bietet das Ergebnisfenster einen echten Neustart.

## Nachweise und Aufnahmen

Das finale Paket besteht für **beide Fraktionen bei 1600×900 und 1280×720 jeweils 73 UI-Prüfungen**. Sie prüfen unter anderem Rollenplatzierung, exakte Erstattungen, Radarabschaltung, Queue-Zuordnung, Kontrollgruppen, Mehrfachauswahl, Doktrin-Cooldown und Pause-/Ergebnisinteraktionen einschließlich unterbrochener Auswahlziehung. Die beiden zusätzlichen 1280-Läufe und 14 FieldVisual-Prüfungen werden getrennt von den 19 erforderlichen Suiten gezählt. Paket CreatedUtc: `2026-10-16T19:36:08.6233213Z`. [Paketnachweise](verification/2026-10-16/tempest-ui-package/Transformation-Checks.json), [UI-1280](verification/2026-10-16/tempest-ui-package/UI-1280-Checks.json), [Interaktionsbericht](research/UI-Interaction-Abnahme.md).

Bereits vorhandene **Editoraufnahmen**:

- [Aegis 1600×900](../Art/Previews/TempestUI-Editor-Aegis-1600.png)
- [Ember 1280×720](../Art/Previews/TempestUI-Editor-Ember-1280.png)

Sieben ausdrücklich diagnostische Zustände sind über `Capture-HeroHour.ps1 -View InterfaceFixture -UIState <Zustand>` verfügbar. Die zusätzlichen Einheiten, Startmittel und Zustandswechsel sind Prüfvorrichtungen; diese Bilder sind keine natürlichen Gefechtsabläufe. Logs tragen `UI_CAPTURE_FIXTURE`, `unnatural_fixture=1` und tatsächliche Strom-/Radar-/Queuewerte.

| Zustand | Tatsächlicher Prüfzustand | Vorhandene Editoraufnahme |
|---|---|---|
| Radar | Eigenes Radar, ausgewählte Infanterie, echte aktive Radarversorgung | [Radar](../Art/Previews/TempestUI-Editor-Radar-Aegis-1600.png) |
| LowPower | Zusätzliche Fabrik und echte Kraftwerksabschaltung; Radar offline | [LowPower](../Art/Previews/TempestUI-Editor-LowPower-Aegis-1280.png) |
| Multi | Infanterie plus drei Fahrzeuge, gespeicherte Kontrollgruppe 1 | [Multi](../Art/Previews/TempestUI-Editor-Multi-Ember-1280.png) |
| Production | Fabrik, ausdrücklich gesetzte 1600 Startkredits, drei tatsächlich bezahlte Fahrzeugaufträge | [Production](../Art/Previews/TempestUI-Editor-Production-Aegis-1600.png) |
| Tooltip | Radarzustand und diagnostisch überlagerte Hoverposition auf der ersten Baukarte | [Tooltip](../Art/Previews/TempestUI-Editor-Tooltip-Ember-1600.png) |
| Pause | Radarzustand mit echter Spielpause | [Pause](../Art/Previews/TempestUI-Editor-Pause-Aegis-1600.png) |
| Victory | Radarzustand mit tatsächlichem HQ-Verlust-Ergebnisaufruf für Team 1 | [Victory](../Art/Previews/TempestUI-Editor-Victory-Ember-1280.png) |

Alle sieben Zustände wurden auch im finalen Paket aufgenommen und persönlich visuell geprüft. [Paketbilder und Prüfsummen](verification/2026-10-16/tempest-ui-package/Captures.json), [Bildabnahme](research/TempestUI-VisualQA.md). Beispiel mit tatsächlich laufender Produktion:

![Produktionsoberfläche im Windows-Paket; diagnostische Prüfszene](../Art/Previews/TempestUI-Package-Production-Aegis-1600.png)

## Verbleibende Grenzen

Die Porträtübersicht zeigt höchstens neun ausgewählte Mitglieder; Befehle gelten trotzdem für alle ausgewählten Einheiten. Die globale Queueübersicht zeigt fünf Aufträge. Ein einzelner Produzent erlaubt maximal fünf offene Aufträge und zeigt nach Auswahl seine vollständige Queue. Seitenwechsel für größere Übersichten sind noch nicht implementiert. Queue-Klick bedeutet Produzentenauswahl, keine beliebige Einzelauftragsstornierung.

Lufteinheiten fehlen. HeroHours eigener begrenzter Roster und seine Porträts bleiben sichtbar; vollständige Tempest-Menüinhalte werden nicht vorgetäuscht. Feine Typografie, sämtliche Hover-/Cooldown-/Mehrfachauswahlzustände und das Zusammenspiel mit der weiterhin ausbaufähigen 3D-Welt erfordern zusätzliche visuelle Abnahme. Dieser UI-Stand bedeutet keine fertige Umsetzung sämtlicher Konzeptbilder oder des vollständigen Spiels.
