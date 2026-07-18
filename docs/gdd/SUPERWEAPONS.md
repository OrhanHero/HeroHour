# HeroHour — Superweapons Design Specification

> **Source**: *HERO_HOUR_Multiplayer_Balancing_Specification.docx* — Section 2
> **Version**: 1.0 — Pure Multiplayer Competitive Architecture

---

## Design Philosophy

Every superweapon in HeroHour follows these competitive principles:

1. **Exactly ONE per faction** — No redundancy, clear identity
2. **Global warning** — 8-12s before impact, visible to all players
3. **Counterplay exists** — Every superweapon has specific counters (super-agents, abilities, structures)
4. **Deterministic execution** — F# simulation calculates exact impact, no RNG
5. **Cooldown 7-9 minutes** — Strategic decision, not spam
6. **Visual clarity** — Distinct audio/visual tell for competitive readability

---

## Superweapon Registry

| Faction | Superweapon | Type | Cooldown | Warning | Radius | Counter |
|---------|-------------|------|----------|---------|--------|---------|
| 🇺🇸 USA | **Chrono-Laser-Phalanx** | Orbital Line Strike | 7 min | 8s | 300m line | K-Sat Cyber-Strahl, Guerilla Shadow |
| 🇨🇳 China | **Nuke-Silicon-Kollaps** | Nuke + EMP Hybrid | 9 min | 12s | 400m + EMP 2km | Underground Silo, Shahin Shield |
| 🏴‍☠️ HERO | **Singularitäts-Schrottkanone** | Gravity Singularity | 7 min | 8s | 500m pull | Omega Boris, Specter Prime |
| 🇷🇺 Russia | **Tesla-Giga-Spule** | Chain Lightning Storm | 8 min | 10s | 600m sector | Omega Boris, Viper |
| 🇮🇷 Iran | **Sejjil-III Giga-Silo** | Staggered Tri-Missile | 9 min | 12s | 350m x3 | Underground Silo, Shahin Shield |
| 🇹🇷 Türkiye | **TAF-Anka Global Strike** | 500 Micro-Missile Swarm | 8 min | 10s | Full map (prioritizes AA) | Kaan, K-Sat Cyber-Strahl |
| 🇵🇰 Pakistan | **Giga-EMP-Kaskade** | Atmospheric EMP Cascade | 8 min | 10s | Global (60s shutdown) | Viper, Guerilla Shadow |
| 🇮🇳 India | **Agni-V Plasmabrenner** | Orbital Plasma Railgun | 8 min | 10s | 300m plasma vent | Akash, Shinobi 2.0 |
| 🇯🇵 Japan | **Mecha-Giga-Wave** | Resonance Shockwave | 8 min | 10s | Hemisphere | Shinobi 2.0, Akash |
| 🇰🇷 South Korea | **K-Sat Cyber-Strahl** | Satellite Cyber Beam | 7 min | 8s | Global (timer block) | K-Sat Cyber-Strahl (self), Viper |

---

## Detailed Superweapon Specifications

### 🇺🇸 USA — Chrono-Laser-Phalanx
```
Type: Orbital Line Strike
Execution: Satellite fires temporal laser; draws line across map (player chooses angle/position)
Damage: Instant structure deletion (100% HP), units vaporized
Travel Time: 0s (temporal targeting)
Warning: 8s — Satellite aligns, red laser targeting line appears globally
Cooldown: 7 minutes
Global Tell: Distinctive satellite hum + red line projection on minimap
```
**Counterplay:**
- **K-Sat Cyber-Strahl** (South Korea): Blocks firing timer globally for 30s
- **Guerilla Shadow** (Pakistan): Rolls back superweapon timer by 60s
- **Specter Prime** (USA): Phases through terrain, can intercept targeting satellite

---

### 🇨🇳 China — Nuke-Silicon-Kollaps
```
Type: Thermonuclear + EMP Hybrid
Execution: ICBM launch → atmospheric detonation → radiation zone + EMP pulse
Damage: Center 500m vaporized, 2km EMP ring disables ALL electronics 45s
Radiation: Persistent 3 min, 50 HP/s to units in zone
Warning: 12s — Missile silo doors open, launch flare visible globally
Cooldown: 9 minutes
Global Tell: ICBM trail + distinctive "silicon collapse" audio cue
```
**Counterplay:**
- **Underground Silo** (Iran): Immune to all conventional + superweapon attacks while closed
- **Commander Shahin** (Iran): Deploys immunity shield to all buildings in radius
- **Guerilla Shadow** (Pakistan): Rolls back timer 60s

---

### 🏴‍☠️ HERO — Singularitäts-Schrottkanone
```
Type: Gravity Singularity Projectile
Execution: Fires compressed scrap singularity → pulls all units/structures in 500m → crushes → ejects as explosion → converts mass to credits
Damage: Structures 80% HP, units instant-kill if pulled to center
Mass Conversion: 1 credit per 10 HP crushed
Warning: 8s — Magnetic anomaly visual + audio hum globally
Cooldown: 7 minutes
Global Tell: Distinctive magnetic distortion shader + deep bass hum
```
**Counterplay:**
- **Omega Boris** (Russia): Tesla aura disrupts singularity formation in radius
- **Specter Prime** (USA): Phases through terrain, avoids pull
- **Nexus Core** (HERO): Can absorb singularity energy, convert to tech unlock

---

### 🇷🇺 Russia — Tesla-Giga-Spule
```
Type: Chain Lightning Storm
Execution: Massive Tesla coil activates → chain lightning arcs across entire sector (600m)
Behavior: Prioritizes high-value targets (super-agents, superweapons, expensive units)
Damage: 400 HP per arc, chains up to 15 targets
Warning: 10s — Atmospheric ionization visual + ozone smell audio
Cooldown: 8 minutes
Global Tell: Thunderstorm forms on map, distinctive crackling audio
```
**Counterplay:**
- **Omega Boris** (Russia): Tesla aura grounds lightning, protects nearby units
- **Viper** (South Korea): Calls instant airstrike on coil before discharge
- **Faraday Cage Upgrade** (Universal): Reduces chain count by 50%

---

### 🇮🇷 Iran — Sejjil-III Giga-Silo
```
Type: Staggered Tri-Ballistic Missile
Execution: Three coupled missiles launch → time-staggered impacts (3s apart)
- Missile 1: Shield breaker (kinetic penetrator)
- Missile 2: Bunker buster (delayed fuse)
- Missile 3: Payload delivery (HE/chemical/radiation)
Damage: Progressive — each missile exploits previous impact
Warning: 12s — Three silo doors open sequentially, triple flare
Cooldown: 9 minutes
Global Tell: Three distinct launch signatures, escalating audio
```
**Counterplay:**
- **Underground Silo** (Iran): Immune while closed (only faction with this)
- **Commander Shahin** (Iran): Immunity shield covers all own buildings
- **Early Intercept** (USA Patriot/SK K-Sat): Can shoot down missiles 1 & 2

---

### 🇹🇷 Türkiye — TAF-Anka Global Strike
```
Type: Coordinated Micro-Missile Swarm (500 missiles)
Execution: Orbital platform deploys 500 micro-missiles → autonomous targeting
Priority: AA towers > superweapons > production > economy
Damage: 50 per missile, distributed intelligently
Warning: 10s — Satellite deployment visual + targeting laser grid
Cooldown: 8 minutes
Global Tell: Satellite deployment animation + targeting laser grid on minimap
```
**Counterplay:**
- **Kaan** (Türkiye): Shoots down incoming missiles at 100% accuracy
- **K-Sat Cyber-Strahl** (South Korea): Blocks satellite targeting for 30s
- **Distributed AA** (All): Spread AA coverage reduces effectiveness

---

### 🇵🇰 Pakistan — Giga-EMP-Kaskade
```
Type: Atmospheric EMP Cascade
Execution: High-altitude detonation → global electronics shutdown in radius
Effect: ALL electronics disabled 60s (power plants, vehicles, radar, superweapons)
Friendly Hardening: Pakistani units have 50% reduced duration
Warning: 10s — Atmospheric ionization + distinctive EMP charge sound
Cooldown: 8 minutes
Global Tell: Screen flicker shader + EMP charge audio buildup
```
**Counterplay:**
- **Viper** (South Korea): Calls airstrike before EMP lands (zero delay)
- **Guerilla Shadow** (Pakistan): Manipulates minefields, rolls back timer 60s
- **Faraday Hardening** (Universal Upgrade): Reduces duration to 20s

---

### 🇮🇳 India — Agni-V Plasmabrenner
```
Type: Orbital Railgun-Fired Plasma Slug
Execution: Railgun fires plasma slug from orbit → penetrates crust → creates volcanic vent
Effect: 300m radius plasma inferno, melts armor (ignores armor values), denies area 3 min
Damage: 500 HP/s to all in zone, armor ignored
Warning: 10s — Orbital railgun charge visible + plasma buildup audio
Cooldown: 8 minutes
Global Tell: Orbital railgun trace + plasma ignition sound
```
**Counterplay:**
- **Akash** (India): Hacks enemy minimap, creates false vent locations
- **Shinobi 2.0** (Japan): Teleports units out of zone instantly
- **Thermal Vents** (Map Feature): Some maps have natural vents that reduce damage

---

### 🇯🇵 Japan — Mecha-Giga-Wave
```
Type: Resonance Shockwave from Quantum Cores
Execution: All friendly Quantum Cores synchronize → emit resonance pulse
Effect: Overloads ALL shields in hemisphere (enemy + friendly), generates shockwaves
Friendly Fire: Own mechas HEAL (quantum resonance), enemy mechanical units shatter
Warning: 10s — Quantum core synchronization visual + harmonic buildup audio
Cooldown: 8 minutes
Global Tell: Distinctive harmonic resonance + all quantum cores glow intensely
```
**Counterplay:**
- **Shinobi 2.0** (Japan): Teleports out of hemisphere
- **Akash** (India): Hacks minimap, creates false resonance sources
- **Non-Mechanical Units** (Infantry, Biological): Unaffected

---

### 🇰🇷 South Korea — K-Sat Cyber-Strahl
```
Type: Satellite Cyber Beam
Execution: Satellite fires focused cyber pulse → rewrites enemy superweapon logic
Effect: Blocks ALL enemy superweapon timers for 30s, reveals ALL stealth units globally
Secondary: Rewrites enemy AI targeting priorities temporarily
Warning: 8s — Satellite targeting laser + digital distortion visual
Cooldown: 7 minutes
Global Tell: Distinctive digital distortion shader + "data stream" audio
```
**Counterplay:**
- **K-Sat Cyber-Strahl** (Mirror): Can block enemy K-Sat
- **Viper** (South Korea): Instant airstrike on satellite before fire
- **Analog Hardening** (Universal Upgrade): Reduces timer block to 10s

---

## Superweapon State Machine (F#)

```fsharp
// Source/HeroHourSimulation/src/Superweapons/SuperWeaponTypes.fs

type SuperweaponState =
    | Ready
    | Charging of float // seconds remaining
    | Firing of FiringData
    | Cooldown of float // seconds remaining
    | Disabled // e.g., by K-Sat Cyber-Strahl

type FiringData = {
    Origin: Position
    Target: Position option
    Angle: float option
    ImpactTick: int64
    WarningStartTick: int64
}

type Superweapon = {
    Id: SuperweaponId
    Faction: Faction
    Config: SuperweaponConfig
    State: SuperweaponState
    LastFiredTick: int64 option
}

type SuperweaponConfig = {
    CooldownTicks: int64
    WarningTicks: int64
    Radius: float
    DamageProfile: DamageProfile
    GlobalWarning: bool
    CounterplayTags: CounterplayTag list
}
```

---

## Network Synchronization

| Aspect | Method |
|--------|--------|
| **State Sync** | Deterministic lockstep — superweapon state derived from tick + inputs |
| **Warning Event** | Replicated via `GameEvent.SuperweaponWarning` (includes origin, target, type) |
| **Impact Event** | `GameEvent.SuperweaponImpact` with exact positions, damage list |
| **Cooldown** | Derived from `LastFiredTick + CooldownTicks` — no separate sync needed |
| **Cancel** | `K-Sat Cyber-Strahl` sets `State = Disabled` for duration; replicated as state change |

---

## Competitive Balance Checklist

- [ ] All superweapons have 7-9 min cooldown
- [ ] All have 8-12s global warning
- [ ] Every superweapon has ≥2 specific counters
- [ ] Visual/audio tells distinct and readable at competitive level
- [ ] No superweapon is "fire and forget" — all require strategic timing
- [ ] Damage numbers tuned for 1v1 and 2v2 viability
- [ ] No superweapon can end game single-handedly without setup

---

*Related: `docs/gdd/SUPER_AGENTS.md`, `docs/technical/SuperWeaponTypes.md`*