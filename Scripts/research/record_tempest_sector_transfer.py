"""Archive bounded transfer metadata and hashes, never game geometry/textures."""
import hashlib
import json
from pathlib import Path

root = Path(__file__).resolve().parents[2]
base = root / 'Saved/Research/TempestSectorTransfer'
out = root / 'docs/verification/2026-10-17/sector-transfer'
out.mkdir(parents=True, exist_ok=True)
files = {
    'selection': base/'FullSector.json',
    'export': base/'Export-01/Probe.json',
    'bindings': base/'MaterialBindings-06/Bindings.json',
    'components': base/'ComponentVerification54.json',
    'import': base/'Unreal58/Sector-Import58.json',
    'blender': base/'VisualQA/OriginalSector-Verification.json',
}
reports = {key: json.loads(path.read_text(encoding='utf-8')) for key,path in files.items()}
for key in ('export','bindings','components','import','blender'):
    assert reports[key]['result'] == 'PASS', key
assert reports['import']['actor_count'] == 38
assert reports['import']['mesh_count'] == 16
assert reports['import']['albedo_complete']
assert reports['components']['actor_transform_sufficient_for_all']
assert len(reports['components']['components']) == 38
assert len(reports['components']['landscape_components']) == 64
assert reports['components']['source_selection_sha256'] == hashlib.sha256(files['selection'].read_bytes()).hexdigest()
assert reports['import']['source_selection_sha256'] == hashlib.sha256(files['selection'].read_bytes()).hexdigest()
report = dict(result='PASS_BOUNDED_SECTOR_TRANSFER', actor_count=38, unique_meshes=16,
              resolved_source_materials=len(reports['bindings']['materials']),
              exported_textures=len(reports['bindings']['textures']),
              imported_materials=reports['import']['material_count'],
              albedo_complete=True, source_components_verified=38,
              landscape_bounds_only=64, isolated_map=reports['import']['map'],
              instantiated_triangles=reports['blender']['instantiated_triangles'],
              limitations=['Not integrated in HeroHour main gameplay map.',
                           'Original albedo/normal baseline, not complete Tempest shader.',
                           'Landscape bounds inspected; landscape geometry not transferred.',
                           'UE5.4 metadata probe passed its checks but native commandlet reports TempestFieldVolume ensure and missing unrelated assets.',
                           'Blender image is independent visual QA, not an Unreal gameplay screenshot.'],
              evidence={key:dict(path=str(path),sha256=hashlib.sha256(path.read_bytes()).hexdigest()) for key,path in files.items()})
(out/'Transfer-Checks.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
(out/'Original-Actor-Selection.json').write_text(files['selection'].read_text(encoding='utf-8'),encoding='utf-8')
print(json.dumps({key:report[key] for key in ('result','actor_count','unique_meshes','resolved_source_materials','instantiated_triangles')}))
