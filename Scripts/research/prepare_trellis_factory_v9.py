"""Prepare one own-image factory generation. CPU-only; never queues a job."""
import hashlib
import json
import shutil
from pathlib import Path
from PIL import Image

ROOT=Path(__file__).resolve().parents[2]
BASE=ROOT/'Saved/Research/TrellisFactoryV9'
BASE.mkdir(parents=True,exist_ok=True)
SOURCE=ROOT/'Art/Concepts/GeneratedV9/AegisFactory-TrellisInput-v1.png'
EXPECTED='323cc71aa80c1a8fa859ed7171db1df635e72ab24a9aad6c92b4402c6ae486c0'
assert hashlib.sha256(SOURCE.read_bytes()).hexdigest()==EXPECTED
(BASE/'input').mkdir(exist_ok=True)
target=BASE/'input/HeroHour-AegisFactoryV9.png'
if target.exists():assert target.read_bytes()==SOURCE.read_bytes()
else:shutil.copyfile(SOURCE,target)
graph=json.loads((ROOT/'Saved/Research/TrellisV8/repaired-texture/workflow-api.json').read_text(encoding='utf-8'))
graph['1']['inputs']['image']=target.name
# Remove low-alpha halo through the installed native ThresholdMask node. The
# source PNG is preserved byte-for-byte; masking occurs inside the pipeline.
graph['50']={'class_type':'ThresholdMask','inputs':{'mask':['2',0],'value':.5}}
graph['3']['inputs']['masks']=['50',0]
graph['51']={'class_type':'SaveImage','inputs':{'images':['3',0],'filename_prefix':'FactoryV9/Conditioning512'}}
graph['27']['inputs']['filename_prefix']='FactoryV9/AegisFactory-Remeshed512-PBR2048'
assert graph['45']['class_type']=='RemeshMesh' and graph['45']['inputs']['resolution']==512
assert graph['46']['inputs']['target_face_count']==700000
assert graph['24']['inputs']['resolution']==2048 and graph['25']['inputs']['texture_size']==2048
assert graph['49']['class_type']=='MeshSmoothNormals' and graph['49']['inputs']['mesh']==['26',0]
assert [graph[str(i)]['inputs']['seed'] for i in (13,18,21)]==[56,42,43]
workflow=BASE/'workflow-api.json'
workflow.write_text(json.dumps(graph,indent=2),encoding='utf-8')
with Image.open(SOURCE) as image:
    alpha=image.getchannel('A');hist=alpha.histogram()
    # Inspect mask statistics only. The actual image operation is the installed
    # Comfy threshold/crop graph, not a hand-edited substitute conditioning file.
    core=alpha.point(lambda value:255 if value>127 else 0)
    report=dict(input_sha256=EXPECTED,input_dimensions=list(image.size),alpha_range=alpha.getextrema(),
        alpha_raw_bbox=alpha.getbbox(),threshold=.5,threshold_core_bbox=core.getbbox(),
        raw_nonzero_pixels=sum(hist[1:]),core_pixels=sum(hist[128:]),
        workflow_sha256=hashlib.sha256(workflow.read_bytes()).hexdigest(),
        inherited_repaired_workflow='Scripts/research/prepare_trellis_repaired_texture_v8.py',
        seeds=[56,42,43],remesh_resolution=512,decimation_target=700000,atlas_resolution=2048,
        fresh_voxel_bake=True,normals_after_final_uv=True,no_weight_download=True,
        inference_queued=False,needs_live_schema_validation=True,
        output_directory='Art/Models/TrellisFactoryV9-Candidate')
(BASE/'preparation.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print('FACTORY_V9_WORKFLOW_PREPARED_NO_INFERENCE',json.dumps(report))
