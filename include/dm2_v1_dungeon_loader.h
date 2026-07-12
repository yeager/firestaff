
#ifndef FIRESTAFF_DM2_V1_DUNGEON_LOADER_H
#define FIRESTAFF_DM2_V1_DUNGEON_LOADER_H
#include <stdint.h>

/* DM2: The Legend of Skullkeep (1993)
 * Uses enhanced dungeon.dat format:
 *   - Outdoor levels (sky, trees, buildings)
 *   - Indoor dungeon levels (similar to DM1)
 *   - Multi-floor buildings within outdoor areas
 *   - Extended creature type table
 *   - Weather zones (rain, fog)
 * Source: SKULL.ASM (522128 lines disassembly) */

/* PROBE_NOTES — DM2 DUNGEON.DAT header contract (PC English, 39437 bytes):
 *
 *   Byte offset  0: uint16_le: 0x0000 (reserved/padding)
 *   Byte offset  2: uint16_le: 0x4731 ("G1" format magic/version, ASCII)
 *   Byte offset  4: uint16_le: 0x002c (44) — first level data offset or header size
 *   Byte offset  6: uint16_le: 0x001c (28) — LEVEL COUNT
 *   Byte offset  8: uint16_le: 0x0101 (257) — dungeon seed
 *   Byte offset 10: uint16_le: 0x0938 (2360) — dungeon flags/metadata
 *   Byte offset 12: uint16_le: 0x0035 (53) — ???
 *   Byte offset 14: uint16_le: 0x00d9 (217) — ???
 *   Byte offset 16: uint16_le: 0x0240 (576) — ???
 *   ...
 *   The PC G1 real-data path reads 16-byte skproject-compatible
 *   Map_definitions from byte 44 and byte-sized column-major map squares
 *   from the trailing map-data block. It also bounds the G1-specific
 *   extension between the standard DB-pool prefix and map tail, without
 *   assigning it record ownership before source proof. Tile type is stored
 *   in the high three bits and bit 0x10 marks a thing-list square. The
 *   bounded legacy loader path still accepts older Firestaff synthetic
 *   16-bit map fixtures.
 *
 *   Confirmed against: SKULL.ASM T560 DUNGEON_Load, local DUNGEON.DAT probe.
 *   Confirmed loader contract: level_count/map_count is byte offset 6. */

#define DM2_V1_MAX_LEVELS 30
#define DM2_V1_MAX_MAP_SIZE 64

typedef enum {
    DM2_LEVEL_OUTDOOR = 0,
    DM2_LEVEL_INDOOR,
    DM2_LEVEL_BUILDING,
} DM2_LevelType;

/*
 * Bounded provenance receipt for the PC DOS G1 c_record pool region.
 *
 * The candidate span is anchored immediately after the already-proven
 * column-index, ground-stack, and text tables.  It is intentionally not a
 * address map. The loader promotes this exact source-ordered span only after
 * every map-rooted c_record chain validates. Unreachable pool slots are not
 * links. The later G1 extension is never included in candidate_pool_bases;
 * the receipt separately reports the narrowly proven DB3/DB4 continuation
 * and leaves every other extension byte untyped.
 */
typedef struct {
    int available;
    int text_end;
    int candidate_base;
    int candidate_end;
    int candidate_bytes;
    int candidate_pool_bases[16];
    int root_count;
    int root_end_markers;
    int root_shape_valid;
    int root_shape_invalid;
    int map_root_count;
    int map_root_end_markers;
    int map_root_null_markers;
    int map_root_shape_valid;
    int map_root_shape_invalid;
    int map_root_extension_shape_valid;
    int map_root_unresolved_after_extension;
    int map_root_direct_by_type[16];
    int map_root_extension_by_type[16];
    int map_root_unresolved_by_type[16];
    int map_root_count_by_map[DM2_V1_MAX_LEVELS];
    int map_root_extension_by_map[DM2_V1_MAX_LEVELS];
    int map_root_unresolved_by_map[DM2_V1_MAX_LEVELS];
    int candidate_record_count;
    int candidate_first_link_end_markers;
    int candidate_first_link_shape_valid;
    int candidate_first_link_shape_invalid;
    int tail_pool_base;
    int tail_pool_base_rejected;
} DM2_V1_G1RecordPoolEvidence;

#define DM2_V1_G1_PARTIAL_BOOT_MAX_BLOCKED_ROOTS 5

/*
 * A map boot may materialize the source-proven G1 root address classes while
 * retaining an explicit incomplete state.  It is not a record graph: no w0
 * link is followed and blocked roots have no substitute address.
 */
typedef struct {
    int map;
    int x;
    int y;
    uint16_t object_id;
    int type;
    int index;
} DM2_V1_G1BlockedRoot;

#define DM2_V1_G1_FIRST_MAP_MAX_ROOTS 70

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    int type;
    int index;
} DM2_V1_G1VerifiedRoot;

/* Source-scoped DB1 teleporter payload. This is not a GenericRecord view:
 * skproject/SKWIN/DME.h Teleporter lines 367-382 fixes this six-byte layout.
 * The next-link word is intentionally absent because partial G1 boot does not
 * authorize GenericRecord::w0 traversal. */
typedef struct {
    int x;
    int y;
    uint16_t object_id;
    int index;
    uint8_t destination_x;
    uint8_t destination_y;
    uint8_t destination_map;
    uint8_t scope;
    uint8_t sound;
    uint8_t rotation;
    uint8_t rotation_type;
} DM2_V1_G1TeleporterRoot;

/* Source-scoped DB2 text payload. skproject/SKWIN/DME.h Text fixes the
 * four-byte layout: w2 carries visibility, mode, and the text-table index.
 * The GenericRecord::w0 link and text-table bytes remain intentionally
 * outside this partial-G1 receipt. */
typedef struct {
    int x;
    int y;
    uint16_t object_id;
    int index;
    uint8_t direction;
    uint8_t visible;
    uint8_t mode;
    uint16_t text_index;
} DM2_V1_G1TextRoot;

#define DM2_V1_G1_MAP5_MAX_TEXT_ROOTS 16

/* Map 5 is the first canonical G1 map with direct DB2 roots. This is a
 * read-only field receipt, not a text decoder or record graph. */
typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int text_root_count;
    int text_record_reads;
    int generic_record_reads;
    int blocked_record_reads;
    DM2_V1_G1TextRoot texts[DM2_V1_G1_MAP5_MAX_TEXT_ROOTS];
} DM2_V1_G1Map5TextRuntimeReceipt;

#define DM2_V1_G1_TEXT_WALL_GFX_MAX 16

/* A source-bound DB2 Text root which skproject dispatches as a WALL_GFX
 * ornament. This remains material metadata only: no chain traversal, text
 * decoding, rectangle selection, or image blit is inferred here. */
typedef struct {
    uint16_t object_id;
    uint16_t text_index;
    uint8_t wall_gfx_index;
    uint16_t colorkey;
    uint16_t position;
    uint16_t do_not_flip;
    uint16_t alcove_type;
    uint16_t image_offset;
} DM2_V1_G1TextWallGfxMaterial;

typedef struct {
    int valid;
    int map;
    int source_text_root_count;
    int material_count;
    DM2_V1_G1TextWallGfxMaterial materials[DM2_V1_G1_TEXT_WALL_GFX_MAX];
} DM2_V1_G1TextWallGfxRuntimeReceipt;

#define DM2_V1_G1_ACTUATOR_WALL_GFX_MAX 32

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint8_t direction;
    uint8_t graphic_ordinal;
    uint8_t wall_gfx_index;
    uint16_t colorkey;
    uint16_t position;
    uint16_t do_not_flip;
    uint16_t alcove_type;
    uint16_t image_offset;
} DM2_V1_G1ActuatorWallGfxMaterial;

typedef struct {
    int valid;
    int map;
    int source_actuator_root_count;
    int material_count;
    DM2_V1_G1ActuatorWallGfxMaterial
        materials[DM2_V1_G1_ACTUATOR_WALL_GFX_MAX];
} DM2_V1_G1ActuatorWallGfxRuntimeReceipt;

#define DM2_V1_G1_CREATURE_MAP_CHIP_MAX 32

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint8_t direction;
    uint8_t creature_type;
    uint32_t raw_hash;
    uint32_t raw_byte_count;
    int image_width;
    int image_height;
    int image_format;
} DM2_V1_G1CreatureMapChipMaterial;

typedef struct {
    int valid;
    int map;
    int source_creature_root_count;
    int material_count;
    DM2_V1_G1CreatureMapChipMaterial
        materials[DM2_V1_G1_CREATURE_MAP_CHIP_MAX];
} DM2_V1_G1CreatureMapChipRuntimeReceipt;

/* The dungeon layer owns G1 record addressing. The boot layer supplies this
 * exact typed-GDAT reader so standalone dungeon validation has no loader or
 * renderer dependency. */
typedef int (*DM2_V1_G1GdatScalarRead)(void *userdata,
                                       int entry_type,
                                       int category,
                                       int index,
                                       int field,
                                       uint16_t *out_value);

typedef int (*DM2_V1_G1GdatRawRead)(void *userdata,
                                    int entry_type,
                                    int category,
                                    int index,
                                    int field,
                                    const uint8_t **out_data,
                                    uint32_t *out_byte_count);

typedef int (*DM2_V1_G1GdatImageMetadataRead)(void *userdata,
                                               int category,
                                               int index,
                                               int field,
                                               int *out_width,
                                               int *out_height,
                                               int *out_format);

/* Why a selected DB1 teleporter did not move the party. `DestinationMap()` is
 * the raw high byte of Teleporter::w4. skproject c_moverec.cpp passes it
 * directly to c_map.cpp CHANGE_CURRENT_MAP_TO(), whose map-array access has no
 * 0xff sentinel handling; Firestaff therefore rejects 0xff before any world
 * mutation. */
typedef enum {
    DM2_V1_G1_TELEPORT_NO_TRANSITION_NONE = 0,
    DM2_V1_G1_TELEPORT_NO_TRANSITION_INCOMPLETE_WORLD,
    DM2_V1_G1_TELEPORT_NO_TRANSITION_SOURCE_TILE,
    DM2_V1_G1_TELEPORT_NO_TRANSITION_SOURCE_DISABLED,
    DM2_V1_G1_TELEPORT_NO_TRANSITION_SCOPE,
    DM2_V1_G1_TELEPORT_NO_TRANSITION_DESTINATION_MAP_SENTINEL,
    DM2_V1_G1_TELEPORT_NO_TRANSITION_DESTINATION_MAP_RANGE,
    DM2_V1_G1_TELEPORT_NO_TRANSITION_DESTINATION_COORDINATES
} DM2_V1_G1TeleporterNoTransitionReason;

/* A map-0 party pose may select one already-decoded direct DB1 teleporter.
 * It preserves DME.h's raw fields and records a transition only after the
 * complete world, source-tile, scope, and destination gates are all valid.
 * It never reads w0, follows a record chain, or reads a blocked root. */
typedef struct {
    int committed;
    int incomplete_world;
    int source_map;
    int source_x;
    int source_y;
    uint16_t source_object_id;
    int source_index;
    uint8_t destination_x;
    uint8_t destination_y;
    uint8_t destination_map;
    uint8_t scope;
    uint8_t sound;
    uint8_t rotation;
    uint8_t rotation_type;
    int generic_record_reads;
    int blocked_record_reads;
    int source_tile_active;
    int party_scope_allowed;
    int destination_map_valid;
    int destination_coordinates_valid;
    int resolved_destination_map;
    int transition_applied;
    int sound_requested;
    int no_transition_reason;
} DM2_V1_G1TeleporterTransitionReceipt;

typedef struct {
    int committed;
    int incomplete;
    int level_count;
    int map_root_count;
    int direct_root_count;
    int db3_root_count;
    int db4_root_count;
    int materialized_root_count;
    int blocked_root_count;
    int blocked_root_count_by_map[DM2_V1_MAX_LEVELS];
    DM2_V1_G1BlockedRoot blocked_roots[DM2_V1_G1_PARTIAL_BOOT_MAX_BLOCKED_ROOTS];
} DM2_V1_G1PartialMapBootReceipt;

/* First-map runtime handoff for the transactional PC G1 boot. This carries
 * source-proven root-address classes only; it deliberately contains no
 * decoded c_record payload or inferred object. */
typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int width;
    int height;
    int root_count;
    int direct_root_count;
    int db3_root_count;
    int db4_root_count;
    int verified_root_count;
    int blocked_root_count;
    int object_count;
    int teleporter_root_count;
    int teleporter_record_reads;
    int blocked_record_reads;
    DM2_V1_G1VerifiedRoot roots[DM2_V1_G1_FIRST_MAP_MAX_ROOTS];
    DM2_V1_G1TeleporterRoot teleporters[DM2_V1_G1_FIRST_MAP_MAX_ROOTS];
} DM2_V1_G1FirstMapRuntimeReceipt;

typedef struct {
    int level_count;
    DM2_LevelType level_types[DM2_V1_MAX_LEVELS];
    int level_widths[DM2_V1_MAX_LEVELS];
    int level_heights[DM2_V1_MAX_LEVELS];
    int level_offsets[DM2_V1_MAX_LEVELS];
    int map_offset_x[DM2_V1_MAX_LEVELS];
    int map_offset_y[DM2_V1_MAX_LEVELS];
    int map_door_set0[DM2_V1_MAX_LEVELS];
    int map_door_set1[DM2_V1_MAX_LEVELS];
    int square_bytes;
    int raw_map_data_base;
    int column_index_base;
    int square_first_thing_base;
    int square_first_thing_count;
    int text_data_base;
    int text_word_count;
    int g1_extension_base;
    int g1_extension_size;
    int thing_data_bases[16];
    int thing_type_counts[16];
    int g1_extension_record_bases[16];
    int g1_extension_record_counts[16];
    /* Set only when the source layout has materialized every map-to-record
     * ownership table.  A byte-square map alone is not a playable graph. */
    int record_graph_complete;
    /* Canonical PC G1 may commit a map-only boot while preserving its five
     * blocked roots and disabled record traversal. */
    DM2_V1_G1PartialMapBootReceipt partial_map_boot;
    uint8_t *raw_data;
    int raw_size;
    /* DM2 outdoor extension */
    int sky_texture_index;
    int weather_zone_count;
} DM2_V1_DungeonData;

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out, const uint8_t *dat, int size);
int dm2_v1_dungeon_get_square_type(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_get_tile_raw(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_set_tile_raw(DM2_V1_DungeonData *d, int level, int x, int y, uint16_t raw);
int dm2_v1_dungeon_get_first_thing(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_get_next_thing(const DM2_V1_DungeonData *d, uint16_t thing);
int dm2_v1_dungeon_find_thing_of_type(
    const DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int desired_type,
    int max_steps);
int dm2_v1_dungeon_find_text_wall_gfx(
    const DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int view_dir,
    int side_index,
    int max_steps,
    int *out_wall_gfx_index,
    int *out_wall_gfx_field);
int dm2_v1_dungeon_find_actuator_wall_gfx_ordinal(
    const DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int view_dir,
    int side_index,
    int max_steps,
    int *out_wall_gfx_ordinal);
int dm2_v1_dungeon_resolve_actuator_wall_gfx(
    const DM2_V1_DungeonData *d,
    uint16_t first_thing,
    int view_dir,
    int side_index,
    int max_steps,
    const uint8_t *wall_gfx_list,
    int wall_gfx_count,
    int *out_wall_gfx_index,
    int *out_wall_gfx_field);
int dm2_v1_dungeon_get_map_wall_gfx_list(
    const DM2_V1_DungeonData *d,
    int level,
    uint8_t *out_wall_gfx_list,
    int out_capacity);
int dm2_v1_dungeon_get_map_graphics_style(
    const DM2_V1_DungeonData *d,
    int level);

typedef struct { int valid; int dir,x,y; uint8_t tile_w2,tile_type,oriented_bits[4]; int first_record_link; uint8_t neighbor_tile_w2[4]; } DM2_V1_StoneRoomInputReceipt;
int dm2_v1_dungeon_stone_room_input_receipt(const DM2_V1_DungeonData *d,int level,int dir,int x,int y,DM2_V1_StoneRoomInputReceipt *out);
typedef struct { int valid; uint8_t w0,w2,w6[4]; uint16_t xvalue; } DM2_V1_StoneRoomBaseCellReceipt;
int dm2_v1_dungeon_stone_room_base_cell(const DM2_V1_StoneRoomInputReceipt *in,DM2_V1_StoneRoomBaseCellReceipt *out);
const uint8_t *dm2_v1_dungeon_get_thing_record(
    const DM2_V1_DungeonData *d,
    uint16_t thing,
    int *out_type,
    int *out_index,
    int *out_size);
int dm2_v1_dungeon_is_outdoor(const DM2_V1_DungeonData *d, int level);
/* Validate the source-shaped map -> ground-stack -> DB-record graph.
 * Returns 1 only when every declared direct c_record link and every
 * thing-bearing square resolve to bounded, terminating ObjectID chains.
 * PC G1 files whose direct graph is incomplete return 0 instead of being
 * promoted as a partial world. */
int dm2_v1_dungeon_validate_record_graph(const DM2_V1_DungeonData *d);
/* Validate only the source-owned c_record pool address transform:
 * text end + sum(glbItemSizePerDB[type] * nRecords[type]).  This is the
 * G1 map-boot gate. It deliberately does not assign GenericRecord::w0 link
 * semantics; callers that need to walk a chain must use the stricter graph
 * validator above. */
int dm2_v1_dungeon_validate_record_pools(const DM2_V1_DungeonData *d);
/* Collect non-mutating PC G1 c_record provenance for the source-ordered pool
 * span. Record lookup/traversal is available only when record_graph_complete
 * is set by the independent bounded graph validator. */
int dm2_v1_dungeon_collect_g1_record_pool_evidence(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1RecordPoolEvidence *out);
/* Transactionally scans the PC G1 map roots and commits a partial-map boot
 * receipt only when every root is either direct, in the proven DB3/DB4
 * continuation, or one of the five explicitly blocked DB8/DB10 roots.
 * The result remains incomplete and does not authorize record traversal. */
int dm2_v1_dungeon_materialize_g1_partial_map_boot(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1PartialMapBootReceipt *out);
/* Consume the transactional receipt for map 0 only. This repeats c_map's
 * ground-stack root lookup and classifies source-proven addresses. It may read
 * only a direct DB1 teleporter's independently verified fields; it never reads
 * GenericRecord::w0, DB4, or a blocked record. */
int dm2_v1_dungeon_materialize_g1_first_map_runtime(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1FirstMapRuntimeReceipt *out);
/* Consume only map 5's direct DB2 Text::w2 payload fields. This never reads
 * GenericRecord::w0, decodes text data, or reads DB3/DB4/blocked roots. */
int dm2_v1_dungeon_materialize_g1_map5_text_runtime(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1Map5TextRuntimeReceipt *out);

/* Consumes only direct DB2 TextMode()==1 roots already materialized by the
 * G1 map-5 receipt. skproject QUERY_CLS2_OF_TEXT_RECORD maps their low
 * TextIndex byte to WALL_GFX; DRAW_WALL_ORNATE consumes the exact five GDAT
 * fields recorded here. Missing or malformed source material rejects all. */
int dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out);

/* Direct DB3 roots only. c_record resolves the address; skproject
 * Actuator::GraphicNumber() maps through the current map's one-based
 * WallGraphics list before DRAW_WALL_ORNATE consumes the GDAT fields. */
int dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out);

/* DRAW_DEFAULT_DOOR_BUTTON can consume a WALL_GFX image only after the
 * direct DB2/DB3 record path has materialized that exact graphic index.
 * Both currently source-bounded paths select image field 1. */
int dm2_v1_g1_text_wall_gfx_allows_button_material(
    const DM2_V1_G1TextWallGfxRuntimeReceipt *receipt,
    int wall_gfx_index,
    int image_field);
int dm2_v1_g1_actuator_wall_gfx_allows_button_material(
    const DM2_V1_G1ActuatorWallGfxRuntimeReceipt *receipt,
    int wall_gfx_index,
    int image_field);

/* Direct or source-proven extension DB4 roots only. It binds
 * Creature::CreatureType() to CREATURES/type dtImage/F9 raw ownership, not
 * sprite decoding, animation selection, palette conversion, or drawing. */
int dm2_v1_dungeon_materialize_g1_creature_map_chip_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatRawRead read_raw,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    void *read_userdata,
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out);

/* Used by the dungeon viewport after the real F9 fetch/decode. A DB4
 * creature is drawable only when its decoded surface matches the source
 * metadata receipt for that creature type. */
int dm2_v1_g1_creature_map_chip_matches_decoded_material(
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt,
    int creature_type,
    int image_width,
    int image_height);
void dm2_v1_dungeon_free(DM2_V1_DungeonData *d);
const char *dm2_v1_dungeon_source_evidence(void);
#endif
