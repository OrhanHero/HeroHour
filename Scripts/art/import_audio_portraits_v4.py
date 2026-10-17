"""Run in Unreal Python after original PNG and WAV source generation."""
from pathlib import Path
import unreal
ROOT=Path(__file__).resolve().parents[2]
tools=unreal.AssetToolsHelpers.get_asset_tools()
for source in list((ROOT/'Art/Audio').glob('A_*.wav'))+list((ROOT/'Art/UI/Portraits').glob('T_HR_*.png')):
    task=unreal.AssetImportTask();task.filename=str(source);task.destination_name=source.stem
    task.destination_path='/Game/HERO/EffectsV4' if source.suffix=='.wav' else '/Game/HERO/UI'
    task.automated=True;task.replace_existing=True;task.save=True
    tools.import_asset_tasks([task])
    asset=unreal.load_asset(task.destination_path+'/'+source.stem);assert asset,source
    if source.suffix=='.png':
        asset.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
        asset.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
        asset.set_editor_property('never_stream',True)
        unreal.EditorAssetLibrary.save_loaded_asset(asset)
unreal.log('AUDIO_PORTRAITS_V4_IMPORT_PASS')
