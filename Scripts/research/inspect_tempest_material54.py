"""Validate actual CUE4Parse material-package exports; never guesses channel names.

Usage: python inspect_tempest_material54.py MASTER_PACKAGE_JSON MI_PACKAGE_JSON OUTPUT
Output retains hashes, numbered graph edges, scalar/vector overrides and switch states.
No Unreal editor, source asset modification, network or runtime execution.
"""
import argparse
import hashlib
import json
from pathlib import Path


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("master", type=Path)
    parser.add_argument("instance", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    master = json.loads(args.master.read_text(encoding="utf-8"))
    instance = json.loads(args.instance.read_text(encoding="utf-8"))
    expected = "/Tempest/MaterialLibrary/Props/M_MASTER_Props."
    checks = []

    def edge(node, socket, target, output=0):
        value = master[node]["Properties"][socket]
        actual = value["Expression"]["ObjectPath"]
        ok = actual.endswith(expected + str(target)) and value.get("OutputIndex", 0) == output
        checks.append({"node": node, "socket": socket, "target": target,
                       "output_index": output, "pass": ok})
        if not ok:
            raise ValueError(f"Unexpected material edge: {node}.{socket}: {value}")

    edge(117, "Alpha", 460, 1)  # explicit MaskR in original graph
    edge(117, "A", 364)
    edge(117, "B", 365)
    edge(287, "Input", 117)
    edge(156, "A", 287)
    edge(156, "B", 460, 2)  # explicit MaskG
    edge(156, "C", 460, 3)  # explicit MaskB
    edge(249, "Input", 128, 2)  # SplitComponents output G -> Metallic
    edge(241, "Input", 128, 3)  # SplitComponents output B -> AO
    edge(179, "A", 459)
    edge(179, "B", 346)
    edge(182, "B", 128, 3)
    edge(175, "A", 341)
    edge(175, "B", 56)
    edge(25, "B", 462, 3)
    edge(184, "B", 142, 1)
    edge(185, "B", 142, 2)
    edge(186, "B", 142, 3)
    props = instance[0]["Properties"]
    overrides = {kind: {x["ParameterInfo"]["Name"]: x["ParameterValue"]
                        for x in props.get(kind, [])}
                 for kind in ["ScalarParameterValues", "VectorParameterValues"]}
    switches = instance[1]["Properties"]["StaticParameters"]["StaticSwitchParameters"]
    report = {
        "schema": "HeroHour.TempestMaterialGraphEvidence.v1", "result": "PASS",
        "master_export_count": len(master), "instance_export_count": len(instance),
        "inputs": [{"path": str(p.resolve()), "sha256": hashlib.sha256(p.read_bytes()).hexdigest()}
                   for p in [args.master, args.instance]],
        "edge_checks": checks, "instance_overrides": overrides,
        "static_switch_overrides": switches,
        "observed_base_channels": {"R": "saturate(lerp(Roughness Min, Roughness Max, R))",
                                   "G": "Metallic before dynamic snow",
                                   "B": "AmbientOcclusion and albedo AO blend"},
        "limitation": "Static source graph, not shader rendering equivalence; rain/snow/grunge functions and live MPC values remain separate dependencies."
    }
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    print(json.dumps({"result": "PASS", "edge_checks": len(checks), "output": str(args.output)}))


if __name__ == "__main__":
    main()
