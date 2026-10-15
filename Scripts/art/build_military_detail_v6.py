"""Add original mechanical detailing to existing own ConceptV2 meshes.
Preserves original source files, pivots, bounds, material semantic names and gameplay footprint.
Run Blender 5.2 --background --python this_file.py.
"""
from pathlib import Path
import sys,math,json,hashlib
import bpy
from mathutils import Vector
sys.path.insert(0,str(Path(__file__).resolve().parent))
import concept_asset_common as c
ROOT=Path(__file__).resolve().parents[2];SOURCE=ROOT/'Art/Models/ConceptV2'
OUT=ROOT/'Art/Models/MilitaryDetailV6';OUT.mkdir(parents=True,exist_ok=True)
RECORDS=[]
def preview(obj,name):
    scene=bpy.context.scene;scene.render.engine='BLENDER_WORKBENCH'
    scene.display.shading.light='STUDIO';scene.display.shading.color_type='MATERIAL'
    scene.display.shading.show_shadows=True;scene.display.shading.show_cavity=True
    scene.display.shading.cavity_type='BOTH';scene.render.film_transparent=True
    scene.render.resolution_x=768;scene.render.resolution_y=768;scene.render.resolution_percentage=100
    b=bounds([obj]);target=Vector(((b[0][0]+b[1][0])/2,(b[0][1]+b[1][1])/2,(b[0][2]+b[1][2])/2))
    bpy.ops.object.camera_add(location=target+Vector((8,-11,8)))
    cam=bpy.context.object;cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler()
    cam.data.type='ORTHO';cam.data.ortho_scale=max(obj.dimensions)*1.55;scene.camera=cam
    scene.render.image_settings.file_format='PNG';scene.render.filepath=str(OUT/(name+'-Preview.png'))
    bpy.ops.render.render(write_still=True)
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def bounds(objects):
    coords=[o.matrix_world@v.co for o in objects for v in o.data.vertices]
    return [[min(p[k] for p in coords) for k in range(3)],[max(p[k] for p in coords) for k in range(3)]]
def box(name,loc,size,mat='Armor',rot=None):return c.box('V6 '+name,loc,size,mat,.008,rot)
def beam(name,a,b,r=.022,mat='Steel'):return c.beam('V6 '+name,a,b,r,mat)
def cyl(name,loc,r,d,mat='Steel',rot=(0,0,0)):return c.cylinder('V6 '+name,loc,r,d,mat,rot,vertices=12)
def grille(name,x,y,z,w,h,axis='Y'):
    c.vent('V6 '+name,(x,y,z),w,h,axis=axis,count=8)
def panel_ring(z,r,count=12):
    for i in range(count):
        a=i*math.tau/count
        box('Segmented roof panel',(r*math.cos(a),r*math.sin(a),z),(.48,.27,.045),'Steel',(0,0,a))
        box('Roof panel recessed seam',(r*math.cos(a),r*math.sin(a),z+.029),(.33,.035,.018),'Dark',(0,0,a))
def hq():
    # Broad independent side annexes give the HQ layered mass at the base-camera distance.
    for s in (-1,1):
        x=s*2.64
        box('Auxiliary armored annex',(x,-.48,-.02),(.80,1.72,.83),'Armor')
        box('Annex service shoulder',(x,-.48,.43),(.86,1.80,.12),'Steel')
        box('Annex recessed roof',(x,-.48,.51),(.69,1.59,.08),'Dark')
        for j in range(4):
            box('Annex segmented roof',(x,-1.04+j*.38,.57),(.64,.31,.065),'Armor')
            box('Annex roof catch',(x+s*.24,-1.04+j*.38,.61),(.065,.14,.028),'Steel')
        grille('Auxiliary heat exchanger',x,-1.365,.01,.59,.41)
        beam('Annex supply manifold',(x,.45,-.02),(x,.94,-.02),.075,'Dark')
        beam('Raised supply feed',(x,.94,-.02),(s*2.15,.94,.56),.075,'Steel')
        for y in (-.94,-.52,-.10):
            box('Side annex independent armor',(x+s*.43,y,.02),(.045,.34,.57),'Steel')
            box('Side annex slot',(x+s*.458,y,.12),(.015,.18,.04),'Neon')
    panel_ring(1.68,1.35,12)
    # Recessed roof HVAC banks and cable bridges read at RTS scale.
    for x in (-1.12,1.12):
        box('Upper command cooling cassette',(x,.92,1.81),(.56,.63,.31),'Armor')
        grille('Roof cassette louvers',x,.58,1.81,.44,.20)
        for j in range(5):box('Cassette radiator',(x-.20+j*.10,.92,1.99),(.045,.50,.07),'Steel')
    for s in (-1,1):
        for z in (-.12,.13,.38,.63):
            box('Entry layered blast baffle',(s*.49,-2.43,z),(.30,.10,.14),'Steel')
        beam('Portal conduit',(s*.81,-2.11,-.40),(s*.81,-2.11,.82),.045,'Dark')
        box('Portal conduit armored cap',(s*.81,-2.11,.88),(.15,.18,.17),'Armor')
    # Offset concave communications dish, original rings and structural struts.
    cx,cy,cz=1.52,.20,2.54
    cyl('Dish pedestal',(cx,cy,2.02),.18,.51,'Armor')
    verts=[];faces=[];rings=5;sides=24
    for j in range(rings+1):
        r=.075+.57*j/rings
        for i in range(sides):
            a=i*math.tau/sides;verts.append((cx+r*math.cos(a),cy+r*math.sin(a),cz+.30*(r/.645)**2))
    for j in range(rings):
        for i in range(sides):
            a=j*sides+i;b=j*sides+(i+1)%sides;faces.append((a,a+sides,b+sides,b))
    mesh=bpy.data.meshes.new('V6 dish');mesh.from_pydata(verts,[],faces);mesh.update()
    obj=bpy.data.objects.new('V6 communications dish',mesh);bpy.context.collection.objects.link(obj);obj.data.materials.append(bpy.data.materials['Steel'])
    c.torus('V6 dish reinforcement',(cx,cy,cz+.30),.645,.025,'Armor')
    for a in (0,math.tau/3,math.tau*2/3):beam('Dish feed strut',(cx+.60*math.cos(a),cy+.60*math.sin(a),cz+.27),(cx,cy,cz+.64),.017)
    cyl('Dish receiver',(cx,cy,cz+.65),.055,.14,'Dark')

def factory():
    # Six separate roof cassettes replace the impression of one broad blank slab.
    for x in (-.58,.58):
        for y in (-.54,.16,.86):
            box('Raised armored roof cassette',(x,y,2.47),(.91,.57,.13),'Armor')
            for k in range(6):box('Roof cassette louver',(x-.33+k*.13,y,2.55),(.045,.42,.075),'Steel')
            box('Roof cassette service band',(x,y-.28,2.55),(.72,.025,.025),'Dark')
    for s in (-1,1):
        x=s*1.64
        for y in (-.65,.14,.93):
            box('Flank armored inspection panel',(x,y,.70),(.055,.54,.54),'Steel')
            box('Flank inspection inset',(x+s*.035,y,.70),(.022,.43,.39),'Dark')
            for j in range(4):box('Flank radiator blade',(x+s*.06,y-.15+j*.10,.70),(.035,.035,.32),'Armor')
        beam('Flank return conduit',(x,-.82,.31),(x,1.25,.31),.047,'Steel')
    for x in (-.75,0,.75):
        box('Portal structural header',(x,-1.49,1.87),(.57,.11,.13),'Armor')
        box('Portal header inset',(x,-1.553,1.87),(.34,.018,.045),'Dark')
    # Offset control cabinet and external stair access remain on the original slab.
    box('Gantry control console',(-1.83,-1.37,.46),(.46,.41,.67),'Armor')
    box('Gantry operator screen',(-1.83,-1.585,.63),(.29,.022,.22),'Glass')
    for j in range(4):box('Maintenance stair',(-1.78,1.22+j*.13,.12+j*.075),(.49,.18,.12),'Steel')
    for y in (-.74,.12,.98):cyl('Roof circular access',(0,y,2.45),.15,.045,'Dark')

def refinery():
    # Real segmented processing armor and visible pressure plumbing.
    for i in range(10):
        a=i*math.tau/10
        if math.sin(a)<-.72:continue
        x,y=1.34*math.cos(a),.08+1.34*math.sin(a)
        box('Segmented process armor',(x,y,1.82),(.075,.40,.28),'Steel',(0,0,a))
        box('Process inspection slot',(x*1.025,(y-.08)*1.025+.08,1.84),(.022,.24,.08),'Dark',(0,0,a))
    for x in (-.51,.51):
        box('Loading canopy raised armor',(x,-1.38,1.62),(.30,.79,.075),'Steel')
        for y in (-1.65,-1.37,-1.09):box('Canopy segmented brace',(x,y,1.67),(.27,.045,.035),'Dark')
    for x in (-.49,.49):
        box('Processing roof heat exchanger',(x,.32,2.82),(.30,.65,.16),'Dark')
        for j in range(7):box('Heat exchanger fin',(x,-.05+j*.105,2.93),(.27,.035,.09),'Steel')
    for s in (-1,1):
        x=s*1.53
        cyl('Auxiliary fluid buffer',(x,.84,.68),.17,.98,'Steel')
        for z in (.26,.57,.98):c.torus('V6 fluid buffer collar',(x,.84,z),.174,.025,'Armor')
        beam('Fluid buffer supply',(x,.84,1.20),(s*1.12,.84,1.20),.045,'Dark')
        box('Buffer armored sight glass',(x,.65,.69),(.11,.035,.40),'Glass')
    for x in (-.49,0,.49):
        box('Receiving deck inset',(x,-1.61,.20),(.12,.51,.025),'Steel')
    # Utility cabinets and protective brackets at the rear keep the loading mouth clear.
    for x in (-.88,.88):
        box('Rear process control cabinet',(x,1.27,.43),(.41,.32,.66),'Armor')
        grille('Control cabinet vent',x,1.45,.43,.31,.37)
        beam('Control conduit',(x,1.24,.82),(x,.98,1.10),.034,'Steel')

for source,role,add in [('CommandHub','HQ',hq),('VehiclePlant','Factory',factory),('Refinery','Depot',refinery)]:
    path=SOURCE/('SM_HERO_'+source+'.blend');before=sha(path)
    bpy.ops.wm.open_mainfile(filepath=str(path));bpy.context.preferences.filepaths.save_version=0
    base=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(base)==1
    original_bounds=bounds(base);oldtris=sum(len(p.vertices)-2 for p in base[0].data.polygons)
    originals=set(base)
    for key,(rgb,metal,rough) in c.PALETTE.items():
        if not bpy.data.materials.get(key):
            mat=bpy.data.materials.new(key);mat.diffuse_color=(*rgb,1);mat.use_nodes=True
            bsdf=mat.node_tree.nodes.get('Principled BSDF');bsdf.inputs['Base Color'].default_value=(*rgb,1);bsdf.inputs['Metallic'].default_value=metal;bsdf.inputs['Roughness'].default_value=rough
    add();new=[o for o in bpy.context.scene.objects if o.type=='MESH' and o not in originals]
    # Added geometry may approach but must never expand any source bounding face.
    for obj in new:
        for v in obj.data.vertices:
            p=obj.matrix_world@v.co
            assert all(original_bounds[0][k]-.001<=p[k]<=original_bounds[1][k]+.001 for k in range(3)),(source,obj.name,tuple(p),original_bounds)
        uv=obj.data.uv_layers.new(name='V6DetailUV')
        for poly in obj.data.polygons:
            for li in poly.loop_indices:
                p=obj.data.vertices[obj.data.loops[li].vertex_index].co;uv.data[li].uv=(p.x,p.y)
    bpy.ops.object.select_all(action='DESELECT')
    for o in base+new:o.select_set(True)
    bpy.context.view_layer.objects.active=base[0];bpy.ops.object.join();obj=bpy.context.object
    name='SM_MDV6_'+source;obj.name=name
    actual=bounds([obj]);assert all(abs(a-b)<.002 for old,row in zip(original_bounds,actual) for a,b in zip(old,row))
    assert obj.location.length<.001
    tris=sum(len(p.vertices)-2 for p in obj.data.polygons);assert oldtris<tris<65000
    dims=list(obj.dimensions);mats=[m.name for m in obj.data.materials]
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/(name+'.blend')))
    bpy.ops.export_scene.fbx(filepath=str(OUT/(name+'.fbx')),use_selection=True,object_types={'MESH'},axis_forward='X',axis_up='Z',apply_unit_scale=False,mesh_smooth_type='FACE')
    assert sha(path)==before,'Original source modified'
    preview(obj,name)
    bpy.ops.object.select_all(action='SELECT');bpy.ops.object.delete(use_global=False);bpy.ops.import_scene.fbx(filepath=str(OUT/(name+'.fbx')))
    imported=[o for o in bpy.context.scene.objects if o.type=='MESH'];assert len(imported)==1
    assert all(abs(a-b)<.002 for a,b in zip(dims,imported[0].dimensions))
    record=dict(name=name,role=role,source=str(path.relative_to(ROOT)),source_sha256=before,source_unchanged=True,source_triangles=oldtris,triangles=tris,added_triangles=tris-oldtris,detail_objects=len(new),dimensions_m=dims,bounds_m=actual,original_bounds_m=original_bounds,materials=mats,pivot_preserved=True,fbx_roundtrip=True,validation='PASS')
    # FBX axis_forward=X / axis_up=Z to Unreal's left-handed frame mirrors Y.
    # Symmetric HQ hid this distinction; compare asymmetric factory/refinery
    # against explicitly transformed bounds, not Blender's right-handed AABB.
    record['unreal_bounds_m']=[[actual[0][0],-actual[1][1],actual[0][2]],
                               [actual[1][0],-actual[0][1],actual[1][2]]]
    record['unreal_axis_transform']='X unchanged, Y negated, Z unchanged'
    RECORDS.append(record);print('MILITARY_DETAIL_V6_ASSET_PASS',json.dumps(record))
(OUT/'manifest.json').write_text(json.dumps(dict(version=6,original_assets=True,assets=RECORDS),indent=2),encoding='utf-8')
print('MILITARY_DETAIL_V6_BUILD_PASS',len(RECORDS))
