# HeroHour

HeroHour is a deliberately small RTS prototype built with Unreal Engine 5.8
and a deterministic F# domain core. The current goal is to prove one enjoyable
10–15 minute match before expanding the design.

## Current playable scope

- One runtime-generated 1v1 greybox arena
- Two factions: **HERO** and **USA**
- Infantry, vehicle, and air unit for each faction
- RTS camera with WASD and mouse-wheel zoom
- Left-click selection and right-click move/attack orders
- Unit health, damage, range, attack cooldown, and destruction
- HERO and USA headquarters with a destroy-the-HQ victory condition
- A simple USA opponent that attacks after a preparation period
- `R` restarts the match

No final art is required: the entire match is generated from C++ and engine
primitives. See [Prototype Scope](docs/PROTOTYPE_SCOPE.md) for the boundaries.

## Requirements

- Unreal Engine 5.8
- Visual Studio with **Game development with C++**
- MSVC x64 toolchain and Windows 11 SDK
- .NET SDK 8.0.423 or a compatible 8.0 patch

## Build and run

```powershell
./Scripts/Build-Prototype.ps1
./Scripts/Run-Prototype.ps1
```

Run the deterministic content/scope check with:

```powershell
dotnet run --project Source/HeroHourPrototype.Tests/HeroHourPrototype.Tests.fsproj
```

## Repository policy

HeroHour does **not** use Git LFS. Large and generated assets remain local.
`Art/`, `Content/`, Unreal `.uasset`/`.umap` files, videos, source art, builds,
intermediate files, and caches are excluded by `.gitignore`. Git tracks source
code, lightweight configuration, scripts, tests, and documentation only.

## Deferred until the core is fun

Additional factions, superweapons, super-agents, campaign, map editor, online
services, matchmaking, advanced AI, weather, live service, and final art are
roadmap ideas—not implemented prototype features.

## License

[MIT](LICENSE)
