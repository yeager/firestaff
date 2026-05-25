# Nexus V1 Graphics / Rendering Audit

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

## 3. Nexus V1 Graphics System: 3D Polygon Rendering

Nexus uses full 3D polygon rendering via the rasterizer pipeline.

Rendering pipeline (nexus_v2_render_pipeline.c):
1. Load DGN file (grid plus 3D geometry blob)
2. Parse embedded polygon data from geometry blob
3. Transform vertices with camera matrix (nexus_v1_math3d.c)
4. Project to 2D framebuffer via perspective divide
5. Depth sort transparent faces (painter is algorithm)
6. Rasterize with texture sampling (nexus_v1_rasterizer.c)

Wall geometry in DGN files: Pre-baked wall polygons per grid square per direction.
Each wall face has vertex list plus texture ID plus normal.
No procedural geometry — geometry loaded from file.

Floor/ceiling in DGN files: nexus_viewport.c draws floor/ceiling per open square.
nexus_draw_floor() projects square polygon to screen. Texture ID determines tile.

Creatures: DMDF 3D models (.MNS files) loaded on demand via nexus_v1_dmdf_load().
3D mesh with vertices, normals, texture coordinates. Transformed and depth-sorted
per frame. vs. DM1 is 2D billboard sprites.

## 4. Math3D System (nexus_v1_math3d.h/c)

- Vec3, Vec4 vector types
- Matrix4x4 transforms (rotation plus translation)
- Camera projection (perspective divide)
- Vertex transformation: model -> world -> view -> projection

## 5. Rasterizer (nexus_v1_rasterizer.c)

- Fixed-point or float polygon rasterization
- Texture-mapped triangles (DMDF u,v coords)
- Z-buffer for depth sorting
- Framebuffer palette system (Nexus_Framebuffer)

## 6. DM1 vs Nexus Graphics Comparison

| Aspect           | DM1                     | Nexus V1                  |
|------------------|-------------------------|---------------------------|
| Rendering        | 2D raycasting           | 3D polygon rasterizer     |
| Walls            | 2D scaled rectangles    | 3D polygon faces          |
| Floor/ceiling    | Pre-baked BITMAP tiles  | 3D mesh projection        |
| Creatures        | 2D billboard sprites    | 3D DMDF mesh models       |
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

Model usage: nexus_viewport_render() calls nexus_draw_creature() for visible
creatures. Creatures projected to screen via camera matrix. Scaled by distance.
Billboard behavior: model always faces camera (DM1-style optimization).

## 8. Saturn-Specific Rendering Constraints

Saturn hardware: Dual SH2 CPUs (master plus slave), framebuffer 16-bit RGBA
or palette mode, texture memory limited and compressed, no dedicated 3D GPU.

Firestaff nexus_v1_rasterizer.c implements the software rasterizer:
- Runs on host CPU, renders to software framebuffer
- Uploads to Saturn framebuffer via CD-ROM data streaming
- For PC/Linux builds: renders directly to SDL framebuffer

## 9. Firestaff Implementation Status

Implemented:
- DMDF header parser (nexus_v1_dmdf_model.c)
- 3D math (nexus_v1_math3d.c)
- Software rasterizer (nexus_v1_rasterizer.c)
- Viewport renderer (nexus_v1_viewport.c)
- DGN grid parser (nexus_v1_dungeon.c, partial)

Not yet implemented:
- DGN 3D geometry blob parser (wall/floor polygon extraction)
- SDDRVS.TSK script VM
- DMDF texture decompression
- Creature billboard alignment to camera
