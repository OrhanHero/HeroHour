"""Bounded read-only UI evidence; no asset extraction or game execution."""
import hashlib
import json
from pathlib import Path
from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
TARGET = Path('F:/Tempest-Rising-SteamRIP.com/Tempest Rising')
OUT = ROOT / 'docs/verification/2026-10-16/tempest-ui'
OUT.mkdir(parents=True, exist_ok=True)
image = ROOT / 'Saved/Research/TempestRising/reference-live-base.png'
raw = image.read_bytes()
with Image.open(image) as reference:
    width, height = reference.size
    image_format = reference.format
assert (width, height) == (2560, 1440)
ui = TARGET / 'MapEditorProject/Content/Tempest/UI'
files = []
for path in sorted(ui.rglob('*')):
    if path.is_file():
        data = path.read_bytes()
        files.append({'path': str(path.relative_to(TARGET)).replace('\\', '/'),
                      'bytes': len(data), 'sha256': hashlib.sha256(data).hexdigest()})
shipping = TARGET / 'Tempest/Binaries/Win64/Tempest-Win64-Shipping.exe'
data = shipping.read_bytes()
digest = hashlib.sha256(data).hexdigest()
assert digest == 'c81828f4749ae1bbac73106f905a81c3ea1bff9cee7300291c3c390def04be56'
# Mere string/declaration evidence, deliberately not inferred widget behavior.
terms = ['ProductionQueue', 'BuildMenu', 'Radar', 'Tooltip', 'UITheme',
         'ThemeColor', 'Hovered', 'Disabled', 'SelectedUnit']
matches = []
for term in terms:
    for encoding in ('ascii', 'utf-16-le'):
        needle = term.encode(encoding)
        offsets = []
        start = 0
        while len(offsets) < 16:
            pos = data.find(needle, start)
            if pos < 0:
                break
            offsets.append(pos)
            start = pos + len(needle)
        matches.append({'term': term, 'encoding': encoding, 'file_offsets': offsets,
                        'limit': 16, 'saturated': len(offsets) == 16})
evidence = {
    'method': 'read-only SHA-256, Pillow image header, complete selected UI directory inventory, bounded literal string search',
    'screenshot': {'path': str(image.relative_to(ROOT)).replace('\\', '/'),
                   'sha256': hashlib.sha256(raw).hexdigest(), 'bytes': len(raw),
                   'width': width, 'height': height, 'actual_format': image_format,
                   'provenance': 'Existing 2026-10-14 Veti runtime frame; docs/WORKLOG_2026-10-14.md and TempestRising-Systems-Evidence.md',
                   'limits': 'One retained frame; timer 2:08; unlimited credits; no new runtime actions; not queue/tooltip animation proof'},
    'editor_ui_inventory': files,
    'shipping': {'sha256': digest, 'bytes': len(data), 'path': str(shipping),
                 'matches': matches},
    'limits': ['No widget tree, font point size or UMG anchors decoded.',
               'No asset pixels or game models extracted.',
               'Approximate screen measurements are not original layout constants.',
               'String presence proves only bytes, not behavior.']
}
(OUT / 'reference-inventory.json').write_text(json.dumps(evidence, indent=2), encoding='utf-8')
print(f'TEMPEST_UI_REFERENCE_PASS {width}x{height}; {len(files)} UI files; {len(matches)} bounded searches')
