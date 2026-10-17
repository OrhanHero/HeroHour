"""Reimport exactly the existing Aegis Factory portrait asset; no definition edits."""
from pathlib import Path
import hashlib
import json
import unreal
ROOT=Path(__file__).resolve().parents[2];SOURCE=ROOT/'Art/UI/PortraitsV5'
NAME='T_HR_Aegis_Factory';DEST='/Game/HERO/PresentationV5/Portraits';PATH=DEST+'/'+NAME
metadata=json.loads((SOURCE/'metadata.json').read_text(encoding='utf-8'))
assert len(metadata['records'])==22
record=next(r for r in metadata['records'] if r['file']==NAME+'.png')
assert record['surface_key']=='Aegis_Factory_TrellisV10' and record['source_unchanged']
assert hashlib.sha256((SOURCE/record['file']).read_bytes()).hexdigest()==record['file_sha256']
assert hashlib.sha256((ROOT/record['source']).read_bytes()).hexdigest()==record['source_sha256']
texture=unreal.load_asset(PATH);assert isinstance(texture,unreal.Texture2D)
definition=unreal.load_asset('/Game/HERO/DataV4/D_Aegis_Factory');assert definition
icon_before=str(definition.get_editor_property('icon'));assert NAME in icon_before,icon_before
definition_before=tuple(str(definition.get_editor_property(k)) for k in ('icon','static_mesh','stats'))
others=[r for r in metadata['records'] if r['file']!=NAME+'.png'];assert len(others)==21
preserved=json.loads((ROOT/'docs/verification/2026-10-16/reference-v10/factory-portrait/Preserved21.json').read_text(encoding='utf-8'))
assert len(preserved)==21 and set(preserved)=={r['file'] for r in others}
for other in others:
 snapshot=preserved[other['file']]
 assert other==snapshot['metadata'],('Preserved portrait record changed',other['file'])
 assert hashlib.sha256((SOURCE/other['file']).read_bytes()).hexdigest()==snapshot['sha256'],('Preserved portrait bytes changed',other['file'])
task=unreal.AssetImportTask();task.filename=str(SOURCE/(NAME+'.png'));task.destination_name=NAME
task.destination_path=DEST;task.automated=True;task.replace_existing=True;task.save=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
texture=unreal.load_asset(PATH);assert isinstance(texture,unreal.Texture2D)
texture.modify();texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_EDITOR_ICON)
texture.set_editor_property('lod_group',unreal.TextureGroup.TEXTUREGROUP_UI)
texture.set_editor_property('never_stream',True);texture.set_editor_property('srgb',True)
assert unreal.EditorAssetLibrary.save_loaded_asset(texture,only_if_is_dirty=False)
assert tuple(str(definition.get_editor_property(k)) for k in ('icon','static_mesh','stats'))==definition_before
report=dict(result='PASS',reimported_texture=texture.get_path_name(),icon_soft_reference_unchanged=icon_before,
 definition_mesh_icon_stats_unchanged=True,reimported_count=1,preserved_source_portraits=21,source_image_sha256=record['file_sha256'])
(ROOT/'Saved/Research/FV10Portrait').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/FV10Portrait/Import.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
unreal.log('TRELLIS_FACTORY_PORTRAIT_V10_IMPORT_PASS '+json.dumps(report))
