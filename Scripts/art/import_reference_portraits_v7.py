"""Reimport four atlas portraits and nine green Ember portraits. No definition mutations.
Root executes through Unreal Python after the V7 Blender portrait render.
"""
from pathlib import Path
import hashlib
import json
import unreal
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/UI/PortraitsV5'
DEST='/Game/HERO/PresentationV5/Portraits'
TARGETS=('T_HR_Aegis_HQ','T_HR_Aegis_Combat','T_HR_Ember_HQ','T_HR_Ember_Combat',
         'T_HR_Ember_Collector','T_HR_Ember_Depot','T_HR_Ember_Factory',
         'T_HR_Ember_PowerPlant','T_HR_Ember_Barracks','T_HR_Ember_Radar',
         'T_HR_Ember_Defense','T_HR_Ember_Infantry','T_HR_Ember_Siege')
metadata=json.loads((SOURCE/'metadata.json').read_text(encoding='utf-8'))
assert len(metadata['records'])==22
records={Path(r['file']).stem:r for r in metadata['records']}
# Resolve every input and existing target before the first mutation.
for name in TARGETS:
 record=records[name];source=SOURCE/(name+'.png')
 assert source.is_file() and record['source_unchanged']
 assert hashlib.sha256(source.read_bytes()).hexdigest()==record['file_sha256']
 assert isinstance(unreal.load_asset(DEST+'/'+name),unreal.Texture2D),name
for name in TARGETS:
 task=unreal.AssetImportTask()
 task.filename=str(SOURCE/(name+'.png'));task.destination_name=name
 task.destination_path=DEST;task.automated=True;task.replace_existing=True;task.save=True
 unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
 texture=unreal.load_asset(DEST+'/'+name);assert isinstance(texture,unreal.Texture2D)
 texture.modify()
 texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
 texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
 texture.set_editor_property('never_stream',True)
 texture.set_editor_property('srgb',True)
 assert unreal.EditorAssetLibrary.save_loaded_asset(texture,only_if_is_dirty=False)
unreal.log('REFERENCE_PORTRAITS_V7_IMPORT_PASS 13')
