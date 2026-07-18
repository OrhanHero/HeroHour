# HeroHour — Urban Warfare & Civilian Infrastructure

> Source: *HERO_HOUR_Ultimate_Core_Specification_v4.docx* — Section 5: Neutral Elements & Urban Warfare

---

## Design Philosophy

> Maps are **living, tactically usable civilian environments** — not empty arenas.

Civilian infrastructure provides emergent gameplay opportunities and faction-specific interactions.

---

## 5.1 Civilian Infrastructure & Capture Mechanics

### Civilian Vehicles (Dynamic Props)
| Vehicle Type | Locations | Capacity | **HERO** Conversion |
|--------------|-----------|----------|---------------------|
| **Sedan/SUV** | Residential, Roads | 2 infantry | SVBIED (high damage, small radius) |
| **Bus** | Urban centers, Highways | 8 infantry | Mobile Militia Platform (4 gun ports) |
| **Tractor/Trailer** | Industrial, Rural | 4 infantry | Ammo Carrier (supplies nearby) |
| **Fuel Tanker** | Highways, Depots | — | Rolling Bomb (massive fire AoE) |

### Conversion Mechanics (HERO Only)
```
1. Infantry approaches civilian vehicle (interaction radius: 3m)
2. "Capture" command → garrison animation (2s)
3. Vehicle transforms:
   - Visual: Civilian texture → Faction texture + improvised armor
   - Physics: Mass increases, speed decreases
   - Weapons: Added based on garrisoned infantry type
4. Vehicle becomes player-controlled unit
```

### Large Urban Buildings (Garrisonable)
| Building Type | Max Garrison | Fireports | Special |
|---------------|--------------|-----------|---------|
| **Skyscraper** | 20 | 8 (4 sides) | Height advantage (+range) |
| **Shopping Mall** | 15 | 6 | Wide fire arcs |
| **Apartment Block** | 12 | 4 | Dense cover |
| **Industrial Warehouse** | 10 | 4 | Vehicle garrison (2 light) |
| **Hospital** | 8 | 2 | Auto-heal garrisoned |

**Garrison Rules**:
- Infantry only (no vehicles except Warehouse)
- Units inside take 50% reduced damage
- Building HP shared — destroyed = all garrisoned units die
- Clear building: "Evacuate" command or destroy building

---

## 5.2 Capturable High-Tech Structures (Engineer Capture)

Neutral structures capturable by **Engineer-type units** (all factions). Capture time: 5s channeling.

| Structure | Capture Benefit | F# Economy Effect |
|-----------|-----------------|-------------------|
| **Civilian Oil Refinery** | +50 Credits/tick | Continuous credit stream |
| **Civilian Major Hospital** | Map-wide passive heal aura | All injured infantry auto-heal when OOC |
| **Intelligence Outpost** | Reveals enemy build queues every 90s | Intel event: `EnemyQueueRevealed` |

### Capture Mechanics
1. Engineer reaches structure (interaction radius)
2. "Capture" command → 5s channel (interruptible by damage)
3. On success: Structure changes ownership, visual updates
4. Benefits apply immediately to capturing player
5. Can be recaptured by enemy engineers

### Visual Indicators
- **Neutral**: White outline, civilian texture
- **Captured**: Faction color outline, faction banner/decal
- **Contested**: Blinking outline during capture channel

---

## 5.3 Urban Map Design Guidelines

### Chokepoint Philosophy
- **3-5 major chokepoints** per 2v2 map
- Each chokepoint has **2-3 flanking routes** (sewers, rooftops, parks)
- Civilian buildings placed to create **natural strongpoints** at chokepoints

### Civilian Density Zones
| Zone | Civilian Count | Building Types | Strategic Value |
|------|----------------|----------------|-----------------|
| **Downtown Core** | High | Skyscrapers, Malls, Hotels | High-value objectives |
| **Residential** | Medium | Apartments, Schools | Flank routes, garrisons |
| **Industrial** | Low | Warehouses, Refineries | Vehicle capture, economy |
| **Suburban** | Low | Houses, Gas Stations | Early game skirmishes |

### Destruction & Persistence
- **Buildings**: Persistent damage states (Intact → Damaged → Ruined)
- **Rubble**: Physics-enabled debris blocks paths, creates new cover
- **Fire**: Spreads between adjacent buildings (Niagara + gameplay)
- **Rebuild**: Engineers can repair to "Damaged" state (not "Intact")

---

## 5.4 Faction-Specific Urban Mechanics

### HERO — Civilian Vehicle Conversion
- Only faction with **"Improvised Warfare"** doctrine
- Conversion speed: 2s base, 1s with "Scavenger" upgrade
- Converted vehicles retain civilian collision (can't crush infantry)

### USA / China / Russia — Engineering Corps
- **Combat Engineers** capture High-Tech Structures 50% faster
- Can **repair** captured structures to full HP
- **"Fortify"** ability: +50% structure HP, +2 fireports (60s cooldown)

### Japan — Mecha Urban Mobility
- **Tetsujin Walker** jumps **onto building rooftops** (garrison from above)
- Bypasses street-level chokepoints entirely

### Pakistan — Fog of War Urban Superiority
- **Guerrilla Regeneration** works **inside buildings** (not just FoW)
- **Hidden Training Camp** produces units inside urban zones invisibly

---

## 5.5 Technical Implementation

### F# Simulation Side
```fsharp
type CivilianVehicle =
    { Id: EntityId
      Type: CivilianVehicleType
      Position: Position
      State: VehicleState
      GarrisonedInfantry: UnitId list }

type UrbanBuilding =
    { Id: EntityId
      Type: BuildingType
      Position: Position
      Bounds: Bounds
      MaxGarrison: int
      CurrentGarrison: UnitId list
      Health: float
      Owner: PlayerId option
      FirePorts: FirePort list }

type CapturableStructure =
    { Id: EntityId
      Type: CapturableType
      Position: Position
      CaptureProgress: float  // 0.0 - 1.0
      CapturingEngineer: UnitId option
      Owner: PlayerId option }
```

### UE5 Presentation Side
- **Civilian Vehicles**: `AActor` with `UCivilianVehicleComponent` → converted to faction unit on capture event
- **Buildings**: `ABuildingActor` with `UGarrisonComponent` + `UDestructibleComponent`
- **High-Tech Structures**: `ACapturableStructure` with `UCaptureProgressWidget`

### Network Events (Deterministic)
```fsharp
type UrbanEvent =
    | VehicleCaptured of UnitId * PlayerId * CivilianVehicleType
    | BuildingGarrisoned of BuildingId * UnitId list
    | BuildingEvacuated of BuildingId * UnitId list
    | StructureCaptured of StructureId * PlayerId
    | StructureRecaptured of StructureId * PlayerId
    | BuildingDamaged of BuildingId * float
    | BuildingDestroyed of BuildingId
```

---

## 5.6 Map Scripting Examples

### Objective: Capture the Hospital
```json
{
  "ObjectiveId": "Capture_Hospital",
  "Description": "Capture and hold the Central Hospital for 5 minutes",
  "TargetBuilding": "Hospital_Central",
  "HoldDuration": 300,
  "Reward": { "TechPoints": 500, "Unlock": "Medical_Upgrade_Tier2" }
}
```

### Dynamic Civilian Traffic
```fsharp
// Spawns civilian vehicles on roads every 30-60s
let spawnCivilianTraffic (map: MapData) : unit =
    map.RoadNetwork.MainRoads
    |> Seq.iter (fun road ->
        if Random.NextDouble() < 0.3 then
            spawnVehicle road.RandomPoint CivilianVehicleType.Sedan)
```

---

## Testing Checklist

- [ ] Civilian vehicles spawn on roads, follow traffic rules
- [ ] **HERO** can convert all 4 vehicle types
- [ ] Converted vehicles have correct weapons by infantry type
- [ ] All 5 building types garrison correctly (limits, fireports)
- [ ] Building destruction kills garrisoned units
- [ ] 3 High-Tech structures capturable by all factions
- [ ] Capture channel interruptible by damage
- [ ] Recapture works (enemy engineer steals)
- [ ] Visual ownership indicators update correctly
- [ ] Rubble blocks pathfinding, creates cover
- [ ] Fire spreads between adjacent buildings
- [ ] Deterministic: same seed = same civilian traffic

---

*Related: `docs/technical/WEATHER_SYSTEM.md`, `docs/gdd/FRACTIONS_OVERVIEW.md`*