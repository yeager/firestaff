# Nexus V1 Graphics / Rendering Audit

> The format and raster notes here are not a parity claim. See
> [`NEXUS_STALE_CLAIM_AUDIT.md`](NEXUS_STALE_CLAIM_AUDIT.md) for older
> “draws/transformed/loaded”-formuleringar som endast betyder diagnostic
> receipt.

## 1. DMDF — Dungeon Master Data Format

Nexus Saturn uses DMDF for all 3D creature models. Defined in nexus_v1_dmdf_model.h.

DMDF file header structure (nexus_v1_dmdf_model.h):
- Magic: 0x444D4446 (DMDF in ASCII) at offset 0
- uint32 file_size, section_count, flags
- uint32 data_offset, vertex_offset
- uint32 vertex_count, face_count

DMDF is big-endian (SH2 Saturn processor). Files use the .MNS extension.
Creature model examples in extracted data:
- ANTMAN.MNS (53,768 bytes), BIGWORM.MNS (53,784 bytes)
- BORKETH.MNS (67,644 bytes), CHAOS.MNS (88,572 bytes)
- DRA_ZOM.MNS (83,508 bytes), GHOST.MNS (48,840 bytes)
- GOLEM.MNS (48,140 bytes), H_HOUND.MNS (46,364 bytes)
- etc.

DMDF stores:
- Nexus_DMDFVertex: int16 x,y,z plus int16 nx,ny,nz (normal) plus uint16 u,v (texcoords)
- Triangle/quad faces as uint16 index arrays
- Embedded texture data (BITMAP format, compressed)

## 2. DM1 Graphics System: 2D Sprites

DM1 renders 2D sprite graphics for all dungeon elements:

Graphics data (DM1 PC 3.4):
- GRAPHICS.DAT: contains all sprite frames (~250KB)
- Pre-baked floor/ceiling patterns in BITMAP format
- Wall texture tiles (4 directions times 3 distances = 12 views)
- Sprite animation frames per creature (4 directions times N frames)

DM1 viewport rendering (2D raycasting):
- Forward render: D0-D3 distance bands, 3 columns wide
- 2D wall projection: scaled rectangle for each visible wall
- Floor/ceiling: pre-baked BITMAP tiles rendered at correct distance
- Creatures: sprite scaling based on distance, billboard orientation
- Items: flat sprite overlay at floor level

DM1 sprite system: every creature has a sprite set in GRAPHICS.DAT.
Sprites are 2D bitmaps, always facing the party (billboard). No vertex data,
no polygon data. Rendering: SDL blit with scaling.

## 3. Nexus V1 Graphics System: source and runtime boundary

The authenticated VDP1 mode-1 capture compositor is reachable through the
explicit `nexus_viewport_replay_vdp1_capture()` adapter. It replays one
capture-bound quad only after exact DGN image/palette joins, command framing,
original-Saturn attestation and a caller-supplied display origin. This is a
capture witness path, not a general DGN mesh renderer; local-coordinate /
system-clip state, command ordering and VDP2 composition still require the
same-run Saturn capture before the normal viewport route can open.

The corresponding VDP2 lane is exposed through
`nexus_viewport_replay_vdp2_nbg1_capture()`. It accepts only an authenticated
NBG1 bitmap-mode register tuple, an exact 512×256 VRAM bitmap join and an
exact 256-entry CRAM join. The crop and destination rectangle are explicit
capture facts supplied by the caller; no host placement is inferred from
registers. Current captures still have no exact retail asset/CLUT owner, so
this adapter does not open startup, menu or HUD presentation by itself.

The retail files contain polygon/texture candidates, and Firestaff has bounded
parsers and host raster primitives. That is not proof that the host pipeline
matches Saturn VDP1 or that the route is production-renderable.

The admitted pipeline is instead:
1. Load and hash-bind real DGN/MNS/DMDF bytes.
2. Record bounded Structure2/Structure3/texture evidence.
3. Stop before an inferred transform, palette upload or VDP1 command is
   promoted to the Nexus viewport.

Wall geometry in DGN files is retained as bounded pre-baked polygon/material
candidates per grid square and direction. Each candidate may contain a vertex
list, texture selector and normal, but its Saturn consumer is not yet bound.
Do not turn this source record into a host-rendered wall by assumption.

Floor/ceiling in DGN are source-data/geometry candidates. `nexus_viewport.c`
must not be presented as Saturn's displayed floor/ceiling; projection,
material, and VDP1/VDP2 placement are capture-gated.

Creatures: DMDF/MNS bytes can be read into bounded mesh/texture receipts.
Transform, depth order, palette, and VDP1 command ownership remain unbound;
this is not a completed creature renderer.

## 4. Math3D System (nexus_v1_math3d.h/c)

- Vec3, Vec4 vector types
- Matrix4x4 transforms (rotation plus translation)
- Camera projection (perspective divide)
- Vertex transformation: model -> world -> view -> projection

## 5. Raster primitives (nexus_v1_rasterizer.c)

The file contains bounded host raster primitives and framebuffer types. They
are not a Saturn-capture substitute and remain gated when source transform,
texture palette or VDP1 command ownership is absent.

## 6. DM1 vs Nexus Graphics Comparison

| Aspect           | DM1                     | Nexus V1                  |
|------------------|-------------------------|---------------------------|
| Rendering        | 2D raycasting           | Source decode; Saturn presentation gated |
| Walls            | 2D scaled rectangles    | DGN face candidates; draw ownership gated |
| Floor/ceiling    | Pre-baked BITMAP tiles  | DGN material candidates; VDP2/VDP1 gated |
| Creatures        | 2D billboard sprites    | DMDF/MNS source models; draw placement gated |
| Items            | 2D sprite overlay       | 3D model or billboard    |
| Textures         | BITMAP (PC CGA/EGA)     | BITMAP in DMDF            |
| Z-buffer         | None                    | Z-buffer                  |
| Camera           | Fixed first-person, 4 dirs | Free rotation via matrix |
| Lighting         | None (flat colors)      | Per-face normals          |
| Geometry source  | None (2D math only)     | Baked in DGN/MNS files    |
| Geometry size    | 0 bytes                 | ~4.3 MB (DGN) + models    |

## 7. DMDF Model Loading

nexus_v1_dmdf_load() parses .MNS files:
- Validates magic (DMDF)
- Reads vertex count, face count from header
- Allocates vertex array (Nexus_DMDFVertex)
- Loads face index array
- Loads embedded texture BITMAP data

The model loader is a source/format receipt. A production creature draw is not
admitted: viewport projection, camera transform, scale, palette and VDP1
command ownership remain unknown.

## 8. Saturn-Specific Rendering Constraints

Saturn hardware: Dual SH2 CPUs (master plus slave), framebuffer 16-bit RGBA
or palette mode, texture memory limited and compressed, no dedicated 3D GPU.

Firestaff contains host-side raster primitives for bounded tests. They do not
upload to a Saturn framebuffer and must not be described as a VDP1 emulator or
as parity evidence.

## 9. Firestaff Implementation Status

Implemented as source-bound diagnostics:
- DMDF header/mesh envelope parser (`nexus_v1_dmdf_model.c`)
- 3D math and bounded software-raster primitives
- DGN Structure1B/Structure1C/Structure1F/Structure2/Structure3 source
  accounting (`nexus_v1_dungeon.c`)
- Real MNS/DGN texture and face relationships retained without an inferred
  Saturn material or draw command

Still capture-gated in production:
- DGN-to-VDP1 command order and camera/transform semantics
- PRS3 pixel/palette consumer and VDP1 texture upload
- DMDF texture palette ownership and creature draw placement
- VDP2 floor/HUD/automap placement and explored-state writes
- SDDRVS/SLEV runtime consumers
