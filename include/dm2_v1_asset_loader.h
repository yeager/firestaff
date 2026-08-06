#ifndef FIRESTAFF_DM2_V1_ASSET_LOADER_H
#define FIRESTAFF_DM2_V1_ASSET_LOADER_H
/*
 * dm2_v1_asset_loader.h — DM2 V1 Graphics Asset Loader
 *
 * DM2 Phase 2: DM2-specific GRAPHICS.DAT loading.
 *
 * DM2 extends the GRAPHICS.DAT format from DM1:
 *   - 240 GDAT categories (vs DM1's 29)
 *   - ~8.6 MB (vs DM1's ~363 KB) — 24x larger
 *   - New outdoor environment art (sky, ground, trees, buildings)
 *   - Extended creature AI (64 vs 42 types) with new sprites
 *   - DM2-specific UI (champion sheets, shops, compass, depth counter)
 *   - Day/night palette variants with runtime palette switching
 *   - 640x400 mode for outdoor levels (not in DM1)
 *
 * DM2 still uses:
 *   - IMG3 (4-bit nibble encoding) for simple textures
 *   - IMG3 overlay for doors/panels
 *   - IMG9 (9-bit per-pixel) for complex walls
 *   - 8-bit palette-indexed core (c_pixel256) with 16-bit overlay passes
 *
 * Source: docs/dm2_v1_phase2_data_formats_H2254.md §3 — GRAPHICS.DAT structure
 * Source: docs/dm2_graphics.md — GDAT categories, image formats, palette system
 * Source: docs/dm2_platform_data.md — DM2 GRAPHICS.DAT size (~8.6 MB)
 * Source: SKULL.ASM T560 — dungeon viewport rendering (indoor)
 * Source: SKULL.ASM T600 — outdoor viewport rendering (sky/terrain)
 * Source: SKULL.ASM — GDAT image decoding (decode_img3_underlay/overlay, decode_img9)
 */

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── GDAT Category Constants (DM2-extended) ─────────────────────── */
/*
 * DM2 extends categories from 29 (0x1D) to 240 (0xF0).
 * Source: SkGlobal.h:636 — GDAT_CATEGORY_LIMIT (DM2) vs (DM1)
 * Source: docs/dm2_v1_phase2_data_formats_H2254.md §1
 */
#define DM2_GDAT_CATEGORY_LIMIT   0xF0   /* skproject extended category limit */
#define DM1_GDAT_CATEGORY_LIMIT   0x1D   /* 29 categories */

typedef enum {
    DM2_GDAT_CATEGORY_TECHDATA           = 0x00,
    DM2_GDAT_CATEGORY_INTERFACE_GENERAL  = 0x01,
    DM2_GDAT_CATEGORY_SPELL_DEF          = 0x02, /* Custom spells (up to 255 vs DM1's 34) */
    DM2_GDAT_CATEGORY_MESSAGES           = 0x03,
    DM2_GDAT_CATEGORY_MUSICS             = 0x04,
    DM2_GDAT_CATEGORY_TITLE              = 0x05,
    DM2_GDAT_CATEGORY_CREDITS            = 0x06,
    DM2_GDAT_CATEGORY_INTERFACE_CHARSHEET = 0x07,
    DM2_GDAT_CATEGORY_GRAPHICSSET        = 0x08,
    DM2_GDAT_CATEGORY_WALL_GFX           = 0x09,
    DM2_GDAT_CATEGORY_FLOOR_GFX          = 0x0A,
    DM2_GDAT_CATEGORY_DOOR_GFX           = 0x0B,
    DM2_GDAT_CATEGORY_DOOR_BUTTONS       = 0x0C,
    DM2_GDAT_CATEGORY_SPELL_MISSILES     = 0x0D,
    DM2_GDAT_CATEGORY_DOORS              = 0x0E, /* Door properties (strength, color keys) */
    DM2_GDAT_CATEGORY_CREATURES          = 0x0F, /* 64 AI creature types */
    DM2_GDAT_CATEGORY_WEAPONS            = 0x10, /* Extended weapons (projectile flags) */
    DM2_GDAT_CATEGORY_CLOTHES            = 0x11, /* Clothing/armor sprites */
    DM2_GDAT_CATEGORY_SCROLLS            = 0x12,
    DM2_GDAT_CATEGORY_POTIONS            = 0x13,
    DM2_GDAT_CATEGORY_CONTAINERS         = 0x14,
    DM2_GDAT_CATEGORY_MISCELLANEOUS      = 0x15,
    DM2_GDAT_CATEGORY_CHAMPIONS          = 0x16, /* Champion NPC data (sounds) */
    DM2_GDAT_CATEGORY_ENVIRONMENT        = 0x17, /* Outdoor assets (sky, ground, trees) */
    DM2_GDAT_CATEGORY_TELEPORTERS        = 0x18, /* Teleporter square type */
    DM2_GDAT_CATEGORY_CREATURE_AI        = 0x19, /* Per-creature AI behaviors */
    DM2_GDAT_CATEGORY_DIALOG_BOXES       = 0x1A, /* Dialog box graphics */
    DM2_GDAT_CATEGORY_JAPANESE_FONT      = 0x1C,
} DM2_GDAT_Category;

typedef enum {
    DM2_GDAT_ENTRY_TYPE_IMAGE        = 0x01,
    DM2_GDAT_ENTRY_TYPE_SOUND        = 0x02,
    /* skproject SKWIN/defines.h: dtHMP.  PC music is an original GDAT
     * payload in GRAPHICS.DAT, not a host-side replacement file. */
    DM2_GDAT_ENTRY_TYPE_HMP          = 0x03,
    DM2_GDAT_ENTRY_TYPE_RAW4         = 0x04,
    /* skproject SKWIN/DME.h dtText.  QUERY_GDAT_TEXT selects this exact
     * type; it must not be confused with a drawable environment image. */
    DM2_GDAT_ENTRY_TYPE_TEXT         = 0x05,
    DM2_GDAT_ENTRY_TYPE_RAW6         = 0x06,
    DM2_GDAT_ENTRY_TYPE_RAW7         = 0x07,
    DM2_GDAT_ENTRY_TYPE_RAW8         = 0x08,
    DM2_GDAT_ENTRY_TYPE_PAL_IRGB     = 0x09,
    DM2_GDAT_ENTRY_TYPE_WORD_VALUE   = 0x0B,
    DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET = 0x0C,
    DM2_GDAT_ENTRY_TYPE_PAL_16       = 0x0D,
} DM2_GDAT_EntryType;

/* Viewport image fields used by the boot/runtime GDAT bridge.
 * skproject/SKWIN/defines.h GRAPHICSSET and map-chip field identifiers. */
#define DM2_GDAT_GFXSET_FLOOR 0x00
#define DM2_GDAT_GFXSET_CEIL  0x01
#define DM2_GDAT_GFXSET_DOOR_FRAME_FRONT_D1 0x06
#define DM2_GDAT_GFXSET_DOOR_FRAME_D1C      0x07
#define DM2_GDAT_GFXSET_DOOR_FRAME_D2C      0x09
#define DM2_GDAT_IMG_MAP_CHIP 0xF9

#define DM2_GDAT_GFXSET_SCENE_COLORKEY       0x64
#define DM2_GDAT_GFXSET_SCENE_FLAGS          0x65
#define DM2_GDAT_GFXSET_AMBIANT_LIGHT        0x66
#define DM2_GDAT_GFXSET_SCENE_RAIN           0x67
#define DM2_GDAT_GFXSET_HIGHEST_LIGHT_LEVEL  0x68
#define DM2_GDAT_GFXSET_MISTY_MAP            0x69
#define DM2_GDAT_GFXSET_VOID_RANDOM_FALL     0x6A
#define DM2_GDAT_GFXSET_ANIMATED_FLOOR       0x6B
#define DM2_GDAT_GFXSET_THUNDER_POSITION     0x6C
#define DM2_GDAT_GFXSET_AMBIANT_DARKNESS     0x6D
#define DM2_GDAT_GFXSET_TRIM_WALL_D1         0x70
#define DM2_GDAT_GFXSET_TRIM_WALL_D2         0x71

/* skproject/SKWIN/SkWinCore.cpp startup/HUD interface raw data:
 * LOAD_GDAT_INTERFACE_00_0A, LOAD_GDAT_INTERFACE_00_02,
 * LOAD_GDAT_ENTRY_DATA_TO(0x1,0x0,dt07,0x0), dtPalIRGB, dtPalette16. */
#define DM2_GDAT_INTERFACE_RAW_LAYOUT_TABLE   0x00
#define DM2_GDAT_INTERFACE_RAW_ACTION_TABLE   0x02
#define DM2_GDAT_INTERFACE_RAW_RECT14_TABLE   0x0A
#define DM2_GDAT_INTERFACE_PALETTE_FIELD      0xFE

/* skproject/SKWIN/SkWinCore.cpp GET_CREATURE_ANIMATION_FRAME consumes the
 * creature action/sequence tables; SKWINSPX/src/v4/skcrture.cpp names the
 * real V5 GDAT records as category 0x0f dtRaw8/0xfb and dtRaw7/0xfc/0xfd. */
#define DM2_GDAT_CREATURE_ANIM_ATTRIBUTION     0xFB
#define DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE   0xFC
#define DM2_GDAT_CREATURE_ANIM_FRAME_SEQUENCE  0xFD

/* ── Image Compression Types ─────────────────────────────────────── */
typedef enum {
    DM2_IMG_FMT_UNKNOWN     = 0,
    DM2_IMG_FMT_IMG3        = 3,   /* 4-bit nibble encoding (simple textures) */
    DM2_IMG_FMT_U4          = 4,   /* unpacked 4-bit source, returned as 8-bit indices */
    DM2_IMG_FMT_U8          = 8,   /* uncompressed 8-bit palette indices */
    DM2_IMG_FMT_IMG9        = 9,   /* 9-bit per-pixel (complex walls) */
} DM2_ImageFormat;

/* ── Asset Loader Context ─────────────────────────────────────────── */

typedef struct DM2_V1_AssetLoader {
    uint8_t cls1;
    uint8_t cls2;
    uint8_t cls3;
    uint8_t cls4;
    uint8_t cls5;
    uint8_t cls6;
    uint16_t data_index;
} DM2_V1_GdatEntry;

typedef struct {
    const uint8_t *data;     /* GRAPHICS.DAT data (owned or ref) */
    size_t         data_size; /* GRAPHICS.DAT size */
    int            category_count;
    int            loaded;    /* 1 if successfully loaded */
    int            big_endian; /* 1 if Mac/Amiga 68k BE format */
    uint32_t       md5_hash;  /* Low 32 bits of MD5 (for verification) */
    uint16_t       gdat_version;
    uint16_t       raw_data_count;
    uint32_t      *raw_offsets;
    uint32_t      *raw_sizes;
    DM2_V1_GdatEntry *entries;
    uint16_t       entry_count;
    uint16_t       category_entry_counts[DM2_GDAT_CATEGORY_LIMIT + 1];
    uint8_t        ent1_ep_present[7];
    uint8_t        ent1_ep_lengths[7];
    uint16_t       ent1_entry_stride;
} DM2_V1_AssetLoader;

#ifndef DM2_V1_GDAT_IMAGE_METADATA_DEFINED
typedef struct {
    uint16_t width;
    uint16_t height;
    uint8_t bits_per_pixel;
    int16_t query_offset_x;
    int16_t query_offset_y;
    int graphicsset_offset_present;
    int image_offset_present;
    uint32_t metadata_hash;
} DM2_V1_GdatImageMetadata;
#define DM2_V1_GDAT_IMAGE_METADATA_DEFINED 1
#endif

/* Main 256-colour palette and the 16-colour logical-index table loaded by
 * SkWinCore::INIT before the HUD and dungeon viewport are drawn.  RGB values
 * are VGA 6-bit components, matching SET_GRAPHICS_RGB_PALETTE's >> 2 step. */
typedef struct {
    uint8_t rgb6[256][3];
    uint8_t palette16[16];
    uint32_t hash;
} DM2_V1_InterfacePalette;


typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t mode;
    uint16_t selected_raw_index;
    uint16_t width;
    uint16_t height;
    uint16_t data_index;
    uint32_t image_hash;
    uint32_t receipt_hash;
} DM2_V1_QueryPicstImageReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t gdat_bypassed_for_ff;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t colors;
    uint8_t palette16[16];
    DM2_V1_GdatImageMetadata metadata;
    uint16_t data_index;
    uint16_t graphicsset_offset_word;
    uint16_t image_offset_word;
    uint32_t offset_receipt_hash;
    uint32_t palette_hash;
    uint32_t receipt_hash;
} DM2_V1_QueryGdatSummaryImageReceipt;

typedef struct {
    uint8_t present;
    uint8_t loadable_raw;
    uint8_t copied_to_destination;
    uint8_t category;
    uint8_t index;
    uint8_t type;
    uint8_t field;
    uint16_t data_index;
    uint16_t raw_index;
    uint32_t raw_file_pos;
    uint32_t raw_length;
    uint32_t copied_length;
    uint32_t receipt_hash;
} DM2_V1_GdatEntryQueryReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t type;
    uint8_t field;
    uint16_t raw_index;
    uint16_t data_index;
    uint32_t raw_length;
    uint32_t raw_hash;
    uint32_t receipt_hash;
} DM2_V1_DirectGdatEntryDataBuffReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint16_t raw_index;
    uint16_t data_index;
    uint32_t text_length;
    uint32_t text_hash;
    uint32_t receipt_hash;
} DM2_V1_DirectGdatTextReceipt;

typedef enum {
    DM2_V1_GDAT_PICT_POOL_FREE = 0,
    DM2_V1_GDAT_PICT_POOL_LOBIG = 1,
    DM2_V1_GDAT_PICT_POOL_HIBIG = 2,
    DM2_V1_GDAT_PICT_POOL_CPXHEAP = 3,
} DM2_V1_GdatPictPool;

typedef struct {
    uint8_t accepted;
    uint8_t is_cpx_heap;
    uint8_t bpp;
    uint8_t pool;
    uint16_t raw_index;
    uint16_t width;
    uint16_t height;
    uint16_t row_bytes;
    uint32_t payload_bytes;
    uint32_t header_bytes;
    uint32_t allocation_bytes;
    uint32_t free_bytes;
    uint32_t receipt_hash;
} DM2_V1_GdatPictAllocationReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t used_bigpool_struct_before;
    uint8_t freed_pool;
    uint8_t removed_from_preserved_list;
    uint16_t width;
    uint16_t height;
    uint16_t row_bytes;
    uint32_t free_bytes;
    uint32_t receipt_hash;
} DM2_V1_GdatPictFreeReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t pool;
    uint8_t clean;
    uint8_t deallocate;
    uint32_t requested_bytes;
    uint32_t aligned_bytes;
    uint32_t receipt_hash;
} DM2_V1_GdatBigpoolMemoryReceipt;

#define DM2_V1_GDAT_CPX_COMPACT_MAX_BLOCKS 16

typedef struct {
    uint8_t accepted;
    uint16_t old_wp08_word;
    uint16_t requested_bytes;
    uint16_t reserved_words;
    uint16_t new_wp08_word;
    uint16_t returned_word;
    uint32_t receipt_hash;
} DM2_V1_GdatCpxReserveReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t source_header_included;
    uint16_t copied_bytes;
    uint16_t returned_payload_word;
    DM2_V1_GdatCpxReserveReceipt reserve;
    uint32_t receipt_hash;
} DM2_V1_GdatCpxCopyReceipt;

typedef struct {
    uint16_t raw_index;
    uint16_t raw_length;
    uint16_t old_start_word;
    uint8_t marked_free;
} DM2_V1_GdatCpxBlockInput;

typedef struct {
    uint8_t preserved;
    uint8_t skipped_free;
    uint16_t raw_index;
    uint16_t raw_length;
    uint16_t word_count;
    uint16_t old_start_word;
    uint16_t new_start_word;
} DM2_V1_GdatCpxCompactBlockReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t empty_pool;
    uint16_t old_wp08_word;
    uint16_t new_wp08_word;
    uint16_t input_block_count;
    uint16_t preserved_block_count;
    uint16_t skipped_free_block_count;
    uint16_t moved_block_count;
    DM2_V1_GdatCpxCompactBlockReceipt
        blocks[DM2_V1_GDAT_CPX_COMPACT_MAX_BLOCKS];
    uint32_t receipt_hash;
} DM2_V1_GdatCpxCompactReceipt;

typedef struct {
    uint8_t accepted;
    uint16_t sound_entry_count;
    uint16_t unique_raw_index_count;
    uint16_t max_raw_length;
    uint32_t scratch_allocation_bytes;
    uint32_t receipt_hash;
} DM2_V1_DballocSoundCensusReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t allowed;
    uint8_t cls5_mask;
    uint8_t active_mask;
    uint16_t entry_ordinal;
    uint32_t receipt_hash;
} DM2_V1_DballocEntryFilterReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t requested_sound_cleanup;
    uint8_t early_dealloc_when_locked;
    uint16_t entry_count;
    uint16_t descriptor_count;
    uint32_t marker_allocation_bytes;
    uint32_t receipt_hash;
} DM2_V1_LoadDyn4AdmissionReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t type_filter;
    uint8_t field_filter;
    uint16_t scanned_entry_count;
    uint16_t max_raw_length;
    uint32_t receipt_hash;
} DM2_V1_GdatMaxRawLengthReceipt;

typedef struct {
    uint8_t valid;
    uint8_t endian_swapped;
    uint8_t group_count;
    uint16_t entry_stride;
    uint16_t entry_count;
    uint32_t raw0_length;
    uint8_t ep_present[7];
    uint8_t ep_lengths[7];
    uint16_t ep_offsets[7];
    uint32_t receipt_hash;
} DM2_V1_GdatEnt1Receipt;

typedef struct {
    uint8_t valid;
    uint16_t entry_count;
    uint16_t loadable_entry_count;
    uint16_t scalar_entry_count;
    uint16_t rejected_raw_count;
    uint32_t payload_bytes;
    uint32_t allocated_bytes_with_length_words;
    uint32_t receipt_hash;
} DM2_V1_GdatLoadEntriesReceipt;

/* Read-only first-pass selection made by DM2_LOAD_DYN4 for one six-byte
 * DM2_MARK_DYN_LOAD descriptor.  A selector byte of 0xff means "all" in
 * the original enumerator.  This receipt deliberately reports only source
 * rows and raw bytes already inside GRAPHICS.DAT: it neither allocates a
 * dynamic GDAT cache nor materializes a host-side substitute. */
typedef struct {
    uint8_t valid;
    uint8_t category;
    uint8_t index;
    uint8_t type;
    uint8_t field;
    uint16_t matched_entry_count;
    uint16_t raw_loadable_entry_count;
    uint16_t scalar_entry_count;
    uint16_t high_bit_data_index_count;
    uint16_t sound_entry_count;
    uint16_t rejected_raw_count;
    uint32_t payload_bytes;
    uint32_t receipt_hash;
} DM2_V1_GdatDyn4SelectionReceipt;

/* Caller-owned RAM image for the source DYN4 raw-data blocks.  Each block is
 * `[u16 raw_length][raw bytes][optional alignment byte][u16 raw_index]`,
 * matching the allocation layout immediately before DM2_LOAD_GDAT_RAW_DATA
 * in SKProject c_gdatfile.cpp::DM2_LOAD_DYN4.  It contains no decoded,
 * generated or disk-materialized data. */
typedef struct {
    uint8_t valid;
    uint8_t category;
    uint8_t index;
    uint8_t type;
    uint8_t field;
    uint16_t block_count;
    uint16_t skipped_sound_entry_count;
    uint32_t byte_count;
    uint32_t payload_hash;
    uint32_t receipt_hash;
    uint8_t *bytes;
    uint16_t *raw_indices;
    uint32_t *block_offsets;
} DM2_V1_GdatDyn4MaterializedSelection;

/* Minimal source state read by DM2_LOAD_DYN4 for type-2 rows.  The original
 * DM2_SOUND7 returns a non-zero queue position when a raw index is already
 * active, and c_dballoc's v1e13fe[2] blocks fresh sample materialisation.
 * The index array is borrowed from the live sound owner. */
typedef struct {
    uint8_t valid;
    uint8_t allocation_failed;
    const uint16_t *active_raw_indices;
    uint16_t active_raw_index_count;
} DM2_V1_GdatDyn4SoundState;

typedef struct {
    uint16_t cursor;
    int category_first;
    int category_last;
    int index_filter;
    int type_filter;
    int field_filter;
} DM2_V1_GdatEntryIterator;

typedef struct {
    uint8_t accepted;
    uint8_t header_state_before;
    uint8_t header_state_after;
    uint8_t flags_before;
    uint8_t flags_after;
    uint16_t toggled_bytes;
    uint32_t payload_hash_before;
    uint32_t payload_hash_after;
} DM2_V1_GdatSoundToggleReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t header_skip_bytes;
    uint16_t data_index;
    uint16_t raw_index;
    uint32_t raw_length;
    uint32_t payload_length;
    uint32_t payload_offset;
    uint32_t receipt_hash;
} DM2_V1_GdatSoundEntryReceipt;

typedef struct {
    int32_t file_open_counter;
    int16_t file_handle;
    int16_t xfile_handle;
    uint32_t primary_file_size;
    uint8_t filetype1;
    uint8_t filetype2;
} DM2_V1_GraphicsDataFileState;

typedef struct {
    uint8_t valid;
    uint8_t opened_primary;
    uint8_t opened_secondary;
    uint8_t blocked_primary_open;
    uint8_t blocked_secondary_open;
    uint8_t syserr_code;
    int32_t counter_before;
    int32_t counter_after;
    int16_t primary_handle;
    int16_t secondary_handle;
    uint32_t receipt_hash;
    uint16_t gdat_version;
    uint16_t raw_data_count;
    uint16_t entry_count;
    uint32_t container_byte_count;
    uint32_t typed_graph_hash;
    uint32_t interface_palette_hash;
    uint32_t title_menu_pixel_count;
    uint32_t title_menu_hash;
    uint32_t hud_hand_action_image_mask;
    uint32_t hud_hand_action_palette_hash;
    uint32_t hud_hand_action_pixel_hash;
    uint32_t environment_text_count;
    uint32_t environment_text_hash;
    uint32_t admission_hash;
} DM2_V1_GraphicsDataOpenReceipt;

typedef struct {
    uint8_t valid;
    uint8_t closed_primary;
    uint8_t closed_secondary;
    uint8_t blocked_underflow;
    int32_t counter_before;
    int32_t counter_after;
    int16_t primary_handle;
    int16_t secondary_handle;
    uint32_t receipt_hash;
} DM2_V1_GraphicsDataCloseReceipt;

typedef struct {
    uint8_t valid;
    uint8_t uses_primary;
    uint8_t uses_secondary;
    uint8_t crosses_secondary_split;
    uint8_t blocked_missing_state;
    int16_t primary_handle;
    int16_t secondary_handle;
    uint32_t request_offset;
    uint32_t request_length;
    uint32_t primary_offset;
    uint32_t primary_length;
    uint32_t secondary_offset;
    uint32_t secondary_length;
    uint32_t receipt_hash;
} DM2_V1_GraphicsDataReadReceipt;

typedef struct {
    uint16_t image_raw_index;
    int16_t underlay_raw_index;
} DM2_V1_GdatUnderlayPair;

typedef struct {
    uint8_t valid;
    uint8_t versionlo;
    uint8_t filetype1;
    uint8_t filetype2;
    uint8_t has_underlay_table;
    uint16_t entries;
    uint16_t raw_data_count;
    uint16_t underlay_pair_count;
    uint32_t raw0_length;
    uint32_t graphics_file_size;
    uint32_t calculated_payload_end;
    uint32_t max_raw_payload_length;
    uint32_t receipt_hash;
} DM2_V1_GraphicsStructureReceipt;

typedef struct {
    uint8_t valid;
    uint8_t uses_underlay;
    uint8_t decode_img3_underlay;
    uint8_t decode_img3_overlay;
    uint8_t decode_img9;
    uint8_t bpp;
    uint8_t gfxalloc_done;
    uint8_t prefer_hi_pool;
    uint16_t raw_index;
    uint16_t underlay_raw_index;
    uint16_t width;
    uint16_t height;
    uint32_t raw_length;
    uint32_t pixel_payload_bytes;
    uint32_t allocation_bytes;
    uint32_t decoded_pixel_hash;
    uint32_t receipt_hash;
} DM2_V1_GdatImageExtractReceipt;

/* Source-owned image material selected by SKULLWIN c_dballoc GFX routes.
 * source_bytes aliases the loaded GRAPHICS.DAT buffer; callers must not treat
 * it as an allocator-owned decoded or writable bitmap. */
typedef struct {
    uint8_t accepted;
    uint8_t used_gfx16_default;
    uint8_t gfxalloc_done;
    uint8_t selected_category;
    uint8_t selected_index;
    uint8_t selected_field;
    uint16_t raw_index;
    const uint8_t *source_bytes;
    size_t source_byte_count;
    DM2_V1_GdatImageExtractReceipt image;
    uint32_t receipt_hash;
} DM2_V1_GdatGfxMaterialReceipt;

typedef struct {
    uint8_t accepted;
    uint16_t raw_index;
    const uint8_t *source_bytes;
    size_t source_byte_count;
    uint32_t source_hash;
    uint32_t receipt_hash;
} DM2_V1_GdatGfxRawMaterialReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t used_default_image;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t bits_per_pixel;
    uint16_t requested_data_index;
    uint16_t selected_data_index;
    uint16_t selected_raw_index;
    uint16_t width;
    uint16_t height;
    uint32_t raw_length;
    uint32_t raw_hash;
    uint32_t receipt_hash;
} DM2_V1_GdatImageEntryBuffReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t used_existing_bitmap;
    uint8_t used_cached_bitmap;
    uint8_t queried_gdat_image;
    uint8_t mode;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint16_t selected_raw_index;
    uint16_t width;
    uint16_t height;
    uint32_t receipt_hash;
} DM2_V1_QueryPictBitsReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t palette16[16];
    uint16_t selected_raw_index;
    uint16_t width;
    uint16_t height;
    uint16_t width_units;
    uint32_t image_hash;
    uint32_t palette_hash;
    uint32_t receipt_hash;
} DM2_V1_Query4BppPictBuffAndPalReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t used_word_value;
    uint8_t used_text_sequence;
    uint8_t category;
    uint8_t index;
    uint16_t length;
    uint16_t frame_base;
    uint16_t frame;
    uint32_t receipt_hash;
} DM2_V1_QueryOrnateAnimFrameReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t decoration_absent;
    uint8_t used_word_value;
    uint8_t used_text_sequence;
    uint8_t category;
    uint8_t index;
    uint16_t length;
    uint32_t receipt_hash;
} DM2_V1_GetOrnateAnimLenReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t used_cache_byte;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint16_t value;
    uint32_t receipt_hash;
} DM2_V1_GdatWordQueryReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t used_explicit_strength;
    uint8_t used_resistance_fallback;
    uint8_t door_index;
    uint16_t strength;
    uint16_t resistance_field_value;
    uint32_t receipt_hash;
} DM2_V1_DoorStrengthReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t creature_route;
    uint8_t mask[64];
    uint16_t set_bits;
    uint32_t text_hash;
    uint32_t receipt_hash;
} DM2_V1_CreaturesItemMaskReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    int16_t inventory_slot;
    uint8_t only_body_part;
    uint8_t used_active_hand_result;
    uint16_t equip_flags;
    uint16_t tested_mask;
    uint16_t result;
    uint32_t receipt_hash;
} DM2_V1_ItemFitForEquipReceipt;

#define DM2_V1_CMDSTR_KEY_COUNT 18u
#define DM2_V1_CMDSTR_TEXT_CAP 128u

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint8_t truncated;
    uint16_t byte_count;
    char text[DM2_V1_CMDSTR_TEXT_CAP];
    uint32_t text_hash;
    uint32_t receipt_hash;
} DM2_V1_GdatNameReceipt;

typedef struct {
    uint8_t accepted;
    uint8_t found;
    uint8_t key_index;
    char key[3];
    int32_t value;
    uint32_t text_hash;
    uint32_t receipt_hash;
} DM2_V1_CmdstrEntryReceipt;

typedef struct {
    uint8_t category;
    uint8_t index;
    uint8_t field;
} DM2_V1_CurCmdstrContext;

typedef struct {
    uint8_t accepted;
    uint8_t category;
    uint8_t index;
    uint8_t field;
    uint16_t requested_order;
    uint16_t enumerated_order;
    uint16_t resolved_item_type;
    int16_t money_item_index;
    uint16_t text_length;
    uint32_t text_hash;
    uint32_t receipt_hash;
} DM2_V1_ItemOrderInContainerReceipt;

/* ── Public API ─────────────────────────────────────────────────── */

/* Initialize asset loader with GRAPHICS.DAT data.
 * Returns 0 on success, -1 on failure.
 * Source: docs/dm2_graphics.md — GDAT file structure */
int dm2_v1_asset_loader_init(DM2_V1_AssetLoader *loader,
                              const uint8_t *data, size_t size);

/* Validate every typed ENT1 entry that owns a raw GDAT payload.  Scalar
 * dtWordValue and dtImageOffset entries intentionally remain immediate
 * values, as in skproject QUERY_GDAT_ENTRY_DATA_INDEX. */
int dm2_v1_asset_loader_validate_typed_graph(const DM2_V1_AssetLoader *loader);

/* Load asset by (category, index, field) triple.
 * Returns raw asset data pointer (NULL on failure).
 * Source: SkGlobal.h — c_gdatfile class, GDAT2 field codes */
const uint8_t *dm2_v1_asset_load(const DM2_V1_AssetLoader *loader,
                                   int category, int index, int field);

/* Load raw asset by (category, index, field), returning its byte size.
 * Immediate dtWordValue/dtImageOffset entries are intentionally excluded;
 * use their exact typed APIs below. Source: skproject SKWIN/SkWinCore.cpp
 * QUERY_GDAT_ENTRY_DATA_PTR */
const uint8_t *dm2_v1_asset_load_sized(const DM2_V1_AssetLoader *loader,
                                        int category, int index, int field,
                                        size_t *out_size);

/* Load raw asset by exact skproject GDAT type (cls3) plus
 * category/index/field.  This mirrors QUERY_GDAT_ENTRY_DATA_PTR(cls1, cls2,
 * cls3, cls4) and avoids conflating typed data with dtImage. */
const uint8_t *dm2_v1_asset_load_typed_sized(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    size_t *out_size);

/* Load an exact skproject dtText payload.  The returned bytes retain their
 * original encoding; callers must not treat them as decoded C strings until
 * the relevant QUERY_GDAT_TEXT/FORMAT_SKSTR contract has been proven. */
const uint8_t *dm2_v1_asset_load_text_sized(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    size_t *out_size);

/* skproject c_gdatfile.cpp/c_querydb.cpp raw GDAT entry queries. These are
 * bounded receipts over the parsed ENT1/raw table only; scalar dtWordValue and
 * dtImageOffset entries expose their data index but are not loadable buffers. */
int dm2_v1_query_gdat_raw_data_file_pos(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    uint32_t *out_file_pos);
int dm2_v1_query_gdat_raw_data_length(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    uint32_t *out_length);
const uint8_t *dm2_v1_load_gdat_raw_data(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    size_t *out_size);
int dm2_v1_query_gdat_entry(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    DM2_V1_GdatEntryQueryReceipt *out_receipt);
int dm2_v1_query_gdat_entry_value(
    const DM2_V1_AssetLoader *loader,
    uint16_t entry_ordinal,
    uint8_t group_index,
    uint32_t *out_value);
int dm2_v1_query_gdat_entry_data_index(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    uint16_t *out_data_index);
const uint8_t *dm2_v1_query_gdat_entry_data_ptr(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    size_t *out_size);
const uint8_t *dm2_v1_direct_query_gdat_entry_data_buff_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    size_t *out_size,
    DM2_V1_DirectGdatEntryDataBuffReceipt *out_receipt);
const uint8_t *dm2_v1_direct_query_gdat_text_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    size_t *out_size,
    DM2_V1_DirectGdatTextReceipt *out_receipt);
int dm2_v1_query_gdat_entry_data_length(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    uint32_t *out_length);
int dm2_v1_query_gdat_entry_if_loadable(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    DM2_V1_GdatEntryQueryReceipt *out_receipt);
int dm2_v1_load_gdat_entry_data_to(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int type,
    int field,
    uint8_t *destination,
    size_t destination_capacity,
    DM2_V1_GdatEntryQueryReceipt *out_receipt);
int dm2_v1_load_ent1_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatEnt1Receipt *out_receipt);
int dm2_v1_load_gdat_entries_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatLoadEntriesReceipt *out_receipt);
/* Source: SKProject SKULLWIN/c_loadlevel.cpp::DM2_MARK_DYN_LOAD and
 * c_gdatfile.cpp::DM2_LOAD_DYN4 (first marking pass).  `resource_id` uses
 * the original byte order: category/index/type/field. */
int dm2_v1_gdat_dyn4_selection_receipt(
    const DM2_V1_AssetLoader *loader,
    uint32_t resource_id,
    DM2_V1_GdatDyn4SelectionReceipt *out_receipt);
/* Initializes the source's empty c_dballoc/DM2_SOUND5 state. */
void dm2_v1_gdat_dyn4_sound_state_init(
    DM2_V1_GdatDyn4SoundState *state);
/* Materialize one selector's source-owned raw blocks in RAM.  This is only
 * the final raw-copy layout of DM2_LOAD_DYN4, not its allocator, cache
 * eviction or gameplay state machine.  `sound_state` must be a real live
 * owner or an explicit source-empty state; NULL defers type-2 rows. */
int dm2_v1_gdat_dyn4_materialize_selection(
    const DM2_V1_AssetLoader *loader,
    uint32_t resource_id,
    const DM2_V1_GdatDyn4SoundState *sound_state,
    DM2_V1_GdatDyn4MaterializedSelection *out_selection);
void dm2_v1_gdat_dyn4_materialized_selection_free(
    DM2_V1_GdatDyn4MaterializedSelection *selection);
int dm2_v1_query_next_gdat_entry(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GdatEntryIterator *iterator,
    DM2_V1_GdatEntryQueryReceipt *out_receipt);
int dm2_v1_gdat_sound_toggle_payload(
    uint8_t *payload,
    uint16_t payload_length,
    uint16_t header_state,
    uint8_t header_flags,
    DM2_V1_GdatSoundToggleReceipt *out_receipt);
int dm2_v1_gdat_sound_entry_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    int sound7_result,
    int extended_header,
    DM2_V1_GdatSoundEntryReceipt *out_receipt);
uint16_t dm2_v1_r_2bad4_swap_word(uint16_t value);
int dm2_v1_r_2d07d_max_raw_length_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t type_filter,
    uint8_t field_filter,
    DM2_V1_GdatMaxRawLengthReceipt *out_receipt);

/* skproject c_gdatfile.cpp GRAPHICS_DATA_OPEN/READ/CLOSE receipts.
 * These expose file-counter, handle, and split-read routing only; callers own
 * real file IO and buffers. */
int dm2_v1_graphics_data_open_receipt(
    DM2_V1_GraphicsDataFileState *state,
    int primary_open_ok,
    int16_t primary_handle,
    int secondary_open_ok,
    int16_t secondary_handle,
    DM2_V1_GraphicsDataOpenReceipt *out_receipt);
int dm2_v1_graphics_data_close_receipt(
    DM2_V1_GraphicsDataFileState *state,
    DM2_V1_GraphicsDataCloseReceipt *out_receipt);
int dm2_v1_graphics_data_read_receipt(
    const DM2_V1_GraphicsDataFileState *state,
    uint32_t offset,
    uint32_t length,
    DM2_V1_GraphicsDataReadReceipt *out_receipt);
int dm2_v1_gdat_track_underlay(
    const DM2_V1_GdatUnderlayPair *pairs,
    size_t pair_count,
    uint16_t image_raw_index,
    int16_t *out_underlay_raw_index);
int dm2_v1_read_graphics_structure_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_GraphicsStructureReceipt *out_receipt);
int dm2_v1_extract_gdat_image_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    int gfxalloc_done,
    int prefer_hi_pool,
    const DM2_V1_GdatUnderlayPair *underlays,
    size_t underlay_count,
    DM2_V1_GdatImageExtractReceipt *out_receipt);
int dm2_v1_gdat_allocate_gfx256_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    int gfxalloc_done,
    DM2_V1_GdatGfxMaterialReceipt *out_receipt);
int dm2_v1_gdat_allocate_gfx256_raw_material_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t raw_index,
    DM2_V1_GdatGfxRawMaterialReceipt *out_receipt);
/* Exact GDAT image ownership: unlike the GFX16 allocator route, this never
 * substitutes MISCELLANEOUS/FE/FE when the requested image is absent. */
int dm2_v1_gdat_image_raw_material_receipt(
    const DM2_V1_AssetLoader *loader, int category, int index, int field,
    DM2_V1_GdatGfxRawMaterialReceipt *out_receipt);
int dm2_v1_gdat_allocate_gfx16_material_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    int gfxalloc_done,
    DM2_V1_GdatGfxMaterialReceipt *out_receipt);
int dm2_v1_query_gdat_image_entry_buff_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_GdatImageEntryBuffReceipt *out_receipt);
int dm2_v1_query_gdat_image_metrics_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint16_t *out_width,
    uint16_t *out_height,
    DM2_V1_GdatImageEntryBuffReceipt *out_receipt);
int dm2_v1_query_pict_bits_receipt(
    const DM2_V1_AssetLoader *loader,
    uint8_t mode,
    int existing_bitmap_present,
    int cached_bitmap_present,
    int category,
    int index,
    int field,
    DM2_V1_QueryPictBitsReceipt *out_receipt);
int dm2_v1_query_4bpp_pict_buff_and_pal_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    uint16_t width_divisor,
    DM2_V1_Query4BppPictBuffAndPalReceipt *out_receipt);
int dm2_v1_query_picst_image_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_QueryPicstImageReceipt *out_receipt);
int dm2_v1_query_gdat_summary_image_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_QueryGdatSummaryImageReceipt *out_receipt);
int dm2_v1_query_ornate_anim_frame_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    uint32_t tick,
    uint32_t delta,
    DM2_V1_QueryOrnateAnimFrameReceipt *out_receipt);
int dm2_v1_get_ornate_anim_len_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int decoration_absent,
    DM2_V1_GetOrnateAnimLenReceipt *out_receipt);
int dm2_v1_query_door_damage_resist_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_get_door_stat_0x10_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_get_graphics_for_door_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_0cee_3275_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_gdat_creature_word_value_receipt(
    const DM2_V1_AssetLoader *loader,
    int creature_index,
    int field,
    const uint8_t *cache3,
    size_t cache_count,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_gdat_food_value_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_gdat_potion_spell_type_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_gdat_potion_behaviour_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_gdat_water_value_from_record_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_gdat_door_is_mirrored_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_GdatWordQueryReceipt *out_receipt);
int dm2_v1_query_door_strength_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_DoorStrengthReceipt *out_receipt);
int dm2_v1_query_creatures_item_mask_receipt(
    const DM2_V1_AssetLoader *loader,
    int creature_index,
    int text_field_base,
    int is_creature,
    DM2_V1_CreaturesItemMaskReceipt *out_receipt);
int dm2_v1_is_item_fit_for_equip_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int inventory_slot,
    int only_body_part,
    int active_hand_fit_result,
    DM2_V1_ItemFitForEquipReceipt *out_receipt);
int dm2_v1_query_gdat_item_name_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    DM2_V1_GdatNameReceipt *out_receipt);
int dm2_v1_query_cmdstr_name_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_GdatNameReceipt *out_receipt);
int dm2_v1_query_cmdstr_entry_receipt(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    int key_index,
    DM2_V1_CmdstrEntryReceipt *out_receipt);
int dm2_v1_query_cur_cmdstr_entry_receipt(
    const DM2_V1_AssetLoader *loader,
    const DM2_V1_CurCmdstrContext *context,
    int key_index,
    DM2_V1_CmdstrEntryReceipt *out_receipt);
int dm2_v1_get_item_order_in_container_receipt(
    const DM2_V1_AssetLoader *loader,
    int container_index,
    int requested_order,
    const uint16_t *money_item_ids,
    size_t money_item_count,
    DM2_V1_ItemOrderInContainerReceipt *out_receipt);

/* skproject c_gdatfile.cpp bitmap allocation/free receipts. These expose
 * only source byte accounting and route ownership; no decoded pixels,
 * CPX heap nodes, or preserved-GFX list nodes are fabricated. */
int dm2_v1_gdat_alloc_pict_buff_receipt(
    uint16_t width,
    uint16_t height,
    uint8_t bpp,
    DM2_V1_GdatPictPool pool,
    DM2_V1_GdatPictAllocationReceipt *out_receipt);
int dm2_v1_gdat_alloc_new_bmp_receipt(
    uint16_t raw_index,
    uint16_t width,
    uint16_t height,
    uint8_t bpp,
    DM2_V1_GdatPictAllocationReceipt *out_receipt);
int dm2_v1_gdat_free_pict_buff_receipt(
    const DM2_V1_GdatPictAllocationReceipt *allocation,
    DM2_V1_GdatPictFreeReceipt *out_receipt);
int dm2_v1_gdat_free_pict_entry_receipt(
    const DM2_V1_GdatPictAllocationReceipt *allocation,
    int header_matches,
    int has_bigpool_struct_tail,
    int preserved_list_member,
    DM2_V1_GdatPictFreeReceipt *out_receipt);
int dm2_v1_gdat_bigpool_memory_receipt(
    uint32_t requested_bytes,
    DM2_V1_GdatPictPool pool,
    int clean,
    int deallocate,
    DM2_V1_GdatBigpoolMemoryReceipt *out_receipt);
int dm2_v1_gdat_cpx_reserve_receipt(
    uint16_t wp08_word,
    uint32_t byte_count,
    DM2_V1_GdatCpxReserveReceipt *out_receipt);
int dm2_v1_gdat_cpx_copy_receipt(
    uint16_t wp08_word,
    uint32_t byte_count,
    const uint8_t *source_with_header,
    uint32_t source_byte_count,
    DM2_V1_GdatCpxCopyReceipt *out_receipt);
int dm2_v1_gdat_cpx_compact_receipt(
    uint16_t pool_top_word,
    uint16_t wp08_word,
    const DM2_V1_GdatCpxBlockInput *blocks,
    uint16_t block_count,
    DM2_V1_GdatCpxCompactReceipt *out_receipt);
int dm2_v1_dballoc_3e74_24b8_receipt(
    const DM2_V1_AssetLoader *loader,
    DM2_V1_DballocSoundCensusReceipt *out_receipt);
int dm2_v1_dballoc_3e74_2162_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t entry_ordinal,
    uint8_t active_mask,
    DM2_V1_DballocEntryFilterReceipt *out_receipt);
int dm2_v1_load_dyn4_admission_receipt(
    const DM2_V1_AssetLoader *loader,
    uint16_t descriptor_count,
    int cache_locked,
    DM2_V1_LoadDyn4AdmissionReceipt *out_receipt);

/* Read a skproject dtWordValue field by exact category/index/field.
 * Returns 1 on success and 0 when the typed entry is absent. */
int dm2_v1_asset_load_word_value(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint16_t *out_value);

/* SKProject QUERY_ORNATE_ANIM_FRAME. Resolves only the original
 * dtWordValue/dtText field 0x0d sequence; malformed or unsupported bytes
 * fail closed rather than selecting a substitute animation frame. */
int dm2_v1_asset_query_ornate_animation_frame(
    const DM2_V1_AssetLoader *loader, int category, int index,
    uint32_t tick, uint32_t delta, uint16_t *out_frame,
    uint32_t *out_receipt_hash);

/* Read a skproject dtImageOffset field by exact category/index/field.
 * Returns 1 on success and 0 when the typed entry is absent. */
int dm2_v1_asset_load_image_offset(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint16_t *out_value);

/* Decode INTERFACE_GENERAL's paired dtPalIRGB/dtPalette16 entries.  The
 * original stores 256 four-byte IRGB rows and 16 one-byte palette indices;
 * only an exact typed pair is accepted. */
int dm2_v1_asset_load_interface_palette(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_InterfacePalette *out_palette);

/* Load image asset and decode to pixel buffer.
 * out_width, out_height set dimensions, out_format sets format.
 * Caller owns returned buffer (must free with dm2_v1_asset_free_pixels).
 * Source: SKULL.ASM T560 — viewport rendering
 * Source: ReDMCSB DUNGEON.C:1371-1421 — wall frame table */
uint8_t *dm2_v1_asset_load_image(const DM2_V1_AssetLoader *loader,
                                  int category, int index,
                                  int *out_width, int *out_height,
                                  DM2_ImageFormat *out_format);

/* Load image asset by explicit GDAT field (cls4).
 * Source: skproject SKWIN/SkWinCore.cpp QUERY_GDAT_IMAGE_ENTRY_BUFF
 * line ~38377 selects dtImage by (cls1, cls2, cls4). */
uint8_t *dm2_v1_asset_load_image_field(const DM2_V1_AssetLoader *loader,
                                        int category, int index, int field,
                                        int *out_width, int *out_height,
                                        DM2_ImageFormat *out_format);

/* Decode one exact raw GDAT image record. This is the raw-table counterpart
 * to QUERY_GDAT_IMAGE_ENTRY_BUFF: it deliberately bypasses category/index/
 * field selection so catalogue and multilingual audits cannot silently pick
 * a different entry sharing the same GDAT address. It does not select an
 * image for gameplay; production callers must retain the source entry query. */
uint8_t *dm2_v1_asset_load_raw_image(const DM2_V1_AssetLoader *loader,
                                     uint16_t raw_index,
                                     int *out_width, int *out_height,
                                     DM2_ImageFormat *out_format);

/* ── SKULLWIN/c_gfx_decode.cpp source-named decode receipts ────────── */

/* skproject: c_gfx_decode.cpp init (line 19) / alloc (line 41).
 * Object-lifecycle boundaries that Firestaff does not replicate; these
 * receipts document the boundary without allocating host state. */
void dm2_v1_decode_img3_init(void);
void dm2_v1_decode_img3_alloc(void);

/* skproject: c_gfx_decode.cpp read_img3_nibble (line 87) /
 * read_img3_duration (line 95). */
int dm2_v1_decode_img3_read_nibble(const uint8_t *raw,
                                   size_t raw_size,
                                   size_t *cursor,
                                   uint8_t *out);
int dm2_v1_decode_img3_read_duration(const uint8_t *raw,
                                     size_t raw_size,
                                     size_t *cursor,
                                     int *out_duration);

/* skproject: c_gfx_decode.cpp func_44c8_1202 (line 52). */
void dm2_v1_decode_img3_func_44c8_1202(uint8_t *dest,
                                       size_t dest_pixels,
                                       size_t offset,
                                       uint8_t pixel4);

/* skproject: c_gfx_decode.cpp spill_img3_pixels (line 63). */
int dm2_v1_decode_img3_spill_pixels(uint8_t *dest,
                                    size_t dest_pixels,
                                    size_t dofs,
                                    size_t sofs,
                                    int num);

/* skproject: c_gfx_decode.cpp transparent_img3_pixels (line 111). */
int dm2_v1_decode_img3_transparent_pixels(uint8_t *dest,
                                          const uint8_t *underlay,
                                          size_t underlay_pixels,
                                          size_t ofs,
                                          int num);

/* skproject: c_gfx_decode.cpp decode_img3_overlay (line 276). */
uint8_t *dm2_v1_decode_img3_overlay(const uint8_t *raw,
                                    size_t raw_size,
                                    const uint8_t *underlay,
                                    size_t underlay_pixels,
                                    int width,
                                    int height,
                                    DM2_ImageFormat *out_format);

/* skproject: c_gfx_decode.cpp decode_img9 (line 744) and mode entries. */
uint8_t *dm2_v1_decode_img9(const uint8_t *raw,
                            size_t raw_size,
                            int width,
                            int height,
                            DM2_ImageFormat *out_format);
uint8_t *dm2_v1_decode_img9_mode1(const uint8_t *raw,
                                  size_t raw_size,
                                  int width,
                                  int height,
                                  DM2_ImageFormat *out_format);
uint8_t *dm2_v1_decode_img9_mode2(const uint8_t *raw,
                                  size_t raw_size,
                                  int width,
                                  int height,
                                  DM2_ImageFormat *out_format);
uint8_t *dm2_v1_decode_img9_mode3(const uint8_t *raw,
                                  size_t raw_size,
                                  int width,
                                  int height,
                                  DM2_ImageFormat *out_format);

/* Get GDAT category count and index range for a category.
 * Returns number of entries in category.
 * Source: SkGlobal.h:636 — GDAT_CATEGORY_LIMIT */
int dm2_v1_asset_category_entry_count(const DM2_V1_AssetLoader *loader,
                                       int category);

/* Free pixel buffer allocated by dm2_v1_asset_load_image. */
void dm2_v1_asset_free_pixels(uint8_t *pixels);

/* Get GDAT2 field code description.
 * Source: SKWIN/knowledge/SKWin.GDAT2.InternalCodes.txt */
const char *dm2_v1_asset_gdat2_field_name(int field_code);

/* Check if DM2 GRAPHICS.DAT is verified (MD5 matches known good hash).
 * DM2 PC English: 25247ede4dabb6a71e5dabdfbcd5907d (~8.6 MB)
 * Source: docs/dm2_platform_data.md */
int dm2_v1_asset_loader_verify(const DM2_V1_AssetLoader *loader);

/* Free asset loader and all owned resources. */
void dm2_v1_asset_loader_free(DM2_V1_AssetLoader *loader);

/* Source-lock citation string. */
const char *dm2_v1_asset_loader_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ASSET_LOADER_H */
