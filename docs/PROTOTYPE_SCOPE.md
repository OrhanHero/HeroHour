# HeroHour Prototype Scope

The first playable is a deliberately small 1v1 vertical slice. Its purpose is
to prove that selecting, producing, moving, and fighting with units is fun.

## Included

- One compact 1v1 map: **Crossroads**
- Two factions: **HERO** and **USA**
- One shared resource: **Credits**
- One infantry, one vehicle, and one air unit per faction
- Unit selection, movement, combat, production, and death
- One headquarters and one production building per faction
- Victory by destroying the opposing headquarters
- A simple AI that produces units and attacks

## Explicitly deferred

- Additional factions and unit roles
- Super-agents and superweapons
- Campaign, map editor, weather, urban warfare, and live service
- Ranked play, matchmaking, replays, and advanced networking
- Technology trees, veterancy, multiple resources, and complex abilities

## Exit criteria

The prototype is successful when a player can complete a stable 10–15 minute
match against the AI and the core decisions around production, counters, and
positioning are enjoyable.

## Running the scope check

```powershell
dotnet run --project Source/HeroHourPrototype.Tests/HeroHourPrototype.Tests.fsproj
```
