#include "dm2_v1_caii_alloc_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_proceed_timers_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"

/* Session-wired AI-spec flags provider (see the header).  The CAII
 * module must not depend on the creature module's translation unit, so
 * the session that owns the AIDefinition table wires the provider —
 * firestaff's proven provider is dm2_v1_creature_ai_spec_flags. */
static DM2_V1_CaiiAiSpecFlagsFn g_ai_spec_flags_fn = 0;
static DM2_V1_CaiiWordValueFn g_ai_base_hp_fn = 0;
static DM2_V1_CaiiWordValueFn g_gdat_word1_fn = 0;

void dm2_v1_caii_set_ai_spec_flags_fn(DM2_V1_CaiiAiSpecFlagsFn fn) {
  g_ai_spec_flags_fn = fn;
}

void dm2_v1_caii_set_ai_base_hp_fn(DM2_V1_CaiiWordValueFn fn) {
  g_ai_base_hp_fn = fn;
}

void dm2_v1_caii_set_gdat_word1_fn(DM2_V1_CaiiWordValueFn fn) {
  g_gdat_word1_fn = fn;
}

/* Provider getters for composed slices (the full
 * DM2_DELETE_CREATURE_RECORD composition lives in its own translation
 * unit so caii_alloc keeps its link boundary; the providers and the
 * verbatim mdata table stay module-owned and are read through these
 * accessors). */
DM2_V1_CaiiAiSpecFlagsFn dm2_v1_caii_get_ai_spec_flags_fn(void) {
  return g_ai_spec_flags_fn;
}

DM2_V1_CaiiWordValueFn dm2_v1_caii_get_gdat_word1_fn(void) {
  return g_gdat_word1_fn;
}

/* table1d607e, bound verbatim from skproject/SKULLWIN/mdata.c:1564-1613
 * (struct s_fourb[0x2f] — 4 bytes per entry; uwarr_00[0] = bytes 0-1
 * little-endian).  Per-module source-locked copy: the CCM dispatch
 * module binds table1d613a for its own use, and keeping each table with
 * its consumer preserves the bounded-slice link boundary. */
static const uint8_t dm2_v1_table1d607e[0x2f][4] = {
  { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x40, 0x01, 0x00 },
  { 0x20, 0x00, 0x00, 0x00 }, { 0x80, 0x01, 0x00, 0x00 },
  { 0xe8, 0x00, 0x00, 0x00 }, { 0x20, 0x00, 0x00, 0x00 },
  { 0x20, 0x40, 0x00, 0x00 }, { 0x8c, 0x00, 0x01, 0x00 },
  { 0x84, 0x20, 0x01, 0x00 }, { 0x8c, 0x00, 0x01, 0x00 },
  { 0x8c, 0x00, 0x01, 0x00 }, { 0xa4, 0x00, 0x00, 0x00 },
  { 0x84, 0x00, 0x01, 0x00 }, { 0x01, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x01, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00 }, { 0x02, 0x00, 0x00, 0x00 },
  { 0xe8, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
  { 0x80, 0x00, 0x01, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
  { 0xe8, 0x00, 0x00, 0x00 }, { 0x00, 0x40, 0x00, 0x00 },
  { 0x00, 0x40, 0x00, 0x00 }, { 0xa0, 0x00, 0x00, 0x00 },
  { 0x20, 0x00, 0x00, 0x00 }, { 0x02, 0x00, 0x00, 0x00 },
  { 0x20, 0x01, 0x00, 0x00 }, { 0x00, 0x11, 0x00, 0x00 },
  { 0x01, 0x40, 0x00, 0x00 }, { 0x60, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x00, 0x00 }, { 0x1b, 0x8a, 0x00, 0x00 },
  { 0x01, 0x42, 0x00, 0x00 }, { 0x02, 0x42, 0x00, 0x00 },
  { 0x00, 0x42, 0x00, 0x00 }, { 0x80, 0x40, 0x01, 0x00 },
  { 0x80, 0x00, 0x00, 0x00 }, { 0xe8, 0x00, 0x00, 0x00 },
  { 0x0a, 0x04, 0x00, 0x00 }, { 0x84, 0x00, 0x00, 0x00 },
  { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x40, 0x00, 0x00 },
  { 0x20, 0x00, 0x00, 0x00 }, { 0x20, 0x00, 0x00, 0x00 },
  { 0x80, 0x40, 0x01, 0x00 }
};

int dm2_v1_caii_table1d607e_uc0(int w1) {
  if (w1 < 0 || w1 >= 0x2f) {
    return -1;
  }
  return (int)dm2_v1_table1d607e[w1][0];
}

/* table1d613a, bound verbatim from skproject/SKULLWIN/mdata.c:1615-1639
 * (86 bytes; proven span b_1a 0x00-0x55).  Same per-module source-locked
 * copy rationale as table1d607e above. */
static const uint8_t dm2_v1_table1d613a[86] = {
  0x08, 0x14, 0x14, 0x14, 0x14, 0x14, 0x10, 0x10,
  0x11, 0x14, 0x11, 0x08, 0x08, 0x08, 0x12, 0x12,
  0x08, 0x10, 0x08, 0x10, 0x10, 0x10, 0x10, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x11, 0x12,
  0x12, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x10, 0x10, 0x10, 0x14, 0x14, 0x14,
  0x14, 0x14, 0x14, 0x10, 0x10, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
  0x08, 0x10
};

/* Session-stream LCG draws, bound verbatim from
 * skproject/SKULLWIN/c_random.cpp:13-47 over the DM2_V1_DropRng state.
 * Per-module source-locked copy — the same rationale the drops module
 * documents (dm2_v1_drops.h:70-73): the bounded slice must not drag
 * another translation unit into every CAII consumer.  Note the
 * RAND16(n) form applies CUTX16 BEFORE the modulo (c_random.cpp:23-28),
 * which differs from dm2_v1_drops_rand16 whenever n does not divide
 * 2^16 — RAND16(100) in the reaction roll is exactly such a case. */
#define DM2_V1_CAII_RANDOM_MAGIC 0xbb40e62du

static uint32_t dm2_v1_caii_rand24(DM2_V1_DropRng *rng) {
  uint32_t n = rng->random * DM2_V1_CAII_RANDOM_MAGIC + 11u;
  rng->random = n;
  return n >> 8;
}

static uint16_t dm2_v1_caii_randbit(DM2_V1_DropRng *rng) {
  return (uint16_t)(dm2_v1_caii_rand24(rng) & 1u);
}

static uint16_t dm2_v1_caii_randdir(DM2_V1_DropRng *rng) {
  return (uint16_t)(dm2_v1_caii_rand24(rng) & 3u);
}

static uint16_t dm2_v1_caii_rand16(DM2_V1_DropRng *rng, uint16_t n) {
  uint32_t draw;
  if (n == 0u) {
    return 0u;
  }
  draw = dm2_v1_caii_rand24(rng);
  return (uint16_t)((uint16_t)(draw & 0xffffu) % (uint32_t)n);
}

/* DM2_CALC_VECTOR_DIR (skproject/SKULLWIN/util.cpp:30-46), verbatim:
 * the 4-way direction from (b, c) toward (a, d), with the source's
 * tie-break RANDBIT consumed from the session stream. */
static int dm2_v1_caii_calc_vector_dir(int a, int d, int b, int c,
                                       DM2_V1_DropRng *rng) {
  int rg5 = a - b;
  int rg2 = rg5 < 0 ? -rg5 : rg5;
  int rg4 = d - c;
  int rg3 = rg4 < 0 ? -rg4 : rg4;
  if (rg2 == rg3) {
    if (dm2_v1_caii_randbit(rng) == 0u) {
      rg3++;
    } else {
      rg2++;
    }
  }
  if (rg2 >= rg3) {
    return rg5 <= 0 ? 1 : 3;
  }
  return rg4 <= 0 ? 2 : 0;
}

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

int dm2_v1_caii_free_slot(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int slot_index,
    DM2_V1_CaiiFreeReceipt *receipt) {
  DM2_V1_CaiiFreeReceipt local;
  DM2_V1_CaiiDeleteTimerReceipt del;
  uint8_t *record;
  uint8_t *slot;
  int16_t handle;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_1c9a.cpp:5896-5957 DM2_1c9a_0fcb bounded slice "
           "(pending timer via bound DM2_1c9a_0db0 c_1c9a.cpp:5952; "
           "DELETE_CREATURE_RECORD branch head taken data-backed, "
           "decision head bound, mutating tail receipted unbound)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || caii == NULL || !caii->valid || queue == NULL) {
    return 0;
  }

  /* Bounds: the source compares slot > ddat.v1e08a0 unsigned
   * (c_1c9a.cpp:5905) and would index out of bounds at slot ==
   * capacity; the bounded slice fails closed. */
  if (slot_index < 0 || slot_index >= caii->capacity) {
    receipt->out_of_range = 1;
    return 0;
  }

  slot = caii->slots + (size_t)slot_index * DM2_V1_CAII_SLOT_SIZE;
  if ((int16_t)dm2_v1_read_u16le(slot + 0) < 0) {
    receipt->already_free = 1;
    return 0;
  }

  /* c_1c9a.cpp:5915: rebuild the DB4 handle from the bare index. */
  receipt->record_index = dm2_v1_read_u16le(slot + 0) & 0x3ffu;
  handle = (int16_t)(0x1000 | receipt->record_index);
  record = dm2_v1_record_pool_address_mut(pool_set, handle);
  if (record == NULL) {
    return 0;
  }

  /* The record-delete flag derives from DM2_QUERY_CREATURE_AI_SPEC_FLAGS
   * (c_1c9a.cpp:5917-5929): flag = ((flags & 0x1) == 0 && byte@1a ==
   * 0x13).  It is computed data-backed through the wired AI-spec flags
   * provider (record byte@4 = creature type); -1 when no provider is
   * wired or the session loaded no AI row for the type. */
  {
    uint16_t ai_flags = 0;
    receipt->record_delete_flag = -1;
    if (g_ai_spec_flags_fn != 0 &&
        g_ai_spec_flags_fn((int)record[4], &ai_flags) == 1) {
      receipt->record_delete_flag =
          ((ai_flags & 0x1u) == 0u && slot[0x1a] == 0x13u) ? 1 : 0;
    }
  }
  receipt->record_delete_unbound = 1;

  /* Branch head (c_1c9a.cpp:5936-5948): flag set AND a pending timer ->
   * read the payload BEFORE the timer is deleted (source order: payload
   * 5939-5945, byte@1a 5950, DM2_1c9a_0db0 5952).  The valueA lo/hi
   * bytes are the branch's (x, y) arguments (c_timer.h getxA/getyA
   * return the valueA bytes). */
  if (receipt->record_delete_flag == 1) {
    uint16_t word2 = dm2_v1_read_u16le(slot + 2);
    if (word2 == 0xffffu || word2 == 0u) {
      /* c_1c9a.cpp:5946-5947: no pending timer forces RG3L = 0. */
      receipt->record_delete_no_timer = 1;
    } else {
      DM2_V1_SourceTimer pending;
      if (dm2_v1_source_timer_peek_ticket(queue, (uint32_t)word2,
                                          &pending) == 1) {
        receipt->record_delete_branch = 1;
        receipt->record_delete_x =
            (int)((uint16_t)pending.value_a & 0xffu);
        receipt->record_delete_y =
            (int)(((uint16_t)pending.value_a >> 8) & 0xffu);
      } else {
        /* Stale ticket (already popped): the source's indices are
         * always live, so mirror the no-timer outcome fail-closed. */
        receipt->record_delete_no_timer = 1;
      }
    }
  }

  slot[0x1a] = 0;                                     /* c_1c9a.cpp:5950 */

  /* c_1c9a.cpp:5952: delete the pending timer through the bound
   * DM2_1c9a_0db0 path. */
  memset(&del, 0, sizeof(del));
  (void)dm2_v1_caii_delete_timer(pool_set, caii, queue, handle, &del);
  receipt->had_pending_timer = del.deleted;

  if (caii->alloc_count > 0) {
    caii->alloc_count--;                              /* ddat.v1d4020-- */
  }
  record[5] = 0xffu;                                  /* record byte@5 */
  dm2_v1_write_u16le(slot + 0, 0xffffu);              /* slot free */

  /* c_1c9a.cpp:5956-5957: the branch runs DM2_DELETE_CREATURE_RECORD
   * after the slot is marked free (record byte@5 already -1, matching
   * the source order).  The bound decision head resolves the creature
   * and computes the gates; its mutating tail stays unbound (receipted
   * there). */
  if (receipt->record_delete_branch == 1 && dungeon != NULL) {
    DM2_V1_CaiiDeleteCreatureRecordReceipt head;
    memset(&head, 0, sizeof(head));
    if (dm2_v1_caii_delete_creature_record_head(
            pool_set, dungeon, caii, receipt->record_delete_x,
            receipt->record_delete_y, &head) == 1) {
      receipt->record_delete_head_resolved = 1;
    }
  }

  receipt->freed = 1;
  receipt->valid = 1;
  return 1;
}

int dm2_v1_caii_delete_creature_record_head(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    int x, int y,
    DM2_V1_CaiiDeleteCreatureRecordReceipt *receipt) {
  DM2_V1_CaiiDeleteCreatureRecordReceipt local;
  uint8_t *record;
  int16_t handle;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_record.cpp:1357-1425 DM2_DELETE_CREATURE_RECORD "
           "bounded decision head (GET_CREATURE_AT + jz_test8 AI gate "
           "data-backed + CAII byte@1a clear c_record.cpp:1408-1413; "
           "invoke-message, tile-rooted move, possession drop, "
           "1c9a_0247 and dealloc receipted unbound)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || dungeon == NULL ||
      x < 0 || y < 0 || x > 0xff || y > 0xff) {
    return 0;
  }

  /* c_record.cpp:1377-1380: resolve the creature at the payload
   * coordinates; the session is single-map (map index 0) exactly like
   * the scheduling producer (c_1c9a.cpp:5698 binding). */
  handle = dm2_v1_get_creature_at(pool_set, dungeon, 0, x, y);
  if (handle == DM2_V1_RECORD_HANDLE_NULL) {
    receipt->creature_not_found = 1;
    return 0;
  }
  receipt->resolved = 1;
  receipt->record_handle = handle;

  record = dm2_v1_record_pool_address_mut(pool_set, handle);
  if (record == NULL) {
    return 0;
  }
  receipt->creature_type = (int)record[4];        /* c_record.cpp:1384 */

  /* jz_test8(aidef byte@0, 1) gate, c_record.cpp:1385 — data-backed
   * through the wired provider; without provenance the gate is treated
   * as not taken (fail-closed). */
  {
    uint16_t ai_flags = 0;
    if (g_ai_spec_flags_fn != 0 &&
        g_ai_spec_flags_fn((int)record[4], &ai_flags) == 1) {
      receipt->ai_flags_known = 1;
      receipt->ai_bit0_clear = (ai_flags & 0x1u) == 0u ? 1 : 0;
    }
  }

  if (receipt->ai_bit0_clear == 1) {
    /* c_record.cpp:1387-1388: the table1d607e[GDAT CREATURES word@1]
     * uc[0] & 0x4 probe decides whether the map-swap/DM2_INVOKE_MESSAGE
     * branch runs — now computed data-backed through the wired word@1
     * provider and the verbatim mdata table.  The branch itself
     * (c_record.cpp:1390-1406: DM2_CHANGE_CURRENT_MAP_TO swap and
     * message scheduling) stays host-owned — receipted, never
     * simulated. */
    receipt->invoke_message_would_run = -1;
    {
      uint16_t w1 = 0;
      if (g_gdat_word1_fn != 0 &&
          g_gdat_word1_fn((int)record[4], &w1) == 1 && w1 < 0x2fu) {
        receipt->invoke_message_would_run =
            (dm2_v1_table1d607e[w1][0] & 0x4u) == 0u ? 1 : 0;
      }
    }
    receipt->invoke_message_unbound = 1;

    /* c_record.cpp:1408-1413: a creature still owning a CAII slot gets
     * its slot mode byte cleared.  In the 0fcb call order byte@5 is
     * already -1, so this fires only for direct callers. */
    if (record[5] != 0xffu && caii != NULL && caii->valid &&
        (int)record[5] < caii->capacity) {
      caii->slots[(size_t)record[5] * DM2_V1_CAII_SLOT_SIZE + 0x1a] = 0;
      receipt->slot_mode_cleared = 1;
    }
  }

  /* The mutating tail (c_record.cpp:1416-1424) stays unbound behind
   * the DM2-002 ground-stack/possession blockers and the unmodelled
   * dballoc free chain — each receipted, never simulated. */
  receipt->move_record_unbound = 1;
  receipt->drop_possession_unbound = 1;
  receipt->dballoc_cleanup_unbound = 1;
  receipt->dealloc_record_unbound = 1;

  receipt->valid = 1;
  return 1;
}

/* Bounded membership pre-walk: the record must be chained on the tile
 * before the pool cut runs, so a corrupt chain can never spin the
 * (unbudgeted) source splice loop.  Budget mirrors the round-16 walk
 * primitive: no source chain outlives the declared record count. */
static int dm2_v1_record_chained_on_tile(DM2_V1_RecordPoolSet *pool_set,
                                         const DM2_V1_DungeonData *dungeon,
                                         int x, int y,
                                         int16_t record) {
  int16_t cursor;
  long budget = 1;
  int first;

  first = dm2_v1_dungeon_get_first_thing(dungeon, 0, x, y);
  if (first < 0) {
    return 0;
  }
  cursor = (int16_t)first;
  for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
    budget += pool_set->pools[i].record_count;
  }
  while (cursor != DM2_V1_RECORD_HANDLE_END &&
         cursor != DM2_V1_RECORD_HANDLE_NULL) {
    int16_t next;

    if (budget-- <= 0) {
      return 0; /* corrupt chain: bounded, fail-closed */
    }
    if (cursor == record) {
      return 1;
    }
    if (!dm2_v1_record_pool_next_link(pool_set, cursor, &next)) {
      return 0;
    }
    cursor = next;
  }
  return 0;
}

int dm2_v1_caii_delete_creature_record_tail(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    int16_t record_handle,
    int x, int y,
    DM2_V1_CaiiDeleteCreatureTailReceipt *receipt) {
  DM2_V1_CaiiDeleteCreatureTailReceipt local;
  uint8_t *record;
  int16_t head;
  int16_t head_before;
  int first;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_record.cpp:1416-1424 DM2_DELETE_CREATURE_RECORD "
           "mutating tail (tile-rooted cut c_moverec.cpp:630-683 end "
           "state + DM2_DEALLOC_RECORD c_record.cpp:1205-1208 bound; "
           "3CE7D side effects, DROP_CREATURE_POSSESSION and 1c9a_0247 "
           "receipted unbound)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || !pool_set->valid || dungeon == NULL ||
      record_handle == DM2_V1_RECORD_HANDLE_NULL ||
      record_handle == DM2_V1_RECORD_HANDLE_END ||
      x < 0 || y < 0 || x > 0xff || y > 0xff) {
    return 0;
  }
  record = dm2_v1_record_pool_address_mut(pool_set, record_handle);
  if (record == NULL) {
    return 0;
  }
  first = dm2_v1_dungeon_get_first_thing(dungeon, 0, x, y);
  if (first < 0 ||
      !dm2_v1_record_chained_on_tile(pool_set, dungeon, x, y,
                                     record_handle)) {
    receipt->cut_miss = 1;
    return 0;
  }

  /* c_record.cpp:1419 — the tile-rooted cut.  The pool splice mirrors
   * DM2_CUT_RECORD_FROM's list semantics; the head write-back lands in
   * the dungeon ground-stack table only when the record was the head. */
  head = (int16_t)first;
  head_before = head;
  if (!dm2_v1_record_pool_cut_from_list(pool_set, &head, record_handle)) {
    receipt->cut_miss = 1;
    return 0;
  }
  receipt->cut_performed = 1;
  if (head != head_before) {
    if (dm2_v1_dungeon_set_first_thing(dungeon, 0, x, y,
                                       (uint16_t)head) != 0) {
      return 0;
    }
    receipt->cut_head_rewritten = 1;
  }
  receipt->cut_side_effects_unbound = 1;
  receipt->drop_possession_unbound = 1;
  receipt->dballoc_cleanup_unbound = 1;

  /* c_record.cpp:1424 via c_record.cpp:1205-1208 — DM2_DEALLOC_RECORD:
   * the record's first word becomes the 0xffff free marker. */
  record[0] = 0xffu;
  record[1] = 0xffu;
  receipt->dealloc_performed = 1;

  receipt->valid = 1;
  return 1;
}

int dm2_v1_caii_attack_guard_allows_alloc(
    DM2_V1_RecordPoolSet *pool_set,
    int16_t record_handle) {
  const uint8_t *record;
  uint16_t ai_flags = 0;

  if (pool_set == NULL) {
    return 0;
  }

  /* The gate lives in DM2_ATTACK_CREATURE (c_creature.cpp:370-385) which
   * only runs for creature records; reject other DBs fail-closed like
   * dm2_v1_caii_delete_timer (c_1c9a.cpp:5741-5744). */
  if (dm2_v1_record_handle_pool(record_handle) != 4) {
    return 0;
  }
  record = dm2_v1_record_pool_address(pool_set, record_handle);
  if (record == NULL) {
    return 0;
  }

  /* vl_18 = AIDefinition word@0 & 0x1 (c_creature.cpp:374-378), resolved
   * from record byte@4 through the source's CREATURES word@5 -> AI-row
   * indirection (c_record.cpp:1351-1354), data-backed through the wired
   * AI-spec flags provider.  Unknown flags are receipted as -1 so
   * callers can distinguish "gate closed" from "provenance missing". */
  if (g_ai_spec_flags_fn == 0 ||
      g_ai_spec_flags_fn((int)record[4], &ai_flags) != 1) {
    return -1;
  }
  return (ai_flags & 0x1u) != 0u ? 1 : 0;
}

int dm2_v1_caii_attack_creature(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_id,
    unsigned long game_tick,
    int16_t record_handle,
    int x, int y,
    int target_x, int target_y,
    uint32_t attack_word,
    int16_t attack_strength,
    int32_t hp_delta,
    DM2_V1_CaiiAttackReceipt *receipt) {
  DM2_V1_CaiiAttackReceipt local;
  uint32_t vol;
  uint8_t *record;
  uint8_t *slot;
  int16_t handle;
  int rg7;
  int rg7_unknown;
  int vl_10;
  int vl_14;
  int vl_18;
  int vw_20;
  int rg1;
  int hp_word;
  uint16_t ai_flags = 0;
  uint16_t w0a;

  memset(&local, 0, sizeof(local));
  local.reaction_roll = -1;
  local.ai_turn_gate_passed = -1;
  local.ai_turn_entry_roll = -1;
  local.ai_turn_vector_dir = -1;
  local.ai_turn_facing = -1;
  local.ai_turn_dir = -2;
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_creature.cpp:318-649 DM2_ATTACK_CREATURE bounded "
           "slice (alloc c_creature.cpp:379-385, HP add 389-393, aggro "
           "394-435, c_ai turn block 438-536 bound with CALC_VECTOR_DIR "
           "util.cpp:30-46 + DM2_ai_13e4_0360 argl0==0 "
           "c_ai.cpp:5912-5960, reaction roll + champion bit 539-563, "
           "reschedule gate 566-635 with mdata.c:1564-1639 tables, "
           "0db0+0cf7 647-648)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || dungeon == NULL || caii == NULL ||
      !caii->valid || queue == NULL ||
      x < 0 || y < 0 || x > 0xff || y > 0xff ||
      target_x < 0 || target_y < 0 || target_x > 0xff || target_y > 0xff) {
    return 0;
  }

  vol = attack_word;

  /* c_creature.cpp:345-352: handle -1 resolves at (x, y). */
  handle = record_handle;
  if (record_handle == DM2_V1_RECORD_HANDLE_NULL) {
    handle = dm2_v1_get_creature_at(pool_set, dungeon, 0, x, y);
    if (handle == DM2_V1_RECORD_HANDLE_NULL) {
      receipt->creature_not_found = 1;
      return 0;
    }
  }
  receipt->record_handle = handle;

  /* c_creature.cpp:353-362: bit 0x4000 survives a RANDBIT coin flip. */
  rg7 = 0;
  rg7_unknown = 0;
  if ((vol & 0x4000u) != 0u) {
    rg7 = 1;
    vol &= ~0x4000u;                                  /* and16 0xbfff */
    if (rng == NULL) {
      receipt->rng_unbound = 1;
      rg7_unknown = 1;
    } else if (dm2_v1_caii_randbit(rng) != 0) {
      rg7 = 0;
    }
  }

  /* c_creature.cpp:363-369: bit 0x2000 -> vl_10. */
  vl_10 = (vol & 0x2000u) != 0u ? 1 : 0;
  if (vl_10 != 0) {
    vol &= ~0x2000u;                                  /* and16 0xdfff */
  }

  record = dm2_v1_record_pool_address_mut(pool_set, handle);
  if (record == NULL) {
    return 0;
  }
  receipt->creature_type = (int)record[4];

  /* vl_18 = AIDefinition word@0 & 1 (c_creature.cpp:374-378),
   * data-backed through the wired provider; the source never lacks the
   * table, so unknown provenance fails the whole body closed. */
  if (g_ai_spec_flags_fn == 0 ||
      g_ai_spec_flags_fn((int)record[4], &ai_flags) != 1) {
    receipt->ai_flags_unknown = 1;
    return 0;
  }
  vl_18 = (ai_flags & 0x1u) != 0u ? 1 : 0;

  /* c_creature.cpp:379-385: no CAII slot -> alloc only when vl_18 set. */
  if (record[5] == 0xffu) {
    if (vl_18 == 0) {
      receipt->denied_static_no_slot = 1;
      return 0;
    }
    {
      DM2_V1_CaiiAllocReceipt alloc_rc;
      memset(&alloc_rc, 0, sizeof(alloc_rc));
      if (dm2_v1_caii_alloc_to_creature(pool_set, dungeon, caii, queue,
                                        map_id, game_tick, handle,
                                        x, y, &alloc_rc) != 1) {
        receipt->alloc_failed = 1;
        return 0;
      }
    }
    receipt->alloc_performed = 1;
  }

  slot = caii->slots + (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;

  /* c_creature.cpp:389-393: slot word@0x14 += hp_delta, 16-bit wrap. */
  hp_word = (int)(int16_t)(dm2_v1_read_u16le(slot + 0x14) +
                           (uint16_t)(int16_t)hp_delta);
  dm2_v1_write_u16le(slot + 0x14, (uint16_t)(int16_t)hp_word);
  receipt->hp_word_after = hp_word;

  /* c_creature.cpp:394-435: the aggro block. */
  w0a = dm2_v1_read_u16le(record + 0xa);
  if (vl_18 == 0 && attack_strength > 0 && (w0a & 0x4u) == 0u) {
    int set = -1;
    receipt->aggro_evaluated = 1;
    if (hp_word > 0x1e) {
      set = 1;
    } else if (hp_word <= 4) {
      set = -2;                                     /* percentage probe */
    } else {
      /* c_creature.cpp:411-414: RANDDIR != 0 -> percentage probe,
       * RANDDIR == 0 -> aggro directly. */
      if (rng == NULL) {
        receipt->rng_unbound = 1;
      } else if (dm2_v1_caii_randdir(rng) != 0) {
        set = -2;
      } else {
        set = 1;
      }
    }
    if (set == -2) {
      /* c_creature.cpp:420-423: 100*hp / aidef word@4 > 0xf. */
      uint16_t base_hp = 0;
      if (g_ai_base_hp_fn == 0 ||
          g_ai_base_hp_fn((int)record[4], &base_hp) != 1 ||
          base_hp == 0u) {
        /* No provenance — or the source would divide by zero. */
        receipt->aggro_undecided = 1;
        set = -1;
      } else {
        uint32_t pct =
            (100u * (uint32_t)(uint16_t)hp_word) / (uint32_t)base_hp;
        set = pct > 0xfu ? 1 : 0;
      }
    }
    if (set == 1) {
      w0a |= 0x4u;                                  /* c_creature.cpp:433 */
      dm2_v1_write_u16le(record + 0xa, w0a);
      receipt->aggro_set = 1;
      rg7 = 1;
    } else if (set != 0) {
      receipt->aggro_undecided = 1;
    }
  }

  /* c_creature.cpp:438-536: the c_ai turn block, bound.  Its entry gate
   * (table1d607e[GDAT word@1].uc[0] & 0x80) is data-backed; when the
   * gate passes and a session stream is bound the whole direction dance
   * runs with the source's exact RNG draw sequence.  When the gate
   * cannot be determined, or the gate passes without a bound stream,
   * the body still stops BEFORE the reaction roll — fail-closed. */
  if (rg7 != 0 || rg7_unknown != 0) {
    int gate = -1;
    uint16_t w1 = 0;
    receipt->ai_turn_unbound = 1;
    if (g_gdat_word1_fn != 0 &&
        g_gdat_word1_fn((int)record[4], &w1) == 1) {
      if (w1 < 0x2fu) {
        gate = (dm2_v1_table1d607e[w1][0] & 0x80u) == 0u ? 1 : 0;
      } else {
        receipt->gdat_w1_out_of_span = 1;
      }
    } else {
      receipt->gdat_w1_unknown = 1;
    }
    receipt->ai_turn_gate_passed = gate;
    if (gate != 0) {
      if (gate < 0 || rng == NULL) {
        receipt->rng_stream_diverged = 1;
        return 0;
      }
      receipt->ai_turn_unbound = 0;
      receipt->ai_turn_ran = 1;
      /* c_creature.cpp:444-445: the entry coin flip. */
      receipt->ai_turn_entry_roll = (int)dm2_v1_caii_randbit(rng);
      if (receipt->ai_turn_entry_roll != 0) {
        int rg4;
        int rg2;
        int dir_e;
        int blo = 0;
        int skip00247 = 0;
        int skip00248 = 0;
        int skip00251 = 0;
        /* c_creature.cpp:449: direction from the creature's CCM
         * dispatch coordinates toward the attack origin. */
        rg4 = dm2_v1_caii_calc_vector_dir(x, y, target_x, target_y, rng);
        receipt->ai_turn_vector_dir = rg4;
        dir_e = (int)((dm2_v1_read_u16le(record + 0xe) >> 8) & 0x3u);
        receipt->ai_turn_facing = dir_e;
        /* c_creature.cpp:451-460: the word@0xa & 8 branch. */
        if ((dm2_v1_read_u16le(record + 0xa) & 0x8u) != 0u) {
          if (dm2_v1_caii_randdir(rng) == 0u) {
            skip00247 = 1;
          }
        } else {
          skip00247 = 1;
        }
        /* c_creature.cpp:462-476. */
        if (skip00247 != 0) {
          if (rg4 != dir_e && dm2_v1_caii_randdir(rng) == 0u) {
            skip00248 = 1;
          }
        } else {
          skip00248 = 1;
        }
        /* c_creature.cpp:478-483: reverse. */
        if (skip00248 != 0) {
          rg4 = (rg4 + 2) & 3;
        }
        /* c_creature.cpp:485-528: the final direction dance. */
        rg2 = (rg4 + 2) & 3;
        if (dir_e != rg2) {
          if (dir_e == rg4) {
            if (dm2_v1_caii_randdir(rng) != 0u) {
              rg4 = -1;
            } else {
              blo = dm2_v1_caii_randbit(rng) == 0u ? 1 : 0;
              rg4 = blo + 6;
            }
            skip00251 = 1;
          } else {
            rg4 = (rg4 - 1) & 3;
            blo = dir_e == rg4 ? 1 : 0;
          }
        } else {
          blo = dm2_v1_caii_randbit(rng) != 0u ? 1 : 0;
        }
        if (skip00251 == 0) {
          rg4 = blo + 6;
        }
        receipt->ai_turn_dir = rg4;
        /* c_creature.cpp:531-533 -> DM2_ai_13e4_0360(handle, x, y, dir,
         * 0), c_ai.cpp:5912-5960: with argl0 == 0 the action is the
         * byte@0x17 direction write behind the 0x13 guards.  The
         * record owns a slot at this point (the body allocated one
         * when byte@5 was 0xff), so the source's slot-index and
         * record guards are already satisfied. */
        if (rg4 != -1) {
          if (slot[0x17] == 0x13u || slot[0x1a] == 0x13u) {
            receipt->ai_turn_guard_denied = 1;
          } else {
            slot[0x17] = (uint8_t)rg4;         /* c_ai.cpp:5946 */
            receipt->ai_turn_applied = 1;
          }
        }
      } else {
        receipt->ai_turn_dir = -1;             /* entry flip: no turn */
      }
    }
  }

  /* c_creature.cpp:539-543: vl_14 = strength > RAND16(100). */
  if (rng == NULL) {
    receipt->rng_unbound = 1;
    return 0;
  }
  {
    uint16_t draw = dm2_v1_caii_rand16(rng, 100);
    receipt->reaction_roll = (int)draw;
    vl_14 = attack_strength > (int)draw ? 1 : 0;
  }
  receipt->reaction_success = vl_14;

  /* c_creature.cpp:546-563: champion bit into/out of record word@0xa. */
  vw_20 = 0;
  if (vl_14 != 0) {
    uint32_t bit;
    vw_20 = (vol & 0x8000u) != 0u ? 1 : 0;
    /* 1 << (vol low byte): the original binary runs x86 SHL whose count
     * is taken mod 32; poke16 keeps the low word. */
    bit = (1u << ((vol & 0xffu) & 31u)) & 0xffffu;
    vol = (vol & 0xffff0000u) | bit;                  /* poke16 */
    w0a = dm2_v1_read_u16le(record + 0xa);
    if (vw_20 == 0) {
      dm2_v1_write_u16le(record + 0xa,
                         (uint16_t)(w0a | (uint16_t)bit));
      receipt->champion_bit_set = 1;
    } else {
      dm2_v1_write_u16le(record + 0xa,
                         (uint16_t)(w0a & (uint16_t)~bit));
      receipt->champion_bit_cleared = 1;
    }
  }

  /* c_creature.cpp:566-635: the reschedule gate. */
  if (vl_18 == 0 && vl_10 != 0 && attack_strength == 0) {
    rg1 = 1;
  } else if (vl_14 == 0) {
    rg1 = 0;
  } else {
    int skip00254 = (vw_20 != 0) || ((vol & 0x40u) != 0u);
    if (skip00254 != 0 && vl_10 == 0) {
      rg1 = 0;
    } else {
      uint8_t t;
      if (slot[0x1a] >= 86u) {
        /* table1d613a proven span is 0x00-0x55; the source would read
         * out of bounds — fail closed. */
        receipt->mode_b1a_out_of_span = 1;
        return 0;
      }
      t = dm2_v1_table1d613a[slot[0x1a]];
      if ((t & 0x10u) == 0u) {
        rg1 = 1;
      } else {
        uint16_t w1 = 0;
        uint32_t t6;
        if (g_gdat_word1_fn == 0 ||
            g_gdat_word1_fn((int)record[4], &w1) != 1) {
          receipt->gdat_w1_unknown = 1;
          return 0;
        }
        if (w1 >= 0x2fu) {
          receipt->gdat_w1_out_of_span = 1;
          return 0;
        }
        t6 = (uint32_t)dm2_v1_table1d607e[w1][0] |
             ((uint32_t)dm2_v1_table1d607e[w1][1] << 8);
        if ((t6 & 0x410u) == 0u) {
          rg1 = 0;
        } else {
          rg1 = (t & 0x2u) != 0u ? 1 : 0;
        }
      }
    }
  }
  receipt->final_rg1 = rg1;

  /* c_creature.cpp:638-640: dying creatures never reschedule. */
  if (slot[0x1a] == 0x13u) {
    receipt->dying_mode = 1;
    return 0;
  }

  /* c_creature.cpp:641-646: below the record word@6 threshold the
   * creature keeps its current timer. */
  if (rg1 == 0 &&
      (uint16_t)(int16_t)hp_word < dm2_v1_read_u16le(record + 6)) {
    receipt->below_threshold = 1;
    return 0;
  }

  /* c_creature.cpp:647-648: the bound DM2_1c9a_0db0 + DM2_1c9a_0cf7. */
  {
    DM2_V1_CaiiDeleteTimerReceipt del;
    DM2_V1_CreatureScheduleReceipt sched;
    memset(&del, 0, sizeof(del));
    if (dm2_v1_caii_delete_timer(pool_set, caii, queue, handle,
                                 &del) == 1) {
      receipt->timer_cancelled = 1;
    }
    memset(&sched, 0, sizeof(sched));
    if (dm2_v1_caii_schedule_creature_at(pool_set, dungeon, caii, queue,
                                         map_id, game_tick, x, y,
                                         &sched) != 1) {
      return 0;
    }
    receipt->rescheduled = 1;
    receipt->timer_ticket = (uint32_t)sched.timer_ticket;
  }

  receipt->completed = 1;
  receipt->valid = 1;
  return 1;
}

int dm2_v1_caii_ai_13e4_0360(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int map_id,
    unsigned long game_tick,
    int16_t record_handle,
    int x, int y,
    int dir,
    int argl0,
    DM2_V1_CaiiAiTurnReceipt *receipt) {
  DM2_V1_CaiiAiTurnReceipt local;
  uint8_t *record;
  uint8_t *slot;
  int16_t handle;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_ai.cpp:5912-5960 DM2_ai_13e4_0360 complete slice "
           "(guards 5934-5944, byte@0x17 write 5945-5946, argl0 tail "
           "5949-5959 with mdata.c:1615-1639 table1d613a + bound "
           "0db0/0cf7)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || dungeon == NULL || caii == NULL ||
      !caii->valid || queue == NULL ||
      x < 0 || y < 0 || x > 0xff || y > 0xff) {
    return 0;
  }

  /* c_ai.cpp:5925-5931: handle -1 resolves at (x, y). */
  handle = record_handle;
  if (record_handle == DM2_V1_RECORD_HANDLE_NULL) {
    handle = dm2_v1_get_creature_at(pool_set, dungeon, 0, x, y);
    if (handle == DM2_V1_RECORD_HANDLE_NULL) {
      receipt->creature_not_found = 1;
      return 0;
    }
  }
  receipt->record_handle = handle;

  /* Same record-DB discipline as the bound 0db0 slice
   * (c_1c9a.cpp:5741-5744): direction bits ignored. */
  if (dm2_v1_record_handle_pool(handle) != 4) {
    receipt->not_creature_db = 1;
    return 0;
  }
  record = dm2_v1_record_pool_address_mut(pool_set, handle);
  if (record == NULL) {
    return 0;
  }

  /* c_ai.cpp:5934-5936: the record must own a CAII slot. */
  if (record[5] == 0xffu) {
    receipt->no_slot = 1;
    return 0;
  }
  receipt->slot_index = record[5];
  slot = caii->slots + (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;

  /* c_ai.cpp:5941-5944: the 0x13 guards (AI stopped / dying). */
  if (slot[0x17] == 0x13u || slot[0x1a] == 0x13u) {
    receipt->guard_denied = 1;
    return 0;
  }

  /* c_ai.cpp:5945-5946: the direction write. */
  slot[0x17] = (uint8_t)(dir & 0xff);
  receipt->dir_written = 1;

  /* c_ai.cpp:5947-5948: argl0 == 0 returns after the write. */
  if (argl0 == 0) {
    receipt->completed = 1;
    receipt->valid = 1;
    return 1;
  }
  receipt->argl0_tail = 1;

  /* c_ai.cpp:5949-5959: table1d613a[slot byte@1a] & 0x10 flags byte@0x21;
   * otherwise cancel + requeue.  Proven span 0x00-0x55 — beyond it the
   * source reads out of bounds; fail closed (the dir write already
   * happened, exactly like the source). */
  if (slot[0x1a] >= 86u) {
    receipt->mode_b1a_out_of_span = 1;
    return 0;
  }
  if ((dm2_v1_table1d613a[slot[0x1a]] & 0x10u) != 0u) {
    slot[0x21] = 1u;                              /* c_ai.cpp:5952 */
    receipt->flag_set = 1;
    receipt->completed = 1;
    receipt->valid = 1;
    return 1;
  }
  {
    DM2_V1_CaiiDeleteTimerReceipt del;
    DM2_V1_CreatureScheduleReceipt sched;
    memset(&del, 0, sizeof(del));
    if (dm2_v1_caii_delete_timer(pool_set, caii, queue, handle,
                                 &del) == 1) {
      receipt->timer_cancelled = 1;               /* c_ai.cpp:5955 */
    }
    memset(&sched, 0, sizeof(sched));
    if (dm2_v1_caii_schedule_creature_at(pool_set, dungeon, caii, queue,
                                         map_id, game_tick, x, y,
                                         &sched) != 1) {
      return 0;
    }
    receipt->rescheduled = 1;                     /* c_ai.cpp:5956-5958 */
    receipt->timer_ticket = (uint32_t)sched.timer_ticket;
  }

  receipt->completed = 1;
  receipt->valid = 1;
  return 1;
}

int dm2_v1_caii_ccm_end_requeue(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    int16_t record_handle,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    int loop_result,
    int suppress_requeue,
    int16_t mticks_map,
    int32_t mticks_delta,
    DM2_V1_CaiiCcmEndRequeueReceipt *receipt) {
  DM2_V1_CaiiCcmEndRequeueReceipt local;
  DM2_V1_SourceTimer rebuilt;
  DM2_V1_SourceTimerResult enq_result;
  uint8_t *record;
  uint8_t *slot;
  uint32_t ticket;

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence),
           "skproject c_ai.cpp:5608-5614+5641-5646 DM2_PROCEED_CCM end "
           "re-queue bounded slice (setmticks c_timer.h:66, bound "
           "0db0 cancel, DM2_QUEUE_TIMER c_timer.cpp:235-257, slot "
           "word@2 ticket store)");

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || caii == NULL || !caii->valid || queue == NULL ||
      timer == NULL) {
    return 0;
  }

  /* Same record-DB discipline as the bound 0db0 slice
   * (c_1c9a.cpp:5741-5744): direction bits ignored. */
  if (dm2_v1_record_handle_pool(record_handle) != 4) {
    receipt->not_creature_db = 1;
    return 0;
  }
  record = dm2_v1_record_pool_address_mut(pool_set, record_handle);
  if (record == NULL) {
    return 0;
  }
  if (record[5] == 0xffu) {
    /* The source would index the creatures array with slot 0xff — the
     * CCM loop only runs for activated creatures, so fail closed. */
    receipt->no_slot = 1;
    return 0;
  }
  slot = caii->slots + (size_t)record[5] * DM2_V1_CAII_SLOT_SIZE;

  /* c_ai.cpp:5609-5611: the timer type from the loop result. */
  receipt->timer_type = (loop_result != 1 ? 1 : 0) + 0x21;

  /* c_ai.cpp:5612-5613: s350.v1e0570 suppresses the whole requeue. */
  if (suppress_requeue != 0) {
    receipt->suppressed = 1;
    return 0;
  }

  /* c_ai.cpp:5614 + c_timer.h:66: setmticks(m, t) ORs the delta into
   * the low 24 bits UNMASKED — kept verbatim; the map rides the high
   * byte.  The loop-owned payload fields pass through unchanged. */
  rebuilt = *timer;
  rebuilt.type = (uint8_t)receipt->timer_type;
  rebuilt.ticks_and_map =
      ((uint32_t)(int32_t)mticks_map << 24) | (uint32_t)mticks_delta;

  /* c_ai.cpp:5641-5643: a pending timer (slot word@2 != -1) is
   * cancelled through the bound DM2_1c9a_0db0 first. */
  if (dm2_v1_read_u16le(slot + 2) != 0xffffu) {
    DM2_V1_CaiiDeleteTimerReceipt del;
    memset(&del, 0, sizeof(del));
    if (dm2_v1_caii_delete_timer(pool_set, caii, queue, record_handle,
                                 &del) == 1) {
      receipt->timer_cancelled = 1;
    }
  }

  /* c_ai.cpp:5644-5646: DM2_QUEUE_TIMER + the ticket store. */
  ticket = dm2_v1_source_timer_enqueue_ticketed(queue, &rebuilt,
                                                source_index,
                                                &enq_result);
  if (ticket == 0u) {
    return 0;
  }
  receipt->enqueued = 1;
  receipt->timer_ticket = ticket;
  dm2_v1_write_u16le(slot + 2, (uint16_t)ticket);

  receipt->completed = 1;
  receipt->valid = 1;
  return 1;
}
