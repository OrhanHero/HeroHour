"""Build original, restrained seamless PBR maps for HeroHour's RTS district.

All noise is filtered on a periodic domain. Normals use DirectX tangent space:
red is -dH/dX, green is +dH/dY (the OpenGL green channel is inverted).
Run using Python with numpy and Pillow; no downloads or external source art.
"""
from pathlib import Path
import json
import numpy as np
from PIL import Image, ImageDraw

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'Art/Textures/DistrictV3'
SIZE = 1024
SEED = 241014

def field(rng, radius):
    noise = rng.standard_normal((SIZE, SIZE))
    freq = np.fft.fftfreq(SIZE)
    filt = np.exp(-2 * np.pi ** 2 * radius ** 2 * (freq[:, None] ** 2 + freq[None, :] ** 2))
    values = np.fft.ifft2(np.fft.fft2(noise) * filt).real
    return values / max(values.std(), 1e-8)

def save(name, color, roughness, height, strength):
    folder = OUT / name
    folder.mkdir(parents=True, exist_ok=True)
    albedo = np.uint8(np.clip(color, 0, 255))
    rough = np.uint8(np.clip(roughness, 0, 1) * 255)
    dx = (np.roll(height, -1, 1) - np.roll(height, 1, 1)) * .5 * strength
    dy = (np.roll(height, -1, 0) - np.roll(height, 1, 0)) * .5 * strength
    normal = np.stack((-dx, dy, np.ones_like(dx)), axis=-1)
    normal /= np.linalg.norm(normal, axis=-1, keepdims=True)
    normal = np.uint8(np.clip(normal * .5 + .5, 0, 1) * 255)
    Image.fromarray(albedo).save(folder / f'T_{name}_Albedo.png')
    Image.fromarray(normal).save(folder / f'T_{name}_Normal.png')
    Image.fromarray(rough).save(folder / f'T_{name}_Roughness.png')
    return albedo, rough

def pebbles(rng, count, radius_range):
    """Periodic rounded mineral fragments, with independent mineral colours.

    Local stamps avoid full-image distance fields and preserve a continuous
    seam even where a pebble straddles a tile boundary. No baked shadows.
    """
    height = np.zeros((SIZE, SIZE), dtype=float)
    pigment = np.zeros((SIZE, SIZE, 3), dtype=float)
    coverage = np.zeros_like(height)
    minerals = np.array([[27, 21, 12], [-22, -20, -17], [40, 36, 24],
                         [12, 6, -3], [-9, -3, 1]], dtype=float)
    for _ in range(count):
        cx, cy = rng.integers(0, SIZE, 2)
        radius = rng.uniform(*radius_range)
        extent = int(np.ceil(radius * 1.3))
        sy, sx = np.mgrid[-extent:extent+1, -extent:extent+1]
        angle = rng.uniform(0, np.pi)
        u = sx * np.cos(angle) + sy * np.sin(angle)
        v = -sx * np.sin(angle) + sy * np.cos(angle)
        distance = np.sqrt((u / radius) ** 2 + (v / (radius * rng.uniform(.55, .9))) ** 2)
        edge = np.clip((1.0 - distance) * 3.0, 0, 1)
        bulge = np.sqrt(np.clip(1.0 - distance ** 2, 0, 1)) * rng.uniform(.5, 1.15)
        yy = (cy + sy) % SIZE
        xx = (cx + sx) % SIZE
        current = height[yy, xx]
        replace = bulge > current
        height[yy, xx] = np.maximum(current, bulge)
        coverage[yy, xx] = np.where(replace, edge, coverage[yy, xx])
        mineral = minerals[rng.integers(len(minerals))] * rng.uniform(.65, 1.1)
        pigment[yy, xx] = np.where(replace[..., None], edge[..., None] * mineral,
                                    pigment[yy, xx])
    return height, pigment, coverage

def main():
    OUT.mkdir(parents=True, exist_ok=True)
    rng = np.random.default_rng(SEED)
    definitions = {
        'Sand': ((176, 145, 105), 4.0, .87, .28),
        'Asphalt': ((57, 59, 60), 3.0, .88, .27),
        'Concrete': ((143, 140, 128), 4.0, .80, .23),
        'Plaster': ((161, 153, 134), 3.0, .84, .16),
        'Metal': ((55, 60, 61), 2.0, .59, .10),
        'Gravel': ((137, 116, 87), 4.0, .89, 1.7),
        'CompactedDirt': ((146, 121, 87), 4.0, .85, .9),
    }
    previews = []
    metadata = {'generator': 'build_district_textures.py', 'seed': SEED,
                'resolution': SIZE, 'source': 'Original synthesized periodic fields; no source game art',
                'normal_convention': 'DirectX; +X right, +Y tangent down; green inverted relative to OpenGL',
                'color_space': {'Albedo': 'sRGB', 'Normal': 'linear', 'Roughness': 'linear'},
                'metallic': '0 for all except exposed-metal wear; Metal is painted metal and uses 0',
                'materials': {}}
    y, x = np.mgrid[:SIZE, :SIZE]
    for name, (base, meters, rough_base, strength) in definitions.items():
        broad, middle, fine, micro = (field(rng, radius) for radius in (70, 14, 2, .55))
        h = .15 * middle + .38 * fine + .16 * micro
        variation = .9 * broad + 1.5 * middle + 2.0 * fine
        if name == 'Sand':
            # Integer cycles and periodic warping keep the entire tile seamless.
            phase = 2 * np.pi * (13 * y + 2 * x) / SIZE + .25 * broad
            ripple = np.sin(phase)
            variation = 2.1 * broad + 1.6 * fine + .8 * ripple
            h = .40 * fine + .14 * micro + .65 * ripple
        elif name == 'Asphalt':
            aggregate = np.maximum(fine - .4, 0)
            variation = .75 * broad + 2.0 * aggregate + .8 * micro
            h = .42 * fine + .28 * micro
        elif name == 'Concrete':
            pores = np.minimum(fine + 1.2, 0)
            variation = 1.9 * broad + 1.1 * middle + 1.8 * pores
            h = .2 * fine + .36 * pores + .1 * micro
        elif name == 'Plaster':
            variation = 1.2 * broad + .8 * middle + .8 * fine
            h = .34 * fine + .14 * micro
        elif name == 'Metal':
            variation = .65 * broad + .45 * fine
            h = .2 * fine + .13 * micro
        elif name in ('Gravel', 'CompactedDirt'):
            variation = 3.1 * broad + 2.2 * middle + 1.6 * fine
            h = .18 * fine + .10 * micro
        color = np.array(base)[None, None, :] + variation[:, :, None]
        # Mild warm mineral variation; no baked directional lighting.
        if name in ('Sand', 'Concrete', 'Plaster'):
            color += broad[:, :, None] * np.array([.55, .15, -.35])
        roughness = rough_base + .009 * middle + .012 * fine
        if name in ('Gravel', 'CompactedDirt'):
            stone_height, pigment, stone_mask = pebbles(
                rng, 5800 if name == 'Gravel' else 2200,
                (1.3, 8.5) if name == 'Gravel' else (1.1, 5.0))
            # Irregular soil exposure means the gravel is not an evenly packed
            # chessboard. Dirt compaction has subtle low spots rather than cracks.
            exposure = np.clip(.8 + .25 * middle + .1 * broad, .15, 1)
            h += stone_height * exposure
            color += pigment * exposure[..., None]
            roughness -= stone_mask * .025
            if name == 'CompactedDirt':
                compaction = np.clip(-.6 * middle - .25 * broad, 0, 1)
                color -= compaction[..., None] * np.array([4.5, 4.2, 3.7])
                roughness -= compaction * .035
                h -= compaction * .35
        albedo, rough = save(name, color, roughness, h, strength)
        metadata['materials'][name] = {
            'tile_meters': meters, 'nominal_albedo_srgb_8bit': base,
            'mean_albedo_srgb_8bit': np.round(albedo.mean(axis=(0, 1)), 2).tolist(),
            'albedo_channel_range': [[int(albedo[..., c].min()), int(albedo[..., c].max())] for c in range(3)],
            'roughness_mean': float(rough.mean() / 255), 'normal_strength': strength,
            'files': {role: f'{name}/T_{name}_{role}.png' for role in ('Albedo', 'Normal', 'Roughness')},
            'recommended_tint_srgb': [1.0, 1.0, 1.0],
        }
        previews.append((name, albedo))
    (OUT / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n', encoding='utf-8')
    sheet = Image.new('RGB', (len(previews) * 256, 294), (22, 24, 25))
    draw = ImageDraw.Draw(sheet)
    for i, (name, albedo) in enumerate(previews):
        sheet.paste(Image.fromarray(albedo).resize((256, 256)), (i * 256, 0))
        draw.text((i * 256 + 12, 267), name, fill=(220, 220, 214))
    preview = ROOT / 'Art/Previews/DistrictV3-Textures.png'
    preview.parent.mkdir(parents=True, exist_ok=True)
    sheet.save(preview)
    print(json.dumps({'output': str(OUT), 'materials': list(definitions), 'preview': str(preview)}))

if __name__ == '__main__':
    main()
