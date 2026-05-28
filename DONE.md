# Firestaff DONE - Completed Work

This file tracks completed capabilities by game. It is not a changelog; see git history and release notes for chronology.

## Legend

- ✅ Done / verified
- 🔒 Source-locked against original references

## Dungeon Master (DM1)

### DM1 V1 - Runtime and Source-Lock

- ✅ Movement and collision: cardinal movement, turning, wall/door/fake-wall blocking, cooldowns, stairs, pits, teleporters, blocked self-damage, empty-party group cleanup, and deterministic capture gates.
- ✅ Viewport rendering: wall/floor/ceiling slices, doors, frames, ornaments, inscriptions, pits, stairs, creatures, projectiles, explosions, floor items, alcoves, occlusion, palette dimming, HiDPI scaling, and teleporter visuals.
- ✅ Viewport wall evidence hardening: `g_dm1_wall_frame_bitmaps` is source-locked to the PC34 `G2107`/door-frame offset model and guarded by an asset-free null-write regression.
- ✅ Door-front occlusion pixel-zone gate: all 11 source-locked front-door branches prove rear cells are masked by door pixels and front cells draw after the door pass.
- ✅ Creature and combat systems: creature groups, AI, attacks, deaths, drops, XP, projectile attacks, sounds, fleeing, special positioning, possession drops, Black Flame behavior, generator/teleporter/fall/drop cases, and Lord Chaos constants.
- ✅ Spells and magic: rune UI, spell casting, mana/skill checks, projectiles, shields, light/dark, open-door magic, poison cloud behavior, and spell failure paths.
- ✅ Champions: recruitment, active selection, health/stamina/mana bars, skill/XP updates, death/resurrection, stats panel routing, weight/load behavior, and stamina regeneration.
- ✅ Inventory and items: leader hand, alcoves, throwing, torches/light, floor pickup, scrolls, potions, food/water, item descriptions, chest/backpack routes, equip/unequip, fountains, and source-blocked direct key action.
- ✅ Survival, sensors, entrance, save/load, audio, and data loading: food/water decay, rest, stamina, sensor/timeline behavior, title/entrance flow, save/load routes, sound routing, and DUNGEON.DAT/GRAPHICS.DAT ingestion.
- ✅ Source-lock verifier hardening: viewport/walls landable metadata, wall-clip source audit, side-wall source-row clipping, D3/D2 wall-ornament order, front-cell collision, D0/D1 visible-square draw-order, wall-alcove C2548, champion stat panel, and ambient dungeon sound gates now resolve current local code/source boundaries and reflect the closed no-ambient-loop source boundary.
- 🔒 DM1 source-lock audit completed across movement, rendering, creatures, combat, spells, champions, inventory, survival, sensors, entrance, save/load, audio, and data structures.

### DM1 V2.0 / V2.1 / V2.2

- ✅ V2.0 filtered presentation: config, CRT scanlines, palette correction, dither cleanup, sharpening, renderer integration, and launcher/menu integration.
- ✅ V2 parity/presentation scaffold: Phase 0 and Phase 1 command routing, deterministic config, profile boundary, and launch-smoke verification.
- ✅ V2.1 asset pipeline: Phase 2 source-preserving upscale/EPX pipeline, deterministic cache behavior, fallback handling, and probe coverage.
- ✅ V2 presentation slices: HUD/action route gate, palette/projectile metadata gates, smooth-movement runtime bridge, touch/controller route gate, and presentation-disabled state-hash gate.
- ✅ DM1 V2 Phase 4 field/projectile VFX binding gate: source explosion thing IDs map to V2 overlay/emitter families, fluxcage remains field-only, unknown things are rejected, and invalid source palette lighting falls back deterministically.

## Chaos Strikes Back (CSB)

### CSB V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Boot/profile split, profile-specific asset discovery, boot state, diagnostics, and hash-matched launch boundary.
- ✅ Phase 2 dungeon-data probe slice: synthetic CSB dungeon loading, square/thing accessors, door table, sensor helpers, endgame helpers, world-model behavior, and CSB-vs-DM1 difference checks are covered by `firestaff_csb_v1_dungeon_model_probe`.
- ✅ Launch/profile fixture: the Atari ST asset-pair manifest and CSB launch-intent gate now recognize hash-matched CSB assets as valid for the M12 profile boundary while keeping gameplay, save, and pixel parity as non-claims.
- ✅ Source-lock audit coverage for CSB startup, utility, dungeon loading, wall rendering, champion import, weapons, magic, creatures, combat, and save behavior.
- ✅ Phase 4 - Mechanics parity slices for CSB-specific movement/interaction/runtime behavior.
- ✅ Phase 5 - Creature and combat parity slices.
- ✅ Completed rendering slices: D3/D2 wall-table mapping, parity bitmap selection, grid routing, and initial CSB viewport source-lock gates.
- ✅ CSB V1 viewport Phase 3 gate: D3L2/D3R2 and D2L2/D2R2 draw-order, coordinate, frame, and PC34 zone contracts are source-locked against F0676-F0679/F0128.
- ✅ CSB V1 back-wall ornament routing gate: D3L2/D3R2 wall cases source-lock their F0107 ordinal slots and view-wall indices, while D2L2/D2R2 prove the no-F0107 return path.

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Boot/profile split, asset discovery, launcher state, and runtime selection.
- ✅ Phase 7 - Save/import compatibility verification.
- ✅ Phase 8 - Verification-suite scaffold and probes.
- ✅ Source-lock audit coverage for DM2 boot, dungeon/data loading, rendering, items, creatures, combat, spells, shops/NPCs, save behavior, and verification paths.

### DM2 V2.0 / V2.1 / V2.2

- ✅ Phase 5 smooth-movement scaffold: DM2 V2 has visual walk, turn, and stair interpolation state, viewport query hooks, and source-evidence strings while preserving V1 tick ownership of game-state movement.

## Dungeon Master Nexus

### Nexus V1

- ✅ Phase 0 - Provenance and source audit setup for Saturn DMDF/DGN references.
- ✅ Phase 1 - Runtime profile and launch/profile boundary scaffolding.
- ✅ Phase 2 - Data format ingestion for Nexus dungeon and supporting Saturn data structures.
- ✅ Phase 3 - Core world model and runtime state mapping.
- ✅ Phase 4 - Rendering pipeline slices and viewport/source-lock scaffolding.
- ✅ Phase 5 - Mechanics parity implementation for movement, click routing, item use, doors, pits, teleporters, triggers, combat, AI, and sound routes.
- ✅ Phase 6 - Save/import compatibility, including Nexus V1 save/load round-trip probe coverage.
- ✅ Phase 7 - Verification-suite coverage for compile, save/load, and runtime-state paths.
- 🔒 Source-lock audit coverage for Nexus DMDF/DGN loading, sensors, movement, input, inventory, doors, triggers, combat, AI, sound, save/load, and launch/runtime boundaries.

## Theron's Quest

### Theron V1

- ✅ Phase 0 - Provenance and source audit setup.
- ✅ Phase 1 - Runtime profile and launch/profile scaffolding.
- ✅ Phase 2 - Dungeon/data model ingestion.
- ✅ Phase 3 - Core world/progression state mapping.
- ✅ Launch/data availability now uses Track 02 hash/provenance discovery through validator, startup, and menu availability state.
- ✅ Phase 5 - Initial mechanics implementation for movement, click routes, doors, pits, teleporters, altar behavior, combat, drops, and sounds.
- ✅ Phase 6 - Dungeon progression probe coverage.
- 🔒 Source-lock audit coverage for Theron profile, dungeon progression, mechanics, and launch/runtime boundaries.

## Cross-Cutting

### Launcher and Settings

- ✅ M12 settings persistence bridge: quick resume, minimap, automap, combat log, soundtrack, ambient audio, UI scale, streamer mode, custom music, custom dungeon, screenshot path, and all five per-game option slots round-trip through the startup menu probe.

### Touch and Input

- ✅ Launcher and entrance click-zone scaffolding.
- ✅ DM1 touch/click routes for movement, turning, status/champion selection, and item interaction.

### Accessibility

- ✅ Accessibility manifest writer and launcher/game-state scaffold.
- ✅ Launcher high-contrast palette and configurable font-scale foundation with M12 probe coverage.

### Platform and Packaging

- ✅ macOS Debug CMake build path.
- ✅ CI Phase A headless probe path.
- ✅ Release packaging scripts for macOS, Windows, and Linux preview builds.
- ✅ macOS app bundle icon resource wiring.
