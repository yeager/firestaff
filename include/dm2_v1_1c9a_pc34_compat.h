#ifndef DM2_V1_1C9A_PC34_COMPAT_H
#define DM2_V1_1C9A_PC34_COMPAT_H

/*
 * dm2_v1_1c9a_pc34_compat.h — DM2 creature AI module.
 *
 * Source: skproject SKULLWIN/c_1c9a.cpp (49 functions).
 * Covers creature movement, pathfinding, combat targeting, CAII
 * management, minion spawning/release, and per-tick AI processing.
 *
 * All public and internal functions use callback-based architecture.
 * Direction tables: table1d27fc (dx) = {0,1,0,-1},
 *                   table1d2804 (dy) = {-1,0,1,0}.
 */

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Direction delta tables (verbatim from skproject dm2data.cpp:172-177)
 * ======================================================================== */

static const int16_t dm2_v1_1c9a_dir_dx[4] = { 0, 1, 0, -1 };
static const int16_t dm2_v1_1c9a_dir_dy[4] = { -1, 0, 1, 0 };

/* ========================================================================
 * table1d607e — creature AI spec table (47 entries, 4 bytes each)
 * Source: skproject/SKULLWIN/mdata.c:1564-1613
 * Only byte[1] bits are read in c_1c9a.cpp.
 * ======================================================================== */

/* Provided by caii_alloc module; declared here for reference. */

/* ========================================================================
 * Receipt structs
 * ======================================================================== */

typedef struct DM2_V1_1c9aTileCheckReceipt {
    bool    passable;
    int16_t tile_type;        /* tile_value >> 5 */
    int16_t tile_sub;         /* tile_value & 0x7 */
    bool    has_creatures;
    bool    has_rebirth_altar;
} DM2_V1_1c9aTileCheckReceipt;

typedef struct DM2_V1_1c9aPathfindReceipt {
    bool    path_found;
    int     path_length;      /* number of steps, or -1 */
} DM2_V1_1c9aPathfindReceipt;

typedef struct DM2_V1_1c9aCreatureGoReceipt {
    bool    moved;
    int16_t new_x;
    int16_t new_y;
    int16_t new_dir;
} DM2_V1_1c9aCreatureGoReceipt;

typedef struct DM2_V1_1c9aMinionReceipt {
    bool    created;
    int16_t record_index;     /* new creature record, or -1 */
} DM2_V1_1c9aMinionReceipt;

typedef struct DM2_V1_1c9aDamageReceipt {
    bool    applied;
    int16_t remaining_hp;
    bool    creature_died;
} DM2_V1_1c9aDamageReceipt;

typedef struct DM2_V1_1c9aHealReceipt {
    bool    applied;
    int16_t new_hp;
} DM2_V1_1c9aHealReceipt;

typedef struct DM2_V1_1c9aCaiiLookupReceipt {
    bool    found;
    int     slot_index;       /* CAII slot or -1 */
} DM2_V1_1c9aCaiiLookupReceipt;

typedef struct DM2_V1_1c9aFillCaiiReceipt {
    int     creatures_activated;
} DM2_V1_1c9aFillCaiiReceipt;

typedef struct DM2_V1_1c9a0891Receipt {
    bool    decided;
    int16_t action_code;      /* creature behavior selected */
} DM2_V1_1c9a0891Receipt;

typedef struct DM2_V1_1c9a0559Receipt {
    bool    turned;
    int16_t new_facing;       /* -1 if no turn */
} DM2_V1_1c9a0559Receipt;

typedef struct DM2_V1_1c9a1BA1BReceipt {
    bool    passable;
    int16_t tile_type;
} DM2_V1_1c9a1BA1BReceipt;

/* ========================================================================
 * Callback struct — external dependencies for the 1c9a module
 * ======================================================================== */

/* Function pointer type for line-of-sight tile check (DM2_1BAAD pattern) */
typedef int32_t (*DM2_V1_1c9aTileCheckFn)(int32_t x, int32_t y);

typedef struct DM2_V1_1c9aCallbacks {
    /* --- Map / tile queries --- */
    uint8_t (*get_tile_value)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_tile_record_link)(void *ctx, int16_t x, int16_t y);
    void   *(*get_address_of_tile_record)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_next_record_link)(void *ctx, uint16_t record);
    void   *(*get_address_of_record)(void *ctx, uint16_t record);
    int16_t (*get_wall_tile_anyitem_record)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_map_width)(void *ctx);
    int16_t (*get_map_height)(void *ctx);
    int16_t (*get_current_map)(void *ctx);
    void    (*change_current_map)(void *ctx, int16_t map);

    /* --- Creature queries --- */
    int16_t (*get_creature_at)(void *ctx, int16_t x, int16_t y);
    bool    (*is_rebirth_altar)(void *ctx, void *tile_record);
    int32_t (*query_creature_ai_spec_flags)(void *ctx, uint16_t creature_type);
    /* CREATURE_1c9a_0958 resolves the creature's AI animation sequence and
     * returns bit 14 from its current CreatureAnimationFrame::w0.  Retain
     * that source-owned record/AI/frame traversal behind one callback: the
     * compatibility layer must not guess DB4 or AI-info offsets. */
    int32_t (*query_creature_animation_frame_bit14)(void *ctx,
                                                    uint16_t record);
    int32_t (*creature_can_handle_it)(void *ctx, uint16_t record, int16_t action);
    void   *(*query_creature_ai_spec_from_record)(void *ctx, uint8_t type_byte);
    int32_t (*get_graphics_for_door)(void *ctx, int32_t altar_result);

    /* --- Distance / direction --- */
    int16_t (*calc_square_distance)(void *ctx, int16_t x1, int16_t y1,
                                    int16_t x2, int16_t y2);
    int16_t (*calc_vector_dir)(void *ctx, int16_t x1, int16_t y1,
                               int16_t x2, int16_t y2);

    /* --- Random --- */
    bool    (*randbit)(void *ctx);
    int16_t (*randdir)(void *ctx);
    int16_t (*rand16)(void *ctx, int16_t max);

    /* --- Party queries --- */
    int16_t (*get_player_at_position)(void *ctx, int16_t pos);
    int16_t (*get_hero_item)(void *ctx, int16_t hero_idx, int16_t slot);
    uint8_t (*get_hero_partypos)(void *ctx, int16_t hero_idx);
    int16_t (*move_2c1d_028c)(void *ctx, int16_t x, int16_t y, int16_t dir);

    /* --- CAII management --- */
    void    (*alloc_caii_to_creature)(void *ctx, int32_t record, int16_t x, int16_t y);
    void    (*creature_schedule_at)(void *ctx, int32_t record, int16_t delay);
    void    (*creature_cancel_timer)(void *ctx, int32_t record);

    /* --- Record allocation --- */
    int16_t (*alloc_new_creature)(void *ctx, int16_t type, int16_t dir,
                                  int16_t x, int16_t y, int16_t pos);
    void    (*delete_creature_record)(void *ctx, int32_t record);

    /* --- DB allocation --- */
    int32_t (*allocation11)(void *ctx, int32_t key, int32_t mode, int16_t *out);
    void    (*dballoc_free)(void *ctx, int16_t dbidx);

    /* --- Misc queries --- */
    int32_t (*query_0cee_3275)(void *ctx, int32_t door_type);
    int32_t (*compute_power_4_within)(void *ctx, int16_t flags, int16_t n);

    /* --- table1d607e access --- */
    uint8_t (*table1d607e_byte)(void *ctx, int index, int byte_offset);

    /* --- s350 / creature state --- */
    uint16_t (*get_creature_word)(void *ctx, int offset);
    void     (*set_creature_byte)(void *ctx, int offset, uint8_t val);
    void     (*set_creature_word)(void *ctx, int offset, uint16_t val);
    uint16_t (*get_creature_flags)(void *ctx);         /* s350.v1e0578 */
    void    *(*get_creature_ptr)(void *ctx);            /* s350.v1e0552 */
    void    *(*get_creature_base)(void *ctx);           /* s350.creatures */
    int16_t  (*get_v1e0584)(void *ctx);                 /* creature type index */
    uint16_t (*get_v1e057a)(void *ctx);

    /* --- Sound / effects --- */
    void    (*play_creature_sound)(void *ctx, int32_t record, int16_t sound_id);

    /* --- ddat state --- */
    int16_t (*get_ddat_current_map)(void *ctx);         /* ddat.v1d3248 */
    int16_t (*get_ddat_party_x)(void *ctx);             /* ddat.v1e08d8 */
    int16_t (*get_ddat_party_y)(void *ctx);             /* ddat.v1e08d4 */
    int16_t (*get_ddat_party_map)(void *ctx);            /* ddat.v1e08d6 */
    int16_t (*get_ddat_party_dir)(void *ctx);            /* ddat.v1e0258 */
} DM2_V1_1c9aCallbacks;

/* ========================================================================
 * Core function declarations
 * ======================================================================== */

/*
 * DM2_1BAAD — tile passability check (static in source, exposed for
 * line-of-sight callback).  Checks whether a tile at (x,y) blocks
 * creature movement.
 * skproject c_1c9a.cpp:23-150
 */
int32_t dm2_v1_1c9a_1baad(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t x, int32_t y,
    DM2_V1_1c9a1BA1BReceipt *receipt);

/*
 * DM2_1BC29 — tile passability with party-position shortcut.
 * If creature is at party position, returns 1 immediately.
 * skproject c_1c9a.cpp:152-160
 */
int32_t dm2_v1_1c9a_1bc29(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t x, int16_t y);

/*
 * DM2_19f0_0207 — Bresenham-style line walk with tile check callback.
 * Walks from (startX,startY) to (endX,endY), calling tile_check_fn at
 * each intermediate tile.  Returns distance if path is clear, 0 if blocked.
 * skproject c_1c9a.cpp:163-468
 */
int32_t dm2_v1_1c9a_19f0_0207(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t start_x, int16_t start_y,
    int16_t end_x, int16_t end_y,
    DM2_V1_1c9aTileCheckFn tile_check_fn);

/*
 * DM2_19f0_045a — tile cache refresh.  Reads tile value at (x,y) and
 * caches record links for subsequent 04bf/050f queries.
 * skproject c_1c9a.cpp:470-501
 */
int32_t dm2_v1_1c9a_19f0_045a(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t x, int16_t y);

/*
 * DM2_19f0_04bf — cached first non-basic record link finder.
 * Walks record chain, skips types <= 3, caches result.
 * skproject c_1c9a.cpp:503-540 (static)
 */
int32_t dm2_v1_1c9a_19f0_04bf(
    const DM2_V1_1c9aCallbacks *cb, void *ctx);

/*
 * DM2_19f0_050f — cached creature record finder.
 * Walks from 04bf result, finds type == 4 (creature), caches result.
 * skproject c_1c9a.cpp:542-573 (static)
 */
int32_t dm2_v1_1c9a_19f0_050f(
    const DM2_V1_1c9aCallbacks *cb, void *ctx);

/*
 * DM2_19f0_0559 — creature turn direction selection.
 * Given a target direction, decides whether creature turns left or right.
 * skproject c_1c9a.cpp:584-645
 */
int32_t dm2_v1_1c9a_19f0_0559(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int16_t target_dir,
    DM2_V1_1c9a0559Receipt *receipt);

/*
 * DM2_19f0_05e8 — creature combat/movement detailed evaluation.
 * Complex function evaluating creature behavior with multiple sub-checks.
 * skproject c_1c9a.cpp:648-931 (static)
 */
int32_t dm2_v1_1c9a_19f0_05e8(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, void *edx_ptr, void *ebx_ptr,
    int32_t dir, int16_t param0, int32_t param1, int32_t param2);

/*
 * DM2_1c9a_0598 — popcount (bit count) of lower 32 bits.
 * Returns number of set bits.
 * skproject c_1c9a.cpp:933-957 (static)
 */
int32_t dm2_v1_1c9a_popcount(int32_t value);

/*
 * DM2_19f0_0891 — creature attack/movement decision engine.
 * Major AI decision function: evaluates targets, selects attack type
 * or movement direction based on creature capabilities and distance.
 * skproject c_1c9a.cpp:960-1660
 */
int32_t dm2_v1_1c9a_19f0_0891(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int16_t target_x, int16_t target_dir,
    DM2_V1_1c9a0891Receipt *receipt);

/*
 * DM2_19f0_0d10 — creature movement decision with flee/approach logic.
 * Evaluates adjacent tiles and selects best movement direction toward
 * or away from target based on AI state.
 * skproject c_1c9a.cpp:1663-2257
 */
int32_t dm2_v1_1c9a_19f0_0d10(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int16_t target_x, int16_t target_y);

/*
 * DM2_19f0_13aa — creature combat target selection.
 * Scans for party members or hostile creatures and selects attack target.
 * skproject c_1c9a.cpp:2259-2427
 */
int32_t dm2_v1_1c9a_19f0_13aa(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t unused);

/*
 * DM2_19f0_1511 — direction normalization helper.
 * skproject c_1c9a.cpp:2430-2435 (static)
 */
int32_t dm2_v1_1c9a_19f0_1511(int32_t value);

/*
 * DM2_D283 — creature record lookup by index and map.
 * Walks thing list to find matching creature entry.
 * skproject c_1c9a.cpp:2438-2510 (static)
 */
void *dm2_v1_1c9a_d283(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t map_index);

/*
 * DM2_CREATURE_GO_THERE — major creature movement orchestrator (~1460 lines).
 * Handles door opening, pit falling, teleporter traversal, tile occupancy,
 * creature blocking, multi-cell movement, and timer scheduling.
 * skproject c_1c9a.cpp:2514-3972
 */
int32_t dm2_v1_1c9a_creature_go_there(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int16_t dest_x, int16_t dest_y,
    DM2_V1_1c9aCreatureGoReceipt *receipt);

/*
 * DM2_19f0_2024 — pre-movement tile validation.
 * Checks whether a creature can enter a specific tile.
 * skproject c_1c9a.cpp:3987-4121 (static)
 */
int32_t dm2_v1_1c9a_19f0_2024(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y);

/*
 * DM2_19f0_2165 — high-level creature AI tick handler.
 * Dispatches to combat, movement, or idle based on creature state.
 * skproject c_1c9a.cpp:4124-4638
 */
int32_t dm2_v1_1c9a_19f0_2165(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t dir, int32_t flags, int16_t param1, int16_t param2);

/*
 * DM2_19f0_266c — ranged attack feasibility (line-of-sight check).
 * skproject c_1c9a.cpp:4641-4718 (static)
 */
int32_t dm2_v1_1c9a_19f0_266c(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y, int16_t dir);

/*
 * DM2_19f0_2723 — adjacent tile threat assessment.
 * Returns bitmask of directions with threats.
 * skproject c_1c9a.cpp:4721-4838 (static)
 */
int32_t dm2_v1_1c9a_19f0_2723(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y, int16_t dir);

/*
 * DM2_19f0_2813 — path obstruction check between two map coordinates.
 * Walks a line checking for impassable terrain.
 * skproject c_1c9a.cpp:4841-5081
 */
bool dm2_v1_1c9a_19f0_2813(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t map_x, int16_t map_y,
    int16_t x, int16_t y, int16_t target_x, int16_t target_y);

/*
 * DM2_1BA1B — tile passability dispatcher (used as indirect callback).
 * Maps tile types to passability: wall=1, pit-open=0, door checks altar,
 * teleporter checks open bit.
 * skproject c_1c9a.cpp:5090-5133
 */
int32_t dm2_v1_1c9a_1ba1b(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t x, int32_t y,
    DM2_V1_1c9a1BA1BReceipt *receipt);

/*
 * DM2_1c9a_0247 — creature CAII cleanup / deallocation.
 * skproject c_1c9a.cpp:5135-5160
 */
void dm2_v1_1c9a_0247(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index);

/*
 * DM2_1c9a_0648 — CAII lookup / map change for creature.
 * skproject c_1c9a.cpp:5162-5196
 */
int32_t dm2_v1_1c9a_0648(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index);

/*
 * DM2_1c9a_06bd — creature record lookup by position.
 * Returns pointer to creature record at (x,y) matching index.
 * skproject c_1c9a.cpp:5218-5245
 */
int16_t *dm2_v1_1c9a_06bd(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t x, int32_t y);

/*
 * DM2_1c9a_078b — recursive creature group movement.
 * Propagates movement to all group members.
 * skproject c_1c9a.cpp:5249-5375
 */
int32_t dm2_v1_1c9a_078b(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    void *creature_ptr, int32_t x, int32_t y);

/*
 * DM2_1c9a_0958 — creature animation-frame flag.
 * Returns `(CreatureAnimationFrame::w0 & 0x4000) >> 14` for the source
 * creature record. The callback owns the record → AI spec → frame traversal.
 * Source: SKProject SKWINSPX/src/v4/skcore.cpp:15447-15455;
 *         SKWINSPX/src/v5/SK1C9A.cpp:5377-5399.
 */
int32_t dm2_v1_1c9a_0958(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index);

/*
 * DM2_1c9a_09b9 — creature property getter.
 * skproject c_1c9a.cpp:5404-5413
 */
int32_t dm2_v1_1c9a_09b9(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t property);

/*
 * DM2_1c9a_09db — creature facing update.
 * skproject c_1c9a.cpp:5416-5431
 */
void dm2_v1_1c9a_09db(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    void *creature_ptr);

/*
 * DM2_CREATURE_SOMETHING_1c9a_0a48 — global creature AI processing loop.
 * Iterates all active CAII entries, performs per-tick updates.
 * skproject c_1c9a.cpp:5434-5693
 */
int32_t dm2_v1_1c9a_creature_something_0a48(
    const DM2_V1_1c9aCallbacks *cb, void *ctx);

/*
 * DM2_1c9a_0cf7 — creature timer scheduling.
 * Creates/updates timer for creature's next AI action.
 * skproject c_1c9a.cpp:5695-5732
 */
void dm2_v1_1c9a_0cf7(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t delay);

/*
 * DM2_1c9a_0db0 — creature timer cancellation.
 * skproject c_1c9a.cpp:5734-5763
 */
void dm2_v1_1c9a_0db0(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index);

/*
 * DM2_ALLOC_CAII_TO_CREATURE — allocate CAII slot for creature.
 * skproject c_1c9a.cpp:5772-5894
 */
void dm2_v1_1c9a_alloc_caii_to_creature(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y);

/*
 * DM2_1c9a_0fcb — creature activation/wakeup.
 * Transitions dormant creature to active state, allocates CAII, schedules timer.
 * skproject c_1c9a.cpp:5896-5958
 */
void dm2_v1_1c9a_0fcb(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index);

/*
 * DM2_CREATE_MINION — spawn summoned creature.
 * skproject c_1c9a.cpp:5961-6146
 */
int16_t dm2_v1_1c9a_create_minion(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_type, int16_t map_x, int16_t map_y,
    int16_t dir, int32_t owner_index, int16_t hit_points,
    int32_t flags, int8_t spell_power,
    DM2_V1_1c9aMinionReceipt *receipt);

/*
 * DM2_RELEASE_MINION — destroy summoned creature.
 * skproject c_1c9a.cpp:6149-6179
 */
void dm2_v1_1c9a_release_minion(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index);

/*
 * DM2_1c9a_17c7 — creature-to-party distance calculation.
 * skproject c_1c9a.cpp:6182-6239
 */
int32_t dm2_v1_1c9a_17c7(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y);

/*
 * DM2_1c9a_19d4 — creature position update (raw record relocation).
 * skproject c_1c9a.cpp:6241-6272
 */
void dm2_v1_1c9a_19d4(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t x, int16_t y, int16_t dir);

/*
 * DM2_1c9a_1a48 — creature damage application.
 * skproject c_1c9a.cpp:6274-6353
 */
int32_t dm2_v1_1c9a_1a48(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t damage,
    DM2_V1_1c9aDamageReceipt *receipt);

/*
 * DM2_1c9a_1b16 — creature healing.
 * skproject c_1c9a.cpp:6355-6417
 */
int32_t dm2_v1_1c9a_1b16(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int32_t heal_amount,
    DM2_V1_1c9aHealReceipt *receipt);

/*
 * DM2_FIND_WALK_PATH — A*-style pathfinding (~3230 lines).
 * Supports multi-level dungeons, door states, creature sizes.
 * skproject c_1c9a.cpp:6439-9668
 */
int32_t dm2_v1_1c9a_find_walk_path(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    int32_t creature_index, int16_t start_x, int16_t start_y,
    int16_t flags, uint8_t *path_buffer, void *button,
    DM2_V1_1c9aPathfindReceipt *receipt);

/*
 * DM2_1c9a_381c — walk path consumer (pop next direction).
 * skproject c_1c9a.cpp:9697-9746
 */
int32_t dm2_v1_1c9a_381c(
    const DM2_V1_1c9aCallbacks *cb, void *ctx);

/*
 * DM2_1c9a_38a8 — CAII table compaction/maintenance.
 * Removes stale entries and compacts table.
 * skproject c_1c9a.cpp:9749-9894
 */
int32_t dm2_v1_1c9a_38a8(
    const DM2_V1_1c9aCallbacks *cb, void *ctx);

/*
 * DM2_FILL_CAII_CUR_MAP — map creature initialization.
 * Walks all tiles, allocates CAII for each creature.
 * skproject c_1c9a.cpp:9896-9993
 */
int32_t dm2_v1_1c9a_fill_caii_cur_map(
    const DM2_V1_1c9aCallbacks *cb, void *ctx,
    DM2_V1_1c9aFillCaiiReceipt *receipt);

/*
 * DM2_FILL_ORPHAN_CAII — orphan CAII recovery.
 * Scans for creatures without CAII entries and allocates slots.
 * skproject c_1c9a.cpp:9996-10022
 */
void dm2_v1_1c9a_fill_orphan_caii(
    const DM2_V1_1c9aCallbacks *cb, void *ctx);

/* ========================================================================
 * Module-internal cached state (tile cache for 045a/04bf/050f chain)
 * ======================================================================== */

typedef struct DM2_V1_1c9aTileCache {
    int16_t cached_x;           /* v1e08a8 */
    int16_t cached_y;           /* v1e08aa */
    int16_t cached_map;         /* v1e08ac */
    int16_t cached_tile_value;  /* v1e08ae */
    int16_t cached_b0;          /* v1e08b0 — tile record link */
    int16_t cached_b2;          /* v1e08b2 — first non-basic record */
    int16_t cached_b4;          /* v1e08b4 — creature record */
    uint8_t cached_b6;          /* v1e08b6 */
    uint8_t cached_b7;          /* v1e08b7 */
    int32_t cached_be;          /* v1e08be */
    int32_t cached_c4;          /* v1e08c4 */
} DM2_V1_1c9aTileCache;

/* Initialize tile cache to invalidated state. */
void dm2_v1_1c9a_tile_cache_init(DM2_V1_1c9aTileCache *cache);

/* ========================================================================
 * Utility: popcount
 * ======================================================================== */

/* Already declared above as dm2_v1_1c9a_popcount. */

#ifdef __cplusplus
}
#endif

#endif /* DM2_V1_1C9A_PC34_COMPAT_H */
