"""Build original, deterministic HUD artwork for the HeroHour concept UI.

These are UI graphics rather than rendered game assets. All paths and atlas
coordinates are exported with the textures; preview output is explicitly a
material-library proof, not an in-game screenshot. Requires Pillow and numpy.
"""
from pathlib import Path
import json

import numpy as np
from PIL import Image, ImageDraw, ImageFont


ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Art/UI/ConceptV3"
SCALE = 4
OUT.mkdir(parents=True, exist_ok=True)


def clipped_box(x0, y0, x1, y1, cut):
    return [(x0 + cut, y0), (x1 - cut, y0), (x1, y0 + cut),
            (x1, y1 - cut), (x1 - cut, y1), (x0 + cut, y1),
            (x0, y1 - cut), (x0, y0 + cut)]


def scaled_points(points):
    return [(round(x * SCALE), round(y * SCALE)) for x, y in points]


def polygon(draw, points, fill, outline=None, width=1):
    draw.polygon(scaled_points(points), fill=fill)
    if outline:
        draw.line(scaled_points(points + [points[0]]), fill=outline,
                  width=round(width * SCALE), joint="curve")


def line(draw, points, fill, width=1):
    draw.line(scaled_points(points), fill=fill, width=round(width * SCALE),
              joint="curve")


def metal_frame(heavy=False):
    size = 256
    image = Image.new("RGBA", (size * SCALE, size * SCALE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    visible = 24 if heavy else 20
    # Nested plates form real separate bevel faces, avoiding a flat outline.
    layers = [(0, 10, (5, 8, 9, 255)), (1, 10, (78, 93, 96, 255)),
              (2, 9, (144, 155, 154, 255)), (3.5, 9, (45, 57, 62, 255)),
              (5, 8, (21, 29, 33, 255)), (7, 7, (74, 89, 96, 255)),
              (8.5, 7, (32, 44, 49, 255)), (visible - 5, 5, (19, 26, 29, 255)),
              (visible - 3, 5, (117, 136, 139, 255)),
              (visible - 2, 4, (38, 54, 61, 255))]
    for inset, cut, color in layers:
        polygon(draw, clipped_box(inset, inset, size - 1 - inset,
                                 size - 1 - inset, cut), color)
    if heavy:
        # Wider outer rim suitable for the map without ornament in the map area.
        line(draw, [(11, 14), (14, 11), (242, 11)], (165, 179, 181, 255), .8)
        line(draw, [(244, 14), (244, 241), (241, 244)], (7, 12, 15, 255), 2)
    # Directional highlight: upper-left faces catch light, lower-right recede.
    line(draw, [(11, 2), (244, 2), (252, 10)], (184, 191, 185, 255), .7)
    line(draw, [(2, 11), (2, 244), (10, 252)], (106, 125, 130, 255), .8)
    line(draw, [(254, 12), (254, 244), (244, 254), (12, 254)], (3, 7, 9, 255), 2)
    # Small electric-cyan inlays, limited to trim rather than a glowing box.
    for points in [[(37, visible - 2.7), (79, visible - 2.7)],
                   [(size - visible + 1.5, 61), (size - visible + 1.5, 94)],
                   [(16, 11), (11, 16)], [(245, 240), (240, 245)]]:
        line(draw, points, (71, 131, 139, 255), 1.2)
        line(draw, points, (118, 190, 194, 255), .4)
    for x, y in [(11, 11), (244, 11), (11, 244), (244, 244)]:
        r = 2.0
        draw.ellipse(tuple(round(v * SCALE) for v in (x-r, y-r, x+r, y+r)),
                     fill=(4, 9, 12, 255))
        draw.ellipse(tuple(round(v * SCALE) for v in (x-r+0.65, y-r+0.5,
                                                     x+r-0.5, y+r-0.5)),
                     fill=(134, 149, 151, 255))
        line(draw, [(x-.8, y+.4), (x+.8, y-.4)], (23, 32, 35, 255), .6)
    # Clear the centre. The edges can be stretched independently of the corners.
    polygon(draw, clipped_box(visible, visible, size - 1 - visible,
                             size - 1 - visible, 3), (0, 0, 0, 0))
    image = image.resize((size, size), Image.Resampling.LANCZOS)
    # A reproducible fine brush grain is restricted to opaque frame pixels.
    pixels = np.array(image).astype(np.int16)
    rng = np.random.default_rng(7301 + int(heavy))
    horizontal = rng.normal(0, 1.2, (size, 1, 1))
    grain = horizontal + rng.normal(0, .5, (size, size, 1))
    pixels[:, :, :3] = np.clip(pixels[:, :, :3] + grain, 0, 255)
    pixels[pixels[:, :, 3] == 0, :3] = 0
    return Image.fromarray(pixels.astype(np.uint8), "RGBA")


def panel_glass():
    size = 512
    y, x = np.mgrid[0:size, 0:size]
    rng = np.random.default_rng(7307)
    # Smoked glass: barely visible diagonal carbon weave and soft top reflection.
    weave = (((x // 3 + y // 3) % 2) * .55)
    brush = rng.normal(0, .45, (size, 1)) + rng.normal(0, .35, (size, size))
    reflection = 3.5 * np.exp(-y / 170)
    rgb = np.stack([10 + reflection + weave + brush,
                    15 + reflection + weave + brush,
                    17 + reflection + weave + brush], axis=2)
    alpha = np.full((size, size, 1), 244)
    return Image.fromarray(np.clip(np.concatenate([rgb, alpha], axis=2),
                                    0, 255).astype(np.uint8), "RGBA")


def title_plate():
    width, height = 256, 64
    y, x = np.mgrid[0:height, 0:width]
    rng = np.random.default_rng(7319)
    brush = rng.normal(0, .6, (height, 1)) + rng.normal(0, .2, (height, width))
    reflect = 9 * np.exp(-y / 18) + 2 * np.sin(x * .018)
    rgb = np.stack([18 + reflect + brush, 24 + reflect + brush,
                    26 + reflect + brush], axis=2)
    alpha = np.full((height, width, 1), 255)
    result = Image.fromarray(np.clip(np.concatenate([rgb, alpha], axis=2),
                                     0, 255).astype(np.uint8), "RGBA")
    draw = ImageDraw.Draw(result)
    draw.line([(0, 0), (255, 0)], fill=(87, 102, 104, 255), width=1)
    draw.line([(0, 62), (255, 62)], fill=(4, 9, 11, 255), width=2)
    draw.line([(0, 63), (255, 63)], fill=(51, 76, 79, 255), width=1)
    return result


GLYPHS = ["Base", "Defense", "Infantry", "Vehicles", "Special", "Move",
          "Attack", "Stop", "Hold", "Rally", "Cancel", "Repair", "Harvest",
          "Power", "Queue", "Camera"]


def glyph(name):
    # Work in 0..128 vector coordinates, then supersample once at export.
    result = Image.new("RGBA", (128*SCALE, 128*SCALE), (0, 0, 0, 0))
    draw = ImageDraw.Draw(result)
    ink = (211, 224, 224, 255)
    dark = (55, 81, 88, 255)
    def p(points, color=ink):
        polygon(draw, points, color)
    def l(points, width=5, color=ink):
        line(draw, points, color, width)
    def ellipse(box, fill=None, stroke=ink, width=5):
        draw.ellipse(tuple(round(v*SCALE) for v in box), fill=fill,
                     outline=stroke, width=round(width*SCALE))
    if name == "Base":
        p([(24, 97), (24, 53), (41, 44), (41, 24), (63, 24), (63, 43),
           (105, 54), (105, 97)])
        p([(51, 97), (51, 73), (72, 73), (72, 97)], dark)
        for xx in [31, 83, 95]:
            p([(xx, 61), (xx+5, 61), (xx+5, 68), (xx, 68)], dark)
        l([(54, 23), (54, 15)], 3)
    elif name == "Defense":
        l([(64, 104), (30, 87), (23, 59), (23, 26), (64, 18), (105, 26),
           (105, 59), (98, 87), (64, 104)], 6)
        p([(55, 78), (55, 48), (46, 48), (46, 37), (81, 37), (81, 48),
           (73, 48), (73, 78)])
        l([(49, 80), (80, 80)], 6)
    elif name == "Infantry":
        ellipse((51, 15, 76, 40), ink, width=0)
        p([(48, 45), (76, 45), (82, 76), (73, 76), (72, 107), (61, 107),
           (58, 82), (54, 107), (42, 107), (48, 74), (43, 71)])
        l([(77, 49), (94, 63), (110, 62)], 8)
        l([(71, 61), (108, 57)], 4)
    elif name == "Vehicles":
        p([(21, 80), (28, 57), (74, 52), (93, 64), (107, 66), (107, 83),
           (91, 92), (33, 92)])
        p([(45, 53), (48, 40), (79, 40), (83, 52)])
        l([(70, 44), (103, 31)], 5)
        for xx in [35, 53, 73, 94]:
            ellipse((xx-7, 84, xx+7, 98), ink, dark, 2)
    elif name == "Special":
        p([(64, 16), (75, 43), (104, 48), (82, 67), (87, 98),
           (64, 82), (41, 98), (45, 67), (23, 48), (53, 43)])
        p([(66, 36), (48, 66), (62, 66), (58, 84), (80, 54), (66, 54)], dark)
    elif name == "Move":
        p([(64, 15), (78, 34), (68, 34), (68, 60), (94, 60), (94, 50),
           (113, 64), (94, 78), (94, 68), (68, 68), (68, 94), (78, 94),
           (64, 113), (50, 94), (60, 94), (60, 68), (34, 68), (34, 78),
           (15, 64), (34, 50), (34, 60), (60, 60), (60, 34), (50, 34)])
    elif name == "Attack":
        ellipse((31, 31, 97, 97), width=5)
        ellipse((55, 55, 73, 73), ink, width=0)
        for points in [[(64, 16), (64, 44)], [(64, 84), (64, 112)],
                       [(16, 64), (44, 64)], [(84, 64), (112, 64)]]:
            l(points, 5)
    elif name == "Stop":
        p(clipped_box(31, 31, 97, 97, 12))
        p(clipped_box(39, 39, 89, 89, 8), dark)
        p([(48, 48), (80, 48), (80, 80), (48, 80)])
    elif name == "Hold":
        p([(33, 22), (49, 22), (49, 83), (59, 83), (59, 106), (23, 106),
           (23, 83), (33, 83)])
        p([(79, 22), (95, 22), (95, 83), (105, 83), (105, 106), (69, 106),
           (69, 83), (79, 83)])
    elif name == "Rally":
        l([(38, 19), (38, 108)], 7)
        p([(43, 22), (102, 22), (89, 44), (102, 63), (43, 63)])
        l([(25, 107), (51, 107)], 5)
    elif name == "Cancel":
        l([(33, 33), (95, 95)], 9)
        l([(95, 33), (33, 95)], 9)
    elif name == "Repair":
        p([(100, 19), (97, 44), (85, 55), (74, 55), (41, 95),
           (35, 106), (23, 106), (17, 96), (24, 85), (61, 48),
           (60, 34), (75, 18), (82, 18), (75, 35), (86, 44), (100, 19)])
        ellipse((25, 92, 33, 100), dark, width=0)
    elif name == "Harvest":
        p([(39, 54), (52, 29), (70, 23), (89, 53), (74, 82), (51, 82)])
        l([(18, 69), (31, 98), (96, 98), (111, 69)], 6)
        l([(20, 70), (35, 70)], 5)
        l([(94, 70), (110, 70)], 5)
        l([(64, 39), (64, 73)], 3, dark)
    elif name == "Power":
        p([(64, 15), (92, 15), (75, 51), (98, 51), (42, 113),
           (56, 73), (29, 73)])
    elif name == "Queue":
        for yy, xx in [(25, 25), (53, 38), (81, 51)]:
            p(clipped_box(xx, yy, xx+62, yy+20, 3))
            p([(xx+6, yy+6), (xx+12, yy+6), (xx+12, yy+14), (xx+6, yy+14)], dark)
    elif name == "Camera":
        l([(23, 45), (40, 45), (49, 30), (79, 30), (88, 45), (105, 45),
           (105, 99), (23, 99), (23, 45)], 6)
        ellipse((45, 49, 83, 87), width=5)
        l([(29, 22), (16, 22), (16, 35)], 4)
        l([(99, 22), (112, 22), (112, 35)], 4)
    return result.resize((128, 128), Image.Resampling.LANCZOS)


textures = {
    "T_UI_Frame_V3": metal_frame(),
    "T_UI_MinimapFrame_V3": metal_frame(True),
    "T_UI_PanelGlass_V3": panel_glass(),
    "T_UI_TitlePlate_V3": title_plate(),
}
atlas = Image.new("RGBA", (512, 512), (0, 0, 0, 0))
for index, name in enumerate(GLYPHS):
    atlas.alpha_composite(glyph(name), ((index % 4)*128, (index // 4)*128))
textures["T_UI_GlyphAtlas_V3"] = atlas
for name, texture in textures.items():
    texture.save(OUT / (name + ".png"), optimize=True)

metadata = {
    "description": "Original deterministic military HUD artwork; HeroHour-owned graphics.",
    "import_directory": "/Game/HERO/UI/ConceptV3",
    "frame": {"size": [256, 256], "corner_source_px": 32,
              "uv_bands": [0.0, .125, .875, 1.0],
              "metal_visible_px": 20, "minimap_metal_visible_px": 24,
              "center_alpha": 0,
              "recommended_destination_corner_px": {"panel": 18, "minimap": 25,
                                                     "card": 10}},
    "glyph_atlas": {"size": [512, 512], "cell_px": 128,
                    "columns": 4, "rows": 4,
                    "glyphs": [{"name": name, "index": i,
                                "uv": [(i % 4) * .25, (i // 4) * .25, .25, .25]}
                               for i, name in enumerate(GLYPHS)]},
    "texture_sizes": {name: list(texture.size) for name, texture in textures.items()},
}
(OUT / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n", encoding="utf-8")

# Contact sheet proves alpha, trim detail and legibility, without posing as a game capture.
preview = Image.new("RGBA", (1280, 920), (8, 13, 16, 255))
pd = ImageDraw.Draw(preview)
font_path = Path("C:/Windows/Fonts/bahnschrift.ttf")
font = ImageFont.truetype(str(font_path), 24) if font_path.exists() else ImageFont.load_default()
small = ImageFont.truetype(str(font_path), 18) if font_path.exists() else ImageFont.load_default()
pd.text((28, 23), "HERO HOUR  /  CONCEPT V3 HUD MATERIAL LIBRARY", font=font,
        fill=(202, 220, 225, 255))
pd.text((28, 59), "Original artwork preview — not an Unreal Engine screenshot", font=small,
        fill=(118, 154, 163, 255))
for i, name in enumerate(["T_UI_Frame_V3", "T_UI_MinimapFrame_V3"]):
    x0 = 30 + i*300
    for yy in range(256//16):
        for xx in range(256//16):
            val = 30 if (xx+yy)%2 else 48
            pd.rectangle((x0+xx*16, 113+yy*16, x0+(xx+1)*16, 113+(yy+1)*16),
                         fill=(val, val+4, val+7, 255))
    preview.alpha_composite(textures[name], (x0, 113))
    pd.text((x0, 379), "PANEL FRAME" if i == 0 else "MINIMAP METAL RIM", font=small,
            fill=(175, 199, 205, 255))
preview.alpha_composite(textures["T_UI_PanelGlass_V3"].resize((580, 240)), (645, 113))
preview.alpha_composite(textures["T_UI_TitlePlate_V3"].resize((580, 40)), (645, 113))
pd.text((662, 121), "SMOKED GLASS / BRUSHED STEEL TITLE", font=small,
        fill=(193, 212, 217, 255))
pd.text((663, 190), "Primary typography and actual portraits remain runtime content.",
        font=small, fill=(133, 162, 170, 255))
pd.text((663, 223), "Metal rims use 32 px UV corners; centres remain transparent.",
        font=small, fill=(133, 162, 170, 255))
pd.text((663, 269), "Alpha / texture grain / icon silhouette proof", font=small,
        fill=(92, 163, 176, 255))
pd.text((30, 429), "CATEGORY AND ORDER GLYPHS — NEUTRAL SILVER, RUNTIME COLOUR TINT",
        font=small, fill=(177, 201, 206, 255))
for i, name in enumerate(GLYPHS):
    col, row = i % 8, i // 8
    px, py = 30 + col*153, 478 + row*199
    pd.rectangle((px, py, px+127, py+127), fill=(15, 23, 27, 255),
                 outline=(56, 80, 86, 255), width=1)
    preview.alpha_composite(glyph(name), (px, py))
    pd.text((px+3, py+135), name.upper(), font=small, fill=(155, 189, 197, 255))
pd.text((30, 887), "All source generation is deterministic. No game textures or models were copied.",
        font=small, fill=(99, 133, 141, 255))
preview.convert("RGB").save(OUT / "Preview-UI-Materials-V3.png", optimize=True)
print("UI_ART_V3_BUILT", json.dumps(metadata["texture_sizes"]))
