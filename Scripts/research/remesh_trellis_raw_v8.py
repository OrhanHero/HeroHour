"""One native-function remesh trial of saved RAW; no diffusion, server or user DB."""
import os,sys,time,json,hashlib,struct,threading
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'Saved/Research/TrellisV8/diagnostic-remesh';OUT.mkdir(exist_ok=True)
COMFY=Path('E:/Comfy-Desktop/ComfyUI-Installs/Hero/ComfyUI');sys.path.insert(0,str(COMFY))
timer=threading.Timer(600,lambda:os._exit(99));timer.daemon=True;timer.start();start=time.monotonic()
import numpy as np,torch
from comfy_extras.mesh3d.postprocess.remesh import remesh_narrow_band_dc
from comfy_extras.mesh3d.postprocess.qem_decimate import qem_decimate_simplify,QEMConfig,_compute_vertex_normals
from comfy_extras.nodes_save_3d import save_glb
source=ROOT/'Art/Models/TrellisV8-Diagnostic/RAW_00001_.glb';raw=source.read_bytes();sha=hashlib.sha256(raw).hexdigest()
assert sha=='cc7309e87a424f63250a5b49cdf9336cc2739b0234986efb278701cb62cbabdc'
chunks={};offset=12
while offset<len(raw):
 size,kind=struct.unpack_from('<I4s',raw,offset);offset+=8;chunks[kind]=raw[offset:offset+size];offset+=size
g=json.loads(chunks[b'JSON']);b=chunks[b'BIN\0'];p=g['meshes'][0]['primitives'][0]
def access(i):
 a=g['accessors'][i];v=g['bufferViews'][a['bufferView']];dt={5126:'<f4',5125:'<u4'}[a['componentType']];cols={'SCALAR':1,'VEC3':3}[a['type']]
 return np.ndarray((a['count'],cols),dtype=dt,buffer=b,offset=v.get('byteOffset',0)+a.get('byteOffset',0)).copy()
v=torch.from_numpy(access(p['attributes']['POSITION'])).cuda();f=torch.from_numpy(access(p['indices']).reshape(-1,3).astype(np.int64)).cuda()
params=dict(resolution=512,band=1.0,project_back=0.0,qef=False,sign_mode='udf',manifold=False,fix_poles=False,smooth_iters=20,drop_small_components=.01,drop_inverted_components=False,drop_enclosed_components=False,scale=(512+3.)/512,center=torch.zeros(3,device='cuda'))
print('RAW_LOADED',len(v),len(f),'RE MESH512',flush=True)
report={'raw_sha256':sha,'function':'comfy_extras.mesh3d.postprocess.remesh.remesh_narrow_band_dc','parameters':{k:(val.tolist() if torch.is_tensor(val) else val) for k,val in params.items()},'target_faces':700000,'diffusion_ran':False,'native_server_started':False,'status':'running'}
def record(): (OUT/'result.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
record()
try:
 rv,rf,_=remesh_narrow_band_dc(v,f,**params);report['remesh_vertices']=len(rv);report['remesh_triangles']=len(rf)
 dest=ROOT/'Art/Models/TrellisV8-Diagnostic'
 save_glb(rv,rf,filepath=str(dest/'REMESHED512-RAW.glb'))
 if len(rf)>700000:dv,df,_,_,_=qem_decimate_simplify(rv,rf,700000,config=QEMConfig())
 else:dv,df=rv,rf
 normals=_compute_vertex_normals(dv,df)
 save_glb(dv,df,normals=normals,filepath=str(dest/'REMESHED_00001_.glb'))
 report.update(status='success',final_vertices=len(dv),final_triangles=len(df),source_unchanged=hashlib.sha256(source.read_bytes()).hexdigest()==sha)
except Exception as exc:report.update(status='failed',error=repr(exc));raise
finally:
 report.update(elapsed_seconds=time.monotonic()-start,torch_peak_allocated_bytes=torch.cuda.max_memory_allocated(),torch_peak_reserved_bytes=torch.cuda.max_memory_reserved());record();timer.cancel();print('REMESH_RESULT',json.dumps(report),flush=True)
