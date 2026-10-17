# FrontlinesV4: Modelle, Definitionen und Sichtmaterialien

Stand: 14. Oktober 2026. Die Modelle sind originale HeroHour-Geometrie. Ihre
Blender-Vorschauen zeigen die tatsächlich exportierten Meshes. Die Zahlen in
DataV4 sind eigene Spielbalance und keine aus Tempest Rising gewonnenen Werte.

## Modellpaket

`Scripts/art/build_frontline_assets_v4.py` erzeugt sechs FBX-Dateien, sechs
bearbeitbare `.blend`-Quellen, sechs Prüfberichte und ein vollständiges Manifest
unter `Art/Models/FrontlinesV4`. Ein gezielter Neuaufbau mit Rollennamen nach `--`
erhält den vollständigen Manifest. Der Export prüft das resultierende FBX durch
erneuten Blender-Import; alle sechs Modelle haben dabei geschlossene Geometrie
ohne nichtmannigfaltige Kanten und Maße innerhalb 1 mm des Sollwerts.

| Mesh | Maße X × Y × Z in m | Dreiecke | Erkennbare Geometrie |
| --- | --- | ---: | --- |
| `SM_HR_PowerPlant` | 4,8 × 4,4 × 5,0 | 20.860 | Zwei Reaktorgehäuse, Kühlrippen, Isolatoren, Stromschienen, Verteilerportal |
| `SM_HR_Barracks` | 5,0 × 4,0 × 3,3 | 9.836 | Zwei Zugänge, Wandstützen, Filtergitter, Dachlüfter, Funkanlage |
| `SM_HR_Radar` | 4,0 × 4,0 × 6,0 | 9.224 | Geschlossene konkave Parabolschüssel, Empfängerstreben, Drehkranz, Mast, Leiter |
| `SM_HR_Defense` | 3,0 × 3,0 × 2,5 | 6.444 | Gepanzerte Kasematte, Geschützturm, zwei Rohre, Optik, Nebelwerfer |
| `SM_HR_Infantry` | 0,9 × 0,7 × 1,8 | 11.076 | Helm, Visier, Atemschutz, Körperpanzerung, Rucksack, gehaltenes Gewehr |
| `SM_HR_Siege` | 3,8 × 2,2 × 2,6 | 19.160 | Kettenlaufwerk, Panzerplatten, Rohrmantel, Rücklaufzylinder, Abstützung |

Alle Modelle besitzen UVs, Materialslots und einen Bodenpivot bei MinZ −0,35 m.
Mit Actor-Z 35 cm und Skalierung 1,0 liegt die Unterkante auf Z 0. Fahrzeuge und
Infanterie richten ihre Waffen nach +X aus. Chamfers, Rohre, Lüfter, Schrauben und
Fahrwerksdetails sind Mesh-Geometrie. Es gibt keine fremden Spielmodelle in diesem
Paket. Die Infanterie ist derzeit ein statisches Mesh ohne Skelett oder Animation.

`Scripts/art/import_frontline_assets_v4.py` importiert ausschließlich diese sechs
Meshes nach `/Game/HERO/FrontlinesV4/Meshes`. Es weist vorhandene Materialien
`/Game/HERO/ConceptV2/Materials/M_CV2_*` zu und verändert keine Materialgraphen
oder Karten. Der Unreal-Import prüft Zentimetermaßstab und Bodenpivot erneut.

Die sechs transparenten, echten Mesh-Portraits liegen als `T_HR_{Role}.png` unter
`Art/UI/Portraits`, mit Vorschaukopien unter `Art/Previews/FrontlinesV4`.
Der DataV4-Importer erwartet ihre Texture2D-Assets unter `/Game/HERO/UI/T_HR_*`.

## Gespeicherte Gameplay-Definitionen

`Scripts/art/create_rts_definitions_v4.py` authoriert 25 UPrimaryDataAssets unter
`/Game/HERO/DataV4`: je elf Definitionen `D_Aegis_*` und `D_Ember_*`, die Fraktionen
`D_Faction_Aegis`, `D_Faction_Ember` und `D_DamageMatrix`. Die stabilen Rollensuffixe
sind `Combat`, `Collector`, `HQ`, `Depot`, `Factory`, `PowerPlant`, `Barracks`,
`Radar`, `Defense`, `Infantry`, `Siege`. Sie entsprechen dem C++-Roster.

Die fünf bestehenden Rollen behalten eigene ConceptV2-Meshes und Portraits.
DataV3 bleibt unverändert; DataV4 beschreibt das neue vollständige Roster mit
Strombedarf, Sichtweite, Produktionskategorien, Fähigkeiten und Veteranenwerten.

| Neue Rolle | Kosten | Bauzeit in s | HP | Strom: Angebot / Bedarf |
| --- | ---: | ---: | ---: | --- |
| PowerPlant | 220 | 9 | 450 | 150 / 0 |
| Barracks | 180 | 7 | 450 | 0 / 40 |
| Radar | 250 | 9 | 350 | 0 / 60 |
| Defense | 200 | 8 | 350 | 0 / 50 |
| Infantry | 80 | 4 | 70 | 0 / 0 |
| Siege | 220 | 8 | 160 | 0 / 0 |

HQ liefert 60 Strom; Raffinerie benötigt 30 und Fahrzeugfabrik 80. Mobile Rollen
besitzen 2.600 cm Sicht, Gebäude 2.200 cm. Basisbewegung: Combat/Collector 220,
Infantry 250, Siege 180 cm/s. Aegis erhält Bewegungsfaktor 1,0 und Rüstungsfaktor
1,15; Ember 280/220 und 0,9. Aegis erhält Rauch für mobile Kampfeinheiten; beide
Fraktionen erhalten Infanteriegranaten und Artillerie-Deployment. Diese Zuordnung
ist ebenfalls in den Ability-Tags der Definitionen gespeichert.

Infanterie: 12 kinetischer Schaden, 450 cm Reichweite, 0,6 s Angriffsintervall.
Artillerie: 45 explosiver Schaden, 900 cm, 2 s. Abwehr: 24 Schaden, 850 cm, 0,8 s.
Ember-Combat und Ember-Defense verwenden Tempest-Schaden. Veteranenstufen bei
100/250/500 XP liefern die explizit gespeicherten Schaden-/Reichweite-/Rüstungsboni.

| Schaden \ Rüstung | Infantry | Vehicle | Structure |
| --- | ---: | ---: | ---: |
| Kinetic | 1,10 | 1,00 | 0,50 |
| Explosive | 1,50 | 0,85 | 1,40 |
| Tempest | 1,00 | 1,20 | 0,90 |

Die Fraktionsdefinitionen enthalten vier mobile und sieben Gebäuderollen,
RegrowingField-Wirtschaft und SidebarPlacement-Bauweise. Beide Fraktionen nutzen
derzeit dieselben Basis-Meshes; Fraktionsfarbe, Werte, Schadensarten und Fähigkeiten
unterscheiden sie. Eigene zusätzliche Silhouetten und Animationen sind ein
weiterer Art-Schritt und werden durch das Datenformat bereits ermöglicht.

Das Skript validiert sämtliche Klassen, native Tags, Meshes, Portraits und
bestehenden Assetklassen vor der ersten Änderung. Nach dem Speichern prüft es
alle Werte, Soft-Referenzen, Roster und jede Matrixabfrage. In einem frischen
Unreal-Prozess mit `-VerifyHRDefinitionsV4` führt es ausschließlich die Leseprüfung
aus. Ergebnisberichte liegen in `Saved/Research/rts-definitions-v4*.json`.

## Fog-of-War und Effekte

`Scripts/art/create_fog_effects_v4.py` erzeugt drei eigene Materialien unter
`/Game/HERO/EffectsV4`: `M_PP_Fog`, `M_FX_Smoke`, `M_FX_Impact`. Es ändert keine Karte.

`M_PP_Fog` ist ein Postprocess-Material nach Tonemapping. Es liest die Weltposition
aus dem Szenentiefenpfad und berechnet `UV=(WorldXY+(12000,9000))/(24000,18000)`.
Die laufend aktualisierte Texture2D muss am MID-Parameter `FogMask` gebunden
werden. Ihr Alphakanal enthält sichtbar 0, erkundet 0,55, unbekannt 0,92. Außerhalb
der Spielgrenzen bleibt Alpha 0,92. Szenenfarbe wird mit Schwarz interpoliert;
der anschließend gezeichnete Canvas-HUD bleibt lesbar.

Das reine C++-Modul `FHRVisibilityGrid` stellt getrennte aktuelle Sicht und
persistente Erkundung je Team bereit. Das Raster liefert die Render-Maske;
Actor-Abfragen prüfen exakte Sichtreichweite und Rauch. Die Alpha-Maske ersetzt
keine Informationsfilter: Gegner-Marker, Lebensleisten, Zielbefehle und Scans
müssen zusätzlich `CanTeamSee` verwenden.

`M_FX_Smoke` liefert statische räumliche Noise-Struktur und weiche Fresnel-Kanten
mit Parametern `Tint` und `Opacity`. `M_FX_Impact` liefert additive Lichtwirkung
mit `Tint`, `Intensity`, `Opacity`. Beide nutzen keine Shader-Wanduhranimation;
Lebenszeit und Aktivierung gehören der simulierten C++-Zeit, damit Pause und
Matchende konsistent bleiben.

Die geprüften lokalen UE-5.8-Quellen sind
`Engine/Shaders/Private/MaterialTemplate.ush::CalcMaterialParametersPost`,
`Engine/Classes/Engine/BlendableInterface.h` und die öffentlichen
`MaterialExpression*.h`-Header. Das Erzeugen eines Graphen bestätigt noch keine
korrekte RHI-Darstellung: Shader-Compile, Cook und Spielscreenshot erfolgen
gesondert durch den Root-Integrationslauf.
