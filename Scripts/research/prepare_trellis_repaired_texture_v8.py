"""Prepare same-image voxel rebake on remeshed geometry. Never queues a job."""
import json,hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];BASE=ROOT/'Saved/Research/TrellisV8';OUT=BASE/'repaired-texture';OUT.mkdir(exist_ok=True)
original=json.loads((BASE/'hq-trellis512-api.json').read_text(encoding='utf-8'))
p={k:v for k,v in original.items() if int(k)<=22}
def node(id,kind,**inputs):p[str(id)]={'class_type':kind,'inputs':inputs}
def link(id,slot=0):return [str(id),slot]
node(45,'RemeshMesh',mesh=link(19),resolution=512,sign_mode='udf',band=1.,project_back=0.,fix_poles=False,smooth_iters=20,drop_small_components=.01,precluster_max_verts=20000000)
p['45']['inputs'].update({'sign_mode.qef':False,'sign_mode.drop_inverted_components':False,'sign_mode.drop_enclosed_components':False})
node(46,'DecimateMesh',mesh=link(45),target_face_count=700000,placement_mode='midpoint')
node(47,'MeshSmoothNormals',mesh=link(46),crease_angle=180.)
node(24,'UnwrapMesh',mesh=link(47),segmenter='pec',resolution=2048,padding=2,weld_distance=.0002)
node(25,'BakeTextureFromVoxel',mesh=link(24),voxel_colors=link(22),texture_size=2048,reference_mesh=link(19))
node(26,'ApplyTextureToMesh',mesh=link(24),base_color=link(25),metallic=link(25,1),roughness=link(25,2))
# Unwrap creates new UV vertices and drops normals, so recalculate after the final UV topology.
node(49,'MeshSmoothNormals',mesh=link(26),crease_angle=180.)
node(27,'SaveGLB',mesh=link(49),filename_prefix='HeroHourV8-Repaired/AegisHQ-Remeshed512-PBR2048')
for k in original:
 if int(k)<=22:assert p[k]==original[k]
path=OUT/'workflow-api.json';path.write_text(json.dumps(p,indent=2),encoding='utf-8')
report={'workflow_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'same_input_and_all_sampler_nodes':True,'new_seed_or_new_image':False,'uses_defective_candidate_uv_or_textures':False,'texture_source':'Original same-input texture-VAE voxel colors node22, back-projected against original decoded node19 onto freshly unwrapped remeshed geometry','resolution':512,'atlas':2048,'normal_recalculation_after_uv':True,'inference_queued':False,'live_validation_required':True,'dynamiccombo_fix':'Flat sign_mode string plus sign_mode.qef / sign_mode.drop_* keys; not nested object'}
(OUT/'preparation.json').write_text(json.dumps(report,indent=2),encoding='utf-8');print(json.dumps(report))
