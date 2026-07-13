
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
#define NEXUS_DGN_STRUCTURE1_POST_GRID_POINTER_COUNT 6
#define NEXUS_DGN_POST_GRID_0X24_ZERO_BYTES 128
#define NEXUS_DGN_POST_GRID_0X30_RECORD_BYTES 16
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_BYTE 3
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_MASK 0x3f
#define NEXUS_DGN_POST_GRID_0X30_ROW_ORDINAL_FLAG_MASK 0x80
#define NEXUS_DGN_MAX_COLLISION_SECTORS 256
#define NEXUS_DGN_MAX_POST_GRID_0X30_RECORDS 4096
#define NEXUS_DGN_STRUCTURE1F_HEADER_BYTES 16
#define NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT 6
#define NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES 4096
#define NEXUS_DGN_STRUCTURE1G_DESCRIPTOR_BYTES 8
#define NEXUS_DGN_STRUCTURE1G_HEADER_BYTES 4
#define NEXUS_DGN_MAX_STRUCTURE1G_ENTRIES 256
#define NEXUS_DGN_STRUCTURE1G_FIRST_IMAGE_INDEX 0x014c
#define NEXUS_DGN_STRUCTURE2_DESCRIPTOR_BYTES 20
#define NEXUS_DGN_MAX_STRUCTURE2_TEXTURES 256
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

/* DMWeb DGN files, Structure1F: a 16-byte header followed by six counted
 * families in this fixed order. These record forms are retained as dungeon
 * semantics; rendering and mechanics do not infer behaviour from fields that
 * the Saturn format reference still marks unknown. */
typedef enum {
    NEXUS_V1_DGN_STRUCTURE1F_ITEMS = 0,
    NEXUS_V1_DGN_STRUCTURE1F_FLOOR_DECORATIONS = 1,
    NEXUS_V1_DGN_STRUCTURE1F_FLOOR_SENSORS = 2,
    NEXUS_V1_DGN_STRUCTURE1F_ALCOVES = 3,
    NEXUS_V1_DGN_STRUCTURE1F_WALL_DECORATIONS = 4,
    NEXUS_V1_DGN_STRUCTURE1F_WALL_SENSORS = 5
} Nexus_V1_DgnStructure1FFamily;

typedef struct {
    int relative_offset;
    int size;
    uint16_t wall_sensor_first_texture_index;
    uint16_t wall_sensor_first_model_index;
    int family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int family_offset[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int family_record_size[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int total_entry_count;
    int valid;
} Nexus_V1_DgnStructure1FTable;

typedef struct {
    Nexus_V1_DgnStructure1FFamily family;
    uint8_t tag;
    uint8_t x;
    uint8_t y;
    /* Items use location/item_id/attribute1/attribute2. Decorations and
     * sensors retain their documented positional/model fields below. */
    uint8_t location;
    uint8_t item_id;
    uint8_t attribute1;
    uint8_t attribute2;
    int8_t offset_x;
    int8_t offset_y;
    uint8_t model_or_aspect;
    uint8_t rotation;
    uint8_t type_or_control;
    uint8_t width;
    uint8_t height;
    uint16_t structure1a_index;
    uint8_t face;
    uint8_t destination_x;
    uint8_t destination_y;
    uint8_t destination_orientation;
} Nexus_V1_DgnStructure1FEntry;

/* Only items, floor decorations, and floor sensors expose documented direct
 * 64x64 cell coordinates. Alcove/wall families bind through Structure1A and
 * remain unresolved until that table has a source-backed parser. */
typedef struct {
    int valid;
    int typed_entry_count;
    int direct_coordinate_entry_count;
    int item_entry_count;
    int floor_decoration_entry_count;
    int floor_sensor_entry_count;
    int direct_coordinate_unique_cell_count;
    int direct_coordinate_duplicate_cell_count;
    int structure1a_bound_entry_count;
} Nexus_V1_DgnStructure1FSpatialReceipt;

/* DMWeb DGN files, Structure1G: optional animated-texture declarations.
 * A present table has a counted descriptor prefix and four-byte instruction
 * streams. Image instructions, backward FF FE gotos, and FF FF terminators
 * are validated here; timing flags and runtime stepping remain unexecuted. */
typedef struct {
    int relative_offset;
    int size;
    int descriptor_count;
    int animated_texture_count;
    int animation_data_relative_offset;
    int sequence_count;
    int image_instruction_count;
    int goto_instruction_count;
    int valid;
} Nexus_V1_DgnStructure1GTable;

typedef struct {
    uint8_t animation_id;
    uint16_t first_image_index;
    /* Structure1G uses the global image space: ITEM.IBS owns 0..0x14b,
     * while Structure2 starts at 0x14c. This is a descriptor ID, never a
     * decoded texture or a substitute material selector. */
    uint16_t first_structure2_image_id;
    int first_structure2_image_valid;
    uint16_t sequence_word_offset;
    int sequence_instruction_count;
    int image_instruction_count;
    int goto_instruction_count;
    /* Every non-control instruction is only an index into the already
     * bounded Structure2 descriptor table. This records the complete
     * original-data relation without assigning payload or texture meaning. */
    int structure2_image_instruction_bound_count;
    int structure2_image_instruction_unbound_count;
} Nexus_V1_DgnStructure1GEntry;

/* DMWeb DGN files, Structure2: 20-byte texture descriptors followed by a
 * FFFF terminator and raw palette/image data. Structure1G's global image
 * index is routed here only after subtracting 0x14c and finding this local
 * descriptor ID. Payload decoding remains a separate host gate. */
typedef struct {
    uint16_t image_id;
    uint16_t encoding;
    uint16_t palette_id;
    uint16_t width;
    uint16_t height;
    uint32_t image_relative_offset;
    uint32_t palette_relative_offset;
} Nexus_V1_DgnStructure2Texture;

/* The only established Structure2 payload grammar is the descriptor envelope:
 * Descriptor[20]... FFFF opaque_bytes[].  The bytes after FFFF are retained
 * as a bounded span, but are deliberately not called image or palette records
 * until the retail LEV corpus establishes their offset base and encoding. */
typedef struct {
    int descriptor_bytes;
    int terminator_offset;
    int opaque_payload_offset;
    int opaque_payload_size;
    /* Read-only descriptor-offset correlation. These counters only compare
     * the already parsed numeric fields with the bounded post-FFFF span;
     * they do not assign an offset base, record grammar, pixel codec, or
     * palette meaning to those bytes. */
    int nonzero_descriptor_offset_count;
    int nonzero_descriptor_offsets_in_opaque_payload_count;
    int nonzero_descriptor_offsets_outside_opaque_payload_count;
    int local_payload_offset_pattern_observed;
    int valid;
    int material_or_image_data_proven;
} Nexus_V1_DgnStructure2Payload;

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
    Nexus_V1_DgnStructure1FTable structure1f;
    Nexus_V1_DgnStructure1GTable structure1g;
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
    /* A non-zero packed Structure1B reference may only address a row in the
     * observed 0x30 typed prefix. The final row stays opaque and is never
     * render-addressable. */
    int post_grid_0x30_references_valid;
    int post_grid_0x30_invalid_ref_count;
    int first_invalid_post_grid_0x30_ref;
    int post_grid_0x30_record_zero_referenced;
    int post_grid_0x30_ref_value_count;
    int structure1f_valid;
    int structure1f_total_entry_count;
    int structure1f_family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int structure1g_present;
    int structure1g_valid;
    int structure1g_animated_texture_count;
    int structure1g_sequence_count;
    int mesh_ready;
} Nexus_V1_DgnGeometryInfo;

/* One bounded Structure1C record placeholder. The table proof establishes
 * only the count and four-byte record form; individual byte semantics are
 * not inferred. These fields deliberately stay clear for retail DGN data
 * until a Saturn executable or capture proves their geometry grammar. */
typedef struct {
    int valid;
    int circle;
    int8_t x1;
    int8_t y1;
    int8_t x2;
    int8_t y2;
} Nexus_V1_DgnCollisionSector;

/* Runtime-facing view of one Structure1B cell. Movement and rendering consume
 * the same documented cell/reference data. Structure1C record bytes are not
 * promoted to collision geometry until their Saturn grammar is evidenced. */
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
    uint8_t floor_animation_ids[NEXUS_MAX_MAP_SIZE][NEXUS_MAX_MAP_SIZE];
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
    Nexus_V1_DgnStructure1FEntry
        structure1f_entries[NEXUS_DGN_MAX_STRUCTURE1F_ENTRIES];
    int structure1f_entry_count;
    Nexus_V1_DgnStructure1GEntry
        structure1g_entries[NEXUS_DGN_MAX_STRUCTURE1G_ENTRIES];
    int structure1g_entry_count;
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    Nexus_V1_DgnStructure2Texture
        structure2_textures[NEXUS_DGN_MAX_STRUCTURE2_TEXTURES];
    int structure2_texture_count;
    int structure2_texture_table_valid;
    Nexus_V1_DgnStructure2Payload structure2_payload;
} Nexus_V1_Level;

typedef enum {
    NEXUS_V1_DGN_RENDERER_HANDOFF_MISSING = 0,
    NEXUS_V1_DGN_RENDERER_HANDOFF_READY_MESH = 1,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_NO_GEOMETRY = 2,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_DESCRIPTOR_BUDGET = 3,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_LEGACY_FALLBACK = 4,
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE_SEMANTICS = 5,
    /* The parsed DGN has no committed canonical Track 1 Structure2 source. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE2_SOURCE = 6,
    /* Visible direct Structure1F decorations/sensors have source cells but
     * no Saturn draw/trigger semantics yet. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_SEMANTICS = 7,
    /* A packed Structure1B reference reached the opaque Structure1F tail or
     * lies outside the observed typed prefix. It is not drawable geometry. */
    NEXUS_V1_DGN_RENDERER_HANDOFF_BLOCKED_STRUCTURE1F_REFERENCE = 8
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
    int post_grid_0x30_references_valid;
    int post_grid_0x30_invalid_ref_count;
    int first_invalid_post_grid_0x30_ref;
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
    int structure1f_valid;
    int structure1f_total_entry_count;
    int structure1f_family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int structure1f_typed_entry_count;
    Nexus_V1_DgnStructure1FSpatialReceipt structure1f_spatial;
    int structure1g_present;
    int structure1g_valid;
    int structure1g_animated_texture_count;
    int structure1g_sequence_count;
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    int structure1g_image_instruction_count;
    int structure1g_goto_instruction_count;
    int structure1g_structure2_image_instruction_bound_count;
    int structure1g_structure2_image_instruction_unbound_count;
} Nexus_V1_DgnRendererHandoffReceipt;

typedef enum {
    NEXUS_V1_DGN_RENDER_COMMAND_FLOOR = 1,
    NEXUS_V1_DGN_RENDER_COMMAND_CEILING = 2,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_FRONT = 3,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_LEFT = 4,
    NEXUS_V1_DGN_RENDER_COMMAND_WALL_RIGHT = 5
} Nexus_V1_DgnRenderCommandKind;

typedef enum {
    NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_NONE = 0,
    /* DMWeb: Structure1B byte4 low nibble 3 declares an animated floor. */
    NEXUS_V1_DGN_ANIMATED_MATERIAL_ROUTE_STRUCTURE2_FLOOR = 1
} Nexus_V1_DgnAnimatedMaterialRoute;

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
    int animated_texture_declared;
    uint8_t animated_texture_id;
    uint16_t animated_texture_first_image_index;
    uint16_t animated_texture_structure2_image_id;
    int animated_texture_structure2_image_valid;
    Nexus_V1_DgnAnimatedMaterialRoute animated_texture_host_route;
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
    int structure1f_valid;
    int structure1f_total_entry_count;
    int structure1f_family_count[NEXUS_DGN_STRUCTURE1F_FAMILY_COUNT];
    int structure1f_typed_entry_count;
    Nexus_V1_DgnStructure1FSpatialReceipt structure1f_spatial;
    /* Direct-coordinate Structure1F records whose documented 64x64 source
     * cell appears in this DGN plan. This is provenance only: no record is
     * interpreted as an object, sensor, trigger, or draw command. */
    int structure1f_plan_direct_entry_count;
    int structure1f_plan_item_entry_count;
    int structure1f_plan_floor_decoration_entry_count;
    int structure1f_plan_floor_sensor_entry_count;
    int structure1g_present;
    int structure1g_valid;
    int structure1g_animated_texture_count;
    int structure1g_sequence_count;
    int structure1g_floor_animation_cell_count;
    int structure1g_floor_animation_bound_count;
    int structure1g_image_instruction_count;
    int structure1g_goto_instruction_count;
    int structure1g_structure2_image_instruction_bound_count;
    int structure1g_structure2_image_instruction_unbound_count;
    int animated_material_command_count;
    int unresolved_animated_material_count;
    int source_cell_count;
    int missing_material_count;
    int first_missing_material_id;
    Nexus_V1_DgnRenderCommandKind first_missing_material_kind;
    int first_blocking_depth;
    int first_blocking_x;
    int first_blocking_y;
    /* Set only by the engine after the loaded level's Structure2 source
     * receipt is bound to its canonical Track 1 materialization. */
    int structure2_source_materialization_bound;
} Nexus_V1_DgnRenderPlanReceipt;

int nexus_v1_level_load(Nexus_V1_Level *level, const uint8_t *data, int size, int level_index);
int nexus_v1_level_get_square(const Nexus_V1_Level *level, int x, int y);
int nexus_v1_level_get_collision_ref(const Nexus_V1_Level *level, int x, int y);
int nexus_v1_level_get_material_ref(const Nexus_V1_Level *level, int x, int y,
                                    Nexus_V1_DgnRenderCommandKind kind,
                                    int wall_dir);
int nexus_v1_level_get_cell_geometry(const Nexus_V1_Level *level, int x, int y,
                                     Nexus_V1_DgnCellGeometry *out_cell);
int nexus_v1_level_structure1f_spatial_receipt(
    const Nexus_V1_Level *level,
    Nexus_V1_DgnStructure1FSpatialReceipt *out_receipt);
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
