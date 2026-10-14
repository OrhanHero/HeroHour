import unreal
from pathlib import Path
root=Path(__file__).resolve().parents[2]
tools=unreal.AssetToolsHelpers.get_asset_tools()
for source in sorted((root/'Art/UI/Portraits').glob('T_HERO_*.png')):
    task=unreal.AssetImportTask();task.filename=str(source);task.destination_path='/Game/HERO/UI'
    task.destination_name=source.stem;task.automated=True;task.replace_existing=True;task.save=True
    tools.import_asset_tasks([task]);texture=unreal.load_asset('/Game/HERO/UI/'+source.stem)
    assert texture,source.stem
    texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    unreal.log('UI_PORTRAIT_IMPORTED '+source.stem)
