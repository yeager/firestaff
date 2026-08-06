#ifndef THERON_V1_WORLD_H
#define THERON_V1_WORLD_H

#include "theron_v1_champions.h"

/* Forward typedef so theron_v1_combat.h prototypes can use Theron_V1_World*
 * without a conflicting redefinition when this header later defines the
 * struct body. */
typedef struct Theron_V1_World Theron_V1_World;
#define THERON_V1_WORLD_TYPEDEF

/* Forward typedef for the Track 02 compact object table used by the real-data
 * initial-level decoder.  The full struct is defined in theron_v1_track02.h. */
typedef struct Theron_Track02ObjectTable Theron_Track02ObjectTable;

#include "theron_v1_combat.h"
#include "theron_v1_dungeon_progression.h"
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
    size_t compressed_bytes;
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
} Theron_RuntimeLevelMedia;

/* ── Square tile types ────────────────────────────────────────────── */
#define THERON_SQUARE_WALL           0
#define THERON_SQUARE_FLOOR          1
#define THERON_SQUARE_DOOR           4   /* open/close on step/command */
#define THERON_SQUARE_PIT            2   /* trap — wounds champion */
#define THERON_SQUARE_STAIRS_UP      3   /* level N → level N-1 */
#define THERON_SQUARE_STAIRS_DOWN    13  /* level N → level N+1 */
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
    /* Per-map source records retained from the real Track 02 map-bank
     * directory. These are provenance fields only: no creature graphics or
     * thing-list semantics are inferred from them. */
    uint16_t source_creature_gfx_bank;
    uint16_t source_cumulative_column_items;
    int   start_x, start_y;    /* party spawn position (THQUEST.ASM T520) */
    int   start_dir;            /* 0=N 1=E 2=S 3=W */
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
} Theron_V1_Object;

#define THERON_MAX_OBJECTS 512

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

    /* Generator state (current dungeon, current level only) */
    int generator_spawn_count[5];
    uint64_t generator_next_tick[5];
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

/* Decode dungeon text data into the world's text table.
 * codons/count come from Theron_ThingData.text_data/text_data_count. */
int theron_v1_world_load_dungeon_text(Theron_V1_World *world,
                                       const uint16_t *codons,
                                       unsigned int codon_count);

const char *theron_v1_world_dungeon_text(const Theron_V1_World *world,
                                          unsigned int text_index);
void theron_v1_party_place(Theron_V1_World *world, int x, int y, int dir);

/* ── Object database API ─────────────────────────────────────────── */
int theron_v1_object_place(Theron_V1_World *world, Theron_V1_Object *object);
int theron_v1_object_remove(Theron_V1_World *world, int id);
Theron_V1_Object *theron_v1_object_at(Theron_V1_World *world, int level, int x, int y);
Theron_V1_Object *theron_v1_object_by_id(Theron_V1_World *world, int id);
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
#define THERON_HASH_FNV_OFFSET  0xCBF29CE484222325UL
#define THERON_HASH_FNV_PRIME   0x00000100000001B3UL

uint64_t theron_v1_world_hash(const Theron_V1_World *world);
void     theron_v1_world_hash_inject(Theron_V1_World *world, uint64_t seed);

/* ── Quest item helpers ─────────────────────────────────────────────── */
int   theron_v1_check_quest_item(const Theron_V1_World *world);
uint8_t theron_v1_collect_quest_item(Theron_V1_World *world, uint8_t item_bit);

/* ── Binary serialization ─────────────────────────────────────────── */
#define THERON_WORLD_SAVE_MAGIC   0x574E5254U  /* 'TRNW' */
#define THERON_WORLD_SAVE_VERSION 1

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
