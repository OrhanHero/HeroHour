"""CPU-only inspection of the saved candidate and installed primary-source nodes."""
import hashlib, json, struct
from pathlib import Path
import numpy as np

ROOT = Path(__file__).resolve().parents[2]
SAVED = ROOT / 'Saved/Research/TrellisV8'
OUT = ROOT / 'docs/verification/2026-10-16/reference-v8/trellis'
OUT.mkdir(parents=True, exist_ok=True)
source = ROOT / 'Art/Models/TrellisV8-Candidate/AegisHQ-Trellis512_00001_.glb'
raw = source.read_bytes()
assert hashlib.sha256(raw).hexdigest() == 'fb53b1c581598eb06507375ffdaa0424b00b0844597fb5ff2cb11237fcd44f28'
chunks = {}; cursor = 12
while cursor < len(raw):
    size, kind = struct.unpack_from('<I4s', raw, cursor)
    cursor += 8; chunks[kind] = raw[cursor:cursor+size]; cursor += size
gltf = json.loads(chunks[b'JSON']); binary = chunks[b'BIN\0']
def accessor(i):
    a = gltf['accessors'][i]; v = gltf['bufferViews'][a['bufferView']]
    dtype = {5126:'<f4',5125:'<u4',5123:'<u2'}[a['componentType']]
    cols = {'SCALAR':1,'VEC2':2,'VEC3':3}[a['type']]
    return np.ndarray((a['count'],cols), dtype=dtype, buffer=binary,
        offset=v.get('byteOffset',0)+a.get('byteOffset',0),
        strides=(v.get('byteStride',np.dtype(dtype).itemsize*cols),np.dtype(dtype).itemsize)).copy()
p = gltf['meshes'][0]['primitives'][0]
vertices = accessor(p['attributes']['POSITION']); faces = accessor(p['indices']).reshape(-1,3)
# UV seams duplicate vertices: compare position-welded topology, never raw UV connectivity.
unique, remap = np.unique(vertices,axis=0,return_inverse=True); welded = remap[faces]
parent = np.arange(len(unique))
def find(x):
    while parent[x] != x:
        parent[x] = parent[parent[x]]; x = parent[x]
    return x
for a,b,c in welded:
    ra,rb,rc = find(a),find(b),find(c)
    parent[rb] = ra; parent[rc] = ra
roots = np.array([find(int(f[0])) for f in welded]); _,sizes = np.unique(roots,return_counts=True)
edges = np.sort(np.concatenate([welded[:,[0,1]],welded[:,[1,2]],welded[:,[2,0]]]),axis=1)
_,edge_counts = np.unique(edges,axis=0,return_counts=True)
area = np.linalg.norm(np.cross(vertices[faces[:,1]]-vertices[faces[:,0]],vertices[faces[:,2]]-vertices[faces[:,0]]),axis=1)/2
_, duplicate_counts = np.unique(np.sort(welded,axis=1),axis=0,return_counts=True)
comfy = Path('E:/Comfy-Desktop/ComfyUI-Installs/Hero/ComfyUI')
files = [comfy/'comfy_extras/nodes_trellis2.py',comfy/'comfy_extras/nodes_mesh_postprocess.py',comfy/'comfy_extras/nodes_save_3d.py',SAVED/'hq-trellis512-api.json',SAVED/'comfy-installed-trellis-template.json']
report = {'source_sha256':hashlib.sha256(raw).hexdigest(),'cpu_only':True,'source_modified':False,
    'raw_glb_vertices':len(vertices),'exact_unique_positions':len(unique),'triangles':len(faces),
    'position_welded_connected_components':len(sizes),'largest_components_faces':sorted(sizes.tolist(),reverse=True)[:20],
    'isolated_single_triangle_components':int(np.sum(sizes==1)),
    'welded_boundary_edges':int(np.sum(edge_counts==1)), 'welded_nonmanifold_edges':int(np.sum(edge_counts>2)),
    'exact_zero_area_triangles':int(np.sum(area==0)),'triangle_area_percentiles':np.percentile(area,[0,1,50,99,100]).tolist(),
    'duplicate_position_triangle_excess':int(np.sum(duplicate_counts-1)),
    'limitations':['Exact Float32 position welding ignores near-coincident vertices; UV seams are removed from this connectivity metric.','Raw decoded and intermediate decimated meshes were not saved: this cannot localize the first failing pipeline stage.','Counts describe exported topology, not artistic acceptance or a runtime game test.'],
    'primary_source_files':[{'path':str(f),'sha256':hashlib.sha256(f.read_bytes()).hexdigest()} for f in files]}
assert source.read_bytes()==raw
(OUT/'cpu-diagnosis.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps(report,indent=2))
