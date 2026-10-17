"""Read actual material parent chains and export inherited textures with CUE.

No Unreal launch. Sources remain read-only. Output must be a fresh Saved folder.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import re


def main():
    args = argparse.ArgumentParser()
    args.add_argument("output", type=Path)
    opt = args.parse_args()
    root = Path(__file__).resolve().parents[2]
    output = opt.output.resolve()
    output.relative_to(root / "Saved")
    output.mkdir(parents=True, exist_ok=False)
    executable = root / "Scripts/research/TempestAssetProbe/bin/Release/net10.0/TempestAssetProbe.exe"
    source = root / "Saved/Tests/TempestMapEditor54"
    mappings = root / "Saved/Research/TempestMappings54"
    materials_root = root / "Saved/Research/TempestSectorTransfer/Export-01/Exports/Tempest/Content"
    cache = {}
    textures = {}
    master_defaults = {}
    report = dict(schema="HeroHour.TempestMaterialBindings.v1", result="FAIL", materials=[], textures=[])

    def normalize(value):
        # CUE includes unversioned-schema field indices in cooked property keys.
        # Keep source JSON intact; normalize only our in-memory lookup keys.
        if isinstance(value, list):
            return [normalize(x) for x in value]
        if isinstance(value, dict):
            result = {}
            for key, item in value.items():
                key = re.sub(r"\[\d+\]$", "", key)
                if key in result:
                    raise RuntimeError("Ambiguous normalized property: " + key)
                result[key] = normalize(item)
            return result
        return value

    def canonical(value):
        if isinstance(value, dict):
            value = value["ObjectPath"]
        if value.startswith("Tempest/Content/"):
            value = "/Game/" + value[len("Tempest/Content/"):]
        package = value.rsplit(".", 1)[0] if "." in value else value
        return package + "." + package.rsplit("/", 1)[1]

    def run(path, mode, folder):
        result = subprocess.run([str(executable), str(source), path, str(folder), mode, str(mappings)],
                                capture_output=True, text=True, encoding="utf-8")
        if result.returncode:
            raise RuntimeError(f"Probe failed for {path}: {result.stdout[-1200:]} {result.stderr[-500:]}")
        return folder

    def read(path):
        if path not in cache:
            folder = output / f"Material-{len(cache):02d}"
            run(path, "inspect", folder)
            obj = normalize(json.loads((folder / "Object.json").read_text(encoding="utf-8")))
            cache[path] = (obj, folder)
        return cache[path]

    try:
        sources = {}
        for file in sorted(materials_root.rglob("*.json")):
            relative = file.relative_to(materials_root).with_suffix("").as_posix()
            sources["/Game/" + relative + "." + file.stem] = str(file)
        selection = json.loads((root / "Saved/Research/TempestSectorTransfer/FullSector.json").read_text())
        for actor in selection["actors"]:
            for component in actor["components"]:
                for value in component.get("materials", []):
                    if value:
                        sources.setdefault(value, "actual_component_override")
        for path, source_json in sorted(sources.items()):
            chain, effective, scalar, vector = [], {}, {}, {}
            cursor = path
            while cursor:
                if cursor in [x["path"] for x in chain] or len(chain) >= 12:
                    raise RuntimeError("Cyclic or excessive parent chain: " + path)
                obj, folder = read(cursor)
                props = obj.get("Properties", {})
                chain.append(dict(path=cursor, type=obj["Type"], object_json=str(folder / "Object.json")))
                for x in props.get("TextureParameterValues", []):
                    name = x["ParameterInfo"]["Name"]
                    if x.get("ParameterValue"):
                        effective.setdefault(name, canonical(x["ParameterValue"]))
                if obj["Type"] == "Material":
                    if cursor not in master_defaults:
                        graph_folder = output / (folder.name + "-Graph")
                        run(cursor, "inspect-package", graph_folder)
                        expressions = json.loads((graph_folder / "Package-00.json").read_text(encoding="utf-8"))
                        master_defaults[cursor] = {}
                        for expression in expressions:
                            q = expression.get("Properties", {})
                            if expression.get("Type", "").startswith("MaterialExpressionTexture") and q.get("ParameterName") and q.get("Texture"):
                                master_defaults[cursor][q["ParameterName"]] = canonical(q["Texture"])
                    for name, value in master_defaults[cursor].items():
                        effective.setdefault(name, value)
                for key, values in [("ScalarParameterValues", scalar), ("VectorParameterValues", vector)]:
                    for x in props.get(key, []):
                        values.setdefault(x["ParameterInfo"]["Name"], x["ParameterValue"])
                parent = props.get("Parent")
                cursor = canonical(parent) if parent else None
            entry = dict(source_material=path, source_export_json=source_json, parent_chain=chain,
                         texture_parameters=effective, scalar_parameters=scalar, vector_parameters=vector,
                         resolved_textures={})
            # Actual parameter names, not filename-pattern guesses.
            for semantic, parameters in [("albedo", ["Albedo"]), ("normal", ["Normal Map"]),
                                        ("rmo", ["RMO(E) Mask", "RMO/E Mask"]), ("color_mask", ["Color Mask", "Player Color Mask"])]:
                texture = next((effective[p] for p in parameters if p in effective), None)
                if texture:
                    if texture not in textures:
                        folder = output / f"Texture-{len(textures):02d}"
                        run(texture, "export", folder)
                        png = list((folder / "Exports").rglob("*.png"))
                        if len(png) != 1:
                            raise RuntimeError("Expected exactly one exported texture: " + texture)
                        textures[texture] = dict(source=texture, png=str(png[0]),
                                                sha256=hashlib.sha256(png[0].read_bytes()).hexdigest())
                    entry["resolved_textures"][semantic] = textures[texture]
            entry["has_albedo"] = "albedo" in entry["resolved_textures"]
            report["materials"].append(entry)
        report["textures"] = list(textures.values())
        report["result"] = "PASS" if all(x["has_albedo"] for x in report["materials"]) else "PARTIAL"
    except Exception as error:
        report["error"] = str(error)
    finally:
        (output / "Bindings.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print(json.dumps({"result": report["result"], "materials": len(report["materials"]),
                          "textures": len(textures), "error": report.get("error"), "output": str(output)}))
    if report["result"] != "PASS":
        raise SystemExit(2)


if __name__ == "__main__":
    main()
