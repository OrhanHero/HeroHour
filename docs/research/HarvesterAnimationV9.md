# Eigene Sammlermechanik V9: echte bewegliche Teilmeshes

Stand: 16. Oktober 2026. Die Aufteilung ist erzeugt und strukturell geprüft; Unreal-Import, Hookintegration und Laufzeit-Bildprüfung sind noch offen.

## Herkunft und REA-Grenze

`Tempest-REA-HarvesterV8.md` belegt getrennte Ernte-/Ablageparameter und offene Animationsfragen. Daraus folgt die Architekturentscheidung, sichtbare Arbeit strikt von autoritativem Abbau und Abladen zu trennen. Weder Armwinkel, Animationsgeschwindigkeit noch Glättungszeiten wurden aus Tempest übernommen. Alle Geometrien stammen unverändert aus unserem `build_concept_harvester.py`; keine externen Spielmodelle oder Texturen werden importiert.

## Erzeugte Aufteilung

`Scripts/art/build_harvester_animation_v9.py` führt ausschließlich die Deklarationen und Autorenfunktionen des ursprünglichen Skripts aus. Dessen Export-/Renderprogramm wird nicht ausgeführt. Originalquellen und ältere Exporte bleiben unverändert. Nach derselben Bodennormalisierung wird jedes Quellobjekt genau einer Gruppe zugewiesen:

| Teil | Inhalt | Dreiecke |
|---|---|---:|
| Base | Rumpf, sechs Räder, Tank, Kabine, feste Pivotlager, feste Entladeflansche | 30.852 |
| ArmAssembly | vorhandene Booms, Hydraulikteile, Antriebsgehäuse, Schutzblech | 1.280 |
| CutterDrum | originale Quertrommel, gezahnte Kragen, Schneidzähne | 3.568 |
| RearCoupler | vorhandene Dichtmanschette, Anschlussinneres und Indexring | 568 |
| Gauge0 bis Gauge6 | jeweils die originalen gegenüberliegenden Leuchtsegmente | je 88 |

Die Summe beträgt **36.884 Dreiecke**, exakt wie der vollständige bisherige Sammler. Die Vereinigung der unveränderten Ausgangspositionen besitzt dieselben Bounds. Jeder FBX-Export wurde wieder importiert und auf Dreieckszahl und lokale Bounds geprüft. UV-Koordinaten sind vorhanden und endlich. Diese neue UV-Aufteilung ist keine Behauptung einer unveränderten UV-Belegung; das bestehende prozedurale Materialsystem verwendet keine individuelle gebackene Sammlertextur.

`Art/Models/HarvesterAnimationV9/manifest.json` enthält Quell-SHAs, die Zuordnung aller Quellobjekte, Dreiecke, lokale Bounds, FBX-SHAs, Materialslots sowie Actor-/Parent-Pivots. `HarvesterAnimationV9.blend` speichert die vollständig wieder zusammengesetzte Ausgangslage. CPU-Exportnachweis: `Saved/Logs/HarvesterAnimationV9-Authoring.log`, Marker `HARVESTER_ANIMATION_V9_SOURCE_PASS 36884 11`. Es wurde dabei kein GPU-Render gestartet.

## Komponente und Hookplan

`UPrototypeCollectorVisualComponent` besitzt alle dekorativen Teilkomponenten als `UPROPERTY` und registriert sie außerdem als Actor-Instance-Components. Sie tickt nicht selbständig. Der Hauptactor ruft nach seiner bestehenden Simulationsprüfung explizit die Präsentation auf. Damit gelten dieselben Pause-/Matchende-/Gesundheitsprüfungen wie beim Sammler selbst.

Der Integrator ergänzt ausschließlich:

```cpp
#include "PrototypeCollectorVisual.h"
// Am Ende von ConfigureRole, nach Modellauswahl und Fraktionsmaterialien:
UPrototypeCollectorVisualComponent::SyncRole(this, Mesh);
// Direkt nach dem bestehenden TickCollector-Aufruf, innerhalb der Tick-Guards:
UPrototypeCollectorVisualComponent::TickVisual(this, DeltaSeconds);
```

Vor dem Wechsel des Rumpfes müssen alle elf Assets vorhanden sein. Sonst bleibt der vollständige bisherige Sammler erhalten, ohne duplizierte Teile. Beim Rollenwechsel wird die alte Präsentationskomponente mitsamt Teilkomponenten zerstört. Bei direktem Entfernen der Komponente aus einem weiterhin als Sammler konfigurierten Actor wird der originale Rumpf samt Materialbindungen restauriert. Actor-Zerstörung verwendet Unreal-Ownership und explizite Teilebereinigung. Alle dekorativen Teile besitzen weder Collision, Overlap, Navigationseinfluss noch Schattenwurf und bleiben unter dem normalen Actor-/Fog-Visibility-System.

Die Komponente liest Role, Cargo, CollectorState und Health, verändert jedoch keine dieser Größen, keine Bewegung, Dockreservierung, Geldwerte oder Timer. Seeking hebt den Kopf stärker; Harvesting senkt ihn und dreht die tatsächliche Trommel; Rückfahrt hebt ihn; Waiting senkt ihn in eine eigene Bereitschaftslage; Unloading streckt die echte Dichtmanschette leicht. Die sieben Originalanzeigesegmente folgen dem geglätteten tatsächlichen Füllstand. Die genaue visuelle Unterscheidbarkeit muss aus Spielkameraaufnahmen bewertet werden; kleine unterschiedliche Armwinkel allein werden nicht als vollständige UX-Abnahme ausgegeben.

## Nächste Abnahme

1. Root führt `import_harvester_animation_v9.py` seriell aus. Es verändert keine Map oder Unit-Definition und prüft alle lokalen Bounds/Pivots und Materialslots in Unreal. Importierte Assets liegen unter `/Game/HERO/HarvesterAnimationV9/Meshes`.
2. Hooks integrieren, Editor bauen und prüfen, dass Missing-Asset-Fallback und Rollenwechsel keinen unsichtbaren oder doppelten Sammler erzeugen.
3. Tatsächlichen Feld→Raffinerie→Feld-Zyklus aufnehmen, inklusive leerem/vollem Tank und Dockwarteschlange. Erntekopf muss sich sichtbar bewegen, Kontakt zum eigenen Fahrzeug behalten und darf im Gelände nicht versinken. Der jetzige Arm ist eine gemeinsame starre Baugruppe inklusive Hydraulik; echte separate Kolbenlängen und Einzelradrotation sind damit noch nicht implementiert.
4. Bestehende Ressourcen-/Dock-/Stressprüfungen unverändert ausführen. Kein neuer Test darf Wirtschaftsprüfungen durch rein visuelle Erfolgsmeldungen ersetzen.

Eine neue Paketabnahme, Konzeptparität oder Übereinstimmung mit Tempest-Animationszeiten wird hier nicht behauptet.
