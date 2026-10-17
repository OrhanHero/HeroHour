"""Repair the evidenced positional Rotator axis error on original HERO props.

UE 5.8 Python positional Rotator(0,90,0) creates pitch=90, not yaw=90.
Root runs this map mutation serially after asset imports. Keyword args are required.
"""
from pathlib import Path
import json,math
import unreal
ROOT=Path(__file__).resolve().parents[2]
LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert LEVEL.load_level('/Game/Maps/CityHero')
records=[];houses=[];untouched=0

def rotation(r):return dict(pitch=r.pitch,yaw=r.yaw,roll=r.roll)

def bounds_z(actor):
    centre,extent=actor.get_actor_bounds(only_colliding_components=False)
    return [centre.z-extent.z,centre.z+extent.z]

for actor in ACTORS.get_all_level_actors():
    if not isinstance(actor,unreal.StaticMeshActor):continue
    c=actor.static_mesh_component;mesh=c.static_mesh
    if not mesh or not mesh.get_path_name().startswith('/Game/HERO/'):continue
    before=actor.get_actor_rotation()
    if abs(before.pitch)>.001 or abs(before.roll)>.001:
        # All affected authoring families intended (0, angle, 0). Euler angles
        # alone are ambiguous beyond +/-90 degrees; forward X/Z retains angle.
        forward=actor.get_actor_forward_vector()
        assert abs(forward.y)<.002,(actor.get_actor_label(),rotation(before),forward)
        angle=math.degrees(math.atan2(forward.z,forward.x))
        before_bounds=bounds_z(actor);pos=actor.get_actor_location();scale=actor.get_actor_scale3d()
        actor.modify()
        actor.set_actor_rotation(unreal.Rotator(pitch=0,yaw=angle,roll=0),False)
        after=actor.get_actor_rotation();after_pos=actor.get_actor_location();after_scale=actor.get_actor_scale3d()
        assert abs(after.pitch)<.001 and abs(after.roll)<.001,rotation(after)
        assert all(abs(getattr(after_pos,k)-getattr(pos,k))<.001 and abs(getattr(after_scale,k)-getattr(scale,k))<.001 for k in ('x','y','z'))
        records.append(dict(label=actor.get_actor_label(),mesh=mesh.get_path_name(),before=rotation(before),after=rotation(after),recovered_yaw_degrees=angle,before_world_z=before_bounds,after_world_z=bounds_z(actor)))
    else:untouched+=1
    if mesh.get_name()=='SM_CH_House':
        actual=bounds_z(actor);expected=[actor.get_actor_location().z,actor.get_actor_location().z+487.5*actor.get_actor_scale3d().z]
        assert all(abs(a-b)<1 for a,b in zip(actual,expected)),(actor.get_actor_label(),actual,expected)
        houses.append(dict(label=actor.get_actor_label(),position=str(actor.get_actor_location()),world_z=actual,expected_z=expected))
assert len(houses)==64,len(houses)
assert LEVEL.save_current_level()
report=dict(result='PASS',map='/Game/Maps/CityHero',fixed_count=len(records),untouched_upright_count=untouched,houses_validated=len(houses),evidence='Saved/Logs/ConceptV6-RotationAudit.log: Rotator(0,90,0) -> pitch90 yaw0 roll0',repair='recover intended yaw from forward X/Z, set explicit pitch/yaw/roll keywords',repairs=records,houses=houses)
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/ConceptV6-RotationRepair.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('CONCEPT_V6_ROTATION_REPAIR_PASS '+str(len(records))+' houses='+str(len(houses)))
