#ifndef FIRESTAFF_DM2_V1_TILE_RECORD_WALK_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_TILE_RECORD_WALK_PC34_COMPAT_H

/*
 * dm2_v1_tile_record_walk_pc34_compat.h — the DM2-002 tile record-link
 * walk as a first-class bounded primitive, plus the two AI-stop callers
 * that were blocked on it.
 *
 * Mirrors the skproject boundary exactly:
 *
 *   c_map.cpp:61-69         DM2_GET_TILE_RECORD_LINK: the cell's ground
 *                           stack head (byte-square bit-0x10 object flag
 *                           + column-index ground-stack table), bound in
 *                           Firestaff by dm2_v1_dungeon_get_first_thing;
 *                           a cell without objects yields
 *                           OBJECT_END_MARKER.
 *   c_record.cpp:54-57      DM2_GET_NEXT_RECORD_LINK: the record's first
 *                           word is the next link; the walk terminates
 *                           at OBJECT_END_MARKER.
 *   c_ai.cpp:2078-2117      DM2_PROCEED_XACT_85: walk the creature's own
 *                           cell chain; DB index (handle bits 10-13)
 *                           above 3 breaks the walk; a DB2 record whose
 *                           word@2 carries (w & 0x6) == 0x2 and
 *                           ((w >> 11) & 0x1f) == 1 ends the walk with
 *                           slot byte@0x1e = 1, byte@0x1a = 59 and the
 *                           source return -2; otherwise the walk end
 *                           runs the bound DM2_ai_13e4_0360 AI-stop
 *                           (dir 0x13, argl0 1), sets slot byte@0x1a =
 *                           51 and returns -3.
 *   c_tim_proc.cpp:2907-2988 DM2_ACTIVATE_CREATURE_KILLER: rectangular
 *                           sweep around (argl0, argl1) with radii
 *                           |ebx - argl0| / |ecx - argl1|; per in-bounds
 *                           cell DM2_GET_CREATURE_AT resolves the first
 *                           DB4 record, DM2_1c9a_09b9 filters on record
 *                           word@8 == vw_04; action 0xb with mode word
 *                           RG6: 0/1 skip, 2 runs the bound
 *                           DM2_ai_13e4_0360 AI-stop, anything above 2
 *                           aborts the whole sweep (source return);
 *                           action 0x28 runs the bound
 *                           DM2_ATTACK_CREATURE with attack word
 *                           (RG6 low 16 bits) | (argw3 ? 0x8000 : 0),
 *                           strength 0x64, hp delta 0.
 *   c_1c9a.cpp:5404-5414    DM2_1c9a_09b9: record word@8 == filter word.
 *
 * Fail-closed contract: corrupt chains are bounded (no walk may visit
 * more records than the pools declare), unresolvable links end the
 * walk, and every source early return is receipted.  s350/ddat stay
 * host-owned; their fields enter as explicit parameters.  The CAII
 * slot byte writes land in the session CAII array exactly like the
 * source's s350.creatures pointer.
 */

#include <stdint.h>

#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_drops.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DM2_GET_TILE_RECORD_LINK (c_map.cpp:61-69): the cell's ground-stack
 * head record handle.  Returns DM2_V1_RECORD_HANDLE_END when the cell
 * carries no objects (source OBJECT_END_MARKER) and
 * DM2_V1_RECORD_HANDLE_NULL when the lookup itself cannot resolve
 * (absent dungeon, out-of-bounds cell — fail-closed). */
int16_t dm2_v1_tile_record_link(const DM2_V1_DungeonData *dungeon,
                                int map, int x, int y);

typedef struct {
    int valid;
    int records_visited;   /* records handed to the visitor */
    int stopped_by_visitor;/* visitor requested the source break */
    int ended_at_marker;   /* OBJECT_END_MARKER terminated the walk */
    int fail_closed;       /* corrupt/bounded chain or unresolvable link */
} DM2_V1_TileRecordWalkReceipt;

/* Visitor for the bounded walk.  Return nonzero to stop the walk (the
 * source's loop break); zero continues to the next link. */
typedef int (*DM2_V1_TileRecordVisitor)(void *context,
                                        int16_t handle,
                                        const uint8_t *record);

/* Bounded tile record-link walk: visit every record chained on cell
 * (map, x, y) in link order.  The walk never visits more records than
 * the pools declare (corrupt self-loops fail closed).  Returns 1 when
 * the walk ran to a source termination (visitor stop or end marker),
 * 0 fail-closed. */
int dm2_v1_tile_record_walk(const DM2_V1_RecordPoolSet *set,
                            const DM2_V1_DungeonData *dungeon,
                            int map, int x, int y,
                            DM2_V1_TileRecordVisitor visitor,
                            void *context,
                            DM2_V1_TileRecordWalkReceipt *receipt);

typedef struct {
    int valid;
    int completed;          /* slice reached a source return */
    int walk_records;       /* records visited on the cell chain */
    int walk_db_break;      /* DB index > 3 broke the walk */
    int walk_fail_closed;   /* corrupt chain bounded */
    int match_found;        /* DB2 word@2 pattern matched */
    int return_code;        /* source return: -2 match, -3 walk end */
    int slot_b1e_written;   /* slot byte@0x1e = 1 (match path) */
    int slot_b1a;           /* slot byte@0x1a written (59 or 51) */
    int ai_stop_invoked;    /* bound 0360 ran (walk-end path) */
    int ai_stop_completed;  /* 0360 receipt.completed */
    int ai_stop_guard_denied;/* 0360 receipt.guard_denied */
    char source_evidence[512];
} DM2_V1_Xact85Receipt;

/* DM2_PROCEED_XACT_85 (c_ai.cpp:2078-2117) — bounded slice.
 * `creature_record` is s350.v1e054c (the acting creature, DB4),
 * `x`/`y` its cell (s350.v1e0562).  The slot byte@0x1a/0x1e writes
 * land in the session CAII array through record byte@5 exactly like
 * the source's s350.creatures.  Returns 1 when the slice reached a
 * source return (receipt.return_code -2 or -3); 0 fail-closed. */
int dm2_v1_proceed_xact_85(DM2_V1_RecordPoolSet *pool_set,
                           const DM2_V1_DungeonData *dungeon,
                           DM2_V1_CaiiArray *caii,
                           DM2_V1_SourceTimerQueue *queue,
                           int map_id,
                           unsigned long game_tick,
                           int16_t creature_record,
                           int x, int y,
                           DM2_V1_Xact85Receipt *receipt);

typedef struct {
    int valid;
    int completed;          /* sweep ran to the source return */
    int cells_visited;      /* in-bounds cells examined */
    int cells_out_of_bounds;/* cells skipped by the map bounds check */
    int creatures_found;    /* cells with a resolved DB4 record */
    int type_filter_miss;   /* 1c9a_09b9 rejected the record */
    int ai_stops;           /* bound 0360 invocations (action 0xb) */
    int ai_stop_completed;  /* bound 0360 completions */
    int attacks;            /* bound ATTACK_CREATURE invocations */
    int attacks_completed;  /* bound ATTACK_CREATURE completions */
    int aborted;            /* action 0xb mode > 2: source return */
    int unknown_action;     /* action neither 0xb nor 0x28 (receipted) */
    char source_evidence[512];
} DM2_V1_CreatureKillerReceipt;

/* DM2_ACTIVATE_CREATURE_KILLER (c_tim_proc.cpp:2907-2988) — bounded
 * slice.  `mode` is the source eaxl (RG6 word: AI-stop selector /
 * attack word base), `type_filter` the edxl low word (vw_04, record
 * word@8 filter), `ebx`/`ecx` the radius-end coordinates, `center_x`/
 * `center_y` argl0/argl1, `action` argw2 (0xb AI-stop, 0x28 attack),
 * `attack_flag` argw3 (sets attack word bit 0x8000).  `target_x`/
 * `target_y` stand in for the ddat.v1e0270/v1e0272 attack-target
 * globals the bound ATTACK_CREATURE slice needs; `map_width`/
 * `map_height` stand in for mapdat.map_width/map_height.  Returns 1
 * when the sweep ran to a source return; 0 fail-closed. */
int dm2_v1_activate_creature_killer(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_id,
    unsigned long game_tick,
    int map_width, int map_height,
    int mode, int type_filter,
    int ebx, int ecx,
    int center_x, int center_y,
    int action, int attack_flag,
    int target_x, int target_y,
    DM2_V1_CreatureKillerReceipt *receipt);

const char *dm2_v1_tile_record_walk_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_TILE_RECORD_WALK_PC34_COMPAT_H */
