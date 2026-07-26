#ifndef FIRESTAFF_DM2_V1_DROP_POSSESSION_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_DROP_POSSESSION_PC34_COMPAT_H

/*
 * dm2_v1_drop_possession_pc34_compat.h — DM2_DROP_CREATURE_POSSESSION
 * (skproject/SKULLWIN/c_record.cpp:1537-1752) as a bounded slice over
 * the DM2-002 record pool.
 *
 * Bound in source order:
 *
 *   c_record.cpp:1562-1563  mode (ecxl low word) == 2: the source
 *                           returns immediately (receipted mode_return);
 *   c_record.cpp:1568-1634  mode == 0: the generated-drops loop over the
 *                           creature's GDAT CREATURES drop fields
 *                           0x0A..0x14 — delegated to the proven
 *                           dm2_v1_drops_place_source_slots binding
 *                           (slot base/extra counts, RAND16 extra roll,
 *                           per-item ALLOC_NEW_DBITEM, the party-cell
 *                           RANDBIT / elsewhere RANDDIR direction draw),
 *                           with the destination head now TILE-ROOTED in
 *                           the dungeon ground-stack table (round-17
 *                           setter) instead of caller-owned;
 *   c_record.cpp:1682-1751  the possession chain walk from the
 *                           creature's word@2 link: each record's next
 *                           link is prefetched BEFORE the move; when the
 *                           creature's AI flags bit0 is clear
 *                           (DM2_QUERY_CREATURE_AI_SPEC_FLAGS, wired
 *                           provider) the item's direction bits are
 *                           randomized with the same party-cell RANDBIT /
 *                           elsewhere RANDDIR draw folded into the handle
 *                           ((dir << 14) | handle & 0x3fff); DB index
 *                           != 0x0e items land on the drop cell through
 *                           the tile-rooted append (the MOVE_RECORD_TO
 *                           from-nowhere path's observable end state);
 *                           DB 0x0e records are deallocated
 *                           (DM2_DEALLOC_RECORD, c_record.cpp:1205-1208).
 *
 * Fail-closed contract: the drop cell must carry a ground-stack slot
 * (the 0x10 object flag — setting it on a flag-less cell would grow the
 * map tables, which stays unproven), every chain walk is budgeted
 * against the declared record count, unresolved AI flags stop the walk
 * BEFORE its first RNG draw, and a missing LCG stops the generated
 * drops before any mutation.  DM2_QUEUE_NOISE_GEN2
 * (c_record.cpp:1652-1663, 1727-1738) stays host-owned — receipted,
 * never simulated.
 */

#include <stdint.h>

#include "dm2_v1_dbitem_alloc_pc34_compat.h"
#include "dm2_v1_drops.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int valid;
    int mode;                 /* ecxl low word */
    int mode_return;          /* mode == 2: source immediate return */
    int drop_slots_unloaded;  /* GDAT drop fields not provided */
    int generated_drops_ran;  /* the mode == 0 generated-drops loop ran */
    int drops_cell_unrooted;  /* fail-closed: drop cell has no
                                 ground-stack slot */
    int drops_placed;         /* items placed by the generated loop */
    int drops_iterations;     /* per-item iterations executed */
    int possession_walk_ran;  /* the word@2 chain walk ran */
    int possession_items;     /* chain records visited */
    int possession_dropped;   /* DB != 0x0e: appended to the tile */
    int possession_dealloced; /* DB == 0x0e: DM2_DEALLOC_RECORD */
    int dir_draws;            /* direction randomizations performed */
    int noise_would_queue;    /* DM2_QUEUE_NOISE_GEN2 receipted
                                 (host-owned, never simulated) */
    int ai_flags_known;       /* creature AI flags resolved */
    int ai_flags_unknown;     /* fail-closed before the walk's first
                                 RNG draw */
    int tile_chain_corrupt;   /* fail-closed: destination chain not
                                 walkable to its end */
    int walk_corrupt;         /* fail-closed: bounded possession chain */
    int rng_unbound;          /* fail-closed: a source draw had no LCG */
    char source_evidence[512];
} DM2_V1_DropPossessionReceipt;

/* AI-spec flags provider for the walk's direction-randomization gate
 * (c_record.cpp:1689-1690).  Wire the proven provider
 * dm2_v1_creature_ai_spec_flags (dm2_v1_creature.h). */
typedef int (*DM2_V1_DropPossessionAiFlagsFn)(int creature_type,
                                              uint16_t *out_flags);

/* DM2_DROP_CREATURE_POSSESSION bounded slice.  `dungeon` is mutable
 * because the ground-stack head writes land in the dungeon data exactly
 * like the source's map state.  `drop_slots` carries the creature's
 * GDAT CREATURES drop fields 0x0A..0x14 (11 words) or NULL when the
 * session has not loaded them (the generated-drops part is then
 * skipped, receipted).  `party_x`/`party_y`/`party_dir` stand in for
 * ddat.v1e0270/v1e0272/v1e0258; `noise_arg` is the source's argw0
 * (< 0 suppresses the noise queue).  `out_items`/`max_items` receipt
 * the generated-drops iterations exactly like
 * dm2_v1_drops_place_source_slots.  Returns 1 when the slice ran to a
 * source return (receipt.valid == 1); 0 (fail-closed, receipted)
 * otherwise. */
int dm2_v1_drop_creature_possession(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    DM2_V1_DropRng *rng,
    DM2_V1_DropPossessionAiFlagsFn ai_flags_fn,
    int16_t creature_record,
    int x, int y,
    int mode,
    int noise_arg,
    int party_x, int party_y, int party_dir,
    const uint16_t drop_slots[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropPlacedItem *out_items,
    int max_items,
    DM2_V1_DropPossessionReceipt *receipt);

const char *dm2_v1_drop_possession_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_DROP_POSSESSION_PC34_COMPAT_H */
