
#ifndef FIRESTAFF_DM2_V1_DUNGEON_LOADER_H
#define FIRESTAFF_DM2_V1_DUNGEON_LOADER_H
#include <stdint.h>

#include "dm2_v1_asset_loader.h"

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

/* G1 record pool thing-list link terminators. Source: ReDMCSB DEFS.H. */
#define DM2_THING_END_MARKER 0xfffeu
#define DM2_THING_NULL_MARKER 0xffffu

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
 * every declared direct c_record link and map-rooted chain validates. The
 * later G1 extension remains untyped and is never included in
 * candidate_pool_bases.
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
    int map_root_extension_shape_valid;
    int root_shape_invalid;
    int candidate_record_count;
    int candidate_first_link_end_markers;
    int candidate_first_link_shape_valid;
    int candidate_first_link_extension_shape_valid;
    int candidate_first_link_shape_invalid;
    int tail_pool_base;
    int tail_pool_base_rejected;
} DM2_V1_G1RecordPoolEvidence;

#define DM2_V1_G1_TEXT_MESSAGE_MAX 16
#define DM2_V1_G1_TEXT_MESSAGE_CHARS 128

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t text_index;
    char text[DM2_V1_G1_TEXT_MESSAGE_CHARS];
    uint16_t source_word_count;
} DM2_V1_G1TextMessage;

typedef struct {
    int valid;
    int map;
    int source_text_root_count;
    int decoded_message_count;
    int blocked_phrase_message_count;
    int skipped_non_dungeon_message_count;
    DM2_V1_G1TextMessage messages[DM2_V1_G1_TEXT_MESSAGE_MAX];
} DM2_V1_G1TextMessageRuntimeReceipt;

#define DM2_V1_G1_GDAT_TEXT_MESSAGE_MAX 16

/* QUERY_MESSAGE_TEXT sends a mode-one Text record with SimpleTextExtUsage 14
 * to QUERY_GDAT_TEXT(MESSAGES, 0, low(TextIndex)). The payload encoding is
 * retained raw until FORMAT_SKSTR has source/corpus coverage; it must not be
 * substituted with a C string. */
typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t text_index;
    uint8_t gdat_field;
    uint32_t raw_byte_count;
    uint32_t raw_hash;
} DM2_V1_G1GdatTextMessage;

typedef struct {
    int valid;
    int map;
    int source_text_root_count;
    int material_count;
    int blocked_missing_text_count;
    DM2_V1_G1GdatTextMessage
        messages[DM2_V1_G1_GDAT_TEXT_MESSAGE_MAX];
} DM2_V1_G1GdatTextMessageRuntimeReceipt;

typedef struct {
    int map;
    int descriptor_base;
    int map_data_offset;
    int width;
    int height;
    uint32_t map_byte_count;
    uint32_t descriptor_hash;
    uint32_t map_hash;
} DM2_V1_G1MapCorpusEntry;

typedef struct {
    int available;
    int g1_layout_absent;
    int raw_only;
    int column_index_base;
    int column_index_word_count;
    uint32_t column_index_byte_count;
    uint32_t column_index_hash;
    int ground_stack_base;
    int ground_stack_word_count;
    uint32_t ground_stack_byte_count;
    uint32_t ground_stack_hash;
    int map_data_base;
    uint32_t map_data_byte_count;
    uint32_t map_data_hash;
    int column_index_semantics_unresolved;
    int ground_stack_semantics_unresolved;
} DM2_V1_G1GroundStackMapCorpusReceipt;

typedef struct {
    int available;
    int g1_layout_absent;
    int raw_only;
    int tile_semantics_unresolved;
    int map_count;
    int map_data_base;
    uint32_t map_data_byte_count;
    uint32_t map_data_hash;
    DM2_V1_G1MapCorpusEntry maps[DM2_V1_MAX_LEVELS];
} DM2_V1_G1MapCorpusReceipt;

#define DM2_V1_G1_TEXT_WALL_GFX_MAX 16

/* A source-bound DB2 Text root which skproject dispatches as a WALL_GFX
 * ornament. The front image fields are populated only by the image-aware
 * materialization route. They prove that the exact dtImage/1 surface which
 * DRAW_WALL_ORNATE selects for a front-facing ornate was decoded from GDAT;
 * the special WALL_GFX zero text-panel route deliberately has no such image. */
typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint8_t direction;
    uint16_t text_index;
    uint8_t wall_gfx_index;
    uint16_t colorkey;
    uint16_t position;
    uint16_t do_not_flip;
    uint16_t alcove_type;
    uint16_t image_offset;
    uint8_t front_image_ready;
    uint16_t front_image_width;
    uint16_t front_image_height;
    uint8_t front_image_format;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t raw_material_index;
    const uint8_t *raw_material_bytes;
    size_t raw_material_byte_count;
    uint32_t raw_material_hash;
    uint32_t raw_material_receipt_hash;
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
    uint8_t front_image_ready;
    uint16_t front_image_width;
    uint16_t front_image_height;
    uint8_t front_image_format;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t raw_material_index;
    const uint8_t *raw_material_bytes;
    size_t raw_material_byte_count;
    uint32_t raw_material_hash;
    uint32_t raw_material_receipt_hash;
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
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
} DM2_V1_G1CreatureMapChipMaterial;

typedef struct {
    int valid;
    int map;
    int source_creature_root_count;
    int material_count;
    DM2_V1_G1CreatureMapChipMaterial
        materials[DM2_V1_G1_CREATURE_MAP_CHIP_MAX];
} DM2_V1_G1CreatureMapChipRuntimeReceipt;

#define DM2_V1_G1_WEAPON_MAP_CHIP_MAX 32

/* skproject/SKWIN/DME.h::Weapon::ItemType() selects the original
 * WEAPONS/itemType/F9 map-chip.  These fields retain the direct G1 owner;
 * they are not a generic item-type cache. */
typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint8_t direction;
    uint8_t item_type;
    uint32_t raw_hash;
    uint32_t raw_byte_count;
    int image_width;
    int image_height;
    int image_format;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    /* Boot binds this to the exact decoded WEAPONS/itemType/F9 pixels.
     * The raw metadata receipt alone is not permission to replay another
     * decoded image with matching dimensions and palette. */
    uint32_t decoded_pixel_hash;
} DM2_V1_G1WeaponMapChipMaterial;

typedef struct {
    int valid;
    int map;
    int source_weapon_root_count;
    int material_count;
    DM2_V1_G1WeaponMapChipMaterial
        materials[DM2_V1_G1_WEAPON_MAP_CHIP_MAX];
} DM2_V1_G1WeaponMapChipRuntimeReceipt;

#define DM2_V1_G1_CONTAINER_MAP_CHIP_MAX 32

/* skproject/SKWIN/DME.h::Container::ContainerType() selects the original
 * CONTAINERS/containerType/F9 map-chip. Container contents remain outside
 * this direct-root receipt. */
typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint8_t direction;
    uint8_t container_type;
    uint32_t raw_hash;
    uint32_t raw_byte_count;
    int image_width;
    int image_height;
    int image_format;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    /* Exact decoded CONTAINERS/containerType/F9 pixel witness. */
    uint32_t decoded_pixel_hash;
} DM2_V1_G1ContainerMapChipMaterial;

typedef struct {
    int valid;
    int map;
    int source_container_root_count;
    int material_count;
    DM2_V1_G1ContainerMapChipMaterial
        materials[DM2_V1_G1_CONTAINER_MAP_CHIP_MAX];
} DM2_V1_G1ContainerMapChipRuntimeReceipt;


/* Hashes one admitted DB4 -> CREATURES/type/F9 material together with the
 * original G1 owner fields. This is a receipt identity, not a replacement
 * image or a generic creature-type lookup. */
int dm2_v1_g1_creature_map_chip_material_identity(
    const DM2_V1_G1CreatureMapChipMaterial *material,
    uint32_t *out_identity);
int dm2_v1_g1_weapon_map_chip_material_identity(
    const DM2_V1_G1WeaponMapChipMaterial *material,
    uint32_t *out_identity);
int dm2_v1_g1_container_map_chip_material_identity(
    const DM2_V1_G1ContainerMapChipMaterial *material,
    uint32_t *out_identity);

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

typedef int (*DM2_V1_G1GdatImageLocalPaletteRead)(void *userdata,
                                                   int category,
                                                   int index,
                                                   int field,
                                                   uint8_t out_palette16[16],
                                                   uint32_t *out_hash);

typedef int (*DM2_V1_G1GdatTextRead)(void *userdata,
                                     int category,
                                     int index,
                                     int field,
                                     const uint8_t **out_data,
                                     uint32_t *out_byte_count);

typedef struct {
    int map;
    int x;
    int y;
    uint16_t object_id;
    int type;
    int index;
} DM2_V1_G1BlockedRoot;

#define DM2_V1_G1_PARTIAL_BOOT_MAX_BLOCKED_ROOTS 5

typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int width;
    int height;
    int map_data_base;
    int map_data_offset;
    uint32_t map_data_byte_count;
    uint32_t map_data_hash;
    int root_count;
    int direct_root_count;
    int db3_root_count;
    int db4_root_count;
    int blocked_root_count;
    int generic_record_reads;
    int blocked_record_reads;
} DM2_V1_G1RuntimeMapValidationReceipt;

typedef struct {
    int committed;
    int incomplete_world;
    int level;
    int x;
    int y;
    uint16_t object_id;
    uint8_t type;
    uint16_t index;
    int record_offset;
    int record_size;
} DM2_V1_G1DirectRootRecordAddressReceipt;

typedef struct {
    uint16_t object_id;
    uint8_t type;
    uint16_t index;
    int record_offset;
    int record_size;
} DM2_V1_G1DirectChainNode;

#define DM2_V1_G1_DIRECT_CHAIN_MAX 16

typedef struct {
    int committed;
    int incomplete_world;
    int level;
    int x;
    int y;
    int node_count;
    int link_word_reads;
    DM2_V1_G1DirectChainNode nodes[DM2_V1_G1_DIRECT_CHAIN_MAX];
} DM2_V1_G1DirectRootChainReceipt;

typedef enum {
    DM2_V1_G1_SCENE_TILE_WALL = 1,
    DM2_V1_G1_SCENE_TILE_FLOOR = 2,
    DM2_V1_G1_SCENE_TILE_DOOR = 3
} DM2_V1_G1SceneTileClass;

typedef enum {
    DM2_V1_G1_SCENE_ROOT_GENERIC = 1,
    DM2_V1_G1_SCENE_ROOT_DOOR = 2,
    DM2_V1_G1_SCENE_ROOT_CREATURE = 3
} DM2_V1_G1SceneRootClass;

typedef struct {
    int committed;
    int incomplete_world;
    int level;
    int x;
    int y;
    uint8_t raw_tile;
    DM2_V1_G1SceneTileClass tile_class;
    DM2_V1_G1SceneRootClass root_class;
    DM2_V1_G1DirectRootChainReceipt chain;
} DM2_V1_G1DungeonSceneClassificationReceipt;

/* Source: skproject/SKULLWIN/c_savegame.cpp::READ_DUNGEON_STRUCTURE,
 * c_map.cpp::DM2_GET_OBJECT_INDEX_FROM_TILE, and
 * c_record.cpp::DM2_GET_ADDRESS_OF_RECORD.  Original raw SKSave dungeons
 * share the source-ordered column/ground-stack/pool layout, but do not carry
 * the PC G1 extension whose census is required by the G1-only receipts
 * above.  This receipt therefore exposes only the selected map's verified
 * addressable roots and their exact source bytes.  It neither substitutes a
 * G1 extension nor follows GenericRecord::w0 beyond the already-complete
 * record graph. */
typedef struct {
    int valid;
    int map;
    int width;
    int height;
    uint32_t map_data_hash;
    uint32_t terrain_hash;
    uint32_t object_record_hash;
    uint16_t thing_bearing_tile_count;
    uint16_t addressable_root_count;
    uint16_t root_count_by_type[16];
} DM2_V1_RawSKSaveMapSceneReceipt;

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t index;
    uint8_t direction;
    uint8_t button;
    uint8_t door_type;
    uint8_t button_state;
    uint8_t opening_dir;
    uint8_t ornate_index;
    uint8_t destroyable_by_fireball;
    uint8_t bashable_by_chopping;
} DM2_V1_G1DirectDoorRoot;

#define DM2_V1_G1_RUNTIME_MAP_MAX_DOOR_ROOTS 32

typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int door_root_count;
    int door_record_reads;
    int generic_record_reads;
    int blocked_record_reads;
    DM2_V1_G1DirectDoorRoot doors[DM2_V1_G1_RUNTIME_MAP_MAX_DOOR_ROOTS];
} DM2_V1_G1RuntimeMapDoorReceipt;

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t index;
    uint8_t direction;
    uint8_t actuator_type;
    uint16_t actuator_data;
    uint8_t graphic_number;
    uint8_t disabled;
    uint8_t delay;
    uint8_t sound_effect;
    uint8_t revert_effect;
    uint8_t action_type;
    uint8_t once_only;
    uint8_t active_status;
    uint8_t target_direction;
    uint8_t target_x;
    uint8_t target_y;
} DM2_V1_G1DirectActuatorRoot;

#define DM2_V1_G1_RUNTIME_MAP_MAX_ACTUATOR_ROOTS 64

typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int actuator_root_count;
    int actuator_record_reads;
    int generic_record_reads;
    int blocked_record_reads;
    DM2_V1_G1DirectActuatorRoot
        actuators[DM2_V1_G1_RUNTIME_MAP_MAX_ACTUATOR_ROOTS];
} DM2_V1_G1RuntimeMapActuatorReceipt;

/* A champion-mirror marker is not a made-up UI object: c_hero.cpp
 * DM2_SELECT_CHAMPION identifies it by Actuator::Type() == 0x7e and reads
 * the raw Actuator::Data() field.  This receipt deliberately admits
 * only a source-addressed DB3 root, including the proven PC G1 DB3
 * continuation.  It neither walks GenericRecord::w0 nor claims that the
 * surrounding world is playable. */
typedef struct {
    int map;
    int x;
    int y;
    uint16_t object_id;
    uint8_t direction;
    /* Raw DB3 w2 high nine bits.  SELECT_CHAMPION passes its low byte as
     * the signed htype, while LOAD_LOCALLEVEL_DYN preloads the exact
     * CHAMPIONS dynamic key formed below.  Do not treat this as a static
     * GDAT index: PC G1's original marker value is 0x1ff. */
    uint16_t actuator_data;
    uint8_t dynamic_hero_type;
    uint32_t dynamic_load_id;
} DM2_V1_G1ChampionMirrorRoot;

#define DM2_V1_G1_MAX_CHAMPION_MIRRORS 16

typedef struct {
    int committed;
    int incomplete_world;
    int mirror_count;
    int actuator_record_reads;
    DM2_V1_G1ChampionMirrorRoot
        mirrors[DM2_V1_G1_MAX_CHAMPION_MIRRORS];
} DM2_V1_G1ChampionMirrorReceipt;

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t index;
    uint8_t direction;
    uint8_t creature_type;
    uint16_t hit_points_1;
    /* DME.h::Creature b5 (creature-info slot; 0xff = unallocated) and the
     * record-owned sk1c9a02c3 animation cursor words w8/w10.  For
     * static-object creatures (AIDefinition w0AIFlags bit 0) the cursor is
     * the record itself, so QUERY_CREATURE_5x5_POS and the Rect14 row derive
     * from these words; for live creatures they stay evidence-only because
     * the live cursor belongs to the runtime creature-info slot. */
    uint8_t info_slot;
    uint16_t cursor_w8;
    uint16_t cursor_w10;
} DM2_V1_G1DirectCreatureRoot;

#define DM2_V1_G1_RUNTIME_MAP_MAX_CREATURE_ROOTS 64

typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int creature_root_count;
    int creature_record_reads;
    int generic_record_reads;
    int blocked_record_reads;
    DM2_V1_G1DirectCreatureRoot
        creatures[DM2_V1_G1_RUNTIME_MAP_MAX_CREATURE_ROOTS];
} DM2_V1_G1RuntimeMapCreatureReceipt;

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t index;
    uint8_t direction;
    uint8_t item_type;
    uint8_t important;
    uint8_t charges;
} DM2_V1_G1DirectWeaponRoot;

#define DM2_V1_G1_RUNTIME_MAP_MAX_WEAPON_ROOTS 32

typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int weapon_root_count;
    int weapon_record_reads;
    int generic_record_reads;
    int blocked_record_reads;
    DM2_V1_G1DirectWeaponRoot
        weapons[DM2_V1_G1_RUNTIME_MAP_MAX_WEAPON_ROOTS];
} DM2_V1_G1RuntimeMapWeaponReceipt;

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t index;
    uint8_t direction;
    uint8_t opened;
    uint8_t container_type;
} DM2_V1_G1DirectContainerRoot;

#define DM2_V1_G1_RUNTIME_MAP_MAX_CONTAINER_ROOTS 16

typedef struct {
    int committed;
    int incomplete_world;
    int map;
    int container_root_count;
    int container_record_reads;
    int generic_record_reads;
    int blocked_record_reads;
    DM2_V1_G1DirectContainerRoot
        containers[DM2_V1_G1_RUNTIME_MAP_MAX_CONTAINER_ROOTS];
} DM2_V1_G1RuntimeMapContainerReceipt;

/* Separate SKWIN DRAW_ITEM material selector. This intentionally does not
 * reuse the F9 DRAW_MAP_CHIP receipts above. */
typedef struct {
    int valid;
    uint16_t object_id;
    int x;
    int y;
    uint8_t category;
    uint8_t item_type;
    uint8_t image_field;
    uint8_t direction;
    uint8_t container_open;
    uint16_t image_offset;
    uint32_t identity_hash;
} DM2_V1_G1StaticObjectMaterialSelector;

typedef struct {
    DM2_V1_G1StaticObjectMaterialSelector selector;
    const uint8_t *raw_gfx256_bytes;
    size_t raw_gfx256_byte_count;
    uint32_t raw_gfx256_hash;
    uint32_t raw_gfx256_receipt_hash;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint32_t decoded_pixel_hash;
    uint16_t clip_rect_id;
    uint32_t raw4_hash;
    uint32_t raw4_receipt_hash;
} DM2_V1_G1StaticObjectMaterialReceipt;

typedef struct {
    int valid;
    uint16_t missile_object_id;
    uint8_t category;
    uint8_t item_type;
    uint8_t image_field;
    uint8_t flip_flags;
    uint8_t cell_pos;
    uint8_t position_5x5;
    uint16_t clip_rect_id;
    uint16_t stretch_factor64;
    uint32_t identity_hash;
} DM2_V1_G1FlyingItemSourceReceipt;

/* The DRAW_FLYING_ITEM image remains source-owned: this receipt ties the
 * QUERY_TEMP_PICST selector to its exact GRAPHICS.DAT image, palette and
 * QUERY_CREATURE_BLIT_RECTI/QUERY_EXPANDED_RECT placement evidence. */
typedef struct {
    int valid;
    DM2_V1_G1FlyingItemSourceReceipt source;
    const uint8_t *raw_gfx256_bytes;
    size_t raw_gfx256_byte_count;
    uint32_t raw_gfx256_hash;
    uint32_t raw_gfx256_receipt_hash;
    uint8_t local_palette16[16];
    uint32_t local_palette_hash;
    uint16_t clip_rect_id;
    uint32_t raw4_hash;
    uint32_t raw4_receipt_hash;
    uint32_t identity_hash;
} DM2_V1_G1FlyingItemMaterialReceipt;

typedef struct {
    int valid;
    uint16_t object_id;
    uint16_t missile_object;
    uint8_t energy_remaining;
    uint8_t energy_remaining2;
    uint16_t timer_index;
    uint32_t record_hash;
} DM2_V1_G1DirectMissileReceipt;

/* Source: SKULLWIN/c_gui_vp.cpp:3545-3770, c_record.cpp:203-279,
 * dm2data.cpp:487.  This is selector evidence only; it never authorizes a
 * pixel decode or viewport draw. */
typedef struct {
    int valid;
    uint16_t missile_object_id;
    uint16_t missile_object;
    uint8_t class1;
    uint8_t class2;
    uint8_t record_byte4;
    uint8_t branch_temp_picst;
    uint16_t image_data_index;
    uint32_t identity_hash;
} DM2_V1_G1FlyingItemSelectorReceipt;

/* Source: SKULLWIN/c_gui_vp.cpp:3458-3770; dm2data.cpp:602,644,659,889.
 * Geometry/branch evidence only. image_field is intentionally unavailable. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t view_position;
    int8_t depth_band;
    int8_t placement_x;
    int8_t placement_y;
    uint8_t temp_picst_eligible;
    uint8_t draw_item_opaque;
    uint8_t image_field_available;
    uint32_t table_hash;
    uint32_t identity_hash;
} DM2_V1_G1FlyingItemGeometryReceipt;

typedef struct {
    uint8_t query_48ae_state;
    uint8_t timer_direction;
    uint8_t viewport_direction;
    uint8_t direction_5x5;
    /* c_gui_vp.cpp:3490-3503 reads these two bytes from the nine-byte
     * viewport table row.  Their sum controls the state-zero 8/9 split. */
    int8_t table_afe;
    int8_t table_b43;
    /* table1d6b15[view_position]; a negative value means the source view
     * cell has no flying-item geometry. */
    int8_t table_b15;
} DM2_V1_G1FlyingItemVb30Inputs;

typedef struct {
    int valid;
    uint8_t vb30;
    uint8_t temp_picst_blocked;
    uint32_t identity_hash;
} DM2_V1_G1FlyingItemVb30Receipt;

/* Source: SKULLWIN/c_gui_vp.cpp:3610-3770 and c_querydb.cpp:2381-2415.
 * This joins the TEMP_PICST summary tuple to its exact raw IMG3 and decoded
 * bytes.  It is evidence only: decoded pixels are freed before return and
 * no renderer may use this receipt as a surface. */
typedef struct {
    int valid;
    int no_draw;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint16_t raw_index;
    uint16_t width;
    uint16_t height;
    DM2_ImageFormat format;
    int16_t source_offset_x;
    int16_t source_offset_y;
    uint32_t offset_receipt_hash;
    uint32_t selector_identity_hash;
    uint32_t vb30_identity_hash;
    uint32_t geometry_identity_hash;
    uint32_t summary_receipt_hash;
    uint32_t raw_gfx256_hash;
    uint32_t raw_gfx256_receipt_hash;
    uint32_t decoded_pixels_hash;
    uint32_t palette_hash;
    uint32_t identity_hash;
} DM2_V1_G1FlyingItemDecodedMaterialReceipt;

typedef struct {
    int valid;
    uint16_t timer_index;
    uint8_t timer_type;
    uint8_t actor;
    uint16_t value;
    uint16_t action_word;
    uint8_t direction;
    uint32_t raw_timer_hash;
} DM2_V1_G1MissileTimerReceipt;

/* SKWIN/SkWinCore.cpp::QUERY_CREATURE_BLIT_RECTI. The returned id is the
 * source RAW4 rectangle index; callers add QUERY_TEMP_PICST flags separately. */
int dm2_v1_g1_query_creature_blit_recti(int cell_pos, int position_5x5,
                                        int direction,
                                        uint16_t *out_rect_id);
int dm2_v1_g1_direct_missile_timer_receipt(const uint8_t *timer_table,
                                           size_t timer_table_size,
                                           uint16_t timer_index,
                                           DM2_V1_G1MissileTimerReceipt *out);

int dm2_v1_g1_flying_item_source_receipt(
    uint16_t missile_object_id, int category, int item_type, int image_field,
    int flip_flags, int cell_pos, int position_5x5, int stretch_factor64,
    DM2_V1_G1FlyingItemSourceReceipt *out);

int dm2_v1_g1_static_object_material_selector(
    const DM2_V1_G1DirectWeaponRoot *weapon, uint16_t image_offset,
    DM2_V1_G1StaticObjectMaterialSelector *out);
int dm2_v1_g1_static_container_material_selector(
    const DM2_V1_G1DirectContainerRoot *container, uint16_t image_offset,
    DM2_V1_G1StaticObjectMaterialSelector *out);

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    int type;
    int index;
} DM2_V1_G1VerifiedRoot;

typedef struct {
    int x;
    int y;
    uint16_t object_id;
    uint16_t index;
    uint8_t direction;
    uint8_t destination_x;
    uint8_t destination_y;
    uint8_t destination_map;
    uint8_t scope;
    uint8_t sound;
    uint8_t rotation;
    uint8_t rotation_type;
} DM2_V1_G1DirectTeleporterRoot;

#define DM2_V1_G1_FIRST_MAP_MAX_ROOTS 64

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
    int teleporter_root_count;
    int teleporter_record_reads;
    int object_count;
    int blocked_record_reads;
    DM2_V1_G1VerifiedRoot roots[DM2_V1_G1_FIRST_MAP_MAX_ROOTS];
    DM2_V1_G1DirectTeleporterRoot
        teleporters[DM2_V1_G1_FIRST_MAP_MAX_ROOTS];
} DM2_V1_G1FirstMapRuntimeReceipt;

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
    int valid;
    int committed;
    int incomplete;
    int map_count;
    int square_bytes;
    int column_index_base;
    int ground_stack_base;
    int ground_stack_count;
    int text_data_base;
    int text_word_count;
    int candidate_pool_base;
    int candidate_pool_end;
    int g1_extension_base;
    int g1_extension_size;
    int raw_map_data_base;
    int record_graph_complete;
    int level_count;
    int map_root_count;
    int direct_root_count;
    int direct_root_count_by_type[16];
    int db3_root_count;
    int db4_root_count;
    int materialized_root_count;
    int blocked_root_count;
    int blocked_root_count_by_type[16];
    int blocked_root_count_by_map[DM2_V1_MAX_LEVELS];
    DM2_V1_G1BlockedRoot blocked_roots[DM2_V1_G1_PARTIAL_BOOT_MAX_BLOCKED_ROOTS];
} DM2_V1_G1PartialMapBootReceipt;

typedef struct {
    int valid;
    int committed;
    int incomplete;
    int map_count;
    int outdoor_map_count;
    int indoor_map_count;
    int square_bytes;
    int raw_map_data_base;
    int column_index_base;
    int ground_stack_base;
    int ground_stack_count;
    int text_data_base;
    int text_word_count;
    int candidate_pool_base;
    int candidate_pool_end;
    int g1_extension_base;
    int g1_extension_size;
    int record_graph_complete;
    uint32_t map_dimension_hash;
    uint32_t map_graphics_style_hash;
    uint32_t arrangement_hash;
} DM2_V1_ArrangeDungeonReceipt;

typedef struct {
    int valid;
    int level;
    int x;
    int y;
    uint16_t raw_tile;
    uint8_t tile_value;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectTileValueReceipt;

typedef struct {
    int valid;
    int level;
    int x;
    int y;
    uint16_t raw_tile;
    uint8_t tile_value;
    int is_passage;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectTilePassageReceipt;

typedef struct {
    int valid;
    int level;
    int x;
    int y;
    uint16_t raw_tile;
    uint8_t tile_value;
    int is_solid;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectTileSolidReceipt;

typedef struct {
    int valid;
    int level;
    int x;
    int y;
    uint16_t raw_tile;
    uint16_t object_id;
    int type;
    int index;
    int record_size;
    int direct_or_proven_extension_address;
    int blocked_missing_record;
    int blocked_no_tile_record_link;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectTileRecordAddressReceipt;

typedef struct {
    int valid;
    int level;
    int x;
    int y;
    uint16_t raw_tile;
    int object_index;
    int column_base_index;
    int column_index_offset;
    int object_index_offset;
    uint16_t object_id;
    int preceding_root_count;
    int blocked_no_tile_record_link;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectObjectIndexReceipt;

typedef struct {
    int valid;
    int requested_map;
    int previous_map;
    int unchanged;
    int current_map;
    int width;
    int height;
    int raw_tile_map_offset;
    int column_index_offset;
    int player_x;
    int player_y;
    int player_map;
    int player_dir;
    int blocked_negative_map;
    int blocked_map_range;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectChangeCurrentMapReceipt;

typedef struct {
    int valid;
    int level;
    int x;
    int y;
    uint16_t appended_object_id;
    uint16_t appended_previous_next;
    uint16_t parent_previous_link;
    uint16_t parent_new_link;
    uint16_t tail_object_id;
    int appended_type;
    int appended_index;
    int appended_record_size;
    int parent_link_route;
    int existing_tile_chain_route;
    int empty_tile_insert_route;
    int object_index;
    int object_index_offset;
    int shifted_ground_stack_words;
    int incremented_column_offsets;
    int blocked_null_or_end_append;
    int blocked_missing_appended_record;
    int blocked_invalid_parent;
    int blocked_invalid_tile;
    int blocked_unbounded_graph;
    int blocked_no_ground_stack_space;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectAppendRecordReceipt;

typedef struct {
    int valid;
    int level;
    int x;
    int y;
    uint16_t cut_object_id;
    uint16_t masked_object_id;
    uint16_t source_previous_next;
    uint16_t parent_previous_link;
    uint16_t parent_new_link;
    uint16_t tile_previous_root;
    uint16_t tile_new_root;
    uint16_t preceding_object_id;
    int cut_type;
    int cut_index;
    int cut_record_size;
    int object_index;
    int object_index_offset;
    int shifted_ground_stack_words;
    int decremented_column_offsets;
    int parent_link_route;
    int tile_single_root_remove_route;
    int tile_root_replace_route;
    int tile_chain_unlink_route;
    int source_reset_to_end;
    int blocked_null_or_end_cut;
    int blocked_missing_cut_record;
    int blocked_invalid_parent;
    int blocked_invalid_tile;
    int blocked_unbounded_graph;
    const char *source_symbol;
    int source_line;
} DM2_V1_SkprojectCutRecordReceipt;

typedef struct {
    int valid;
    int mode;
    int target_low_byte;
    int countdown_start;
    int countdown_remaining;
    int maps_scanned;
    int squares_scanned;
    int root_tiles_scanned;
    int text_records_scanned;
    int link_word_reads;
    int matched_ext_usage_0b;
    int matched_ext_usage_10;
    int cleared_ext_usage_0f_visibility;
    int result_map;
    int result_x;
    int result_y;
    int return_value;
    int blocked_incomplete_record_graph;
    int blocked_missing_record;
    const char *source_symbol;
    int source_line;
} DM2_V1_Skproject3D93BReceipt;

typedef struct DM2_V1_DungeonData {
    int level_count;
    DM2_LevelType level_types[DM2_V1_MAX_LEVELS];
    int level_widths[DM2_V1_MAX_LEVELS];
    int level_heights[DM2_V1_MAX_LEVELS];
    int level_offsets[DM2_V1_MAX_LEVELS];
    int map_offset_x[DM2_V1_MAX_LEVELS];
    int map_offset_y[DM2_V1_MAX_LEVELS];
    int map_door_set0[DM2_V1_MAX_LEVELS];
    int map_door_set1[DM2_V1_MAX_LEVELS];
    int map_use_door0[DM2_V1_MAX_LEVELS];
    int map_use_door1[DM2_V1_MAX_LEVELS];
    int map_door_ornate_count[DM2_V1_MAX_LEVELS];
    int initial_party_pose_valid;
    int initial_party_x;
    int initial_party_y;
    int initial_party_dir;
    int square_bytes;
    int raw_map_data_base;
    int column_index_base;
    int square_first_thing_base;
    int square_first_thing_count;
    int text_data_base;
    int text_word_count;
    int g1_extension_base;
    int g1_extension_size;
    int g1_extension_record_bases[16];
    int g1_extension_record_counts[16];
    int thing_data_bases[16];
    int thing_type_counts[16];
    /* Set only when the source layout has materialized every map-to-record
     * ownership table.  A byte-square map alone is not a playable graph. */
    int record_graph_complete;
    int g1_w0_chains_disabled;
    DM2_V1_G1PartialMapBootReceipt partial_map_boot;
    uint8_t *raw_data;
    int raw_size;
    /* DM2 outdoor extension */
    int sky_texture_index;
    int weather_zone_count;
} DM2_V1_DungeonData;

int dm2_v1_g1_direct_missile_receipt(const DM2_V1_DungeonData *d,
                                     uint16_t object_id,
                                     DM2_V1_G1DirectMissileReceipt *out);
int dm2_v1_g1_flying_item_selector_receipt(
    const DM2_V1_DungeonData *d, const DM2_V1_AssetLoader *loader,
    const DM2_V1_G1DirectMissileReceipt *missile,
    DM2_V1_G1FlyingItemSelectorReceipt *out);
int dm2_v1_g1_flying_item_geometry_receipt(
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    int view_position, DM2_V1_G1FlyingItemGeometryReceipt *out);
int dm2_v1_g1_flying_item_vb30_receipt(
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Inputs *inputs,
    DM2_V1_G1FlyingItemVb30Receipt *out);
int dm2_v1_g1_flying_item_summary_image_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    DM2_V1_QueryGdatSummaryImageReceipt *out);
int dm2_v1_g1_flying_item_decoded_material_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_G1FlyingItemSelectorReceipt *selector,
    const DM2_V1_G1FlyingItemVb30Receipt *vb30,
    const DM2_V1_G1FlyingItemGeometryReceipt *geometry,
    DM2_V1_G1FlyingItemDecodedMaterialReceipt *out);

typedef int (*DM2_V1_DungeonThingVisitor)(
    void *user,
    uint16_t thing,
    int type,
    int index,
    const uint8_t *record,
    int record_size,
    int level,
    int x,
    int y);

/* SKProject DME.h::Map_definitions::Difficulty() is the high nibble of w12.
 * c_light.cpp selects its fixed-light branch only for difficulty zero. Keep
 * this raw map-descriptor proof separate from the later live light
 * accumulator and modifier state. */
typedef struct {
    int valid;
    int level;
    uint8_t difficulty;
    uint8_t dynamic_light;
    uint32_t descriptor_hash;
} DM2_V1_CLightMapDescriptorReceipt;

int dm2_v1_dungeon_load(DM2_V1_DungeonData *out, const uint8_t *dat, int size);
int dm2_v1_dungeon_get_square_type(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_get_tile_raw(const DM2_V1_DungeonData *d, int level, int x, int y);
uint32_t dm2_v1_skproject_tile_to_ulong(uint16_t tile);
uint8_t dm2_v1_skproject_tile_to_ubyte(uint16_t tile);
uint16_t dm2_v1_skproject_mk_record(int16_t record);
int16_t dm2_v1_skproject_record_to_word(uint16_t record);
int32_t dm2_v1_skproject_record_to_long(uint16_t record);
int dm2_v1_dungeon_c_map_get_tile_value(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_c_map_is_tile_passage(const DM2_V1_DungeonData *d, int level, int x, int y);
int dm2_v1_dungeon_c_map_is_tile_solid(const DM2_V1_DungeonData *d, int level, int x, int y);
const uint8_t *dm2_v1_dungeon_c_map_get_address_of_tile_record(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    uint16_t *out_object_id,
    int *out_record_offset);
int dm2_v1_dungeon_set_tile_raw(DM2_V1_DungeonData *d, int level, int x, int y, uint16_t raw);
int dm2_v1_dungeon_get_first_thing(const DM2_V1_DungeonData *d, int level, int x, int y);
/* Mutable counterpart of dm2_v1_dungeon_get_first_thing: write the
 * ground-stack head word for a byte-square cell that carries the 0x10
 * object flag (the tile-rooted list mutation the source performs in
 * DM2_CUT_RECORD_FROM/DM2_MOVE_RECORD_TO).  Same index computation and
 * bounds discipline as the getter; returns 0 on success, -1 fail-closed
 * (absent data, out-of-bounds cell, no object flag, word-square maps
 * whose head is inline and has no table slot). */
int dm2_v1_dungeon_set_first_thing(DM2_V1_DungeonData *d, int level, int x, int y, uint16_t first);
int dm2_v1_skproject_get_tile_value(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTileValueReceipt *out);
int dm2_v1_skproject_is_tile_passage(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTilePassageReceipt *out);
int dm2_v1_skproject_is_tile_solid(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTileSolidReceipt *out);
int dm2_v1_skproject_get_address_of_tile_record(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectTileRecordAddressReceipt *out);
int dm2_v1_skproject_get_object_index_from_tile(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectObjectIndexReceipt *out);
int dm2_v1_skproject_change_current_map_to(
    const DM2_V1_DungeonData *d,
    int previous_map,
    int new_map,
    int player_x,
    int player_y,
    int player_map,
    int player_dir,
    DM2_V1_SkprojectChangeCurrentMapReceipt *out);
int dm2_v1_skproject_append_record_to(
    DM2_V1_DungeonData *d,
    uint16_t record_to_append,
    uint16_t *parent_link,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectAppendRecordReceipt *out);
int dm2_v1_skproject_cut_record_from(
    DM2_V1_DungeonData *d,
    uint16_t record_to_cut,
    uint16_t *parent_link,
    int level,
    int x,
    int y,
    DM2_V1_SkprojectCutRecordReceipt *out);
int dm2_v1_skproject_3d93b_text_scan(
    DM2_V1_DungeonData *d,
    int mode,
    int countdown,
    int target_low_byte,
    int *out_y,
    int *out_x,
    DM2_V1_Skproject3D93BReceipt *out);
int dm2_v1_dungeon_get_next_thing(const DM2_V1_DungeonData *d, uint16_t thing);
int dm2_v1_dungeon_walk_square_things(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    int max_steps,
    DM2_V1_DungeonThingVisitor visitor,
    void *user);
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
/* SKProject DME.h::Map_definitions::FloorGraphics() is w10 bits 8..11.
 * LOAD_LOCALLEVEL_DYN reads the corresponding map-local byte list directly
 * after CreaturesTypes and WallGraphics. */
int dm2_v1_dungeon_get_map_floor_gfx_list(
    const DM2_V1_DungeonData *d,
    int level,
    uint8_t *out_floor_gfx_list,
    int out_capacity);
int dm2_v1_dungeon_get_map_door_ornate_list(
    const DM2_V1_DungeonData *d,
    int level,
    uint8_t *out_door_ornate_list,
    int out_capacity);
int dm2_v1_dungeon_get_map_graphics_style(
    const DM2_V1_DungeonData *d,
    int level);
int dm2_v1_dungeon_c_light_map_descriptor_receipt(
    const DM2_V1_DungeonData *d, int level,
    DM2_V1_CLightMapDescriptorReceipt *out);

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
int dm2_v1_dungeon_validate_record_pools(const DM2_V1_DungeonData *d);
/* Source-named DM2_ARRANGE_DUNGEON receipt.  This does not invent records or
 * complete the PC G1 graph; it only admits the arranged map/dungeon layout
 * already proven by dm2_v1_dungeon_load. */
int dm2_v1_DM2_ARRANGE_DUNGEON_receipt(
    const uint8_t *dat,
    int size,
    DM2_V1_ArrangeDungeonReceipt *out);
/* Collect non-mutating PC G1 c_record provenance for the source-ordered pool
 * span. Record lookup/traversal is available only when record_graph_complete
 * is set by the independent bounded graph validator. */
int dm2_v1_dungeon_collect_g1_record_pool_evidence(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1RecordPoolEvidence *out);
/* Retain hashes and counts for the verified raw G1 c_map table/map ranges.
 * A non-G1 or truncated layout returns a receipt with g1_layout_absent set;
 * it never attempts an alternate table decode. */
int dm2_v1_dungeon_collect_g1_ground_stack_map_corpus_receipt(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1GroundStackMapCorpusReceipt *out);

int dm2_v1_dungeon_collect_g1_map_corpus_receipt(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1MapCorpusReceipt *out);
/* Transactionally scans the PC G1 map roots and commits a partial-map boot
 * receipt only when every root is either direct, in the proven DB3/DB4
 * continuation, or one of the five explicitly blocked DB8/DB10 roots.
 * The result remains incomplete and does not authorize record traversal. */
int dm2_v1_dungeon_materialize_g1_partial_map_boot(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1PartialMapBootReceipt *out);
/* Validate a selected G1 runtime map against the transactional partial boot.
 * It reads only c_map's column/ground-stack ObjectID roots and classifies
 * their already-proven c_record addresses; it never reads GenericRecord::w0.
 * The output is unchanged when the map cannot be source-validated. */
int dm2_v1_dungeon_validate_g1_runtime_map(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapValidationReceipt *out);
/* Resolve a selected tile through c_map.cpp's ground-stack lookup to a
 * declared direct DB0..DB5/DB9 record address. Other types, extensions, and
 * tiles without a root fail closed without mutating out. */
int dm2_v1_dungeon_resolve_g1_direct_root_record(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_G1DirectRootRecordAddressReceipt *out);
/* Follow a selected tile's local chain only while each node remains a
 * declared direct DB0..DB5/DB9 record and terminates at OBJECT_END_MARKER.
 * Unknown types, extensions, loops, oversized paths, and malformed bounds
 * fail closed without mutating out. */
int dm2_v1_dungeon_collect_g1_direct_root_chain(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_G1DirectRootChainReceipt *out);
/* Bind a bounded direct G1 chain to the source-defined wall/floor/door tile
 * classes and generic/door/creature root classes. This is an address-only
 * runtime admission receipt: no DB payload is decoded. Unsupported terrain,
 * malformed chains, loops, and non-direct roots fail closed. */
int dm2_v1_dungeon_classify_g1_direct_root_scene(
    const DM2_V1_DungeonData *d,
    int level,
    int x,
    int y,
    DM2_V1_G1DungeonSceneClassificationReceipt *out);
/* Materialize the source-addressed map/object identity of an original raw
 * SKSave map. This is deliberately separate from PC G1 partial-world
 * admission: a raw save has no untyped extension. Any marked square without
 * a complete c_map -> c_record address rejects the entire receipt. */
int dm2_v1_dungeon_collect_raw_sksave_map_scene(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_RawSKSaveMapSceneReceipt *out);
/* Consume only direct DB0 roots on a runtime-admitted G1 map.  The payload is
 * limited to DME.h::Door w2; it never reads GenericRecord::w0 or follows a
 * map/record link.  The output is unchanged when any source gate fails. */
int dm2_v1_dungeon_materialize_g1_runtime_map_doors(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapDoorReceipt *out);
/* Finds one previously materialized direct DB0 root.  This does not reopen
 * the map or traverse GenericRecord::w0, so a runtime caller cannot turn an
 * incomplete G1 record graph into a guessed door chain. */
int dm2_v1_g1_runtime_map_door_at(
    const DM2_V1_G1RuntimeMapDoorReceipt *receipt,
    int x,
    int y,
    const DM2_V1_G1DirectDoorRoot **out_door);
/* Consume only declared direct DB3 roots on a runtime-admitted G1 map. It
 * reads the source-defined Actuator w2/w4/w6 fields and never GenericRecord::
 * w0, an extension DB3 record, or an unvalidated map/record route. */
int dm2_v1_dungeon_materialize_g1_runtime_map_actuators(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapActuatorReceipt *out);
/* Enumerate only real G1 champion-mirror marker roots (DB3 Actuator type
 * 0x7e).  c_hero.cpp::DM2_SELECT_CHAMPION reads the same w2 fields after locating
 * the tile record.  The receipt remains fail-closed if c_map cannot resolve
 * a root through either the declared or proven DB3 continuation pool. */
int dm2_v1_dungeon_collect_g1_champion_mirrors(
    const DM2_V1_DungeonData *d,
    DM2_V1_G1ChampionMirrorReceipt *out);
/* Consume only declared direct DB4 roots on a runtime-admitted G1 map. It
 * reads the source-defined Creature b4 and w6 fields, never the w0 next link,
 * the w2 possession ObjectID, extension DB4 records, or an unvalidated map. */
int dm2_v1_dungeon_materialize_g1_runtime_map_creatures(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapCreatureReceipt *out);
/* Consume only declared direct DB5 roots on a runtime-admitted G1 map. It
 * reads source-defined Weapon::w2 fields and never the w0 next link, an
 * unvalidated map, or any inferred object traversal. */
int dm2_v1_dungeon_materialize_g1_runtime_map_weapons(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapWeaponReceipt *out);
/* Consume only declared direct DB9 roots on a runtime-admitted G1 map. It
 * reads source-defined Container::b4 fields and never the w0 next link, w2
 * contained-object ObjectID, or an inferred object traversal. */
int dm2_v1_dungeon_materialize_g1_runtime_map_containers(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1RuntimeMapContainerReceipt *out);
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

/* Decode only visible, direct map-5 TextMode==0 dunTextData messages using
 * skproject SKWIN/SkWinCore.cpp QUERY_MESSAGE_TEXT.  The input must be the
 * committed map-5 receipt; unknown phrase-bank escapes fail closed per
 * message. */
int dm2_v1_dungeon_materialize_g1_map5_text_messages(
    const DM2_V1_DungeonData *d,
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1TextMessageRuntimeReceipt *out);

/* Raw-only G1 mode-one message route. It admits only the source's extension
 * usage 14 and MESSAGES/0/dtText low-byte lookup; any missing text blocks the
 * entire receipt rather than rendering a fabricated string. */
int dm2_v1_dungeon_materialize_g1_map5_gdat_text_messages(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatTextRead read_text,
    void *read_userdata,
    DM2_V1_G1GdatTextMessageRuntimeReceipt *out);

/* Consumes only direct DB2 TextMode()==1 roots already materialized by the
 * G1 map-5 receipt, which must report zero generic or blocked record reads.
 * skproject QUERY_CLS2_OF_TEXT_RECORD maps their low TextIndex byte to
 * WALL_GFX; DRAW_WALL_ORNATE consumes the exact five GDAT fields recorded
 * here. Missing or malformed source material rejects all. */
int dm2_v1_dungeon_materialize_g1_text_wall_gfx_runtime(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out);

/* The boot-owned GDAT route uses this stronger variant before allowing an
 * original-data frame to consume a DB2 custom wall button. It probes the
 * exact WALL_GFX dtImage/1 front bitmap and retains dimensions/source format
 * when it decodes; an unavailable bitmap remains explicitly non-drawable. */
int dm2_v1_dungeon_materialize_g1_text_wall_gfx_image_runtime(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatScalarRead read_scalar,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    void *read_userdata,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out);

/* Stronger DRAW_WALL_ORNATE/DRAW_DEFAULT_DOOR_BUTTON material boundary. The
 * source renderer obtains the image and its exact QUERY_GDAT_IMAGE_LOCALPAL
 * result as one IMG3 operation; callers using this entry must not draw an
 * ornate/button from metadata alone. */
int dm2_v1_dungeon_materialize_g1_text_wall_gfx_image_material_runtime(
    const DM2_V1_G1Map5TextRuntimeReceipt *texts,
    DM2_V1_G1GdatScalarRead read_scalar,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1TextWallGfxRuntimeReceipt *out);

/* Direct DB3 roots only after the G1 record graph is complete. c_record
 * resolves the address; skproject Actuator::GraphicNumber() maps through the
 * current map's one-based WallGraphics list before DRAW_WALL_ORNATE consumes
 * the GDAT fields. */
int dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatScalarRead read_scalar,
    void *read_userdata,
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt *out);

/* Same source-owned image/palette binding for direct DB3 actuator graphics. */
int dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_image_material_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatScalarRead read_scalar,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
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

/* Direct or source-proven extension DB4 roots only after the G1 record graph
 * is complete. It binds Creature::CreatureType() to CREATURES/type dtImage/F9
 * raw ownership, not sprite decoding, animation selection, palette
 * conversion, or drawing. */
int dm2_v1_dungeon_materialize_g1_creature_map_chip_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatRawRead read_raw,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1CreatureMapChipRuntimeReceipt *out);

/* Direct DB5 roots use Weapon::ItemType() exactly as DRAW_MAP_CHIP does:
 * WEAPONS/itemType/dtImage/F9 plus that image's local palette.  The receipt
 * admits no inferred record chain and is invalid as a whole on missing or
 * malformed original GDAT material. */
int dm2_v1_dungeon_materialize_g1_weapon_map_chip_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatRawRead read_raw,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1WeaponMapChipRuntimeReceipt *out);
int dm2_v1_dungeon_materialize_g1_container_map_chip_runtime(
    const DM2_V1_DungeonData *d,
    int map,
    DM2_V1_G1GdatRawRead read_raw,
    DM2_V1_G1GdatImageMetadataRead read_image_metadata,
    DM2_V1_G1GdatImageLocalPaletteRead read_local_palette,
    void *read_userdata,
    DM2_V1_G1ContainerMapChipRuntimeReceipt *out);

/* Used by the dungeon viewport after the real F9 fetch/decode. A DB4
 * creature is drawable only when its decoded surface matches the source
 * metadata receipt for that creature type. */
int dm2_v1_g1_creature_map_chip_matches_decoded_material(
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt,
    int creature_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash);
/* A live DB4 draw additionally has to retain its exact source object, tile,
 * and b15_0_1 direction owner. Matching a same-type creature elsewhere in
 * the corpus, or replaying its material with a different record direction,
 * is not sufficient authorization for a viewport sprite. */
int dm2_v1_g1_creature_map_chip_matches_decoded_instance(
    const DM2_V1_G1CreatureMapChipRuntimeReceipt *receipt,
    uint16_t object_id,
    int x,
    int y,
    int direction,
    int creature_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash);
int dm2_v1_g1_weapon_map_chip_matches_decoded_instance(
    const DM2_V1_G1WeaponMapChipRuntimeReceipt *receipt,
    uint16_t object_id,
    int x,
    int y,
    int item_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash,
    uint32_t decoded_pixel_hash);
int dm2_v1_g1_container_map_chip_matches_decoded_instance(
    const DM2_V1_G1ContainerMapChipRuntimeReceipt *receipt,
    uint16_t object_id,
    int x,
    int y,
    int container_type,
    int image_width,
    int image_height,
    uint32_t local_palette_hash,
    uint32_t decoded_pixel_hash);
void dm2_v1_dungeon_free(DM2_V1_DungeonData *d);
const char *dm2_v1_dungeon_source_evidence(void);
const char *dm2_v1_DM2_ARRANGE_DUNGEON_source_evidence(void);

/* Return a pointer to the raw tile data for a given level, plus its
 * width and height.  Returns NULL if the level is out of range or the
 * dungeon has no raw data.  The pointer points into d->raw_data.
 * Used by GET_TELEPORTER_DETAIL and other cross-map queries. */
const uint8_t *dm2_v1_dungeon_level_tile_data(
    const DM2_V1_DungeonData *d,
    int level,
    int16_t *out_width,
    int16_t *out_height);

#endif
