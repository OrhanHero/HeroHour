# HeroHour — Dynamic Weather System Technical Specification

> Source: *HERO_HOUR_Ultimate_Core_Specification_v4.docx* — Section 4.3

---

## Overview

The weather system is a **deterministic, simulation-driven** feature that directly modifies gameplay stats. Not cosmetic — every weather state applies concrete stat modifiers via the F# simulation layer.

---

## Weather States

```fsharp
// F# Type Definition (Source/HeroHourSimulation/src/Weather/WeatherTypes.fs)
type WeatherState =
    | Clear
    | HeavyRain
    | Sandstorm
    | HeavySnow
    | NuclearWinter // Late-game superweapon aftermath

type WeatherIntensity = {
    Intensity: float // 0.0 - 1.0
    Duration: float  // Remaining seconds
    Transition: float // 0.0 - 1.0 (lerp between states)
}
```

---

## Stat Modifiers per Weather

| Weather | Modifier | Value | Affected |
|---------|----------|-------|----------|
| **Heavy Rain** | LaserDamageMult | 0.7 | USA, Japan |
|  | LaserRangeMult | 0.7 | USA, Japan |
|  | MovementSpeedMult (Air) | 0.9 | All aircraft |
|  | VisionRangeMult | 0.85 | All ground |
| **Sandstorm** | VisionRangeMult | 0.3 | All units/towers |
|  | RadarEnabled | false | All radar systems |
|  | MovementSpeedMult (Ground) | 0.8 | Tracked/Wheeled |
|  | StealthDetectionMult | 0.1 | All detection |
| **Heavy Snow** | MovementSpeedMult (Tracked) | 0.75 | Russia, China, USA |
|  | MovementSpeedMult (Wheeled) | 0.85 | Türkiye, Pakistan |
|  | MovementSpeedMult (Mecha) | 1.0 | Japan (unaffected) |
|  | TurnRateMult (Tracked) | 0.7 | All tracked |
|  | FreezeChance (Infantry) | 0.05/s | All infantry |

---

## Weather Transitions

- **Deterministic**: Seeded at match start; same seed = same weather timeline
- **Duration**: 60-180s per state (configurable per map)
- **Transition**: 10s lerp between modifier values
- **Map Override**: Maps can define weather weights (desert → more sandstorms)

---

## UE5 Presentation Integration

| Component | Technology |
|-----------|------------|
| **Precipitation** | Niagara Particle Systems (GPU sim) |
| **Fog/Visibility** | Volumetric Fog + Exponential Height Fog |
| **Ground Wetness** | Material Parameter Collection (wetness 0-1) |
| **Snow Accumulation** | Vertex displacement + texture blend |
| **Wind** | Wind Directional Source + foliage animation |

### Material Parameter Collection (MPC_Weather)
```cpp
// Updated each tick via UFSharpBridge
MPC_Weather.SetScalarParameterValue("RainIntensity", Weather.RainIntensity);
MPC_Weather.SetScalarParameterValue("SnowAccumulation", Weather.SnowAccumulation);
MPC_Weather.SetScalarParameterValue("SandDensity", Weather.SandDensity);
MPC_Weather.SetVectorParameterValue("FogColor", Weather.FogColor);
```

---

## Network Synchronization

- **Seed-only sync**: Initial weather seed sent in match start packet
- **Deterministic progression**: All clients simulate identical timeline
- **No per-tick weather packets** — zero bandwidth cost

---

## Map Weather Profiles

```json
// Content/Maps/WeatherProfiles/DesertMap.json
{
  "MapName": "Sahara_Conflict",
  "WeatherWeights": {
    "Clear": 0.4,
    "Sandstorm": 0.4,
    "HeavyRain": 0.1,
    "HeavySnow": 0.1
  },
  "MinStateDuration": 90,
  "MaxStateDuration": 240
}
```

---

## Testing Checklist

- [ ] Rain reduces laser damage by exactly 30%
- [ ] Sandstorm disables radar UI elements
- [ ] Snow reduces tracked speed by 25% (not wheeled/mecha)
- [ ] Transitions are smooth (10s lerp, no pop)
- [ ] Deterministic: Same seed = identical weather on all clients
- [ ] MPC parameters update correctly in editor PIE

---

*Related: `docs/technical/URBAN_WARFARE.md`, `docs/gdd/WEATHER_SYSTEM.md`*