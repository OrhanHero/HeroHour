"""Use authored PBR micro/macro texture scales instead of procedural 3D noise.

Run serially in Unreal Python after the V6 terrain/city import. Does not change
geometry, collision or gameplay coordinates. Existing V5 assets remain intact.
"""
from pathlib import Path
import json
import unreal

ROOT=Path(__file__).resolve().parents[2]
DEST='/Game/HERO/PresentationV6/Materials'
LIB=unreal.MaterialEditingLibrary
ASSETS=unreal.EditorAssetLibrary
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()

def node(m,kind): return LIB.create_material_expression(m,kind,0,0)
def link(a,out,b,pin): assert LIB.connect_material_expressions(a,out,b,pin)
def scalar(m,value):
    n=node(m,unreal.MaterialExpressionConstant);n.set_editor_property('r',value);return n
def color(m,value):
    n=node(m,unreal.MaterialExpressionConstant3Vector)
    n.set_editor_property('constant',unreal.LinearColor(*value,1));return n
def op(m,kind,a,b):
    n=node(m,kind);link(a,'',n,'A');link(b,'',n,'B');return n
def sample(m,texture,uv,normal=False,mask=False):
    n=node(m,unreal.MaterialExpressionTextureSample);n.set_editor_property('texture',texture)
    n.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if normal else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS if mask else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
    link(uv,'',n,'UVs');return n

def make_surface(name,source,tile,dark,light):
    textures={role:unreal.load_asset('/Game/HERO/DistrictV3/Textures/T_DV3_'+source+'_'+role)
              for role in ['Albedo','Normal','Roughness']}
    assert all(textures.values()),source
    m=unreal.load_asset(DEST+'/'+name)
    if not m:m=TOOLS.create_asset(name,DEST,unreal.Material,unreal.MaterialFactoryNew())
    LIB.delete_all_material_expressions(m)
    position=node(m,unreal.MaterialExpressionWorldPosition)
    xy=node(m,unreal.MaterialExpressionComponentMask)
    xy.set_editor_property('r',True);xy.set_editor_property('g',True);link(position,'',xy,'')
    uv=op(m,unreal.MaterialExpressionDivide,xy,scalar(m,tile))
    macro_uv=op(m,unreal.MaterialExpressionDivide,xy,scalar(m,tile*11.3))
    base=sample(m,textures['Albedo'],uv)
    macro=sample(m,textures['Albedo'],macro_uv)
    # Albedo red is a stable bounded mask. The macro lookup is independent of
    # the fine grain, with an inexpensive texture lookup rather than 3D Noise.
    tint=node(m,unreal.MaterialExpressionLinearInterpolate)
    link(color(m,dark),'',tint,'A');link(color(m,light),'',tint,'B');link(macro,'R',tint,'Alpha')
    albedo=op(m,unreal.MaterialExpressionMultiply,base,tint)
    normal=sample(m,textures['Normal'],uv,normal=True)
    rough=sample(m,textures['Roughness'],uv,mask=True)
    rough_range=node(m,unreal.MaterialExpressionMultiply);rough_range.set_editor_property('const_b',.25)
    link(rough,'R',rough_range,'A')
    rough_out=node(m,unreal.MaterialExpressionAdd);rough_out.set_editor_property('const_b',.72)
    link(rough_range,'',rough_out,'A')
    LIB.connect_material_property(albedo,'',unreal.MaterialProperty.MP_BASE_COLOR)
    LIB.connect_material_property(normal,'RGB',unreal.MaterialProperty.MP_NORMAL)
    LIB.connect_material_property(rough_out,'',unreal.MaterialProperty.MP_ROUGHNESS)
    LIB.recompile_material(m);ASSETS.save_loaded_asset(m,only_if_is_dirty=False)
    return m

ground=make_surface('M_PV6_DesertGround','Sand',240.,(.70,.53,.33),(1.15,1.04,.84))
road=make_surface('M_PV6_WeatheredAsphalt','Asphalt',180.,(.72,.75,.70),(1.18,1.17,1.07))
level=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert level.load_level('/Game/Maps/CityHero')
changed=[]
for actor in actors.get_all_level_actors():
    if not isinstance(actor,unreal.StaticMeshActor):continue
    component=actor.static_mesh_component
    for index in range(component.get_num_materials()):
        old=component.get_material(index)
        if not old:continue
        if actor.get_actor_label()=='CITYHERO Desert terrain' and index==0:
            component.modify();component.set_material(index,ground)
            changed.append([actor.get_actor_label(),index,ground.get_path_name()])
        elif old.get_name() in ('M_PV5_WeatheredAsphalt','M_PV6_WeatheredAsphalt'):
            component.modify();component.set_material(index,road)
            changed.append([actor.get_actor_label(),index,road.get_path_name()])
assert len(changed)>1,'Expected terrain plus road surfaces'
assert level.save_current_level()
report=dict(result='PASS',materials=[ground.get_path_name(),road.get_path_name()],
            procedural_noise_nodes=0,texture_samples_per_material=4,changed_components=changed,
            performance='Reduced shader work by construction; no measured FPS claim before runtime comparison.')
(ROOT/'Saved/Research/WorldSurfacesV6.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
unreal.log('WORLD_SURFACES_V6_PASS '+str(len(changed)))
