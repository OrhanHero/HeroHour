# REA als Referenzanalyse für HeroHour

Stand: 14. Oktober 2026. Reihenfolge des Nutzers: zuerst C&C Generals / Zero Hour, anschließend Tempest Rising.

## Zweck und Ergebnisformat

Jede Untersuchung beantwortet eine konkrete Entwicklungsfrage. Ein Ergebnis enthält die genaue Spiel-/Artefaktidentität, Belege, Grenzen, einen eigenen HeroHour-Entwurf und überprüfbare Abnahmekriterien. Quellenbefund, installierte Konfiguration, native Binäranalyse und tatsächliche Spielbeobachtung werden getrennt ausgewiesen. Eine gefundene Zeichenkette allein belegt noch keinen ausgeführten Codepfad.

## Eingerichtete Werkzeuge

| Werkzeug | Verifizierter Stand |
| --- | --- |
| REA | npm-Paket `rea-agents@6.4.0`, offizieller Ursprung `morluto/rea` |
| Laufzeit | Node 26.7.0 |
| Native Analyse | Ghidra 12.1.4, Windows x64, read-only PE-Unterstützung |
| Java | vollständiges Temurin JDK 21.0.12.1, x64 |
| Codex | REA-MCP-Eintrag und passender gebündelter Skill installiert |

Die portablen Werkzeuge liegen unter `Saved/Tools`. REA-Doctor hat den Ghidra-Provider einschließlich JDK, NTFS-/Job-Object-/DACL-Voraussetzungen als verfügbar geprüft. Die Codex-Konfiguration wurde mit einer Sicherung unter `C:\Users\Hero\.codex\config.toml.rea.backup` ergänzt. Die Providerpfade sind im REA-MCP-Eintrag enthalten. Neue MCP-Einträge werden nach Neustart des Codex-Clients in dessen Werkzeugsitzung geladen; die CLI funktioniert bereits unabhängig davon.

Das Ghidra-Archiv wurde gegen den SHA256-Digest der offiziellen GitHub-Veröffentlichung geprüft:
`ddac49f903da9d5bac833e5cc79395098b9c33cfd3279be5f31bd00387d2d4db`.
Der JDK-Digest aus der offiziellen Adoptium-Metadatenabfrage:
`f9d6e191ab098c0d416e7d588a24420a8621cd2f4720dab2459b8b7b2d2d8b4e`.

## Zero-Hour-Ziele

Vom Nutzer ausgewählt:
`C:\Program Files\EA Games\Command and Conquer Generals Zero Hour\Command and Conquer Generals Zero Hour\Generals.exe`.

Zusätzlich identifizierte native PE-Datei im selben Spielordner: `game.dat`. Die EXE und DAT wurden getrennt mit REA erfasst; die Dateiendung allein entscheidet nicht über das Analyseverfahren.

Offizielle historische Quellreferenz: [electronicarts/CnC_Generals_Zero_Hour](https://github.com/electronicarts/CnC_Generals_Zero_Hour), Commit `0a05454d8574207440a5fb15241b98ad0b435590`. Sie liegt unter `Saved/Research/ZeroHourSource`. Die Veröffentlichung ist eine Referenz; Übereinstimmung mit jeder Funktion der installierten Binärdatei ist dadurch nicht festgestellt.

## Reproduzierbare Befehle

```powershell
& .\Scripts\research\Invoke-REA.ps1 -ReaArguments @('doctor','--provider','ghidra','--json')
& .\Scripts\research\Invoke-REA.ps1 -ReaArguments @('analyze','C:\Program Files\EA Games\Command and Conquer Generals Zero Hour\Command and Conquer Generals Zero Hour\game.dat','--provider','ghidra','--json')
```

Der Wrapper liest die lokalen Providerpfade aus `Saved/Tools/rea-environment.json` und stellt veränderte Prozess-Umgebungswerte nach dem Aufruf wieder her. Beim Einrichtungstransfer auf einen anderen Rechner werden REA sowie Ghidra/JDK dort neu eingerichtet; der Projektcache ist kein Git-Bestandteil.

Die MCP-Verbindung wurde separat über das mit REA installierte offizielle MCP-SDK geprüft: 170 Werkzeuge wurden aufgelistet und Zielöffnung, Stringsuche, Referenzsuche sowie Funktionsanalyse am tatsächlichen Spielartefakt erfolgreich ausgeführt. Die lokale Testhilfe und vollständige JSON-Evidence liegen unter `Saved/Research`; für die reguläre Nutzung dienen die eingerichtete Codex-MCP-Anbindung und der CLI-Wrapper. Werkzeugaufrufe werden anhand ihrer tatsächlichen Schemas und Verfügbarkeit gewählt. Native Erstanalysen großer Dateien können mehrere Minuten benötigen.

Beispiel einer gezielten CLI-Abfrage nach der Übersicht:

```powershell
& .\Scripts\research\Invoke-REA.ps1 -ReaArguments @('search','C:\Program Files\EA Games\Command and Conquer Generals Zero Hour\Command and Conquer Generals Zero Hour\game.dat','SupplyTruckAIUpdate','--provider','ghidra','--json')
```

Ergänzende Konfigurationsprüfung:

```powershell
python .\Scripts\research\inspect_zero_hour_balance.py --game-dir 'C:\Program Files\EA Games\Command and Conquer Generals Zero Hour\Command and Conquer Generals Zero Hour' --output Saved/Research/ZeroHour/installed-balance-facts.json
```

Dieser eigene Parser liest begrenzte BIG-Verzeichnisse und ausgewählte numerische INI-Felder. Er ist ergänzende Forschung und keine behauptete REA-BIG-Funktion. Patchreihenfolge, Objektvererbung und Laufzeitmodifikatoren werden dabei nicht aufgelöst.

## Untersuchungsfolge

1. **Sammler:** manuelle Befehle, Zustände, Zielausfall, Wartesituationen, Ladung und Auszahlung.
2. **Produktion:** gebäudespezifische Queue, Abbruch/Erstattung, Produktionsausgang und Sammelpunkt.
3. **Steuerung:** kontextbezogene Befehle, Kontrollgruppen, Auswahl und Kamerafokus.
4. **Bewegung:** Formationen, gegenseitige Blockierung, Brücken und Planungsbudget.
5. **Darstellung:** reale Spielszenen mit den bereitgestellten Konzepten und HeroHour-Screenshots vergleichen.
6. **Tempest Rising:** lokale Untersuchung der vom Nutzer ausgewählten Installation; [native Zielprüfung und Grenzen](TempestRising-REA-FirstPass.md), [Konfiguration](TempestRising-ConfigReview.md) und [Karteneditor/API](TempestRising-MapEditorReview.md).

Erster umgesetzter HeroHour-Schritt: manuelle Sammlerbefehle haben Vorrang; nach Zielankunft setzt sich die Logistik fort. Wasser- oder blockierte Ziele beschädigen den bisherigen Auftrag nicht. Umsetzung und Tests: [Mechanikbericht](ZeroHour-Mechanics-SourceReview.md). Weitere Bedienaufgaben: [Steuerungsbericht](ZeroHour-Controls-SourceReview.md).
