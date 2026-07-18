# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Initial project structure and configuration
- GitHub commit strategy documentation
- Contributing guidelines
- EditorConfig for consistent formatting
- Git LFS configuration for binary assets
- .gitignore for UE5, F#, .NET projects
- **10 Asymmetric World Powers complete design** (USA, China, **HERO**, Türkiye, Iran, Pakistan, India, Japan, South Korea, Russia)
- **Faction deep-dive documentation** with signature units, structures, and upgrades
- **Core Gameplay Features**: Combat School, Generals Challenge, Dynamic Weather, Urban Warfare
- **Turkish localization** (tr/Game.po) with 200+ strings
- **English source localization** (en/Game.po) with 300+ strings
- **Weather System technical specification** with F# stat modifiers
- **Urban Warfare & Civilian Infrastructure design docs**
- **Superweapons & Super-Agents matrix** for all 10 factions (1 superweapon + 1 super-agent per faction)
- **In-Game Map Editor design** (UE5.8 runtime editor with AoD wave designer)
- **AoD (Art of Defense) Infrastructure** — deterministic F# wave spawning system with intelligent routing
- **Faction Concept Art** — `Art/Concepts/Factions/Factions_Core_ConceptArt.png` (all 10 factions + superweapons/super-agents)
- **Documentation**: SUPERWEAPONS.md, SUPER_AGENTS.md, MAP_EDITOR.md, AOD_INFRASTRUCTURE.md

### Changed
- **Pivoted to pure multiplayer competitive architecture** — removed singleplayer campaigns, Generals Challenge, Combat School moved to post-match analytics
- Updated FRACTIONS_OVERVIEW.md with superweapon & super-agent details per faction
- Updated README.md with superweapon/super-agent matrix, concept art, map editor & AoD infrastructure

### Deprecated
- N/A

### Removed
- Singleplayer campaign references (Generals Challenge mode)
- PvE content planning

### Fixed
- N/A

### Security
- N/A

---

## [0.1.0] - 2025-07-18

### Added
- HeroHour project initialization
- Complete Game Design Document (GDD) v2.0
- Technical architecture specification
- Project directory structure
- Development environment setup

---

## Version History Template

### [MAJOR.MINOR.PATCH] - YYYY-MM-DD

#### Added
- New features

#### Changed
- Changes in existing functionality

#### Deprecated
- Soon-to-be removed features

#### Removed
- Now removed features

#### Fixed
- Bug fixes

#### Security
- Vulnerability fixes

---

## Release Tags

| Version | Date | Branch | Notes |
|---------|------|--------|-------|
| 0.1.0 | 2025-07-18 | main | Initial project setup |

---

## Upgrade Guides

### 0.x → 1.0.0 (Planned)
- Breaking changes to simulation API
- UE5.8 migration
- Network protocol v2

---

*Generated with [Keep a Changelog](https://keepachangelog.com/)*