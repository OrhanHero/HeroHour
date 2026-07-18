# Mitwirken an HERO HOUR

Willkommen im HERO HOUR Repository! Wir freuen uns, dass du an diesem RTS-Projekt mitwirken möchtest. Um die Konsistenz und technische Stabilität in unserer Unreal Engine 5.8 Umgebung zu gewährleisten, befolge bitte die folgenden Richtlinien.

## Entwicklungs-Richtlinien

- **Engine-Version:** Das Projekt basiert strikt auf **Unreal Engine 5.8**. Stelle sicher, dass deine lokale Installation diese Version nutzt.
- **Workflow:** Wir arbeiten mit einem Git-Flow-Modell. Erstelle für neue Features oder UI-Elemente bitte immer einen eigenen Branch.
- **UI & Assets:** 
    - UI-Elemente werden in UMG (Unreal Motion Graphics) implementiert. 
    - Orientierung für den visuellen Stil bietet die Dokumentation im Ordner `/docs/assets/`.
- **Epic Online Services (EOS):**
    - Alle EOS-spezifischen Funktionen müssen über das offizielle EOS-Plugin integriert werden.
    - Achte darauf, niemals `.ini`-Dateien mit sensiblen Daten (Client Secrets, Encryption Keys) zu committen.

## Repository-Struktur

- `/Source`: C++ Klassen für Spielmechanik und RTS-Logik.
- `/Content`: Unreal Engine Assets (Blueprints, UI-Widgets, Materialien).
- `/docs`: Projektdokumentation, inklusive der EOS-Integrationsanleitung.

## Pull Request Prozess

1. Beschreibe präzise, welches Feature oder welchen Fix dein PR beinhaltet.
2. Achte darauf, dass das Projekt nach dem Merge kompilierbar bleibt.
3. Markiere den PR mit dem entsprechenden Label (z.B. `feature`, `bugfix`, `ui-design`).

## Fragen?

Bei technischen Problemen oder Fragen zur Architektur kannst du dich jederzeit an die Organisation HERO unter @hero-c31fffa8 wenden.