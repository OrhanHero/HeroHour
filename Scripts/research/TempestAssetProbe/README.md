# Targeted Tempest asset probe

This is an independent .NET 10 command-line research harness. It never launches the game, injects code or modifies source assets. Exports require a fresh directory outside the source installation. Geometry and decoded game textures belong under `HeroHour/Saved`, not the repository's production Content directory.

Verified CUE4Parse source commit: `605672df7a993a90094bad92d30c388d87ae6c1a`.

```powershell
git clone --recursive https://github.com/FabianFG/CUE4Parse.git Saved/Tools/CUE4Parse
git -C Saved/Tools/CUE4Parse checkout 605672df7a993a90094bad92d30c388d87ae6c1a
git -C Saved/Tools/CUE4Parse submodule update --init --recursive
dotnet build Scripts/research/TempestAssetProbe/TempestAssetProbe.csproj -c Release -p:CUE4PARSE_SKIP_NATIVE=true
```

The optional `CUE4ParseSource` MSBuild property overrides the default checkout path. Native ACL animation helpers are skipped for this bounded static-mesh/texture test; animation support is not established.

Arguments: `projectRoot objectPath freshOutput [export|inspect] [reflectionMappingDirectory]`. The mapping directory comes from the matching UE5.4 `TempestMappingsProbe` commandlet and contains `structs.json` plus `enums-numeric.json`. It is not an invented or downloaded schema.

Successful tested object: `/Game/Tempest/Props/Faction_Containers/SM_GDF_Container.SM_GDF_Container`. The first unmapped run failed explicitly. The mapped run exported one GLB, one material description and five PNG textures. A separate UE5.8 test imported the GLB and four container textures, preserving dimensions and material-slot count.

The GLB contains geometry/UVs and a named material slot, not an embedded reconstruction of the full Tempest shader. Full details and failure history: `docs/research/Tempest-Container-Transfer-2026-10-17.md`.
