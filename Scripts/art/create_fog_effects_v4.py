"""Create original world-space fog and lightweight smoke/impact materials.

Run in Unreal Python after compiling HeroHourPrototype. Runtime integration is
explicit: create a MID for M_PP_Fog, set FogMask to the per-team RGBA texture,
and attach it to the local camera's postprocess settings. No map is modified.
The fog samples mask alpha with UV=(WorldXY+(12000,9000))/(24000,18000).
It runs after tonemapping, before the Canvas HUD, keeping interface panels clear.
Outside the playable bounds the alpha is .92; no edge texture repeats.

UE 5.8 source contracts: MaterialTemplate.ush::CalcMaterialParametersPost
reconstructs per-pixel world position from scene depth; BlendableInterface.h
defines BL_SceneColorAfterTonemapping. Shader names/pins come from this host's
MaterialExpression*.h headers. Cooking/RHI must still verify shader compilation.
"""
import json
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
DEST = "/Game/HERO/EffectsV4"
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.MaterialEditingLibrary
ASSETS = unreal.EditorAssetLibrary
RECORDS = []


def enum_value(enum_class, *candidates):
    for candidate in candidates:
        if hasattr(enum_class, candidate):
            return getattr(enum_class, candidate)
    raise AssertionError((enum_class, candidates))


WHITE = unreal.load_asset("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")
assert isinstance(WHITE, unreal.Texture2D), "Missing engine fallback texture"


class Graph:
    def __init__(self, name):
        self.name = name
        self.material = unreal.load_asset(DEST + "/" + name)
        if self.material:
            assert isinstance(self.material, unreal.Material), "Wrong existing material class: " + name
        else:
            self.material = TOOLS.create_asset(name, DEST, unreal.Material, unreal.MaterialFactoryNew())
        assert self.material, name
        LIB.delete_all_material_expressions(self.material)
        self.nodes = 0
        self.links = 0

    def node(self, kind, x=0, y=0, **values):
        result = LIB.create_material_expression(self.material, kind, x, y)
        assert result, (self.name, kind)
        for key, value in values.items():
            result.set_editor_property(key, value)
        self.nodes += 1
        return result

    def link(self, start, output, end, input_name):
        assert LIB.connect_material_expressions(start, output, end, input_name), (
            self.name, start.get_class().get_name(), output, end.get_class().get_name(), input_name)
        self.links += 1

    def prop(self, start, output, property_name):
        assert LIB.connect_material_property(start, output, property_name), (self.name, property_name)
        self.links += 1

    def scalar(self, value, x=0, y=0):
        return self.node(unreal.MaterialExpressionConstant, x, y, r=float(value))

    def color(self, value, x=0, y=0):
        return self.node(unreal.MaterialExpressionConstant3Vector, x, y,
                         constant=unreal.LinearColor(*value, 1))

    def tint(self, value, x=0, y=0):
        return self.node(unreal.MaterialExpressionVectorParameter, x, y,
                         parameter_name="Tint", default_value=unreal.LinearColor(*value, 1))

    def save(self, **metadata):
        LIB.recompile_material(self.material)
        assert ASSETS.save_loaded_asset(self.material, only_if_is_dirty=False), self.name
        RECORDS.append(dict(path=self.material.get_path_name(), expressions=self.nodes,
                            connections=self.links, **metadata))


def create_fog():
    g = Graph("M_PP_Fog")
    g.material.set_editor_property("material_domain", unreal.MaterialDomain.MD_POST_PROCESS)
    g.material.set_editor_property("blendable_location", enum_value(unreal.BlendableLocation,
        "BL_SCENE_COLOR_AFTER_TONEMAPPING", "BL_AFTER_TONEMAPPING"))
    g.material.set_editor_property("blendable_priority", 90)
    world = g.node(unreal.MaterialExpressionWorldPosition, -1300, 0)
    xy = g.node(unreal.MaterialExpressionComponentMask, -1080, 0, r=True, g=True, b=False, a=False)
    g.link(world, "", xy, "")
    offset = g.node(unreal.MaterialExpressionConstant2Vector, -1080, 180, r=12000.0, g=9000.0)
    shifted = g.node(unreal.MaterialExpressionAdd, -850, 0)
    g.link(xy, "", shifted, "A")
    g.link(offset, "", shifted, "B")
    span = g.node(unreal.MaterialExpressionConstant2Vector, -850, 200, r=24000.0, g=18000.0)
    uv = g.node(unreal.MaterialExpressionDivide, -630, 0)
    g.link(shifted, "", uv, "A")
    g.link(span, "", uv, "B")
    saturated = g.node(unreal.MaterialExpressionSaturate, -420, 0)
    g.link(uv, "", saturated, "")
    sample = g.node(unreal.MaterialExpressionTextureSampleParameter2D, -200, 0,
        parameter_name="FogMask", texture=WHITE,
        sampler_type=(unreal.MaterialSamplerType.SAMPLERTYPE_COLOR
                      if WHITE.get_editor_property("srgb") else unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR))
    g.link(saturated, "", sample, "UVs")
    outside = g.node(unreal.MaterialExpressionCustom, 60, 0,
        output_type=unreal.CustomMaterialOutputType.CMOT_FLOAT1,
        description="Mask alpha and explicit playable-map boundary",
        code="return (any(UV < 0.0) || any(UV >= 1.0)) ? 0.92 : saturate(Alpha);")
    inputs = []
    for name in ("UV", "Alpha"):
        custom_input = unreal.CustomInput()
        custom_input.set_editor_property("input_name", name)
        inputs.append(custom_input)
    outside.set_editor_property("inputs", inputs)
    g.link(uv, "", outside, "UV")
    g.link(sample, "A", outside, "Alpha")
    scene = g.node(unreal.MaterialExpressionSceneTexture, 40, 260,
        scene_texture_id=enum_value(unreal.SceneTextureId, "PPI_POST_PROCESS_INPUT0"))
    scene_rgb = g.node(unreal.MaterialExpressionComponentMask, 240, 320,
                       r=True, g=True, b=True, a=False)
    g.link(scene, "Color", scene_rgb, "")
    black = g.color((0.0, 0.0, 0.0), 70, 440)
    color = g.node(unreal.MaterialExpressionLinearInterpolate, 470, 200)
    g.link(scene_rgb, "", color, "A")
    g.link(black, "", color, "B")
    g.link(outside, "", color, "Alpha")
    g.prop(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    g.save(kind="postprocess_world_fog", parameter="FogMask", alpha_channel="A",
        map_min_cm=[-12000, -9000], map_max_cm=[12000, 9000],
        visible_alpha=0, explored_alpha=.55, unknown_alpha=.92,
        blend_location="SceneColorAfterTonemapping", requires_runtime_mask=True)


def create_smoke():
    g = Graph("M_FX_Smoke")
    g.material.set_editor_property("material_domain", unreal.MaterialDomain.MD_SURFACE)
    g.material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    g.material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    g.material.set_editor_property("two_sided", True)
    tint = g.tint((.13, .145, .15), -700, -180)
    emission = g.node(unreal.MaterialExpressionMultiply, -230, -180, const_b=.55)
    g.link(tint, "", emission, "A")
    g.prop(emission, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    fresnel = g.node(unreal.MaterialExpressionFresnel, -700, 100,
                     exponent=1.7, base_reflect_fraction=0.0)
    soft_edge = g.node(unreal.MaterialExpressionOneMinus, -480, 100)
    g.link(fresnel, "", soft_edge, "")
    noise = g.node(unreal.MaterialExpressionNoise, -700, 360, scale=.011,
                   quality=1, levels=2, output_min=.35, output_max=1.0)
    shape = g.node(unreal.MaterialExpressionMultiply, -230, 150)
    g.link(soft_edge, "", shape, "A")
    g.link(noise, "", shape, "B")
    opacity = g.node(unreal.MaterialExpressionScalarParameter, -250, 390,
                     parameter_name="Opacity", default_value=.65)
    alpha = g.node(unreal.MaterialExpressionMultiply, 0, 170)
    g.link(shape, "", alpha, "A")
    g.link(opacity, "", alpha, "B")
    g.prop(alpha, "", unreal.MaterialProperty.MP_OPACITY)
    g.save(kind="smoke_surface", parameters=["Tint", "Opacity"],
           animated_shader_time=False, lifetime_owner="simulation visibility volume")


def create_impact():
    g = Graph("M_FX_Impact")
    g.material.set_editor_property("material_domain", unreal.MaterialDomain.MD_SURFACE)
    g.material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_ADDITIVE)
    g.material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    g.material.set_editor_property("two_sided", True)
    tint = g.tint((1.0, .35, .045), -480, 0)
    intensity = g.node(unreal.MaterialExpressionScalarParameter, -480, 170,
                       parameter_name="Intensity", default_value=6.0)
    emission = g.node(unreal.MaterialExpressionMultiply, -220, 0)
    g.link(tint, "", emission, "A")
    g.link(intensity, "", emission, "B")
    g.prop(emission, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    fresnel = g.node(unreal.MaterialExpressionFresnel, -480, 350,
                     exponent=2.0, base_reflect_fraction=0.0)
    core = g.node(unreal.MaterialExpressionOneMinus, -220, 350)
    g.link(fresnel, "", core, "")
    opacity = g.node(unreal.MaterialExpressionScalarParameter, -250, 500,
                     parameter_name="Opacity", default_value=.8)
    alpha = g.node(unreal.MaterialExpressionMultiply, 10, 320)
    g.link(core, "", alpha, "A")
    g.link(opacity, "", alpha, "B")
    g.prop(alpha, "", unreal.MaterialProperty.MP_OPACITY)
    g.save(kind="impact_surface", parameters=["Tint", "Intensity", "Opacity"],
           animated_shader_time=False, lifetime_owner="runtime simulation effect")


create_fog()
create_smoke()
create_impact()
output = ROOT / "Saved/Research/FogEffectsV4-Materials.json"
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(RECORDS, indent=2), encoding="utf-8")
unreal.log("FOG_EFFECTS_V4_CREATE_PASS " + json.dumps(RECORDS))
