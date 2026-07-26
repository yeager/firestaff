#ifndef DM2_V1_CREATURE_SCHEDULE_PC34_COMPAT_H
#define DM2_V1_CREATURE_SCHEDULE_PC34_COMPAT_H

#include "dm2_v1_think_creature_pc34_compat.h"

/*
 * DM2 v1 PC 3.4 creature scheduling producer (bounded compatibility slice).
 *
 * Binds the observable slice of the creature-scheduling producer
 * DM2_1c9a_0cf7 (c_1c9a.cpp:5695-5728): the function invoked at creature
 * spawn/activation sites (c_1c9a.cpp:5860 via DM2_ALLOC_CAII_TO_CREATURE,
 * c_creature.cpp:648, c_move.cpp:700, c_ai.cpp:5958). For the creature record
 * at cell (x, y) it derives the timer tuple and enqueues one source-ordered
 * timer through the local source timer queue (DM2_QUEUE_TIMER call at
 * c_1c9a.cpp:5723):
 *
 *   - map        := ddat.v1d3248 (c_1c9a.cpp:5707); the current-map owner is
 *                   not yet bound (DM2_CHANGE_CURRENT_MAP_TO), so the bounded
 *                   slice takes the map id as a caller-owned parameter.
 *   - due        := gametick + 1 (c_1c9a.cpp:5707 setmticks).
 *   - timer type := 0x22 when the record group/leader link (word@8) != 0xffff,
 *                   else 0x21 (c_1c9a.cpp:5708-5712).
 *   - owner      := record byte@4, the creature type (c_1c9a.cpp:5713-5714).
 *   - payload    := CUTX16(x) | (CUTX16(y) << 8) via setxyA
 *                   (c_1c9a.cpp:5715-5716); matches the think-consumer
 *                   (c_creature.cpp:777-778 and c_1c9a.cpp:5778-5779).
 *
 * The same producer first deletes a previously queued timer through
 * DM2_1c9a_0db0 when the creature-array slot timer word is not -1
 * (c_1c9a.cpp:5699-5706), and stores the DM2_QUEUE_TIMER return index
 * into the slot timer word (c_1c9a.cpp:5724-5728).  This base module
 * stays CAII-agnostic: it issues the stable session ticket
 * (dm2_v1_source_timer_enqueue_ticketed) and reports it in
 * receipt.timer_ticket; the CAII-aware composition in
 * dm2_v1_caii_alloc_pc34_compat owns the slot word@2 write and the
 * replacement delete exactly like the source.  Enqueue failure is
 * fail-closed (receipt.enqueued == 0).
 */

typedef struct {
  int valid;
  int resolved;
  int enqueued;
  int replaced_existing;
  int no_caii_slot;
  int creature_type;
  int has_group_link;
  int timer_type;
  int map_id;
  unsigned long due_tick;
  uint32_t timer_ticket;
  char source_evidence[512];
} DM2_V1_CreatureScheduleReceipt;

/*
 * Schedule one creature timer for the creature record at cell (x, y),
 * mirroring DM2_1c9a_0cf7 (c_1c9a.cpp:5695-5728). `map_id` stands in for
 * ddat.v1d3248 (caller-owned until the current-map owner is proven) and
 * `game_tick` is the gametick the source reads at scheduling time.
 * Returns 1 when a timer was enqueued; 0 (fail-closed, receipt.valid == 0)
 * when no creature record resolves at the cell or the queue rejects the
 * timer. When `receipt` is non-NULL it always receives the audit record.
 */
int dm2_v1_creature_schedule_at(
    const DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int x, int y,
    DM2_V1_CreatureScheduleReceipt *receipt);

#endif
