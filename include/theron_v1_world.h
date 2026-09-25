#ifndef THERON_V1_WORLD_H
#define THERON_V1_WORLD_H

#include "theron_v1_champions.h"

/* Forward typedef so theron_v1_combat.h prototypes can use Theron_V1_World*
 * without a conflicting redefinition when this header later defines the
 * struct body. */
typedef struct Theron_V1_World Theron_V1_World;
#define THERON_V1_WORLD_TYPEDEF

/* Forward typedef for the Track 02 compact object table used by the real-data
 * initial-level decoder. The full struct is defined in theron_v1_track02.h.
 * The declaration marker prevents that header's complete definition from
 * re-typedefing the same C11 tag in strict-warning builds. */
#ifndef THERON_TRACK02_OBJECT_TABLE_TYPEDEF
typedef struct Theron_Track02ObjectTable Theron_Track02ObjectTable;
#define THERON_TRACK02_OBJECT_TABLE_TYPEDEF
#endif

#include "theron_v1_combat.h"
#include "theron_v1_dungeon_progression.h"
#include "theron_v1_track02_spawn_binding.h"
#include "theron_v1_track02_item_name_source.h"
#include "theron_v1_track02_retrieval_text_source.h"
#include "theron_v1_track02_campaign_mask_source.h"
#include "theron_v1_track02_text_decode.h"
#include "theron_v1_track19_inventory.h"
#include <stddef.h>
#include <stdint.h>

/* ══════════════════════════════════════════════════════════════════════
 * Theron V1 Phase 3 — Core World Model
 *
 * Map loading, party placement (Theron + 3 champions), map transitions,
 * timers, object database, and deterministic world-state hashing.
 *
 * Key design constraints (from TQR provenance):
 *   - 7 mini-dungeons, 3-8 maps each including the hub map (matching Track 02 quest blocks).
 *   - Between-dungeon saves only (no in-dungeon save).
 *   - Theron persists fully across dungeons.
 *   - Champions reset inventories each dungeon, keep stats/skills.
 *   - 7 quest items (one per dungeon) must be found before exit.
 *   - Party of 1-4: Theron plus up to 3 companions chosen in the Soul Room.
 *
 * Source references:
 *   THQUEST.ASM T000  — title/startup entry
 *   THQUEST.ASM T400  — dungeon bank loading
 *   THQUEST.ASM T520  — party placement / start position
 *   THQUEST.ASM T560  — dungeon loading (header parsing, dungeon_seed)
 *   THQUEST.ASM T600  — map transitions
 *   THQUEST.ASM T700  — timers / world tick
 *   THQUEST.ASM T800  — champion persistence + inventory reset
 *   THQUEST.ASM T900  — object database / thing list
 * ══════════════════════════════════════════════════════════════════════ */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Map / level constants ─────────────────────────────────────────── */
#define THERON_MAX_LEVELS_PER_DUNGEON  8
#define THERON_MAX_MAP_SIZE            32
#define THERON_MAX_OBJECTS_PER_LEVEL  128
#define THERON_MAX_TIMERS              16
#define THERON_TICK_MS                 55  /* ~18.2 Hz world tick (matches DM1) */

/* Decoded Track 02 bitmap surfaces retained by the live world.  These are
 * deliberately copies, rather than pointers into the launch asset, so a
 * Continue route retains its level media for the lifetime of the world. */
#define THERON_RUNTIME_MEDIA_MAX_WIDTH 256u
#define THERON_RUNTIME_MEDIA_HEIGHT    8u
#define THERON_RUNTIME_MEDIA_PIXELS \
    (THERON_RUNTIME_MEDIA_MAX_WIDTH * THERON_RUNTIME_MEDIA_HEIGHT)

typedef enum {
    THERON_RUNTIME_MEDIA_SURFACE_NONE = 0,
    THERON_RUNTIME_MEDIA_SURFACE_TITLE,
    THERON_RUNTIME_MEDIA_SURFACE_STAGE,
    THERON_RUNTIME_MEDIA_SURFACE_SOUL_ROOM,
    THERON_RUNTIME_MEDIA_SURFACE_FORCEFIELD
} Theron_RuntimeMediaSurfaceKind;

typedef struct {
    int ready;
    /* Indexed pixels remain tied to their authenticated Track 02 envelope.
     * This does not assert palette or screen-layout semantics. */
    int raw_source_verified;
    char track02_md5[33];
    unsigned int route_bit;
    uint16_t width;
    uint16_t height;
    size_t first_raw_offset;
    size_t last_raw_offset;
    size_t first_user_data_offset;
    size_t tile_count;
    size_t nonzero_pixel_count;
    uint32_t checksum;
    uint8_t pixels[THERON_RUNTIME_MEDIA_PIXELS];
} Theron_RuntimeMediaSurface;

/* An observed original CD_READ payload retained by runtime before any later
 * game-owned consumer proves its level, object, palette, or visual meaning. */
typedef struct {
    int ready;
    int raw_source_verified;
    int no_semantic_promotion;
    char track02_md5[33];
    uint32_t record;
    uint32_t destination;
    size_t raw_user_data_offset;
    size_t payload_bytes;
    uint32_t payload_checksum;
    int level_envelope_bound;
    size_t level_envelope_offset;
    size_t level_envelope_bytes;
    uint32_t level_envelope_checksum;
    size_t post_envelope_offset;
    size_t post_envelope_bytes;
    uint32_t post_envelope_checksum;
} Theron_RuntimeTrack02LoaderRecord;

/* Provenance for the bank and audio frame paired with decoded Track 02
 * surfaces.  This records byte-verified routing only; it does not claim a
 * decoded palette payload or playable audio stream. */
typedef struct {
    int ready;
    int track02_variant;
    size_t bank_anchor_index;
    size_t bank_descriptor_offset;
    uint16_t bank_first_value;
    uint16_t bank_last_value;
    uint16_t bank_stride;
    int audio_frame_ready;
    uint32_t audio_bank_id;
    size_t audio_bank_id_offset;
    size_t audio_bank_prefix_offset;
    uint32_t checksum;
} Theron_RuntimeMediaIdentity;

typedef enum {
    THERON_RUNTIME_LEVEL_BANK_NONE = 0,
    THERON_RUNTIME_LEVEL_BANK_STARTUP_FORCEFIELD = 1,
    THERON_RUNTIME_LEVEL_BANK_LATER_LEVEL = 2,
    THERON_RUNTIME_LEVEL_BANK_SAVE_RESUME = 3
} Theron_RuntimeLevelBankKind;

typedef struct {
    int ready;
    int real_media_gate;
    Theron_RuntimeLevelBankKind kind;
    Theron_DungeonID dungeon_id;
    int level_index;
    unsigned int route_bit;
    int raw_source_verified;
    char track02_md5[33];
    size_t first_raw_offset;
    size_t last_raw_offset;
    size_t first_user_data_offset;
    uint16_t width;
    uint16_t height;
    size_t tile_count;
    size_t nonzero_pixel_count;
    uint32_t surface_checksum;
    uint32_t identity_checksum;
    uint64_t cache_generation;
} Theron_RuntimeLevelBankSelection;

/* One authenticated later-level block copied into runtime state.  The
 * source parser borrows its compressed span from the caller, so runtime keeps
 * only the byte-range/hash/meta receipt and deliberately no source pointer.
 * This is a source handoff, not a decompressed tile, map, or object claim.
 * The HuC6280 consumer/decompressor remains gated by the reviewed
 * bank-1f disassembly in docs/source-lock/theron-disassembly/. */
typedef struct {
    int ready;
    int no_semantic_promotion;
    int track02_variant;
    unsigned int level;
    uint32_t block_ud_offset;
    uint32_t compressed_ud_offset;
    uint32_t resource_end_ud_offset;
    size_t compressed_bytes;
    uint16_t resource_length;
    uint32_t compressed_fnv1a;
    uint32_t shared_prologue_fnv1a;
    uint8_t per_level_meta[8];
} Theron_RuntimeLevelDataBlockReceipt;

typedef struct {
    int restored;
    unsigned int route_mask;
    uint32_t checksum;
    /* Title/stage/Soul Room/forcefield preserve the four verified Track 02
     * startup bitmap routes.  A later (non-zero) level uses stage; gameplay
     * forcefield explicitly selects forcefield. */
    Theron_RuntimeMediaSurface title;
    Theron_RuntimeMediaSurface stage;
    Theron_RuntimeMediaSurface soul_room;
    Theron_RuntimeMediaSurface forcefield;
    Theron_RuntimeTrack02LoaderRecord loader_record;
    Theron_RuntimeMediaIdentity identity;
    uint64_t cache_generation;
    Theron_RuntimeLevelBankSelection level_bank;
    Theron_RuntimeLevelDataBlockReceipt later_level_data;
    int startup_palette_valid;
    uint8_t startup_palette_rgb8[16][3];
    /* Candidate bitmap routes and palette bytes are source receipts only.
     * This flag stays clear until a captured VDC/VCE consumer binds the
     * screen destination and semantic route together. */
    int startup_presentation_allowed;
} Theron_RuntimeLevelMedia;

/* ── Square tile types ────────────────────────────────────────────── */
#define THERON_SQUARE_WALL           0
#define THERON_SQUARE_FLOOR          1
#define THERON_SQUARE_DOOR           4   /* open/close on step/command */
#define THERON_SQUARE_PIT            2   /* trap — wounds champion */
#define THERON_SQUARE_STAIRS_UP      3   /* level N → level N-1 */
#define THERON_SQUARE_STAIRS_DOWN    13  /* level N → level N+1 */
#define THERON_SQUARE_STAIRS_UNRESOLVED 14 /* source says stairs; direction unknown */
#define THERON_SQUARE_TELEPORTER     5   /* warp to linked target */
#define THERON_SQUARE_ALARM          6   /* alert all creatures */
#define THERON_SQUARE_EXIT           8   /* quest-complete exit */
#define THERON_SQUARE_TRIGGER        9   /* event trigger */
#define THERON_SQUARE_POOL           10  /* water/food recovery */
#define THERON_SQUARE_SECRET        11  /* hidden door wall */

#define THERON_SQUARE_IS_PASSABLE(t) ((t) != THERON_SQUARE_WALL && (t) != THERON_SQUARE_SECRET)
#define THERON_SQUARE_IS_SPECIAL(t)  ((t) >= 2 && (t) != 4)

/* ── Level struct ──────────────────────────────────────────────────── */
typedef struct {
    int   level_index;          /* 0..7 map index within this dungeon */
    int   width, height;        /* typically 20x20 for TQR mini-dungeons */
    uint32_t dungeon_seed;      /* source header seed, never inferred */
    uint16_t source_header_level_index; /* opaque Track 02 header value */
    /* Exact Track 02 map-header bytes retained without assigning gameplay
     * meaning. Source: theron_v1_track02_dungeon_map.c, the real 11-byte
     * per-map header reader; the two unknown bytes remain opaque. */
    uint8_t source_map_x_offset;
    uint8_t source_map_y_offset;
    uint8_t source_header_unk1;
    uint8_t source_header_unk2;
    uint8_t source_xp_modifier;
    uint8_t source_door_type1;
    uint8_t source_door_type2;
    uint8_t source_header_verified;
    /* The T900 item-property consumer may only run after the complete
     * source-owned 66-row table has been verified for this loaded bank. */
    uint8_t source_item_property_table_verified;
    /* Per-map source records retained from the real Track 02 map-bank
     * directory. These are provenance fields only: no creature graphics or
     * thing-list semantics are inferred from them. */
    uint16_t source_creature_gfx_bank;
    uint16_t source_cumulative_column_items;
    int   start_x, start_y;    /* party spawn position (THQUEST.ASM T520) */
    int   start_dir;            /* 0=N 1=E 2=S 3=W */
    /* Exact Track 02 map bytes, including the source-owned low-nibble
     * runtime attributes (for example teleporter OPEN = 0x08). */
    uint8_t source_tiles[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
    uint8_t squares[THERON_MAX_MAP_SIZE][THERON_MAX_MAP_SIZE];
    int   has_3d_geometry;
    int   geometry_offset;
    int   geometry_size;
    int   thing_count;
    int   creature_budget;      /* per-map creature_count from dungeon header */
} Theron_V1_Level;

/* ── Object database ───────────────────────────────────────────────── */

/* TQR "light" object types — subset of DM1 item field */
typedef enum {
    THERON_OBJTYPE_NONE         = 0,
    THERON_OBJTYPE_CHEST        = 1,
    THERON_OBJTYPE_DOOR         = 2,
    THERON_OBJTYPE_LEVER         = 3,
    THERON_OBJTYPE_BUTTON       = 4,
    THERON_OBJTYPE_POTION       = 5,
    THERON_OBJTYPE_SCROLL       = 6,
    THERON_OBJTYPE_FOOD         = 7,
    THERON_OBJTYPE_KEY          = 8,
    THERON_OBJTYPE_WEAPON       = 9,
    THERON_OBJTYPE_ARMOR        = 10,
    THERON_OBJTYPE_ALTAR_VI     = 11,   /* altar-of-vi resurrection point */
    THERON_OBJTYPE_TELEPORTER   = 12,   /* teleporter warp pad */
    THERON_OBJTYPE_POOL         = 13,   /* water/food recovery pool */
    THERON_OBJTYPE_ALARM        = 14,   /* alert all creatures */
    THERON_OBJTYPE_CREATURE_SPAWNER = 15, /* creature generator */
    THERON_OBJTYPE_TRIGGER      = 16,   /* event trigger */
    THERON_OBJTYPE_SOUND        = 17,   /* ambient/one-shot sound trigger */
    THERON_OBJTYPE_PIT          = 18,   /* pit trap object record */
    /* Neutral carrier for a real category-10 Track 02 misc record.  This is
     * deliberately not a guessed food/key/equipment meaning; the original
     * T900 consumer still owns that distinction. */
    THERON_OBJTYPE_SOURCE_ITEM  = 19,
    /* Neutral carrier for a real category-3 actuator.  It must not alias the
     * fixture BUTTON kind before the original linked-actuator consumer is
     * authenticated. */
    THERON_OBJTYPE_SOURCE_ACTUATOR = 20,
    THERON_OBJTYPE_QUEST_ITEM   = 128,
} Theron_V1_ObjectType;

#define THERON_OBJ_F_PICKED_UP   (1U << 0)
#define THERON_OBJ_F_OPENED      (1U << 1)
#define THERON_OBJ_F_ACTIVATED   (1U << 2)
#define THERON_OBJ_F_DESTROYED   (1U << 3)
#define THERON_OBJ_F_LOCKED      (1U << 4)
#define THERON_OBJ_F_USED        (1U << 5)
/* Track 02 teleporter records store linked_id as packed level/y/x fields,
 * not as a host object-table id.  Fixture object chains omit this bit and
 * retain the explicit object-id contract. */
#define THERON_OBJ_F_TRACK02_COORD_LINK (1U << 6)
/* Authenticated Track 02 door-record metadata.  Keep these bits separate
 * from THERON_DOOR_F_* and the generic object mutation flags: source
 * position/type describe the record and must never masquerade as LOCKED,
 * BROKEN, PICKED_UP, or another runtime state. */
#define THERON_OBJ_F_SOURCE_POSITION_SHIFT      28u
#define THERON_OBJ_F_SOURCE_POSITION_MASK       (3U << 28)
#define THERON_OBJ_F_SOURCE_ACTUATOR_ONCE        (1U << 20)
#define THERON_OBJ_F_SOURCE_ACTUATOR_SOUND       (1U << 21)
#define THERON_OBJ_F_SOURCE_ACTUATOR_LOCAL_EFFECT (1U << 22)
#define THERON_OBJ_F_SOURCE_ACTUATOR_REVERT_EFFECT (1U << 23)
#define THERON_OBJ_F_SOURCE_DOOR_IRON           (1U << 16)
#define THERON_OBJ_F_SOURCE_DOOR_OPENS_UP       (1U << 17)
#define THERON_OBJ_F_SOURCE_DOOR_BUTTON         (1U << 18)
#define THERON_OBJ_F_SOURCE_DOOR_DESTROYABLE    (1U << 19)
#define THERON_OBJ_F_SOURCE_DOOR_BASHABLE       (1U << 20)
#define THERON_OBJ_F_SOURCE_TELEPORTER_SCOPE_SHIFT 21u
#define THERON_OBJ_F_SOURCE_TELEPORTER_SCOPE_MASK  (3U << 21)

typedef struct {
    int      id;
    uint8_t  type;
    uint8_t  state;       /* 0=closed/locked, 1=open, 2=used, 3=broken */
    int      x, y;
    int      level;
    int      dungeon_id;
    int      quantity;
    int      item_index;  /* Track 02 item index (0-65), -1 = none/gold */
    int      linked_id;
    uint32_t flags;

    /* Source-bound Track 02 occurrence.  These fields preserve the exact
     * object record alongside the host kind; they do not imply inventory
     * ownership, pickup, equip, or use semantics. */
    uint16_t source_ref;
    uint16_t source_next_ref;
    uint16_t source_index;
    uint8_t  source_category;
    uint8_t  source_position;
    uint8_t  source_origin_valid;
    uint8_t  source_dungeon;
    uint8_t  source_level;
    uint8_t  source_x;
    uint8_t  source_y;
    uint8_t  source_raw_size;
    uint8_t  source_raw[16];
    uint8_t  source_item_type;
    uint8_t  source_keep;
    uint8_t  source_cursed;
    uint8_t  source_broken;
    uint8_t  source_poisoned;
    uint8_t  source_closed;
    uint8_t  source_dump;
    uint8_t  source_power;
    uint8_t  source_capacity;
    uint16_t source_text_ref;
    int16_t  source_chested;
    uint16_t source_data1;
    uint8_t  source_item_category;
    uint8_t  source_property_valid;
    uint8_t  source_property[6];
} Theron_V1_Object;

#define THERON_MAX_OBJECTS 4096

/* Raw Track 02 dungeon-text words retained by the loaded world.  These are
 * source bytes for the unresolved HuC6280 text consumer, not host strings. */
#define THERON_MAX_SOURCE_DUNGEON_TEXT 1024u
#define THERON_MAX_SOURCE_DUNGEON_TEXT_TOKENS \
    (THERON_MAX_SOURCE_DUNGEON_TEXT * 3u)

/* A decoded source monster remains available as provenance even after its
 * static group members are admitted to the live creature pool.  The live
 * admission copies only fields present in the record; combat/AI consumers
 * still refuse to invent missing attack, drop or sound semantics. */
#define THERON_MAX_SOURCE_MONSTERS 256

typedef struct {
    int dungeon_id;
    int level;
    int x;
    int y;
    uint16_t source_ref;
    uint16_t source_index;
    /* DMBUILDER6/src/dms.h:145-157: signed category-4 `chested` word.
     * This is source provenance only; T900 ownership semantics remain gated. */
    int16_t chested;
    uint8_t type;
    uint8_t position;
    uint8_t number;
    uint8_t direction_flags;
    uint16_t flags_word;
    uint16_t unknown_word;
    uint16_t health[4];
    /* DMBUILDER6 category-4 records are exactly 16 bytes. Preserve the raw
     * authenticated payload beside the decoded fields so later consumers
     * never need to reconstruct or guess source bits. */
    uint8_t raw_size;
    uint8_t raw[16];
} Theron_V1_SourceMonsterRecord;

/* Track 02 actuator type 6 is the source's floor monster-generator record.
 * Keep its decoded fields opaque to the live spawn path until the original
 * generator consumer and re-enable/timing semantics are bound. */
#define THERON_MAX_SOURCE_GENERATORS 64

typedef struct {
    int dungeon_id;
    int level;
    int x;
    int y;
    uint16_t source_ref;
    uint16_t source_index;
    uint8_t type;
    uint16_t value;
    uint8_t once;
    uint8_t effect;
    uint8_t revert_effect;
    uint8_t sound;
    uint8_t delay;
    uint8_t local_effect;
    uint8_t graphism;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t target_facing;
    uint8_t generator_fields_valid;
    uint8_t generator_generation;
    uint8_t generator_toughness;
    uint8_t generator_pause;
} Theron_V1_SourceGeneratorRecord;

/* Source occurrence for every decoded record.  The loader owns the
 * category/index/chain contract; this bank keeps the actual bytes and map
 * occurrence available after the temporary load result dies, including
 * records that also have a source-backed ground-object representation. */
#define THERON_MAX_SOURCE_OBJECT_RECORDS 4096

typedef struct {
    int dungeon_id;
    int level;
    int x;
    int y;
    uint16_t source_ref;
    uint16_t next_ref;
    uint16_t source_index;
    uint8_t category;
    uint8_t position;
    uint8_t raw_size;
    uint8_t raw[16];
} Theron_V1_SourceObjectRecord;

/* F0276/F0272 dispatch envelope produced from an authenticated category-3
 * floor-party record.  Queueing preserves the original decision and delay;
 * target mutation belongs to the separate, source-bound consumer. */
#define THERON_MAX_SOURCE_ACTUATOR_EVENTS 256u

typedef struct {
    uint64_t due_tick;
    uint16_t source_ref;
    uint16_t source_index;
    int16_t dungeon_id;
    int16_t level;
    uint8_t source_x;
    uint8_t source_y;
    uint8_t target_x;
    uint8_t target_y;
    uint8_t target_facing;
    uint8_t effect;
    uint8_t local_effect;
    uint8_t sound;
    uint8_t delay;
    uint16_t local_multiple;
} Theron_V1_SourceActuatorEvent;

#define THERON_MAX_SOURCE_SQUARE_STATES 1024u

/* Sparse mutable overlay for source map bits such as PIT/FAKEWALL OPEN.
 * source_tiles remains the exact Track 02 byte; only entries that differ
 * from that byte are retained here and serialized. */
typedef struct {
    int16_t dungeon_id;
    int16_t level;
    uint8_t x;
    uint8_t y;
    uint8_t tile;
} Theron_V1_SourceSquareState;

#define THERON_MAX_SOURCE_OBJECT_STATES 1024u

/* Sparse mutable replacement for source record word 1 (raw bytes 2..3).
 * The immutable source occurrence remains authoritative for identity and
 * all other fields. */
typedef struct {
    int16_t dungeon_id;
    int16_t level;
    uint16_t source_ref;
    uint16_t source_index;
    uint16_t word;
    uint8_t category;
} Theron_V1_SourceObjectState;

/* Runtime provenance for a carried Track 02 object.  The legacy champion
 * array keeps its compact item ID for compatibility; this parallel record
 * preserves the source payload needed by the future T900 inventory/equip/use
 * consumer without pretending that a pickup is already an authenticated
 * save-format or T900 rule. */
typedef struct {
    uint8_t valid;
    uint8_t category;
    uint8_t item_type;
    uint8_t keep;
    uint8_t cursed;
    uint8_t broken;
    uint8_t poisoned;
    uint8_t closed;
    uint8_t dump;
    uint8_t power;
    uint8_t charges;
    uint16_t source_ref;
    uint16_t source_next_ref;
    uint16_t source_index;
    uint8_t source_position;
    uint8_t source_origin_valid;
    uint8_t source_dungeon;
    uint8_t source_level;
    uint8_t source_x;
    uint8_t source_y;
    uint16_t text_ref;
    int16_t chested;
    uint16_t data1;
    uint8_t item_category;
    uint8_t property_valid;
    uint8_t property[6];
    /* Preserve the complete authenticated Track 02 item record across
     * pickup, drop and save/load.  This is provenance, not a T900 semantic
     * interpretation of the bytes. */
    uint8_t source_raw_size;
    uint8_t source_raw[16];
} Theron_V1_InventorySourceRecord;

/* ── Timer system ─────────────────────────────────────────────────── */
typedef enum {
    THERON_TIMER_ONESHOT   = 0,
    THERON_TIMER_REPEAT    = 1,
    THERON_TIMER_COUNTDOWN = 2,
} Theron_TimerKind;

#define THERON_TIMER_F_ACTIVE  (1U << 0)
#define THERON_TIMER_F_PAUSED (1U << 1)
#define THERON_TIMER_F_DESTROY (1U << 2)

typedef struct {
    int       id;
    Theron_TimerKind kind;
    int       level;
    int       remaining_ticks;
    int       interval_ticks;
    uint32_t  flags;
    void     *userdata;
} Theron_V1_Timer;

/* ── Map loading result codes ─────────────────────────────────────── */
typedef enum {
    THERON_MAP_OK                =  0,
    THERON_MAP_ERR_NULL          = -1,
    THERON_MAP_ERR_SIZE_TOO_SMALL = -2,
    THERON_MAP_ERR_INVALID_GRID  = -3,
    THERON_MAP_ERR_NO_ENTRANCE   = -4,
} Theron_MapLoadResult;

/* ── Transition result codes ───────────────────────────────────────── */
typedef enum {
    THERON_TRANSITION_STAIRS     = 1,
    THERON_TRANSITION_TELEPORTER = 2,
    THERON_TRANSITION_EXIT       = 3,
} Theron_TransitionType;

#define THERON_SQUARE_TO_TRANSITION_TYPE(t) \
    (((t) == THERON_SQUARE_STAIRS_UP || (t) == THERON_SQUARE_STAIRS_DOWN) \
     ? THERON_TRANSITION_STAIRS \
     : ((t) == THERON_SQUARE_TELEPORTER) \
       ? THERON_TRANSITION_TELEPORTER \
       : ((t) == THERON_SQUARE_EXIT) \
         ? THERON_TRANSITION_EXIT : 0)

/* ── World state struct ────────────────────────────────────────────── */
struct Theron_V1_World {
    /* Dungeon maps — levels[7][8]: [dungeon_id-1][map_index]. */
    Theron_V1_Level levels[THERON_DUNGEON_COUNT][THERON_MAX_LEVELS_PER_DUNGEON];
    int level_loaded[THERON_DUNGEON_COUNT][THERON_MAX_LEVELS_PER_DUNGEON];

    /* Real Track 02 map-directory metadata retained per dungeon. The
     * descriptor widths and aggregate column count are source receipts only;
     * they do not publish an object layout or graphics route. */
    uint8_t source_thing_descriptor_sizes[THERON_DUNGEON_COUNT][12];
    uint16_t source_column_thing_count_total[THERON_DUNGEON_COUNT];
    uint8_t source_thing_directory_verified[THERON_DUNGEON_COUNT];

    /* Dungeon progression (quest items, dungeon sequence) */
    Theron_DungeonProgression progression;

    /* Party: Theron plus up to 3 companions */
    Theron_V1_Party party;

    /* Object pool */
    Theron_V1_Object objects[THERON_MAX_OBJECTS];
    int object_count;

    /* Creature pool (current level only; respawned on dungeon entry).
     * Source: THQUEST.ASM T500/T600 creature spawn + combat resolution. */
    Theron_V1_Creature creatures[THERON_MAX_CREATURES_PER_LEVEL];
    int creature_count;

    /* Authentic category-4 thing-list records from the loaded Track 02
     * dungeon. Static groups may be admitted to the live creature pool with
     * source identity; random spawn, AI, combat and loot remain separate
     * source-consumer gates. */
    Theron_V1_SourceMonsterRecord source_monsters[THERON_MAX_SOURCE_MONSTERS];
    unsigned int source_monster_count;
    /* The US raw Track 02 pointer/zone records are copied here only after
     * the complete BIN hash and roster marker pass.  A zero/JP variant keeps
     * regular-spawn category publication gated because the US offsets are not
     * valid for JP. */
    Theron_Track02SpawnSource track02_spawn_source;
    int track02_spawn_source_variant;
    /* Complete authentic Track 19 name table for the selected region.
     * Names are addressable only by their explicit Track 19 table index;
     * no Track 02 object/type mapping or JP host-font rendering is implied. */
    Theron_V1Track19ItemNameBank track19_item_names;
    Theron_Track02ItemNameSource
        track02_item_names[THERON_DUNGEON_COUNT];
    /* Seven regional completion-message records from the authenticated
     * Track 02 UI bank.  This source is addressable by record index only;
     * it does not promote an inventory action to a retrieval event. */
    Theron_Track02RetrievalTextSource track02_retrieval_text;
    /* Hash-verified original campaign/dungeon byte at $267c. This receipt is
     * evidence only and must remain detached from artifact collection until
     * an original event consumer proves that relation. */
    Theron_Track02CampaignMaskSource track02_campaign_mask;
    Theron_V1_SourceGeneratorRecord
        source_generators[THERON_MAX_SOURCE_GENERATORS];
    unsigned int source_generator_count;
    Theron_V1_SourceObjectRecord
        source_objects[THERON_MAX_SOURCE_OBJECT_RECORDS];
    unsigned int source_object_count;
    Theron_V1_SourceActuatorEvent
        source_actuator_events[THERON_MAX_SOURCE_ACTUATOR_EVENTS];
    unsigned int source_actuator_event_count;
    Theron_V1_SourceSquareState
        source_square_states[THERON_MAX_SOURCE_SQUARE_STATES];
    unsigned int source_square_state_count;
    Theron_V1_SourceObjectState
        source_object_states[THERON_MAX_SOURCE_OBJECT_STATES];
    unsigned int source_object_state_count;
    Theron_V1_InventorySourceRecord
        inventory_source[THERON_MAX_CHAMPIONS][THERON_INVENTORY_SLOTS];

    /* Generator state (current dungeon, current level only) */
    /* Runtime slots mirror the authenticated source-generator bank.  The
     * old five-slot array was a fixture limit and silently truncated real
     * Track 02 maps (some contain 14 generator records). */
    int generator_spawn_count[THERON_MAX_SOURCE_GENERATORS];
    uint64_t generator_next_tick[THERON_MAX_SOURCE_GENERATORS];
    int generator_active_count;

    /* Timer pool */
    Theron_V1_Timer timers[THERON_MAX_TIMERS];
    int timer_count;

    /* World tick */
    uint64_t world_tick;

    /* Current dungeon/location */
    int current_dungeon;   /* 1..7 */
    int current_level;     /* 0..2 */

    /* Transition queue */
    int transition_pending;
    Theron_TransitionType transition_type;
    int transition_target_level;
    int transition_spawn_x, transition_spawn_y;

    /* Quest item tracking */
    uint8_t quest_items_in_dungeon;
    uint8_t dungeon_complete;

    /* Entry guard */
    int entry_reset_applied;

    /* Track 02 visual state restored together with a resumed world. */
    Theron_RuntimeLevelMedia runtime_media;

    /* Decoded dungeon text messages (wall plaques, scrolls).
     * Loaded from Track 02 thing data text block per dungeon. */
    unsigned int dungeon_text_count;
    char dungeon_texts[64][256];

    /* The diagnostic/UI text table above intentionally stays empty while the
     * original control-code consumer is unresolved.  Keep the authenticated
     * little-endian source words in the live world so a later consumer join
     * does not have to reload or reconstruct them. */
    int source_dungeon_text_dungeon_id;
    unsigned int source_dungeon_text_count;
    uint16_t source_dungeon_text[THERON_MAX_SOURCE_DUNGEON_TEXT];
    /* Positional codec provenance for the same source words.  Token kind is
     * limited to raw/control/end at the codec boundary; it does not assign
     * the original HuC6280 control-code meaning or a UI destination. */
    unsigned int source_dungeon_text_token_count;
    Theron_TextToken source_dungeon_text_tokens[
        THERON_MAX_SOURCE_DUNGEON_TEXT_TOKENS];

    /* Deterministic state hash */
    uint64_t state_hash;
};

/* ── Initialization ───────────────────────────────────────────────── */
void theron_v1_world_init(Theron_V1_World *world);

/* Production startup initialization.  Unlike the legacy fixture-oriented
 * initializer, this never invents a champion roster or default stats before
 * source records have been admitted. */
void theron_v1_world_init_runtime(Theron_V1_World *world);
void theron_v1_world_reset_for_dungeon(Theron_V1_World *world,
                                         Theron_DungeonID dungeon_id);

/* ── Map loading ─────────────────────────────────────────────────── */
/*
 * TQR 12-byte header + grid:
 *   bytes 0-1:  width  (big-endian uint16_t, LE on disk)
 *   bytes 2-3:  height (big-endian uint16_t, LE on disk)
 *   bytes 4-7:  dungeon_seed (uint32_t LE)
 *   bytes 8-9:  level_index (uint16_t LE)
 *   bytes 10-11: reserved
 *   byte 12+:  W×H grid of uint8_t tile values
 */
Theron_MapLoadResult theron_v1_level_load(Theron_V1_Level *level,
                                           const uint8_t *data,
                                           int data_size,
                                           int dungeon_id,
                                           int sub_level_index);

uint8_t theron_v1_world_get_square(const Theron_V1_World *world, int x, int y);
int theron_v1_world_track02_runtime_tile(
    const Theron_V1_World *world, int dungeon_id, int level, int x, int y,
    uint8_t *out_tile);
int theron_v1_world_track02_runtime_object_word(
    const Theron_V1_World *world,
    const Theron_V1_SourceObjectRecord *source, uint16_t *out_word);

/* Decode dungeon text data into the world's text table.
 * codons/count come from Theron_ThingData.text_data/text_data_count. */
int theron_v1_world_load_dungeon_text(Theron_V1_World *world,
                                       const uint16_t *codons,
                                       unsigned int codon_count);

const char *theron_v1_world_dungeon_text(const Theron_V1_World *world,
                                          unsigned int text_index);
unsigned int theron_v1_world_source_dungeon_text_count(
    const Theron_V1_World *world);
int theron_v1_world_source_dungeon_text_word(
    const Theron_V1_World *world,
    unsigned int word_index,
    uint16_t *out_word);
unsigned int theron_v1_world_source_dungeon_text_token_count(
    const Theron_V1_World *world);
int theron_v1_world_source_dungeon_text_token(
    const Theron_V1_World *world,
    unsigned int token_index,
    Theron_TextToken *out_token);
void theron_v1_party_place(Theron_V1_World *world, int x, int y, int dir);

/* ── Object database API ─────────────────────────────────────────── */
int theron_v1_object_place(Theron_V1_World *world, Theron_V1_Object *object);
int theron_v1_object_remove(Theron_V1_World *world, int id);
Theron_V1_Object *theron_v1_object_at(Theron_V1_World *world, int level, int x, int y);
/* Production lookup: an object belongs to the selected dungeon as well as
 * its level and square.  The legacy object_at() remains for fixture callers
 * that intentionally omit dungeon scope. */
Theron_V1_Object *theron_v1_object_at_in_dungeon(
    Theron_V1_World *world, int dungeon_id, int level, int x, int y);
Theron_V1_Object *theron_v1_object_by_id(Theron_V1_World *world, int id);
const Theron_V1_InventorySourceRecord *theron_v1_inventory_source_at(
    const Theron_V1_World *world, int champion_slot, int inventory_slot);
/* Provenance-preserving storage operation. This moves a compact inventory ID
 * together with its complete authenticated Track 02 source record. It is not
 * an assertion of the original T900 equip/use/stack command grammar. */
int theron_v1_swap_inventory_source_slots(
    Theron_V1_World *world,
    int champion_slot,
    int inventory_slot_a,
    int inventory_slot_b);
/* Source-backed inventory roundtrip.  This is deliberately an explicit
 * provenance API, not an assertion of the original T900 drop command. */
int theron_v1_drop_inventory_source_item(
    Theron_V1_World *world,
    int champion_slot,
    int inventory_slot,
    int x,
    int y);
int theron_v1_object_set_state(Theron_V1_World *world, int id, uint8_t new_state);
int theron_v1_object_set_flag(Theron_V1_World *world, int id, uint32_t flag);
int theron_v1_object_clear_flag(Theron_V1_World *world, int id, uint32_t flag);

/* Apply a decoded Track 02 compact object table to one loaded level.
 * Door and teleporter records update both the object database and the grid
 * tile so movement/click mechanics resolve them correctly.  Other object
 * kinds are placed as objects on their existing tile.  Unsupported or out-of-
 * bounds records are skipped.  Returns 0 on success, -1 on invalid input. */
int theron_v1_world_apply_track02_object_table(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    const Theron_Track02ObjectTable *table);

/* Apply a decoded Track 02 compact object table to every loaded level of a
 * dungeon.  Records are routed to the level that matches their level_index.
 * Returns 0 on success, -1 on invalid input. */
int theron_v1_world_apply_track02_object_table_for_dungeon(
    Theron_V1_World *world,
    int dungeon_id,
    const Theron_Track02ObjectTable *table);

/* Load all maps from a decoded Track 02 quest block into the world model.
 * Converts Track 02 tile types to world square types and sets level geometry.
 * dungeon_id is 1-based (THERON_DUNGEON_1_AKUTUBA..THERON_DUNGEON_7_DEMON).
 * Returns the number of levels loaded, or -1 on error. */
struct Theron_DungeonData;
int theron_v1_world_load_track02_dungeon(
    Theron_V1_World *world,
    int dungeon_id,
    const struct Theron_DungeonData *dd);

/* ── Timer API ───────────────────────────────────────────────────── */
int  theron_v1_timer_add(Theron_V1_World *world,
                         Theron_TimerKind kind,
                         int level,
                         int remaining_ticks,
                         int interval_ticks,
                         void *userdata);
void theron_v1_timer_remove(Theron_V1_World *world, int id);
void theron_v1_timer_pause(Theron_V1_World *world, int id);
void theron_v1_timer_resume(Theron_V1_World *world, int id);
void theron_v1_tick_timers(Theron_V1_World *world);
void theron_v1_timers_clear_level(Theron_V1_World *world, int level);

/* ── Level transitions ────────────────────────────────────────────── */
Theron_TransitionType theron_v1_check_transition(Theron_V1_World *world, int x, int y);
int theron_v1_transition_execute(Theron_V1_World *world);
int theron_v1_world_spawn_level_creatures(Theron_V1_World *world);
int theron_v1_world_bind_track02_spawn_source(
    Theron_V1_World *world,
    const Theron_Track02SpawnSource *source,
    int variant);
int theron_v1_world_bind_track19_item_name_bank(
    Theron_V1_World *world,
    const Theron_V1Track19ItemNameBank *bank,
    int variant);
int theron_v1_world_bind_track02_item_name_source(
    Theron_V1_World *world,
    const Theron_Track02ItemNameSource *source,
    int variant);
int theron_v1_world_track02_item_name_raw(
    const Theron_V1_World *world,
    unsigned int dungeon_id,
    unsigned int item_index,
    const uint8_t **out_bytes,
    size_t *out_size);
int theron_v1_world_quest_item_name_raw(
    const Theron_V1_World *world,
    unsigned int quest_index,
    const uint8_t **out_bytes,
    size_t *out_size);
int theron_v1_world_bind_track02_retrieval_text_source(
    Theron_V1_World *world,
    const Theron_Track02RetrievalTextSource *source,
    int variant);
int theron_v1_world_retrieval_text_record_raw(
    const Theron_V1_World *world,
    unsigned int record_index,
    const uint8_t **out_bytes,
    size_t *out_size);
int theron_v1_world_bind_track02_campaign_mask_source(
    Theron_V1_World *world,
    const Theron_Track02CampaignMaskSource *source,
    int variant);

/* Decode the seven artifact-completion bits from original RAM $267C only
 * after the world has cross-bound the campaign writer and retrieval-message
 * selector. Bit 7 is preserved by the original serializer but is not part of
 * the artifact mask. */
int theron_v1_world_campaign_artifact_mask(
    const Theron_V1_World *world,
    uint8_t serialized_campaign_byte,
    uint8_t *out_artifact_mask);

/* Apply an original DMS-SG.001 byte loaded at RAM $267C to the live
 * progression. The current dungeon, level, timers and seeds remain owned by
 * the existing world; only source-proven completion states are refreshed. */
int theron_v1_world_apply_campaign_artifact_byte(
    Theron_V1_World *world,
    uint8_t serialized_campaign_byte);
int theron_v1_world_object_item_name_raw(
    const Theron_V1_World *world,
    const Theron_V1_Object *object,
    const uint8_t **out_bytes,
    size_t *out_size);
int theron_v1_world_object_item_type_code(
    const Theron_V1_World *world,
    const Theron_V1_Object *object,
    uint8_t *out_code);
int theron_v1_world_object_item_property_raw(
    const Theron_V1_World *world,
    const Theron_V1_Object *object,
    const uint8_t **out_bytes,
    size_t *out_size);
int theron_v1_world_track19_item_name_raw(
    const Theron_V1_World *world,
    unsigned int track19_index,
    const uint8_t **out_bytes,
    size_t *out_size);
int theron_v1_world_object_track19_item_name_raw(
    const Theron_V1_World *world,
    const Theron_V1_Object *object,
    const uint8_t **out_bytes,
    size_t *out_size);
int theron_v1_world_inventory_source_track19_item_name_raw(
    const Theron_V1_World *world,
    int champion_slot,
    int inventory_slot,
    const uint8_t **out_bytes,
    size_t *out_size);
uint8_t theron_v1_world_track02_spawn_category(
    const Theron_V1_World *world,
    unsigned int creature_index);
int theron_v1_world_bind_track02_monster(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    uint16_t source_ref,
    uint16_t source_index,
    int x,
    int y,
    uint8_t type,
    uint8_t position,
    const uint16_t health[4],
    uint8_t number,
    uint8_t direction_flags,
    uint16_t flags_word,
    uint16_t unknown_word,
    int16_t chested);
int theron_v1_world_bind_track02_generator(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    uint16_t source_ref,
    uint16_t source_index,
    int x,
    int y,
    uint8_t type,
    uint16_t value,
    uint8_t once,
    uint8_t effect,
    uint8_t revert_effect,
    uint8_t sound,
    uint8_t delay,
    uint8_t local_effect,
    uint8_t graphism,
    uint8_t target_x,
    uint8_t target_y,
    uint8_t target_facing,
    uint8_t generator_fields_valid,
    uint8_t generator_generation,
    uint8_t generator_toughness,
    uint8_t generator_pause);
int theron_v1_world_bind_track02_source_object(
    Theron_V1_World *world,
    int dungeon_id,
    int level_index,
    uint16_t source_ref,
    uint16_t next_ref,
    uint16_t source_index,
    uint8_t category,
    uint8_t position,
    int x,
    int y,
    const uint8_t *raw,
    uint8_t raw_size);
int theron_v1_world_queue_track02_party_events(
    Theron_V1_World *world, int level, int x, int y,
    int is_addition, int party_already_on_square,
    unsigned int party_direction);

/* Resolve one verified floor-party event to the unique authentic Track 02
 * monster-generator record at its target square. This publishes identity
 * only: it consumes no RNG, schedules no timer and creates no creature. */
int theron_v1_world_resolve_track02_generator_event(
    const Theron_V1_World *world,
    const Theron_V1_SourceActuatorEvent *event,
    unsigned int *out_generator_index);

/* Dynamic provenance required before a resolved source generator may create
 * a group.  The raw records must be copied from the same authenticated
 * Track 02 execution as the RNG edge.  The *_verified flags are outputs
 * of capture/source correlation, not permissions a gameplay caller may
 * synthesize. */
typedef struct {
    int authenticated_track02_execution;
    int same_execution_window_verified;
    int rng_return_boundary_verified;
    int rng_caller_source_bytes_verified;
    int generator_consumer_contract_verified;
    uint32_t capture_sequence;
    uint16_t rng_entry_pc;
    uint32_t rng_physical_entry_pc;
    uint16_t rng_caller_pc;
    uint32_t rng_physical_caller_pc;
    uint8_t rng_return_value;
    uint8_t rng_caller_source_size;
    uint8_t rng_caller_source[32];
    uint8_t rng_state_before[3];
    uint8_t rng_state_after[3];
    uint16_t successor_caller_pc;
    uint32_t successor_physical_caller_pc;
    uint8_t successor_state_before[3];
    int runtime_materialization_verified;
    uint16_t runtime_copy_pc;
    uint32_t runtime_physical_copy_pc;
    uint8_t runtime_slot;
    uint16_t runtime_record_address;
    uint8_t runtime_record[10];
    int runtime_first_consumer_verified;
    uint16_t runtime_first_consumer_pc[3];
    uint32_t runtime_first_consumer_physical_pc[3];
    uint8_t runtime_first_consumer_value[3];
    int runtime_unlink_verified;
    uint16_t runtime_unlink_pc;
    uint32_t runtime_unlink_physical_pc;
    int runtime_lifecycle_window_verified;
    uint32_t runtime_lifecycle_sequence[8];
    uint16_t runtime_lifecycle_pc[8];
    uint32_t runtime_lifecycle_physical_pc[8];
    int runtime_position_consumer_source_verified;
    uint16_t runtime_position_consumer_pc;
    uint32_t runtime_position_consumer_raw_offset;
    uint8_t runtime_position_consumer_source_size;
    uint8_t runtime_position_consumer_source[27];
    uint8_t event_raw[8];
    uint8_t generator_raw[8];
} Theron_V1_GeneratorExecutionWitness;

typedef struct {
    int event_identity_verified;
    int event_raw_verified;
    int generator_identity_verified;
    int generator_raw_verified;
    int event_bound_rng_witness_verified;
    int materialization_allowed;
    unsigned int generator_index;
    uint16_t creature_type_value;
    uint8_t count_is_random;
    uint8_t fixed_count_minus_one;
    uint8_t random_count_bound;
    uint8_t toughness;
    uint8_t pause;
    uint8_t rng_return_value;
    uint8_t runtime_slot;
    uint16_t runtime_record_address;
    uint8_t runtime_record[10];
    uint16_t runtime_first_consumer_pc[3];
    uint32_t runtime_first_consumer_physical_pc[3];
    uint8_t runtime_first_consumer_value[3];
    uint16_t runtime_unlink_pc;
    uint32_t runtime_unlink_physical_pc;
    int runtime_lifecycle_window_verified;
    uint32_t runtime_lifecycle_sequence[8];
    uint16_t runtime_lifecycle_pc[8];
    uint32_t runtime_lifecycle_physical_pc[8];
    /* Legacy field names below describe the working hypothesis only.  The
     * authenticated C852 span proves byte copies and the two-bit transform,
     * not host-local coordinates or a direction consumer. */
    int runtime_position_fields_source_verified;
    uint8_t runtime_x;
    uint8_t runtime_y;
    uint8_t runtime_direction_raw;
    uint8_t runtime_direction;
} Theron_V1_GeneratorMaterializationReceipt;

/* Correlates an event, its unique type-6 record and a live RNG witness.
 * Returns 1 only when all source and execution joins are complete.  It does
 * not consume the event or create a creature. */
int theron_v1_world_bind_track02_generator_execution_witness(
    const Theron_V1_World *world,
    const Theron_V1_SourceActuatorEvent *event,
    const Theron_V1_GeneratorExecutionWitness *witness,
    Theron_V1_GeneratorMaterializationReceipt *out);
void theron_v1_world_init_generators(Theron_V1_World *world);
void theron_v1_world_tick_generators(Theron_V1_World *world);

/* ── World tick ───────────────────────────────────────────────────── */
void theron_v1_world_tick(Theron_V1_World *world);

/* ── Decoded Track 02 runtime media ───────────────────────────────── */
void theron_v1_world_runtime_media_clear(Theron_V1_World *world);
void theron_v1_world_runtime_media_invalidate_cache(Theron_V1_World *world);
int theron_v1_world_runtime_media_set_surface(
    Theron_V1_World *world,
    Theron_RuntimeMediaSurfaceKind kind,
    const char *track02_md5,
    unsigned int route_bit,
    uint16_t width,
    uint16_t height,
    size_t first_raw_offset,
    size_t last_raw_offset,
    size_t first_user_data_offset,
    size_t tile_count,
    size_t nonzero_pixel_count,
    uint32_t checksum,
    const uint8_t *pixels,
    size_t pixel_count);
const Theron_RuntimeMediaSurface *theron_v1_world_runtime_media_for_level(
    const Theron_V1_World *world,
    int level_index,
    int forcefield_active);
int theron_v1_world_runtime_media_set_identity(
    Theron_V1_World *world,
    const Theron_RuntimeMediaIdentity *identity);
int theron_v1_world_runtime_media_set_loader_record(
    Theron_V1_World *world,
    const char *track02_md5,
    uint32_t record,
    uint32_t destination,
    size_t raw_user_data_offset,
    size_t payload_bytes,
    uint32_t payload_checksum,
    size_t level_envelope_offset,
    size_t level_envelope_bytes,
    uint32_t level_envelope_checksum,
    size_t post_envelope_offset,
    size_t post_envelope_bytes,
    uint32_t post_envelope_checksum);
int theron_v1_world_runtime_media_select_level_bank(
    Theron_V1_World *world,
    Theron_RuntimeLevelBankKind kind,
    Theron_DungeonID dungeon_id,
    int level_index);
int theron_v1_world_runtime_media_bind_level_data_block(
    Theron_V1_World *world,
    const uint8_t *user_data,
    size_t user_data_size,
    int track02_variant,
    unsigned int level);

/* ── Deterministic world hash (FNV-1a 64-bit) ─────────────────────── */
#define THERON_HASH_SEED_PARTY     0x50415254UL  /* 'PART' */
#define THERON_HASH_SEED_OBJECT    0x4F424A45UL  /* 'OBJE' */
#define THERON_HASH_SEED_CREATURE  0x43524554UL  /* 'CRET' */
#define THERON_HASH_SEED_TIMER     0x54494D45UL  /* 'TIME' */
#define THERON_HASH_SEED_DUNG      0x444E4753UL  /* 'DNGS' */
#define THERON_HASH_SEED_SOURCE    0x53524345UL  /* 'SRCE' */
#define THERON_HASH_FNV_OFFSET  0xCBF29CE484222325UL
#define THERON_HASH_FNV_PRIME   0x00000100000001B3UL

uint64_t theron_v1_world_hash(const Theron_V1_World *world);
void     theron_v1_world_hash_inject(Theron_V1_World *world, uint64_t seed);

/* ── Quest item helpers ─────────────────────────────────────────────── */
int   theron_v1_check_quest_item(const Theron_V1_World *world);
uint8_t theron_v1_collect_quest_item(Theron_V1_World *world, uint8_t item_bit);

/* ── Portable in-memory world snapshots ────────────────────────────
 * This private TRNW format is used for deterministic world round-trips.
 * It is separate from the user-facing between-dungeon slotN.tqsv format in
 * theron_v1_save_load.h; changing this version does not change TQSV. */
#define THERON_WORLD_SAVE_MAGIC   0x574E5254U  /* 'TRNW' */
#define THERON_WORLD_SAVE_VERSION 18

size_t theron_v1_world_serialize_size(const Theron_V1_World *world);
size_t theron_v1_world_serialize(const Theron_V1_World *world,
                                  void *buf, size_t bufsize);
int    theron_v1_world_deserialize(Theron_V1_World *world,
                                    const void *buf, size_t bufsize);

/* ── Source citation ───────────────────────────────────────────────── */
const char *theron_v1_world_source_evidence(void);

/* ── First-room synthetic fixtures (Phase 4 startup proof) ─────────── */
/*
 * theron_v1_first_room_default_width / height
 *   Canonical first-room dimensions for the Theron's Quest V1 startup
 *   probe.  Matches the documented TQR mini-dungeon size used in
 *   docs/source-lock/tqr_v1_phase1_boot_H2338.md and the existing
 *   theron_v1_level_load header contract.
 *
 *   The default 20x20 grid + 12-byte header = 412-byte fixture buffer
 *   total, which is small enough to allocate on the stack of a
 *   headless probe and large enough to prove movement, wall-block,
 *   and party placement.
 */
#define THERON_V1_FIRST_ROOM_DEFAULT_WIDTH  20
#define THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT 20
#define THERON_V1_FIRST_ROOM_HEADER_BYTES   12
#define THERON_V1_FIRST_ROOM_DEFAULT_SIZE   \
    (THERON_V1_FIRST_ROOM_HEADER_BYTES + \
     THERON_V1_FIRST_ROOM_DEFAULT_WIDTH * \
     THERON_V1_FIRST_ROOM_DEFAULT_HEIGHT)

/*
 * theron_v1_first_room_buffer_size — bytes required to hold a synthetic
 *   first-room fixture of the given dimensions.  Returns
 *   THERON_V1_FIRST_ROOM_HEADER_BYTES + width * height.
 */
size_t theron_v1_first_room_buffer_size(int width, int height);

/*
 * theron_v1_first_room_synthesize — fill `out_buf` with a deterministic
 *   first-room fixture using the documented 12-byte header contract.
 *
 * The header layout matches theron_v1_level_load() in
 * src/theron/theron_v1_world.c:
 *   bytes 0-1:  width  (big-endian uint16_t, LE on disk)
 *   bytes 2-3:  height (big-endian uint16_t, LE on disk)
 *   bytes 4-7:  dungeon_seed (uint32_t BE)
 *   bytes 8-9:  level_index (uint16_t BE)
 *   bytes 10-11: reserved (0)
 *
 * The grid is a fully walled room with a single floor tile at
 * (start_x, start_y), one floor tile to the east (so the probe can
 * also verify a successful forward step), and an exit/stairs tile
 * down-and-to-the-east so wall-block and forward-step directions
 * are unambiguous.
 *
 * On success, out_level->start_x / start_y are written to the
 * entrance coordinates.  Returns the total byte count written, or
 * 0 on invalid inputs (NULL / out of range / size too small).
 *
 * Source-lock: THQUEST.ASM T520 (party placement) and
 *   THQUEST.ASM T560 (dungeon loading, 12-byte header) — see
 *   docs/source-lock/tqr_v1_phase1_boot_H2338.md.
 *
 * Negative fixtures:
 *   - NULL out_buf or level -> 0
 *   - width or height <= 0 or > THERON_MAX_MAP_SIZE -> 0
 *   - buf_size < THERON_V1_FIRST_ROOM_HEADER_BYTES + width*height -> 0
 */
size_t theron_v1_first_room_synthesize(uint8_t *out_buf,
                                        size_t buf_size,
                                        int width,
                                        int height,
                                        int level_index,
                                        uint32_t dungeon_seed,
                                        Theron_V1_Level *out_level);

/* Build the bounded startup fixture room for a selected Theron's Quest
 * stage.  This compatibility helper is reserved for data-free probes and
 * legacy fixture callers; it is not a production launch source. M11 must
 * use the verified-only Track 02 handoff instead. The returned buffer uses
 * the same 12-byte header and grid contract as theron_v1_level_load();
 * out_level receives a preview with stage-specific dimensions, start pose,
 * exit, and marker tile. Returns bytes written, or 0 on invalid input. */
size_t theron_v1_startup_fallback_room_synthesize(uint8_t *out_buf,
                                                   size_t buf_size,
                                                   Theron_DungeonID dungeon_id,
                                                   Theron_V1_Level *out_level);

/* ── Startup runtime readiness (skip-safe) ──────────────────────────── */

typedef enum {
    THERON_RUNTIME_READINESS_OK              =  0, /* Track 02 staged + boot path viable */
    THERON_RUNTIME_READINESS_NO_DATA_ROOT    = -1, /* data_dir is NULL or empty */
    THERON_RUNTIME_READINESS_NO_TRACK02      = -2, /* no known Track 02 file found */
    THERON_RUNTIME_READINESS_NOT_VERIFIED    = -3, /* file found but MD5 not on the four-MD5 list */
    THERON_RUNTIME_READINESS_BAD_INPUT       = -4  /* NULL scan_out / mismatch */
} Theron_RuntimeReadinessStatus;

/*
 * theron_v1_runtime_readiness — skip-safe gate used by the V1 startup
 *   probe before exercising the real-Track-02 path.
 *
 * Walks `data_root` looking for one of the documented Track 02
 * filenames (jp/us, bin/iso).  When a candidate exists, it
 * independently hashes the file with libcs md5-style fallback and
 * compares the result against the four locked-in Track 02 MD5s:
 *
 *     JP Track 02 BIN: b7afb338ad31be1025b53f9aff12d73a
 *     US Track 02 BIN: f23601102138f87c33025877767ebf76
 *     JP Rev 1 ISO:    397039af02d50d15c70b74088eb8a1cb
 *     US ISO:          ceb02343868f80cec899e9b239aff2da
 *
 * On success (THERON_RUNTIME_READINESS_OK) scan_out is filled with
 * the resolved path and matching MD5.  When the data root is empty
 * or the file is not hash-verified, the function returns one of the
 * NO_* statuses and writes empty strings into scan_out.  This lets
 * probes report SKIP without lying about full playability.
 *
 * libcs dependency: stdio + string only.  No crypto dependency.
 *
 * Source-lock: src/shared/asset_status_m12.c hash-verified catalog.
 */
Theron_RuntimeReadinessStatus theron_v1_runtime_readiness(
    const char *data_root,
    char *scan_out,
    size_t scan_out_size,
    char *md5_out,
    size_t md5_out_size);

/* Human-readable name for a readiness status. */
const char *theron_v1_runtime_readiness_status_name(
    Theron_RuntimeReadinessStatus status);

#ifdef __cplusplus
}
#endif

#endif /* THERON_V1_WORLD_H */
