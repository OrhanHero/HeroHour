"""Import the inventoried original sector into the disposable UE5.8 probe only.

Actual child-to-parent texture bindings are used, never guessed from filenames.
An unresolved albedo is a failed import rather than a silent gray substitute.
"""
import hashlib
import json
import struct
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
PROJECT = ROOT / 'Saved/Tests/TempestTransferProbe58'
if Path(unreal.Paths.project_dir()).resolve() != PROJECT.resolve():
    raise RuntimeError('Refusing to modify a project other than the isolated transfer probe')
SOURCE = ROOT / 'Saved/Research/TempestSectorTransfer/Export-01/Exports'
BINDINGS = ROOT / 'Saved/Research/TempestSectorTransfer/MaterialBindings-06/Bindings.json'
SELECTION = ROOT / 'Saved/Research/TempestSectorTransfer/FullSector.json'
OUT = ROOT / 'Saved/Research/TempestSectorTransfer/Unreal58'
DEST = '/Game/TransferProbe/SectorRetry02'
MAP = '/Game/TransferProbe/Maps/OriginalSector02'
OUT.mkdir(parents=True, exist_ok=True)
report = dict(schema='HeroHour.TempestSectorImport58.v1', result='FAIL',
              engine=unreal.SystemLibrary.get_engine_version(), imports=[], meshes=[],
              materials=[], actors=[], limitations=[
                  'Baseline uses evidenced inherited albedo and normal bindings.',
                  'Full Tempest shader, landscape and gameplay are not reconstructed.',
                  'Actor transforms are preserved with common anchor translation; source component verification is recorded separately.'])


def import_file(path, folder):
    task = unreal.AssetImportTask()
    task.filename = str(path)
    task.destination_path = folder
    task.automated = True
    task.save = True
    task.replace_existing = False
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    paths = list(task.imported_object_paths)
    report['imports'].append(dict(source=str(path), objects=paths))
    if not paths:
        raise RuntimeError('Import returned no objects: ' + str(path))
    return [unreal.load_asset(p) for p in paths]


def glb_material_names(path):
    with path.open('rb') as stream:
        header = stream.read(20)
        if header[:4] != b'glTF' or header[16:20] != b'JSON':
            raise RuntimeError('Unsupported GLB header')
        doc = json.loads(stream.read(struct.unpack_from('<I', header, 12)[0]))
    return [m['name'] for m in doc.get('materials', [])]


try:
    selection = json.loads(SELECTION.read_text(encoding='utf-8'))
    report['source_selection_sha256'] = hashlib.sha256(SELECTION.read_bytes()).hexdigest()
    report['source_inventory_sha256'] = selection['source_inventory_sha256']
    report['anchor'] = selection['anchor']
    json_files = {}
    for path in SOURCE.rglob('*.json'):
        if path.stem in json_files:
            raise RuntimeError('Ambiguous material basename: ' + path.stem)
        json_files[path.stem] = path
    texture_cache, material_cache, mesh_cache = {}, {}, {}
    resolved = json.loads(BINDINGS.read_text(encoding='utf-8'))
    if resolved.get('result') != 'PASS':
        raise RuntimeError('Inherited material bindings are not fully resolved')
    material_bindings = {x['source_material']: x for x in resolved['materials']}
    report['binding_manifest'] = str(BINDINGS)
    report['binding_manifest_sha256'] = hashlib.sha256(BINDINGS.read_bytes()).hexdigest()

    def texture(binding, normal=False):
        source = Path(binding['png']) if 'png' in binding else SOURCE / (binding['ObjectPath'].rsplit('.', 1)[0] + '.png')
        if not source.is_file():
            return None
        key = str(source)
        if key not in texture_cache:
            folder = DEST + '/Textures/' + hashlib.sha256(key.encode()).hexdigest()[:10]
            objects = import_file(source, folder)
            obj = next(o for o in objects if isinstance(o, unreal.Texture2D))
            obj.set_editor_property('srgb', not normal)
            if normal:
                obj.set_editor_property('compression_settings', unreal.TextureCompressionSettings.TC_NORMALMAP)
            unreal.EditorAssetLibrary.save_loaded_asset(obj, only_if_is_dirty=False)
            texture_cache[key] = obj
        return texture_cache[key]

    def material(name):
        if name in material_cache:
            return material_cache[name]
        path = json_files.get(name)
        if name.startswith('/'):
            inherited = material_bindings.get(name)
        else:
            candidates = [x for x in resolved['materials']
                          if x['source_material'].startswith('/Game/') and x['source_material'].rsplit('.',1)[-1] == name]
            if len(candidates) != 1:
                raise RuntimeError('Ambiguous or absent GLB material: ' + name)
            inherited = candidates[0]
        if inherited is None:
            raise RuntimeError('Missing evidenced inherited material: ' + name)
        bindings = inherited['resolved_textures']
        albedo_binding = bindings.get('albedo')
        normal_binding = bindings.get('normal')
        albedo = texture(albedo_binding) if albedo_binding else None
        normal = texture(normal_binding, True) if normal_binding else None
        if not albedo:
            raise RuntimeError('Missing original albedo binding: ' + name)
        obj = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
            'M_Sector_' + name.rsplit('.',1)[-1] + '_' + hashlib.sha256(name.encode()).hexdigest()[:8], DEST + '/Materials', unreal.Material, unreal.MaterialFactoryNew())
        if not obj:
            raise RuntimeError('Destination material already exists; use a clean sector import folder')
        for tex, prop, is_normal in ((albedo, unreal.MaterialProperty.MP_BASE_COLOR, False),
                                      (normal, unreal.MaterialProperty.MP_NORMAL, True)):
            if tex:
                node = unreal.MaterialEditingLibrary.create_material_expression(obj, unreal.MaterialExpressionTextureSample)
                node.texture = tex
                if is_normal:
                    node.sampler_type = unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
                unreal.MaterialEditingLibrary.connect_material_property(node, 'RGB', prop)
        if not albedo:
            node = unreal.MaterialEditingLibrary.create_material_expression(obj, unreal.MaterialExpressionConstant3Vector)
            node.constant = unreal.LinearColor(.18, .18, .18, 1)
            unreal.MaterialEditingLibrary.connect_material_property(node, '', unreal.MaterialProperty.MP_BASE_COLOR)
        unreal.MaterialEditingLibrary.recompile_material(obj)
        unreal.EditorAssetLibrary.save_loaded_asset(obj, only_if_is_dirty=False)
        report['materials'].append(dict(source_name=name, json=str(path) if path else None,
            object=obj.get_path_name(), albedo=albedo.get_path_name() if albedo else None,
            normal=normal.get_path_name() if normal else None,
            parent_chain=inherited['parent_chain'],
            state='INHERITED_ALBEDO_BASELINE'))
        material_cache[name] = obj
        return obj

    for source_mesh in selection['mesh_paths']:
        name = source_mesh.rsplit('.', 1)[-1]
        matches = list(SOURCE.rglob(name + '.glb'))
        if len(matches) != 1:
            raise RuntimeError('Missing/ambiguous GLB: ' + name)
        glb = matches[0]
        imported = import_file(glb, DEST + '/Meshes/' + name)
        meshes = [o for o in imported if isinstance(o, unreal.StaticMesh)]
        if len(meshes) != 1:
            raise RuntimeError('Expected one mesh: ' + name)
        mesh = meshes[0]
        names = glb_material_names(glb)
        if len(names) != len(mesh.get_editor_property('static_materials')):
            raise RuntimeError('Material slot count mismatch: ' + name)
        for index, mat_name in enumerate(names):
            mesh.set_material(index, material(mat_name))
        unreal.EditorAssetLibrary.save_loaded_asset(mesh, only_if_is_dirty=False)
        extent = mesh.get_bounds().box_extent
        report['meshes'].append(dict(source=source_mesh, object=mesh.get_path_name(),
                                    slots=names, dimensions_cm=[getattr(extent, k)*2 for k in ('x','y','z')]))
        mesh_cache[source_mesh] = mesh

    subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not subsystem.new_level(MAP):
        raise RuntimeError('Could not create isolated sector map')
    actor_system = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for source in selection['actors']:
        loc, rot, scale = source['relative_location'], source['rotation'], source['scale']
        actor = actor_system.spawn_actor_from_class(unreal.StaticMeshActor,
            unreal.Vector(loc['x'], loc['y'], loc['z']),
            unreal.Rotator(pitch=rot['pitch'], yaw=rot['yaw'], roll=rot['roll']))
        actor.set_actor_label(source['label'])
        actor.set_actor_scale3d(unreal.Vector(scale['x'], scale['y'], scale['z']))
        component = actor.static_mesh_component
        component.set_static_mesh(mesh_cache[source['mesh_source']])
        for index, mat_path in enumerate(source['components'][0].get('materials', [])):
            if mat_path:
                component.set_material(index, material(mat_path))
        origin, extent = actor.get_actor_bounds(False)
        report['actors'].append(dict(source=source['path'], object=actor.get_path_name(),
            source_location=source['location'], location=loc, rotation=rot, scale=scale,
            bounds_origin={k:getattr(origin,k) for k in ('x','y','z')},
            bounds_extent={k:getattr(extent,k) for k in ('x','y','z')}))
    if not subsystem.save_current_level():
        raise RuntimeError('Could not save isolated sector map')
    report.update(result='PASS', map=MAP, actor_count=len(report['actors']), mesh_count=len(mesh_cache),
                  material_count=len(material_cache), texture_count=len(texture_cache),
                  albedo_complete=all(m['albedo'] for m in report['materials']))
except Exception as error:
    report['error'] = str(error)
finally:
    destination = OUT / 'Sector-Import58.json'
    destination.write_text(json.dumps(report, indent=2)+'\n', encoding='utf-8')
    unreal.log('TEMPEST_SECTOR_IMPORT58_RESULT='+report['result']+' report='+str(destination))
if report['result'] != 'PASS':
    raise RuntimeError(report.get('error', 'Sector import failed'))
