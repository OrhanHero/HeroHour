module HeroHour.Simulation.Core.Constants

/// Game time constants
let TICK_RATE = 20.0 // Ticks per second
let TICK_INTERVAL = 1.0 / TICK_RATE
let MAX_DELTA_TIME = 0.1 // Clamp delta time

/// Population limits
let DEFAULT_POPULATION_CAP = 200
let MAX_POPULATION_CAP = 1000

/// Resource defaults
let STARTING_CREDITS = 5000
let STARTING_ENERGY = 100
let STARTING_TECH_POINTS = 0

/// Resource tick rates
let CREDIT_TICK_RATE = 0.5 // Seconds
let ENERGY_TICK_RATE = 1.0
let TECH_TICK_RATE = 2.0

/// Combat constants
let BASE_DAMAGE_VARIANCE = 0.15 // ±15%
let CRITICAL_CHANCE_BASE = 0.05
let CRITICAL_MULTIPLIER = 2.0

/// Movement
let DEFAULT_MOVE_SPEED = 5.0 // Meters per second
let FORMATION_TOLERANCE = 2.0

/// AI
let AI_DECISION_INTERVAL = 1.0 // Seconds
let AI_DIFFICULTY_SCALING = 0.1 // Per difficulty level

/// Fog of War
let FOG_REVEAL_RANGE = 10.0
let FOG_RECON_RANGE = 20.0
let FOG_UPDATE_INTERVAL = 0.5

/// Network
let NETWORK_TICK_RATE = 30 // Server ticks per second
let MAX_PACKET_SIZE = 1400 // Bytes

/// Save/Load
let SAVE_VERSION = 1
let AUTOSAVE_INTERVAL = 300.0 // 5 minutes