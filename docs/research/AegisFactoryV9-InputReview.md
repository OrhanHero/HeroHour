# Aegis-Fabrik: neue Bildvorlage V9

Die gelieferte Fraktionszeichnung `Codex-Bild 1. Sept. 2026, 01_22_20.png`, die Gameplayzeichnung `01_22_00.png` sowie der tatsächliche bisherige Fabrikrender `Art/Models/MilitaryDetailV6/SM_MDV6_VehiclePlant-Preview.png` wurden geöffnet und verglichen. Für die neue Vorlage wurde der eingebaute Imagegen-Skill verwendet. Die drei Dateien wurden auch als Bildreferenzen an das Werkzeug übergeben.

Ergebnis: `Art/Concepts/GeneratedV9/AegisFactory-TrellisInput-v1.png`, 1536×1024 RGBA. Der exakte finale Prompt liegt daneben als `.prompt.txt`; `.json` enthält Ausgabe- und Referenz-SHAs sowie die Herkunft. Keine lokalen TRELLIS-, Unreal- oder GPU-Renderprozesse wurden für diese Aufgabe gestartet.

Das ausgegebene Bild wurde tatsächlich betrachtet. Die breite weiß-blaue Halle mit großem segmentiertem Fahrzeugtor, kurzem Rampenanschluss und niedrigerem Serviceflügel ist als Fahrzeugfabrik erkennbar. Große Dachlüfter, dicke schräg stehende Panzerstützen und abgenutzte Lackflächen verbessern Silhouette und Materialwirkung gegenüber dem alten dunklen, gleichförmigen Modell. Es besitzt weder die HQ-Schüssel noch einen Kommandoturm. Die Bauart übernimmt die architektonische Sprache der Aegis-Konzeptseite und bleibt eine eigenständige HeroHour-Vorlage.

Offen bleiben die verdeckte Rückseite, echte Topologie und sinnvolle Rekonstruktion der Lüfter. Das Bild enthält echte Transparenz (Alpha 0 bis 254, alle vier Ecken Alpha 0), allerdings auch einen weichen teiltransparenten Saum außerhalb der Silhouette. Vor einer Rekonstruktion muss deren Freistellmaske kontrolliert werden. Der Saum darf nicht als Gelände oder Gebäudeteil mitgeneriert werden. Die geschlossene Front vermeidet eine große schwarze Hallenhöhle, ist aber noch keine später animierbare Torbaugruppe.

**Nur Bildvorlage:** Es wurde kein fertiges 3D-Spielasset erstellt oder in HeroHour ersetzt. Ein nachfolgender Rekonstruktionsversuch benötigt dieselbe Prüfung von Vorderseite, Rückseite, Drahtgitter, Maßstab, Materialbindungen und tatsächlicher Unreal-Darstellung wie das HQ.
