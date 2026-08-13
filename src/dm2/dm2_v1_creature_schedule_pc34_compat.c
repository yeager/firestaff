#include "dm2_v1_creature_schedule_pc34_compat.h"

#include <stdio.h>
#include <string.h>

#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"

static uint16_t dm2_v1_read_u16le(const uint8_t *p) {
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

int dm2_v1_creature_schedule_at(
    const DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int x, int y,
    DM2_V1_CreatureScheduleReceipt *receipt) {
  DM2_V1_CreatureScheduleReceipt local;
  DM2_V1_SourceTimer timer;
  int16_t handle;
  const uint8_t *record;
  uint16_t group_link;
  DM2_V1_SourceTimerResult enq;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_1c9a.cpp:5695-5728 DM2_1c9a_0cf7 bounded slice "
           "(timer derive + DM2_QUEUE_TIMER c_1c9a.cpp:5723; CAII slot word "
           "and DM2_1c9a_0db0 delete stay host-owned)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || dungeon == NULL || queue == NULL ||
      map_id < 0 || map_id > 0xff || x < 0 || y < 0 || x > 0xff || y > 0xff) {
    return 0;
  }

  /* DM2_GET_CREATURE_AT(x, y), c_1c9a.cpp:5698, runs after the source has
   * switched to ddat.v1d3248. The public runtime boundary carries that map
   * explicitly; resolving against map 0 would miss creatures on other
   * authentic dungeon levels. */
  handle = dm2_v1_get_creature_at(pool_set, dungeon, map_id, x, y);
  if (handle == DM2_V1_RECORD_HANDLE_NULL) {
    return 0;
  }
  receipt->resolved = 1;

  record = dm2_v1_record_pool_address(pool_set, handle);
  if (record == NULL) {
    return 0;
  }

  receipt->creature_type = record[4];                 /* byte@4, c_1c9a.cpp:5713 */
  group_link = dm2_v1_read_u16le(record + 8);         /* word@8, c_1c9a.cpp:5708 */
  receipt->has_group_link = (group_link != 0xffffu) ? 1 : 0;
  receipt->timer_type = receipt->has_group_link
                            ? DM2_V1_TIMER_THINK_CREATURE_B   /* 0x22 */
                            : DM2_V1_TIMER_THINK_CREATURE_A;  /* 0x21 */
  receipt->map_id = map_id;
  receipt->due_tick = game_tick + 1u;                 /* setmticks, c_1c9a.cpp:5707 */

  memset(&timer, 0, sizeof(timer));
  timer.ticks_and_map =
      ((uint32_t)receipt->due_tick & DM2_V1_SOURCE_TIMER_TICK_MASK) |
      ((uint32_t)map_id << 24);
  timer.type = (uint8_t)receipt->timer_type;
  timer.actor = (uint8_t)receipt->creature_type;
  timer.value_a = (int16_t)((x & 0xff) | ((y & 0xff) << 8));  /* setxyA */

  enq = DM2_V1_SOURCE_TIMER_OK;
  receipt->timer_ticket =
      dm2_v1_source_timer_enqueue_ticketed(queue, &timer, 0u, &enq);
  if (enq != DM2_V1_SOURCE_TIMER_OK || receipt->timer_ticket == 0u) {
    return 0;
  }

  receipt->enqueued = 1;
  receipt->valid = 1;
  return 1;
}
