# Tempest Rising: REA V7 zu Darstellung, UI-Palette und Ressourcenmeshes

Dokumentstand: 16. Oktober 2026. Eigenständiger REA-Agent parallel zu HeroHours Art-/UI-Integration. Verwendeter Skill: `C:/Users/Hero/.agents/skills/reverse-engineer-anything/SKILL.md`. REA bedeutet hier **Reverse Engineer Anything**, kein RTS-Framework.

## Ergebnis und Untersuchungsgrenze

Dieser Pass ergänzt die vorhandenen Feld-, Produktions- und Docking-Berichte. Er wiederholt weder die bereits zeitüberschrittene vollständige Shipping-Analyse noch die gescheiterte verschlüsselte Paketauflistung. Es wurde kein Spiel oder Editor gestartet, keine native UI bedient und kein fremdes Modell, Shader oder Texturbild in HeroHour übernommen.

Neu sind ein nativer Ressourcen-Darstellungskörper mit **1.351 gegen die Originaldatei geprüften Bytes**, **35 tatsächlich zugeordneten gespeicherten UI-Farbwerten** und ein begrenzter Metadatenbeleg zu Sammleranimationen sowie separaten Fahrzeug-/Gebäudematerialien. Diese Belege erklären konkrete Arbeitsschritte für unsere eigenen Assets. Sie beweisen keine fertige Tempest-Rekonstruktion und keine visuelle Konzepttreue von HeroHour.

| Frage | Status | Belastbarer Umfang |
| --- | --- | --- |
| Welche Zustandsfarben sind im ausgelieferten UI-Materialsystem tatsächlich gespeichert? | Beantwortet für zwei Editor-MPCs | Alle 14 bzw. 21 VectorParameters wurden mit Namen und Floatwerten aus geprüften Propertyframes gelesen. Runtime-Overrides und Widgetbindungen bleiben offen. |
| Besitzt die Feld-Darstellung echten Code statt lediglich benannter Stubs? | Teilweise beantwortet | `GenerateSplineMeshComps` erstellt und konfiguriert SplineMeshComponents. Wachstumsraten und Shipping-Ausführung werden damit nicht bestimmt. |
| Wie werden Sammlerbewegung, Ernte und Ablieferung visuell getrennt? | Teilweise beantwortet | Ausgelieferte Asset-Metadaten enthalten getrennte Animations-/Montagenamen und Referenzpfade. Abspielzeitpunkte, Animationsdauer und Runtime-Verknüpfung sind nicht beobachtet. |
| Welche sichtbaren Oberflächeneigenschaften fehlen unseren glatten Modellen? | Begründete Arbeitsableitung | Materialmetadaten weisen auf eigene Normal-/Masken-/Schmutz-/Schadensbereiche hin. Die konkrete Shaderauswertung und tatsächlichen Texturpixel wurden nicht rekonstruiert. |

## Ziele und Identitäten

Der vom Nutzer genannte Einstieg bleibt `F:/Tempest-Rising-SteamRIP.com/Tempest Rising/Tempest.exe`. Die vorherigen Untersuchungen haben Launcher, große Shipping-EXE und mitgeliefertes Editorprojekt unterschieden. Dieser neue Pass verwendet das bereits identifizierte Editorartefakt aus genau dieser Installation; er gibt dessen Befunde nicht als direkten Laufzeitnachweis der Shipping-EXE aus.

| Artefakt unter der Installationswurzel | SHA-256 |
| --- | --- |
| `MapEditorProject/Binaries/Win64/UnrealEditor-Tempest.dll` | `2fff65ec4f3809e8f24010d5cbfa9ada2f2834b59c997569c219d6a494f0b4fa` |
| `MapEditorProject/Content/Tempest/UI/Materials/MPC_UI.uasset` | `84392ebbca7751b72c7c8ac880d95faa82c8ea7f96c4e4b54d6c29a2f220a482` |
| `MapEditorProject/Content/Tempest/UI/Materials/MPC_UI_Constant.uasset` | `704324d8b6896144cd21693951dbcf194015e7d0d6f64b2231bfea8b38a987a2` |
| `MapEditorProject/Content/Tempest/MaterialLibrary/Vehicles/M_MASTER_Vehicles.uasset` | `231f9ee0935bd68ca8c52666e34bdb24a2115e6f4d4f8e3138e1e1efc1897447` |

Die Zuordnung dieser lokalen Installation zu einer unveränderten offiziellen Veröffentlichung ist weiterhin nicht geprüft. Editorartefakte und Shipping-IoStore-Inhalte sind nicht als byteidentisch nachgewiesen.

## REA-Sitzung und neue native Belege

REA war erreichbar, ohne geöffnetes Ziel. Das passende vorhandene Editor-Snapshot wurde importiert. Neue fokussierte Suchen und ein Funktionsdossier wurden mit Ghidra 12.1.4 erstellt. Nach Evidence-Export wurde die Sitzung mit einem neuen Snapshot geschlossen. Der Export enthält 22 erhaltene Records **einschließlich übernommener Altbelege**; die Zahl ist keine Anzahl neuer Funktionsanalysen.

| Evidence-ID | Beobachtung und Grenze |
| --- | --- |
| `ev_60a942d63fc50c776a824d56ff4c7757d8f452490208d3aa2bcd91190910ddff` | Regexsuche nach `SetPlayerColor`, `SetSelected`, `SetSelection`, `Update.*Material`, `GetUnitIcon`, `GetUnitPortrait`: keine passenden analysierten Prozedurnamen. Das ist kein Beweis, dass es diese Funktionen im Shipping-Spiel oder in Blueprints nicht gibt. |
| `ev_927ee3a4cf398f87cfa8104f9482223f13d8e07f58d30782431ffb7436c47748` | Erweiterte Suche nach Mesh/Material/Highlight/Selection/Portrait/Harvester: 200 Namen, davon 68 innerhalb der DLL. Viele übrige Treffer sind Engine-Imports. Generierte Konstruktor-/Reflexionsnamen wurden nicht als echte HUD-Implementierung ausgegeben. |
| `ev_6d3b6a254d08a1a880e2e4e97de0d12e23a4392cc2a5f979d2bef7c05e9d05c4` | Dossier für `ATempestFieldVolume::GenerateSplineMeshComps`, Adresse `0x1800dc950`, RVA `0xDC950`, ein inklusiver Körperbereich bis `0x1800dce96`, 1.351 Bytes. |
| `ev_cb6c6b8b4e53fc5f4a15bcb14256a31cdbdf43f3dbd5c56a7cc6839dfa08c902` | Genau 1.351 vollständige analysierte Bytes ab dieser Adresse gelesen. Separater PE-Prüfer bildet sie auf Original-Dateioffset 900.432 ab und vergleicht jedes Byte. |

**Observed:** Der Körper iteriert eine Liste von Splines und deren benachbarte Punkte. Er konstruiert `USplineMeshComponent`-Objekte, registriert sie, fragt Positionen und Tangenten ab, setzt Start/Ende und Start-/Endskala und ruft `UpdateMesh` auf. Er ruft `SetUsingAbsoluteScale(true)` und `SetReceivesDecals(false)` auf. Ein virtueller Aufruf erhält den importierten Namen `NoCollision_ProfileName`. Er legt erzeugte Komponenten in einem Array ab. Ein eingebetteter Assertionpfad verweist auf `Resource/TempestFieldVolume.cpp`.

**Inference:** Dies ist eine echte geometrische Darstellungsstrecke für segmentierte Feldformen. Der NoCollision-Argumentname spricht für eine Trennung sichtbarer Feldgeometrie und Kollision. Die exakte Zielmethode dieses virtuellen Aufrufs wurde nicht vollständig typisiert; ein tatsächlich laufender Kollisionszustand ist damit nicht beobachtet.

**Unknown:** Koordinatenraum und sämtliche privaten Felder, Quelle der Splines, konkrete Materialien, Wachstums-/Erschöpfungstrigger sowie der entsprechende Shipping-Aufrufpfad. Mehrere dekompilierte Argumenttypen sind unvollständig rekonstruiert. Ghidra meldet drei terminale Calls ohne vollständig geklärte No-Return-Basis; das bleibt eine Abdeckungsgrenze. Pseudocode ist kein Originalquelltext.

Der Bereichs-SHA-256 lautet `87a879461f5c125bd3f6d322ee608d12083a00a076e368fdeabf98ab6b0bbe65`. Originaldatei und analysierte Bytes sind gleich. Das prüft die Byteverankerung, **nicht** die Korrektheit einer Rekompilation oder beobachtete Spielausführung.

## UI: Zuordnung von Namen und gespeicherten Farben

Die beiden kleinen MPC-Dateien wurden direkt und nur lesend untersucht. Ein begrenzter Parser prüft Package-Tag, Legacy-Version −8, UE4-Version 522, UE5-Version 1012 und die Nametabelle. Er verfolgt den vollständigen Typnamen von `VectorParameters` zu `CollectionVectorParameter`, den deklarierten Elementcount und für jedes Element den unmittelbar passenden `DefaultValue`-Frame vom Typ `LinearColor` sowie den `ParameterName`-Frame vom Typ `NameProperty`. FName-Zahlensuffixe werden korrekt erhalten. Dies ist eine überprüfte Wertezuordnung, keine Nähevermutung zwischen benachbarten Strings.

`MPC_UI` enthält 14 Vektorparameter, darunter Self/Friendly/Hostile/Neutral und nummerierte PlayerColor-Parameter. `MPC_UI_Constant` enthält 21 Vektorparameter: sieben benannte Zustände für jeweils GDF, Dynasty und VETI.

| Gespeicherter Parameter / Gruppe | Linear-RGBA im Editorartefakt |
| --- | --- |
| `ThemeColor_Default_GDF`, `Hovered_GDF`, `Pressed_GDF`, `Selected_GDF`, `Active_GDF` | jeweils `(0.099899, 0.376262, 0.955974, 1)` |
| `ThemeColor_Disabled_GDF`, `Empty_GDF` | jeweils `(0.955974, 0.955974, 0.955974, 1)` |
| `SelectionColor_Self` | `(1, 1, 1, 1)` |
| `SelectionColor_Friendly` | `(1, 0.879623, 0, 1)` |
| `SelectionColor_Hostile` | `(0.701102, 0.049707, 0.049707, 1)` |
| `SelectionColor_Neutral` | `(0, 0.644480, 0.015996, 1)` |

**Observed:** Mehrere verschieden benannte Zustände besitzen identische gespeicherte Farben. **Inference:** Eine sichtbare Zustandshierarchie kann daher nicht allein aus sieben unterschiedlichen RGB-Werten dieser Sammlung erklärt werden. **Unknown:** Welche Widgets diese Parameter verwenden, ihre Laufzeitänderungen, zusätzliche Opacity-/Roughness-/Rahmen-/Animationseffekte und die finalen Bildschirmfarben. Linearwerte sind keine direkt beobachteten sRGB-Pixel. Die Zustandsnamen allein beweisen keine bestimmte Hoverdarstellung.

## Sammler- und Materialmetadaten

Drei ausdrücklich ausgewählte Editorordner wurden inventarisiert: `GDF_Harvester` mit 74, `GDF_RiotVan` mit 60 und `BD_GDF_ConYard` mit 85 Dateien. 50 ausgewählte `.uasset`-Metadaten mit höchstens 1 MB pro Datei wurden gehasht und nach begrenzten ASCII-Sequenzen gelesen. Die großen `.uexp`-/`.ubulk`-Daten wurden nicht dekodiert. Diese lokalen Befunde besitzen SHA-/Offsetbelege; ihnen werden **keine erfundenen REA-Evidence-IDs** zugewiesen.

**Observed:** Die Harvester-Metadaten enthalten ein SkeletalMesh, ein eigenes Skeleton, getrennte Materialpfade für Körper und Treads sowie Namen für Idle, MoveStart/Move/MoveStop, Harvest_Start/Loop/Stop und Unload_Loop/Stop. Die HarvestResource-Montagemetadaten enthalten die drei Harvest-Clip-Pfade; DumpResource enthält die beiden Unload-Clip-Pfade. Die konkreten Referenzen sind in `visual-metadata.json` mit Quell-Dateioffsets und vollständigen Datei-SHAs gespeichert.

**Observed:** `MI_GDF_Harvester` und `MI_GDF_RiotVan` enthalten Pfade zum Fahrzeugmaster und mehreren eigenen Texturassets; die Conyard-Materialmetadaten verweisen auf einen separaten Gebäudemaster. Der Fahrzeugmaster enthält unter anderem benannte Bereiche für Normalintensität, Damage Roughness, Dirt Color und Damage Mask. RiotVan nennt eine eigene PlayerMask. Das ist stärker als ein reiner Ordnername, bleibt jedoch Metadaten-/Stringbeobachtung.

**Unknown:** Die tatsächliche Zuordnung jedes Texturkanals, deren Pixel, Shaderverbindungen, Materialinstanz-Overrides, Bone-Transforms, Meshabmessungen, Animationsdauer und Abspielsequenzen. Suffixe wie RMOE/RMAO werden nicht ohne Graph-/Formatbeleg als feste Kanalpackung ausgegeben. Keine dieser Dateien wurde als HeroHour-Asset importiert.

## Konkrete Umsetzung in HeroHour

Diese Punkte sind eigene Designentscheidungen aus den Belegen, noch keine in diesem REA-Pass implementierten Features:

1. **UI-Zustand statt zusätzlicher Einfärbung:** Eine gemeinsame HeroHour-Palette für Default, Hovered, Pressed, Selected und Disabled anlegen. Spielzustand und Rahmen-/Füllkontrast gemeinsam definieren; Input und Zeichnung lesen denselben Zustand. Gesperrte Produktion braucht einen erkennbaren Grund und einen echten Verfügbarkeitscheck. Unsere Aegis-/Ember-Farben folgen den Nutzerkonzepten. Die Originalfloats sind Referenzdaten, keine ungeprüfte Ersatzgestaltung.
2. **Eigene Materialien für verschiedene Oberflächen:** Das neue weiße HQ und der sandfarbene APC benötigen unterschiedlich behandelte Beschichtung, blankes Metall, Gummi, Glas und Schmutzmasken. Formkorrektur bleibt notwendig; ein RGB-Wechsel ersetzt sie nicht. Als nächster Art-Schritt lokale Normal-/Rauheitsvariation, Kanten und bodennahe Verschmutzung an genau diesen beiden Referenzassets bearbeiten und im echten Spiellicht vergleichen.
3. **Sammlerzustände sichtbar machen:** Unsere vorhandenen Simulationszustände auf eigene visuelle Phasen abbilden: HarvestStart → HarvestLoop → HarvestStop, getrennt von UnloadStart/Loop/Stop und Bewegung. Die Darstellung darf weder Cargo buchen noch die autoritative Ernte-/Abladezeit verändern. Erst einen eigenen Sammler riggen und einen realen Sammel-/Lieferzyklus aufnehmen; keine Bestandslogik an die Animationsframerate koppeln.
4. **Ressourcenfeld als gewachsene Form:** Der eigene Feldvisualisierer kann kurze geschwungene Verbindungen zwischen Zellen und getrennte Wachstumsstadien verwenden. Für HeroHours festes Raster zuerst wenige eigene Segmentvarianten bzw. gepoolte Instanzen erproben, statt unkontrolliert Komponenten pro Tick zu erzeugen. Das ist unsere Performanceentscheidung; der Originalfund bestimmt weder Poolgröße noch Budget. Simulationsbestand, Claims und Gameplay-Kollision bleiben unabhängig.

Visuelle nächste Abnahme: Vorher/Nachher vom gleichen HQ-/APC-Kamerawinkel, sichtbare UI-Hover-/Sperrzustände und ein echter Sammlerzyklus mit getrenntem Ernten/Entladen. Technische Byteprüfungen ersetzen diese Bilder nicht.

## Lokale Belege und Reproduktion

Die kompakte versionierte Ablage liegt unter [`docs/verification/2026-10-16/reference-v7/REA/`](../verification/2026-10-16/reference-v7/REA/): `native-byte-verification.json`, `ui-palette.json`, `visual-metadata.json` sowie die beiden Prüfer `decode_ui_palette.py` und `inspect_visual_metadata.py`. Die drei JSON-Dateien bewahren die unten genannten Digests; die beiden Prüfer lassen sich von dort mit den expliziten lokalen Originalpfaden erneut ausführen und schreiben neben sich.

Der vollständige REA-Rohbundle, das Sitzungs-Snapshot und der zugehörige native Byteprüfer bleiben unter `Saved/Research/TempestRising/V7/`. `Saved` ist im Repository ignoriert. Ein anderer Rechner benötigt diese vollständigen Sitzungsdaten separat oder eine erneute REA-Untersuchung, um den nativen Analysebytevergleich selbst zu wiederholen. Der versionierte JSON-Beleg dokumentiert dessen Ergebnis; er ersetzt nicht den vollständigen Rohbundle. Es werden keine fremden Assets versioniert.

| Datei | Inhalt / SHA-256 |
| --- | --- |
| `rea-evidence.json` | Vollständiger REA-Export mit Alt- und Neubelegen; `64079cf9a0ce6e41bbae828dc178fa79926bc170f6cf2572f78fe95b740c0f60` |
| `editor-visual-snapshot.json` | Geschlossene, wiederverwendbare Sitzung; `db5ef57a953aefc158d914b0d8f4a5235e28e0d9774b668c0bbc7f8416eca5b9` |
| `native-byte-verification.json` | Original-/Analysebytevergleich; `3718760da8c86bfbc85b7fc61bb24ecd277ccf67c562f973034d9f83c9d5ad47` |
| `ui-palette.json` | 35 Werte mit Property-/Wert-/Namensoffsets und Frame-SHAs; `8e224678b6be9c7779a5e2eaf881b1a7e273c17b6f22bee168f3275273f8b764` |
| `visual-metadata.json` | 50 Metadaten-Digests und drei begrenzte Inventare; `af9d0e5634e16033014211140190cc11eee52a94f4415f330e199c4e329c8b71` |

Die dort gespeicherten Prüfer `inspect_visual_metadata.py`, `decode_ui_palette.py` und `verify_native_bytes.py` verwenden nur die expliziten Originalpfade und das neue Forschungsunterverzeichnis. Ausführen mit dem vorhandenen Bundled-Python:

```powershell
& 'C:/Users/Hero/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Users/Hero/HeroHour/Saved/Research/TempestRising/V7/inspect_visual_metadata.py'
& 'C:/Users/Hero/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Users/Hero/HeroHour/Saved/Research/TempestRising/V7/decode_ui_palette.py'
& 'C:/Users/Hero/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' 'C:/Users/Hero/HeroHour/Saved/Research/TempestRising/V7/verify_native_bytes.py'
```

Erfolgsmarker dieses Durchlaufs: `VISUAL_METADATA_READ_PASS 50 records 1094 strings`, `UI_PALETTE_FRAME_PASS 35 colors`, `V7_NATIVE_BYTE_ANCHORING_PASS 1351 bytes`. Ein fehlgeschlagener früher Decoder-Versuch hatte nummerierte FNames zunächst abgewiesen; die finale Version erhält die Nummern und prüft den deklarierten Arraycount. Es wurden keine ungültigen Zwischenwerte als Befund verwendet.
