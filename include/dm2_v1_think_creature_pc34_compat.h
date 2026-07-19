#ifndef FIRESTAFF_DM2_V1_THINK_CREATURE_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_THINK_CREATURE_PC34_COMPAT_H

/*
 * dm2_v1_think_creature_pc34_compat.h — per-cell DM2_THINK_CREATURE binding
 * over the DM2-002 record pool.
 *
 * Mirrors the skproject boundary exactly:
 *
 *   c_tim_proc.cpp:4079-4088  the 0x21/0x22 timer dispatch unpacks the
 *                             c_tim payload: x = getxA() (value_a low byte),
 *                             y = getyA() (value_a high byte), think type =
 *                             the timer type word, then calls
 *                             DM2_THINK_CREATURE(x, y, type) on the timer's
 *                             map (DM2_CHANGE_CURRENT_MAP_TO already ran).
 *   c_ai.cpp:5649-5677        DM2_THINK_CREATURE prologue: resolve the
 *                             creature record AT THE TIMER CELL via
 *                             DM2_GET_CREATURE_AT(x, y); when no DB4 record
 *                             is chained on that cell the source returns
 *                             immediately (the timer is consumed, nothing
 *                             thinks).
 *   c_querydb.cpp:1486-1507   DM2_GET_CREATURE_AT: walk the cell's tile
 *                             record link chain and return the first record
 *                             whose DB index (handle bits 10-13, direction
 *                             bits 14-15 ignored) is dbCreature (4);
 *                             OBJECT_END_MARKER terminates the walk with
 *                             "no creature" (0xffff).
 *   c_map.cpp:44-69           DM2_GET_TILE_RECORD_LINK: bit 0x10 byte-square
 *                             object flag + column-index ground-stack table
 *                             (bound in Firestaff by
 *                             dm2_v1_dungeon_get_first_thing).
 *
 * Fail-closed contract: when the pool set or dungeon is absent, the cell
 * chain is corrupt, or no DM2-owned think body is bound, nothing is
 * simulated — the receipt records exactly which source boundary stopped
 * the think.  The think body itself (DM2_PREPARE_LOCAL_CREATURE_VAR +
 * the c_ai.cpp body) stays unbound until the CCM stream owner/grammar is
 * proven (DM2-005); a bound body callback receives the resolved record.
 */

#include <stdint.h>

#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"

#ifdef __cplusplus
extern "C" {
#endif

/* DM2_GET_CREATURE_AT (c_querydb.cpp:1486-1507): return the first DB4
 * creature record chained on cell (map, x, y), direction bits preserved
 * exactly as the link word carries them.  Returns
 * DM2_V1_RECORD_HANDLE_NULL when the cell has no creature, the tile link
 * is absent, or the chain cannot be walked (fail-closed). */
int16_t dm2_v1_get_creature_at(const DM2_V1_RecordPoolSet *set,
                               const DM2_V1_DungeonData *dungeon,
                               int map, int x, int y);

typedef struct {
    int valid;
    int think_timers;         /* 0x21/0x22 timers seen by the binding */
    int no_creature_at_cell;  /* source early return (c_ai.cpp:5671-5673) */
    int resolved;             /* creature record resolved at the cell */
    int body_unbound;         /* resolved but no DM2-owned think body */
    int body_consumed;        /* bound body accepted the think */
    int body_rejected;        /* bound body returned 0 */
    int16_t last_record;      /* last resolved creature record, -1 none */
    int last_map;
    int last_x;
    int last_y;
    int last_type;            /* 0x21 or 0x22 (source passes the type word) */
    int last_creature_type;   /* DB4 record byte@4, -1 unknown */
} DM2_V1_ThinkCreatureReceipt;

/* Bound think body: invoked only when a creature record resolves at the
 * timer cell.  Return 1 when the think was consumed, 0 to reject (both are
 * receipted; the timer itself is always consumed exactly like the source's
 * void DM2_THINK_CREATURE). */
typedef int (*DM2_V1_ThinkCreatureBody)(
    void *context,
    int16_t creature_record,
    const DM2_V1_SourceTimer *timer,
    int map, int x, int y, int think_type);

typedef struct {
    const DM2_V1_RecordPoolSet *pool_set;  /* DM2-002 record ownership */
    const DM2_V1_DungeonData *dungeon;     /* tile record links */
    DM2_V1_ThinkCreatureBody think_body;   /* NULL = body unbound */
    void *think_body_context;
    DM2_V1_ThinkCreatureReceipt receipt;
} DM2_V1_ThinkCreatureBinding;

void dm2_v1_think_creature_binding_init(
    DM2_V1_ThinkCreatureBinding *binding,
    const DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon);

/* DM2_V1_TimerTypeHandler for DM2_V1_TIMER_THINK_CREATURE_A/B (0x21/0x22);
 * `context` is a DM2_V1_ThinkCreatureBinding.  Mirrors the source dispatch:
 * the timer is always consumed (return 1) once the binding is usable — a
 * cell without a creature is the source's early return, not an error.
 * Returns 0 only when the binding itself is incomplete (fail-closed). */
int dm2_v1_think_creature_timer_handler(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt);

const char *dm2_v1_think_creature_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_THINK_CREATURE_PC34_COMPAT_H */
