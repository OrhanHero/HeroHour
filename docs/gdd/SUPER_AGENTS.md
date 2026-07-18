# HeroHour — Super-Agents (Hero Units) Design Document

> Source: *HERO_HOUR_Multiplayer_Balancing_Specification.docx* — Section 2.2

---

## Core Principles

| Rule | Description |
|------|-------------|
| **Exactly 1 per faction** | No exceptions, no variants |
| **Max 1 per player** | Enforced in F# simulation state; spawn fails if alive instance exists |
| **F# State Tree** | All behavior driven by deterministic State Trees |
| **Competitive Viability** | Balanced for 1v1 and 2v2; impactful but not game-ending alone |
| **Unique Identity** | Each agent defines their faction's competitive personality |

---

## Super-Agent State Machine (F#)

```fsharp
// Source/HeroHourSimulation/src/SuperAgents/HeroTypes.fs

type HeroInstanceState =
    | Available // Not yet deployed
    | Deployed of HeroDeployedState
    | Dead of RespawnTimer // Respawn after 3 minutes

type HeroDeployedState = {
    UnitId: UnitId
    CurrentHealth: float
    MaxHealth: float
    AbilityCooldowns: Map<AbilityId, float>
    CurrentStateTree: HeroStateTreeNode
    Veterancy: VeterancyLevel
}

type HeroStateTreeNode =
    | Idle
    | Moving of Position
    | Attacking of TargetId
    | UsingAbility of AbilityId * Target
    | Retreating
    | Dead

// Super-agent restriction enforced at spawn time:
let tryDeploySuperAgent (state: GameState) (playerId: PlayerId) (heroType: HeroType) : Result<Unit, string> =
    match state.Players.[playerId].SuperAgentInstance with
    | Some _ -> Error "Super-agent already active"
    | None ->
        // Validate resources, population, tech requirements
        Ok ()
```

---

## The 10 Super-Agents

---

### 🇺🇸 USA — Specter Prime (Colonel Burton Evolution)

| Property | Value |
|----------|-------|
| **Role** | Stealth Spectral Assassin / Surgical Strike |
| **Cost** | 3000 Credits, 50 Energy, 5 Tech Points |
| **Population** | 3 |
| **Health** | 800 (shielded: +200 regenerating) |
| **Speed** | 8.5 m/s (high) |
| **Production** | Satellite Uplink Command (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Phase Shift** | 15s | Becomes untargetable, moves through terrain/cliffs for 3s. Cannot attack during. |
| **Laser Designator** | 20s | Marks building/unit for 8s. All friendly air units gain +50% damage vs marked target. |
| **Temporal Decoy** | 30s | Deploys holographic copy at target location. Draws fire for 5s. Explodes on death (150 AoE). |
| **Specter Strike (Ultimate)** | 90s | Teleports behind target unit, executes guaranteed-crit melee strike (500 true damage). 3s channel, interruptible. |

#### State Tree: `SpecterPrimeState`
```
Idle
├── PhaseShift → (3s) → Idle
├── Move → Position
├── LaserDesignate → Target → (8s) → Idle
├── TemporalDecoy → Position → (5s) → Idle
├── SpecterStrike → Target → Channel(3s) → Strike → Idle
└── Retreat → Position → Idle
```

#### Competitive Role
- **Map Control**: Phase Shift crosses cliffs/rivers instantly
- **Intel**: Laser Designator enables surgical air strikes
- **Assassination**: Specter Strike removes key units (super-agents, commanders)
- **Counterplay**: Detectable by stealth detection; Phase Shift has audio tell; Specter Strike interruptible

---

### 🇨🇳 China — Lotus X (Black Lotus Evolution)

| Property | Value |
|----------|-------|
| **Role** | Cyber Infiltrator / Production Saboteur |
| **Cost** | 2800 Credits, 100 Energy, 3 Tech Points |
| **Population** | 2 |
| **Health** | 400 (low) |
| **Speed** | 6.0 m/s |
| **Production** | China Internet Center (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Stealth Infiltration** | Passive | Permanently stealthed while stationary. Moving breaks stealth for 2s. |
| **Production Infection** | 45s | Targets enemy production structure. For 45s: output reversed (produces Chinese units for enemy cost). |
| **Credit Siphon** | 30s | Hacks enemy supply line. Drains 50 credits/s for 10s, grants to China. |
| **Cyber Dominion (Ultimate)** | 120s | Global hack: Reveals ALL enemy production queues for 30s. Can remotely cancel one queued item per enemy. |

#### State Tree: `LotusXState`
```
Idle (Stealthed)
├── Infiltrate → Target Structure → Channel(3s) → Infection(45s) → Idle
├── Siphon → Target Supply → Channel(2s) → Siphon(10s) → Idle
├── CyberDominion → Channel(5s) → GlobalReveal(30s) → Idle
└── Escape → Stealth → Move → Idle
```

#### Competitive Role
- **Economic Warfare**: Credit Siphon + Production Infection disrupts enemy economy
- **Intel**: Cyber Dominion provides perfect scouting
- **Counterplay**: Stealth detection reveals; low HP means instant death if caught; Infection cancellable by engineer repair

---

### 🏴‍☠️ HERO — Nexus Core

| Property | Value |
|----------|-------|
| **Role** | Shapeshifting Tech-Thief / Adaptive Hybrid |
| **Cost** | 3500 Credits (no energy cost), 5 Tech Points |
| **Population** | 4 |
| **Health** | 1000 (regenerates 10/s out of combat) |
| **Speed** | 7.0 m/s (variable by form) |
| **Production** | Schwarzmarkt-Hub (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Form Shift** | 5s | Transforms into copy of last enemy unit killed (gains its stats/abilities for 30s). |
| **Tech Absorption** | Passive | Killing enemy super-agent permanently unlocks their faction's Tier 2 tech for HERO. |
| **Singularity Siphon** | 20s | Targets enemy vehicle/structure → disables for 5s, grants HERO credits = 50% target cost. |
| **Nexus Singularity (Ultimate)** | 180s | Deploys micro-singularity at target location. Pulls all units in 400m for 8s. Nexus Core gains all absorbed abilities permanently. |

#### State Tree: `NexusCoreState`
```
Idle
├── FormShift → TargetUnit → CopyStats(30s) → Idle
├── SingularitySiphon → Target → Disable(5s) + Credits → Idle
├── NexusSingularity → Channel(4s) → Singularity(8s) → AbsorbAbilities → Idle
└── Adapt → (Auto) → FormShift on kill
```

#### Competitive Role
- **Adaptability**: Form Shift counters specific threats dynamically
- **Snowball**: Tech Absorption rewards aggressive play
- **Ultimate Zone Control**: Nexus Singularity decides teamfights
- **Counterplay**: High priority target; Form Shift has tell; Tech Absorption requires killing super-agent (hard)

---

### 🇷🇺 Russia — Omega Boris

| Property | Value |
|----------|-------|
| **Role** | Tesla Titan / Frontline Anchor |
| **Cost** | 4000 Credits, 150 Energy, 5 Tech Points |
| **Population** | 6 |
| **Health** | 2500 (highest in game) + 500 Tesla Shield (regens 50/s) |
| **Speed** | 4.0 m/s (slow) |
| **Production** | Giga-Fabrik (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Tesla Aura** | Passive | 150m radius: enemy mechanical units take 25 DPS, 20% slow. Friendly vehicles gain 15% speed. |
| **Ground Slam** | 20s | Jumps to target location (ignores terrain), 400 AoE damage, 2s stun. |
| **Overload** | 30s | Next 3 attacks chain lightning to 5 targets (200 dmg each). |
| **Bomber Chain (Ultimate)** | 120s | Calls heavy bomber squadron. Carpet bombs 600m line. Each bomber drops thermobaric payload (300 dmg, fire DoT). |

#### State Tree: `OmegaBorisState`
```
Idle (Aura Active)
├── GroundSlam → Target → Leap → Impact(2s stun) → Idle
├── Overload → (3 attacks) → ChainLightning → Idle
├── BomberChain → TargetLine → CallIn(5s) → BombingRun → Idle
└── HoldPosition → AuraMax → Idle
```

#### Competitive Role
- **Frontline Anchor**: Holds ground, aura wins attrition
- **Zone Denial**: Bomber Chain controls massive area
- **Counterplay**: Slow, kiteable; Tesla Shield burstable; bombers interceptable by AA

---

### 🇮🇷 Iran — Commander Shahin

| Property | Value |
|----------|-------|
| **Role** | Fortress Commander / Defensive Anchor |
| **Cost** | 3200 Credits, 100 Energy, 4 Tech Points |
| **Population** | 3 |
| **Health** | 1200 |
| **Speed** | 5.0 m/s |
| **Production** | Underground Silo (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Immunity Protocol** | 45s | 150m radius: all friendly buildings gain 100% damage immunity for 10s. |
| **Precision Strike** | 25s | Calls Sejjil missile on target structure (guaranteed hit, 800 dmg). |
| **Instant Bunker** | 30s | Deploys pre-fab bunker at target location (garrison 5 infantry, 2000 HP). |
| **Iron Dome (Ultimate)** | 120s | 300m radius dome: projectiles intercepted, superweapons blocked for 30s. |

#### State Tree: `CommanderShahinState`
```
Idle
├── ImmunityProtocol → Radius(150m) → Immunity(10s) → Idle
├── PrecisionStrike → TargetStructure → GuaranteedHit → Idle
├── InstantBunker → Position → Deploy → Idle
├── IronDome → Radius(300m) → InterceptAll(30s) → Idle
└── Command → BuffAllies → Idle
```

#### Competitive Role
- **Defensive Linchpin**: Immunity Protocol + Iron Dome shut down pushes
- **Map Control**: Instant Bunkers secure territory instantly
- **Counterplay**: Vulnerable to EMP; Iron Dome has tell; Precision Strike single-target

---

### 🇹🇷 Türkiye — Kaan

| Property | Value |
|----------|-------|
| **Role** | Anti-Air Specialist / Mobile Railgun Platform |
| **Cost** | 3000 Credits, 80 Energy, 4 Tech Points |
| **Population** | 3 |
| **Health** | 900 |
| **Speed** | 7.5 m/s |
| **Production** | TAF-Anka Command (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Railgun Volley** | 8s | Fires 3 hyper-velocity slugs. 100% hit rate on air. 250 dmg each. |
| **Drone Swarm Command** | 30s | Deploys 12 Bayraktar micro-drones. Auto-target air, 50 dmg each, 15s duration. |
| **Target Painter** | 20s | Marks ground target. All friendly artillery gains 2x range vs marked target for 15s. |
| **Sky Dominance (Ultimate)** | 100s | Calls TAF-Anka orbital strike. 50 micro-missiles rain on all enemy air units globally. 100% hit. |

#### State Tree: `KaanState`
```
Idle
├── RailgunVolley → TargetAir → 3xSlug → Idle
├── DroneSwarm → Deploy → AutoAttack(15s) → Idle
├── TargetPainter → Position → ArtilleryBuff(15s) → Idle
├── SkyDominance → GlobalAirStrike → Idle
└── Patrol → AirPatrol → Idle
```

#### Competitive Role
- **Air Superiority**: Railgun + Sky Dominance = absolute air denial
- **Artillery Force Multiplier**: Target Painter enables safe shelling
- **Counterplay**: Vulnerable to EMP; ground units can close; Sky Dominance has long cooldown

---

### 🇵🇰 Pakistan — Guerilla Shadow

| Property | Value |
|----------|-------|
| **Role** | Infiltrator / Superweapon Saboteur |
| **Cost** | 2500 Credits, 50 Energy, 3 Tech Points |
| **Population** | 2 |
| **Health** | 350 (very low) |
| **Speed** | 9.0 m/s (fastest) |
| **Production** | Hidden Training Camp (Tier 2) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Phase Walk** | 10s | Enters alternate dimension. Invisible, moves through units/terrain, immune to damage. 4s duration. |
| **Minefield Manipulation** | 15s | Reveals all enemy mines in 200m. Can flip ownership or detonate remotely. |
| **Timer Rewind** | 60s | Targets enemy superweapon → adds 60s to its cooldown. |
| **Ghost Protocol (Ultimate)** | 150s | 30s: Permanent stealth, 2x speed, immune to detection. Can capture neutral structures instantly. |

#### State Tree: `GuerillaShadowState`
```
Idle (Stealthed when stationary)
├── PhaseWalk → Phase(4s) → Invulnerable → Idle
├── MinefieldManipulation → Reveal/Flip/Detonate → Idle
├── TimerRewind → TargetSuperweapon → +60sCooldown → Idle
├── GhostProtocol → Stealth(30s) + Speed2x + ImmuneDetection → Idle
└── Infiltrate → CaptureNeutral → Instant → Idle
```

#### Competitive Role
- **Superweapon Counter**: Timer Rewind delays enemy win conditions
- **Map Control**: Phase Walk + Ghost Protocol = uncatchable scout/saboteur
- **Counterplay**: Extremely fragile; true sight (rare) counters; Phase Walk has audio tell

---

### 🇮🇳 India — Akash

| Property | Value |
|----------|-------|
| **Role** | Cyber Infantry / Minimap Hacker |
| **Cost** | 2800 Credits, 120 Energy, 4 Tech Points |
| **Population** | 2 |
| **Health** | 500 + 300 Plasma Shield (regens 30/s) |
| **Speed** | 6.5 m/s |
| **Production** | SDRF Tech Center (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Minimap Hack** | 25s | Enemy minimap shows false data for 15s: units mirrored, structures hidden, fake armies. |
| **Railgun Precision** | 12s | Railgun shot penetrates 3 units in line. 300 dmg, ignores armor. |
| **Ally Disguise** | 30s | Target enemy unit appears as friendly on minimap for 20s. Friendly fire enabled. |
| **Orbital Array (Ultimate)** | 110s | Calls 3 Agni-V plasma strikes at chosen locations. 400m radius each, armor-melting. |

#### State Tree: `AkashState`
```
Idle
├── MinimapHack → TargetEnemy → FalseData(15s) → Idle
├── RailgunPrecision → TargetLine → Penetrate(3) → Idle
├── AllyDisguise → TargetEnemy → Disguise(20s) → Idle
├── OrbitalArray → 3xPosition → PlasmaStrikes → Idle
└── Hack → PassiveIntel → Idle
```

#### Competitive Role
- **Information Warfare**: Minimap Hack creates chaos, enables ambushes
- **Precision Damage**: Railgun punishes clumping
- **Counterplay**: Plasma Shield burstable; Minimap Hack has tell (screen glitch); predictable strike patterns

---

### 🇯🇵 Japan — Shinobi 2.0

| Property | Value |
|----------|-------|
| **Role** | Quantum Assassin / Multi-Target Executioner |
| **Cost** | 3300 Credits, 100 Energy, 4 Tech Points |
| **Population** | 3 |
| **Health** | 600 + 400 Quantum Shield (regen 40/s after 6s no dmg) |
| **Speed** | 10.0 m/s (with jumps) |
| **Production** | Quantum Core (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Quantum Leap** | 6s | Teleports to target location (max 50m). Can chain 3x in 2s. |
| **Phase Blade** | 10s | Next 3 attacks ignore armor, teleport behind target. 200 true dmg each. |
| **Shadow Clone** | 25s | Creates 2 clones (30% HP, no abilities). Last 10s. Explode on death (100 AoE). |
| **Omnislash (Ultimate)** | 130s | Time stop (1.5s). Teleports to 5 targets in sequence, executes each (400 true dmg). Untargetable during. |

#### State Tree: `Shinobi2State`
```
Idle
├── QuantumLeap → Position → (Chainable 3x) → Idle
├── PhaseBlade → 3x(Teleport+Strike) → Idle
├── ShadowClone → 2xClone(10s) → Idle
├── Omnislash → TimeStop(1.5s) → 5xExecute → Idle
└── Retreat → QuantumLeap x3 → Idle
```

#### Competitive Role
- **Assassination**: Omnislash removes 5 key targets instantly
- **Mobility**: Quantum Leap crosses map instantly
- **Counterplay**: Quantum Shield burstable; Omnislash predictable after first; clones distinguishable (no abilities)

---

### 🇰🇷 South Korea — Viper

| Property | Value |
|----------|-------|
| **Role** | Elite Pilot / Instant Air Superiority |
| **Cost** | 3000 Credits, 80 Energy, 4 Tech Points |
| **Population** | 3 |
| **Health** | 700 (in aircraft: 1500) |
| **Speed** | 15.0 m/s (air), 6.0 m/s (ground) |
| **Production** | K-Sat Relay Center (Tier 3) |
| **Max per Player** | 1 |

#### Abilities
| Ability | Cooldown | Description |
|---------|----------|-------------|
| **Instant Strike** | 15s | Calls airstrike on target location. Zero delay. 300 dmg, 50m radius. |
| **Superweapon Lock** | 40s | Targets enemy superweapon → blocks firing for 30s. |
| **Agent Neutralize** | 60s | Targets enemy super-agent → disables abilities for 20s, reveals position. |
| **Cyber Override (Ultimate)** | 120s | Hacks enemy superweapon → fires on enemy's own base. 50% chance success. |

#### State Tree: `ViperState`
```
Idle (Ground)
├── EnterAircraft → Transform → AirMode → Idle
├── InstantStrike → Target → ZeroDelay → Idle
├── SuperweaponLock → TargetSW → Block(30s) → Idle
├── AgentNeutralize → TargetHero → Disable(20s) + Reveal → Idle
├── CyberOverride → TargetSW → Hack(5s) → 50%FireOnSelf → Idle
└── ExitAircraft → Transform → GroundMode → Idle
```

#### Competitive Role
- **Reactive Control**: Instant Strike punishes aggression instantly
- **Strategic Denial**: Superweapon Lock + Agent Neutralize controls enemy tempo
- **High-Risk/High-Reward**: Cyber Override can win/lose games
- **Counterplay**: Vulnerable on ground; aircraft targetable by AA; hack interruptible

---

## Super-Agent Restriction Enforcement

```fsharp
// Source/HeroHourSimulation/src/SuperAgents/SuperAgentManager.fs

module SuperAgentManager =

    let validateSuperAgentSpawn (state: GameState) (playerId: PlayerId) (heroType: HeroType) : Result<Unit, string> =
        // Check max 1 per player
        match state.Players.[playerId].SuperAgentInstance with
        | Some existing ->
            Error $"Player already has super-agent: {existing.HeroType}"
        | None ->
            // Check tech requirements
            let hasTech = state.Players.[playerId].ResearchedTechs
                            |> Set.contains (heroType.TechRequirement)
            if not hasTech then
                Error $"Missing tech requirement: {heroType.TechRequirement}"
            else
                // Check resources
                let canAfford = EconomyManager.canAfford state playerId heroType.Cost
                if not canAfford then Error "Insufficient resources"
                else Ok ()

    let spawnSuperAgent (state: GameState) (playerId: PlayerId) (heroType: HeroType) (position: Position) : Result<GameState, string> =
        result {
            do! validateSuperAgentSpawn state playerId heroType
            let newState = EconomyManager.spendResources state playerId heroType.Cost
            let (newState', unitId) = UnitManager.createUnit newState' playerId heroType.UnitType position
            let heroInstance = {
                HeroType = heroType
                UnitId = unitId
                DeployedAt = newState'.GameTime
            }
            let finalState = { newState' with Players = newState'.Players.SetItem(playerId, { newState'.Players.[playerId] with SuperAgentInstance = Some heroInstance }) }
            return finalState
        }
```

---

## Balance & Competitive Checklist

- [ ] All super-agents have unique, readable silhouettes
- [ ] All ultimates have ≥100s cooldown
- [ ] All have clear audio/visual tells
- [ ] Max 1 per player enforced in F# state
- [ ] Counterplay exists for every ability
- [ ] No super-agent can 1v1 a maxed army alone
- [ ] Super-agent death = 3 min respawn timer (scales with game time)
- [ ] Veterancy: +10% stats per level, max 3 levels

---

*Related: `docs/gdd/SUPERWEAPONS.md`, `docs/technical/HeroTypes.md`*