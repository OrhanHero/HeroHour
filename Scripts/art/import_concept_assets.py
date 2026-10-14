"""Import verified original ConceptV2 models without modifying the legacy kit."""
from pathlib import Path
import json, unreal

ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/Models/ConceptV2'
DEST='/Game/HERO/ConceptV2'
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
LIB=unreal.MaterialEditingLibrary
PALETTE={
    'Armor':((.105,.125,.135),.72,.52), 'Steel':((.24,.27,.28),.8,.42),
    'Dark':((.026,.035,.039),.45,.66), 'Concrete':((.31,.30,.26),0,.88),
    'Dust':((.43,.37,.25),.15,.76), 'Rubber':((.018,.022,.024),0,.85),
    'Glass':((.02,.09,.09),.55,.24), 'Team':((.035,.24,.15),.55,.48),
    'Neon':((.035,.6,.30),.25,.30), 'Lamp':((.8,.45,.12),.1,.36),
}

def expr(material,kind,x,y): return LIB.create_material_expression(material,kind,x,y)
def scalar(material,value,x,y):
    n=expr(material,unreal.MaterialExpressionConstant,x,y); n.set_editor_property('r',value); return n
def material(name,color,metal,rough):
    path=DEST+'/Materials/M_CV2_'+name
    m=unreal.load_asset(path)
    if m: return m
    m=TOOLS.create_asset('M_CV2_'+name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    assert m, path
    tint=expr(m,unreal.MaterialExpressionVectorParameter,-800,0)
    tint.set_editor_property('parameter_name','Tint'); tint.set_editor_property('default_value',unreal.LinearColor(*color,1))
    base=tint
    if name not in ('Neon','Lamp','Glass'):
        noise=expr(m,unreal.MaterialExpressionNoise,-800,-240)
        noise.set_editor_property('scale',.065); noise.set_editor_property('quality',1)
        dark=expr(m,unreal.MaterialExpressionMultiply,-560,80);dark.set_editor_property('const_b',.72)
        LIB.connect_material_expressions(tint,'',dark,'A')
        mix=expr(m,unreal.MaterialExpressionLinearInterpolate,-270,0)
        LIB.connect_material_expressions(dark,'',mix,'A');LIB.connect_material_expressions(tint,'',mix,'B')
        LIB.connect_material_expressions(noise,'',mix,'Alpha');base=mix
        surface=expr(m,unreal.MaterialExpressionLinearInterpolate,-260,250)
        surface.set_editor_property('const_a',max(.1,rough-.1));surface.set_editor_property('const_b',min(1,rough+.1))
        LIB.connect_material_expressions(noise,'',surface,'Alpha');LIB.connect_material_property(surface,'',unreal.MaterialProperty.MP_ROUGHNESS)
    else: LIB.connect_material_property(scalar(m,rough,-400,250),'',unreal.MaterialProperty.MP_ROUGHNESS)
    LIB.connect_material_property(base,'',unreal.MaterialProperty.MP_BASE_COLOR)
    LIB.connect_material_property(scalar(m,metal,-400,400),'',unreal.MaterialProperty.MP_METALLIC)
    if name in ('Neon','Lamp'):
        emission=expr(m,unreal.MaterialExpressionMultiply,-260,-150)
        emission.set_editor_property('const_b',2.0 if name=='Neon' else 1.0)
        LIB.connect_material_expressions(tint,'',emission,'A');LIB.connect_material_property(emission,'',unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    LIB.recompile_material(m);unreal.EditorAssetLibrary.save_loaded_asset(m)
    return m

mats={key:material(key,*values) for key,values in PALETTE.items()}
names=['SM_HERO_CommandHub','SM_HERO_VehiclePlant','SM_HERO_Refinery','SM_HERO_Striker','SM_HERO_Harvester']
records=[]
for name in names:
    source=SOURCE/(name+'.fbx');assert source.is_file(),str(source)
    task=unreal.AssetImportTask();task.filename=str(source);task.destination_path=DEST+'/Meshes'
    task.destination_name=name;task.automated=True;task.replace_existing=True;task.save=True
    settings=unreal.FbxImportUI();settings.import_mesh=True;settings.import_as_skeletal=False
    settings.import_materials=False;settings.import_textures=False;settings.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    settings.static_mesh_import_data.combine_meshes=True;settings.static_mesh_import_data.auto_generate_collision=True
    task.options=settings;TOOLS.import_asset_tasks([task])
    mesh=unreal.load_asset(DEST+'/Meshes/'+name);assert mesh,name
    slots=mesh.get_editor_property('static_materials')
    for index,slot in enumerate(slots):
        key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0]
        assert key in mats, (name,index,key)
        mesh.set_material(index,mats[key])
    bounds=mesh.get_bounding_box();size=bounds.max-bounds.min
    # Verify metre-source to centimetre-Unreal conversion rather than assuming import settings.
    expected=json.loads((SOURCE/(name+'.json')).read_text(encoding='utf-8'))['dimensions_m']
    actual=[size.x,size.y,size.z]
    for measured,meters in zip(actual,expected):
        assert abs(measured-meters*100)<max(3,meters*3),(name,actual,expected)
    unreal.EditorAssetLibrary.save_loaded_asset(mesh)
    records.append({'name':name,'unreal_path':mesh.get_path_name(),'bounds_cm':actual,
        'material_slots':len(slots),'source_fbx':str(source)})
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/concept-assets-import.json').write_text(json.dumps(records,indent=2),encoding='utf-8')
unreal.log('CONCEPT_V2_IMPORT_PASS '+json.dumps(records))
