# Firestaff v2.3 — native 3D plan

Status: proposed release plan, 2026-08-25.

## Goal

v2.3 adds a native, deterministic 3D presentation route for all five game
families: Dungeon Master, Chaos Strikes Back, Dungeon Master II, Theron's
Quest, and DM Nexus. It does not bundle or invoke an emulator, BIOS, firmware,
System Card, game dump, generated texture, or generated gameplay data.

“3D” means the original game's appropriate visual model:

| Game | v2.3 3D model | Production data rule |
|---|---|---|
| DM1 | grid-based first-person 2.5D corridor renderer | Only hash-recognised original DAT assets, palettes and dungeon records. |
| CSB | grid-based first-person 2.5D corridor renderer | Atari ST, Amiga and FM Towns assets remain edition-bound; no fictitious DOS/PC route. |
| DM2 | indoor grid renderer plus outdoor terrain/building route | Original GDAT/DUNGEON records per admitted edition. |
| Theron's Quest | PC Engine CD grid-based first-person 2.5D renderer | Direct JP/US Track 02 data; later-dungeon/material promotion requires an authentic consumer binding. |
| Nexus | perspective textured polygon renderer | Japanese Saturn disc data and authenticated DGN/DMDF/VDP ownership; unknown material ownership stays no-draw. |

V2.2's generated artwork remains an explicitly labelled non-production fixture;
it is not an input to this release.

## Architecture

Introduce `fs_render3d` as an SDL-backed native software renderer with a
stable, fixed-point scene contract. The core owns camera transforms, clipping,
depth ordering, fog/lighting inputs, indexed/direct-colour conversion and
deterministic raster output. It accepts only a scene emitted by a game adapter;
it never guesses source assets, palettes, textures, UVs, object semantics or
timing.

Three scene adapters keep game formats honest:

1. `grid_scene`: DM1, CSB and Theron. It converts verified square, door,
   object and camera records into walls, billboards and overlays, retaining the
   original palette and draw order.
2. `outdoor_scene`: DM2. It adds source-owned terrain, weather, time-of-day,
   buildings and indoor/outdoor transitions while retaining the indoor grid
   path.
3. `polygon_scene`: Nexus. It consumes only verified DGN/DMDF geometry and
   VDP1/VDP2 material receipts. An unknown ownership edge produces no-draw,
   never a placeholder surface.

The renderer must be deterministic on Linux, macOS and Windows at a source
resolution selected by the game adapter. SDL remains the platform/output
layer; OpenGL/Vulkan may be optional presentation backends later, but neither
may alter the authoritative software framebuffer or become a requirement.

## Delivery sequence

### 0. Contracts and safety rails

- Define immutable `FS3DScene`, `FS3DMaterial`, `FS3DCamera`, `FS3DLight` and
  `FS3DFrameReceipt` interfaces; every material carries source identity and
  edition/media hash.
- Add no-draw reasons for missing asset, palette, consumer, capture or
  platform ownership.
- Add a production-boundary test that rejects generated V2.2 assets, external
  process launches, BIOS/firmware paths and cross-edition material mixing.

### 1. Shared renderer core

- Implement fixed-point projection, near-plane clipping, depth buffer,
  textured quads, billboards, palette lookup, direct-colour conversion and
  deterministic fog/light composition.
- Produce stable software-frame hashes on Linux, macOS and Windows.
- Add golden tests using only tiny test fixtures; fixtures prove renderer
  mechanics, never game-image parity.

### 2. DM1 and CSB grid routes

- Bind each renderer submission to the selected original edition's dungeon,
  graphics and palette records.
- Implement source-order wall, door, pit, stairs, ornament, creature, item and
  champion-panel composition.
- Add real-media capture comparisons for DM1 PC/Atari/FM Towns/Amiga and CSB
  Atari ST/Amiga/FM Towns. A platform is not promoted from a fixture-only test.

### 3. DM2 indoor and outdoor routes

- Finish original GDAT material/palette ownership for DOS, Amiga, FM Towns and
  supported Macintosh routes.
- Bind indoor view, outdoor terrain, sky/weather, buildings and transitions to
  authenticated source data; test each platform separately.
- Keep any missing outdoor asset no-draw rather than substituting modern art.

### 4. Theron grid route

- Use direct archive/CUE/BIN reading and the verified Track 02 loader for the
  first dungeon's geometry path.
- Promote later dungeons, actors, items, saves, text, palette and audio only
  after an authentic CD-to-native-consumer trace exists for that edition.
- Verify JP and US independently; no System Card or emulator is required at
  runtime.

### 5. Nexus polygon route

- Complete DGN/DMDF geometry, transform, face selection, culling, texture,
  CLUT/palette and VDP1/VDP2 layer ownership from the Japanese retail disc.
- Bind title/menu and first gameplay scene to one same-session authentic
  material-to-consumer receipt before any production draw is admitted.
- Add native software replay tests of admitted frames; retain fail-closed
  behaviour for unknown Saturn state instead of using a BIOS or an emulator.

### 6. Release integration

- Wire every route into CLI direct launch and M12 start-menu launch.
- Run the real-data five-game matrix and the per-platform matrix without media
  extraction to disk.
- Publish per-game/per-platform completion evidence and a v2.3 known-differences
  list. Mark only capture-compared routes as visually matched.

## Release gates

v2.3 may ship only when all of the following hold:

- Every game starts through CLI and M12 using only its supplied game-data
  directory/archive.
- The three renderer adapters accept original-data-backed materials only;
  generated V2.2 art is rejected in production.
- Linux, macOS and Windows have identical authoritative framebuffer hashes for
  each admitted regression scene.
- Each advertised platform has a verified original-media launch, gameplay
  scene, input action and save/load boundary.
- Nexus and Theron missing capture edges remain visibly blocked/no-draw rather
  than silently approximated.
- SPDX SBOM, secret scan, repository-media policy and native-runtime policy all
  pass.

## Main risks

- Nexus material/VDP ownership and Theron later-dungeon consumer traces are
  evidence blockers, not implementation shortcuts. They must be acquired or
  the relevant route remains closed.
- DM2 outdoor rendering has the largest cross-platform data variance and needs
  per-edition tests before shared code is promoted.
- Pixel matching requires original captures; source-format decoding alone is
  insufficient evidence.
