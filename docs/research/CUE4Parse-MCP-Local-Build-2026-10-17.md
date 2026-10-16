# CUE4Parse MCP: isolated local build and actual stdio probe

Source: https://github.com/f14XuanLv/CUE4Parse-MCP

Commit: `f0504f3653a084664b02561cee93d62ff9600463`.

Local checkout: `Saved/Tools/CUE4Parse-MCP`. No global MCP registration or application settings were changed. Original Tempest assets were read only. No editor/game was launched, and no asset was exported.

## Reproduction

From the checkout:

```powershell
dotnet build CUE4Parse.Mcp.csproj -c Release
dotnet test Tests/CUE4Parse.Mcp.Tests.csproj -c Release
& 'C:/Users/Hero/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/python.exe' probe_local.py
```

Direct stdio command: `dotnet C:/Users/Hero/HeroHour/Saved/Tools/CUE4Parse-MCP/bin/Release/net10.0/CUE4Parse.Mcp.dll`.

Build: .NET SDK 10.0.402, CUE4Parse NuGet 1.2.2.202609, zero warnings/errors. Upstream unit tests: four passed.

## Observed results

Actual JSON-RPC initialize, initialized notification and tools/call exchanges succeeded. `init_provider` with the MapEditorProject root, `GAME_UE5_4`, `AllDirectories`, case-insensitive paths produced:

- projectName Tempest;
- 18,580 indexed files;
- 8,843 loose files;
- 8,808 .uasset, eight .umap;
- zero mounted archives and zero encrypted archives;
- no mappings loaded, no provider warnings.

DefaultFileProvider scans loose editor assets and companion .uexp/.ubulk files as well as archive directories. All four Setdressing maps were found under `Tempest/Mods/SetdressingExampleMap/Content/Maps/Multiplayer/4Player/SEM/`.

Bounded `get_package_summary` on the Foliage map succeeded: 18 exports, 41 imports, PKG_ContainsMap, isUnversioned false, isFullyLoaded true. The uncooked SoundClass `SC_Ambient_Environment` also succeeded: two exports, six imports, PKG_None, isUnversioned false.

The companion-.uexp candidate `C_Weather_WindAngle.uasset` returned `package_not_found`. This server wraps TryLoadPackage failures in that generic error; it does not establish that the file is absent or that the cause is definitely missing mappings. Root's direct CUE4Parse harness should preserve the actual exception for that case. No .usmap was found under MapEditorProject. Source accepts an optional FileUsmapTypeMappingsProvider, useful specifically for unversioned property serialization; the proven uncooked reads did not need one.

Evidence retained locally: build-local.log, test-local.log, stdio-probe.json, stdio-stderr.log, probe-console.log and probe_local.py in the checkout. The Python probe closes stdin and waits for server shutdown, terminating only its own child on timeout. No server session remains running.

Status: READY for isolated local read-only queries. Cooked data decoding remains a separate concrete compatibility test; no complete asset-transfer claim.
