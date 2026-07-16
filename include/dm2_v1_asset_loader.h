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
    /* skproject SKWIN/DME.h dtText.  QUERY_GDAT_TEXT selects this exact
     * type; it must not be confused with a drawable environment image. */
    DM2_GDAT_ENTRY_TYPE_TEXT         = 0x05,
    /* skproject/SKWIN/DME.h dtIndex: dtRectangle/dt04 = 4.
     * LOAD_RECTS_AND_COMPRESS reads this exact typed payload before it
     * expands the title-menu and HUD rectangle tables. */
    DM2_GDAT_ENTRY_TYPE_RAW4         = 0x04,
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
#define DM2_GDAT_IMG_MAP_CHIP 0xF9

#define DM2_GDAT_GFXSET_SCENE_COLORKEY       0x64
#define DM2_GDAT_GFXSET_SCENE_FLAGS          0x65
#define DM2_GDAT_GFXSET_SCENE_RAIN           0x66
#define DM2_GDAT_GFXSET_AMBIANT_LIGHT        0x67
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

typedef struct {
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

/* Main 256-colour palette and the 16-colour logical-index table loaded by
 * SkWinCore::INIT before the HUD and dungeon viewport are drawn.  RGB values
 * are VGA 6-bit components, matching SET_GRAPHICS_RGB_PALETTE's >> 2 step. */
typedef struct {
    uint8_t rgb6[256][3];
    uint8_t palette16[16];
    uint32_t hash;
} DM2_V1_InterfacePalette;

/* Exact non-pixel portion of the original IMG3 record consumed by
 * QUERY_GDAT_SUMMARY_IMAGE/QUERY_TEMP_PICST.  Width and height come from
 * the IMG3 header; the two offsets are the category-wide field 0xfe and the
 * image-specific field.  No image decoder or generated surface is involved. */
typedef struct {
    uint16_t width;
    uint16_t height;
    uint16_t bits_per_pixel;
    int graphicsset_offset_present;
    int image_offset_present;
    int8_t graphicsset_offset_x;
    int8_t graphicsset_offset_y;
    int8_t image_offset_x;
    int8_t image_offset_y;
    int16_t query_offset_x;
    int16_t query_offset_y;
    uint32_t metadata_hash;
} DM2_V1_GdatImageMetadata;

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
 * Source: skproject SKWIN/SkWinCore.cpp QUERY_GDAT_ENTRY_DATA_PTR */
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
int dm2_v1_query_door_strength_receipt(
    const DM2_V1_AssetLoader *loader,
    int door_index,
    DM2_V1_DoorStrengthReceipt *out_receipt);

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

/* Read only the source metadata that QUERY_TEMP_PICST receives before image
 * realization. Returns zero unless the exact dtImage record is present;
 * absent original dtImageOffset fields retain the source's zero offset. */
int dm2_v1_asset_load_image_metadata(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_GdatImageMetadata *out_metadata);

/* Decode INTERFACE_GENERAL's paired dtPalIRGB/dtPalette16 entries.  The
 * original stores 256 four-byte IRGB rows and 16 one-byte palette indices;
 * only an exact typed pair is accepted. */
int dm2_v1_asset_load_interface_palette(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    DM2_V1_InterfacePalette *out_palette);

/* Read the 16-byte palette stored at the tail of one four-bit IMG3 GDAT
 * image.  skproject QUERY_GDAT_IMAGE_LOCALPAL returns this exact payload to
 * DRAW_CHIP_OF_MAGIC_MAP; it is not the global INTERFACE palette. */
int dm2_v1_asset_load_image_local_palette(
    const DM2_V1_AssetLoader *loader,
    int category,
    int index,
    int field,
    uint8_t out_palette16[16],
    uint32_t *out_hash);

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
