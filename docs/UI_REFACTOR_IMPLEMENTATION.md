# HeroHour: konkrete UI-Weiterentwicklung

Verbindliche Nutzerentscheidung: **Radar links unten, Befehle/Baukarten mittig, Auswahl rechts unten, Ressourcen darüber.** Die anders angeordneten Bereiche der eingefügten Vorlage werden nicht übernommen.

## Tatsächlich vorhandener Code

- `Source/HeroHourPrototype/HeroHUD.cpp`: `APrototypeHUD::DrawHUD()` zeichnet derzeit per Canvas; `HandleClick()` verarbeitet die vorhandenen HUD-Aktionen.
- `Source/HeroHourPrototype/PrototypeHUDLayout.h`: gemeinsame Geometrie für Darstellung und Klickprüfung, bereits mit Auflösungsskalierung und Mindestschriftgröße.
- `Source/HeroHourPrototype/HUD/PrototypeHUDChrome.cpp`: eigene Stahlrahmen und Tabdarstellung.
- `Source/HeroHourPrototype/PrototypeGameMode.h`: HUD-Deklaration und Spiel-API.
- `Source/HeroHourPrototype/HeroHourPrototype.Build.cs`: bisher keine UMG-/Slate-Abhängigkeiten.

Es gibt damit kein bestehendes WBP-Hauptwidget, dessen Hierarchie nur umgeordnet werden müsste. BackgroundBlur, BindWidget und WidgetAnimation lassen sich nicht direkt in Canvas-Aufrufe einstecken. Eine UMG-Umstellung ist eine eigene, schrittweise Migration. Die bestehenden 917 Paketprüfungen sind der Ausgangsstand, keine Abnahme der hier beschriebenen Migration.

## Warum der Look noch abweicht

Die funktionale Anordnung ist inzwischen umgesetzt. Der verbleibende Unterschied betrifft Materialwirkung und Gestaltung: flache Flächen, zu wenig gezielt ausgearbeitete Rahmen-/Trennkanten, uneinheitliche Bildsprache der Porträts, Standardschrift und eine noch deutlich abweichende Spielwelt. Mehr Neon oder Blur allein behebt diese Unterschiede nicht. Für die nächste Gestaltung werden Rahmen, Schrift, Icons und Zustände zunächst gemeinsam an einer einzigen Befehlskarte abgestimmt.

## Umsetzungsreihenfolge

1. **Eine gemeinsame Darstellungsspezifikation erstellen.** Dunkle Panelbasis, cyanfarbene aktive Kanten, orange Warnungen, 8 logische Pixel Eckabschrägung, 1–2 Pixel Kontur. Ausgewählt, Hover, deaktiviert, Baufortschritt und Cooldown müssen sich auch ohne Farbe unterscheiden. Fraktions- und Warnfarben nicht pauschal durch Cyan ersetzen.
2. **UMG als optionale zweite Darstellung anbinden.** `UHeroHUDRoot` einmal im lokalen Controller/HUD erzeugen. Nur im neuen Modus Canvas-Bedienpanels und deren Mausweiterleitung abschalten; Weltmarkierungen und Lebensbalken zunächst beibehalten. Zwei gleichzeitig aktive Eingabesysteme würden doppelte Käufe auslösen. Der geprüfte Canvas-Modus bleibt bis zur Abnahme verfügbar.
3. **Befehlskarte zuerst migrieren.** `WBP_CommandButton` aus einer C++-Basisklasse ableiten. Bestehende Befehlsdaten und Verfügbarkeitsprüfung zuführen. OnClicked ruft dieselbe semantische Befehlsfunktion wie bisher auf, keine neue Wirtschaft implementieren. Wenn dafür die Verzweigung aus `HandleClick()` in eine Hilfsfunktion extrahiert werden muss, Eingangsprüfung und Verhalten unverändert erhalten.
4. **Produktionspanel übernehmen.** Kategorien, fünf Spalten, echte Producer-Queue, Fortschritt, Kosten, Abbruch und Cooldown anbinden. Der Presenter liest Spielzustand; ein Widget speichert keine eigene Geldsumme oder Produktionswarteschlange.
5. **Radar, Auswahl und Ressourcen übernehmen.** Radarbewegung, Zoom, Low-Power-Gating, Mehrfachauswahl und aktive Fähigkeiten erhalten. Tooltips und Pause-/Endbildschirme müssen Mausaktionen zum Schlachtfeld blockieren.
6. **Darstellung abnehmen.** 1280×720, 1920×1080, 2560×1440 und 3840×2160 tatsächlich rendern und prüfen. Befehlskarten, Mehrfachauswahl, lange Texte, Low Power, Tooltip, Pause und Sieg abdecken. Die vorhandenen UI-Interaktionstests auf das echte neue Widget-Routing umstellen, nicht lediglich alte Canvas-Koordinaten testen. Danach Paketregression.

## Zielhierarchie

```text
WBP_HeroHUDRoot / CanvasPanel
  BottomCommandStrip / horizontaler Container, unten gestreckt
    WBP_Minimap             feste logische Breite
    WBP_CommandPanel        füllt verbleibende Breite
      Overlay
        BackgroundBlur      optional, nur dieser Bereich
        Image               parametrierbarer dunkler Rahmen
        VerticalBox
          Kategorien
          HorizontalBox
            UniformGridPanel / WBP_CommandButton
            WBP_ProductionQueue
    WBP_SelectionPanel      feste logische Breite
  WBP_ResourceStrip         über CommandPanel ausgerichtet
  WBP_TooltipLayer
  WBP_ModalLayer            blockiert darunterliegende Eingabe
```

Die Minimap wird nicht automatisch quadratisch umgebaut; das vorhandene leicht rechteckige Zero-Hour-Feld bleibt die Grundlage. Eine gemeinsame DPI-Regel genügt. Keine zweite unkontrollierte Skalierung aus UMG-DPI, bestehendem `L.S` und ScaleBox multiplizieren. Buttons erhalten feste **logische** Größen und skalieren konsistent mit der UI; physisch gleich kleine 80-Pixel-Buttons auf einem 4K-Bild wären keine brauchbare Umsetzung. Text und Klickgeometrie folgen derselben Skalierung. Hover vergrößert nur einen inneren Bildcontainer, nicht den klickbaren Außenrahmen.

## C++-Beispiel: wiederverwendbare Befehlskarte

**Implementierungsvorlage, noch nicht als Runtime-Code eingebaut oder kompiliert.** Das zugehörige Widget-Blueprint benötigt exakt die benannten Controls. Kostenprüfung und Aktion werden außerhalb der Karte an den bestehenden Befehlsweg gebunden.

Build.cs bei tatsächlicher Einführung ergänzen:

```csharp
PublicDependencyModuleNames.Add("UMG");
PrivateDependencyModuleNames.AddRange(new[] { "Slate", "SlateCore" });
```

Header `HeroCommandButton.h`:

```cpp
#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HeroCommandButton.generated.h"

class UButton;
class UImage;
class UMaterialInstanceDynamic;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHeroCommandRequested, FName, CommandId);

UCLASS(Abstract)
class HEROHOURPROTOTYPE_API UHeroCommandButton : public UUserWidget
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Command")
    FName CommandId;

    UPROPERTY(BlueprintAssignable, Category="Command")
    FHeroCommandRequested OnCommandRequested;

protected:
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> CommandButton;
    UPROPERTY(meta=(BindWidget)) TObjectPtr<UImage> FrameImage;
    UPROPERTY(Transient) TObjectPtr<UMaterialInstanceDynamic> FrameMID;

    virtual void NativeOnInitialized() override;
    UFUNCTION() void RequestCommand();
    UFUNCTION() void Hover();
    UFUNCTION() void Unhover();
};
```

Source `HeroCommandButton.cpp`:

```cpp
#include "HeroCommandButton.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Materials/MaterialInstanceDynamic.h"

void UHeroCommandButton::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    CommandButton->OnClicked.AddUniqueDynamic(this, &ThisClass::RequestCommand);
    CommandButton->OnHovered.AddUniqueDynamic(this, &ThisClass::Hover);
    CommandButton->OnUnhovered.AddUniqueDynamic(this, &ThisClass::Unhover);
    // FrameImage brush must already reference the UI material.
    FrameMID = FrameImage->GetDynamicMaterial();
}

void UHeroCommandButton::RequestCommand()
{
    OnCommandRequested.Broadcast(CommandId);
    // The receiving command path must still revalidate authoritative state.
}

void UHeroCommandButton::Hover()
{
    if (FrameMID) FrameMID->SetScalarParameterValue(TEXT("HoverAmount"), 1.f);
}

void UHeroCommandButton::Unhover()
{
    if (FrameMID) FrameMID->SetScalarParameterValue(TEXT("HoverAmount"), 0.f);
}
```

Das ist die visuelle Kartenhülle. Produktionsabbruch, Zielauswahl und doctrines bleiben separate vorhandene Befehle; `CommandId` allein ist kein Ersatz für Rollen-/Producer-/Zielparameter. Vor echtem Anschluss deren vorhandene Typen explizit im Befehlsmodell abbilden. Hover kann anschließend über eine kurze 0,1-Sekunden-Widgetanimation interpoliert werden. Selected bleibt ein eigener Zustand und darf beim Unhover nicht gelöscht werden.

## UI-Material mit abgeschrägten Ecken

`M_UI_ChamferPanel`: Material Domain **User Interface**, Blend Mode **Translucent**. Ausgabe über **Final Color** und **Opacity**. Das normale Bildschirm-UMG erhält keinen verlässlichen Welt-Bloom nur durch hohe Farbwerte. Einen kontrollierten Halo als weich auslaufende Kontur im UI-Material erzeugen; dafür Platz innerhalb des Brush-Randes reservieren, damit er nicht abgeschnitten wird.

Parameter: `PanelSize` in logischen Pixeln, `ChamferPx=8`, `BorderPx=1.5`, `GlowWidthPx=3`, `PanelOpacity=0.80`, `AccentColor`, `AlertColor`, `HoverAmount`, `SelectedAmount`, `DisabledAmount`. Größe bei Geometrieänderung aktualisieren, nicht einmalig eine feste 16:9-UV-Maske auf alle Panels strecken.

Materialgraph-Rezept:

1. `TextureCoordinate` auf Panelpixel umrechnen und um die Mitte zentrieren.
2. Rechteckgrenze und vier diagonale Eckgrenzen kombinieren; das ergibt eine achteckige Maske mit in Pixeln kontrollierter Abschrägung.
3. Zweite, nach innen versetzte Maske von der äußeren Maske abziehen: scharfe Kontur.
4. Eine weich auslaufende erweiterte Maske erzeugt den Halo; aktive Zustände erhöhen diesen Anteil moderat. Deaktivierte Karten reduzieren Kontrast und erhalten ein eigenes Sperrsymbol.
5. Dunkle Füllfarbe, Kontur und Halo zusammensetzen; Opacity unabhängig für Füllung und Rand berechnen. Text bleibt als separater TextBlock scharf.

BackgroundBlur mit Stärke 2–3 nur hinter dem mittleren Hauptpanel beginnen. Eine dunkle Low-Quality-Fallback-Brush und einen abschaltbaren Qualitätsmodus vorsehen. Das Material beschränkt nicht automatisch den Blur auf eine abgeschrägte Form; Blur zunächst etwas innerhalb des opaken Rahmens halten, damit keine rechteckigen Ecken herausragen. GPU-Zeit tatsächlich messen, bevor zusätzliche Blur-Flächen aktiviert werden.

Schrift: eine lizenzgeprüfte Fontdatei importieren, UMG-Fontasset anlegen und Überschriften/Fließtext konsistent definieren. Rajdhani/Orbitron/Eurostile sind Vorschläge, keine bereits im Projekt eingerichteten Fonts. Breite Buchstabenabstände und ausschließlich Großbuchstaben nicht auf längere Tooltiptexte anwenden.

## Technische Referenzen

- Epic BackgroundBlur: https://dev.epicgames.com/documentation/en-us/unreal-engine/using-the-background-blur-widget-in-unreal-engine
- Lokal geprüfte UE5.8-APIs: `UMG/Public/Components/BackgroundBlur.h` (`SetBlurStrength`, `SetLowQualityFallbackBrush`), `UMG/Public/Components/Image.h` (`GetDynamicMaterial`).

**Stand dieses Dokuments:** Analyse, Zielhierarchie und Implementierungsvorlagen. Es wurde hierdurch keine UMG-Migration vorgetäuscht und kein neuer UMG-Build-/Performance-Erfolg behauptet. Der bestehende spielbare Canvas-HUD bleibt funktionsfähig.
