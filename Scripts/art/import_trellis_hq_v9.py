"""Import prepared HQ9 mesh/PBR only. Never updates definitions or maps."""
import hashlib,json
from pathlib import Path
import unreal
ROOT=Path(__file__).resolve().parents[2];SOURCE=ROOT/'Art/Models/TrellisHQV9';NAME='SM_HQ9_AegisHQ'
META=json.loads((SOURCE/(NAME+'.json')).read_text(encoding='utf-8'))
DEST='/Game/HERO/TrellisHQV9';TOOLS=unreal.AssetToolsHelpers.get_asset_tools();ASSETS=unreal.EditorAssetLibrary;LIB=unreal.MaterialEditingLibrary
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert META['fbx_roundtrip_pass'] and META['source_unchanged'] and META['custom_normals_preserved']
assert sha(ROOT/META['source'])==META['source_sha256'] and sha(SOURCE/(NAME+'.fbx'))==META['fbx_sha256']
assert abs(META['zoffset_m']+1.1)<.0001 and max(META['dimensions_m'][:2])*150<=1200
definitions={}
for name in ['D_Aegis_HQ','D_Ember_HQ']:
 obj=unreal.load_asset('/Game/HERO/DataV4/'+name);assert obj
 definitions[name]=(str(obj.get_editor_property('static_mesh')),str(obj.get_editor_property('icon')),str(obj.get_editor_property('stats')))
textures={}
for semantic,info in META['textures'].items():
 path=ROOT/info['file'];assert sha(path)==info['sha256']
 task=unreal.AssetImportTask();task.filename=str(path);task.destination_path=DEST+'/Textures';task.destination_name=path.stem
 task.automated=True;task.replace_existing=True;task.save=True;TOOLS.import_asset_tasks([task])
 texture=unreal.load_asset(task.destination_path+'/'+path.stem);assert isinstance(texture,unreal.Texture2D)
 texture.set_editor_property('srgb',info['srgb']);texture.set_editor_property('compression_settings',unreal.TextureCompressionSettings.TC_DEFAULT if info['srgb'] else unreal.TextureCompressionSettings.TC_MASKS)
 texture.set_editor_property('max_texture_size',2048);texture.set_editor_property('never_stream',False)
 assert ASSETS.save_loaded_asset(texture,only_if_is_dirty=False);assert texture.get_editor_property('srgb')==info['srgb']
 assert int(texture.blueprint_get_size_x())==2048 and int(texture.blueprint_get_size_y())==2048
 textures[semantic]=texture
material_name='M_HQ9_Baked_AegisHQ';material=unreal.load_asset(DEST+'/Materials/'+material_name)
if not material:material=TOOLS.create_asset(material_name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
assert isinstance(material,unreal.Material);LIB.delete_all_material_expressions(material)
def node(kind,x,y):
 result=LIB.create_material_expression(material,kind,x,y);assert result;return result
def link(a,out,b,key):assert LIB.connect_material_expressions(a,out,b,key),(out,key)
def prop(a,out,key):assert LIB.connect_material_property(a,out,key),key
uv=node(unreal.MaterialExpressionTextureCoordinate,-1000,0);uv.set_editor_property('coordinate_index',0)
samples={}
for i,(semantic,texture) in enumerate(textures.items()):
 sample=node(unreal.MaterialExpressionTextureSampleParameter2D,-800,i*250);sample.set_editor_property('parameter_name',semantic);sample.set_editor_property('texture',texture)
 sample.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_COLOR if semantic=='Albedo' else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
 link(uv,'',sample,'UVs');samples[semantic]=sample
tint=node(unreal.MaterialExpressionVectorParameter,-800,-240);tint.set_editor_property('parameter_name','Tint');tint.set_editor_property('default_value',unreal.LinearColor(1,1,1,1))
multiply=node(unreal.MaterialExpressionMultiply,-400,0);link(samples['Albedo'],'RGB',multiply,'A');link(tint,'',multiply,'B');prop(multiply,'',unreal.MaterialProperty.MP_BASE_COLOR)
prop(samples['MetallicRoughness'],'G',unreal.MaterialProperty.MP_ROUGHNESS);prop(samples['MetallicRoughness'],'B',unreal.MaterialProperty.MP_METALLIC)
ao=node(unreal.MaterialExpressionConstant,-400,500);ao.set_editor_property('r',1.0);prop(ao,'',unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
LIB.recompile_material(material);assert ASSETS.save_loaded_asset(material,only_if_is_dirty=False)
task=unreal.AssetImportTask();task.filename=str(SOURCE/(NAME+'.fbx'));task.destination_path=DEST+'/Meshes';task.destination_name=NAME
task.automated=True;task.replace_existing=True;task.save=True
options=unreal.FbxImportUI();options.import_mesh=True;options.import_as_skeletal=False;options.import_materials=False;options.import_textures=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False;options.static_mesh_import_data.generate_lightmap_u_vs=False
options.static_mesh_import_data.normal_import_method=unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS
task.options=options;TOOLS.import_asset_tasks([task]);mesh=unreal.load_asset(task.destination_path+'/'+NAME);assert isinstance(mesh,unreal.StaticMesh)
slots=list(mesh.get_editor_property('static_materials'));assert len(slots)==1
assert str(slots[0].get_editor_property('imported_material_slot_name')).split('.')[0]=='Team'
mesh.modify();mesh.set_material(0,material)
nanite=mesh.get_editor_property('nanite_settings');nanite.set_editor_property('enabled',True);mesh.set_editor_property('nanite_settings',nanite)
bounds=mesh.get_bounding_box();actual=[[bounds.min.x,bounds.max.x],[bounds.min.y,bounds.max.y],[bounds.min.z,bounds.max.z]]
assert all(abs(a-b)<1 for pair,expected in zip(actual,META['unreal_expected_bounds_cm']) for a,b in zip(pair,expected)),(actual,META['unreal_expected_bounds_cm'])
assert abs(bounds.min.z+110)<1 and max(bounds.max.x-bounds.min.x,bounds.max.y-bounds.min.y)*1.5<=1200
assert ASSETS.save_loaded_asset(mesh,only_if_is_dirty=False)
assert mesh.get_editor_property('static_materials')[0].get_editor_property('material_interface')==material
assert mesh.get_editor_property('nanite_settings').get_editor_property('enabled')
for name,before in definitions.items():
 obj=unreal.load_asset('/Game/HERO/DataV4/'+name);assert (str(obj.get_editor_property('static_mesh')),str(obj.get_editor_property('icon')),str(obj.get_editor_property('stats')))==before
report={'mesh':mesh.get_path_name(),'material':material.get_path_name(),'textures':{k:v.get_path_name() for k,v in textures.items()},'bounds_cm':actual,'source_sha256':META['source_sha256'],'fbx_sha256':META['fbx_sha256'],'slot':'Team','tint':[1,1,1,1],'MR_channels':{'roughness':'G','metallic':'B','AO':'constant1'},'emission':'none','nanite_enabled':True,'runtime_ground_z_cm':110+55+bounds.min.z*1.5,'definitions_unchanged':len(definitions),'maps_touched':False,'normal_import':'import authored normals and tangents; no geometric reshaping'}
(ROOT/'Saved/Research/TrellisHQV9-Import.json').write_text(json.dumps(report,indent=2),encoding='utf-8');unreal.log('TRELLIS_HQ_V9_IMPORT_PASS '+json.dumps(report))
