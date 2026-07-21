#ifndef FIRESTAFF_DM2_V1_DELETE_CREATURE_FULL_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_DELETE_CREATURE_FULL_PC34_COMPAT_H

/*
 * dm2_v1_delete_creature_full_pc34_compat.h — the COMPLETE
 * DM2_DELETE_CREATURE_RECORD composition
 * (skproject/SKULLWIN/c_record.cpp:1357-1425) over the DM2-002 record
 * pool, binding the decision head (previously bound standalone as
 * dm2_v1_caii_delete_creature_record_head), the map-swap/DM2_INVOKE_MESSAGE
 * branch, the tile-rooted cut (previously dm2_v1_caii_delete_creature_record_tail),
 * and the bound DM2_DROP_CREATURE_POSSESSION in one source-ordered
 * flow.
 *
 * Source order (c_record.cpp:1377-1424):
 *
 *   1377-1380  DM2_GET_CREATURE_AT(x, y); -1 takes the source early
 *              return (receipted creature_not_found, completed);
 *   1384-1385  creature type (byte@4) -> AIDefinition via the wired
 *              AI-spec flags provider (read through the caii module's
 *              getter to keep the link boundary); jz_test8 gate
 *              (aidef byte@0 & 1 == 0) data-backed, unknown flags treat
 *              the gate as not taken (the head's discipline);
 *   1387-1388  inside the gate: the table1d607e[GDAT CREATURES word@1]
 *              .uc[0] & 0x4 probe — data-backed through the wired word@1
 *              provider and the verbatim mdata table (via the caii
 *              module's accessors).  When the gate is open but word@1 is
 *              unknown or out of the proven span 0x2f the branch is
 *              unprovable: fail-closed BEFORE any mutation
 *              (gdat_w1_unknown);
 *   1389-1406  probe == 0: vw_10 = ddat.v1d3248 saved, the record's
 *              word@0xc decode — map = (w >> 10) & 0x3f, the y word
 *              w >> 5 (the source passes the unmasked shifted word;
 *              setxyA keeps the low byte), x = w & 0x1f — then
 *              DM2_CHANGE_CURRENT_MAP_TO(map),
 *              DM2_INVOKE_MESSAGE(x, y, 0, 0, gametick + 1) BOUND
 *              (dm2_v1_invoke_message), DM2_CHANGE_CURRENT_MAP_TO(vw_10).
 *              The session is single-map: the swap is receipted
 *              (map_swap_receipted) and the queued timer carries the
 *              decoded map byte exactly like the source's post-swap
 *              ddat.v1d3248 read;
 *   1408-1413  record byte@5 != 0xff clears the owning CAII slot's
 *              byte@1a (bound when `caii` is non-NULL; receipted
 *              slot_mode_clear_unbound otherwise);
 *   1416-1419  the tile-rooted cut (DM2_MOVE_RECORD_TO x == -4 path's
 *              observable end state, c_moverec.cpp:630-683): bounded
 *              membership pre-walk + DM2_CUT_RECORD_FROM list splice +
 *              ground-stack head rewrite through
 *              dm2_v1_dungeon_set_first_thing.  The membership pre-walk
 *              runs BEFORE the invoke branch so a corrupt chain fails
 *              closed (cut_miss) before ANY mutation — the observable
 *              effect order is unchanged.  The 3CE7D timer/text side
 *              effects and the recursive DM2_1c9a_0fcb slot free inside
 *              the cut stay host-owned (receipted
 *              cut_side_effects_unbound);
 *   1422       DM2_DROP_CREATURE_POSSESSION BOUND through the
 *              dm2_v1_drop_creature_possession slice with the wired
 *              AI flags provider, the caller's drop slots (nullable),
 *              mode (vw_04) and noise arg (parw02 = vql_00).  A
 *              fail-closed drop (unrooted cell, unbound RNG, unknown AI
 *              flags) receipts drop_failed and returns 0 — the dealloc
 *              is then skipped, mirroring the bounded slices' stop
 *              discipline;
 *   1423       DM2_1c9a_0247 tagged-dballoc cleanup stays host-owned
 *              (the host's preserved-GFX cache, c_1c9a.cpp:5135-5160) —
 *              receipted, never simulated;
 *   1424       DM2_DEALLOC_RECORD BOUND (c_record.cpp:1205-1208): the
 *              record's first word becomes the 0xffff free marker.
 *
 * The standalone head/tail slices remain for their granular callers;
 * this composition supersedes neither.
 */

#include <stdint.h>

#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_drop_possession_pc34_compat.h"
#include "dm2_v1_invoke_message_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  int valid;
  int completed;             /* ran to a source return: the early return
                                (creature_not_found) or the dealloc */
  int creature_not_found;    /* GET_CREATURE_AT early return
                                (c_record.cpp:1379-1380) */
  int16_t record_handle;     /* resolved DB4 handle (direction bits kept) */
  int creature_type;         /* record byte@4 */
  int ai_flags_known;        /* provider returned the AI flags word */
  int ai_bit0_clear;         /* jz_test8 gate passed (c_record.cpp:1385) */
  int gdat_w1_unknown;       /* fail-closed: gate open but GDAT word@1
                                unknown/out-of-span — the invoke branch is
                                then unprovable */
  int invoke_message_queued; /* BOUND: the map-swap/DM2_INVOKE_MESSAGE
                                branch ran (c_record.cpp:1389-1406) */
  int map_swap_receipted;    /* CHANGE_CURRENT_MAP_TO swap + restore
                                receipted (single-map session) */
  uint32_t invoke_ticket;    /* the queued type-0x4 timer's ticket */
  int invoke_queue_rejected; /* fail-closed: DM2_QUEUE_TIMER rejected */
  int slot_mode_cleared;     /* record byte@5 != 0xff -> slot byte@1a = 0
                                (c_record.cpp:1408-1413) */
  int slot_mode_clear_unbound;/* byte@5 != 0xff but no CAII array wired */
  int cut_performed;         /* BOUND: the tile-rooted unlink
                                (c_record.cpp:1419) */
  int cut_miss;              /* fail-closed: record not chained on the
                                tile (before any mutation) */
  int cut_head_rewritten;    /* ground-stack head rewritten through
                                dm2_v1_dungeon_set_first_thing */
  int cut_side_effects_unbound;/* 3CE7D timer/text side effects and the
                                recursive DM2_1c9a_0fcb inside the cut
                                stay host-owned */
  int drop_ran;              /* BOUND DM2_DROP_CREATURE_POSSESSION ran to
                                a source return (c_record.cpp:1422) */
  int drop_failed;           /* fail-closed drop: the dealloc is skipped */
  int dballoc_cleanup_unbound;/* DM2_1c9a_0247 host-owned preserved-GFX
                                cache cleanup (c_record.cpp:1423) */
  int dealloc_performed;     /* BOUND: DM2_DEALLOC_RECORD free marker
                                (c_record.cpp:1424 via 1205-1208) */
  DM2_V1_DropPossessionReceipt drop; /* the drop slice's own audit */
  char source_evidence[224];
} DM2_V1_DeleteCreatureFullReceipt;

/*
 * The complete DM2_DELETE_CREATURE_RECORD composition.  `map_current`
 * stands in for ddat.v1d3248 and `game_tick` for timdat.gametick (both
 * caller-owned); `x`/`y` are the payload coordinates (vql_0c/vql_08),
 * `mode` is vw_04 and `noise_arg` is parw02 (vql_00) exactly like the
 * source's DROP_CREATURE_POSSESSION arguments; `party_x`/`party_y`/
 * `party_dir` stand in for ddat.v1e0270/v1e0272/v1e0258; `drop_slots`
 * carries the creature's GDAT CREATURES drop fields 0x0A..0x14 or NULL.
 * `queue` receives the invoke-message timer and must be non-NULL;
 * `caii` may be NULL (the slot-mode clear is then receipted unbound).
 *
 * Returns 1 when the composition ran to a source return
 * (receipt.valid == 1 — including the creature_not_found early
 * return); 0 (fail-closed, receipted) otherwise.  When `receipt` is
 * non-NULL it always receives the audit record.
 */
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
    DM2_V1_DeleteCreatureFullReceipt *receipt);

const char *dm2_v1_delete_creature_full_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DELETE_CREATURE_FULL_PC34_COMPAT_H */
