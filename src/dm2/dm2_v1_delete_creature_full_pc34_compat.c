/*
 * dm2_v1_delete_creature_full_pc34_compat.c — bounded test/probe
 * DM2_DELETE_CREATURE_RECORD composition
 * (skproject/SKULLWIN/c_record.cpp:1357-1425).  See the header for the
 * full source-lock documentation.
 */

#include "dm2_v1_delete_creature_full_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_think_creature_pc34_compat.h"

static const char dm2_v1_delete_creature_full_evidence[] =
    "skproject c_record.cpp:1357-1425 DM2_DELETE_CREATURE_RECORD complete "
    "composition (GET_CREATURE_AT + jz_test8 gate + table1d607e probe "
    "data-backed; map-swap/DM2_INVOKE_MESSAGE c_tim_proc.cpp:4332-4367, "
    "tile-rooted cut c_moverec.cpp:630-683 end state, "
    "DROP_CREATURE_POSSESSION c_record.cpp:1537-1752 and DEALLOC_RECORD "
    "c_record.cpp:1205-1208 bound; 3CE7D side effects and 1c9a_0247 "
    "receipted host-owned)";

const char *dm2_v1_delete_creature_full_source_evidence(void) {
  return dm2_v1_delete_creature_full_evidence;
}

static uint16_t dm2_v1_delete_full_read_u16le(const uint8_t *p) {
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

/* Bounded membership pre-walk (same discipline as the caii module's
 * standalone tail: no source chain outlives the declared record count).
 * Runs BEFORE any mutation so a corrupt chain fails the composition
 * closed without partial effects. */
static int dm2_v1_delete_full_chained_on_tile(
    DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon,
    int map, int x, int y,
    int16_t record) {
    int16_t cursor;
    size_t budget = 1u;
    int first;
    int found = 0;

  first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
  if (first < 0) {
    return 0;
  }
    cursor = (int16_t)first;
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        if (pool_set->pools[i].record_count > 0)
            budget += (size_t)pool_set->pools[i].record_count;
        if (pool_set->pools[i].extension_count > 0)
            budget += (size_t)pool_set->pools[i].extension_count;
    }
    while (cursor != DM2_V1_RECORD_HANDLE_END &&
           cursor != DM2_V1_RECORD_HANDLE_NULL) {
        int16_t next;

        if (budget-- == 0u) {
            return 0;
        }
        if (cursor == record) {
            found = 1;
        }
        if (!dm2_v1_record_pool_next_link(pool_set, cursor, &next)) {
            return 0;
        }
        /* OBJECT_NULL is not the source empty-chain terminator.  A null
         * next-link in the middle of a chain must not allow the delete tail
         * to mutate a partially verified graph. */
        if (next == DM2_V1_RECORD_HANDLE_NULL) {
            return 0;
        }
        cursor = next;
    }
    if (cursor != DM2_V1_RECORD_HANDLE_END) {
        return 0;
    }
    return found;
}

static int dm2_v1_delete_creature_record_full_impl(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_current,
    unsigned long game_tick,
    int x, int y,
    int mode,
    int noise_arg,
    int party_x, int party_y, int party_dir,
    const uint16_t drop_slots[DM2_DROP_SLOT_COUNT],
    DM2_V1_DeleteCreatureFullReceipt *receipt,
    DM2_V1_DeleteAiFlagsContextFn ai_flags_context_fn,
    void *ai_flags_context,
    DM2_V1_DeleteGdatWord1ContextFn gdat_word1_context_fn,
    void *gdat_word1_context,
    DM2_V1_DeleteInvokeContextFn invoke_context_fn,
    void *invoke_context) {
  DM2_V1_DeleteCreatureFullReceipt local;
  DM2_V1_CaiiAiSpecFlagsFn ai_flags_fn;
  DM2_V1_CaiiWordValueFn gdat_w1_fn;
  uint8_t *record;
  int16_t handle;
  int16_t head;
  int16_t head_before;
  int first;
  DM2_V1_RecordPoolSet pool_backup;
  DM2_V1_RecordPoolSet snapshot_source;
  DM2_V1_SourceTimerQueue queue_backup;
  DM2_V1_DropRng rng_backup;
  uint8_t *dungeon_backup = NULL;
  uint8_t *caii_backup = NULL;
  size_t caii_bytes = 0u;
  int transaction_ready = 0;

  memset(&pool_backup, 0, sizeof(pool_backup));

  memset(&local, 0, sizeof(local));
  snprintf(local.source_evidence, sizeof(local.source_evidence), "%s",
           dm2_v1_delete_creature_full_evidence);

  if (receipt == NULL) {
    receipt = &local;
  } else {
    *receipt = local;
  }

  if (pool_set == NULL || !pool_set->valid || dungeon == NULL ||
      queue == NULL ||
      map_current < 0 || map_current >= dungeon->level_count ||
      x < 0 || y < 0 || x > 0xff || y > 0xff) {
    return 0;
  }

  /* c_record.cpp:1377-1380 — resolve the creature at the payload
   * coordinates on the current c_map, not an implicit map-0 surrogate. */
  handle = dm2_v1_get_creature_at(pool_set, dungeon, map_current, x, y);
  if (handle == DM2_V1_RECORD_HANDLE_NULL) {
    receipt->creature_not_found = 1;
    receipt->completed = 1;
    receipt->valid = 1;
    return 1;
  }
  receipt->record_handle = handle;

  record = dm2_v1_record_pool_address_mut(pool_set, handle);
  if (record == NULL) {
    return 0;
  }
  receipt->creature_type = (int)record[4]; /* c_record.cpp:1384 */

  /* Bounded membership pre-walk BEFORE any mutation: the invoke branch
   * below queues a timer, and a corrupt tile chain must fail the
   * composition closed without partial effects.  Observable effect
   * order unchanged — the source's cut only runs at c_record.cpp:1419
   * too. */
  if (!dm2_v1_delete_full_chained_on_tile(pool_set, dungeon, map_current,
                                          x, y,
                                          handle)) {
    receipt->cut_miss = 1;
    return 0;
  }

  /* The source operation has several owners after this admission point:
   * invoke timer, CAII mode, tile root, possession drops and RNG.  Snapshot
   * them together so a later unproven owner cannot leave a partial delete. */
  queue_backup = *queue;
  rng_backup = rng ? *rng : (DM2_V1_DropRng){0};
  caii_bytes = (caii && caii->valid && caii->slots && caii->capacity > 0)
      ? (size_t)caii->capacity * DM2_V1_CAII_SLOT_SIZE : 0u;
  snapshot_source = *pool_set;
  /* Standalone probes may intentionally carry an incomplete graph flag while
   * still owning valid pool bytes.  The snapshot needs byte storage, not a
   * permission to publish that graph as runtime-ready. */
  snapshot_source.record_graph_complete = 1;
  if (!dm2_v1_record_pool_set_clone(&pool_backup, &snapshot_source) ||
      dungeon->raw_size <= 0 || !dungeon->raw_data ||
      !(dungeon_backup = (uint8_t *)malloc((size_t)dungeon->raw_size)) ||
      (caii_bytes != 0u &&
       !(caii_backup = (uint8_t *)malloc(caii_bytes))))
    goto delete_rollback;
  memcpy(dungeon_backup, dungeon->raw_data, (size_t)dungeon->raw_size);
  if (caii_bytes != 0u) memcpy(caii_backup, caii->slots, caii_bytes);
  transaction_ready = 1;

  /* jz_test8(aidef byte@0, 1) gate, c_record.cpp:1385 — data-backed
   * through the caii module's wired provider; without provenance the
   * gate is treated as not taken (the standalone head's discipline). */
  ai_flags_fn = dm2_v1_caii_get_ai_spec_flags_fn();
  {
    uint16_t ai_flags = 0;
    if ((ai_flags_context_fn != NULL
             ? ai_flags_context_fn(ai_flags_context, (int)record[4], &ai_flags)
             : (ai_flags_fn != 0 ? ai_flags_fn((int)record[4], &ai_flags) : 0)) == 1) {
      receipt->ai_flags_known = 1;
      receipt->ai_bit0_clear = (ai_flags & 0x1u) == 0u ? 1 : 0;
    }
  }

  if (receipt->ai_bit0_clear == 1) {
    /* c_record.cpp:1387-1388 — the table1d607e[GDAT CREATURES word@1]
     * uc[0] & 0x4 probe.  Gate open but word@1 unknown/out-of-span
     * leaves the branch unprovable: fail-closed before any mutation. */
    gdat_w1_fn = dm2_v1_caii_get_gdat_word1_fn();
    {
      uint16_t w1 = 0;
      int uc0 = -1;

      if ((gdat_word1_context_fn != NULL
               ? gdat_word1_context_fn(gdat_word1_context,
                                       (int)record[4], &w1)
               : (gdat_w1_fn != 0 ? gdat_w1_fn((int)record[4], &w1) : 0)) == 1) {
        uc0 = dm2_v1_caii_table1d607e_uc0((int)w1);
      }
      if (uc0 < 0) {
        receipt->gdat_w1_unknown = 1;
        goto delete_rollback;
      }

      if ((uc0 & 0x4) == 0) {
        /* c_record.cpp:1389-1406 — the map swap and the bound
         * DM2_INVOKE_MESSAGE(x, y, 0, 0, gametick + 1).  The record's
         * word@0xc decode: map = (w >> 10) & 0x3f (RG1UW >>= 0xa),
         * the y word = w >> 5 (RG1L <<= 6, ushiftr11 — the source
         * passes the unmasked shifted word; setxyA keeps the low
         * byte), x = w & 0x1f.  The CHANGE_CURRENT_MAP_TO swap/restore is
         * receipted and the queued timer carries the decoded map byte exactly like the
         * source's post-swap ddat.v1d3248 read. */
        uint16_t w = dm2_v1_delete_full_read_u16le(record + 0xc);
        int msg_map = (int)((w >> 10) & 0x3fu);
        int msg_y = (int)(w >> 5);
        int msg_x = (int)(w & 0x1fu);
        DM2_V1_InvokeMessageReceipt inv;

        (void)map_current; /* vw_10 save/restore — receipted below */
        receipt->map_swap_receipted = 1;

        memset(&inv, 0, sizeof(inv));
        if (invoke_context_fn != NULL) {
          uint32_t ticket = 0u;
          if (invoke_context_fn(invoke_context, msg_map, msg_x, msg_y,
                                0, 0, (int32_t)(game_tick + 1ul),
                                &ticket) != 1 || ticket == 0u) {
            receipt->invoke_queue_rejected = 1;
            goto delete_rollback;
          }
          inv.ticket = ticket;
        } else if (dm2_v1_invoke_message(queue, msg_map, msg_x, msg_y, 0, 0,
                                         (int32_t)(game_tick + 1ul),
                                         &inv) != 1) {
          receipt->invoke_queue_rejected = 1;
          goto delete_rollback;
        }
        receipt->invoke_message_queued = 1;
        receipt->invoke_ticket = inv.ticket;
      }
    }

    /* c_record.cpp:1408-1413 — a creature still owning a CAII slot gets
     * its slot mode byte cleared. */
    if (record[5] != 0xffu) {
      if (caii != NULL && caii->valid &&
          (int)record[5] < caii->capacity) {
        caii->slots[(size_t)record[5] * DM2_V1_CAII_SLOT_SIZE + 0x1a] = 0;
        receipt->slot_mode_cleared = 1;
      } else {
        receipt->slot_mode_clear_unbound = 1;
      }
    }
  }

  /* c_record.cpp:1416-1419 — the tile-rooted cut (the DM2_MOVE_RECORD_TO
   * x == -4 skip00823/3CE7D path's observable end state,
   * c_moverec.cpp:630-683).  Membership was pre-walked above. */
  first = dm2_v1_dungeon_get_first_thing(dungeon, map_current, x, y);
  if (first < 0) {
    receipt->cut_miss = 1;
    goto delete_rollback;
  }
  head = (int16_t)first;
  head_before = head;
  if (!dm2_v1_record_pool_cut_from_list(pool_set, &head, handle)) {
    receipt->cut_miss = 1;
    goto delete_rollback;
  }
  receipt->cut_performed = 1;
  if (head != head_before) {
    if (dm2_v1_dungeon_set_first_thing(dungeon, map_current, x, y,
                                       (uint16_t)head) != 0) {
      goto delete_rollback;
    }
    receipt->cut_head_rewritten = 1;
  }
  receipt->cut_side_effects_unbound = 1;

  /* c_record.cpp:1422 — DM2_DROP_CREATURE_POSSESSION, bound through the
   * proven slice with the session's wired AI flags provider.  A
   * fail-closed drop skips the dealloc (bounded-slice stop
   * discipline). */
  int drop_result;
  if (ai_flags_context_fn != NULL) {
    drop_result = dm2_v1_drop_creature_possession_with_context(
        pool_set, dungeon, map_current, rng, ai_flags_context_fn,
        ai_flags_context, handle, x, y, mode, noise_arg, party_x, party_y,
        party_dir, drop_slots, NULL, 0, &receipt->drop);
  } else {
    drop_result = dm2_v1_drop_creature_possession(
        pool_set, dungeon, map_current, rng, ai_flags_fn, handle, x, y, mode,
        noise_arg, party_x, party_y, party_dir, drop_slots,
        NULL, 0, &receipt->drop);
  }
  if (drop_result != 1) {
    receipt->drop_failed = 1;
    goto delete_rollback;
  }
  receipt->drop_ran = 1;

  /* c_record.cpp:1423 — DM2_1c9a_0247 tagged-dballoc cleanup: the host's
   * preserved-GFX cache (c_1c9a.cpp:5135-5160), receipted, never
   * simulated. */
  receipt->dballoc_cleanup_unbound = 1;

  /* c_record.cpp:1424 via c_record.cpp:1205-1208 — DM2_DEALLOC_RECORD:
   * the record's first word becomes the 0xffff free marker. */
  record[0] = 0xffu;
  record[1] = 0xffu;
  receipt->dealloc_performed = 1;

  receipt->completed = 1;
  receipt->valid = 1;
  dm2_v1_record_pool_set_free(&pool_backup);
  free(dungeon_backup);
  free(caii_backup);
  return 1;

delete_rollback:
  if (transaction_ready) {
    *queue = queue_backup;
    if (rng) *rng = rng_backup;
    if (dungeon_backup && dungeon->raw_data)
      memcpy(dungeon->raw_data, dungeon_backup, (size_t)dungeon->raw_size);
    if (caii_bytes != 0u && caii && caii->slots)
      memcpy(caii->slots, caii_backup, caii_bytes);
    if (pool_backup.valid) {
      dm2_v1_record_pool_set_free(pool_set);
      *pool_set = pool_backup;
      memset(&pool_backup, 0, sizeof(pool_backup));
    }
    receipt->transaction_rolled_back = 1;
  }
  dm2_v1_record_pool_set_free(&pool_backup);
  free(dungeon_backup);
  free(caii_backup);
  return 0;
}

int dm2_v1_delete_creature_record_full(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_current,
    unsigned long game_tick,
    int x, int y,
    int mode,
    int noise_arg,
    int party_x, int party_y, int party_dir,
    const uint16_t drop_slots[DM2_DROP_SLOT_COUNT],
    DM2_V1_DeleteCreatureFullReceipt *receipt) {
  return dm2_v1_delete_creature_record_full_impl(
      pool_set, dungeon, caii, queue, rng, map_current, game_tick, x, y,
      mode, noise_arg, party_x, party_y, party_dir, drop_slots, receipt,
      NULL, NULL, NULL, NULL, NULL, NULL);
}

int dm2_v1_delete_creature_record_full_with_context(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_current,
    unsigned long game_tick,
    int x, int y,
    int mode,
    int noise_arg,
    int party_x, int party_y, int party_dir,
    const uint16_t drop_slots[DM2_DROP_SLOT_COUNT],
    DM2_V1_DeleteAiFlagsContextFn ai_flags_fn,
    void *ai_flags_context,
    DM2_V1_DeleteGdatWord1ContextFn gdat_word1_fn,
    void *gdat_word1_context,
    DM2_V1_DeleteCreatureFullReceipt *receipt) {
  if (!ai_flags_fn || !gdat_word1_fn) return 0;
  return dm2_v1_delete_creature_record_full_impl(
      pool_set, dungeon, caii, queue, rng, map_current, game_tick, x, y,
      mode, noise_arg, party_x, party_y, party_dir, drop_slots, receipt,
      ai_flags_fn, ai_flags_context, gdat_word1_fn, gdat_word1_context,
      NULL, NULL);
}

int dm2_v1_delete_creature_record_full_with_context_and_invoke(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_CaiiArray *caii,
    DM2_V1_SourceTimerQueue *queue,
    DM2_V1_DropRng *rng,
    int map_current,
    unsigned long game_tick,
    int x, int y,
    int mode,
    int noise_arg,
    int party_x, int party_y, int party_dir,
    const uint16_t drop_slots[DM2_DROP_SLOT_COUNT],
    DM2_V1_DeleteAiFlagsContextFn ai_flags_fn,
    void *ai_flags_context,
    DM2_V1_DeleteGdatWord1ContextFn gdat_word1_fn,
    void *gdat_word1_context,
    DM2_V1_DeleteInvokeContextFn invoke_fn,
    void *invoke_context,
    DM2_V1_DeleteCreatureFullReceipt *receipt) {
  if (!ai_flags_fn || !gdat_word1_fn || !invoke_fn) return 0;
  return dm2_v1_delete_creature_record_full_impl(
      pool_set, dungeon, caii, queue, rng, map_current, game_tick, x, y,
      mode, noise_arg, party_x, party_y, party_dir, drop_slots, receipt,
      ai_flags_fn, ai_flags_context, gdat_word1_fn, gdat_word1_context,
      invoke_fn, invoke_context);
}
