"""Import model-local UV0 albedo/ORM/emission atlases; no shared material edits.

Run only after the matching mesh importers and after all requested Blender bakes.
Runtime must preserve white Tint for M_RV7_Baked_* instead of faction retinting.
"""
import hashlib
import json
from pathlib import Path
import unreal

ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/Textures/ReferenceV7'
TEXTURES='/Game/HERO/ReferenceV7/Textures'
MATERIALS='/Game/HERO/ReferenceV7/Materials'
LIB=unreal.MaterialEditingLibrary
ASSETS=unreal.EditorAssetLibrary
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
metadata=json.loads((SOURCE/'metadata.json').read_text(encoding='utf-8'))
records=metadata['records']
assert records and len({r['key'] for r in records})==len(records)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


# Validate all source geometry and output textures before mutating any packages.
meshes={}
definitions={}
for record in records:
    assert record['validation']=='PASS' and record['source_unchanged']
    source=ROOT/record['source']
    assert digest(source)==record['source_sha256'],('Rebake changed source',source)
    assert digest(source.with_suffix('.fbx'))==record['source_fbx_sha256']
    mesh=unreal.load_asset('/Game/HERO/ReferenceV7/Meshes/'+record['name'])
    assert isinstance(mesh,unreal.StaticMesh),record['name']
    meshes[record['key']]=mesh
    for channel in ('Albedo','ORM','EmissionMask'):
        info=record['textures'][channel]
        assert digest(SOURCE/info['file'])==info['sha256'],info['file']
        assert info['srgb']==(channel=='Albedo')
    definition=unreal.load_asset('/Game/HERO/DataV4/D_'+record['faction']+'_'+record['role'])
    assert definition
    definitions[record['key']]=(definition,definition.get_editor_property('icon'),
                                  str(definition.get_editor_property('stats')),
                                  str(definition.get_editor_property('static_mesh')))

reports=[]
for record in records:
    key=record['key'];mesh=meshes[key];textures={}
    for channel,info in record['textures'].items():
        source=SOURCE/info['file']
        task=unreal.AssetImportTask();task.filename=str(source)
        task.destination_path=TEXTURES;task.destination_name=source.stem
        task.automated=True;task.replace_existing=True;task.save=True
        TOOLS.import_asset_tasks([task])
        texture=unreal.load_asset(TEXTURES+'/'+source.stem)
        assert isinstance(texture,unreal.Texture2D),source.stem
        texture.set_editor_property('srgb',channel=='Albedo')
        texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_DEFAULT
                                    if channel=='Albedo' else unreal.TextureCompressionSettings.TC_MASKS)
        texture.set_editor_property('max_texture_size',info['size'][0])
        texture.set_editor_property('never_stream',False)
        assert ASSETS.save_loaded_asset(texture,only_if_is_dirty=False)
        textures[channel]=texture

    name='M_RV7_Baked_'+key
    material=unreal.load_asset(MATERIALS+'/'+name)
    if not material:
        material=TOOLS.create_asset(name,MATERIALS,unreal.Material,unreal.MaterialFactoryNew())
    assert isinstance(material,unreal.Material),name
    LIB.delete_all_material_expressions(material)

    def node(kind,x,y):
        result=LIB.create_material_expression(material,kind,x,y)
        assert result,kind
        return result

    def link(start,output,end,input_name):
        assert LIB.connect_material_expressions(start,output,end,input_name), (key,output,input_name)

    def property_link(start,output,prop):
        assert LIB.connect_material_property(start,output,prop),(key,prop)

    uv=node(unreal.MaterialExpressionTextureCoordinate,-1100,0)
    uv.set_editor_property('coordinate_index',0)
    samples={}
    for index,(channel,texture) in enumerate(textures.items()):
        sample=node(unreal.MaterialExpressionTextureSampleParameter2D,-850,index*240)
        sample.set_editor_property('parameter_name',channel)
        sample.set_editor_property('texture',texture)
        sample.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_COLOR
                                   if channel=='Albedo' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
        link(uv,'',sample,'UVs');samples[channel]=sample
    tint=node(unreal.MaterialExpressionVectorParameter,-850,-240)
    tint.set_editor_property('parameter_name','Tint')
    tint.set_editor_property('default_value',unreal.LinearColor(1,1,1,1))
    base=node(unreal.MaterialExpressionMultiply,-450,-60)
    link(samples['Albedo'],'RGB',base,'A');link(tint,'',base,'B')
    property_link(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
    property_link(samples['ORM'],'R',unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
    property_link(samples['ORM'],'G',unreal.MaterialProperty.MP_ROUGHNESS)
    property_link(samples['ORM'],'B',unreal.MaterialProperty.MP_METALLIC)
    emissive=node(unreal.MaterialExpressionMultiply,-450,460)
    link(base,'',emissive,'A');link(samples['EmissionMask'],'R',emissive,'B')
    strength=node(unreal.MaterialExpressionMultiply,-200,460)
    strength.set_editor_property('const_b',2.0)
    link(emissive,'',strength,'A')
    property_link(strength,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    LIB.recompile_material(material)
    assert ASSETS.save_loaded_asset(material,only_if_is_dirty=False)
    # Preserve section/material slot names and order; only replace their saved
    # materials. All UVs already address the same whole-mesh UV0 atlas.
    slots=list(mesh.get_editor_property('static_materials'))
    mesh.modify()
    for index in range(len(slots)):mesh.set_material(index,material)
    assert ASSETS.save_loaded_asset(mesh,only_if_is_dirty=False)
    for slot in mesh.get_editor_property('static_materials'):
        assert slot.get_editor_property('material_interface')==material
    definition,old_icon,old_stats,old_mesh=definitions[key]
    assert definition.get_editor_property('icon')==old_icon
    assert str(definition.get_editor_property('stats'))==old_stats
    assert str(definition.get_editor_property('static_mesh'))==old_mesh
    reports.append({'key':key,'mesh':mesh.get_path_name(),'material':material.get_path_name(),
                    'slot_count':len(slots),'uv_channel':0,'Tint':[1,1,1,1],
                    'textures':{k:t.get_path_name() for k,t in textures.items()},
                    'source_sha256':record['source_sha256'],'definition_unchanged':True,
                    'emission_policy':'Only separately baked Neon/Lamp mask; no full surface emission'})
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/ReferenceSurfacesV7-Import.json').write_text(
    json.dumps({'kind':'Original mesh-local UV0 atlases','models':reports},indent=2)+'\n',encoding='utf-8')
unreal.log('REFERENCE_SURFACES_V7_IMPORT_PASS '+str(len(reports)))
