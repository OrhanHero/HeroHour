"""Activate the independently imported/reviewed original HQ, preserving stats."""
import json
from pathlib import Path
import unreal

ROOT=Path(unreal.Paths.project_dir()).resolve()
report=json.loads((ROOT/'Saved/Research/TrellisHQV9-Import.json').read_text(encoding='utf-8'))
mesh=unreal.load_asset(report['mesh'])
assert isinstance(mesh,unreal.StaticMesh)
assert mesh.get_editor_property('nanite_settings').get_editor_property('enabled')
assert mesh.get_editor_property('static_materials')[0].material_interface.get_path_name()==report['material']
definition=unreal.load_asset('/Game/HERO/DataV4/D_Aegis_HQ')
assert definition
previous=definition.get_editor_property('static_mesh')
old_icon=definition.get_editor_property('icon')
old_stats=str(definition.get_editor_property('stats'))
definition.modify()
definition.set_editor_property('static_mesh',mesh)
assert unreal.EditorAssetLibrary.save_loaded_asset(definition,only_if_is_dirty=False)
assert definition.get_editor_property('icon')==old_icon
assert str(definition.get_editor_property('stats'))==old_stats
out=dict(result='PASS',definition=definition.get_path_name(),previous_mesh=str(previous),
         mesh=mesh.get_path_name(),stats_and_icon_unchanged=True,
         limitation='Runtime screenshot and final package acceptance remain separate; portrait update pending.')
(ROOT/'Saved/Research/TrellisHQV9-Activation.json').write_text(json.dumps(out,indent=2),encoding='utf-8')
unreal.log('TRELLIS_HQ_V9_ACTIVATION_PASS '+json.dumps(out))
