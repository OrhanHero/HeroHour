# HeroHour — Core Gameplay Features

> Source: *HERO_HOUR_Ultimate_Core_Specification_v4.docx* — Sections 4 & 5

---

## 4.1 Combat School (Tactical Post-Mission Analysis)

The **Combat School** is a deep analysis tool embedded in the F# Simulation Layer. During every match, the engine logs **every player action**:
- Click coordinates & timestamps
- Hotkey usage
- Build order timings
- Resource efficiency ratios
- Unit control groups
- Camera positions

### Post-Match Analysis Features

| Feature | Description |
|---------|-------------|
| **Loss Heatmap** | Geographic visualization of where most units were lost — color-coded by time |
| **APM Curve** | Actions-per-Minute over game time with percentile comparison |
| **Build Order Efficiency** | Timeline comparing actual vs optimal build order; highlights delays (e.g., "2nd Depot 45s late → 20% tank delay") |
| **Resource Efficiency** | Income vs spending ratio; highlights float/waste periods |
| **AI-Generated Recommendations** | Natural-language suggestions: *"Your 2nd War Factory was 45s late, delaying tank production by 20%"* |

### Technical Implementation
- **F# Simulation Layer**: Immutable event log per tick → serialized to replay
- **UE5 UI**: Interactive Recharts/D3 visualizations in UMG
- **Privacy**: Local-only by default; opt-in for cloud analysis

---

## 4.2 Generals Challenge Mode (Ultimate Singleplayer Campaign)

The centerpiece of singleplayer. Player chooses **one of 10 factions** and defeats **9 specialized AI Generals** sequentially.

### Structure
| Phase | Opponent | Specialization |
|-------|----------|----------------|
| 1-3 | Faction Specialists | Extreme doctrine variants (e.g., US Laser General, China Nuke General) |
| 4-6 | Cross-Faction Counters | Generals designed to counter player's faction |
| 7-9 | Boss Generals | Hybrid doctrines with unique mechanics |
| Final | **Supreme Commander** | Adaptive meta-AI using all faction tech |

### AI Architecture
- **UE5 State Trees** drive high-level strategic decisions
- **Behavior Trees** handle tactical micro
- **EQS** for dynamic target selection
- **Meta-Learning**: Each General analyzes player counters and adapts attack routes

### Rewards
- **Unlock**: Defeated General becomes playable in Multiplayer (cosmetic commander skin)
- **Visual Medals**: Unique profile decorations for online lobby
- **Combat School Data**: Each challenge generates detailed analysis

---

## 4.3 Dynamic Weather Simulation (Direct Gameplay Impact)

Weather is not cosmetic — it **modifies unit stats and abilities in real-time** via UE5's physical particle simulation + F# simulation modifiers.

| Weather | Mechanics | Affected Factions |
|---------|-----------|-------------------|
| **Heavy Rain** | -30% damage & range for **laser weapons** (light scattering in droplets) | 🇺🇸 USA, 🇯🇵 Japan |
| **Massive Sandstorm** | -70% vision range for all units/towers; **radar completely disabled** | Favors: **HERO**, 🇵🇰 Pakistan |
| **Heavy Snowfall** | Ground freezes; **tracked vehicles -25% speed/turn rate**; mechas unaffected | Favors: 🇯🇵 Japan |

### Implementation
- **UE5**: Niagara particle systems + volumetric fog + physical material changes
- **F#**: WeatherState enum propagated each tick; applies stat modifiers to unit templates
- **Network**: Deterministic seed sync; all clients simulate identical weather progression

---

## 5. Neutral Elements & Urban Warfare

Maps are **living tactical environments**, not empty arenas.

### 5.1 Civilian Infrastructure & Capture Mechanics

| Element | Description | Faction Interaction |
|---------|-------------|---------------------|
| **Civilian Vehicles** (Cars, Buses, Tractors) | Parked in urban zones | **HERO**: Garrison infantry → instant improvised VBIED or mobile militia gun platform |
| **Skyscrapers / Malls** | Up to 20 infantry garrison | Becomes fortified strongpoint at chokepoints; destructible floors create vertical combat |

### 5.2 Capturable High-Tech Structures (Engineer Capture)

| Structure | Global Effect (Capturing Player) |
|-----------|----------------------------------|
| **Civilian Oil Refinery** | Continuous, reliable **Credit income** stream in F# economy |
| **Civilian Major Hospital** | **Passive healing aura** map-wide: all injured infantry auto-heal out of combat |
| **Intelligence Outpost** | **Every 90s**: Reveals exact build & production queue of all enemies for brief moment |

---

## Integration Points

| System | F# Simulation | UE5 Presentation |
|--------|---------------|------------------|
| Combat School | Event log, stat aggregation | UMG charts, heatmap renderer |
| Generals Challenge | AI personality weights, unlock flags | State Tree assets, cinematic intros |
| Weather | WeatherState tick modifier | Niagara, volumetric fog, material params |
| Urban Capture | Structure ownership, aura effects | Garrison visualizers, destruction states |

---

## Development Priorities (Sprint Mapping)

| Sprint | Feature | Dependencies |
|--------|---------|--------------|
| 1-2 | Weather System Core | F# WeatherState, UE5 Niagara |
| 3-4 | Urban Capture Mechanics | Engineer logic, garrison system |
| 5-6 | Combat School Logging | F# EventLog, UE5 UMG charts |
| 7-9 | Generals Challenge AI | State Trees, EQS, meta-learning |
| 10 | Cross-System Integration | All above |

---

*See `docs/technical/WEATHER_SYSTEM.md` and `docs/technical/URBAN_WARFARE.md` for implementation specs.*