# Auto-run script for Unreal Engine 5.8 - imports Fab assets and builds the game

import sys
import os

# Add UE5 Python path
ue_path = r"C:\Program Files\Epic Games\UE_5.8\Engine\Extras\DistPython"
if os.path.exists(ue_path):
    sys.path.insert(0, ue_path)

import unreal

def main():
    project_dir = unreal.Paths.project_dir()
    print(f"Project dir: {project_dir}")
    
    # Run the import script
    import_script = os.path.join(project_dir, "Scripts", "import_all_assets.py")
    if os.path.exists(import_script):
        with open(import_script, 'r') as f:
            exec(f.read())
    else:
        print(f"Import script not found: {import_script}")
    
    # Save all packages
    unreal.EditorAssetLibrary.save_all_packages()
    print("All packages saved.")
    
    # Cook the game content
    print("Cooking content...")
    cook_result = unreal.CookUtil.cook_everything_asynchronously()
    print(f"Cook started: {cook_result}")
    
    return 0

if __name__ == "__main__":
    sys.exit(main())
