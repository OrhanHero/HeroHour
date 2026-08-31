import os
import unreal


project_dir = unreal.Paths.project_dir()
source_dir = os.path.join(project_dir, "Art", "Imported", "RTSModern")
destination = "/Game/LocalAssets/RTSModern"

sources = [
    os.path.join(source_dir, "ATV_N1_LE.fbx"),
    os.path.join(source_dir, "FA_N26_LE.fbx"),
    os.path.join(source_dir, "MSH_N2_LE.fbx"),
]

tasks = []
for filename in sources:
    if not os.path.isfile(filename):
        raise RuntimeError(f"Missing local Fab source: {filename}")

    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = False
    options.import_textures = False
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.generate_lightmap_u_vs = True
    options.static_mesh_import_data.auto_generate_collision = True

    task = unreal.AssetImportTask()
    task.filename = filename
    task.destination_path = destination
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    tasks.append(task)

unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

for task in tasks:
    if not task.imported_object_paths:
        raise RuntimeError(f"Unreal imported no object from {task.filename}")
    unreal.log(f"HeroHour imported {task.filename}: {task.imported_object_paths}")
    for object_path in task.imported_object_paths:
        mesh = unreal.load_asset(object_path)
        if isinstance(mesh, unreal.StaticMesh):
            unreal.log(f"HeroHour bounds {object_path}: {mesh.get_bounding_box()} sphere={mesh.get_bounds().sphere_radius}")

unreal.EditorAssetLibrary.save_directory(destination, only_if_is_dirty=False, recursive=True)
