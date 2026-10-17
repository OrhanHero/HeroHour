"""One bounded own-server diagnostic run. Requires --execute after integration approval."""
import argparse, datetime, hashlib, json, shutil, subprocess, time, urllib.request
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];BASE=ROOT/'Saved/Research/TrellisV8'
COMFY=Path('E:/Comfy-Desktop/ComfyUI-Installs/Hero/ComfyUI');URL='http://127.0.0.1:8189'
parser=argparse.ArgumentParser();parser.add_argument('--execute',action='store_true');parser.add_argument('--repair-texture',action='store_true');args=parser.parse_args()
OUT=BASE/('repaired-texture' if args.repair_texture else 'diagnostic')
assert args.execute,'Explicit integration approval required before starting GPU server'
assert not (OUT/'prompt-response.json').exists(),'Never submit twice'
def save(name,value):(OUT/name).write_text(json.dumps(value,indent=2),encoding='utf-8')
def get(route):
 with urllib.request.urlopen(URL+route,timeout=15) as r:return json.load(r)
def post(route,value):
 request=urllib.request.Request(URL+route,data=json.dumps(value).encode(),headers={'Content-Type':'application/json'})
 with urllib.request.urlopen(request,timeout=30) as r:return json.load(r) if r.headers.get('Content-Type','').startswith('application/json') else None
userdb=COMFY/'user/comfyui.db';before=hashlib.sha256(userdb.read_bytes()).hexdigest() if userdb.exists() else None
dburl='sqlite:///'+(OUT/'isolated.db').as_posix()
(OUT/'user').mkdir(exist_ok=True)
arguments=[str(COMFY/'.venv/Scripts/python.exe'),str(COMFY/'main.py'),'--listen','127.0.0.1','--port','8189',
 '--disable-auto-launch','--disable-all-custom-nodes','--lowvram','--reserve-vram','2',
 '--database-url',dburl,'--extra-model-paths-config',str(BASE/'comfy-model-paths.yaml'),
 '--input-directory',str(BASE/'input'),'--output-directory',str(BASE/'output'),'--user-directory',str(OUT/'user')]
try:
 get('/queue');raise RuntimeError('8189 already occupied: do not attach blindly')
except urllib.error.URLError:pass
log=open(OUT/'server.log','w',encoding='utf-8');proc=subprocess.Popen(arguments,cwd=COMFY,stdout=log,stderr=subprocess.STDOUT,creationflags=subprocess.CREATE_NO_WINDOW)
save('server-process.json',{'pid':proc.pid,'arguments':arguments,'user_db_sha_before':before})
start=time.monotonic();state={'status':'starting','server_pid':proc.pid,'limit_seconds':600,'gpu_peak_mib':0,'samples':[]}
try:
 for i in range(60):
  if proc.poll() is not None:raise RuntimeError('Owned server exited during startup')
  try:get('/queue');break
  except urllib.error.URLError:time.sleep(1)
 else:raise TimeoutError('Own server startup timeout')
 graph=json.loads((OUT/'workflow-api.json').read_text(encoding='utf-8'));schemas={}
 for kind in sorted({n['class_type'] for n in graph.values()}):schemas[kind]=get('/object_info/'+kind)[kind]
 save('live-schemas.json',schemas)
 errors=[]
 for id,node in graph.items():
  schema=schemas[node['class_type']];required=schema['input'].get('required',{});allinputs={**required,**schema['input'].get('optional',{})}
  # V3 DynamicCombo sends flat dotted option inputs; expand the actual selected schema.
  for root,declared in list(allinputs.items()):
   if declared[0]=='COMFY_DYNAMICCOMBO_V3':
    selected=node['inputs'].get(root)
    for option in declared[1].get('options',[]):
     if option['key']==selected:
      for group in ('required','optional'):
       for sub,subdecl in option.get('inputs',{}).get(group,{}).items():allinputs[root+'.'+sub]=subdecl
  for key in required:
   if key not in node['inputs']:errors.append(id+':missing '+key)
  for key,value in node['inputs'].items():
   if key not in allinputs:errors.append(id+':unknown '+key);continue
   kind=allinputs[key][0];opts=allinputs[key][1] if len(allinputs[key])>1 else {}
   if isinstance(value,list) and len(value)==2 and isinstance(value[0],str):
    actual=schemas[graph[value[0]]['class_type']]['output'][value[1]]
    if isinstance(kind,str) and kind!='*' and actual not in kind.split(','):errors.append(id+':type '+key)
   elif kind=='COMFY_DYNAMICCOMBO_V3':
    choice=value.get(key) if isinstance(value,dict) else value
    if choice not in [x['key'] for x in opts.get('options',[])]:errors.append(id+':dynamic '+key)
   elif isinstance(kind,list) and value not in kind:errors.append(id+':enum '+key)
 save('schema-validation.json',{'errors':errors,'passed':not errors,'http_methods':['GET'],'inference_queued':False})
 assert not errors,errors
 assert not get('/queue')['queue_running'] and not get('/queue')['queue_pending']
 response=post('/prompt',{'prompt':graph,'client_id':'HeroHourV8-Diagnostic'});save('prompt-response.json',response);pid=response['prompt_id']
 print('DIAGNOSTIC_SUBMITTED',pid,flush=True)
 while time.monotonic()-start<600:
  used=int(subprocess.check_output(['C:/Windows/System32/nvidia-smi.exe','--query-gpu=memory.used','--format=csv,noheader,nounits'],text=True,creationflags=subprocess.CREATE_NO_WINDOW,timeout=10).strip())
  state['gpu_peak_mib']=max(state['gpu_peak_mib'],used);state['samples'].append({'seconds':round(time.monotonic()-start,2),'gpu_used_mib':used})
  history=get('/history/'+pid)
  if pid in history:
   record=history[pid];save('history.json',record);state['status']=record['status']['status_str'];break
  state['status']='running';save('status.json',state);time.sleep(3)
 else:
  state['status']='timeout';post('/interrupt',{})
except Exception as exc:
 state['status']='error';state['error']=repr(exc)
 try:post('/interrupt',{})
 except Exception:pass
finally:
 state['elapsed_seconds']=round(time.monotonic()-start,2)
 try:post('/free',{'unload_models':True,'free_memory':True})
 except Exception:pass
 proc.terminate()
 try:proc.wait(timeout=20)
 except subprocess.TimeoutExpired:proc.kill();proc.wait()
 log.close();state['own_server_stopped']=proc.poll() is not None
 state['user_db_unchanged']=(hashlib.sha256(userdb.read_bytes()).hexdigest() if userdb.exists() else None)==before
 dest=ROOT/'Art/Models'/('TrellisV8-Repaired' if args.repair_texture else 'TrellisV8-Diagnostic');dest.mkdir(exist_ok=True)
 outputs=[]
 for f in (BASE/'output'/('HeroHourV8-Repaired' if args.repair_texture else 'HeroHourV8-Diagnostic')).glob('*.glb'):
  target=dest/f.name
  assert not target.exists() or target.read_bytes()==f.read_bytes()
  if not target.exists():shutil.copyfile(f,target)
  outputs.append({'file':str(target.relative_to(ROOT)),'bytes':f.stat().st_size,'sha256':hashlib.sha256(f.read_bytes()).hexdigest()})
 state['outputs']=outputs;save('status.json',state)
 print('DIAGNOSTIC_FINISHED',json.dumps(state),flush=True)
assert state['status']=='success' and state['user_db_unchanged'],state
