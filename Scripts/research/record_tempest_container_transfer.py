"""Archive bounded transfer evidence; never copy game geometry or textures to Git."""
import collections
import hashlib
import json
from pathlib import Path

ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'docs/verification/2026-10-17/container-transfer'
OUT.mkdir(parents=True,exist_ok=True)
def read(path):return json.loads(path.read_text(encoding='utf-8-sig'))
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
probe_path=ROOT/'Saved/Research/TempestAssetTransfer/Container-Probe-02/Probe.json'
probe=read(probe_path)
assert probe['result']=='PASS' and len(probe['exports'])==7
assert all(item['Success'] for item in probe['exports'])
import_path=ROOT/'Saved/Research/TempestAssetTransfer/Unreal58/Container-Import58.json'
imported=read(import_path)
assert imported['result']=='PASS' and imported['dimensions_match_source'] and imported['material_slots']==1
native_path=max((ROOT/'Saved/Research/TempestAssetTransfer').glob('Engine54-*/Container-Engine54-Report.json'),key=lambda p:p.stat().st_mtime)
native=read(native_path)
assert native['result']=='PASS' and native['exports'][0]['success']
visual_path=max((ROOT/'Saved/Research/TempestAssetTransfer/VisualQA').glob('*/Container-Blender-Verification.json'),key=lambda p:p.stat().st_mtime)
visual=read(visual_path)
editor_path=max((ROOT/'Saved/Research/TempestMapTransfer/Editor54').glob('*.json'),key=lambda p:p.stat().st_mtime)
editor=read(editor_path)
assert editor['status']=='PASS' and editor['actor_count']==8279 and len(editor['levels'])==3
containers=[actor for actor in editor['actors'] if any('SM_GDF_Container.' in str(c.get('mesh','')) for c in actor['components'])]
assert len(containers)==20
files=[]
for item in probe['exports']:
    for filename in item['DiskFilePaths']:
        p=Path(filename)
        assert p.is_file() and p.stat().st_size>0
        files.append(dict(path=str(p.relative_to(ROOT)),bytes=p.stat().st_size,sha256=digest(p)))
png=visual_path.parent/'Container-OriginalAlbedo-Baseline.png'
assert png.is_file()
summary=dict(schema='HeroHour.TempestContainerTransfer.v1',result='PASS_BOUNDED_CONTAINER_TRANSFER',
    source_read_only_assets=True,source_project_association_corrected_from='5.8',source_project_association_corrected_to='5.4',
    cue4parse_commit='605672df7a993a90094bad92d30c388d87ae6c1a',
    reflection_structs_sha256=digest(ROOT/'Saved/Research/TempestMappings54/structs.json'),
    reflection_enums_sha256=digest(ROOT/'Saved/Research/TempestMappings54/enums-numeric.json'),
    artifacts=files,preview=dict(path=str(png.relative_to(ROOT)),sha256=digest(png)),
    editor_inventory=dict(file=str(editor_path.relative_to(ROOT)),sha256=digest(editor_path),
        result=editor['status'],actor_count=editor['actor_count'],loaded_levels=editor['levels'],
        actor_classes=dict(collections.Counter(actor['class'] for actor in editor['actors'])),
        container_instances=len(containers),
        process_exit_code=1,limitation='JSON inventory completed; process also logged three missing airplane textures and TempestFieldVolume ensure !IsRunningCommandlet(). This is not a clean whole-map validation.'),
    import58=imported,
    limitations=['One isolated container imported, not the complete map or active HeroHour CityHero.',
                 'GLB/FBX share measured bounds and triangle count but differ in UV/vertex representation.',
                 'Original-albedo Blender baseline; Unreal baseline adds normal map. Full Tempest shader graph not reconstructed.',
                 'Native cooked texture PNG export asserted in ImageCore; successful texture decoding uses CUE4Parse instead.'])
(OUT/'Transfer-Checks.json').write_text(json.dumps(summary,indent=2)+'\n',encoding='utf-8')
for name,data in [('Native54-Container.json',native),('Blender-Geometry.json',visual),('Container-InScene.json',containers)]:
    (OUT/name).write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8')
print(json.dumps(dict(result=summary['result'],exported_files=len(files),unreal58_import='PASS',container_instances=len(containers))))
