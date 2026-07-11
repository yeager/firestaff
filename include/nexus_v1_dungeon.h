
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
#define NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT 5
#define NEXUS_DGN_POST_GRID_0X24_ZERO_BYTES 128
#define NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES 16
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE 3
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK 0x3f
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_FLAG_MASK 0x80
#define NEXUS_DGN_MAX_COLLISION_SECTORS 256
#define NEXUS_DGN_MAX_POST_GRID_0X30_RECORDS 4096
#define NEXUS_V1_DGN_VIEW_DISTANCE 4
#define NEXUS_V1_DGN_VIEW_RENDER_MAX_COMMANDS 48
#define NEXUS_V1_DGN_VIEWPORT_UNITS 1024

/* These are real Structure1 header offsets observed across LEV00..LEV15.
 * Only the first span has a source-backed name: Structure1B collision
 * indexes refer to Structure1C. The remaining spans stay opaque. */
typedef struct {
    int header_offset;
    int relative_offset;
    int size_to_next;
    int present;
    int bounded;
} Nexus_V1_DgnStructure1PostGridPointer;

/* The first bounded post-grid span is the Structure1C collision record
 * table. Every real LEV00..LEV15 form is a four-byte record array whose
 * first byte equals its record count; record zero is not addressable by a
 * Structure1B collision index. Field meanings inside each record remain
 * deliberately undecoded here. DMWeb DGN files: Structure1B bytes 5..7. */
typedef struct {
    int relative_offset;
    int size;
    int record_size;
    int record_count;
    int indexed_record_count;
    int valid;
} Nexus_V1_DgnStructure1CRecordTable;

/* Real-media evidence, LEV00..LEV15: the bounded span selected by header
 * offset 0x24 is exactly 128 zero bytes.  This establishes a reserved span,
 * not a payload interpretation. */
typedef struct {
    int relative_offset;
    int size;
    int valid;
} Nexus_V1_DgnPostGrid0x24ZeroSpan;

/* Real-media evidence, LEV00..LEV15: header offset 0x30 reaches a span
 * bounded by the following 0x34 pointer. It contains a typed prefix of
 * 16-byte rows followed by one opaque tail row. In every prefix row, byte 3
 * stores its six-bit row ordinal and may carry only bit 7 as an observed
 * flag. No other record byte has a gameplay or rendering interpretation. */
typedef struct {
    int relative_offset;
    int size;
    int record_size;
    int record_count;
    int typed_prefix_record_count;
    int opaque_tail_record_count;
    int row_ordinal_prefix_valid;
    /* Byte 3's high bit is observed per-row provenance only. It is never
     * selected through a Structure1B packed reference value. */
    int row_ordinal_flagged_prefix_record_count;
    int first_row_ordinal_flagged_prefix_record;
    int last_row_ordinal_flagged_prefix_record;
    /* Measurements only: byte positions remain deliberately unnamed. */
    int field_distinct_value_count[NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES];
    int referenced_field_distinct_value_count[
        NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES];
    int valid;
} Nexus_V1_DgnPostGrid0x30RecordTable;

typedef struct {
    int structure1_offset;
    int useful_size;
    int structure1b_relative_offset;
    int structure1b_end_relative_offset;
    int post_grid_offset;
    int post_grid_size;
    Nexus_V1_DgnStructure1PostGridPointer
        post_grid[NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT];
    Nexus_V1_DgnStructure1CRecordTable structure1c;
    Nexus_V1_DgnPostGrid0x24ZeroSpan post_grid_0x24_zero_span;
    Nexus_V1_DgnPostGrid0x30RecordTable post_grid_0x30_records;
    int valid;
} Nexus_V1_DgnStructure1Layout;

typedef struct {
    int dmweb_container;
    int structure1_offset;
    int structure1_size;
    int structure1_useful_size;
    int structure1b_offset;
    int structure1b_size;
    int geometry_offset;
    int geometry_size;
    int structure1c_offset;
    int structure1c_size;
    int structure1c_record_count;
    int structure1c_indexed_record_count;
    int collision_records_valid;
    int post_grid_0x24_zero_span_valid;
    int post_grid_0x24_zero_span_size;
    int post_grid_0x30_record_table_valid;
    int post_grid_0x30_record_count;
    int post_grid_0x30_typed_prefix_record_count;
    int post_grid_0x30_opaque_tail_record_count;
    int post_grid_0x30_row_ordinal_prefix_valid;
    int post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    int post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    int post_grid_0x30_last_row_ordinal_flagged_prefix_record;
    int collision_ref_count;
    int collision_ref_unique_count;
    int max_collision_ref;
    int post_grid_0x30_ref_count;
    int post_grid_0x30_ref_unique_count;
    int max_post_grid_0x30_ref;
    int post_grid_0x30_record_zero_referenced;
    int post_grid_0x30_ref_value_count;
    int mesh_ready;
} Nexus_V1_DgnGeometryInfo;

/* One bounded Structure1C record as used by the existing collision route.
 * This table proof establishes only the count and four-byte record form;
 * individual byte semantics are not inferred here. */
typedef struct {
    int valid;
    int circle;
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
} Nexus_V1_DgnCollisionSector;

/* Runtime-facing view of one Structure1B cell. Movement and rendering must
 * consume this same decoded record so a resumed party cannot collide against
 * a stale square map while the viewport uses newer mesh/material data. */
typedef struct {
    int square_type;
    uint16_t collision_ref;
    uint16_t post_grid_0x30_ref;
    uint8_t floor_material_ref;
    uint8_t ceiling_material_ref;
    uint8_t wall_material_refs[4];
    int8_t floor_height[4];
    int8_t ceiling_height[4];
    uint8_t floor_slope;
    uint8_t floor_rotation;
    Nexus_V1_DgnCollisionSector collision_sector;
    int post_grid_0x30_row_prefix_valid;
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
    uint16_t post_grid_0x30_refs[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
    Nexus_V1_DgnCollisionSector
        collision_sectors[NEXUS_DGN_MAX_COLLISION_SECTORS];
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
    int post_grid_0x30_ref_count;
    int post_grid_0x30_ref_unique_count;
    int max_post_grid_0x30_ref;
    int post_grid_0x30_ref_value_count;
    int post_grid_0x24_zero_span_valid;
    int post_grid_0x30_record_table_valid;
    int post_grid_0x30_record_count;
    int post_grid_0x30_typed_prefix_record_count;
    int post_grid_0x30_opaque_tail_record_count;
    int post_grid_0x30_row_ordinal_prefix_valid;
    int post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    int post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    int post_grid_0x30_last_row_ordinal_flagged_prefix_record;
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
    uint16_t post_grid_0x30_ref;
    Nexus_V1_DgnCollisionSector collision_sector;
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
    int post_grid_0x30_row_prefix_valid;
    uint8_t material_id;
    Nexus_V1_DgnRenderCommandKind material_source_kind;
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
    int ceiling_count;
    int wall_count;
    int floor_material_command_count;
    int ceiling_material_command_count;
    int wall_material_command_count;
    int material_semantics_complete;
    int post_grid_0x30_reference_command_count;
    int post_grid_0x30_valid_reference_command_count;
    int first_post_grid_0x30_ref;
    int max_post_grid_0x30_ref;
    int post_grid_0x30_row_ordinal_flagged_prefix_record_count;
    int post_grid_0x30_first_row_ordinal_flagged_prefix_record;
    int post_grid_0x30_last_row_ordinal_flagged_prefix_record;
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
int nexus_v1_dgn_structure1_layout(Nexus_V1_DgnStructure1Layout *out_layout,
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
