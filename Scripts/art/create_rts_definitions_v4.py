"""Author and verify HeroHour's two complete faction rosters as saved DataAssets.

Run in Unreal Python after the FrontlinesV4 meshes and portraits are imported.
The names are stable runtime paths. -VerifyHRDefinitionsV4 performs read-only
verification in a fresh commandlet; no create/set/save operation is then used.
All balance is explicitly authored HeroHour design, not extracted game values.
Existing DataV3 definitions and maps are never modified.
"""
import json
from pathlib import Path
import unreal

ROOT = Path(__file__).resolve().parents[2]
DEST = "/Game/HERO/DataV4"
VERIFY_ONLY = "-verifyhrdefinitionsv4" in unreal.SystemLibrary.get_command_line().lower()
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
LIB = unreal.EditorAssetLibrary
TAGS = unreal.GameplayTagLibrary


def tag(name):
    result = unreal.GameplayTag()
    assert result.import_text('(TagName="' + name + '")'), name
    assert TAGS.is_gameplay_tag_valid(result), "Unregistered native tag: " + name
    assert str(TAGS.get_tag_name(result)) == name, name
    return result


def container(names):
    return TAGS.make_gameplay_tag_container_from_array([tag(name) for name in names])


def names(value):
    return sorted(str(TAGS.get_tag_name(t)) for t in TAGS.break_gameplay_tag_container(value))


def props(obj, values):
    for key, value in values.items():
        obj.set_editor_property(key, value)


def check_float(actual, expected, label):
    assert abs(float(actual) - float(expected)) < .0001, (label, actual, expected)


def load_class(name):
    result = unreal.load_class(None, "/Script/HeroHourPrototype." + name)
    assert result, "Compile HeroHourPrototype first: " + name
    return result


UNIT_CLASS = load_class("HRUnitDefinition")
BUILDING_CLASS = load_class("HRBuildingDefinition")
FACTION_CLASS = load_class("HRFactionDefinition")
MATRIX_CLASS = load_class("HRDamageMatrix")
PRODUCTION = {name: "HeroHour.Production." + name
              for name in ("Buildings", "Defense", "Infantry", "Vehicles", "Specialists")}

# Role names match EPrototypeRole order; Source meshes remain original HeroHour geometry.
SPECS = [
    dict(role="Combat", label="STRIKER", tag="HeroHour.Unit.Striker", mesh="Striker",
         old=True, building=False, hp=100, cost=100, seconds=5, speed=220,
         damage=20, range=480, interval=1, category="Vehicles", cells=1,
         supply=0, demand=0, tech=["HeroHour.Tech.Factory"], allowed=[]),
    dict(role="Collector", label="SAMMLER", tag="HeroHour.Unit.Harvester", mesh="Harvester",
         old=True, building=False, hp=100, cost=150, seconds=6, speed=220,
         damage=0, range=0, interval=1, category="Vehicles", cells=1,
         supply=0, demand=0, tech=[], allowed=[]),
    dict(role="HQ", label="KOMMANDOZENTRALE", tag="HeroHour.Building.CommandHub", mesh="CommandHub",
         old=True, building=True, hp=800, cost=0, seconds=1, speed=0,
         damage=0, range=0, interval=1, category="Buildings", cells=6,
         supply=60, demand=0, tech=[], allowed=["Buildings", "Defense", "Vehicles"]),
    dict(role="Depot", label="RAFFINERIE", tag="HeroHour.Building.Refinery", mesh="Refinery",
         old=True, building=True, hp=400, cost=200, seconds=8, speed=0,
         damage=0, range=0, interval=1, category="Buildings", cells=4,
         supply=0, demand=30, tech=[], allowed=[]),
    dict(role="Factory", label="FAHRZEUGFABRIK", tag="HeroHour.Building.VehiclePlant", mesh="VehiclePlant",
         old=True, building=True, hp=600, cost=350, seconds=12, speed=0,
         damage=0, range=0, interval=1, category="Buildings", cells=6,
         supply=0, demand=80, tech=[], allowed=["Vehicles", "Specialists"]),
    dict(role="PowerPlant", label="KRAFTWERK", tag="HeroHour.Building.PowerPlant", mesh="PowerPlant",
         old=False, building=True, hp=450, cost=220, seconds=9, speed=0,
         damage=0, range=0, interval=1, category="Buildings", cells=3,
         supply=150, demand=0, tech=[], allowed=[]),
    dict(role="Barracks", label="KASERNE", tag="HeroHour.Building.Barracks", mesh="Barracks",
         old=False, building=True, hp=450, cost=180, seconds=7, speed=0,
         damage=0, range=0, interval=1, category="Buildings", cells=3,
         supply=0, demand=40, tech=[], allowed=["Infantry"]),
    dict(role="Radar", label="RADARSTATION", tag="HeroHour.Building.Radar", mesh="Radar",
         old=False, building=True, hp=350, cost=250, seconds=9, speed=0,
         damage=0, range=0, interval=1, category="Buildings", cells=3,
         supply=0, demand=60, tech=[], allowed=[]),
    dict(role="Defense", label="ABWEHRGESCHÜTZ", tag="HeroHour.Building.Defense", mesh="Defense",
         old=False, building=True, hp=350, cost=200, seconds=8, speed=0,
         damage=24, range=850, interval=.8, category="Defense", cells=2,
         supply=0, demand=50, tech=[], allowed=[]),
    dict(role="Infantry", label="RANGER", tag="HeroHour.Unit.Infantry", mesh="Infantry",
         old=False, building=False, hp=70, cost=80, seconds=4, speed=250,
         damage=12, range=450, interval=.6, category="Infantry", cells=1,
         supply=0, demand=0, tech=["HeroHour.Tech.Barracks"], allowed=[]),
    dict(role="Siege", label="BELAGERUNGSARTILLERIE", tag="HeroHour.Unit.Siege", mesh="Siege",
         old=False, building=False, hp=160, cost=220, seconds=8, speed=180,
         damage=45, range=900, interval=2, category="Specialists", cells=1,
         supply=0, demand=0, tech=["HeroHour.Tech.Factory"], allowed=[]),
]
FACTIONS = {"Aegis": dict(move=1.0, armor=1.15, label="AEGIS"),
            "Ember": dict(move=280.0 / 220.0, armor=.9, label="EMBER")}
MATRIX = {"Kinetic": (1.1, 1.0, .5), "Explosive": (1.5, .85, 1.4),
          "Tempest": (1.0, 1.2, .9)}
ARMORS = ("Infantry", "Vehicle", "Structure")
VETERANCY = ((100, 1.15, 1.08, 1.10), (250, 1.30, 1.16, 1.20), (500, 1.50, 1.25, 1.35))


def presentation(spec):
    if spec["old"]:
        name = "SM_HERO_" + spec["mesh"]
        mesh = "/Game/HERO/ConceptV2/Meshes/" + name + "." + name
        name = "T_HERO_" + spec["mesh"]
    else:
        name = "SM_HR_" + spec["mesh"]
        mesh = "/Game/HERO/FrontlinesV4/Meshes/" + name + "." + name
        name = "T_HR_" + spec["mesh"]
    return mesh, "/Game/HERO/UI/" + name + "." + name


def armor(spec):
    return "Structure" if spec["building"] else "Infantry" if spec["role"] == "Infantry" else "Vehicle"


def damage(spec, faction):
    if spec["role"] == "Siege":
        return "Explosive"
    if faction == "Ember" and spec["role"] in ("Combat", "Defense"):
        return "Tempest"
    return "Kinetic"


def abilities(spec, faction):
    result = []
    if spec["role"] == "Infantry":
        result.append("HeroHour.Ability.Grenade")
    if spec["role"] == "Siege":
        result.append("HeroHour.Ability.Deploy")
    if faction == "Aegis" and spec["role"] in ("Combat", "Infantry", "Siege"):
        result.append("HeroHour.Ability.Smoke")
    return result


def stats_for(spec, faction):
    return dict(health=float(spec["hp"]), move_speed_cm=float(spec["speed"]),
        sight_range_cm=2200.0 if spec["building"] else 2600.0,
        weapon_damage=float(spec["damage"]), weapon_range_cm=float(spec["range"]),
        attack_interval=float(spec["interval"]),
        armor_type=tag("HeroHour.Armor." + armor(spec)),
        damage_type=tag("HeroHour.Damage." + damage(spec, faction)))


CLASSES = {"D_" + faction + "_" + spec["role"]: BUILDING_CLASS if spec["building"] else UNIT_CLASS
           for faction in FACTIONS for spec in SPECS}
CLASSES.update({"D_Faction_" + faction: FACTION_CLASS for faction in FACTIONS})
CLASSES["D_DamageMatrix"] = MATRIX_CLASS

# Full dependency preflight before the first asset mutation.
ALL_TAGS = {"HeroHour.Faction." + faction for faction in FACTIONS}
ALL_TAGS.update(PRODUCTION.values())
ALL_TAGS.update("HeroHour.Armor." + value for value in ARMORS)
ALL_TAGS.update("HeroHour.Damage." + value for value in MATRIX)
for spec in SPECS:
    ALL_TAGS.add(spec["tag"])
    ALL_TAGS.update(spec["tech"])
    for faction in FACTIONS:
        ALL_TAGS.update(abilities(spec, faction))
for name in sorted(ALL_TAGS):
    tag(name)
PRESENTATION = {}
for spec in SPECS:
    paths = presentation(spec)
    objects = tuple(unreal.load_asset(path) for path in paths)
    assert isinstance(objects[0], unreal.StaticMesh), (spec["role"], paths[0])
    assert isinstance(objects[1], unreal.Texture2D), (spec["role"], paths[1])
    PRESENTATION[spec["role"]] = objects
ASSETS = {}
for name, cls in CLASSES.items():
    path = DEST + "/" + name
    value = unreal.load_asset(path) if LIB.does_asset_exist(path) else None
    assert value is None or value.get_class() == cls, "Wrong existing class: " + path
    assert not VERIFY_ONLY or value, "Missing saved DataV4 asset: " + path
    ASSETS[name] = value


def get_or_create(name):
    if ASSETS[name]:
        return ASSETS[name]
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", CLASSES[name])
    result = TOOLS.create_asset(name, DEST, CLASSES[name], factory)
    assert result and result.get_class() == CLASSES[name], name
    ASSETS[name] = result
    return result


def save(asset):
    assert LIB.save_loaded_asset(asset, only_if_is_dirty=False), asset.get_path_name()


if not VERIFY_ONLY:
    for faction in FACTIONS:
        for spec in SPECS:
            asset = get_or_create("D_" + faction + "_" + spec["role"])
            stats = unreal.HRUnitStats()
            props(stats, stats_for(spec, faction))
            ranks = []
            if spec["damage"] > 0:
                for xp, dm, rm, am in VETERANCY:
                    rank = unreal.HRVeterancyLevel()
                    props(rank, dict(xp_threshold=xp, damage_multiplier=dm,
                                     range_multiplier=rm, armor_multiplier=am))
                    ranks.append(rank)
            mesh, icon = PRESENTATION[spec["role"]]
            props(asset, dict(unit_id=tag(spec["tag"]), faction_id=tag("HeroHour.Faction." + faction),
                display_name=unreal.Text(spec["label"]), production_type=tag(PRODUCTION[spec["category"]]),
                stats=stats, credit_cost=spec["cost"], production_seconds=float(spec["seconds"]),
                prerequisites=container(spec["tech"]), abilities=container(abilities(spec, faction)),
                veterancy_levels=ranks, static_mesh=mesh, icon=icon))
            if spec["building"]:
                props(asset, dict(footprint_cells=unreal.IntPoint(spec["cells"], spec["cells"]),
                    connected_build_radius_cm=2300.0, power_supply=spec["supply"], power_demand=spec["demand"],
                    provides_radar=spec["role"] == "Radar", requires_power_for_defense=spec["role"] == "Defense",
                    allowed_production_types=container([PRODUCTION[value] for value in spec["allowed"]])))
            save(asset)
        faction_asset = get_or_create("D_Faction_" + faction)
        props(faction_asset, dict(faction_id=tag("HeroHour.Faction." + faction),
            display_name=unreal.Text(FACTIONS[faction]["label"]),
            move_multiplier=FACTIONS[faction]["move"], armor_multiplier=FACTIONS[faction]["armor"],
            units=[ASSETS["D_" + faction + "_" + spec["role"]] for spec in SPECS if not spec["building"]],
            buildings=[ASSETS["D_" + faction + "_" + spec["role"]] for spec in SPECS if spec["building"]],
            economy_style=unreal.HREconomyStyle.REGROWING_FIELD, build_style=unreal.HRBuildStyle.SIDEBAR_PLACEMENT))
        save(faction_asset)
    matrix_asset = get_or_create("D_DamageMatrix")
    rules = []
    for damage_name, values in MATRIX.items():
        for armor_name, multiplier in zip(ARMORS, values):
            rule = unreal.HRDamageRule()
            props(rule, dict(damage_type=tag("HeroHour.Damage." + damage_name),
                armor_type=tag("HeroHour.Armor." + armor_name), multiplier=float(multiplier)))
            rules.append(rule)
    matrix_asset.set_editor_property("rules", rules)
    save(matrix_asset)

# Save/reload verification uses reflected values, including roster references and matrix lookups.
records = []
for faction, faction_spec in FACTIONS.items():
    for spec in SPECS:
        asset = ASSETS["D_" + faction + "_" + spec["role"]]
        assert str(TAGS.get_tag_name(asset.get_editor_property("unit_id"))) == spec["tag"]
        assert str(TAGS.get_tag_name(asset.get_editor_property("faction_id"))) == "HeroHour.Faction." + faction
        assert str(TAGS.get_tag_name(asset.get_editor_property("production_type"))) == PRODUCTION[spec["category"]]
        assert str(asset.get_editor_property("display_name")) == spec["label"]
        assert asset.get_editor_property("credit_cost") == spec["cost"]
        check_float(asset.get_editor_property("production_seconds"), spec["seconds"], spec["role"] + ".seconds")
        assert names(asset.get_editor_property("prerequisites")) == sorted(spec["tech"])
        assert names(asset.get_editor_property("abilities")) == sorted(abilities(spec, faction))
        stats = asset.get_editor_property("stats")
        for key, expected in stats_for(spec, faction).items():
            actual = stats.get_editor_property(key)
            if key.endswith("_type"):
                assert str(TAGS.get_tag_name(actual)) == str(TAGS.get_tag_name(expected)), (spec["role"], key)
            else:
                check_float(actual, expected, spec["role"] + ".stats." + key)
        for key, expected in zip(("static_mesh", "icon"), presentation(spec)):
            assert asset.get_editor_property(key).get_path_name() == expected, (spec["role"], key)
        ranks = asset.get_editor_property("veterancy_levels")
        assert len(ranks) == (3 if spec["damage"] > 0 else 0)
        for rank, (xp, dm, rm, am) in zip(ranks, VETERANCY):
            assert rank.get_editor_property("xp_threshold") == xp
            for key, value in (("damage_multiplier", dm), ("range_multiplier", rm), ("armor_multiplier", am)):
                check_float(rank.get_editor_property(key), value, "rank." + key)
        if spec["building"]:
            cells = asset.get_editor_property("footprint_cells")
            assert cells.x == spec["cells"] and cells.y == spec["cells"]
            assert asset.get_editor_property("power_supply") == spec["supply"]
            assert asset.get_editor_property("power_demand") == spec["demand"]
            check_float(asset.get_editor_property("connected_build_radius_cm"), 2300, "connected radius")
            assert asset.get_editor_property("provides_radar") == (spec["role"] == "Radar")
            assert asset.get_editor_property("requires_power_for_defense") == (spec["role"] == "Defense")
            assert names(asset.get_editor_property("allowed_production_types")) == sorted(PRODUCTION[v] for v in spec["allowed"])
        records.append(dict(asset=asset.get_path_name(), faction=faction, authored=spec,
            damage_type=damage(spec, faction), abilities=abilities(spec, faction)))
    asset = ASSETS["D_Faction_" + faction]
    assert str(TAGS.get_tag_name(asset.get_editor_property("faction_id"))) == "HeroHour.Faction." + faction
    assert str(asset.get_editor_property("display_name")) == faction_spec["label"]
    check_float(asset.get_editor_property("move_multiplier"), faction_spec["move"], faction + ".move")
    check_float(asset.get_editor_property("armor_multiplier"), faction_spec["armor"], faction + ".armor")
    for property_name, building in (("units", False), ("buildings", True)):
        actual = [value.get_path_name() for value in asset.get_editor_property(property_name)]
        expected = [ASSETS["D_" + faction + "_" + spec["role"]].get_path_name()
                    for spec in SPECS if spec["building"] == building]
        assert actual == expected, (faction, property_name, actual, expected)
    assert asset.get_editor_property("economy_style") == unreal.HREconomyStyle.REGROWING_FIELD
    assert asset.get_editor_property("build_style") == unreal.HRBuildStyle.SIDEBAR_PLACEMENT
    records.append(dict(asset=asset.get_path_name(), faction=faction, modifiers=faction_spec))
asset = ASSETS["D_DamageMatrix"]
rules = asset.get_editor_property("rules")
assert len(rules) == 9
seen = set()
for rule in rules:
    dt, at = rule.get_editor_property("damage_type"), rule.get_editor_property("armor_type")
    dn, an = str(TAGS.get_tag_name(dt)).split(".")[-1], str(TAGS.get_tag_name(at)).split(".")[-1]
    assert (dn, an) not in seen
    seen.add((dn, an))
    expected = MATRIX[dn][ARMORS.index(an)]
    check_float(rule.get_editor_property("multiplier"), expected, "matrix authored pair")
    check_float(asset.get_damage_multiplier(dt, at), expected, "matrix runtime lookup")
records.append(dict(asset=asset.get_path_name(), rows=MATRIX, armor_columns=ARMORS))
assert len(ASSETS) == len(records) == 25
for name in ASSETS:
    assert (ROOT / "Content/HERO/DataV4" / (name + ".uasset")).is_file(), "Package not saved: " + name
output = ROOT / "Saved/Research" / ("rts-definitions-v4-verify.json" if VERIFY_ONLY else "rts-definitions-v4.json")
output.parent.mkdir(parents=True, exist_ok=True)
output.write_text(json.dumps(dict(mode="verify_saved" if VERIFY_ONLY else "create_or_update",
    scope="HeroHour-owned balance; complete two-faction roster", records=records),
    indent=2, ensure_ascii=False), encoding="utf-8")
unreal.log("HR_DEFINITIONS_V4_VERIFY_PASS" if VERIFY_ONLY else "HR_DEFINITIONS_V4_CREATE_PASS")
