# Architecture Overview

## Layer Model

Firestaff uses a strict four-layer ownership model. Each layer owns specific data and responsibilities; cross-layer access follows typed receipts rather than direct memory sharing.

| Layer | Identifier | Owns | Must Not Own |
|-------|-----------|------|-------------|
| M12 | Launch/Profile | Source discovery, selected game/profile, launch facts | Dungeon mutation |
| M11 | Game View | SDL events, frame presentation, private host surfaces | Thing semantics |
| M10 | Engine Core | Loaded maps, Thing chains, timeline, transactional saves | Generated art |
| V1 Modules | Game-Specific | PC34/skproject decision, render, and input receipts | Unverified asset fallback |

## Module Layout

```
src/
  engine/       M11 game view (m11_game_view.c)
  csb/          CSB-specific: viewport, boot, runtime, DSA
  dm1/          DM1-specific: viewport 3D, music, movement, combat, inventory
  dm1v2/        DM1 V2 presentation layer (camera, modern rendering)
  dm2/          DM2 runtime (skproject reference)
  shared/       Cross-game: audio (SDL3), rendering, main entry
  memory/       Dungeon data decoding, save/load, combat serialization
  frontend/     UI frontend, dialog, text rendering
  audio/        Audio decoding (SND, SONG.DAT)

include/        All public headers (~2554 files)
tests/          Test sources (~3138 files)
parity-evidence/ Source-lock evidence documents
```

## Game State

`M11_GameViewState` (defined in `include/m11_game_view.h`) is the monolithic state struct (~1570 lines). It contains:

- World state (current map, party position, direction)
- Dungeon data (loaded maps, Thing chains, dungeon metadata)
- Audio state (music source, sound driver)
- Viewport state (render plans, material cache)
- HUD state (champion panels, action/spell, cursor)
- DM2 state (GDAT, G1 records, creature occupancy)
- Presentation state (V2 camera, scaling, surfaces)

## Data Ownership Principles

1. **Source identity over filenames**: Data is identified by content hash, never by filename or folder structure.
2. **Fail-closed rendering**: If a required original asset cannot be verified, the frame records a no-draw receipt rather than substituting synthetic art.
3. **Candidate-first saves**: Save imports stage all data into a detached candidate. Publication occurs only after all ranges, checksums, and queue indexes validate.
4. **Chain mutation via loaded-chain primitive**: Every Thing list edit goes through the source-shaped F0267 unlink/append path, preserving tail ordering and map context.
5. **Receipt-based handoff**: Each layer communicates through typed receipts. A receipt records what was consumed, what was blocked, and why.

## Rendering Pipeline

### V1 (Source-Compatible)

The V1 indexed framebuffer is 320x200. Render requests name a source graphic, palette/indexed material, rectangle, and destination layer. The pipeline:

1. M10 supplies the static Thing chain and typed live projectile/explosion lists
2. V1 module builds the F0115 summary and layer plan
3. M11 consumes the receipt and presents via SDL3

### V2 (Presentation)

V2 scaling, filtering, and camera transformations happen after the V1 decision boundary. V2 cannot repair a missing source draw — it only transforms what V1 produced.

## Cross-Game Shared Infrastructure

- **SDL3 backend**: Window management, input, audio mixing, GPU rendering
- **Dungeon data decoder**: Shared Thing/map/sensor structures (`memory_dungeon_dat_pc34_compat.h`)
- **Combat system**: Shared attack types, wound probabilities, creature attributes
- **Save system**: Candidate-first import with transactional commit
- **Timeline dispatcher**: Source event classes with ordered dispatch

## Per-Game Specifics

### DM1/CSB (ReDMCSB Reference)

Shared M10 engine with game-specific V1 modules. CSB adds DSA execution, extended saves, and IMG1 nibble-RLE graphics decoding. Both use the same dungeon chain primitives but have independent viewport renderers.

### DM2 (skproject Reference)

Separate runtime path with GDAT-based material resolution. G1 byte-square format with record graphs instead of linked Thing chains. Independent creature occupancy grid, combat drops, and PCM sound decode.

### Theron's Quest (No Reference Source)

Record-based PC Engine CD layout. Evidence-driven: Track 02 IPL/stage-two/stage-three handoff chain proved from binary analysis. Multi-level object tables, SRM save boundary, and transactional Continue.

### DM Nexus (No Reference Source)

Saturn DMDF/DGN data family. Structure1B geometry, Structure3 face topology, MNS static materials, PRS3 compressed assets (decode blocked pending Saturn capture evidence), SLEV scripts, SAL/MAP audio.
