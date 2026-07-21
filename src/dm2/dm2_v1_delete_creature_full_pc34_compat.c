/*
 * dm2_v1_delete_creature_full_pc34_compat.c — the COMPLETE
 * DM2_DELETE_CREATURE_RECORD composition
 * (skproject/SKULLWIN/c_record.cpp:1357-1425).  See the header for the
 * full source-lock documentation.
 */

#include "dm2_v1_delete_creature_full_pc34_compat.h"

#include <stdio.h>
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
      return 0;
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
  DM2_V1_DeleteCreatureFullReceipt local;
  DM2_V1_CaiiAiSpecFlagsFn ai_flags_fn;
  DM2_V1_CaiiWordValueFn gdat_w1_fn;
  uint8_t *record;
  int16_t handle;
  int16_t head;
  int16_t head_before;
  int first;

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
      map_current < 0 || map_current > 0xff ||
      x < 0 || y < 0 || x > 0xff || y > 0xff) {
    return 0;
  }

  /* c_record.cpp:1377-1380 — resolve the creature at the payload
   * coordinates; the session is single-map (map index 0). */
  handle = dm2_v1_get_creature_at(pool_set, dungeon, 0, x, y);
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
  if (!dm2_v1_delete_full_chained_on_tile(pool_set, dungeon, x, y,
                                          handle)) {
    receipt->cut_miss = 1;
    return 0;
  }

  /* jz_test8(aidef byte@0, 1) gate, c_record.cpp:1385 — data-backed
   * through the caii module's wired provider; without provenance the
   * gate is treated as not taken (the standalone head's discipline). */
  ai_flags_fn = dm2_v1_caii_get_ai_spec_flags_fn();
  {
    uint16_t ai_flags = 0;
    if (ai_flags_fn != 0 &&
        ai_flags_fn((int)record[4], &ai_flags) == 1) {
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

      if (gdat_w1_fn != 0 &&
          gdat_w1_fn((int)record[4], &w1) == 1) {
        uc0 = dm2_v1_caii_table1d607e_uc0((int)w1);
      }
      if (uc0 < 0) {
        receipt->gdat_w1_unknown = 1;
        return 0;
      }

      if ((uc0 & 0x4) == 0) {
        /* c_record.cpp:1389-1406 — the map swap and the bound
         * DM2_INVOKE_MESSAGE(x, y, 0, 0, gametick + 1).  The record's
         * word@0xc decode: map = (w >> 10) & 0x3f (RG1UW >>= 0xa),
         * the y word = w >> 5 (RG1L <<= 6, ushiftr11 — the source
         * passes the unmasked shifted word; setxyA keeps the low
         * byte), x = w & 0x1f.  The session is single-map: the
         * CHANGE_CURRENT_MAP_TO swap/restore is receipted and the
         * queued timer carries the decoded map byte exactly like the
         * source's post-swap ddat.v1d3248 read. */
        uint16_t w = dm2_v1_delete_full_read_u16le(record + 0xc);
        int msg_map = (int)((w >> 10) & 0x3fu);
        int msg_y = (int)(w >> 5);
        int msg_x = (int)(w & 0x1fu);
        DM2_V1_InvokeMessageReceipt inv;

        (void)map_current; /* vw_10 save/restore — receipted below */
        receipt->map_swap_receipted = 1;

        memset(&inv, 0, sizeof(inv));
        if (dm2_v1_invoke_message(queue, msg_map, msg_x, msg_y, 0, 0,
                                  (int32_t)(game_tick + 1ul),
                                  &inv) != 1) {
          receipt->invoke_queue_rejected = 1;
          return 0;
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
  first = dm2_v1_dungeon_get_first_thing(dungeon, 0, x, y);
  if (first < 0) {
    receipt->cut_miss = 1;
    return 0;
  }
  head = (int16_t)first;
  head_before = head;
  if (!dm2_v1_record_pool_cut_from_list(pool_set, &head, handle)) {
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

  /* c_record.cpp:1422 — DM2_DROP_CREATURE_POSSESSION, bound through the
   * proven slice with the session's wired AI flags provider.  A
   * fail-closed drop skips the dealloc (bounded-slice stop
   * discipline). */
  if (dm2_v1_drop_creature_possession(
          pool_set, dungeon, rng, ai_flags_fn, handle, x, y, mode,
          noise_arg, party_x, party_y, party_dir, drop_slots,
          NULL, 0, &receipt->drop) != 1) {
    receipt->drop_failed = 1;
    return 0;
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
  return 1;
}
