"""Activate only the imported original Aegis factory mesh, preserving gameplay."""
import json
from pathlib import Path
import unreal
ROOT=Path(unreal.Paths.project_dir()).resolve()
mesh=unreal.load_asset('/Game/HERO/TrellisFactoryV10/Meshes/SM_FV10_AegisFactory')
definition=unreal.load_asset('/Game/HERO/DataV4/D_Aegis_Factory')
assert mesh and definition
assert mesh.get_editor_property('nanite_settings').get_editor_property('enabled')
before={key:str(definition.get_editor_property(key)) for key in ('icon','stats')}
previous=str(definition.get_editor_property('static_mesh'))
definition.modify();definition.set_editor_property('static_mesh',mesh)
assert unreal.EditorAssetLibrary.save_loaded_asset(definition,only_if_is_dirty=False)
assert before=={key:str(definition.get_editor_property(key)) for key in ('icon','stats')}
report=dict(result='PASS',mesh=mesh.get_path_name(),definition=definition.get_path_name(),previous=previous,stats_and_icon_unchanged=True)
(ROOT/'Saved/Research/FactoryV10-Activation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('FACTORY_V10_ACTIVATION_PASS '+json.dumps(report))
