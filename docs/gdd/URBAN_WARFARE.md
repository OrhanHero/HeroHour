# HeroHour — Urban Warfare & Civilian Infrastructure Technical Specification

> Source: *HERO_HOUR_Ultimate_Core_Specification_v4.docx* — Section 5

---

## Overview

Maps are **living tactical environments** with dynamic civilian elements that can be captured, destroyed, or weaponized. Not cosmetic — every element has gameplay function.

---

## 5.1 Civilian Infrastructure & Capture Mechanics

### Civilian Vehicles

| Vehicle Type | Spawn Context | Health | GLA/HERO Interaction |
|--------------|---------------|--------|----------------------|
| **Sedan** | Residential streets, parking lots | 150 | Garrison 1 infantry → **Improvised VBIED** (suicide, 800 dmg, 5m radius) |
| **Bus** | Bus stops, highways | 400 | Garrison 4 infantry → **Mobile Militia Platform** (4 gun ports, 360° fire) |
| **Tractor** | Rural/industrial edges | 300 | Garrison 2 infantry → **Heavy VBIED** (1500 dmg, 8m radius, leaves burning debris) |
| **Truck** | Industrial zones, depots | 350 | Garrison 3 infantry → **Supply Truck** (heals nearby friendly units 20 HP/s) |

### Implementation
```fsharp
// F# Type (Source/HeroHourSimulation/src/Urban/CivilianVehicles.fs)
type CivilianVehicle =
    | Sedan of VehicleState
    | Bus of VehicleState
    | Tractor of VehicleState
    | Truck of VehicleState

type VehicleConversion =
    | VBIED of Damage: float * Radius: float
    | MilitiaPlatform of GunPorts: int * FireRate: float
    | HeavyVBIED of Damage: float * Radius: float * BurningDebris: bool
    | SupplyTruck of HealPerSecond: float * Radius: float
```

### Conversion Mechanics
1. **GLA/HERO only** — Other factions cannot interact
2. **Infantry garrison** — Right-click vehicle with selected infantry
3. **Instant conversion** — No build time, vehicle model swaps
4. **Permanent** — Cannot be reverted; vehicle becomes hostile to all

---

### Large Urban Buildings (Garrisonable Structures)

| Building | Capacity | Floors | Fire Ports | Strategic Value |
|----------|----------|--------|------------|-----------------|
| **Skyscraper** | 20 | 10 | 4 per floor | Height advantage, vision over walls |
| **Shopping Mall** | 15 | 3 | 3 per floor | Covers wide area, multiple exits |
| **Apartment Block** | 12 | 6 | 2 per floor | Dense urban chokepoints |
| **Warehouse** | 8 | 2 | 4 per floor | Industrial zones, vehicle cover |
| **Office Building** | 10 | 5 | 3 per floor | Commercial districts |

### Garrison Mechanics
- **Engineer capture** → Neutral building becomes player-owned
- **Infantry garrison** → Right-click with infantry selected (max capacity)
- **Fire ports** → Auto-target enemies in range; 180° arc per port
- **Destructible floors** → Heavy damage collapses upper floors (physics)
- **Vertical combat** → Units can fight floor-to-floor

---

## 5.2 Capturable High-Tech Structures (Engineer Capture)

### Capture Process
1. **Engineer unit** approaches neutral structure
2. **Capture ability** (30s channel, interruptible)
3. **On completion** → Structure ownership transfers
4. **Global effect activates** for capturing player only

### Structures

| Structure | Global Effect | Visual Indicator |
|-----------|---------------|------------------|
| **Civilian Oil Refinery** | +50 Credits/sec continuous income | Flaming flare stack (owner color) |
| **Civilian Major Hospital** | All injured infantry (owner) auto-heal 15 HP/s when out of combat for 5s | Green cross glow (owner color) |
| **Intelligence Outpost** | Every 90s: reveals ALL enemy build/production queues for 10s | Satellite dish rotates (owner color) |

### Technical Implementation

```fsharp
// F# Type (Source/HeroHourSimulation/src/Urban/CapturableStructures.fs)
type CapturableStructure =
    | OilRefinery of StructureState
    | MajorHospital of StructureState
    | IntelligenceOutpost of StructureState

type GlobalEffect =
    | CreditIncome of float // per second
    | PassiveHeal of HealRate: float * OOCThreshold: float
    | IntelReveal of Interval: float * Duration: float
```

### Capture Rules
- **Engineer only** — No other unit can capture
- **Channel time**: 30 seconds (interruptible by damage)
- **Cooldown**: 120s before same structure can be recaptured
- **Destruction**: If destroyed, respawns neutral after 5 minutes
- **Visual**: Owner's faction color on key meshes + flag

---

## UE5 Presentation Integration

### Civilian Vehicle Visual States
| State | Visual |
|-------|--------|
| **Neutral** | Standard civilian paint, no faction markers |
| **Garrisoned** | Infantry visible in windows/ports |
| **Converted (VBIED)** | Faction decals, warning decals, smoke particles |
| **Converted (Platform)** | Gun barrels visible, faction banner |
| **Destroyed** | Burning wreck, persistent debris (physics) |

### Building Visual States
| State | Visual |
|-------|--------|
| **Neutral** | Standard textures, white flag |
| **Capturing** | Progress bar above, engineer particles |
| **Owned** | Faction flag + color tint on key meshes |
| **Garrisoned** | Infantry silhouettes in windows, muzzle flash |
| **Damaged** | Progressive decals, broken windows, smoke |
| **Collapsed** | Physics rubble pile, persistent |

### Intelligence Outpost Effect (UE5)
- **90s timer** → UI countdown widget
- **Reveal event** → All enemy production queues flash in world + minimap
- **Sound** → Global "satellite ping" SFX + MetaSound intel sweep

---

## F# Simulation Events

```fsharp
// Urban Events (Source/HeroHourSimulation/src/Urban/UrbanEvents.fs)
type UrbanEvent =
    | VehicleGarrisoned of VehicleId * PlayerId * InfantryList
    | VehicleConverted of VehicleId * VehicleConversion * PlayerId
    | BuildingCaptured of BuildingId * PlayerId * CapturableType
    | BuildingGarrisoned of BuildingId * PlayerId * InfantryList
    | FloorCollapsed of BuildingId * FloorIndex
    | IntelligenceRevealed of PlayerId * EnemyQueueData list
    | PassiveHealTick of PlayerId * UnitId list
```

---

## Balance Considerations

| Element | Risk | Mitigation |
|---------|------|------------|
| **VBIED spam** | High | GLA/HERO only; requires infantry investment; 30s cooldown per vehicle |
| **Building camping** | Medium | Floors destructible; artillery/flamethrowers clear garrisons |
| **Refinery snowball** | High | Single capture point; contested; destroyable; 5min respawn |
| **Hospital heal** | Low | Only out-of-combat; 5s delay; infantry only |
| **Intel reveal** | Medium | 90s interval; 10s duration; predictably timed |

---

## Map Design Requirements

### Urban Map Checklist
- [ ] **Civilian vehicles**: 8-12 per map, distributed in residential/commercial
- **Garrisonable buildings**: 4-6 large structures at strategic chokepoints
- **Capturable structures**: 1 of each type per map (3 total), placed centrally located
- **Destruction paths**: Clear lines of fire for artillery to hit garrisoned buildings
- **Vertical space**: At least 2 skyscrapers for vertical combat
- **Vehicle navigation**: Roads wide enough for converted platforms

### Example Layout (4-player Urban)
```
[Refinery]     [Hospital]     [Intel Outpost]
      \           |           /
       \          |          /
    [Skyscraper] [Mall] [Apartment]
       /          |          \
      /           |           \
[Bus Stop]  [Tractor Lot]  [Sedan Parking]
```

---

## Related Systems

| System | Integration |
|--------|-------------|
| **Weather** | Sandstorm hides VBIEDs; Rain reduces VBIED visibility |
| **Combat School** | Tracks VBIED efficiency, garrison kills, capture timings |
| **Generals Challenge** | Urban Specialist General uses enhanced VBIEDs |
| **Modding** | Data-driven: new vehicle types, building types via DataAssets |

---

*See `docs/technical/WEATHER_SYSTEM.md` for weather interactions with urban elements.*