"""UE5.4 probe of a cooked container, restricted to the disposable ModKit copy.

Run only by the coordinating agent in UnrealEditor-Cmd with Python support.
No asset saving, imports, source changes or global configuration writes.
"""
import datetime
import json
from pathlib import Path
import uuid
import unreal


EXPECTED_PROJECT = Path(r'C:/Users/Hero/HeroHour/Saved/Tests/TempestMapEditor54')
OUTPUT_ROOT = Path(r'C:/Users/Hero/HeroHour/Saved/Research/TempestAssetTransfer')
MESH_PATH = '/Game/Tempest/Props/Faction_Containers/SM_GDF_Container'
TEXTURE_ROOT = '/Game/Tempest/Props/Faction_Containers/Textures/GDF/'


def json_value(value):
    if value is None or isinstance(value, (str, int, float, bool)):
        return value
    if isinstance(value, unreal.Object):
        return value.get_path_name()
    if hasattr(value, 'x') and hasattr(value, 'y') and hasattr(value, 'z'):
        return dict(x=float(value.x), y=float(value.y), z=float(value.z))
    return str(value)


def property_value(obj, name):
    try:
        return json_value(obj.get_editor_property(name))
    except Exception as error:
        return dict(error=str(error))


def export_asset(obj, target, exporter_name):
    result = dict(path=str(target), exporter=exporter_name, success=False)
    try:
        exporter_class = getattr(unreal, exporter_name)
        task = unreal.AssetExportTask()
        task.object = obj
        task.filename = str(target)
        task.automated = True
        task.prompt = False
        task.replace_identical = False
        task.write_empty_files = False
        task.exporter = exporter_class()
        if exporter_name == 'StaticMeshExporterFBX':
            options = unreal.FbxExportOption()
            options.ascii = False
            options.level_of_detail = False
            options.collision = False
            task.options = options
        result['exporter_return'] = bool(unreal.Exporter.run_asset_export_task(task))
        result['task_errors'] = [str(error) for error in task.errors]
        result['bytes'] = target.stat().st_size if target.is_file() else 0
        result['success'] = result['exporter_return'] and result['bytes'] > 0
    except Exception as error:
        result['error'] = str(error)
        result['bytes'] = target.stat().st_size if target.is_file() else 0
    return result


def parameter_values(material):
    result = dict(path=material.get_path_name(), class_name=material.get_class().get_name())
    for property_name in ('parent', 'texture_parameter_values', 'scalar_parameter_values',
                          'vector_parameter_values'):
        try:
            value = material.get_editor_property(property_name)
            if property_name == 'parent':
                result[property_name] = json_value(value)
                continue
            parameters = []
            for parameter in value:
                item = {}
                try:
                    info = parameter.get_editor_property('parameter_info')
                    item['name'] = str(info.get_editor_property('name'))
                    item['association'] = str(info.get_editor_property('association'))
                    item['index'] = int(info.get_editor_property('index'))
                except Exception as error:
                    item['parameter_info_error'] = str(error)
                item['value'] = property_value(parameter, 'parameter_value')
                parameters.append(item)
            result[property_name] = parameters
        except Exception as error:
            result[property_name] = dict(error=str(error))
    return result


def main():
    project = Path(unreal.Paths.convert_relative_path_to_full(unreal.Paths.project_dir())).resolve()
    if project != EXPECTED_PROJECT.resolve():
        raise RuntimeError(f'Refusing project outside the owned test copy: {project}')
    unique = datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%SZ') + '-' + uuid.uuid4().hex[:8]
    output = OUTPUT_ROOT / ('Engine54-' + unique)
    output.mkdir(parents=True, exist_ok=False)
    report = dict(schema='HeroHour.TempestContainerEngine54.v1', project=str(project),
                  output=str(output), source_mesh=MESH_PATH, engine=unreal.SystemLibrary.get_engine_version(),
                  source_assets_saved=False, exports=[], textures=[], result='FAIL')
    try:
        mesh = unreal.load_asset(MESH_PATH)
        if not isinstance(mesh, unreal.StaticMesh):
            raise RuntimeError(f'Expected StaticMesh, received {mesh}')
        report['mesh_class'] = mesh.get_class().get_name()
        bounds = mesh.get_bounds()
        report['bounds'] = dict(origin=json_value(bounds.origin), box_extent=json_value(bounds.box_extent),
                                sphere_radius=float(bounds.sphere_radius), units='Unreal centimeters')
        report['material_slots'] = []
        for index, slot in enumerate(mesh.get_editor_property('static_materials')):
            material = slot.get_editor_property('material_interface')
            report['material_slots'].append(dict(index=index,
                slot_name=str(slot.get_editor_property('material_slot_name')),
                imported_slot_name=str(slot.get_editor_property('imported_material_slot_name')),
                material=parameter_values(material) if material else None))
        fbx = export_asset(mesh, output / 'SM_GDF_Container.fbx', 'StaticMeshExporterFBX')
        report['exports'].append(fbx)
        if not fbx['success']:
            report['exports'].append(export_asset(mesh, output / 'SM_GDF_Container.obj', 'StaticMeshExporterOBJ'))
        for suffix in ('D', 'N', 'RMO', 'C'):
            package = TEXTURE_ROOT + 'T_GDF_Container_' + suffix
            item = dict(package=package)
            try:
                texture = unreal.load_asset(package)
                if not texture:
                    raise RuntimeError('Texture load returned None')
                item['class_name'] = texture.get_class().get_name()
                for name in ('srgb', 'compression_settings', 'filter', 'lod_group'):
                    item[name] = property_value(texture, name)
                # The cooked PNG exporter asserted in ImageCore in the first
                # bounded test. Use the successfully tested CUE4Parse texture
                # decoder instead of repeating an unsafe native export.
                item['export'] = dict(status='SKIPPED_COOKED_NATIVE_TEXTURE_EXPORT',
                    reason='Use CUE4Parse decoded PNGs; native ImageCore assertion recorded in first run.')
            except Exception as error:
                item['error'] = str(error)
            report['textures'].append(item)
        report['result'] = 'PASS' if any(record['success'] for record in report['exports']) else 'INSPECTED_EXPORT_FAILED'
    except Exception as error:
        report['error'] = str(error)
    finally:
        report_path = output / 'Container-Engine54-Report.json'
        report_path.write_text(json.dumps(report, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
        unreal.log('TEMPEST_CONTAINER_ENGINE54_RESULT=' + report['result'] + ' report=' + str(report_path))


main()
