# Tempest: erster konkreter Umgebungstransfer

Stand: 17. Oktober 2026. Ausschliesslich lokale Dateianalyse, keine Engine gestartet,
keine Installation veraendert, kein Asset importiert. REA-Skill angewandt; verwendet
wurde der vorhandene begrenzte Python-Paketparser, keine laufende REA-Nativesitzung.

## Auswahl

1. **Bevorzugt:** `/Game/Tempest/Props/Faction_Containers/SM_GDF_Container`.
   Kleines eigenstaendiges Umgebungsobjekt, direkt von der Beispielkarte referenziert.
   `.uasset` 1.692 Byte, `.uexp` 181.379 Byte, `.ubulk` 686.668 Byte.
   Paketflag `0x80002200`: cooked und FilterEditorOnly, unversionierter Summaryprefix.
   Uasset-SHA256 `0d6786a7f243874703e819bf774aa7bba419ffc313f77cb3ba1c3404b62fa774`.
   Die validierte Namenstabelle nennt genau ein Materialpaket:
   `/Game/Tempest/Props/Faction_Containers/Materials/MI_GDF_Container_01`.
2. **Kleiner Geometrie-Fallback:** `/Game/Tempest/Props/GDFCutoffs/SM_AirtowerDetail_01`.
   `.uasset` 1.722 Byte, `.uexp` 21.141 Byte, `.ubulk` 6.192 Byte; ebenfalls cooked.
   Materialreferenz in der Namenstabelle:
   `/Game/Tempest/GDF/Structures/BD_GDF_AirControlTower/Materials/MI_BD_GDF_AirControlTower`.
   Kleiner, aber optisch nur ein Ausschnitt eines Gebaeudes und daher schlechter
   als eigenstaendiges Abnahmeobjekt.
3. **Vegetations-Fallback:**
   `/Game/Tempest/Environments/Temperate/Foliage_New/Meshes/SM_Temperate_HeatherPlant_01`.
   Cooked; Namenstabelle nennt `.../Foliage_New/Materials/MI_Temperate_Heather_01`
   und `/Game/Tempest/DataAssets/FoliageConfigs/DA_FIC_Bush`.
   Vegetation benoetigt zusaetzliche Pruefung von Maskierung, zweiseitiger Darstellung
   und eventuell Wind. Fuer den ersten geometrischen Nachweis weniger geeignet.

## Warum der Container zuerst

Die Materialinstanz `MI_GDF_Container_01` ist **nicht cooked** und vom bestehenden
Parser einschliesslich Importtabelle lesbar. Ihre Importtabelle belegt:

- Parent-Material `/Game/Tempest/MaterialLibrary/Props/M_MASTER_Props`.
- Texturen unter `/Game/Tempest/Props/Faction_Containers/Textures/GDF/`:
  `T_GDF_Container_D`, `T_GDF_Container_N`, `T_GDF_Container_RMO`, `T_GDF_Container_C`.
- Zusaetzliche importierte Library-Textur `T_Grunge_01_M`.

`T_SmokeSoft_M` und `T_SnowMask_01_M` stehen ebenfalls in der Namenstabelle,
aber **nicht** in der dekodierten Importtabelle dieser Instanz. Sie sind deshalb
keine nachgewiesenen aktiven Materialabhaengigkeiten.

Die Endungen legen Albedo/Normal/Masken nahe; Kanalbelegung und tatsaechliche
Parameterzuordnung sind hier noch nicht dekodiert und duerfen nicht als verifiziert
bezeichnet werden. Ebenfalls noch unbekannt: reale Materialslotanzahl,
Vertex-/Dreieckszahlen, UVs, Abmessungen und sichtbare Originalqualitaet.
Das beantwortet der anschliessende CUE4Parse-Exportversuch und Bildvergleich.

## Reproduzierbare Evidenz

`Scripts/research/inspect_tempest_transfer_candidate.py` liest vier Paketrecords
(drei Meshkandidaten plus Container-Materialinstanz), validiert Namenstabellengrenzen
und hasht alle passenden Sidecars. Erfolgreicher Lauf:

```powershell
python Scripts/research/inspect_tempest_transfer_candidate.py `
  --content 'F:/Tempest-Rising-SteamRIP.com/Tempest Rising/MapEditorProject/Content' `
  --output Saved/Research/TempestMapTransfer/Transfer-Candidate.json
```

Ergebnis: `PASS: 4 package records`. Die JSON-Datei enthaelt beim ungecookten
Material auch die eigentlichen Importrecords und Byteoffsets. Bei cooked Meshes
wird nur der unterstuetzte Summaryprefix plus Namenstabelle gelesen; keine
Geometrie-/Exportdeserialisierung wird behauptet.
