"""Read-only exact component-transform/bounds probe in the disposable ModKit copy.

Loads the original example map. Never saves assets or changes components.
Set TEMPEST_SECTOR_LANDSCAPE_BOUNDS=0 to omit the optional 64-component probe.
"""
import datetime
import hashlib
import json
import os
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
PROJECT = ROOT / 'Saved/Tests/TempestMapEditor54'
if Path(unreal.Paths.project_dir()).resolve() != PROJECT.resolve():
    raise RuntimeError('Refusing inspection outside the disposable TempestMapEditor54 copy')
SELECTION = ROOT / 'Saved/Research/TempestSectorTransfer/FullSector.json'
OUTPUT = ROOT / 'Saved/Research/TempestSectorTransfer/ComponentVerification54.json'
MAP = '/SetdressingExampleMap/Maps/Multiplayer/4Player/SEM/MAP_MP_SetdressingExampleMap_4P_P'
report = dict(schema='HeroHour.TempestSectorComponents54.v1', result='FAIL',
    created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),
    engine=unreal.SystemLibrary.get_engine_version(), project=str(PROJECT),
    read_only_assets=True, components=[], landscape_components=[], errors=[],
    tolerances=dict(location_cm=0.001, rotation_degrees=0.001, scale=0.000001))


def vec(value):
    return {key: float(getattr(value, key)) for key in ('x', 'y', 'z')}


try:
    data = json.loads(SELECTION.read_text(encoding='utf-8'))
    report['source_selection_sha256'] = hashlib.sha256(SELECTION.read_bytes()).hexdigest()
    report['source_inventory_sha256'] = data['source_inventory_sha256']
    dirty = list(unreal.EditorLoadingAndSavingUtils.get_dirty_content_packages())
    dirty += list(unreal.EditorLoadingAndSavingUtils.get_dirty_map_packages())
    report['discarded_startup_dirty_packages_in_owned_copy'] = [p.get_path_name() for p in dirty]
    # Startup dirty transient packages may be discarded only because of the hard
    # project path guard above. The user's original ModKit is never opened here.
    if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP):
        raise RuntimeError('Original example map did not load')
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    by_path = {a.get_path_name(): a for a in actors}
    for source in data['actors']:
        actor = by_path.get(source['path'])
        if not actor:
            raise RuntimeError('Selected actor not loaded: ' + source['path'])
        components = {c.get_path_name(): c for c in actor.get_components_by_class(unreal.SceneComponent)}
        for component_source in source['components']:
            if not component_source.get('mesh'):
                continue
            component = components.get(component_source['path'])
            if not component:
                raise RuntimeError('Selected mesh component missing: ' + component_source['path'])
            location = vec(component.get_editor_property('relative_location'))
            scale = vec(component.get_editor_property('relative_scale3d'))
            rotator = component.get_editor_property('relative_rotation')
            rotation = {key: float(getattr(rotator, key)) for key in ('pitch', 'yaw', 'roll')}
            # Rotations differing by whole turns are geometrically identical.
            rotation_identity = all(abs((value+180) % 360-180) <= 0.001 for value in rotation.values())
            identity = (all(abs(v) <= 0.001 for v in location.values()) and rotation_identity
                        and all(abs(v-1) <= 0.000001 for v in scale.values()))
            origin, extent, radius = unreal.SystemLibrary.get_component_bounds(component)
            row = dict(actor=source['path'], component=component_source['path'],
                root_component=actor.get_editor_property('root_component').get_path_name(),
                is_actor_root=component == actor.get_editor_property('root_component'),
                relative_location=location, relative_rotation=rotation, relative_scale=scale,
                relative_identity=identity, world_bounds_origin=vec(origin),
                world_bounds_extent=vec(extent), world_bounds_sphere_radius=float(radius),
                mesh=component.static_mesh.get_path_name() if component.static_mesh else None)
            # A root component stores actor-to-world transform as relative
            # transform. Nonidentity root transform is expected and is not an
            # extra component offset. Only attached mesh components require identity.
            row['actor_transform_sufficient'] = row['is_actor_root'] or identity
            report['components'].append(row)
    if os.environ.get('TEMPEST_SECTOR_LANDSCAPE_BOUNDS', '1') != '0':
        landscape = by_path.get(data['landscape_source']['path'])
        if not landscape:
            raise RuntimeError('Inventoried landscape actor not loaded')
        wanted = {c['path'] for c in data['landscape_source']['components']
                  if c['class'] == '/Script/Landscape.LandscapeComponent'}
        for comp in landscape.get_components_by_class(unreal.SceneComponent):
            if comp.get_path_name() not in wanted:
                continue
            origin, extent, radius = unreal.SystemLibrary.get_component_bounds(comp)
            bounds = data['bounds_xy_actor_origins']
            overlaps = all(getattr(origin, key)+getattr(extent, key) >= bounds['min'][key]
                and getattr(origin, key)-getattr(extent, key) <= bounds['max'][key] for key in ('x', 'y'))
            report['landscape_components'].append(dict(path=comp.get_path_name(),
                world_bounds_origin=vec(origin), world_bounds_extent=vec(extent),
                world_bounds_sphere_radius=float(radius), overlaps_sector_xy=overlaps))
        if len(report['landscape_components']) != len(wanted):
            raise RuntimeError('Not all inventoried landscape components resolved')
    if len(report['components']) != len(data['actors']):
        raise RuntimeError('Expected exactly one mesh component per selected actor')
    report['actor_transform_sufficient_for_all'] = all(c['actor_transform_sufficient'] for c in report['components'])
    report['result'] = 'PASS' if report['actor_transform_sufficient_for_all'] else 'COMPONENT_OFFSETS_REQUIRE_TRANSFER'
except Exception as error:
    report['errors'].append(str(error))
finally:
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    unreal.log('TEMPEST_SECTOR_COMPONENT54_RESULT='+report['result']+' report='+str(OUTPUT))
if report['result'] == 'FAIL':
    raise RuntimeError('; '.join(report['errors']))
