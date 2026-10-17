// Copy into the independent CUE4Parse harness, not the Unreal module.
using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Globalization;
using Newtonsoft.Json.Linq;
using CUE4Parse.MappingsProvider;

public sealed class ReflectionJsonMappingsProvider : JsonTypeMappingsProvider
{
    private string _directory;
    public ReflectionJsonMappingsProvider(string directory) { _directory=directory; Reload(); }
    public override void Load(string path, StringComparer? comparer=null)
    {
        _directory=Directory.Exists(path)?path:Path.GetDirectoryName(Path.GetFullPath(path))!;
        MappingsForGame=new TypeMappings();
        AddStructs(File.ReadAllText(Path.Combine(_directory,"structs.json")));
        var enums=JArray.Parse(File.ReadAllText(Path.Combine(_directory,"enums-numeric.json")));
        foreach(var e in enums)
        {
            var name=e["name"]!.Value<string>()!;
            MappingsForGame!.Enums[name]=e["numericValues"]!
                .GroupBy(v=>long.Parse(v["value"]!.Value<string>()!,CultureInfo.InvariantCulture))
                .ToDictionary(g=>g.Key,g=>g.First()["name"]!.Value<string>()!);
        }
    }
    public override void Load(byte[] bytes, StringComparer? comparer=null)
    {
        // Bytes explicitly represent structs only; named numeric enum file remains required.
        MappingsForGame=new TypeMappings();
        AddStructs(Encoding.UTF8.GetString(bytes));
    }
    public override void Reload()=>Load(_directory);
}
