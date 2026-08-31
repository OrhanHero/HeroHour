import os
import unreal


project_dir = unreal.Paths.project_dir()

# RTS Modern Vehicles (from Fab download)
rts_source_dir = os.path.join(project_dir, "Art", "Imported", "RTSModern")
rts_destination = "/Game/LocalAssets/RTSModern"

# Military Free Props (from Fab download)
mil_source_dir = os.path.join(project_dir, "Art", "Imported", "MilitaryFree", "Military_Free_fbx", "Separate_Assets_fbx")
mil_destination = "/Game/LocalAssets/MilitaryFree"

tasks = []

# Import RTS vehicles
rts_files = [
    ("ATV_N1_LE.fbx", "ground"),
    ("FA_N26_LE.fbx", "air"),
    ("MSH_N2_LE.fbx", "air"),
]

for filename, role in rts_files:
    filepath = os.path.join(rts_source_dir, filename)
    if not os.path.isfile(filepath):
        print(f"WARNING: Missing RTS file: {filepath}")
        continue
    
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = True
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.generate_lightmap_u_vs = True
    options.static_mesh_import_data.auto_generate_collision = True
    
    task = unreal.AssetImportTask()
    task.filename = filepath
    task.destination_path = rts_destination
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    tasks.append(task)
    print(f"Queued import: {filename} -> {rts_destination}")

# Import key military props
mil_props = ["Barrier_004", "Barrier_006", "Box_003", "Tower_003", "Tent_002"]
for prop_name in mil_props:
    filepath = os.path.join(mil_source_dir, f"{prop_name}.fbx")
    if not os.path.isfile(filepath):
        print(f"WARNING: Missing prop: {filepath}")
        continue
    
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = True
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.auto_generate_collision = True
    
    task = unreal.AssetImportTask()
    task.filename = filepath
    task.destination_path = mil_destination
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    tasks.append(task)
    print(f"Queued import: {prop_name}.fbx -> {mil_destination}")

print(f"\nTotal tasks: {len(tasks)}")
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)

# Verify imports
for task in tasks:
    if task.imported_object_paths:
        print(f"SUCCESS: {task.filename} -> {task.imported_object_paths}")
    else:
        print(f"FAILED: {task.filename}")

# Save directories
unreal.EditorAssetLibrary.save_directory(rts_destination, only_if_is_dirty=False, recursive=True)
unreal.EditorAssetLibrary.save_directory(mil_destination, only_if_is_dirty=False, recursive=True)
print("\nDone importing assets.")
