# HeroHour — In-Game Map Editor Design Document

> **Source**: *HERO_HOUR_Multiplayer_Balancing_Specification.docx* — Section 3

---

## Vision

> **The Map Editor IS the game.** No external tools. No SDK download. Open HeroHour → Click "Create Map" → Edit → Test → Publish. All within the UE5.8 client.

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────────────────┐
│                      UE5.8 Client                               │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  Map Editor Mode (Editor Utility Widgets + Slate)         │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ Terrain Sculpting Tools                              │  │ │
│  │  │ - Heightmap Brushes (Raise/Lower/Smooth/Flatten)    │  │ │
│  │  │ - Erosion Simulation (Hydraulic/Thermal)            │  │ │
│  │  │ - Biome Blending (Paint layers)                     │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ Object Placement System                              │  │ │
│  │  │ - Resource Nodes (Oil, Tech, Civilian)              │  │ │
│  │  │ - Civilian Infrastructure (Buildings, Roads)        │  │ │
│  │  │ - Neutral High-Tech Structures (Refinery/Hospital)  │  │ │
│  │  │ - Player Start Positions (1-8)                      │  │ │
│  │  │ - AoD Wave Nodes (Spawn points, routes, triggers)   │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ Trigger/Scripting System                             │  │ │
│  │  │ - Visual Node Graph (Blueprint-style)               │  │ │
│  │  │ - Events: Wave Start, Capture, Timer, Unit Death    │  │ │
│  │  │ - Actions: Spawn, Modify, Message, Victory          │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  │  ┌─────────────────────────────────────────────────────┐  │ │
│  │  │ AoD Wave Designer                                    │  │ │
│  │  │ - Wave Composition (Unit types, counts, scaling)    │  │ │
│  │  │ - Difficulty Curves (Linear/Exponential/Adaptive)   │  │ │
│  │  │ - Attack Route Editor (Flow-field visualization)    │  │ │
│  │  │ - Boss Wave Designer                                │  │ │
│  │  └─────────────────────────────────────────────────────┘  │ │
│  └───────────────────────────────────────────────────────────┘ │
│                            ↓                                    │
│  ┌───────────────────────────────────────────────────────────┐ │
│  │  F# Simulation Integration (UFSharpBridge)                │ │
│  │  - "Test Wave" → Runs F# wave simulation headless         │ │
│  │  - "Validate Map" → Checks playability, balance           │ │
│  │  - "Publish" → Serializes to Data Asset + Workshop        │ │
│  └───────────────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────────────────┘
```

---

## Editor Modes

### 1. Terrain Mode
| Tool | Function |
|------|----------|
| **Raise/Lower** | Brush-based heightmap modification (size 5-200m, strength 0.1-1.0) |
| **Smooth** | Laplacian smoothing (iterations 1-10) |
| **Flatten** | Sets height to average/selected value |
| **Ramp** | Creates linear gradient between two points |
| **Erosion** | Hydraulic (water flow) / Thermal (talus) simulation (GPU compute) |
| **Biome Paint** | Layer-based: Grass/Sand/Snow/Rock/Urban (8 layers max) |
| **Texture Blend** | Automatic triplanar + slope-based blending |

**Technical**: Uses `ULandscapeSubsystem` + `FLandscapeEditDataInterface` for runtime editing. Heightmap stored as 16-bit PNG (lossless, diffable).

### 2. Object Placement Mode
| Category | Objects | Placement Rules |
|----------|---------|-----------------|
| **Resources** | Oil Derrick, Tech Cache, Civilian Supply Drop | Snap to valid terrain (slope < 15°) |
| **Civilian** | Houses, Skyscrapers, Malls, Hospitals, Refineries | Snap to roads/flat areas; max 50 per map |
| **Neutral Techs** | Oil Refinery, Major Hospital, Intel Outpost | Max 1 each per map; strategic placement |
| **Player Starts** | 1-8 positions | Enforced symmetry for 1v1/2v2; min 500m apart |
| **AoD Nodes** | Spawn Points, Waypoints, Trigger Zones | Connected via route graph |

### 3. Trigger System (Visual Node Graph)
**Nodes:**
| Event Nodes | Action Nodes |
|-------------|--------------|
| `OnWaveStart(waveId)` | `SpawnUnits(type, count, location)` |
| `OnCapture(player, structure)` | `ModifyResources(player, amount)` |
| `OnTimer(seconds)` | `SendMessage(all, message)` |
| `OnUnitDeath(unit, killer)` | `GrantVeterancy(unit, amount)` |
| `OnStructureDestroyed(structure)` | `SetVictory(player)` |
| `OnPlayerJoin(player)` | `ModifyWaveScaling(factor)` |

**UI**: Blueprint-style drag-drop. Auto-layout. Comment boxes. Grouping. Search.

### 4. AoD Wave Designer
| Feature | Description |
|---------|-------------|
| **Wave List** | Ordered waves 1-N with foldable details |
| **Wave Editor** | Unit types (checkbox grid), base count, scaling, interval, formation |
| **Scaling Curve** | Linear / Exponential / Logistic / Custom (Bezier) |
| **Route Editor** | Flow-field visualization; drag waypoints; auto-calculate alternate paths |
| **Boss Waves** | Special flag; unique unit types; cinematic trigger |
| **Test Simulation** | "Run Wave" → Headless F# sim → Returns: unit survival %, path efficiency, player DPS required |

---

## Map Data Structure (F# Serialization)

```fsharp
// Source/HeroHourSimulation/src/MapEditor/MapData.fs

type MapMetadata = {
    MapId: Guid
    Name: string
    Author: PlayerId
    Version: int
    CreatedAt: DateTime
    UpdatedAt: DateTime
    Tags: string list // ["Competitive", "AoD", "2v2", "Desert"]
    Description: string
    ThumbnailHash: string
}

type MapTerrain = {
    Heightmap: HeightmapData // 16-bit PNG, 1024x1024 max
    BiomeLayers: BiomeLayerData array // 8 layers max
    TextureWeights: TextureWeightData
    WaterLevel: float
}

type MapObjects = {
    ResourceNodes: ResourceNodeData list
    CivilianBuildings: CivilianBuildingData list
    NeutralStructures: NeutralStructureData list
    PlayerStarts: PlayerStartData list // 1-8
    AoDNodes: AoDNodeData list
}

type AoDNodeData = {
    NodeId: int
    NodeType: AoDNodeType
    Position: Position
    ConnectedNodes: int list
    Properties: Map<string, string>
}

type AoDNodeType =
    | SpawnPoint of SpawnConfig
    | Waypoint of WaypointConfig
    | TriggerZone of TriggerConfig
    | Objective of ObjectiveConfig

type MapTriggers = {
    TriggerGraph: TriggerGraphData // Serialized node graph
}

type MapData = {
    Metadata: MapMetadata
    Terrain: MapTerrain
    Objects: MapObjects
    Triggers: MapTriggers
    AoDConfig: AoDConfig option
}
```

---

## Publishing Pipeline

```
[Edit in Editor] 
    → [Validate Map] → [Test Play] → [Set Metadata] 
        → [Generate Thumbnail] → [Package] 
            → [Workshop Upload] 
                → [Moderation Queue] → [Live]
```

### Validation Checks
| Check | Criteria |
|-------|----------|
| **Playability** | All player starts reachable; resources accessible |
| **Symmetry** | 1v1/2v2 maps: resource/start symmetry within 5% |
| **AoD Balance** | Wave 1 survivable; final wave winnable; scaling smooth |
| **Performance** | < 10k draw calls; < 2GB texture memory |
| **NavMesh** | All player starts connected; no isolated islands |

### Workshop Integration
- **File Format**: `.hhmap` (custom package: heightmap PNG + JSON metadata + trigger graph)
- **Size Limit**: 50 MB compressed
- **Versioning**: Semantic versioning (Map v1.2.3)
- **Dependencies**: Auto-detects required mods/Data Assets
- **Ratings**: ELO-based matchmaking for maps

---

## Multiplayer Co-Editing (Real-time Collaboration)

### Architecture
- **Host** owns map state; **Clients** send operational transforms (OT)
- **Operational Transforms**: Terrain brush strokes, object moves, node graph edits
- **Conflict Resolution**: Last-write-wins per property; terrain uses heightmap differential merge
- **Presence**: Live cursors, selection highlights, voice chat integration

### Sync Protocol
```
Client A: BrushStroke(Position, Radius, Strength) 
    → Server: ApplyOT(ClientA, BrushStroke)
    → Server: BroadcastOT(AllExceptA, BrushStroke)
    → Client B: ReceiveOT → ApplyLocal → Ack
```

---

## Map Data Assets (UE5)

| Asset Type | Path | Description |
|------------|------|-------------|
| `UHeroHourMapData` | `/Game/Maps/Data/` | Main map data asset (references heightmap PNG + JSON) |
| `UMapHeightmap` | `/Game/Maps/Heightmaps/` | 16-bit PNG texture (streamable) |
| `UMapTriggerGraph` | `/Game/Maps/Triggers/` | Blueprint-serialized trigger graph |
| `UMapThumbnail` | `/Game/Maps/Thumbnails/` | 512x512 render target capture |

---

## Competitive Map Pool Curation

| Season | Map Pool Size | Rotation | Criteria |
|--------|---------------|----------|----------|
| **Pre-Season** | 12 maps | Weekly | Community votes + dev picks |
| **Season 1** | 8 maps | Bi-weekly | Pro feedback + win-rate balance |
| **Tournament** | 5 maps | Fixed | Zero mirror matches; diverse biomes |

---

*Related: `docs/gdd/AOD_INFRASTRUCTURE.md`, `docs/technical/MapEditorTypes.md`*