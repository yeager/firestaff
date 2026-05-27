# Firestaff TODO - Open Work

This file tracks remaining work only. Completed work belongs in `DONE.md`.

## Legend

- ❌ Not started
- 🔧 In progress / partial
- 🐛 Known bug

## Dungeon Master (DM1)

### DM1 V1

- 🔧 Original DOS capture parity: paired original PC 3.4 transcripts/screenshots are still needed before promoting exact framebuffer parity for viewport, wall, collision, creature-chain, and champion-panel cases. Firestaff-side gates, source locks, and runtime routing are already complete.
- 🔧 Inventory/chest polish beyond source-locked routes: core slot/body/chest/backpack/source routes are implemented; remaining work is broader chest runtime detail coverage and pixel-polish evidence.
- 🔧 Viewport wall evidence hardening: wall draw loop, door frames, and source-locked viewport rendering are implemented; remaining work is narrow verification of `g_dm1_wall_frame_bitmaps`, D4 far-object pass, and full door-occlusion pixel gates.
- 🐛 P1 visual bugs needing capture/repro: missing or incorrect viewport walls, champion Z-order/floating, champion mirrors not visible, blurry wall inscriptions, and macOS app icon bundle resource. Treat each as unconfirmed until it has a reproducible capture or focused probe.

### DM1 V2.0 / V2.1 / V2.2

- 🔧 Phase 3 - Modern UI overlay hardening: HUD/action route gates exist; remaining work is optional inventory, champion, rune, and action-panel polish without bypassing V1 command routes or inventory transactions.
- 🔧 Phase 4 - Lighting and visual effects hardening: palette/projectile metadata gates exist; remaining work is full enhanced lighting, shadows, field effects, and deterministic fallback coverage.
- 🔧 Phase 5 - Smooth movement presentation hardening: runtime bridge/gates exist; remaining work is broader interpolation coverage while preserving V1 cooldowns, collision, sensors, creature timing, and redraw cadence.
- 🔧 Phase 6 - Touch/controller ergonomics hardening: route gates exist; remaining work is broader V2-only gesture/controller affordances with V1 touch/click parity preserved.
- 🔧 Phase 7 - V2 verification suite hardening: presentation-disabled state-hash gate exists; remaining work is full side-by-side V1/V2 deterministic input scripts plus screenshot/pixel gates for enhanced presentation.
- ❌ Phase 8 - V2.2 modern asset pipeline: define and implement the generated/modern art path, asset provenance, fallback behavior, and visual verification.

## Chaos Strikes Back (CSB)

### CSB V1

- ❌ Phase 1 - Boot/profile split: add selectable CSB profile, boot state, profile-specific asset discovery, and launch transitions.
- ❌ Phase 2 - Dungeon data model: map CSB dungeon records into Firestaff memory/runtime structures without reusing DM1-only assumptions.
- 🔧 Phase 3 - Rendering parity hardening: D3/D2 wall tables, bitmap selection, grid routing, and initial viewport gates exist; remaining work includes F0107 ornaments, F0108 floor ornaments, F0115 creature/item/projectile pass, F0111 door panel, and `CustomBackgrounds`.
- ❌ Phase 6 - Utility/import flow: implement utility-disk style champion import and CSB-specific setup flow.
- ❌ Phase 7 - Verification suite: add deterministic boot, dungeon, combat, save/import, and rendering probes.
- 🐛 Runtime handoff: title/intro/import path and CSB-specific viewport integration still need end-to-end launch verification.

### CSB V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting and magic effects.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Dungeon Master II: Skullkeep (DM2)

### DM2 V1

- ❌ Phase 2 - Dungeon/world data model: complete DM2 map, object, tile, and world-state ingestion.
- ❌ Phase 3 - Rendering pipeline: implement DM2 viewport, UI chrome, items, outdoor/indoor presentation, and palette behavior.
- ❌ Phase 4 - Mechanics parity: movement, interactions, shops/NPCs, doors, pressure plates, triggers, combat, magic, and timeline.
- ❌ Phase 5 - Creature/combat parity: complete DM2 creature AI, projectile, damage, death/drop, and sound behavior.
- ❌ Phase 6 - Utility/import flow: implement DM2-specific load/start flow and compatibility behavior.

### DM2 V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting and outdoor effects.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Dungeon Master Nexus

### Nexus V1

- 🔧 Runtime handoff/playability proof: V1 phases 0-7 are implemented/source-locked; remaining work is launcher/game-loop handoff and real Saturn asset-path proof.
- 🔧 Mechanics parity hardening: movement, click routes, item usage, doors, pits, teleporters, triggers, combat, AI, and sound are implemented; remaining work is broader runtime/probe coverage beyond compile/save-load gates.

### Nexus V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting and Saturn presentation effects.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Theron's Quest

### Theron V1

- ❌ Launch/data availability: menu entry exists, but asset validator/startup still reports Theron unavailable; wire asset hash/provenance into startup and launch path.
- ❌ Phase 4 - Rendering pipeline: implement Theron viewport/UI presentation and asset selection.
- 🔧 Phase 5 - Mechanics parity hardening: initial movement, click route, door, pit, teleporter, altar, combat, drop, and sound behavior is implemented; remaining work is focused runtime/probe coverage.
- ❌ Phase 7 - Save/import compatibility: implement and verify Theron save/load behavior and any transfer/import constraints.
- ❌ Phase 8 - Verification suite: add deterministic launch, dungeon progression, mechanics, rendering, and save/load probes.

### Theron V2.0 / V2.1 / V2.2

- ❌ Phase 0 - V1 compatibility lock before V2 work.
- ❌ Phase 1 - V2 launch/profile separation.
- ❌ Phase 2 - Enhanced asset pipeline.
- ❌ Phase 3 - Enhanced UI overlays.
- ❌ Phase 4 - Enhanced lighting/effects.
- ❌ Phase 5 - Smooth movement and viewport interpolation.
- ❌ Phase 6 - Touch/controller ergonomics.
- ❌ Phase 7 - V2 verification suite.

## Cross-Cutting

### Touch and Controller Support

- ❌ Gesture navigation for runtime movement and turning.
- ❌ UI scaling and touch-target audit across launcher and game views.

### Accessibility

- ❌ Screen reader integration for launcher and game-critical state.
- ❌ High-contrast presentation mode.
- ❌ Configurable font sizing for launcher and overlays.

### Platform Polish

- ❌ macOS app icon bundle resource verification.

## Known Bugs

- 🐛 Viewport/collision reports without capture manifests must stay as bugs until paired original PC 3.4 evidence or a reproducible local probe exists.
