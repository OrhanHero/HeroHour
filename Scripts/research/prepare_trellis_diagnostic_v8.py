"""Prepare controlled same-input geometry branches; no server or inference launch."""
import json, hashlib
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
BASE=ROOT/'Saved/Research/TrellisV8'
OUT=BASE/'diagnostic';OUT.mkdir(exist_ok=True)
original=json.loads((BASE/'hq-trellis512-api.json').read_text(encoding='utf-8'))
graph={k:v for k,v in original.items() if int(k)<=19}
# Original decode still depends on its exact same shape/structure conditioning.
graph.pop('8') # texture VAE is intentionally never loaded
def n(id,kind,**inputs):graph[str(id)]={'class_type':kind,'inputs':inputs}
def l(id):return [str(id),0]
n(40,'SaveGLB',mesh=l(19),filename_prefix='HeroHourV8-Diagnostic/RAW')
n(41,'DecimateMesh',mesh=l(19),target_face_count=60000,placement_mode='midpoint')
n(42,'SaveGLB',mesh=l(41),filename_prefix='HeroHourV8-Diagnostic/DECIMATED')
n(43,'UnwrapMesh',mesh=l(41),segmenter='pec',resolution=1024,padding=2,weld_distance=.0002)
n(44,'SaveGLB',mesh=l(43),filename_prefix='HeroHourV8-Diagnostic/UNWRAPPED')
n(45,'RemeshMesh',mesh=l(19),resolution=512,
  sign_mode={'sign_mode':'udf','qef':False,'drop_inverted_components':False,'drop_enclosed_components':False},
  band=1.0,project_back=0.0,fix_poles=False,smooth_iters=20,drop_small_components=.01,precluster_max_verts=20000000)
n(46,'DecimateMesh',mesh=l(45),target_face_count=700000,placement_mode='midpoint')
n(47,'MeshSmoothNormals',mesh=l(46),crease_angle=180.0)
n(48,'SaveGLB',mesh=l(47),filename_prefix='HeroHourV8-Diagnostic/REMESHED')
for key in graph:
 if int(key)<=19:assert graph[key]==original[key]
path=OUT/'workflow-api.json';path.write_text(json.dumps(graph,indent=2),encoding='utf-8')
report={'workflow_sha256':hashlib.sha256(path.read_bytes()).hexdigest(),'same_input_seeds_models_sampler_nodes':True,
 'outputs':['RAW','DECIMATED','UNWRAPPED','REMESHED'],'texture_inference_or_baking':False,
 'remesh_source':'Installed template node241 UDF, qefFalse, band1, project0, smooth20, drop0.01, precluster20M; resolution lowered 768 to 512 for 16GiB GPU',
 'memory_caution':'512 allocation estimator resolution^3*64 is 8GiB; 768 would estimate 27GiB and is deliberately not attempted. One bounded trial, no retries.',
 'inference_queued':False}
(OUT/'preparation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report))
