"""Original authored tactical map from HeroHour's existing level generators.

This is a map drawing, not a Unreal screenshot. World X increases right and
world Y increases down, matching HeroHUD's MapX/MapY and click conversion.
The visible gameplay rectangle is level in build_terrain_slice_v3.py; no
fictional hills are drawn inside it. Banks use the actual EnvironmentV2 shape.
Units, ownership, resources, camera and rally markers remain runtime overlays.
"""
from pathlib import Path
import hashlib
import json
import math
import random

import numpy as np
from PIL import Image, ImageDraw, ImageFilter


ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "Art/UI/ConceptV3"
NAME = "T_UI_CityHeroMap_V3"
WIDTH, HEIGHT, AA = 1024, 768, 2
EX, EY = 12000, 9000
OUT.mkdir(parents=True, exist_ok=True)
W, H = WIDTH*AA, HEIGHT*AA
y, x = np.mgrid[0:H, 0:W]
wx = ((x+.5)/W*2-1)*EX
wy = ((y+.5)/H*2-1)*EY


def smooth_field(seed, cells_x, cells_y):
    rng = np.random.default_rng(seed)
    source = Image.fromarray(rng.integers(0, 255, (cells_y, cells_x), dtype=np.uint8), "L")
    return np.asarray(source.resize((W, H), Image.Resampling.BICUBIC), dtype=np.float32)/255


def material_sample(name, tint=(1, 1, 1)):
    path = ROOT/f"Art/Textures/DistrictV3/{name}/T_{name}_Albedo.png"
    tex = np.asarray(Image.open(path).convert("RGB"), dtype=np.float32)
    # All three source materials tile every 4m in the actual layered floor.
    iy = np.floor((wy/400)*tex.shape[0]).astype(np.int32) % tex.shape[0]
    ix = np.floor((wx/400)*tex.shape[1]).astype(np.int32) % tex.shape[1]
    return tex[iy, ix] * np.array(tint, dtype=np.float32)


# The base preserves the muted source material palette. Coarse masks represent
# ground material variation; they do not imply height or changed navigation.
gravel_mask = np.clip((smooth_field(241014, 32, 24)-.54)*2.4, 0, 1)[:, :, None]
dirt_mask = np.clip((smooth_field(241015, 14, 11)-.48)*1.4, 0, 1)[:, :, None]
tint = (.62, .82, 1.06)
soil = material_sample("Sand", tint)
soil = soil*(1-gravel_mask)+material_sample("Gravel", tint)*gravel_mask
soil = soil*(1-dirt_mask)+material_sample("CompactedDirt", tint)*dirt_mask
river = np.abs(wx+3000) <= 700
water_variation = .90 + .14*smooth_field(241016, 10, 24)
water = np.array((29, 76, 82), dtype=np.float32)[None, None, :] * water_variation[:, :, None]
soil = np.where(river[:, :, None], water, soil)

# EnvironmentV2 banks occupy the blocked river band. Evaluate their real mesh
# equation, including the deliberate openings around the two bridge corridors.
metres_y = wy/100
bridge_factor = np.clip((np.minimum(np.abs(metres_y-32), np.abs(metres_y+32))-7)/3, 0, 1)
bank_width = (2.2+.8*np.sin(metres_y*.13)+.5*np.sin(metres_y*.43))*bridge_factor*100
distance_inward = 700-np.abs(wx+3000)
bank = river & (distance_inward >= 0) & (distance_inward < bank_width)
bank_t = np.clip(distance_inward/np.maximum(bank_width, 1), 0, 1)
bank_z = -.08-bank_t-1.1*bank_t*bank_t
bank_shade = .96-.28*bank_t
bank_color = np.stack((117*bank_shade, 111*bank_shade, 92*bank_shade), axis=2)
soil = np.where(bank[:, :, None], bank_color, soil)
image = Image.fromarray(np.clip(soil, 0, 255).astype(np.uint8), "RGB")
ground = image.copy()
draw = ImageDraw.Draw(image)
PAVEMENT = (139, 141, 130)
ASPHALT = (57, 60, 59)
KERB = (111, 119, 117)
ROOF = (147, 146, 131)
ROOF_DARK = (102, 113, 111)
records = []


def xy(px, py):
    return ((px+EX)/(2*EX)*W, (py+EY)/(2*EY)*H)


def rectangle(cx, cy, width, height, color, outline=None):
    points = (*xy(cx-width/2, cy-height/2), *xy(cx+width/2, cy+height/2))
    draw.rectangle(points, fill=color, outline=outline, width=AA if outline else 1)


def path(points, width, color):
    # A measured ribbon matches the mesh generator and stays solid at bends.
    # ImageDraw's broad curved-line join can leave small seams between segments.
    left, right = [], []
    for i, (px, py) in enumerate(points):
        before, after = points[max(0, i-1)], points[min(len(points)-1, i+1)]
        dx, dy = after[0]-before[0], after[1]-before[1]
        length = max(.001, math.hypot(dx, dy))
        nx, ny = dy/length, -dx/length
        left.append(xy(px+nx*width/2, py+ny*width/2))
        right.append(xy(px-nx*width/2, py-ny*width/2))
    draw.polygon(left+list(reversed(right)), fill=color)


def ellipse(cx, cy, width, height, color, outline=None):
    draw.ellipse((*xy(cx-width/2, cy-height/2), *xy(cx+width/2, cy+height/2)),
                 fill=color, outline=outline, width=AA if outline else 1)


# Base streets from create_cityhero_map.py, retaining their actual physical widths.
for road_x in (-5000, 0, 7500):
    rectangle(road_x, 0, 550, 17000, ASPHALT)
for road_y in (-6000, 0, 6000):
    rectangle(-7850, road_y, 7300, 450, ASPHALT)
    rectangle(4700, road_y, 15800, 450, ASPHALT)
for road_y in (-3200, 3200):
    rectangle(0, road_y, 23500, 1250, ASPHALT)
for road_x in (750, 2250, 3750, 5250, 6750):
    rectangle(road_x, 0, 380, 10600, ASPHALT)
for road_y in (-5250, -3750, -2250, -750, 750, 2250, 3750, 5250):
    rectangle(3750, road_y, 6500, 380, ASPHALT)

# Southern road and city sidewalk modules from import_district_v3.py.
for road_x in range(-10800, 10801, 1200):
    if -4300 < road_x < -1700:
        continue
    rectangle(road_x, -3200, 1200, 1155, KERB)
    rectangle(road_x, -3200, 1200, 900, ASPHALT)
for road_x in (750, 2250, 3750, 5250, 6750):
    for road_y in (-4500, -3000, -1500, 0, 1500, 3000, 4500):
        rectangle(road_x, road_y, 652, 1120, KERB)
        rectangle(road_x, road_y, 380, 1120, ASPHALT)
for road_y in (-5250, -3750, -2250, -750, 750, 2250, 3750, 5250):
    for road_x in (1500, 3000, 4500, 6000):
        rectangle(road_x, road_y, 1120, 652, KERB)
        rectangle(road_x, road_y, 1120, 380, ASPHALT)

# A tactical drawing must not imply extra traversable river crossings. The old
# east-road slabs overlap water at Y=0/+/-6000; navigation blocks those portions.
# Preserve the actual blocked band, then draw the two legitimate bridge spans.
image.paste(ground, (0, 0), Image.fromarray(river.astype(np.uint8)*255, "L"))
draw = ImageDraw.Draw(image)
# Bridges draw over the river, but keep the source's two corridors and span.
for bridge_y in (-3200, 3200):
    rectangle(-3000, bridge_y, 2000, 1400, PAVEMENT)
    rectangle(-3000, bridge_y, 2000, 1240, ASPHALT)
    for edge in (-645, 645):
        path([(-4000, bridge_y+edge), (-2000, bridge_y+edge)], 30, (168, 174, 160))
    for dash_x in range(-3900, -2099, 300):
        rectangle(dash_x, bridge_y, 140, 13, (179, 181, 153))
    for dash_x in range(-11000, 11500, 700):
        if abs(dash_x+3000)>1000:
            rectangle(dash_x, bridge_y, 250, 12, (179, 181, 153))

# Civic plots: actual rotated footprint at XY scale2. No faction-coloured regions.
for col, city_x in enumerate((1500, 3000, 4500, 6000)):
    for row, city_y in enumerate((-4500, -3000, -1500, 0, 1500, 3000, 4500)):
        kind = 2 if col in (1, 2) and abs(city_y) <= 1500 else (row+col)%2
        rectangle(city_x, city_y, 1020, 1020, PAVEMENT)
        rectangle(city_x, city_y, 780, 780, ROOF_DARK, (70, 80, 78))
        crown = 560 if kind == 2 else 540 if kind == 1 else 530
        rectangle(city_x, city_y, crown, crown, ROOF, (174, 173, 156))
        for dx in (-145, 145):
            rectangle(city_x+dx, city_y+95, 155, 140, (72, 86, 87))
        records.append({"kind":"civic", "xy_cm":[city_x, city_y], "variant":kind})

# Residential lanes and the 64 real house footprints from the original map.
for lane_x in (-10150, -8850, -7600, -6350, -5100):
    rectangle(lane_x, 4800, 250, 6100, ASPHALT)
for lane_y in (2400, 3650, 4900, 6150, 7400):
    rectangle(-7600, lane_y, 5600, 250, ASPHALT)
for house_x in (-9500, -8250, -7000, -5750):
    for house_y in (3000, 4250, 5500, 6750):
        for dx, dy in ((-190, -190), (-190, 190), (190, -190), (190, 190)):
            rectangle(house_x+dx, house_y+dy, 335.5, 247.5, (163, 164, 143), (98, 110, 105))
            rectangle(house_x+dx-55, house_y+dy, 66, 66, ROOF_DARK)
            records.append({"kind":"house", "xy_cm":[house_x+dx, house_y+dy]})

# Neutral, environmental landmarks. Their silhouettes convey world geography.
rectangle(-9000, -6000, 2200, 1800, ASPHALT)
rectangle(-9000, -6600, 1500, 500, ROOF_DARK, KERB)
for tower_x in (-9500, -8500):
    ellipse(tower_x, -6000, 760, 760, (170, 174, 166), (80, 99, 100))
    ellipse(tower_x, -6000, 430, 430, (57, 77, 79))
rectangle(3000, 7000, 3200, 2500, ASPHALT)
ellipse(3000, 7000, 2600, 2000, (175, 174, 157), (65, 80, 80))
ellipse(3000, 7000, 2110, 1510, (84, 103, 85), (116, 128, 108))
rectangle(3000, 7000, 1700, 1100, (83, 97, 65))
for refinery_x, refinery_y in ((8900, 3900), (10300, 5600), (8700, 7000)):
    rectangle(refinery_x, refinery_y, 1170, 975, PAVEMENT, ROOF_DARK)
    for dx, dy in ((-325, -260), (-325, 260), (0, 260)):
        ellipse(refinery_x+dx, refinery_y+dy, 325, 325, (145, 155, 145), (69, 90, 88))
    for dx, dy in ((130, -260), (325, -195), (325, 130)):
        ellipse(refinery_x+dx, refinery_y+dy, 130, 130, (181, 177, 153), ROOF_DARK)
    records.append({"kind":"environment_refinery", "xy_cm":[refinery_x, refinery_y]})
rectangle(9500, -5100, 4500, 1400, PAVEMENT)
rectangle(9500, -6500, 4500, 700, ASPHALT)
for runway_x in range(7600, 11500, 450):
    rectangle(runway_x, -6500, 180, 22, (172, 177, 157))
for hangar_x in (8200, 9300, 10400):
    rectangle(hangar_x, -4650, 790, 620, ROOF_DARK, PAVEMENT)

# Aprons are static ground geometry, not headquarters or producer markers.
for base_x in (-8000, 8000):
    rectangle(base_x, 0, 1800, 1800, ASPHALT, KERB)
for pad_x, pad_y in ((-7500, 1600), (7500, 1600), (7600, -1600)):
    rectangle(pad_x, pad_y, 900, 900, ASPHALT, KERB)
approach = []
for i in range(81):
    t = i/80; u = 1-t
    approach.append((-8000-3*u*t*t*400,
                     u**3*(-3200)+3*u*u*t*(-2300)+3*u*t*t*(-1500)+t**3*(-900)))
path(approach, 830, KERB)
path(approach, 600, ASPHALT)

# Ground cover uses exactly the same positions/skip conditions as the scene pass.
rng = random.Random(241014)
for i in range(1600):
    foliage_x = rng.uniform(-11500, 11500); foliage_y = rng.uniform(-8400, 8400)
    if abs(foliage_x+3000)<1100 or abs(foliage_y-3200)<850 or abs(foliage_y+3200)<850: continue
    if any(abs(foliage_x-road)<450 for road in (-8000, -5000, 0, 7500)): continue
    if any(abs(foliage_y-road)<450 for road in (-6000, 0, 6000)): continue
    if 0<foliage_x<7300 and abs(foliage_y)<6100: continue
    if -10800<foliage_x<-4700 and 1900<foliage_y<8000: continue
    if any(math.hypot(foliage_x-px, foliage_y-py)<1300 for px,py in ((-8000,0),(8000,0),(3000,7000),(-9000,-6000),(8900,3900),(10300,5600),(8700,7000))): continue
    if any(math.hypot(foliage_x-px, foliage_y-py)<650 for px,py in ((-7500,1600),(7500,1600),(7600,-1600))): continue
    size = rng.uniform(.65, 1.2); rng.uniform(0,360)
    if i%5<2:
        ellipse(foliage_x, foliage_y, 120*size, 75*size, (88, 94, 86))
    elif i%3 == 0:
        ellipse(foliage_x, foliage_y, 116*size, 108*size, (91, 105, 63))
    else:
        ellipse(foliage_x, foliage_y, 60*size, 58*size, (127, 124, 89))

image = image.filter(ImageFilter.GaussianBlur(.4)).resize((WIDTH, HEIGHT), Image.Resampling.LANCZOS).convert("RGBA")
image.save(OUT/(NAME+".png"), optimize=True)
# Validate the integration-critical orientation against the rendered pixels,
# rather than only reporting a coordinate formula. No simulated markers are baked.
def output_pixel(px, py):
    return [round((px+EX)/(2*EX)*(WIDTH-1)), round((py+EY)/(2*EY)*(HEIGHT-1))]

probe_points = {
    "river_north": (-3000, -8000), "river_centre": (-3000, 0),
    "river_south": (-3000, 8000),
    "north_bridge": (-3200, -3200), "south_bridge": (-3200, 3200),
    "northwest_power_plant": (-9000, -6000), "southeast_stadium": (3000, 7000),
}
probe_pixels = {name: output_pixel(*point) for name, point in probe_points.items()}
assert image.size == (1024, 768) and image.getextrema()[3] == (255, 255)
assert probe_pixels["north_bridge"][1] < probe_pixels["south_bridge"][1]
for name in ("river_north", "river_centre", "river_south"):
    r, g, b, a = image.getpixel(tuple(probe_pixels[name]))
    assert r < 45 and g > r*1.6 and b > r*1.6, (name, (r,g,b,a))
for name in ("north_bridge", "south_bridge"):
    r, g, b, a = image.getpixel(tuple(probe_pixels[name]))
    assert max(r,g,b)-min(r,g,b) < 20, (name, (r,g,b,a))
assert len([record for record in records if record["kind"] == "civic"]) == 28
assert len([record for record in records if record["kind"] == "house"]) == 64
metadata_path = OUT/"metadata.json"
metadata = json.loads(metadata_path.read_text())
metadata["texture_sizes"][NAME] = [WIDTH, HEIGHT]
metadata["tactical_map"] = {
        "texture": NAME, "kind":"Authored tactical drawing from own level data; not an engine capture",
    "world_extents_cm":[EX,EY], "uv_world_corners_cm":[[-EX,-EY],[EX,EY]],
    "orientation":"X increases right, Y increases down; identical to HeroHUD MapX/MapY",
    "blocked_road_clipping":"Legacy east-road slabs are clipped to the blocked river band outside Y=+/-3200 bridge corridors; only traversable crossings are represented",
    "height_model":"Authored interior Z=-0.05m, blocked river bed Z=-3.5m, evaluated bank equation; exterior ridges excluded by gameplay crop",
    "source_scripts":["create_cityhero_map.py","import_district_v3.py","build_district_streets.py","build_terrain_slice_v3.py","build_environment_v2.py"],
    "dynamic_layers_excluded":["units","runtime buildings","ownership","resource markers","camera","rally points","fog"],
    "civic_count":28,"house_count":64,"environment_refinery_count":3,
    "qa":{"dimensions":[WIDTH,HEIGHT], "fully_opaque":True,
          "coordinate_pixel_probes":probe_pixels, "river_and_bridge_pixel_checks":"PASS"},
    "sha256":hashlib.sha256((OUT/(NAME+".png")).read_bytes()).hexdigest()
}
metadata_path.write_text(json.dumps(metadata, indent=2)+"\n", encoding="utf-8")
print("UI_TACTICAL_MAP_V3_BUILT", json.dumps(metadata["tactical_map"]))
