"""Import HeroHour's original concept UI art, with UI-safe texture settings.

Run from Unreal's Python commandlet after build_ui_frame_v3.py. Does not mutate
maps or HUD C++; runtime code owns rendering and nine-slice scaling.
"""
from pathlib import Path
import json

import unreal

root = Path(__file__).resolve().parents[2]
source_root = root / "Art/UI/ConceptV3"
metadata = json.loads((source_root / "metadata.json").read_text(encoding="utf-8"))
destination = metadata["import_directory"]
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
for name, size in metadata["texture_sizes"].items():
    source = source_root / (name + ".png")
    assert source.is_file(), source
    task = unreal.AssetImportTask()
    task.filename = str(source)
    task.destination_path = destination
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.save = True
    asset_tools.import_asset_tasks([task])
    texture = unreal.load_asset(destination + "/" + name)
    assert texture, "Missing imported UI texture: " + name
    texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property("compression_settings", unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property("mip_gen_settings", unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS)
    texture.set_editor_property("filter", unreal.TextureFilter.TF_BILINEAR)
    texture.set_editor_property("srgb", True)
    texture.set_editor_property("never_stream", True)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    unreal.log("UI_FRAME_V3_IMPORTED " + name + " " + str(size))
unreal.log("UI_FRAME_V3_IMPORT_PASS")
