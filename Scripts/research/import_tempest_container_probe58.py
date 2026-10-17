"""Isolated UE5.8 import test; writes only the disposable transfer project."""
import json
from pathlib import Path
import unreal

ROOT=Path(__file__).resolve().parents[2]
PROJECT=ROOT/'Saved/Tests/TempestTransferProbe58'
if Path(unreal.Paths.project_dir()).resolve()!=PROJECT.resolve():
    raise RuntimeError('Refusing imports outside the owned isolated test project')
SOURCE=ROOT/'Saved/Research/TempestAssetTransfer/Container-Probe-02/Exports/Tempest/Content/Tempest/Props/Faction_Containers'
DEST='/Game/TransferProbe'
OUT=ROOT/'Saved/Research/TempestAssetTransfer/Unreal58'
OUT.mkdir(parents=True,exist_ok=True)
report=dict(schema='HeroHour.TempestContainerImport58.v1',engine=unreal.SystemLibrary.get_engine_version(),
            project=str(PROJECT),source=str(SOURCE),result='FAIL',imports=[],
            limitation='Isolated geometry/texture import and original-albedo baseline; not the original complete Tempest material graph.')
try:
    def import_file(path,folder):
        task=unreal.AssetImportTask()
        task.filename=str(path)
        task.destination_path=folder
        task.automated=True
        task.save=True
        task.replace_existing=False
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        paths=list(task.imported_object_paths)
        report['imports'].append(dict(source=str(path),objects=paths))
        if not paths:
            raise RuntimeError('No imported assets: '+str(path))
        return [unreal.load_asset(path) for path in paths]
    objects=import_file(SOURCE/'SM_GDF_Container.glb',DEST+'/Mesh')
    meshes=[obj for obj in objects if isinstance(obj,unreal.StaticMesh)]
    if len(meshes)!=1:
        raise RuntimeError('Expected exactly one imported mesh')
    mesh=meshes[0]
    textures={}
    for suffix in ('D','N','RMO','C'):
        items=import_file(SOURCE/'Textures/GDF'/('T_GDF_Container_'+suffix+'.png'),DEST+'/Textures')
        textures[suffix]=next(obj for obj in items if isinstance(obj,unreal.Texture2D))
    textures['D'].set_editor_property('srgb',True)
    textures['N'].set_editor_property('srgb',False)
    textures['N'].set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_NORMALMAP)
    textures['RMO'].set_editor_property('srgb',False)
    material=unreal.AssetToolsHelpers.get_asset_tools().create_asset('M_Container_OriginalAlbedoBaseline',DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError('Material was not created in fresh project')
    for suffix,prop in [('D',unreal.MaterialProperty.MP_BASE_COLOR),('N',unreal.MaterialProperty.MP_NORMAL)]:
        sample=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionTextureSample)
        sample.texture=textures[suffix]
        if suffix=='N':sample.sampler_type=unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
        unreal.MaterialEditingLibrary.connect_material_property(sample,'RGB',prop)
    mesh.set_material(0,material)
    bounds=mesh.get_bounds()
    dimensions=[float(getattr(bounds.box_extent,axis))*2 for axis in ('x','y','z')]
    report.update(mesh=mesh.get_path_name(),dimensions_cm=dimensions,
                  material_slots=len(mesh.get_editor_property('static_materials')),
                  material=material.get_path_name(),textures={k:v.get_path_name() for k,v in textures.items()})
    expected=[701.230224609375,309.35581970214844,299.1515827178955]
    report['dimensions_match_source']=all(abs(a-b)<.1 for a,b in zip(dimensions,expected))
    if not report['dimensions_match_source'] or report['material_slots']!=1:
        raise RuntimeError('Imported dimensions or material-slot count differs from source')
    for obj in [mesh,material,*textures.values()]:
        if not unreal.EditorAssetLibrary.save_loaded_asset(obj,only_if_is_dirty=False):
            raise RuntimeError('Failed to save isolated imported asset '+obj.get_path_name())
    report['result']='PASS'
except Exception as error:
    report['error']=str(error)
finally:
    destination=OUT/'Container-Import58.json'
    destination.write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    unreal.log('TEMPEST_CONTAINER_IMPORT58_RESULT='+report['result']+' report='+str(destination))
if report['result']!='PASS':raise RuntimeError(report.get('error','Import failed'))
