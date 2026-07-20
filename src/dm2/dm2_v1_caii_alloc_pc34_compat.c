#include "dm2_v1_caii_alloc_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"

static uint16_t dm2_v1_read_u16le(const uint8_t *p) {
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void dm2_v1_write_u16le(uint8_t *p, uint16_t v) {
  p[0] = (uint8_t)(v & 0xffu);
  p[1] = (uint8_t)((v >> 8) & 0xffu);
}

void dm2_v1_caii_array_init(DM2_V1_CaiiArray *caii, int capacity) {
  int i;

  if (caii == NULL) {
    return;
  }
  memset(caii, 0, sizeof(*caii));
  if (capacity <= 0) {
    return;
  }
  caii->slots = calloc((size_t)capacity, DM2_V1_CAII_SLOT_SIZE);
  if (caii->slots == NULL) {
    return;
  }
  /* Free slot == signed word@0 < 0 (c_1c9a.cpp:5802). */
  for (i = 0; i < capacity; i++) {
    dm2_v1_write_u16le(caii->slots + (size_t)i * DM2_V1_CAII_SLOT_SIZE,
                       0xffffu);
  }
  caii->capacity = capacity;
  caii->valid = 1;
}

void dm2_v1_caii_array_free(DM2_V1_CaiiArray *caii) {
  if (caii == NULL) {
    return;
  }
  free(caii->slots);
  memset(caii, 0, sizeof(*caii));
}

const uint8_t *dm2_v1_caii_slot(const DM2_V1_CaiiArray *caii, int index) {
  if (caii == NULL || !caii->valid || index < 0 || index >= caii->capacity) {
    return NULL;
  }
  return caii->slots + (size_t)index * DM2_V1_CAII_SLOT_SIZE;
}

int dm2_v1_caii_alloc_to_creature(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int16_t record_handle,
    int x, int y,
    DM2_V1_CaiiAllocReceipt *receipt) {
  DM2_V1_CaiiAllocReceipt local;
  DM2_V1_CreatureScheduleReceipt sched;
  uint8_t *record;
  uint8_t *slot;
  int slot_index;
  int i;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_1c9a.cpp:5772-5894 DM2_ALLOC_CAII_TO_CREATURE "
           "bounded slice (slot init c_1c9a.cpp:5809-5866 + producer "
           "DM2_1c9a_0cf7 c_1c9a.cpp:5860; PREPARE/UNPREPARE local var, "
           "14cd_0802 and the s350 group scan stay host-owned)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || dungeon == NULL || caii == NULL || !caii->valid ||
      queue == NULL || map_id < 0 || map_id > 0x3f ||
      x < 0 || y < 0 || x > 0x1f || y > 0x1f) {
    return 0;
  }

  record = dm2_v1_record_pool_address_mut(pool_set, record_handle);
  if (record == NULL) {
    return 0;
  }

  /* Source early return: the record already owns a CAII slot
   * (c_1c9a.cpp:5783-5784). */
  if (record[5] != 0xffu) {
    receipt->already_allocated = 1;
    return 0;
  }

  /* The word@0xe bit-10 read-modify-write (c_1c9a.cpp:5785-5807)
   * restores the bit to its original value — receipted no-op. */
  receipt->record_flag_rewrite_noop = 1;

  /* Free-slot scan (c_1c9a.cpp:5798-5805): free == signed word@0 < 0. */
  slot_index = -1;
  for (i = 0; i < caii->capacity; i++) {
    const uint8_t *candidate = caii->slots + (size_t)i * DM2_V1_CAII_SLOT_SIZE;
    if ((int16_t)dm2_v1_read_u16le(candidate) < 0) {
      slot_index = i;
      break;
    }
  }
  if (slot_index < 0) {
    /* The source recycles records from the world until a slot frees
     * (DM2_RECYCLE_A_RECORD_FROM_THE_WORLD, c_1c9a.cpp:5880-5891);
     * that path is unproven — fail closed without mutation. */
    receipt->no_free_slot = 1;
    return 0;
  }

  slot = caii->slots + (size_t)slot_index * DM2_V1_CAII_SLOT_SIZE;

  /* Slot init, c_1c9a.cpp:5809-5866.  The CAII slot stores the bare
   * record index (handle & 0x3ff): DM2_1c9a_0fcb rebuilds the DB4
   * handle by OR-ing 0x1000 (c_1c9a.cpp:5915). */
  memset(slot, 0, DM2_V1_CAII_SLOT_SIZE);
  receipt->record_index = record_handle & 0x3ff;
  dm2_v1_write_u16le(slot + 0, (uint16_t)receipt->record_index);
  dm2_v1_write_u16le(slot + 2, 0xffffu);              /* timer index */
  slot[0x1a] = 0xffu;
  slot[6] = (uint8_t)((game_tick >> 2) - 1u);         /* byte@6 */
  slot[4] = (uint8_t)(game_tick - 0x7fu);             /* byte@4 */
  dm2_v1_write_u16le(slot + 0xc,
                     (uint16_t)((x & 0x1f) | ((y & 0x1f) << 5) |
                                ((map_id & 0x3f) << 10)));
  slot[0x16] = 0xffu;
  slot[0x17] = 0xffu;
  slot[7] = 0;

  record[5] = (uint8_t)slot_index;                    /* record byte@5 */
  caii->alloc_count++;                                /* ddat.v1d4020 */
  receipt->slot_index = slot_index;

  /* DM2_PREPARE/UNPREPARE_LOCAL_CREATURE_VAR, DM2_14cd_0802 and the
   * s350.v1e0552/v1e054e group scan with DM2_CREATURE_SOMETHING_1c9a_0a48
   * stay host-owned until the CCM body is proven. */
  receipt->local_var_unbound = 1;
  receipt->group_scan_unbound = 1;

  /* The bound scheduling producer, c_1c9a.cpp:5860. */
  memset(&sched, 0, sizeof(sched));
  if (dm2_v1_creature_schedule_at(pool_set, dungeon, queue, map_id,
                                  game_tick, x, y, &sched) == 1) {
    receipt->timer_scheduled = 1;
    receipt->timer_type = sched.timer_type;
    receipt->due_tick = sched.due_tick;
    /* The producer's post-queue store: the issued ticket lands in the
     * slot timer word (c_1c9a.cpp:5724-5728). */
    dm2_v1_write_u16le(slot + 2, (uint16_t)sched.timer_ticket);
  }

  /* c_1c9a.cpp:5861-5866: grouped creatures start in mode 0x00,
   * ungrouped in 0x11. */
  slot[0x1a] = (dm2_v1_read_u16le(record + 8) != 0xffffu) ? 0x00u : 0x11u;

  receipt->allocated = 1;
  receipt->valid = 1;
  return 1;
}

int dm2_v1_caii_delete_timer(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int16_t record_handle,
    DM2_V1_CaiiDeleteTimerReceipt *receipt) {
  DM2_V1_CaiiDeleteTimerReceipt local;
  const uint8_t *record;
  uint8_t *slot;
  uint16_t word2;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_1c9a.cpp:5734-5763 DM2_1c9a_0db0 bounded slice "
           "(DM2_DELETE_TIMER c_timer.cpp:215-232 via the session ticket)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || caii == NULL || !caii->valid || queue == NULL) {
    return 0;
  }

  /* The source verifies the record DB (c_1c9a.cpp:5741-5744):
   * ((handle >> 10) & 0xf) == dbCreature (4), direction bits ignored. */
  if (dm2_v1_record_handle_pool(record_handle) != 4) {
    receipt->not_creature_db = 1;
    return 0;
  }

  record = dm2_v1_record_pool_address(pool_set, record_handle);
  if (record == NULL) {
    return 0;
  }
  if (record[5] == 0xffu) {
    receipt->no_caii_slot = 1;
    return 0;
  }
  receipt->slot_index = record[5];

  slot = caii->slots + (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;
  word2 = dm2_v1_read_u16le(slot + 2);
  if (word2 == 0xffffu || word2 == 0u) {
    receipt->no_pending_timer = 1;
    return 0;
  }

  /* DM2_DELETE_TIMER(word@2) + word@2 = -1 (c_1c9a.cpp:5754-5759). */
  if (dm2_v1_source_timer_cancel(queue, (uint32_t)word2) != 1) {
    /* The owner word referenced a ticket the queue no longer holds
     * (already popped); the source cannot hit this (its indices are
     * always live), so fail closed and still clear the stale word. */
    dm2_v1_write_u16le(slot + 2, 0xffffu);
    receipt->no_pending_timer = 1;
    return 0;
  }
  receipt->cancelled_ticket = (uint32_t)word2;
  dm2_v1_write_u16le(slot + 2, 0xffffu);

  receipt->deleted = 1;
  receipt->valid = 1;
  return 1;
}

int dm2_v1_caii_schedule_creature_at(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int x, int y,
    DM2_V1_CreatureScheduleReceipt *receipt) {
  DM2_V1_CreatureScheduleReceipt local;
  DM2_V1_CaiiDeleteTimerReceipt del;
  uint8_t *record;
  uint8_t *slot;
  int16_t handle;
  int scheduled;
  int replaced = 0;

  memset(&local, 0, sizeof(local));

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }
  snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
           "skproject c_1c9a.cpp:5695-5728 DM2_1c9a_0cf7 complete slice "
           "(DM2_1c9a_0db0 replace c_1c9a.cpp:5699-5706 + slot word@2 "
           "store c_1c9a.cpp:5724-5728 over the session CAII array)");

  if (pool_set == NULL || dungeon == NULL || caii == NULL || !caii->valid ||
      queue == NULL || x < 0 || y < 0) {
    return 0;
  }

  handle = dm2_v1_get_creature_at(pool_set, dungeon, 0, x, y);
  if (handle == DM2_V1_RECORD_HANDLE_NULL) {
    return 0;
  }
  record = dm2_v1_record_pool_address_mut(pool_set, handle);
  if (record == NULL) {
    return 0;
  }
  if (record[5] == 0xffu) {
    /* The source would index the creatures array with slot 0xff — the
     * direct callers (c_creature.cpp:648, c_move.cpp:700) only reach
     * the producer for activated creatures, so fail closed. */
    receipt->no_caii_slot = 1;
    return 0;
  }

  /* c_1c9a.cpp:5699-5706: delete a previously queued timer first. */
  slot = caii->slots + (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;
  if (dm2_v1_read_u16le(slot + 2) != 0xffffu &&
      dm2_v1_read_u16le(slot + 2) != 0u) {
    memset(&del, 0, sizeof(del));
    if (dm2_v1_caii_delete_timer(pool_set, caii, queue, handle, &del) == 1) {
      replaced = 1;
    }
  }

  scheduled = dm2_v1_creature_schedule_at(pool_set, dungeon, queue,
                                          map_id, game_tick, x, y,
                                          receipt);
  if (scheduled == 1) {
    /* c_1c9a.cpp:5724-5728: the issued ticket lands in the slot timer
     * word. */
    dm2_v1_write_u16le(slot + 2, (uint16_t)receipt->timer_ticket);
    receipt->replaced_existing = replaced;
  }
  snprintf(receipt->source_evidence, sizeof(receipt->source_evidence),
           "skproject c_1c9a.cpp:5695-5728 DM2_1c9a_0cf7 complete slice "
           "(DM2_1c9a_0db0 replace c_1c9a.cpp:5699-5706 + slot word@2 "
           "store c_1c9a.cpp:5724-5728 over the session CAII array)");
  return scheduled;
}
