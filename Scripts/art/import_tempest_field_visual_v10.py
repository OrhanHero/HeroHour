"""Root-owned serial Unreal import. Original forms only; no map or economy edits."""
from pathlib import Path
import hashlib,json
import unreal

ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/Models/TempestFieldV10'
DEST='/Game/HERO/ReferenceV10/TempestField'
manifest=json.loads((SOURCE/'manifest.json').read_text(encoding='utf-8'))
tools=unreal.AssetToolsHelpers.get_asset_tools()
lib=unreal.MaterialEditingLibrary
assets=unreal.EditorAssetLibrary
for record in manifest['records']:
    filename=SOURCE/(record['name']+'.fbx')
    assert hashlib.sha256(filename.read_bytes()).hexdigest()==record['fbx_sha256']
    task=unreal.AssetImportTask()
    task.set_editor_property('filename',str(filename))
    task.set_editor_property('destination_path',DEST)
    task.set_editor_property('automated',True)
    task.set_editor_property('replace_existing',True)
    task.set_editor_property('save',True)
    options=unreal.FbxImportUI()
    options.set_editor_property('import_mesh',True)
    options.set_editor_property('import_materials',False)
    options.set_editor_property('import_textures',False)
    options.set_editor_property('import_as_skeletal',False)
    options.static_mesh_import_data.set_editor_property('auto_generate_collision',False)
    options.static_mesh_import_data.set_editor_property('combine_meshes',True)
    task.set_editor_property('options',options)
    tools.import_asset_tasks([task])
    mesh=unreal.load_asset(DEST+'/'+record['name'])
    assert isinstance(mesh,unreal.StaticMesh)

def material(name,pod):
    path=DEST+'/'+name
    mat=unreal.load_asset(path) or tools.create_asset(name,DEST,unreal.Material,unreal.MaterialFactoryNew())
    lib.delete_all_material_expressions(mat)
    def node(kind):return lib.create_material_expression(mat,kind)
    def constant(value):
        n=node(unreal.MaterialExpressionConstant);n.set_editor_property('r',value);return n
    def vector(rgb):
        n=node(unreal.MaterialExpressionConstant3Vector)
        n.set_editor_property('constant',unreal.LinearColor(*rgb,1));return n
    def connect(a,b,input_name,output=''):
        assert lib.connect_material_expressions(a,output,b,input_name),(
            a.get_class().get_name(),b.get_class().get_name(),input_name,
            list(lib.get_material_expression_input_names(b)))
    activity=node(unreal.MaterialExpressionPerInstanceCustomData)
    activity.set_editor_property('data_index',0)
    visibility=node(unreal.MaterialExpressionPerInstanceCustomData)
    visibility.set_editor_property('data_index',1)
    # Interpolators make instance custom data available to pixel shading on UE ISM.
    # UE5.8 MaterialExpressionVertexInterpolator.h:GetInputName returns "VS",
    # although its reflected FExpressionInput property is called Input.
    # MaterialEditingLibrary connects graph pin names, not reflected property names.
    def interpolate(source):
        expression=node(unreal.MaterialExpressionVertexInterpolator)
        names=list(lib.get_material_expression_input_names(expression))
        assert names==['VS'],('Unexpected VertexInterpolator pins',names)
        connect(source,expression,'VS')
        return expression
    activity_pixel=interpolate(activity)
    visibility_pixel=interpolate(visibility)
    active=node(unreal.MaterialExpressionMultiply)
    connect(activity_pixel,active,'A');connect(visibility_pixel,active,'B')
    color=node(unreal.MaterialExpressionLinearInterpolate)
    connect(vector((.065,.033,.014)),color,'A')
    connect(vector((.36,.14,.025) if pod else (.16,.075,.022)),color,'B')
    connect(activity_pixel,color,'Alpha')
    assert lib.connect_material_property(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
    emission=node(unreal.MaterialExpressionMultiply)
    connect(active,emission,'A');connect(vector((.38,.075,.004) if pod else (.025,.006,.001)),emission,'B')
    assert lib.connect_material_property(emission,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    assert lib.connect_material_property(constant(.82),'',unreal.MaterialProperty.MP_ROUGHNESS)
    assert lib.connect_material_property(constant(0),'',unreal.MaterialProperty.MP_METALLIC)
    lib.recompile_material(mat);assets.save_loaded_asset(mat)
    return mat

root=material('M_TFV10_OrganicRoot',False)
pod=material('M_TFV10_SeedPod',True)
for mesh_name,mat in [('SM_TFV10_OrganicRootSegment',root),('SM_TFV10_SeedPod',pod)]:
    mesh=unreal.load_asset(DEST+'/'+mesh_name)
    mesh.set_material(0,mat);assets.save_loaded_asset(mesh)
report={'result':'IMPORTED','destination':DEST,'map_modified':False,
        'collision_import_generation':False,'visual_acceptance':False,
        'note':'NoCollision/noNav enforced by presenter. Root must test real material compile and instance output.'}
(ROOT/'Saved/Research').mkdir(exist_ok=True)
(ROOT/'Saved/Research/TempestFieldV10-Import.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print('TEMPEST_FIELD_V10_IMPORT_PASS',json.dumps(report))
