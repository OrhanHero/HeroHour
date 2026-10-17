using CUE4Parse;
using CUE4Parse.FileProvider;
using CUE4Parse.UE4.Versions;
using CUE4Parse_Conversion;
using CUE4Parse_Conversion.Options;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using Serilog;

if (args.Length < 3)
    throw new ArgumentException("Usage: TempestAssetProbe projectRoot objectPath freshOutput [export]");
var root = Path.GetFullPath(args[0]);
var output = Path.GetFullPath(args[2]);
if (output.StartsWith(root.TrimEnd(Path.DirectorySeparatorChar) + Path.DirectorySeparatorChar, StringComparison.OrdinalIgnoreCase))
    throw new ArgumentException("Output must be outside the source installation");
if (Directory.Exists(output))
    throw new ArgumentException("Output must be a fresh directory");
Directory.CreateDirectory(output);
Log.Logger = new LoggerConfiguration().WriteTo.Console(standardErrorFromLevel: Serilog.Events.LogEventLevel.Verbose).CreateLogger();
CUE4ParseLog.UseLogger(Log.Logger);
var report = new Dictionary<string, object?> {
    ["schema"] = "HeroHour.TempestAssetProbe.v1", ["created_utc"] = DateTime.UtcNow,
    ["source_root"] = root, ["object_path"] = args[1], ["game_version_override"] = "GAME_UE5_4",
    ["source_read_only"] = true, ["result"] = "FAIL",
    ["limitation"] = "Object parsing/export only; no proof of native gameplay or original editable material graph."
};
try {
    using var provider = new DefaultFileProvider(root, SearchOption.AllDirectories,
        new VersionContainer(EGame.GAME_UE5_4), StringComparer.OrdinalIgnoreCase);
    if (args.Length >= 5) {
        provider.MappingsContainer = new ReflectionJsonMappingsProvider(args[4]);
        report["reflection_mapping_directory"] = Path.GetFullPath(args[4]);
        report["mapping_structs_sha256"] = Convert.ToHexString(System.Security.Cryptography.SHA256.HashData(
            File.ReadAllBytes(Path.Combine(args[4],"structs.json")))).ToLowerInvariant();
    }
    provider.Initialize();
    report["virtual_mounts"] = provider.LoadVirtualPaths();
    report["indexed_files"] = provider.Files.Count;
    var objectPaths = args[1].StartsWith('@')
        ? JObject.Parse(File.ReadAllText(args[1][1..]))["mesh_paths"]!.ToObject<string[]>()!
        : new[] { args[1] };
    objectPaths = objectPaths.Distinct(StringComparer.OrdinalIgnoreCase).ToArray();
    if (objectPaths.Length is < 1 or > 16) throw new ArgumentException("Bounded probe allows 1..16 objects");
    var objects = new List<object>();
    var failures = 0;
    var session = new ExportSession { MaxDegreeOfParallelism = 1 };
    for (var index=0; index<objectPaths.Length; ++index) {
        try {
            var export = provider.LoadPackageObject(objectPaths[index]);
            objects.Add(new { source=objectPaths[index], resolved_path=export.GetPathName(),
                export_type=export.ExportType, parser_type=export.GetType().FullName });
            if (objectPaths.Length==1) {
                report["resolved_path"] = export.GetPathName();
                report["export_type"] = export.ExportType;
                report["parser_type"] = export.GetType().FullName;
            }
            File.WriteAllText(Path.Combine(output,objectPaths.Length==1 ? "Object.json" : $"Object-{index:00}.json"),
                JsonConvert.SerializeObject(export, Formatting.Indented));
            if (args.Length>=4 && args[3]=="inspect-package") {
                var packagePath=objectPaths[index].Split('.')[0];
                var packageExports=provider.LoadPackage(packagePath).GetExports().ToArray();
                File.WriteAllText(Path.Combine(output,$"Package-{index:00}.json"),
                    JsonConvert.SerializeObject(packageExports, Formatting.Indented));
            }
            if (args.Length>=4 && args[3]=="export") session.Add(export);
        } catch (Exception error) {
            ++failures;
            objects.Add(new { source=objectPaths[index], error=error.ToString() });
        }
    }
    report["objects"] = objects;
    report["object_load_failures"] = failures;
    if (args.Length >= 4 && args[3] == "export" && session.HasQueuedItems) {
        var results = await session.RunAsync(Path.Combine(output,"Exports"),
            new ExportOptions(meshFormat: EMeshFormat.Gltf2, exportMaterials: true));
        report["exports"] = results.Select(r => new { r.Success, r.ObjectPath, r.DiskFilePaths,
            Error = r.Error?.ToString() }).ToArray();
        report["result"] = failures==0 && results.Count > 0 && results.All(r => r.Success) ? "PASS" : "PARTIAL_OR_FAILED_EXPORT";
    } else report["result"] = failures==0 ? "PASS" : "OBJECT_LOAD_FAILED";
} catch (Exception error) {
    report["error"] = error.ToString();
} finally {
    File.WriteAllText(Path.Combine(output,"Probe.json"), JsonConvert.SerializeObject(report, Formatting.Indented));
    Console.WriteLine(JsonConvert.SerializeObject(new { result=report["result"], output }));
    await Log.CloseAndFlushAsync();
}
return Equals(report["result"],"PASS") ? 0 : 2;
