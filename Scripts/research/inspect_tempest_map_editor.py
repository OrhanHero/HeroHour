"""Read-only UE 5.4 editor inventory; never saves, migrates or exports assets.

Run in a disposable COPY of the Tempest ModKit, with its example map open.
Set TEMPEST_INSPECTION_OUTPUT to an absolute directory beneath HeroHour/Saved.
Optional TEMPEST_INSPECTION_LOAD_MAP is an existing Unreal package path. Loading
replaces the current world, so this is refused if any currently loaded package
is dirty. Default behavior inspects the already open map without replacing it.
"""
import datetime
import json
import os
from pathlib import Path

import unreal


def inspect():
    workspace = Path(__file__).resolve().parents[2]
    saved = (workspace / "Saved").resolve()
    configured = os.environ.get("TEMPEST_INSPECTION_OUTPUT", "")
    if not configured or not Path(configured).is_absolute():
        raise RuntimeError("Set TEMPEST_INSPECTION_OUTPUT to an absolute HeroHour/Saved subdirectory")
    output = Path(configured).resolve()
    if saved not in output.parents:
        raise RuntimeError("Output must be strictly beneath this script's HeroHour/Saved directory")
    report = {"schema": 1, "created_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
              "status": "INCOMPLETE", "engine": unreal.SystemLibrary.get_engine_version(),
              "project": unreal.Paths.get_project_file_path(), "read_only_assets": True,
              "limitations": ["Loaded actors only; unloaded World Partition cells are not enumerated.",
                             "Registry dependencies are references, not proof that assets are editable or portable.",
                             "No geometry, texture pixels, landscape heightmaps or asset files are exported."],
              "errors": [], "actors": [], "levels": [], "dependencies": {}}

    def attempt(label, callback, fallback=None):
        try:
            return callback()
        except Exception as exc:
            report["errors"].append({"operation": label, "error": str(exc)})
            return fallback

    def ref(obj):
        return obj.get_path_name() if obj else None

    def vec(value):
        return {name: float(getattr(value, name)) for name in ("x", "y", "z")}

    try:
        map_path = os.environ.get("TEMPEST_INSPECTION_LOAD_MAP", "")
        if map_path:
            dirty = list(unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages())
            dirty += list(unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages())
            if dirty:
                disposable = os.environ.get("TEMPEST_INSPECTION_DISPOSABLE_COPY", "")
                current_project = Path(unreal.Paths.get_project_file_path()).resolve()
                if (not disposable or Path(disposable).resolve() != current_project.parent
                        or saved not in current_project.parents):
                    raise RuntimeError("Refusing map replacement: loaded dirty packages exist")
                report["discarded_startup_dirty_packages_in_owned_copy"] = [ref(p) for p in dirty]
            if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(map_path):
                raise RuntimeError("Requested map failed to load: " + map_path)
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        if not world:
            raise RuntimeError("No editor world loaded")
        report["world"] = ref(world)
        package = world.get_path_name().split(".", 1)[0]
        report["world_package"] = package
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        registry.search_all_assets(True)
        registry.wait_for_completion()
        options = unreal.AssetRegistryDependencyOptions(
            include_soft_package_references=True, include_hard_package_references=True,
            include_searchable_names=False, include_soft_management_references=False,
            include_hard_management_references=False)
        report["dependencies"][package] = sorted(str(p) for p in (registry.get_dependencies(package, options) or []))
        loaded_levels = unreal.EditorLevelUtils.get_levels(world)
        for level in loaded_levels:
            row = {"level_class": ref(level.get_class()),
                   "package": level.get_path_name().split(".", 1)[0],
                   "loaded": True, "visible": None,
                   "visibility_limitation": "Loaded level enumeration does not independently establish viewport visibility."}
            report["levels"].append(row)
            name = row["package"]
            report["dependencies"][name] = attempt("dependencies:" + name,
                lambda name=name: sorted(str(p) for p in (registry.get_dependencies(name, options) or [])), [])
        actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
        for actor in actors:
            row = {"path": ref(actor), "class": ref(actor.get_class()), "label": actor.get_actor_label(),
                   "location": vec(actor.get_actor_location()), "scale": vec(actor.get_actor_scale3d()),
                   "rotation": {}, "tags": [str(t) for t in actor.tags], "components": []}
            rotation = actor.get_actor_rotation()
            row["rotation"] = {key: float(getattr(rotation, key)) for key in ("pitch", "yaw", "roll")}
            row["level"] = attempt("actor_level:" + ref(actor), lambda: ref(actor.get_level()))
            for comp in actor.get_components_by_class(unreal.ActorComponent):
                item = {"path": ref(comp), "class": ref(comp.get_class())}
                if isinstance(comp, unreal.StaticMeshComponent):
                    item["mesh"] = ref(comp.static_mesh)
                    item["materials"] = [ref(m) for m in comp.get_materials()]
                    if isinstance(comp, unreal.InstancedStaticMeshComponent):
                        item["instance_count"] = comp.get_instance_count()
                elif isinstance(comp, unreal.MeshComponent):
                    item["materials"] = [ref(m) for m in comp.get_materials()]
                row["components"].append(item)
            report["actors"].append(row)
        report["actor_count"] = len(report["actors"])
        report["status"] = "PASS_WITH_API_GAPS" if report["errors"] else "PASS"
    except Exception as exc:
        report["status"] = "FAIL"
        report["errors"].append({"operation": "inspect", "error": str(exc)})
    output.mkdir(parents=True, exist_ok=True)
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    destination = output / ("tempest-editor-inventory-" + stamp + ".json")
    with destination.open("x", encoding="utf-8") as handle:
        json.dump(report, handle, ensure_ascii=False, indent=2)
    unreal.log("TEMPEST_MAP_INSPECTION status=%s report=%s" % (report["status"], destination))
    if report["status"] == "FAIL":
        raise RuntimeError("Inspection incomplete; see " + str(destination))


if __name__ == "__main__":
    inspect()
