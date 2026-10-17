"""Create the first saved HeroHour RTS definitions from the current live balance.

Run with Unreal's Python commandlet after compiling HeroHourPrototype. The root
agent owns Unreal execution and C++ integration. Run again in a fresh process
with -VerifyHRDefinitions to verify saved packages without changing them.

All values are HeroHour's existing prototype values, not extracted Tempest
Rising balance. Both prototype teams currently share Aegis; this creates no
asymmetric roster. Grid footprints are future metadata; active placement still
uses the documented circular C++ footprints. No fake power/radar/abilities.

The tag importer and soft-object setter are backed by this installation's
PyWrapperStruct.cpp, GameplayTagContainer.cpp and PyConversion.cpp. Missing
native tags, asset classes, or presentation references fail before mutation.
"""
from pathlib import Path
import json

import unreal


ROOT = Path(__file__).resolve().parents[2]
DEST = "/Game/HERO/DataV3"
SCRIPT_PACKAGE = "/Script/HeroHourPrototype."
VERIFY_ONLY = "-verifyhrdefinitions" in unreal.SystemLibrary.get_command_line().lower()
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
TAGS = unreal.GameplayTagLibrary


def gameplay_tag(name):
    value = unreal.GameplayTag()
    assert value.import_text('(TagName="' + name + '")'), name
    assert TAGS.is_gameplay_tag_valid(value), "Unregistered native tag: " + name
    assert str(TAGS.get_tag_name(value)) == name, "Tag import mismatch: " + name
    return value


def tag_container(names):
    return TAGS.make_gameplay_tag_container_from_array([gameplay_tag(name) for name in names])


def tag_name(value):
    return str(TAGS.get_tag_name(value))


def explicit_tag_names(value):
    return sorted(tag_name(tag) for tag in TAGS.break_gameplay_tag_container(value))


def load_class(name):
    path = SCRIPT_PACKAGE + name
    value = unreal.load_class(None, path)
    assert value, "Compile HeroHourPrototype first; class missing: " + path
    return value


def load_presentation(path, cls):
    value = unreal.load_asset(path)
    assert value and isinstance(value, cls), "Missing or wrong presentation asset: " + path
    assert value.get_path_name() == path, (path, value.get_path_name())
    return value


def set_props(obj, values):
    for key, value in values.items():
        obj.set_editor_property(key, value)


def same_float(actual, expected, label):
    assert abs(float(actual) - float(expected)) < 0.0001, (label, actual, expected)


UNIT_CLASS = load_class("HRUnitDefinition")
BUILDING_CLASS = load_class("HRBuildingDefinition")
FIELD_CLASS = load_class("HRTempestFieldDefinitionAsset")
FACTION_CLASS = load_class("HRFactionDefinition")
MATRIX_CLASS = load_class("HRDamageMatrix")
FACTION_TAG = "HeroHour.Faction.Aegis"
VEHICLES_TAG = "HeroHour.Production.Vehicles"
BUILDINGS_TAG = "HeroHour.Production.Buildings"

# All five roles use the current original ConceptV2 meshes and portraits.
ROLE_SPECS = [
    dict(name="DA_HR_Striker", label="STRIKER", unit="HeroHour.Unit.Striker",
         health=100, cost=100, seconds=5, structure=False, radius=80,
         mesh="/Game/HERO/ConceptV2/Meshes/SM_HERO_Striker.SM_HERO_Striker",
         icon="/Game/HERO/UI/T_HERO_Striker.T_HERO_Striker",
         prerequisites=["HeroHour.Tech.Factory"], allowed=[]),
    dict(name="DA_HR_Harvester", label="SAMMLER", unit="HeroHour.Unit.Harvester",
         health=100, cost=150, seconds=6, structure=False, radius=80,
         mesh="/Game/HERO/ConceptV2/Meshes/SM_HERO_Harvester.SM_HERO_Harvester",
         icon="/Game/HERO/UI/T_HERO_Harvester.T_HERO_Harvester",
         prerequisites=[], allowed=[]),
    dict(name="DA_HR_CommandHub", label="KOMMANDOZENTRALE", unit="HeroHour.Building.CommandHub",
         health=800, cost=0, seconds=5, structure=True, radius=600,
         mesh="/Game/HERO/ConceptV2/Meshes/SM_HERO_CommandHub.SM_HERO_CommandHub",
         icon="/Game/HERO/UI/T_HERO_CommandHub.T_HERO_CommandHub",
         prerequisites=[], allowed=[VEHICLES_TAG]),
    dict(name="DA_HR_Refinery", label="RAFFINERIE", unit="HeroHour.Building.Refinery",
         health=400, cost=200, seconds=8, structure=True, radius=400,
         mesh="/Game/HERO/ConceptV2/Meshes/SM_HERO_Refinery.SM_HERO_Refinery",
         icon="/Game/HERO/UI/T_HERO_Refinery.T_HERO_Refinery",
         prerequisites=[], allowed=[]),
    dict(name="DA_HR_VehiclePlant", label="FAHRZEUGFABRIK", unit="HeroHour.Building.VehiclePlant",
         health=600, cost=350, seconds=12, structure=True, radius=540,
         mesh="/Game/HERO/ConceptV2/Meshes/SM_HERO_VehiclePlant.SM_HERO_VehiclePlant",
         icon="/Game/HERO/UI/T_HERO_VehiclePlant.T_HERO_VehiclePlant",
         prerequisites=[], allowed=[VEHICLES_TAG]),
]

FIELD_SETTINGS = dict(capacity=4000, initial_amount=4000, harvest_rate=20,
                      regrowth_per_second=2, regrowth_delay=10, seed_floor=0)
CLASS_BY_NAME = {spec["name"]: BUILDING_CLASS if spec["structure"] else UNIT_CLASS
                 for spec in ROLE_SPECS}
CLASS_BY_NAME.update(DA_HR_TempestField=FIELD_CLASS, DA_HR_Aegis=FACTION_CLASS,
                     DA_HR_DamageMatrix=MATRIX_CLASS)

# Validate the full tag and presentation dependency set before touching assets.
ALL_TAG_NAMES = {FACTION_TAG, VEHICLES_TAG, BUILDINGS_TAG, "HeroHour.Damage.Kinetic",
                 "HeroHour.Damage.Explosive", "HeroHour.Damage.Tempest",
                 "HeroHour.Armor.Infantry", "HeroHour.Armor.Vehicle", "HeroHour.Armor.Structure"}
for spec in ROLE_SPECS:
    ALL_TAG_NAMES.add(spec["unit"])
    ALL_TAG_NAMES.update(spec["prerequisites"])
    ALL_TAG_NAMES.update(spec["allowed"])
for name in sorted(ALL_TAG_NAMES):
    gameplay_tag(name)
PRESENTATION = {spec["name"]: (load_presentation(spec["mesh"], unreal.StaticMesh),
                             load_presentation(spec["icon"], unreal.Texture2D))
                for spec in ROLE_SPECS}
ASSETS = {}
for name, cls in CLASS_BY_NAME.items():
    path = DEST + "/" + name
    existing = unreal.load_asset(path) if LIB.does_asset_exist(path) else None
    assert existing is None or existing.get_class() == cls, "Refusing wrong existing class: " + path
    assert not VERIFY_ONLY or existing, "Missing saved definition: " + path
    ASSETS[name] = existing


def get_or_create(name):
    existing = ASSETS[name]
    if existing:
        return existing
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", CLASS_BY_NAME[name])
    asset = TOOLS.create_asset(name, DEST, CLASS_BY_NAME[name], factory)
    assert asset and asset.get_class() == CLASS_BY_NAME[name], "Definition creation failed: " + name
    ASSETS[name] = asset
    return asset


def save(asset):
    assert LIB.save_loaded_asset(asset, only_if_is_dirty=False), "Asset save failed: " + asset.get_path_name()


def stats_values(spec):
    armed = spec["name"] == "DA_HR_Striker"
    return dict(health=float(spec["health"]), move_speed_cm=0.0 if spec["structure"] else 220.0,
                sight_range_cm=0.0 if spec["structure"] else 650.0,
                weapon_damage=20.0 if armed else 0.0,
                weapon_range_cm=480.0 if armed else 0.0, attack_interval=1.0)


def grid_side(spec):
    # Metadata for a conservative enclosing square, using the current 200-cm grid.
    # Circular clearance is still selected by APrototypeGameMode::Footprint.
    return (2 * spec["radius"] + 199) // 200


if not VERIFY_ONLY:
    for spec in ROLE_SPECS:
        asset = get_or_create(spec["name"])
        stats = unreal.HRUnitStats()
        set_props(stats, stats_values(spec))
        set_props(stats, dict(armor_type=gameplay_tag("HeroHour.Armor.Structure" if spec["structure"]
                                                     else "HeroHour.Armor.Vehicle"),
                             damage_type=gameplay_tag("HeroHour.Damage.Kinetic")))
        mesh, icon = PRESENTATION[spec["name"]]
        set_props(asset, dict(unit_id=gameplay_tag(spec["unit"]), faction_id=gameplay_tag(FACTION_TAG),
                              display_name=unreal.Text(spec["label"]),
                              production_type=gameplay_tag(BUILDINGS_TAG if spec["structure"] else VEHICLES_TAG),
                              stats=stats, credit_cost=spec["cost"], production_seconds=float(spec["seconds"]),
                              prerequisites=tag_container(spec["prerequisites"]), abilities=tag_container([]),
                              veterancy_levels=[], static_mesh=mesh, icon=icon))
        if spec["structure"]:
            side = grid_side(spec)
            set_props(asset, dict(footprint_cells=unreal.IntPoint(side, side), connected_build_radius_cm=2300.0,
                                  power_supply=0, power_demand=0, provides_radar=False,
                                  requires_power_for_defense=False,
                                  allowed_production_types=tag_container(spec["allowed"])))
        save(asset)

    field = get_or_create("DA_HR_TempestField")
    settings = unreal.HRTempestFieldDefinition()
    set_props(settings, {key: float(value) for key, value in FIELD_SETTINGS.items()})
    field.set_editor_property("settings", settings)
    save(field)

    faction = get_or_create("DA_HR_Aegis")
    set_props(faction, dict(faction_id=gameplay_tag(FACTION_TAG), display_name=unreal.Text("AEGIS · PROTOTYP"),
                            units=[ASSETS[spec["name"]] for spec in ROLE_SPECS if not spec["structure"]],
                            buildings=[ASSETS[spec["name"]] for spec in ROLE_SPECS if spec["structure"]],
                            economy_style=unreal.HREconomyStyle.FINITE_SUPPLY,
                            build_style=unreal.HRBuildStyle.SIDEBAR_PLACEMENT))
    save(faction)

    matrix = get_or_create("DA_HR_DamageMatrix")
    rules = []
    for damage in ("Kinetic", "Explosive", "Tempest"):
        for armor in ("Infantry", "Vehicle", "Structure"):
            rule = unreal.HRDamageRule()
            set_props(rule, dict(damage_type=gameplay_tag("HeroHour.Damage." + damage),
                                 armor_type=gameplay_tag("HeroHour.Armor." + armor), multiplier=1.0))
            rules.append(rule)
    matrix.set_editor_property("rules", rules)
    save(matrix)

# The same assertions run after authoring and in the fresh-process verify-only
# mode. Verify-only never calls a save/set/create operation.
records = []
for spec in ROLE_SPECS:
    asset = ASSETS[spec["name"]]
    assert asset and asset.get_class() == CLASS_BY_NAME[spec["name"]]
    assert tag_name(asset.get_editor_property("unit_id")) == spec["unit"]
    assert tag_name(asset.get_editor_property("faction_id")) == FACTION_TAG
    assert str(asset.get_editor_property("display_name")) == spec["label"]
    assert tag_name(asset.get_editor_property("production_type")) == (BUILDINGS_TAG if spec["structure"] else VEHICLES_TAG)
    assert asset.get_editor_property("credit_cost") == spec["cost"]
    same_float(asset.get_editor_property("production_seconds"), spec["seconds"], spec["name"] + ".seconds")
    assert explicit_tag_names(asset.get_editor_property("prerequisites")) == sorted(spec["prerequisites"])
    assert explicit_tag_names(asset.get_editor_property("abilities")) == []
    assert len(asset.get_editor_property("veterancy_levels")) == 0
    assert asset.get_editor_property("static_mesh").get_path_name() == spec["mesh"]
    assert asset.get_editor_property("icon").get_path_name() == spec["icon"]
    stats = asset.get_editor_property("stats")
    for key, value in stats_values(spec).items():
        same_float(stats.get_editor_property(key), value, spec["name"] + ".stats." + key)
    assert tag_name(stats.get_editor_property("armor_type")) == ("HeroHour.Armor.Structure" if spec["structure"] else "HeroHour.Armor.Vehicle")
    assert tag_name(stats.get_editor_property("damage_type")) == "HeroHour.Damage.Kinetic"
    if spec["structure"]:
        cells = asset.get_editor_property("footprint_cells")
        assert cells.x == grid_side(spec) and cells.y == grid_side(spec)
        same_float(asset.get_editor_property("connected_build_radius_cm"), 2300, spec["name"] + ".buildradius")
        assert asset.get_editor_property("power_supply") == 0 and asset.get_editor_property("power_demand") == 0
        assert not asset.get_editor_property("provides_radar") and not asset.get_editor_property("requires_power_for_defense")
        assert explicit_tag_names(asset.get_editor_property("allowed_production_types")) == sorted(spec["allowed"])
    records.append(dict(asset=asset.get_path_name(), class_path=asset.get_class().get_path_name(),
                        authored=spec, stats=stats_values(spec)))

field = ASSETS["DA_HR_TempestField"]
settings = field.get_editor_property("settings")
for key, value in FIELD_SETTINGS.items():
    same_float(settings.get_editor_property(key), value, "Field." + key)
records.append(dict(asset=field.get_path_name(), settings=FIELD_SETTINGS))

faction = ASSETS["DA_HR_Aegis"]
assert tag_name(faction.get_editor_property("faction_id")) == FACTION_TAG
assert str(faction.get_editor_property("display_name")) == "AEGIS · PROTOTYP"
assert [asset.get_path_name() for asset in faction.get_editor_property("units")] == [ASSETS[spec["name"]].get_path_name() for spec in ROLE_SPECS if not spec["structure"]]
assert [asset.get_path_name() for asset in faction.get_editor_property("buildings")] == [ASSETS[spec["name"]].get_path_name() for spec in ROLE_SPECS if spec["structure"]]
assert faction.get_editor_property("economy_style") == unreal.HREconomyStyle.FINITE_SUPPLY
assert faction.get_editor_property("build_style") == unreal.HRBuildStyle.SIDEBAR_PLACEMENT
records.append(dict(asset=faction.get_path_name(), faction=FACTION_TAG, teams_share_roster=True,
                    economy="FiniteSupply", build_metadata="SidebarPlacement"))

matrix = ASSETS["DA_HR_DamageMatrix"]
rules = matrix.get_editor_property("rules")
assert len(rules) == 9
pairs = set()
for rule in rules:
    damage = rule.get_editor_property("damage_type")
    armor = rule.get_editor_property("armor_type")
    pair = (tag_name(damage), tag_name(armor))
    assert pair not in pairs
    pairs.add(pair)
    same_float(rule.get_editor_property("multiplier"), 1, "Matrix authored multiplier")
    same_float(matrix.get_damage_multiplier(damage, armor), 1, "Matrix lookup multiplier")
assert pairs == {("HeroHour.Damage." + damage, "HeroHour.Armor." + armor)
                 for damage in ("Kinetic", "Explosive", "Tempest")
                 for armor in ("Infantry", "Vehicle", "Structure")}
records.append(dict(asset=matrix.get_path_name(), rule_count=9, neutral_multiplier=1))

assert len(ASSETS) == len(records) == 8
for name in ASSETS:
    assert (ROOT / "Content/HERO/DataV3" / (name + ".uasset")).is_file(), "Package not saved: " + name
output = ROOT / "Saved/Research" / ("rts-definitions-v3-verify.json" if VERIFY_ONLY else "rts-definitions-v3.json")
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(dict(mode="verify_saved" if VERIFY_ONLY else "create_or_update",
                                  scope="current HeroHour prototype balance; no Tempest parity claim",
                                  records=records), indent=2, ensure_ascii=False), encoding="utf-8")
unreal.log("HR_DEFINITIONS_V3_VERIFY_PASS" if VERIFY_ONLY else "HR_DEFINITIONS_V3_CREATE_PASS")
