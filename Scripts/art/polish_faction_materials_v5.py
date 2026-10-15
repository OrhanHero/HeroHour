"""Create 20 HeroHour-owned faction material variants without editing source assets.

Run in Unreal Python after DistrictV3 texture import and ConceptV2 material polish.
Root integration assigns variants by source material name, never slot index.
This script does not modify meshes, runtime code, maps, lighting or old materials.
"""
from pathlib import Path
import json

import unreal


ROOT = Path(__file__).resolve().parents[2]
DEST = "/Game/HERO/PresentationV5/Materials"
SOURCE_DEST = "/Game/HERO/ConceptV2/Materials"
TEXTURE_DEST = "/Game/HERO/DistrictV3/Textures"
LIB = unreal.MaterialEditingLibrary
ASSETS = unreal.EditorAssetLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
META = json.loads((ROOT / "Art/Textures/DistrictV3/metadata.json").read_text(encoding="utf-8"))

# Tint is linear colour, as in the existing runtime FLinearColor team overrides.
# Painted armor is dielectric. Only exposed steel has a high metallic value.
# name: (Tint, metallic, roughness, PBR source, atlas tiles, colour detail,
#        roughness variation, normal strength)
# All colours are linear. Team markings intentionally occupy only named Team /
# Neon slots; armor never inherits the runtime team tint by accident.
PALETTES = {
    "Aegis": {
        "Armor": ((.48,.42,.30), .08,.66,"Metal",2.,.23,.04,.07),
        "Steel": ((.24,.30,.34), .78,.43,"Metal",2.,.22,.06,.06),
        "Dark": ((.045,.060,.070), .08,.77,"Metal",2.,.18,.035,.05),
        "Rubber": ((.018,.022,.025),0.,.90,"Plaster",2.,.12,.025,.04),
        "Team": ((.025,.18,.48),.10,.58,"Metal",2.,.16,.035,.05),
        "Glass": ((.018,.075,.13),.40,.22,None,1.,0.,0.,0.),
        "Neon": ((.025,.35,.80),.10,.30,None,1.,0.,0.,0.),
        "Concrete": ((.34,.32,.27),0.,.86,"Concrete",1.5,.20,.04,.12),
        "Dust": ((.30,.255,.18),0.,.90,"CompactedDirt",1.5,.15,.035,.08),
        "Lamp": ((.85,.57,.22),0.,.36,None,1.,0.,0.,0.),
    },
    "Ember": {
        "Armor": ((.085,.095,.105),.12,.64,"Metal",2.,.25,.045,.07),
        "Steel": ((.21,.22,.23),.80,.40,"Metal",2.,.24,.06,.06),
        "Dark": ((.024,.028,.032),.08,.79,"Metal",2.,.17,.035,.05),
        "Rubber": ((.014,.017,.019),0.,.91,"Plaster",2.,.12,.025,.04),
        "Team": ((.48,.045,.018),.08,.61,"Metal",2.,.17,.035,.05),
        "Glass": ((.085,.025,.015),.40,.24,None,1.,0.,0.,0.),
        "Neon": ((.90,.12,.025),.10,.30,None,1.,0.,0.,0.),
        "Concrete": ((.24,.23,.21),0.,.87,"Concrete",1.5,.20,.04,.12),
        "Dust": ((.25,.205,.15),0.,.90,"CompactedDirt",1.5,.15,.035,.08),
        "Lamp": ((.90,.32,.08),0.,.36,None,1.,0.,0.,0.),
    },
}
SURFACES = {faction + "_" + surface: values
            for faction, palette in PALETTES.items()
            for surface, values in palette.items()}



def srgb_to_linear(value):
    value /= 255.
    return value / 12.92 if value <= .04045 else ((value + .055) / 1.055) ** 2.4


# Preflight every dependency before deleting any existing material expression.
textures = {}
for source in sorted({surface[3] for surface in SURFACES.values() if surface[3]}):
    assert source in META["materials"], source
    textures[source] = {}
    for role in ("Albedo", "Normal", "Roughness"):
        path = TEXTURE_DEST + "/T_DV3_" + source + "_" + role
        texture = unreal.load_asset(path)
        assert texture, "Run import_district_v3.py first: " + path
        assert texture.get_editor_property("srgb") == (role == "Albedo"), (path, "Incorrect colour space")
        if role == "Normal":
            assert texture.get_editor_property("compression_settings") == unreal.TextureCompressionSettings.TC_NORMALMAP, path
        elif role == "Roughness":
            assert texture.get_editor_property("compression_settings") == unreal.TextureCompressionSettings.TC_MASKS, path
        textures[source][role] = texture

# Validate all source materials before creating anything. Existing ConceptV2
# packages are read only: duplicate into this pass's own destination, then edit.
sources = {}
for surface in PALETTES["Aegis"]:
    path = SOURCE_DEST + "/M_CV2_" + surface
    sources[surface] = unreal.load_asset(path)
    assert sources[surface], "Missing source material: " + path
ASSETS.make_directory(DEST)
materials = {}
for name in SURFACES:
    surface = name.split("_", 1)[1]
    path = DEST + "/M_HR_" + name
    material = unreal.load_asset(path)
    if not material:
        material = ASSETS.duplicate_asset(sources[surface].get_path_name().split(".")[0], path)
    assert isinstance(material, unreal.Material), path
    materials[name] = material

records = []


def rebuild(name, values):
    tint_color, metallic, roughness, source, tiles, detail, rough_range, normal_strength = values
    material = materials[name]
    LIB.delete_all_material_expressions(material)
    counts = {"expressions": 0, "connections": 0, "property_connections": 0}

    def node(kind, x=0, y=0):
        result = LIB.create_material_expression(material, kind, x, y)
        assert result, (name, kind)
        counts["expressions"] += 1
        return result

    def link(start, output, end, input_name):
        assert LIB.connect_material_expressions(start, output, end, input_name), (
            name, start.get_class().get_name(), output, end.get_class().get_name(), input_name)
        counts["connections"] += 1

    def property_link(start, output, prop):
        assert LIB.connect_material_property(start, output, prop), (name, output, prop)
        counts["property_connections"] += 1

    def scalar(value, x=-400, y=500):
        result = node(unreal.MaterialExpressionConstant, x, y)
        result.set_editor_property("r", value)
        return result

    tint = node(unreal.MaterialExpressionVectorParameter, -1000, -150)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(*tint_color, 1))
    base = tint

    if source:
        coordinates = node(unreal.MaterialExpressionTextureCoordinate, -1400, 100)
        uv = node(unreal.MaterialExpressionMultiply, -1150, 100)
        uv.set_editor_property("const_b", tiles)
        link(coordinates, "", uv, "A")

        def sample(role, y):
            result = node(unreal.MaterialExpressionTextureSample, -900, y)
            result.set_editor_property("texture", textures[source][role])
            sampler = (unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL if role == "Normal"
                       else unreal.MaterialSamplerType.SAMPLERTYPE_MASKS if role == "Roughness"
                       else unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)
            result.set_editor_property("sampler_type", sampler)
            link(uv, "", result, "UVs")
            return result

        albedo = sample("Albedo", 70)
        # Unreal decodes sRGB albedo before shading. Normalizing its red-channel
        # detail prevents the original dark Metal colour from blackening Tint.
        source_mean = srgb_to_linear(META["materials"][source]["mean_albedo_srgb_8bit"][0])
        assert source_mean > .001, (source, source_mean)
        normalized = node(unreal.MaterialExpressionDivide, -650, 70)
        normalized.set_editor_property("const_b", source_mean)
        link(albedo, "R", normalized, "A")
        modulation = node(unreal.MaterialExpressionLinearInterpolate, -420, 50)
        modulation.set_editor_property("const_a", 1.)
        modulation.set_editor_property("const_alpha", detail)
        link(normalized, "", modulation, "B")
        base = node(unreal.MaterialExpressionMultiply, -180, -100)
        link(tint, "", base, "A")
        link(modulation, "", base, "B")

        rough_texture = sample("Roughness", 330)
        rough_detail = node(unreal.MaterialExpressionLinearInterpolate, -400, 320)
        # Keep the requested baseline at the actual texture's measured mean.
        mean_roughness = META["materials"][source]["roughness_mean"]
        rough_detail.set_editor_property("const_a", roughness - mean_roughness * rough_range * 2)
        rough_detail.set_editor_property("const_b", roughness + (1 - mean_roughness) * rough_range * 2)
        link(rough_texture, "R", rough_detail, "Alpha")
        property_link(rough_detail, "", unreal.MaterialProperty.MP_ROUGHNESS)

        normal_texture = sample("Normal", 610)
        flat = node(unreal.MaterialExpressionConstant3Vector, -650, 800)
        flat.set_editor_property("constant", unreal.LinearColor(0, 0, 1, 1))
        normal_mix = node(unreal.MaterialExpressionLinearInterpolate, -400, 610)
        normal_mix.set_editor_property("const_alpha", normal_strength)
        link(flat, "", normal_mix, "A")
        link(normal_texture, "RGB", normal_mix, "B")
        normal = node(unreal.MaterialExpressionNormalize, -170, 610)
        link(normal_mix, "", normal, "VectorInput")
        property_link(normal, "", unreal.MaterialProperty.MP_NORMAL)
    else:
        property_link(scalar(roughness), "", unreal.MaterialProperty.MP_ROUGHNESS)

    property_link(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    property_link(scalar(metallic, -400, 970), "", unreal.MaterialProperty.MP_METALLIC)
    if name.split("_", 1)[1] in ("Neon", "Lamp"):
        emission = node(unreal.MaterialExpressionMultiply, -180, -340)
        emission.set_editor_property("const_b", 2. if name.endswith("_Neon") else 1.)
        link(tint, "", emission, "A")
        property_link(emission, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    LIB.set_material_usage(material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
    LIB.recompile_material(material)
    assert ASSETS.save_loaded_asset(material, only_if_is_dirty=False), material.get_path_name()
    record = {"material": material.get_path_name(), "tint_linear": tint_color,
              "metallic": metallic, "roughness": roughness, "detail_source": source,
              "atlas_detail_tiles": tiles, "colour_detail_strength": detail,
              "normal_detail_strength": normal_strength, "graph": counts,
              "runtime_tint_parameter": "Tint", "procedural_noise_nodes": 0}
    records.append(record)
    unreal.log("FACTION_SURFACE_V5_REBUILT " + json.dumps(record))


for name, values in SURFACES.items():
    rebuild(name, values)

mapping = {faction: {"M_CV2_" + surface: DEST + "/M_HR_" + faction + "_" + surface
                     for surface in palette} for faction, palette in PALETTES.items()}
report = {"kind": "Original faction surfaces V5; existing assets unchanged",
          "source": "Read-only ConceptV2 material templates and original DistrictV3 PBR textures",
          "uv_basis": "Existing normalized mesh atlas, including skeletal UVs; no world-space swimming",
          "runtime_parameter": "Tint (linear); override only named Team/Neon slots if desired",
          "skeletal_usage": True,
          "mapping": mapping,
          "verification": "Graph links, texture colour spaces, source presence and forced saves asserted; fresh-process shader/render verification required",
          "materials": records}
report_path = ROOT / "Saved/Research/faction-surfaces-v5.json"
report_path.parent.mkdir(parents=True, exist_ok=True)
report_path.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
unreal.log("FACTION_SURFACES_V5_PASS " + str(len(records)))
