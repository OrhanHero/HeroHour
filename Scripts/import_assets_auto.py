# Auto-run script for Unreal Engine 5.8 - imports Fab assets

import os
import sys

project_dir = r"E:\HeroHour"
print(f"Running in project: {project_dir}")

# Add UE Python path
ue_path = r"F:\UE\UE_5.8\Engine\Extras\DistPython"
if os.path.exists(ue_path):
    sys.path.insert(0, ue_path)

try:
    import unreal
except ImportError as e:
    print(f"Failed to import unreal: {e}")
    sys.exit(1)

def import_fbx(filepath, dest_path, name_hint=None):
    """Import an FBX file as a StaticMesh"""
    if not os.path.exists(filepath):
        print(f"WARNING: File not found: {filepath}")
        return None
    
    options = unreal.FbxImportUI()
    options.import_mesh = True
    options.import_as_skeletal = False
    options.import_materials = True
    options.import_textures = True
    options.static_mesh_import_data.combine_meshes = True
    options.static_mesh_import_data.auto_generate_collision = True
    
    task = unreal.AssetImportTask()
    task.filename = filepath
    task.destination_path = dest_path
    task.automated = True
    task.replace_existing = True
    task.save = True
    task.options = options
    
    # if name_hint:
    #     task.asset_replacer = name_hint
    
    return task

def main():
    # Import RTS vehicles
    rts_dir = os.path.join(project_dir, "Art", "Imported", "RTSModern")
    rts_dest = "/Game/LocalAssets/RTSModern"
    
    rts_files = [
        ("ATV_N1_LE.fbx", "ATV_N1_LE"),
        ("FA_N26_LE.fbx", "FA_N26_LE"),
        ("MSH_N2_LE.fbx", "MSH_N2_LE"),
    ]
    
    tasks = []
    for filename, asset_name in rts_files:
        filepath = os.path.join(rts_dir, filename)
        task = import_fbx(filepath, rts_dest, f"/Game{rts_dest}/{asset_name}")
        if task:
            tasks.append(task)
            print(f"Queued: {filename} -> {rts_dest}/{asset_name}")
    
    # Import military props
    mil_dir = os.path.join(project_dir, "Art", "Imported", "MilitaryFree", "Military_Free_fbx", "Separate_Assets_fbx")
    mil_dest = "/Game/LocalAssets/MilitaryFree"
    
    mil_props = ["Barrier_004", "Barrier_006", "Box_003", "Tower_003", "Tent_002"]
    for prop in mil_props:
        filepath = os.path.join(mil_dir, f"{prop}.fbx")
        task = import_fbx(filepath, mil_dest, f"/Game{mil_dest}/{prop}")
        if task:
            tasks.append(task)
            print(f"Queued: {prop}.fbx -> {mil_dest}/{prop}")
    
    if not tasks:
        print("ERROR: No tasks to import!")
        return 1
    
    print(f"\nImporting {len(tasks)} assets...")
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks(tasks)
    
    # Check results
    success_count = 0
    for task in tasks:
        if task.imported_object_paths:
            print(f"SUCCESS: {os.path.basename(task.filename)}")
            success_count += 1
        else:
            print(f"FAILED: {os.path.basename(task.filename)}")
    
    print(f"\nImported {success_count}/{len(tasks)} assets successfully.")
    
    # Save packages
    print("Saving packages...")
    unreal.EditorAssetLibrary.save_directory(rts_dest, recursive=True)
    unreal.EditorAssetLibrary.save_directory(mil_dest, recursive=True)
    print("Done!")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
