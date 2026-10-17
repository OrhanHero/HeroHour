"""Import 22 V5 portraits and update only existing DataV4 definition Icon fields.
Run in Unreal Python after the Blender render script. No balance/mesh changes.
"""
from pathlib import Path
import json
import unreal
ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/UI/PortraitsV5'
DEST='/Game/HERO/PresentationV5/Portraits'
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
LIB=unreal.EditorAssetLibrary
metadata=json.loads((SOURCE/'metadata.json').read_text(encoding='utf-8'))
assert len(metadata['records'])==22
# Preflight all files and definitions before mutation.
records=[]
for record in metadata['records']:
    source=SOURCE/record['file'];assert source.is_file(),source
    path='/Game/HERO/DataV4/D_'+record['faction']+'_'+record['role']
    definition=unreal.load_asset(path);assert definition,path
    records.append((record,source,definition))
for record,source,definition in records:
    task=unreal.AssetImportTask()
    task.set_editor_property('filename',str(source))
    task.set_editor_property('destination_name',source.stem)
    task.set_editor_property('destination_path',DEST)
    task.set_editor_property('automated',True)
    task.set_editor_property('replace_existing',True)
    task.set_editor_property('save',True)
    TOOLS.import_asset_tasks([task])
    texture=unreal.load_asset(DEST+'/'+source.stem);assert isinstance(texture,unreal.Texture2D),source
    texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
    texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
    texture.set_editor_property('never_stream',True)
    texture.set_editor_property('srgb',True)
    assert LIB.save_loaded_asset(texture,only_if_is_dirty=False)
    definition.modify()
    definition.set_editor_property('icon',texture)
    assert LIB.save_loaded_asset(definition,only_if_is_dirty=False)
    assert definition.get_editor_property('icon').get_path_name()==texture.get_path_name()
unreal.log('FACTION_PORTRAITS_V5_IMPORT_PASS 22')
