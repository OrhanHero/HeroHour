"""Read controlled GLB branches, without importing Blender or loading GPU libraries."""
import hashlib,json,struct
from pathlib import Path
import numpy as np
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'docs/verification/2026-10-16/reference-v8/trellis'
results=[]
for source in sorted((ROOT/'Art/Models/TrellisV8-Diagnostic').glob('*.glb')):
 raw=source.read_bytes();chunks={};offset=12
 while offset<len(raw):
  size,kind=struct.unpack_from('<I4s',raw,offset);offset+=8;chunks[kind]=raw[offset:offset+size];offset+=size
 g=json.loads(chunks[b'JSON']);b=chunks[b'BIN\0'];p=g['meshes'][0]['primitives'][0]
 def access(i):
  a=g['accessors'][i];v=g['bufferViews'][a['bufferView']];dt={5126:'<f4',5125:'<u4',5123:'<u2'}[a['componentType']];cols={'SCALAR':1,'VEC2':2,'VEC3':3}[a['type']]
  return np.ndarray((a['count'],cols),dtype=dt,buffer=b,offset=v.get('byteOffset',0)+a.get('byteOffset',0),strides=(v.get('byteStride',np.dtype(dt).itemsize*cols),np.dtype(dt).itemsize)).copy()
 v=access(p['attributes']['POSITION']);f=access(p['indices']).reshape(-1,3)
 area=np.linalg.norm(np.cross(v[f[:,1]]-v[f[:,0]],v[f[:,2]]-v[f[:,0]]),axis=1)/2
 unique,remap=np.unique(v,axis=0,return_inverse=True);wf=remap[f]
 _,duplicates=np.unique(np.sort(wf,axis=1),axis=0,return_counts=True)
 edges=np.sort(np.concatenate([wf[:,[0,1]],wf[:,[1,2]],wf[:,[2,0]]]),axis=1);_,counts=np.unique(edges,axis=0,return_counts=True)
 result={'file':source.name,'sha256':hashlib.sha256(raw).hexdigest(),'bytes':len(raw),'vertices':len(v),'triangles':len(f),'unique_positions':len(unique),'bounds_min':v.min(0).tolist(),'bounds_max':v.max(0).tolist(),'zero_area':int(np.sum(area==0)),'duplicate_position_triangle_excess':int(np.sum(duplicates-1)),'boundary_edges':int(np.sum(counts==1)),'nonmanifold_edges':int(np.sum(counts>2)),'area_percentiles':np.percentile(area,[0,1,50,99,100]).tolist(),'has_normals':'NORMAL' in p['attributes'],'has_uv':'TEXCOORD_0' in p['attributes']}
 results.append(result);print(json.dumps(result),flush=True)
(OUT/'controlled-geometry-comparison.json').write_text(json.dumps({'results':results,'limits':['Same seeds and inputs do not guarantee GPU bitwise determinism.','Position welding is exact; close positions may still appear separate.','No artistic acceptance without neutral three-dimensional view.']},indent=2),encoding='utf-8')
