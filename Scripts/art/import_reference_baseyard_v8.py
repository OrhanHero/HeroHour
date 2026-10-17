"""Prepared V8 baseyard integration; Root executes serially through Unreal Python.

Imports two merged decorative meshes and six shared local PBR instances.
Only BYV8 actors are replaced. Existing roads, structures and C++ are untouched.
"""
import hashlib
import json
from pathlib import Path
import unreal

ROOT=Path(__file__).resolve().parents[2]
SOURCE=ROOT/'Art/Models/ReferenceBaseyardV8'
DEST='/Game/HERO/ReferenceV8/Baseyard'
TAG='HERO_BaseyardV8'
manifest=json.loads((SOURCE/'manifest.json').read_text(encoding='utf-8'))
assert len(manifest['assets'])==2 and len(manifest['materials'])==6
TOOLS=unreal.AssetToolsHelpers.get_asset_tools()
ASSETS=unreal.EditorAssetLibrary
LIB=unreal.MaterialEditingLibrary
LEVEL=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ACTORS=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
STATIC=unreal.get_editor_subsystem(unreal.StaticMeshEditorSubsystem)
# StaticMeshEditorSubsystem is optional in UnrealEditor-Cmd/NullRHI. The old
# EditorStaticMeshLibrary also delegates to it, so it is not a valid fallback.

# Resolve source evidence and all external textures before creating materials.
textures={}
for source in sorted({v['source'] for v in manifest['materials'].values()}|{'Sand'}):
    textures[source]={}
    for channel in ('Albedo','Normal','Roughness'):
        path='/Game/HERO/DistrictV3/Textures/T_DV3_'+source+'_'+channel
        texture=unreal.load_asset(path)
        assert isinstance(texture,unreal.Texture2D),path
        assert texture.get_editor_property('srgb')==(channel=='Albedo'),path
        textures[source][channel]=texture
for meta in manifest['assets']:
    assert meta['validation']=='PASS' and meta['fbx_roundtrip_pass']
    assert meta['maximum_relative_relief_cm']<=3.001
    assert meta['non_manifold_edges']==meta['degenerate_faces']==0
    assert hashlib.sha256((SOURCE/(meta['name']+'.fbx')).read_bytes()).hexdigest()==meta['fbx_sha256']
assert LEVEL.load_level('/Game/Maps/CityHero')
existing=ACTORS.get_all_level_actors()
for x in (-8000.,8000.):
    assert any(isinstance(a,unreal.StaticMeshActor) and a.static_mesh_component.static_mesh
               and 'SM_DV3_BaseApron' in a.static_mesh_component.static_mesh.get_name()
               and abs(a.get_actor_location().x-x)<1 and abs(a.get_actor_location().y)<1
               for a in existing),'Existing HQ apron moved or removed: '+str(x)


def ours(actor):
    return actor.get_actor_label().startswith('BYV8 ') or TAG in [str(t) for t in actor.tags]


def snapshot(actor):
    p=actor.get_actor_location();r=actor.get_actor_rotation();s=actor.get_actor_scale3d()
    value=dict(location=[p.x,p.y,p.z],rotation=[r.pitch,r.yaw,r.roll],scale=[s.x,s.y,s.z])
    if isinstance(actor,unreal.StaticMeshActor):
        component=actor.static_mesh_component
        value.update(mesh=component.static_mesh.get_path_name() if component.static_mesh else None,
                     collision=str(component.get_collision_enabled()),
                     materials=[component.get_material(i).get_path_name() if component.get_material(i) else None
                                for i in range(component.get_num_materials())])
    return value


def collision_inventory(mesh):
    """Read the native collision aggregate without an editor subsystem.

    UE5.8 StaticMesh.h exposes BodySetup as an editor property; BodySetup.h
    exposes AggGeom. AggregateGeom.h lists all nine collision shape arrays.
    No BodySetup means zero shapes, matching the native editor implementation.
    Unknown/unreadable properties raise instead of silently accepting a mesh.
    """
    def reflected(owner,python_name,cpp_name):
        try:return owner.get_editor_property(python_name)
        except Exception as python_error:
            try:return owner.get_editor_property(cpp_name)
            except Exception as cpp_error:
                raise RuntimeError('Cannot inspect collision property '+cpp_name+
                                   ': '+str(python_error)+'; '+str(cpp_error)) from cpp_error
    body=reflected(mesh,'body_setup','BodySetup')
    arrays=(('sphere_elems','SphereElems'),('box_elems','BoxElems'),
            ('sphyl_elems','SphylElems'),('convex_elems','ConvexElems'),
            ('tapered_capsule_elems','TaperedCapsuleElems'),('level_set_elems','LevelSetElems'),
            ('skinned_level_set_elems','SkinnedLevelSetElems'),('ml_level_set_elems','MLLevelSetElems'),
            ('skinned_triangle_mesh_elems','SkinnedTriangleMeshElems'))
    if body is None:
        return dict(body_setup_present=False,shapes={cpp:0 for _,cpp in arrays},total=0)
    aggregate=reflected(body,'agg_geom','AggGeom')
    counts={cpp:len(reflected(aggregate,python,cpp)) for python,cpp in arrays}
    return dict(body_setup_present=True,shapes=counts,total=sum(counts.values()))


def ensure_no_simple_collision(mesh):
    before=collision_inventory(mesh)
    cleaned=False
    if before['total']:
        # Interactive editor runs can repair an old asset through the native
        # cleanup API. In a commandlet, fail before changing or saving the map
        # rather than editing AggGeom without invalidating cooked physics data.
        assert STATIC is not None,('Unexpected collision on imported decorative mesh; '
            'auto_generate_collision=False did not produce an empty aggregate',mesh.get_name(),before)
        assert STATIC.remove_collisions(mesh),('Native collision cleanup failed',mesh.get_name())
        cleaned=True
    after=collision_inventory(mesh)
    assert after['total']==0,('Decorative mesh still has collision shapes',mesh.get_name(),after)
    return dict(mesh=mesh.get_name(),api='reflected UBodySetup.AggGeom',
                initial=before,final=after,editor_subsystem_cleanup_used=cleaned)


preserved={a.get_path_name():snapshot(a) for a in existing if not ours(a)}


def master(name,feather):
    path=DEST+'/Materials/'+name
    mat=unreal.load_asset(path)
    if not mat:mat=TOOLS.create_asset(name,DEST+'/Materials',unreal.Material,unreal.MaterialFactoryNew())
    assert isinstance(mat,unreal.Material)
    LIB.delete_all_material_expressions(mat)
    def node(kind):return LIB.create_material_expression(mat,kind,0,0)
    def link(a,out,b,into):assert LIB.connect_material_expressions(a,out,b,into)
    def prop(a,out,into):assert LIB.connect_material_property(a,out,into)
    def scalar(name,value):
        result=node(unreal.MaterialExpressionScalarParameter)
        result.set_editor_property('parameter_name',name);result.set_editor_property('default_value',value)
        return result
    def vector(name,value):
        result=node(unreal.MaterialExpressionVectorParameter)
        result.set_editor_property('parameter_name',name);result.set_editor_property('default_value',unreal.LinearColor(*value,1))
        return result
    def sample(name,texture,uv,normal=False,masks=False):
        result=node(unreal.MaterialExpressionTextureSampleParameter2D)
        result.set_editor_property('parameter_name',name);result.set_editor_property('texture',texture)
        result.set_editor_property('sampler_type',unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if normal else
                                   unreal.MaterialSamplerType.SAMPLERTYPE_MASKS if masks else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
        link(uv,'',result,'UVs');return result
    coord=node(unreal.MaterialExpressionTextureCoordinate)
    uv=node(unreal.MaterialExpressionDivide);link(coord,'',uv,'A');link(scalar('TileMeters',4.),'',uv,'B')
    albedo=sample('AlbedoTexture',textures['Concrete']['Albedo'],uv)
    normal=sample('NormalTexture',textures['Concrete']['Normal'],uv,normal=True)
    rough=sample('RoughnessTexture',textures['Concrete']['Roughness'],uv,masks=True)
    color=node(unreal.MaterialExpressionMultiply);link(albedo,'RGB',color,'A');link(vector('Tint',(1,1,1)),'',color,'B')
    roughness=node(unreal.MaterialExpressionMultiply);link(rough,'R',roughness,'A');link(scalar('RoughnessScale',1.),'',roughness,'B')
    final_normal=normal;normal_output='RGB'
    if feather:
        # Opaque PBR blend: vertex alpha approaches actual world-space sand at
        # the drift perimeter. No transparency, opacity mask or pixel offset.
        position=node(unreal.MaterialExpressionWorldPosition)
        xy=node(unreal.MaterialExpressionComponentMask)
        xy.set_editor_property('r',True);xy.set_editor_property('g',True);xy.set_editor_property('b',False)
        link(position,'',xy,'')
        ground_uv=node(unreal.MaterialExpressionDivide);link(xy,'',ground_uv,'A')
        divisor=node(unreal.MaterialExpressionConstant);divisor.set_editor_property('r',400.);link(divisor,'',ground_uv,'B')
        ground_color=sample('GroundAlbedoTexture',textures['Sand']['Albedo'],ground_uv)
        ground_normal=sample('GroundNormalTexture',textures['Sand']['Normal'],ground_uv,normal=True)
        ground_rough=sample('GroundRoughnessTexture',textures['Sand']['Roughness'],ground_uv,masks=True)
        ground_tint=node(unreal.MaterialExpressionMultiply)
        link(ground_color,'RGB',ground_tint,'A');link(vector('GroundTint',(1,.9,.78)),'',ground_tint,'B')
        vertex=node(unreal.MaterialExpressionVertexColor)
        def blend(a,aout,b,bout):
            lerp=node(unreal.MaterialExpressionLinearInterpolate)
            link(a,aout,lerp,'A');link(b,bout,lerp,'B');link(vertex,'A',lerp,'Alpha');return lerp
        color=blend(ground_tint,'',color,'')
        roughness=blend(ground_rough,'R',roughness,'')
        final_normal=blend(ground_normal,'RGB',normal,'RGB');normal_output=''
    base_normal=node(unreal.MaterialExpressionConstant3Vector)
    base_normal.set_editor_property('constant',unreal.LinearColor(0,0,1,1))
    strength=node(unreal.MaterialExpressionLinearInterpolate)
    link(base_normal,'',strength,'A');link(final_normal,normal_output,strength,'B')
    link(scalar('NormalStrength',.28),'',strength,'Alpha')
    normalized=node(unreal.MaterialExpressionNormalize);link(strength,'',normalized,'VectorInput')
    prop(color,'',unreal.MaterialProperty.MP_BASE_COLOR)
    prop(roughness,'',unreal.MaterialProperty.MP_ROUGHNESS)
    prop(normalized,'',unreal.MaterialProperty.MP_NORMAL)
    prop(scalar('Metallic',0.),'',unreal.MaterialProperty.MP_METALLIC)
    LIB.recompile_material(mat)
    assert ASSETS.save_loaded_asset(mat,only_if_is_dirty=False)
    return mat


parents={False:master('M_BY8_SurfacePBR',False),True:master('M_BY8_DustPBR',True)}
texture_meta=json.loads((ROOT/'Art/Textures/DistrictV3/metadata.json').read_text(encoding='utf-8'))
materials={}
for key,values in manifest['materials'].items():
    name='MI_BY8_'+key;path=DEST+'/Materials/'+name
    instance=unreal.load_asset(path)
    if not instance:
        instance=TOOLS.create_asset(name,DEST+'/Materials',unreal.MaterialInstanceConstant,unreal.MaterialInstanceConstantFactoryNew())
    assert isinstance(instance,unreal.MaterialInstanceConstant)
    LIB.set_material_instance_parent(instance,parents[bool(values.get('feather_ground'))])
    LIB.set_material_instance_vector_parameter_value(instance,'Tint',unreal.LinearColor(*values['tint'],1))
    for channel in ('Albedo','Normal','Roughness'):
        LIB.set_material_instance_texture_parameter_value(instance,channel+'Texture',textures[values['source']][channel])
    for parameter,value in [('TileMeters',texture_meta['materials'][values['source']]['tile_meters']),
                            ('RoughnessScale',values['roughness']),('Metallic',values['metallic']),('NormalStrength',.28)]:
        LIB.set_material_instance_scalar_parameter_value(instance,parameter,value)
    LIB.update_material_instance(instance)
    assert ASSETS.save_loaded_asset(instance,only_if_is_dirty=False)
    materials[key]=instance

meshes={};collision_records=[]
for meta in manifest['assets']:
    name=meta['name'];task=unreal.AssetImportTask()
    task.filename=str(SOURCE/(name+'.fbx'));task.destination_path=DEST+'/Meshes';task.destination_name=name
    task.automated=True;task.replace_existing=True;task.save=True
    options=unreal.FbxImportUI();options.import_mesh=True;options.import_materials=False;options.import_textures=False
    options.import_as_skeletal=False;options.mesh_type_to_import=unreal.FBXImportType.FBXIT_STATIC_MESH
    options.static_mesh_import_data.combine_meshes=True;options.static_mesh_import_data.auto_generate_collision=False
    options.static_mesh_import_data.generate_lightmap_u_vs=True
    options.static_mesh_import_data.set_editor_property('vertex_color_import_option',unreal.VertexColorImportOption.REPLACE)
    task.options=options;TOOLS.import_asset_tasks([task])
    mesh=unreal.load_asset(DEST+'/Meshes/'+name);assert isinstance(mesh,unreal.StaticMesh)
    bounds=mesh.get_bounding_box();actual=[[bounds.min.x,bounds.max.x],[bounds.min.y,bounds.max.y],[bounds.min.z,bounds.max.z]]
    assert all(abs(a-b)<1 for pair,expected in zip(actual,meta['expected_ue_bounds_cm']) for a,b in zip(pair,expected)),(name,actual)
    for index,slot in enumerate(mesh.get_editor_property('static_materials')):
        key=str(slot.get_editor_property('imported_material_slot_name')).split('.')[0].removeprefix('BY8_')
        assert key in materials,(name,key);mesh.set_material(index,materials[key])
    assert len(mesh.get_editor_property('static_materials'))==6
    collision_records.append(ensure_no_simple_collision(mesh))
    assert ASSETS.save_loaded_asset(mesh,only_if_is_dirty=False)
    meshes[name]=mesh

for actor in ACTORS.get_all_level_actors():
    if ours(actor):ACTORS.destroy_actor(actor)
actors=[]
for meta in manifest['assets']:
    actor=ACTORS.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*meta['actor_location_cm']),
                                      unreal.Rotator(pitch=0,yaw=0,roll=0))
    assert actor
    actor.set_actor_label('BYV8 '+meta['side']+' flat industrial courtyard')
    actor.tags=[unreal.Name(TAG),unreal.Name('HERO_DecorativeGround'),unreal.Name('HERO_NoNav')]
    component=actor.static_mesh_component;component.modify();component.set_static_mesh(meshes[meta['name']])
    component.set_collision_profile_name(unreal.Name('NoCollision'))
    component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    component.set_editor_property('can_ever_affect_navigation',False)
    component.set_editor_property('cast_shadow',False)
    component.set_editor_property('affect_distance_field_lighting',False)
    actor.set_actor_enable_collision(False)
    assert component.get_collision_enabled()==unreal.CollisionEnabled.NO_COLLISION
    assert not component.get_editor_property('cast_shadow')
    actors.append(actor)
after={a.get_path_name():snapshot(a) for a in ACTORS.get_all_level_actors() if not ours(a)}
assert after==preserved,'Existing map actors changed'
assert sum(ours(a) for a in ACTORS.get_all_level_actors())==2
assert LEVEL.save_current_level()
report=dict(result='PASS',actors=2,shared_material_instances=len(materials),
            total_triangles=sum(m['triangles'] for m in manifest['assets']),
            source_fbx_sha256={m['name']:m['fbx_sha256'] for m in manifest['assets']},
            labels=[a.get_actor_label() for a in actors],no_collision=True,casts_shadow=False,
            navigation_changed=False,existing_actor_snapshots_unchanged=True,preserved_actor_count=len(preserved),
            collision_validation=collision_records,
            dust_edges='Opaque world-space sand/dirt PBR blend from imported vertex alpha; runtime image review required')
(ROOT/'Saved/Research').mkdir(parents=True,exist_ok=True)
(ROOT/'Saved/Research/ReferenceBaseyardV8-Import.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
unreal.log('REFERENCE_BASEYARD_V8_IMPORT_PASS '+json.dumps(report))
