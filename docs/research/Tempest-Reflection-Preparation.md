# Reflection: isolierte Importvorbereitung

Stand 17. Oktober 2026. Repository https://github.com/JsonAsAsset/Reflection
liegt ausschliesslich unter `Saved/Tools/Reflection`, Commit
`f5413f656f1f45f1384ea9eb4bb1f1fc37546369` (shallow clone, sauberer Arbeitsbaum).
Version im Pluginmanifest: 1.5.1, Editor-only. Kein Plugin in HeroHour installiert,
kein Editor gestartet, keine Projekteinstellungen oder Tempest-Dateien geaendert.

## Kompatibilitaet: belegt versus noch zu testen

`Reflection.uplugin` schreibt keine EngineVersion vor. `Compatibility.h` besitzt
explizite Zweige fuer UE5.4 und UE5.8; `Reflection.Build.cs` beruecksichtigt sogar
eine UE5.8-RigLogic-API-Aenderung. Damit ist Unterstuetzung im Quellcode vorgesehen,
aber ein erfolgreicher Build unter unseren konkreten 5.4.4/5.8.3 ist **nicht**
nachgewiesen. UE5 verwendet C++20. Der Clone beinhaltet Detex/NVTT-Quellmodule;
kein zusaetzliches Downloadpaket ist in deren Build.cs verlangt.

Wichtige Buildabhaengigkeiten sind UnrealEd, MaterialEditor, Json/JsonUtilities,
HTTP, Niagara/NiagaraEditor, UMGEditor, Landscape/Foliage, AssetTools und weitere
Editormodule. ControlRig, RigLogic und CurveExpression werden versions- und
headerabhaengig erkannt. Dies ist kein schlanker Laufzeitimporter.

## Container-Material

Der registrierte `MaterialInstanceConstantImporter` unterstuetzt explizit
MaterialInstanceConstant, deserialisiert Objektproperties und damit Parent sowie
ScalarParameterValues, TextureParameterValues und VectorParameterValues. Hinzu
kommen StaticSwitch-/ComponentMask-Parameter und Nanite-Override-Behandlung.

Zielpfad: `/Game/Tempest/Props/Faction_Containers/Materials/MI_GDF_Container_01`.
Die echte Importtabelle belegt Parent `M_MASTER_Props` und fuenf Texturen, siehe
`Tempest-Transfer-Candidate.md`. Fuer einen belastbaren Import benoetigen wir:

1. CUE4Parse-kompatible Export-JSON mit Typ, Objektname, Properties und Referenzen.
2. Den vollstaendigen Expression-/Connection-Export von
   `/Game/Tempest/MaterialLibrary/Props/M_MASTER_Props` samt transitiven
   MaterialFunctions/Collections/Textures. Deren Umfang ist noch nicht vollstaendig
   erfasst; Instanzimport allein rekonstruiert den Parentgraph nicht.
3. Dekodierte Pixel oder einen erreichbaren Cloud-Server fuer die Texturen.
4. Identische virtuelle Paketpfade in einem isolierten Testprojekt, alternativ
   gezielt pruefbare Redirectors.

`MaterialImporter.cpp` lehnt Null-Expressions ab. Bei fehlendem Materialgraph
meldet es einen Fehler, sofern optionale Stubs nicht aktiviert sind. Ein erzeugter
Stub waere kein originalgetreues Material und darf keinen erfolgreichen Transfer
vortaeuschen. Der ModKit-Parent ist ungecooked: gute Voraussetzung, noch kein
Beweis eines vollstaendigen Expressionexports.

## Cloud und lokale JSON

Der Clone enthaelt den Unreal-Client, **nicht** den Server. Upstream verweist auf
https://github.com/JsonAsAsset/Core, insbesondere `Source/Core.Cloud`, bzw. dessen
Releases. Dieses weitere Repository wurde hier nicht heruntergeladen.

Clientstandard ist `http://localhost:1500`; Statusabfrage `/api/status`.
Export, Textur- und Meshdaten werden ueber weitere Client-Endpunkte angefragt.
Ein Cloud-Dienst wurde weder gestartet noch seine Erreichbarkeit behauptet.

Lokaler JSON-Weg existiert: `UReflectionSettings::UseLocalJson`; im Defaultbuild
steht `REFLECTION_CLOUD_SERVER=1`, wodurch der lokale Einstieg nicht als editierbare
Option exponiert ist. Fuer einen offline orientierten isolierten Pluginbuild
kann die lokale Clone-Kopie in `Reflection.Build.cs` auf `bCloudServer=false`
gestellt werden. Das aendert den Dateidialog-Einstieg, entfernt den Cloudcode aber
nicht; fehlende Referenzen werden dadurch nicht automatisch geloest. Bisher wurde
auch diese lokale Anpassung **nicht** vorgenommen.

## Naechster exakt abgegrenzter Test

Zuerst Root-CUE4Parse-Export auf Instanz und Parent anwenden und die Graphdaten
pruefen. Danach Plugin isoliert bauen, z.B. fuer die installierte UE5.8:

```powershell
& 'F:/UE/UE_5.8/Engine/Build/BatchFiles/RunUAT.bat' BuildPlugin `
  '-Plugin=C:/Users/Hero/HeroHour/Saved/Tools/Reflection/Reflection.uplugin' `
  '-Package=C:/Users/Hero/HeroHour/Saved/Tools/Reflection-Build-UE58' `
  -TargetPlatforms=Win64
```

Dieser Befehl ist vorbereitet, **nicht ausgefuehrt**. Ein anschliessender Import
gehoert in ein separates Testprojekt; erst Parentgraph, dann Texturen/Instanz,
danach Meshzuweisung und Bildvergleich. Originalassets, Exportbilder und geklonte
Fremdrepositories bleiben ausserhalb des HeroHour-Git-Contents.
