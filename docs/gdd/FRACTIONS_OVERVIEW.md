# HeroHour — 10 Asymmetric World Powers Overview

> Source: *HERO_HOUR_Ultimate_Core_Specification_v4.docx* & *HERO_HOUR_Multiplayer_Balancing_Specification.docx*

---

## Design Philosophy: Absolute Asymmetry

> **No faction shares base units or economic structures with another.** Every army has:
> - Unique resource efficiencies
> - Distinct movement properties
> - Faction-specific tactical disadvantages
> - Independent victory conditions
> - **Exactly ONE superweapon** (cooldown-based, global warning)
> - **Exactly ONE super-agent** (hero unit, max 1 per player, F# State Tree)

---

### Faction Concept Art

![HeroHour Faction Concepts](../../Art/Concepts/Factions/Factions_Core_ConceptArt.png)

*All 10 factions with their Superweapons & Super-Agents visualized*

---

## Faction Comparison Matrix

| Faction | Core Doctrine | Economy Style | Signature Mechanic | Superweapon | Super-Agent | Difficulty |
|---------|---------------|---------------|-------------------|-------------|-------------|------------|
| 🇺🇸 **USA** | Tech/Air Superiority | High-cost, high-efficiency | Precision strikes, stealth | Chrono-Laser-Phalanx | Specter Prime | High |
| 🇨🇳 **China** | Mass/Artillery | Low-cost, high-volume | Swarm bonus (group buffs) | Nuke-Silicon-Kollaps | Lotus X | Medium |
| 🏴‍☠️ **HERO** | Hybrid/Tech Theft | No energy, scrap recycling | Vehicle hijacking, tech unlocking | Singularitäts-Schrottkanone | Nexus Core | High |
| 🇹🇷 **Türkiye** | Drone/Mobile Fortress | Balanced, drone-heavy | Fortress Mode, APS | TAF-Anka Global Strike | Kaan | Medium |
| 🇮🇷 **Iran** | Ballistic/Fortress | Slow, high-HP structures | Invulnerable silos | Sejjil-III Giga-Silo | Commander Shahin | Medium |
| 🇵🇰 **Pakistan** | Asymmetric/EMP | Stealth, ambush | Fog of War regeneration | Giga-EMP-Kaskade | Guerilla Shadow | High |
| 🇮🇳 **India** | Railgun/Tech | Efficient research | Railgun penetration | Agni-V Plasmabrenner | Akash | Medium |
| 🇯🇵 **Japan** | Mecha/Quantum | High-power, low-footprint | Jump mechanics, plasma shields | Mecha-Giga-Wave | Shinobi 2.0 | High |
| 🇰🇷 **South Korea** | Cyber/Stealth Air | Intel-focused | 100% hit prediction, Cyber Blackout | K-Sat Cyber-Strahl | Viper | High |
| 🇷🇺 **Russia** | Heavy Armor/Thermobaric | Dual-production, high HP | Thermobaric AoE, Tesla chains | Tesla-Giga-Spule | Omega Boris | Medium |

---

## 🇺🇸 United States — Technological Dominance & Air Superiority

**Playstyle**: Precision micro, air control, stealth operations. Expensive units, highest survivability.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **Specter Gunship II** | Heavy Air Support | Orbits sector permanently; auto-attacks with 105mm howitzer + 25mm gatling until shot down |
| **Microwave Stryker** | Anti-Structure/Infantry | High-frequency microwave field — disables building power, cooks infantry in cover |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **Satellite Uplink Command** | Global scan (20s fog reveal); permanent stealth detection radius |

### Core Upgrades
- **Hellfire Missiles** — Laser-guided Hellfires on drones/jets: armor penetration, bonus vs heavy armor
- **Advanced Chemical Suits** — 100% immunity to bio-toxins & radioactive fallout

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Chrono-Laser-Phalanx** | Orbital satellite death ray; draws a line across the map, pulverizes all structures in path. Zero travel time, global warning 10s before impact. Cooldown: 8 minutes. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Specter Prime** (Colonel Burton Evolution) | Stealth spectral gunship; phases through terrain via cliff teleportation, marks buildings with laser designator for drone strikes, executes surgical strikes. Max 1 per player. F# State Tree: `SpecterPrimeState` with `PhaseShift`, `DesignateTarget`, `Strike` behaviors. |

---

## 🇨🇳 China — Iron Mass & Cyber Warfare

**Playstyle**: Overwhelming numbers, area denial, cyber economy. Swarm Bonus: attack/speed buffs in groups.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **Hacker Elite** | Cyber/Infiltration | Stealth while stationary; hacks enemy vehicles (disable) or supply lines (credit siphon) |
| **Overlord Flame-Tank** | Heavy Assault | Dual napalm projectors; burns defensive lines in seconds, leaves burning ground |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **China Internet Center** | Garrison up to 8 Hackers for global cyber-mining (massive passive income) |

### Core Upgrade
- **Napalm Uplink** — All tank shells & artillery gain napalm component: burning inferno areas on impact

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Nuke-Silicon-Kollaps** | Thermonuclear ICBM; creates massive irradiated zone, vaporizes center. Cooldown: 9 minutes. Global warning 15s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Lotus X** (Black Lotus Evolution) | Cyber commando; infiltrates enemy HQ from safe distance, infects production — reverses entire enemy production for 45s. Max 1 per player. F# State Tree: `LotusXState` with `Infiltrate`, `InfectProduction`, `Extract` behaviors. |

---

## 🏴‍☠️ HERO — Die Ultimative Hybrid-Armee (Technologieraub & Schrott-Recycling)

Die HERO-Fraktion ist ein hochgradig experimentelles Bündnis, das vollständig auf eine klassische Energieversorgung verzichtet. Ihre gesamte Struktur basiert auf Mobilität, Guerilla-Taktiken, Täuschung und dem aktiven Diebstahl gegnerischer Technologien direkt auf dem Schlachtfeld.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **HERO Combat Cycle** | Extremely Agile Motorcycle | Can be occupied by **any infantry unit** in the game. Place a US Laser Trooper → High-Speed Laser Vehicle; place a Chinese Hacker → Mobile Cyber Disruption Platform. |
| **Prototype Hijacker** | Permanently Stealthed Special Unit | Hijacks enemy vehicles. Special: Capturing a special vehicle (e.g., Indian Railgun Tank) analyzes the tech and **permanently unlocks it in HERO factories**. |
| **Chimeran Overlord** | Heavy Hybrid Tank | Uses captured Russian tank chassis, combined with modified US Microwave Cannon in turret. |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **Black Market Hub** | Generates permanent illegal credits in F# economy; mirrors tech trees of all enemies present, granting HERO access to their upgrades. |

### Core Upgrade
- **Hellfire Anthrax** — Drenches all bio-toxin weapons with US Hellfire accelerants. Creates a toxic, burning cloud that melts infantry AND melts vehicle armor.

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Singularitäts-Schrottkanone** | Fires compressed scrap singularity; pulls all units/structures in 500m radius, crushes them, ejects as explosion, converts mass to credits. Cooldown: 7 minutes. Global warning 8s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Nexus Core** | Shapeshifter cyborg; absorbs killed enemy super-agents, permanently copies their abilities. Max 1 per player. F# State Tree: `NexusCoreState` with `Absorb`, `Morph`, `ReplicateAbility` behaviors. Unique: learns from every match. |

---

## 🇹🇷 Türkiye — Autonomous Drone Dominance & Mobile Fortresses

**Playstyle**: Modern networked unmanned warfare + highly mobile mechanized divisions.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **Bayraktar Swarm Drone** | Swarm Air Support | Remote-operated; swarm AI, precision missiles, marks targets for artillery (2x range) |
| **Altay-M MBT** | Main Battle Tank | **Fortress Mode**: digs in, +80% armor, switches to heavy kinetic cannon with massive range |

### Core Upgrade
- **Active Protection System (APS)** — All heavy vehicles intercept first incoming explosive projectile (15s cooldown)

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **TAF-Anka Global Strike** | Massive coordinated long-range missile barrage; prioritizes AA towers. 500 micro-missiles from orbit. Cooldown: 8 minutes. Global warning 10s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Kaan** | Exo-warrior with mobile railgun; shoots down enemy aircraft at 100% accuracy in passing. Max 1 per player. F# State Tree: `KaanState` with `RailgunLock`, `Intercept`, `Overwatch` behaviors. |

---

## 🇮🇷 Iran — Ballistic Armor & Fortress Doctrine

**Playstyle**: Territory lockdown, long-range ballistic barrages for psychological/physical attrition.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **Fateh Mobile Launcher** | Strategic Artillery | Massive tracked vehicle; manual anchoring; fires heavy SRBMs devastating base areas |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **Underground Silo** | Deep-earth missile bunker; **immune to ALL conventional attacks (including superweapons)** while closed; opens only 3s during launch |

### Core Upgrade
- **Bunker Reinforcement** — +50% structure HP on all defenses; garrisoned infantry fire ATGMs with increased velocity

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Sejjil-III Giga-Silo** | Three coupled ballistic missiles; time-staggered impacts to break shields. First cracks shields, second penetrates, third delivers payload. Cooldown: 9 minutes. Global warning 12s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Commander Shahin** | Tactical commander; grants temporary immunity shield to all own buildings in radius. Max 1 per player. F# State Tree: `ShahinState` with `DeployShield`, `CallStrike`, `Rally` behaviors. |

---

## 🇵🇰 Pakistan — Asymmetric Counterstrike & EMP Ambushes

**Playstyle**: Terrain geometry mastery, concealed ambushes, EMP neutralization, lightning needle strikes.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **EMP Commando** | Elite Sabotage | Invisible in Fog of War; places EMP charges on buildings or throws EMP grenades — disables entire tank columns for 12s |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **Hidden Training Camp** | Barracks with permanent radar-jamming field; units invisible on enemy minimap until first shot |

### Core Upgrade
- **Guerrilla Regeneration** — Units auto-regenerate HP rapidly while in Fog of War outside enemy LOS

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Giga-EMP-Kaskade** | Shuts down all electronics, power plants, vehicles in target radius for 45s. Global EMP cascade. Cooldown: 8 minutes. Global warning 10s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Guerilla Shadow** | Phasing infiltrator; manipulates minefields invisibly, rolls back enemy superweapon timers by 60s. Max 1 per player. F# State Tree: `GuerillaShadowState` with `PhaseShift`, `MineManipulate`, `TimerRollback` behaviors. |

---

## 🇮🇳 India — Technological Mass & Railgun Prototypes

**Playstyle**: Gigantic advanced arms industry + breakthrough lab/software research. Mechanized excellence + kinetic high-energy weapons.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **Arjun Mk-III** | Tech Showcase Tank | **Experimental Railgun**: projectile penetrates multiple vehicles in line, ignores physical armor |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **SDRF Tech Center** | **-25% cost & research time for ALL global upgrades**; detects enemy cyber attacks instantly |

### Core Upgrade
- **Composite Reactive Armor** — Intelligent armor reducing explosive damage (artillery/rockets) by flat 40%

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Agni-V Plasmabrenner** | Railgun-fired plasma slug from orbit; puts target zone under artificial plasma inferno, melts armor. Cooldown: 8 minutes. Global warning 10s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Akash** | Cyber infantry; hacks enemy minimaps, disguises enemy units as 'friendly'. Max 1 per player. F# State Tree: `AkashState` with `HackMinimap`, `DisguiseUnit`, `CyberStrike` behaviors. |

---

## 🇯🇵 Japan — Precise Mecha Robotics & Quantum Networks

**Playstyle**: Almost no tracked vehicles — bipedal mechas, sci-fi energy weapons, autonomous shields. Extreme mobility.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **Tetsujin Walker** | Agile Mech | Hydraulically optimized joints — **jumps cliffs, rivers, rough terrain** for devastating flanking |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **Quantum Core** | **5x output of standard power plant** on minimal footprint; explodes in massive plasma shockwave when destroyed |

### Core Upgrade
- **Plasma Shield Harmonizer** — All mechs gain blue plasma hull; absorbs fixed damage, fully regenerates after 6s without damage

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Mecha-Giga-Wave** | Gigantic kinetic pulse from Quantum Cores; overloads all shields in hemisphere, generates shockwaves. Cooldown: 8 minutes. Global warning 10s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Shinobi 2.0** | Futuristic ninja; blades cut through armor, teleport ambushes. Max 1 per player. F# State Tree: `Shinobi2State` with `Teleport`, `PhaseBlade`, `MultiTarget` behaviors. |

---

## 🇰🇷 South Korea — K-Cyber Network & Subsonic Stealth Air Force

**Playstyle**: Information warfare mastery + surgical air strikes with extreme-speed jets. Enemy fights blind; SK Commander sees everything.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **KF-X Black Shark** | Stealth Bomber | **Radar-invisible** until bomb release; then visible briefly, accelerates to Mach 2 to escape AA |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **K-Sat Relay Center** | Enables **Cyber Blackout** — blinds enemy radar & minimap completely for 30s |

### Core Upgrade
- **Smart-Predictive Munition** — All projectiles use pre-calculation algorithm — **guaranteed 100% hit rate on moving targets** (especially fast aircraft/bikes)

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **K-Sat Cyber-Strahl** | Targeted cyber pulse from satellite; blocks enemy superweapon timers, reveals all stealth. Cooldown: 7 minutes. Global warning 8s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Viper** | Elite pilot; calls airstrikes with zero delay directly on battlefield. Max 1 per player. F# State Tree: `ViperState` with `InstantStrike`, `Coordinate`, `SuperweaponLock` behaviors. |

---

## 🇷🇺 Russian Federation — Heavy Armor Industry & Thermobaric Devastation

**Playstyle**: Raw violence, massive steel tracks, Tesla energy, ruthless thermobaric weapons. Highest structure HP in game.

### Signature Units
| Unit | Role | Key Ability |
|------|------|-------------|
| **T-14 Armata Overlord** | Super-Heavy Tank | Dual main guns; **auto AA flak on turret** — autonomous defense vs air & ground |
| **Tesla Trooper** | Heavy Infantry | Chain lightning jumps between vehicles, slows mechanical units |

### Signature Structure
| Structure | Function |
|-----------|----------|
| **Giga-Fabrik** | **Only building producing two vehicles simultaneously in same queue** |

### Core Upgrade
- **Thermobaric Warheads** — Rocket artillery (TOS-2) gets thermobaric ammo: massive heatwave eliminates infantry instantly, burns ground 10s

### Superweapon
| Superweapon | Description |
|-------------|-------------|
| **Tesla-Giga-Spule** | Massive artificial thunderstorm; chain lightning grills all units in sector relentlessly. Cooldown: 8 minutes. Global warning 10s. |

### Super-Agent
| Super-Agent | Description |
|-------------|-------------|
| **Omega Boris** | Tesla exoskeleton titan; projects shielding aura, overloads enemy vehicles on proximity, calls heavy bomber chain. Max 1 per player. F# State Tree: `OmegaBorisState` with `TeslaAura`, `Overload`, `CallBomberChain` behaviors. |

---

## Cross-Faction Interaction Notes

### Weather Synergies
| Weather | Favors | Hinders |
|---------|--------|---------|
| **Heavy Rain** | HERO, Pakistan, Iran | USA (Laser -30%), Japan (Laser -30%) |
| **Sandstorm** | HERO, Pakistan, Türkiye | All radar/visual units |
| **Snow** | Japan (Mechas) | Russia, China (Tracked -25% speed) |

### Counterplay Triangle (Examples)
- **USA Stealth** → Countered by **SK K-Sat** (Cyber Blackout reveals all) / **China Hackers** (detect stealth)
- **Russia Heavy Armor** → Countered by **India Railgun** (ignores armor) / **USA Hellfire** (armor pen)
- **HERO Tech Theft** → Countered by **USA Chemical Suits** (100% immunity to stolen toxins) / **Iran Bunkers** (garrisoned ATGMs)
- **Pakistan EMP** → Countered by **Japan Plasma Shields** (regenerate) / **SK Smart Munitions** (hit anyway)

### Superweapon Counterplay
- **Chrono-Laser-Phalanx** → Countered by **K-Sat Cyber-Strahl** (blocks timer) / **Guerilla Shadow** (rolls back timer)
- **Nuke-Silicon-Kollaps** → Countered by **Underground Silo** (immune) / **Commander Shahin** (immunity shield)
- **Singularitäts-Schrottkanone** → Countered by **Omega Boris** (Tesla aura disrupts) / **Specter Prime** (phases out)
- **Giga-EMP-Kaskade** → Countered by **Viper** (instant strike before EMP) / **K-Sat Cyber-Strahl** (blocks timer)

---

## Implementation Status

| Faction | Data Assets | Simulation Logic | UE5 Units | VFX/SFX | Localization |
|---------|-------------|------------------|-----------|---------|--------------|
| USA | ✅ Planned | 📝 In Progress | 📝 Planned | 📝 Planned | ✅ EN/TR |
| China | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| HERO | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| Türkiye | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| Iran | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| Pakistan | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| India | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| Japan | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| South Korea | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |
| Russia | ✅ Planned | 📝 Planned | 📝 Planned | 📝 Planned | ✅ EN/TR |

---

*See individual faction design docs in `docs/gdd/factions/` for complete unit rosters, tech trees, and balance numbers.*