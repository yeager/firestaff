
#ifndef NEXUS_V1_DUNGEON_H
#define NEXUS_V1_DUNGEON_H
#include <stdint.h>

/* DM Nexus dungeon level format (.DGN files).
 * Source-lock: DMWeb "Dungeon Master Nexus DGN files", fetched 2026-05-28.
 * Real LEV*.DGN files are 2048-byte block containers. Structure1 contains
 * Structure1B: a 64x64 grid with 8 bytes per cell (0x8000 bytes total).
 * The old Firestaff "raw 32x32 at offset 0" layout is retained only as a
 * synthetic-fixture fallback while older probes are retired. */

#define NEXUS_MAX_MAP_SIZE 64
#define NEXUS_DGN_BLOCK_SIZE 2048
#define NEXUS_DGN_STRUCTURE1B_BYTES 0x8000
#define NEXUS_DGN_STRUCTURE1B_CELL_BYTES 8
#define NEXUS_DGN_GEOMETRY_DESCRIPTOR_MIN_BYTES 4
#define NEXUS_DGN_MAX_COLLISION_SECTORS 256
#define NEXUS_DGN_MAX_MESH_DESCRIPTORS 4096
#define NEXUS_V1_DGN_VIEW_DISTANCE 4
#define NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS 48
#define NEXUS_V1_DGN_VIEWPORT_UNITS 1024

typedef struct {
    int dmweb_container;
    int structure1_offset;
    int structure1_size;
    int structure1_useful_size;
    int structure1b_offset;
    int structure1b_size;
    int geometry_offset;
    int geometry_size;
    int collision_ref_count;
    int collision_ref_unique_count;
    int max_collision_ref;
    int mesh_ref_count;
    int mesh_ref_unique_count;
    int max_mesh_ref;
    int mesh_ready;
} Nexus_V1_DgnGeometryInfo;

/* DMWeb DGN Structure1C collision descriptor. Coordinates are signed tile
 * parts relative to the cell centre; a final value of -128 denotes a circle. */
typedef struct {
    int valid;
    int circle;
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
} Nexus_V1_DgnCollisionSector;

typedef struct {
    int valid;
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
} Nexus_V1_DgnMeshDescriptor;

/* Runtime-facing view of one Structure1B cell. Movement and rendering must
 * consume this same decoded record so a resumed party cannot collide against
 * a stale square map while the viewport uses newer mesh/material data. */
typedef struct {
    int square_type;
    uint16_t collision_ref;
    uint16_t mesh_ref;
    uint8_t floor_material_ref;
    uint8_t ceiling_material_ref;
    uint8_t wall_material_refs[4];
    int8_t floor_height[4];
    int8_t ceiling_height[4];
    uint8_t floor_slope;
    uint8_t floor_rotation;
    Nexus_V1_DgnCollisionSector collision_sector;
    Nexus_V1_DgnMeshDescriptor mesh_descriptor;
} Nexus_V1_DgnCellGeometry;

typedef struct {
    int width, height;
    uint8_t squares[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint16_t collision_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t floor_material_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t ceiling_material_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t wall_material_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE][4];
    int8_t floor_heights[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t floor_slopes[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint8_t floor_rotations[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    uint16_t mesh_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    Nexus_V1_DgnCollisionSector
        collision_sectors[NEXUS_DGN_MAX_COLLISION_SECTORS];
    Nexus_V1_DgnMeshDescriptor
        mesh_descriptors[NEXUS_DGN_MAX_MESH_DESCRIPTORS];
    int thing_count;
    int creature_count;
    int has_3d_geometry;
    int geometry_offset;
    int geometry_size;
    Nexus_V1_DgnGeometryInfo geometry_info;
} Nexus_V1_Level;

typedef enum {
    NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING = 0,
    NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH = 1,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY = 2,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET = 3,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK = 4
} Nexus_V1_DgnRendererHandoffStatus;

typedef struct {
    Nexus_V1_DgnRendererHandoffStatus status;
    int dmweb_container;
    int mesh_ready;
    int can_render_dgn_mesh;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
    int width;
    int height;
    int geometry_offset;
    int geometry_size;
    int collision_ref_count;
    int collision_ref_unique_count;
    int max_collision_ref;
    int mesh_ref_count;
    int mesh_ref_unique_count;
    int max_mesh_ref;
    int descriptor_capacity;
} Nexus_V1_DgnRendererHandoffReceipt;

typedef enum {
    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR = 1,
    NEXUS_V1_DGN_RENDER_COMMAND_CEILING = 2,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT = 3,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT = 4,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT = 5
} Nexus_V1_DgnRenderCommandKind;

typedef struct {
    Nexus_V1_DgnRenderCommandKind kind;
    int x;
    int y;
    int depth;
    int lateral; /* -1=left, 0=center, +1=right */
    int square_type;
    int wall_dir;
    uint16_t collision_ref;
    uint16_t mesh_ref;
    Nexus_V1_DgnCollisionSector collision_sector;
    Nexus_V1_DgnMeshDescriptor mesh_descriptor;
    /* DGN byte3 is signed 1/32 world-unit height. The four values are
     * NW, NE, SE, SW and carry the source-cell slope where present. */
    int8_t floor_height[4];
    int8_t ceiling_height[4];
    uint8_t floor_rotation;
    uint8_t floor_slope;
    /* Screen-space quad in NEXUS_V1_DGN_VIEWPORT_UNITS. The DGN plan
     * owns projection and material selection; hosts only rasterize it. */
    int16_t quad_x[4];
    int16_t quad_y[4];
    int mesh_descriptor_projected;
    uint8_t material_id;
    uint8_t palette_index;
    uint8_t draw_order;
} Nexus_V1_DgnRenderCommand;

typedef struct {
    Nexus_V1_DgnRendererHandoffStatus status;
    int plan_ready;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
    int command_count;
    int floor_count;
    int wall_count;
    int mesh_command_count;
    int mesh_descriptor_command_count;
    int first_mesh_ref;
    int max_mesh_ref;
    int source_cell_count;
    int missing_material_count;
    int first_missing_material_id;
    Nexus_V1_DgnRenderCommandKind first_missing_material_kind;
    int first_blocking_depth;
    int first_blocking_x;
    int first_blocking_y;
} Nexus_V1_DgnRenderPlanReceipt;

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index);
int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y);
int nexus_v1_level_get_collision_ref(const Nexus_V1_Level *level, int x, int y);
int nexus_v1_level_get_material_ref(const Nexus_V1_Level *level, int x, int y,
                                    Nexus_V1_DgnRenderCommandKind kind,
                                    int wall_dir);
int nexus_v1_level_get_cell_geometry(const Nexus_V1_Level *level, int x, int y,
                                     Nexus_V1_DgnCellGeometry *out_cell);
/* Returns non-zero only when the DGN target cell and its collision sector
 * admit a center-to-center party step. */
int nexus_v1_level_move_allowed(const Nexus_V1_Level *level,
                                int from_x, int from_y,
                                int to_x, int to_y);
int nexus_v1_dgn_geometry_info(Nexus_V1_DgnGeometryInfo *out_info,
                               const uint8_t *data,
                               int size);
int nexus_v1_level_dgn_renderer_handoff_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnRendererHandoffReceipt *out_receipt);
const char *nexus_v1_dgn_renderer_handoff_status_name(
    Nexus_V1_DgnRendererHandoffStatus status);
int nexus_v1_level_build_dgn_view_render_plan(
    const Nexus_V1_Level *level,
    int party_x,
    int party_y,
    int party_dir,
    Nexus_V1_DgnRenderCommand *commands,
    int max_commands,
    Nexus_V1_DgnRenderPlanReceipt *out_receipt);

#endif
