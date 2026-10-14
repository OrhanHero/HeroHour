"""Read-only transform/render bounds audit for the new civic assets."""
import unreal,json
from pathlib import Path
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);assert level.load_level('/Game/Maps/CityHero')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
records=[]
for a in actors.get_all_level_actors():
    if not a.get_actor_label().startswith('DV3 SM_DV3_Civic'):continue
    c=a.static_mesh_component;m=c.static_mesh;b=m.get_bounding_box()
    records.append({'label':a.get_actor_label(),'position':str(a.get_actor_location()),'scale':str(a.get_actor_scale3d()),'bounds_min':str(b.min),'bounds_max':str(b.max),'visible':c.get_editor_property('visible'),'materials':[str(c.get_material(i).get_path_name()) for i in range(m.get_num_sections(0))]})
Path(__file__).resolve().parents[2].joinpath('Saved/Research/district-scene-audit.json').write_text(json.dumps(records,indent=2))
unreal.log('DISTRICT_AUDIT '+json.dumps(records[:3]))
