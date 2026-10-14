# Tempest Rising: offenliegende Konfiguration

Untersucht am 14. Oktober 2026, ausschließlich lesend. Lokale Referenzwurzel: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject`.

Dieser Bericht beschreibt mitgelieferte Editor-Konfiguration und Plugin-Deskriptoren. Das sind statische Deklarationen. Weder das Verhalten der laufenden Spiel-EXE noch die Verwendung jeder Deklaration im aktuellen Spielbuild wurde damit bestätigt. Es wurden keine Spielassets kopiert und kein Spiel oder Editor gestartet.

## Projekt und verfügbare Quellen

- `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Tempest.uproject:3` nennt EngineAssociation 5.4; `README.txt:1–2` spezifiziert Unreal Engine 5.4.4 für den Karteneditor. Dies ist ein Beleg für dieses Editorprojekt, nicht automatisch für die genaue Version der separat ausgelieferten Spiel-EXE.
- `Tempest.uproject:8–15` deklariert die Module Tempest/Runtime und TempestEditor/Editor. Zeilen 27–32 aktivieren GameplayAbilities und GameplayCommands.
- In der Projektwurzel existiert kein `Source`-Verzeichnis. Die begrenzte Suche unter `Plugins` fand keine `.cpp`, `.h` oder `.Build.cs`; GameplayCommands und NavGrid enthalten unter anderem Binaries und Deskriptoren. Daher liegt hier kein offenlesbarer C++-Implementierungsbeleg vor.
- `Plugins\GameplayCommands\GameplayCommands.uplugin:19–21` deklariert ein Runtime-Modul mit PreDefault-Ladephase; Zeilen 26–27 aktivieren dessen GameplayAbilities-Abhängigkeit.
- `Plugins\NavGrid\NavGrid.uplugin:4` nennt VersionName 0.73, Zeilen 23–30 die Module NavGrid/Runtime und NavGridEditor/Editor. Aus einem Deskriptor folgen weder ein bestimmter Suchalgorithmus noch seine Laufzeitkosten.
- `README.txt:5–9` verweist auf die lokal vorhandenen Docs/beginner.pdf, manual.pdf, setdressing.pdf und api.pdf. Deren Inhalte wurden für diesen Config-Bericht nicht untersucht.

## Eingabe

Datei: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Config\DefaultInput.ini`.

| Zeilen | Deklaration | Belastbare Aussage |
|---|---|---|
| 75–76 | CapturePermanently_IncludingInitialMouseDown / LockOnCapture | Vorgesehene Standardkonfiguration für Mausaufnahme und -sperre. Kein Nachweis des tatsächlichen Menüzustands oder Verhaltens bei Fokusverlust. |
| 78 | DoubleClickTime=0.200000 | Konfigurierter Engine-Standard von 0,2 Sekunden. Keine belegte Funktion eines RTS-Doppelklicks. |
| 79–80 | EnhancedPlayerInput / EnhancedInputComponent | Enhanced Input ist als Standard vorgesehen. |
| 86–94 | IMC_CameraControl, Priority=0, bAddImmediately=True; DefaultMappingContexts aktiviert | Ein Kamera-Mapping-Context ist referenziert. Die konkreten WASD-, Maus- oder Zoombelegungen stehen hier nicht offen. |
| 88–92 | EnhancedInputUserSettings und KeyProfile-Klasse; bEnableUserSettings=False | Die Einstellungsklassen sind benannt, aber das Standardflag ist deaktiviert. Daraus folgt kein gesicherter Befund zur Neubelegung im Spielmenü. |

Die Datei enthält keine offenliegenden RTS-ActionMappings für Kontrollgruppen, Sammelpunkte oder Produktionsabbruch. Aussagen zu deren Tasten würden eine weitere Untersuchung der referenzierten Daten bzw. Laufzeitbeobachtung erfordern.

## Befehle und Fähigkeiten

Datei: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Config\Tags\CommandTags.ini`.

- Zeilen 49–51: Attack, AttackGround und AutoHarvest.
- Zeilen 71–72: Harvest und Harvest.NoTarget.
- Zeilen 74, 77, 79: HoldPosition, Move und Patrol.
- Zeile 93: SetRallyPoint.
- Zeilen 97–98: Stop und Stop.Blocked.
- Zeile 137: HarvesterRefinery.
- Zeilen 9–47: eigene Command.Cooldown-Namen, unter anderem Repair (35), Nuke (30), TempestMissile (46).
- Zeilen 101–126: SupportPower-Namen mit mehreren gesonderten Blocked-Varianten.

Datei: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Config\Tags\AbilityTags.ini`.

- Zeilen 5–6: Attack und Attack.Looping; Zeile 8: allgemeines Ability.Blocked.
- Zeilen 16–19: DeployStructure, DeployStructure.Blocked und DepositResource.
- Zeile 26: Enqueue; Zeile 34: Repair; Zeilen 39–40: SpawnUnit und SpawnUnit.Enlightened.
- Zeilen 54–60: Event-Namen für CustomBPEvent, DealDamage, DeployPawn, ShootProjectile, SpawnPawn und SpawnVFX.

Die getrennten Namensräume CommandType, Command.Cooldown, Ability und Event sind eine nützliche Architekturspur. Sie belegen keine konkrete Queue-Semantik, keinen Rückerstattungssatz, keine Cooldown-Dauer und keine verbindliche Zuordnung eines Commands zu einer Ability. Insbesondere ist Ability.Enqueue kein Beleg für eine bereits rekonstruierte Produktionswarteschlange.

`Config\DefaultGameplayTags.ini:2` aktiviert ImportTagsFromConfig; Zeile 7 setzt FastReplication=True. Dies dokumentiert die Tag-Konfiguration des Editorprojekts, keine abgeschlossene Prüfung des Netzwerkprotokolls.

## Produktions-, Energie- und UI-Merkmale

Datei: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Config\Tags\PlayerDataTags.ini`.

Die folgenden Entwicklerkommentare sind aussagekräftiger als reine Namen, bleiben aber deklarierte Absicht statt ausgeführtem Programmbeweis:

| Zeile | Tag | Paraphrase des Entwicklerkommentars |
|---|---|---|
| 155 | AllowInsufficientPower | Gebäude dürfen einen Zustand unzureichender Stromversorgung annehmen. |
| 156 | AllowPowerDown | Der Spieler darf das Gebäude abschalten. |
| 158 | BuildOnlyInFactoryRadius | Eine so markierte Fabrik darf andere Gebäude nur in ihrem bereitgestellten Baubereich bauen. |
| 159 | InsufficientPowerSlowDown | Bei Energiemangel bleibt das Gebäude funktionsfähig und wird verlangsamt. |
| 160 | NoConstruction | Laufende Konstruktion wird angehalten und neue Konstruktion kann nicht eingereiht werden. |
| 164–165 | NoRepair / NoSell | Reparatur bzw. Verkauf durch den Spieler ist untersagt. |
| 167 | SimultaneousConstruction | In einer markierten Fabrik können alle Produktionen gleichzeitig fortschreiten. |
| 168 | Worker | So markierte Pawns sollen im Workers Widget angezeigt werden. |
| 203 | FactoryType.PawnSpecific.Refinery | Eine spezifische Bauwarteschlange der Dynasty-Raffinerie wird benannt. |
| 208 | PowerDown | Wird laut Kommentar bei manuellem Abschalten oder Energiemangel angewendet. |

Zeilen 197–204 klassifizieren FactoryType u. a. als Buildings, Defenses, Infantry, Vehicles und AerialAndNaval. Zeilen 205–207 deklarieren Minimap.ForceOff, ForceOn und IgnoreRequirements. Keine dieser Zeilen liefert Kosten, Produktionszeiten, Energiewerte oder die vollständige UI-Auswertung.

## Navigation und Bauplatz

Datei: `F:\Tempest-Rising-SteamRIP.com\Tempest Rising\MapEditorProject\Config\DefaultNavGridSettings.ini`.

- Zeilen 2–5 benennen Oberflächen NoStructures, FoliageObstacle, TempestField und TempestFieldCrater.
- Zeile 6 ordnet Oberflächentypen NavArea-Klassen zu. SurfaceType5 enthält sowohl TedNavArea_NoStructures als auch TedNavArea_NoGroundUnits. SurfaceType_Unwalkable enthält NavArea_Obstacle und TedNavArea_VetiDome. Damit ist eine semantische Kennzeichnung von Hindernis- und Platzierungsflächen sichtbar; wie einzelne Agenten sie auswerten, ist nicht gezeigt.
- Zeilen 23–26 deklarieren Visualisierungen für Default, Verge, LowCover, HighCover und InfiniteCover. Zeile 25 enthält Darstellungs-WallHeights; diese sind hier keine belegten physikalischen Deckungshöhen oder Schadensmodifikatoren.
- Zeilen 29–34 deklarieren Editor-Sprites und Farben für Tags/Oberflächen. Die Datei liefert keine Zellgröße, kein A*-Verfahren, keine Crowd-Vermeidung und keine Pfadsuchbudgets.

## Vorschläge für HeroHour nach Abschluss der Untersuchung

Dies sind eigene Vorschläge, keine bereits ausgeführten Codeänderungen und keine Pflicht zur Übernahme des kompletten Systems:

1. **Befehlsverfügbarkeit vereinheitlichen:** Ein eigener Command-Typ mit einer gemeinsamen Abfrage für verfügbar, blockiert samt Grund und Cooldown. HUD und RMB-Auflösung verwenden dieselbe Abfrage. Verhindert aktiv wirkende Schaltflächen, die keine gültige Aktion auslösen.
2. **Sammler explizit steuerbar machen:** AutoHarvest und gezieltes Harvest als getrennte Benutzerabsichten; Ausgabe von Kein Ziel, Depot blockiert und manueller Auftrag. Ergänzt den bereits korrigierten manuellen Bewegungsbefehl ohne neue Fraktionssysteme.
3. **Sammelpunkte und Produktion priorisieren:** Produktionsgebäude erhalten einen sichtbaren Sammelpunkt und später Warteschlangenabbruch. Rückerstattung und Abbruchregeln werden für HeroHour selbst festgelegt; diese INIs beweisen keine Tempest-Regel dazu.
4. **Bauplatz vom Laufweg unterscheiden:** Eigene Rasterflags Walkable und Buildable statt ausschließlich einer Hindernisprüfung. Brücken können z. B. befahrbar und gleichzeitig unbebaubar sein. Mit einem belegbaren Grenzfall testen.
5. **Arbeiterzugriff im HUD:** Anzeige aller Sammler sowie eines Leerlaufzählers mit Auswahlfunktion. Der Worker-Tag-Kommentar gibt eine mögliche UI-Struktur vor, aber nicht deren aktuelle Darstellung.
6. **Energie erst nach dem Kernablauf:** Wenn Energie eingeführt wird, einzelne Gebäudezustände Running, PoweredDown und LowPower mit sichtbarer Produktionsauswirkung. Simultane Produktion, Naval/Air-Fabriken und zahlreiche Spezialfähigkeiten bleiben außerhalb des ersten Prototyps.

## Identität der untersuchten Konfiguration

SHA-256, erhoben am 14. Oktober 2026. Originaldateien wurden nicht verändert.

| Datei relativ zur Referenzwurzel | SHA-256 |
|---|---|
| Config/DefaultInput.ini | `5fdc46db1e8f878a2cc7e1e1a0f291074dcb764e08cee2111b2bf655009134c3` |
| Config/Tags/CommandTags.ini | `a8624656fd57d1889521f311fd1aab2cb9498f830819a06a1a77fb3937512211` |
| Config/Tags/PlayerDataTags.ini | `f7997e9d768c7240c3ce8417358659dc18524e8393dac3eabae63b08e068c394` |
| Config/Tags/AbilityTags.ini | `e7f193caffe21d47138876b2bc367161e0d8b174d2a073a5892503463287ce7e` |
| Config/DefaultNavGridSettings.ini | `4e062cb7f45a26fb19f78e392da7b5be27509b46843dbf44aea6b2e9d11535fc` |
