# Reference V7: Sichtprüfung und Integrationsvertrag

Aktualisiert am 16. Oktober 2026. Die neue facettierte Aegis-Zentrale und eigenständige Ember-Modelle wurden in ihren tatsächlichen Blender-Renders betrachtet. Der Atlasimport wird parallel vorbereitet. **Eine aktuelle Paketaufnahme dieser vollständigen Kombination liegt für diese Prüfung noch nicht vor.** Das ist keine Freigabe als fertiges Spiel oder vollständige Konzepttreue.

## Neu betrachtete Quellen

- Erneut tatsächlich geöffnet: `Art/Models/ReferenceHQV7/SM_RV7_AegisHQ.png`, neue facettierte Revision mit 35.568 Dreiecken laut zugehörigem Manifest. Sie ersetzt die zuvor geprüfte flache Zentrale mit großem Garagentor.
- Tatsächlich geöffnet: `Art/Models/ReferenceEmberHQV7/SM_RV7_EmberHQ.png` und `Art/Models/ReferenceEmberAPCV7/SM_HR_Ember_APC8.png` sowie zuvor der Aegis-APC-Render.
- Vergleich mit derselben Nutzerreferenz `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_20.png` für beide Basen und Fahrzeuge.
- Der unten beschriebene Unreal-Capture vom 15. Oktober um 20:15:09 ist **historisch**. Sein damaliger SHA-256 war `4b086289f46cd01b23735f195814c4b2f6c8aeddbbacb603bd159b837ffe2896`. Er belegt weder die neue facettierte Zentrale noch die vollständigen vier Materialatlanten. Der generische Capture-Pfad kann inzwischen überschrieben worden sein.

## Neuer visueller Befund

| Bereich | Tatsächlich betrachteter Stand | Verbleibende Lücke / nächste Abnahme |
| --- | --- | --- |
| Aegis-HQ | Aufragende facettierte Kommandozentrale mit gestuftem achteckigem Schüsseldach, umlaufenden blauen Fensterfeldern, breiten diagonalen Wangen, kleinen Türen und einem flachen seitlichen Technikbau. Die hausartige Flachbox mit übergroßem Garagentor ist ersetzt. | Deutlich näher an der Referenzsilhouette; weiterhin eigene vereinfachte Panel-/Rohranordnung, statische Schüssel und saubere prozedurale Studiomaterialien. Die Wirkung der neuen Albedo-/ORM-Atlanten im Spiellicht ist noch nicht belegt. |
| Ember-HQ | Eigenständige Portalhalle mit schrägen Wangen, asymmetrischen Flügeln, versetzter Dachkrone, zwei schlanken Antennen und grünen Kennzeichnungen. Form und Ausstattung unterscheiden es vom Aegis-Bau. | Die Referenz ist dunkler, dichter gepanzert und stärker gebraucht. Der Studio-Render bleibt relativ gleichmäßig grau. Aktuelles Spiellicht mit Materialvariation prüfen; eine dunklere Wirkung darf nicht allein aus Unterbelichtung entstehen. |
| Aegis-APC | Acht Räder, flache sandfarbene Panzerwanne, Keilfront und kantiger Turm ersetzen den Pickup-Grundriss. | Neue Combat-Nahaufnahme mit finalen Atlanten erforderlich. Bewegliche Räder/Turm, Normaldetails und Gebrauchsspuren bleiben gesonderte Qualitätsfragen. |
| Ember-APC | Eigenständige gekappte Front, breiter flacher Sensorturm, versetzte Bewaffnung, große Radkästen und acht Räder mit grünen Kennzeichnungen. | Gleichmäßige Flächen und statische zusammengefügte Geometrie bleiben sichtbar. Dunklen Materialeindruck und Formunterschied in echter Spielansicht prüfen. |

Neuere Änderungen an UI-Lesbarkeit, Umgebung und Atlanten sind durch den historischen Capture nicht nachgewiesen. Die Weltlücke mit großen gleichmäßigen Bodenflächen und wiederholten Fassaden wird durch neue HQ-/APC-Modelle nicht automatisch behoben.

## Atlasvertrag und aktuelle technische Prüfungen

Der Art-Agent erstellt pro Modell `/Game/HERO/ReferenceV7/Materials/M_RV7_Baked_{key}`. Alle gespeicherten Slots eines Modells verwenden dieses Material; semantische Slotnamen bleiben erhalten. Texturen liegen unter `/Game/HERO/ReferenceV7/Textures/T_RV7_{key}_{Albedo|ORM|EmissionMask}` und werden über die gleichnamigen TextureSampleParameter2D gebunden.

| Key | Exaktes Mesh unter `/Game/HERO/ReferenceV7/Meshes/` | Atlasauflösung |
| --- | --- | --- |
| `Aegis_HQ` | `SM_RV7_AegisHQ` | 2048 × 2048 |
| `Aegis_APC8` | `SM_HR_Aegis_APC8` | 1024 × 1024 |
| `Ember_HQ` | `SM_RV7_EmberHQ` | 2048 × 2048 |
| `Ember_APC8` | `SM_HR_Ember_APC8` | 1024 × 1024 |

Albedo verwendet sRGB/TC_Default. ORM und EmissionMask verwenden lineare Daten/TC_Masks. ORM ist als AO/Rauheit/Metallizität vorgesehen, EmissionMask.R bleibt separat. **Farbgestaltung liegt im Atlas; ein zusätzlicher Fraktions-Tint würde sie verfälschen.** Initialwert und Wert nach Schadensblitz sind daher neutrales Weiß. Gegnerkennzeichnung über Lebensbalken, Auswahl und UI bleibt unabhängig von grünen Ember-Assetmarkierungen.

`HRPresentationTests.cpp` enthält jetzt **36 Referenzprüfungen**, die die zehn Prüfungen der ersten V7-Revision ersetzen. Übrige bestehende Presentation-Prüfungen bleiben erhalten:

1. Alle vier Runtime-Actors zeigen ihre exakten gespeicherten Meshes; die vier Geometrieobjekte sind verschieden.
2. Transformierte Bounds beider HQs und APCs stehen nach Actorhöhe, Meshversatz, Skalierung und Rotation auf Welt-Z0. Beide HQs passen in den bestehenden Bau-Footprint; die APCs behalten den vorgesehenen Fahrzeugmaßstab.
3. Jeder gespeicherte und tatsächliche Runtime-Slot behält das eigene Atlasmaterial mit neutralem Tint. Die alte Ausnahme für einzelne weiße HQ-Materialnamen ist durch diese Texturbindung ersetzt.
4. Alle zwölf Atlastexturen besitzen den exakten Assetpfad, die richtige Auflösung, sRGB- und Kompressionsklasse. Ihre Parameterbindung wird zusätzlich in jedem tatsächlich verwendeten Material geprüft. Der NullRHI-Editor prüft die importierte Sourceauflösung; gerenderter Editor und Paket benötigen die volle Plattformtexturauflösung.
5. Jede der vier Einheiten erhält real `ReceiveHit(1)` und `Tick(.03)`. Gesundheit muss sinken und der gelesene Team-Materialparameter muss sich ändern. Nach `Tick(.2)` muss der ursprüngliche neutrale Atlas-Tint wiederhergestellt sein. Damit wird eine dauerhafte erneute Fraktionseinfärbung nach Treffern abgefangen.

Dies sind implementierte Prüfungen, keine Behauptung über bereits bestandene neue Pakettests. Dieser Agent führt keine Builds, Imports oder Commits aus. Root prüft nach den Importen und archiviert die tatsächlichen Ergebnisse. Als nächste visuelle Abnahme aktuelle separate Paketbilder für beide HQs und APCs aus Front-/Seitenwinkeln erstellen, passende Auswahlportraits kontrollieren und Trefferblitz samt Ruhebild im Rendering beurteilen.

## Historischer Audit der ersten V7-Revision

Die folgenden Beobachtungen und die Zahl zehn beschreiben ausschließlich den damaligen Stand vor dem facettierten HQ und den vier Atlanten. Sie werden als Verlauf erhalten und sind keine aktuellen Abnahmebelege.

Stand: 15. Oktober 2026. Dies ist eine Bewertung der tatsächlich sichtbaren Modelle und des Unreal-Captures, keine Freigabe als fertiges Spiel oder vollständige Konzepttreue. Neue Ember-Modelle werden parallel erstellt und waren im hier betrachteten Bild noch nicht nachgewiesen.

## Betrachtete Belege

- Originalreferenz: `E:/HeroHour/Art/Concepts/HH Art/Codex-Bild 1. Sept. 2026, 01_22_20.png` (Basen und gepanzerte Fahrzeuge beider Fraktionen).
- UI-Komposition: die ebenfalls vom Nutzer gelieferte Illustration `01_22_00.png`.
- Tatsächliche Blender-Renders der final exportierten Geometrie: `Art/Models/ReferenceHQV7/SM_RV7_AegisHQ.png` und `Art/Models/ReferenceAPCV7/SM_HR_Aegis_APC8.png`.
- Unreal-Spielaufnahme: `Art/Previews/ConceptV3-Battle.png`, Dateistand 15. Oktober 2026, 20:15:09. Der generische Capture-Pfad wird bei weiteren Durchläufen überschrieben; endgültige Nachweise müssen unter eigenständigen V7-Dateinamen archiviert werden.

## Vergleich und verbleibende Abweichungen

| Bereich | Im betrachteten Stand sichtbar | Noch nicht konzeptgerecht / nächste konkrete Abnahme |
| --- | --- | --- |
| Aegis-HQ | Eigenständiger rechteckiger weiß-blauer Bau, gestufte horizontale Dachlinie, große echte Schüssel. Die frühere Rundfestung dominiert die Silhouette nicht mehr. Das Auswahlportrait zeigt dieses neue Modell. | Die Spielkamera zeigt vorwiegend die geschlossene Rückseite. Garage und asymmetrischer Seitenflügel aus der Referenz sind kaum lesbar. Eine Aufnahme von der Garagenseite ist notwendig. Fassaden sind zu gleichmäßig und sauber; Paneltiefe, angeschlagene Kanten, lokale Verschmutzung und technische Dachdetails fehlen gegenüber der Referenz. |
| Aegis-APC | Die Quelldarstellung zeigt acht große Räder, eine zusammenhängende flache Panzerwanne, Keilfront und kantigen Geschützturm. Damit ist der frühere Pickup-Grundriss ersetzt. | Die Fahrzeuge im Battle-Capture sind zu klein und von einer ungünstigen Seite sichtbar, um Radzahl, Radkästen und Turm vernünftig abzunehmen. Eine echte Combat-Nahaufnahme ist erforderlich. Wanne und Turm wirken in der Studiovorschau noch zu glatt; Rad-/Turmanimation und gebrauchte Materialdetails sind offen. |
| UI-Aufteilung | Kompakte Ressourcenzeile oben links, Armee oben mittig, echte Commander-Fähigkeiten rechts oben, Auswahl unten mittig und fünf Produktionsspalten rechts unten sind sichtbar. Die Auswahl verwendet das neue HQ-Portrait. | Das Konzept besitzt stärkere metallische Tiefenstaffelung, deutlicher getrennte Informationshierarchien und reichere Icon-/Portraitdetails. Aktuell bleibt viel gleichmäßiges grauschwarzes Flächenmaterial; mehrere Texte sind klein. Die Ähnlichkeit der Positionen allein ist keine visuelle Abnahme. Funktionstests müssen die neuen Kartenpositionen zusätzlich prüfen. |
| Basisumgebung | Heller Wüstenboden, Straßenmarkierungen, Strommasten, Gebäude und ein klarer befahrbarer Bereich sind sichtbar. | Die sichtbaren Flächen sind groß, flach und gleichmäßig. Die Referenz lebt von Felsabbrüchen, Schotter, abgenutzten Vorplätzen, kleinen Vegetationsgruppen und spezifischen industriellen Requisiten. Diese Dichte und Materialvariation ist noch nicht erreicht. |
| Stadt / Fraktionsunterschied | Aufrechte Stadtbauten stehen im erkennbaren Straßennetz. Aegis-HQ und APC erhalten eigene Geometrie. | Die Hintergrundstadt wiederholt weiterhin einfache Fassaden; sie besitzt noch nicht die urbane Dichte und Höhenstaffelung der Konzeptbilder. Der hier geprüfte Screenshot belegt keine fertige neue Ember-Silhouette. |

Unaufgedeckte Bereiche erscheinen im Spiel durch Fog of War dunkel. Aus ihnen lässt sich in dieser Aufnahme keine Aussage über die dahinterliegende Detailqualität ableiten. Eine separate gekennzeichnete Art-Review-Ansicht ist dafür der geeignete Beleg; ein Gameplay-Capture soll die echte Spielsicht behalten.

## Technische Schutzprüfungen dieser Welle

`Source/HeroHourPrototype/RTS/HRPresentationTests.cpp` ergänzt zehn Prüfungen am tatsächlich erzeugten Runtime-Actor:

1. Aegis-HQ und APC zeigen exakt das jeweilige V7-Mesh, statt lediglich einen gespeicherten Soft-Reference-Pfad zu besitzen.
2. Die jeweiligen Ember-Rollen zeigen ihre eigenen gespeicherten Meshes und keine umgefärbte Aegis-Geometrie. Alte Ember-Pfade werden nicht festgeschrieben; neue eigenständige Ember-Modelle können integriert werden.
3. Die transformierten Aegis-Meshbounds stehen nach realer Actorhöhe, Meshversatz, Skalierung und Rotation auf Welt-Z0. Das HQ bleibt innerhalb seines bestehenden Bau-Footprints; der APC behält den bisherigen Fahrzeugmaßstab.
4. Beide lokalen HQ-Materialien behalten nach der Runtime-Fraktionszuweisung tatsächlich einen hellen neutralen Tint. Das verhindert, dass die neue weiße Architektur durch die allgemeine sandfarbene Fahrzeugpalette überschrieben wird.

Diese Prüfungen ersetzen weder die Bildkontrolle noch eine neue Paketierung. Dieser Agent hat keine Builds, Imports, Commits oder fremden Dateien ausgeführt bzw. verändert. Root führt die Integration und die finalen Editor-/Pakettests aus und dokumentiert deren Ergebnisse separat.

## Abnahmereihenfolge

Zuerst HQ-Garagenseite und APC in echten Unreal-Nahaufnahmen prüfen; danach dieselbe Material-/Geometrieprüfung auf die neuen Ember-Modelle anwenden. Anschließend Oberflächenalterung und Basis-Nahbereich gezielt an diesen Bildausschnitten ausarbeiten. Erst nach einer sichtbar kohärenten Referenzszene weitere Rosterrollen verbreitern. Weder eine höhere Anzahl erzeugter Modelle noch bestandene technische Gates belegen für sich allein die gewünschte visuelle Qualität.
