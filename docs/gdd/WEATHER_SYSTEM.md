# HeroHour — Weather System Game Design Document

> Part of the Ultimate Core Specification v4

---

## Core Philosophy

> **Weather is not cosmetic — it is a core tactical mechanic that directly modifies unit stats, abilities, and faction balance.**

Every weather state applies deterministic, simulation-layer stat modifiers. No visual-only effects.

---

## Weather States Overview

| Weather | Icon | Frequency | Duration | Core Mechanic |
|---------|------|-----------|----------|---------------|
| **Clear** | ☀️ | 40-50% | 60-180s | Baseline (no modifiers) |
| **Heavy Rain** | 🌧️ | 15-20% | 45-120s | **Laser weapons -30% dmg/range** |
| **Sandstorm** | 🌪️ | 10-15% | 30-90s | **-70% vision; radar disabled** |
| **Heavy Snow** | ❄️ | 10-15% | 60-150s | **Tracked vehicles -25% speed/turn** |
| **Nuclear Winter** | ☢️ | Superweapon only | Until match end | Global -50% vision; radiation DoT |

---

## Faction Weather Matrix

### Favored (Gain Advantage)
| Weather | Favored Factions | Why |
|---------|-----------------|-----|
| **Rain** | GLA, Pakistan, Iran | No laser units; toxin/EMP/ballistic unaffected |
| **Sandstorm** | GLA, Pakistan, Türkiye | Stealth + FoW synergy; Bayraktar marks for artillery |
| **Snow** | Japan, South Korea, USA | Mechas/air unaffected; microwave/energy weapons work |

### Disfavored (Suffer Penalty)
| Weather | Disfavored Factions | Penalty |
|---------|---------------------|---------|
| **Rain** | USA, Japan | Laser weapons -30% dmg/range |
| **Sandstorm** | All radar-reliant | Radar disabled; vision -70% |
| **Snow** | Russia, China, USA | Tracked vehicles -25% speed/turn |

---

## Detailed Modifier Tables

### Heavy Rain 🌧️
| Stat | Modifier | Affected |
|------|----------|----------|
| LaserDamageMult | 0.7 | USA (Specter, Microwave, Hellfire), Japan (Tetsujin plasma) |
| LaserRangeMult | 0.7 | Same as above |
| AirUnitSpeedMult | 0.9 | All aircraft |
| GroundVisionRangeMult | 0.85 | All ground units/towers |
| ProjectileSpreadMult | 1.15 | Unguided projectiles |
| InfantryMoveSpeedMult | 0.95 | All infantry |

### Massive Sandstorm 🌪️
| Stat | Modifier | Affected |
|------|----------|----------|
| VisionRangeMult | 0.3 | All units/towers |
| RadarEnabled | false | All radar systems |
| GroundMoveSpeedMult (Tracked) | 0.8 | Tracked vehicles |
| GroundMoveSpeedMult (Wheeled) | 0.9 | Wheeled vehicles |
| StealthDetectionMult | 0.1 | All detection |
| ProjectileAccuracyMult (Unguided) | 0.5 | Artillery, tank shells |

### Heavy Snow ❄️
| Stat | Modifier | Affected |
|------|----------|----------|
| MoveSpeedMult (Tracked) | 0.75 | Russia, China, USA tracked |
| TurnRateMult (Tracked) | 0.7 | Russia, China, USA tracked |
| MoveSpeedMult (Wheeled) | 0.85 | Türkiye, Pakistan wheeled |
| MoveSpeedMult (Mecha) | 1.0 | Japan (immune) |
| MoveSpeedMult (Air) | 0.95 | All aircraft |
| FreezeChancePerSec (Infantry) | 0.05 | All infantry (slows 50% for 3s) |
| VisionRangeMult | 0.9 | All units |

### Nuclear Winter ☢️ (Superweapon Aftermath)
| Stat | Modifier | Affected |
|------|----------|----------|
| GlobalVisionMult | 0.5 | All players |
| RadiationDamagePerSec | 5 HP | All units in open |
| HealingReceivedMult | 0.5 | All healing |
| CreditIncomeMult | 0.8 | All economies |
| Duration | Permanent | Until match end |

---

## Map Weather Profiles

```json
{
  "MapName": "Rhine_Valley",
  "Archetype": "Temperate_European",
  "WeatherWeights": {
    "Clear": 0.50,
    "HeavyRain": 0.30,
    "Sandstorm": 0.00,
    "HeavySnow": 0.20
  },
  "MinStateDuration": 60,
  "MaxStateDuration": 180
}
```

| Archetype | Clear | Rain | Sandstorm | Snow |
|-----------|-------|------|-----------|------|
| Temperate European | 0.5 | 0.3 | 0.0 | 0.2 |
| Desert/Middle East | 0.3 | 0.1 | 0.5 | 0.1 |
| Arctic/Northern | 0.3 | 0.2 | 0.0 | 0.5 |
| Tropical/Asian | 0.4 | 0.4 | 0.1 | 0.1 |
| Urban/Industrial | 0.5 | 0.2 | 0.2 | 0.1 |

---

## Deterministic Implementation

### Seed Generation
```fsharp
// Match start
let weatherSeed = MapSeed ^^^ MatchId ^^^ 0xDEADBEEF
let rng = Xorshift64Star(weatherSeed)

// Per tick
let nextWeatherState (current: WeatherState) (rng: Xorshift64Star) : WeatherState =
    if rng.NextDouble() < transitionProbability current then
        pickNextState rng current
    else
        current
```

### Properties
- **Zero bandwidth**: Only initial seed sent in match start packet
- **Deterministic**: Same seed = identical weather on all clients
- **Replay-perfect**: Weather timeline fully reproducible from seed

---

## UE5 Presentation Layer

### Niagara Systems
| Weather | System | Key Params |
|---------|--------|------------|
| Rain | `NS_Rain_Heavy` | `RainIntensity`, `DropSize`, `SplashScale` |
| Sandstorm | `NS_Sandstorm` | `SandDensity`, `WindDirection`, `ParticleSize` |
| Snow | `NS_Snow_Heavy` | `SnowIntensity`, `FlakeSize`, `AccumulationRate` |
| Nuclear Winter | `NS_NuclearWinter` | `AshDensity`, `GlowIntensity`, `RadiationPulse` |

### Material Parameter Collection: `MPC_Weather`
| Parameter | Type | Range | Source |
|-----------|------|-------|--------|
| `RainIntensity` | Scalar | 0-1 | F# tick |
| `SnowAccumulation` | Scalar | 0-1 | F# tick |
| `SandDensity` | Scalar | 0-1 | F# tick |
| `Wetness` | Scalar | 0-1 | Derived (Rain + time) |
| `FogDensity` | Scalar | 0-1 | F# tick |
| `FogColor` | Vector | RGB | Weather state |
| `WindDirection` | Vector | XYZ | Weather state |
| `WindSpeed` | Scalar | 0-100 | Weather state |

### Volumetric Fog
- `ExponentialHeightFog` per map
- `FogDensity` driven by `MPC_Weather.FogDensity`
- Sandstorm: `FogColor` = orange/brown
- Nuclear Winter: `FogColor` = dark gray, `FogDensity` = 2x

### Ground Accumulation
- **Snow**: Vertex displacement + texture blend (height-based)
- **Rain**: Puddle normal maps + wetness MPC
- **Sand**: Dust layer material blend

---

## Combat School Integration

| Metric | Description |
|--------|-------------|
| **WeatherAdaptationScore** | % of time player used weather-favored units |
| **WeatherCounterplayScore** | % of time player countered enemy weather advantage |
| **VisionManagement** | Units lost in sandstorm/snow vs clear |
| **LaserEfficiencyRain** | Laser damage dealt during rain vs clear |

---

## Testing Checklist

- [ ] Rain reduces laser damage by exactly 30%
- [ ] Sandstorm disables radar UI (minimap, unit radar icons)
- [ ] Snow reduces tracked speed by 25% (not wheeled/mecha)
- [ ] Transitions smooth (10s lerp, no pop)
- [ ] Deterministic: same seed = identical weather
- [ ] MPC parameters update each tick
- [ ] Niagara intensity matches MPC values
- [ ] Nuclear Winter persists correctly
- [ ] Map profiles load correct weights

---

*Related: `docs/technical/WEATHER_SYSTEM.md`, `docs/gdd/URBAN_WARFARE.md`*