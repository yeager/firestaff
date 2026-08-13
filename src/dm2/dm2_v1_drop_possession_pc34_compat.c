/*
 * dm2_v1_drop_possession_pc34_compat.c — DM2_DROP_CREATURE_POSSESSION
 * (skproject/SKULLWIN/c_record.cpp:1537-1752) bounded slice.
 */

#include "dm2_v1_drop_possession_pc34_compat.h"
#include "dm2_v1_record_ops_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static long walk_budget_total(const DM2_V1_RecordPoolSet *set)
{
    long budget = 1;
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        budget += set->pools[i].record_count;
        budget += set->pools[i].extension_count;
    }
    return budget;
}

/* Bounded "the chain walks to its end" pre-check, so the (unbudgeted)
 * source append splice can never spin on a corrupt destination chain. */
static int chain_terminates(const DM2_V1_RecordPoolSet *set, int16_t head)
{
    long budget = walk_budget_total(set);
    int16_t cursor = head;

    if (cursor == DM2_V1_RECORD_HANDLE_NULL) return 0;

    while (cursor != DM2_V1_RECORD_HANDLE_END &&
           cursor != DM2_V1_RECORD_HANDLE_NULL) {
        int16_t next;

        if (budget-- <= 0) {
            return 0;
        }
        if (!dm2_v1_record_pool_next_link(set, cursor, &next)) {
            return 0;
        }
        if (next == DM2_V1_RECORD_HANDLE_NULL) return 0;
        cursor = next;
    }
    return 1;
}

/* The direction draw shared by the generated-drops loop and the
 * possession walk (c_record.cpp:1622-1637 / 1695-1710): on the party
 * cell (party_dir + RANDBIT) & 3, elsewhere RANDDIR. */
static int draw_direction(DM2_V1_DropRng *rng,
                          int x, int y,
                          int party_x, int party_y, int party_dir)
{
    if (x == party_x && y == party_y) {
        return (int)((dm2_v1_drops_randbit(rng) +
                      (uint16_t)party_dir) & 0x3u);
    }
    return (int)(dm2_v1_drops_randdir(rng) & 0x3u);
}

static int dm2_v1_drop_creature_possession_impl(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    int map,
    DM2_V1_DropRng *rng,
    DM2_V1_DropPossessionAiFlagsFn ai_flags_fn,
    DM2_V1_DropPossessionAiFlagsContextFn ai_flags_context_fn,
    void *ai_flags_context,
    int16_t creature_record,
    int x, int y,
    int mode,
    int noise_arg,
    int party_x, int party_y, int party_dir,
    const uint16_t drop_slots[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropPlacedItem *out_items,
    int max_items,
    DM2_V1_DropPossessionReceipt *receipt)
{
    DM2_V1_DropPossessionReceipt local;
    uint8_t *record;
    int16_t head;
    int first;
    int16_t cursor;
    long budget;
    uint16_t ai_flags = 0;
    DM2_V1_DropPlacedItem generated_items[DM2_DROP_SLOT_COUNT];

    memset(&local, 0, sizeof(local));
    memset(generated_items, 0, sizeof(generated_items));
    snprintf(local.source_evidence, sizeof(local.source_evidence),
             "skproject c_record.cpp:1537-1752 "
             "DM2_DROP_CREATURE_POSSESSION (mode gates 1562-1567, "
             "generated drops 1568-1634 via the proven slots binding, "
             "possession walk 1682-1751 with tile-rooted append; "
             "DM2_QUEUE_NOISE_GEN2 receipted host-owned)");
    if (receipt == NULL) {
        receipt = &local;
    } else {
        *receipt = local;
    }

    if (pool_set == NULL || !pool_set->valid || dungeon == NULL ||
        map < 0 || map >= dungeon->level_count ||
        x < 0 || y < 0 || x > 0xff || y > 0xff) {
        return 0;
    }
    record = dm2_v1_record_pool_address_mut(pool_set, creature_record);
    if (record == NULL) {
        return 0;
    }
    receipt->mode = (int)(uint16_t)mode;

    /* c_record.cpp:1562-1563: mode 2 returns immediately. */
    if ((uint16_t)mode == 2u) {
        receipt->mode_return = 1;
        receipt->valid = 1;
        return 1;
    }

    /* The drop cell must carry a ground-stack slot; growing the map
     * tables for a flag-less cell stays unproven (fail-closed). */
    first = dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);
    if (first < 0) {
        receipt->drops_cell_unrooted = 1;
        return 0;
    }
    head = (int16_t)first;
    cursor = (int16_t)rd16(record + 2);
    /* Validate the possession graph before generated drops can append or
     * consume RNG.  The source delete transaction owns both walks; a
     * malformed possession tail must leave the drop cell untouched. */
    if (cursor != DM2_V1_RECORD_HANDLE_END &&
        !chain_terminates(pool_set, cursor)) {
        receipt->walk_corrupt = 1;
        return 0;
    }

    /* c_record.cpp:1568-1634: the generated-drops loop (mode == 0),
     * delegated to the proven slots binding with the destination head
     * tile-rooted in the dungeon ground-stack table. */
    if ((uint16_t)mode == 0u) {
        if (drop_slots == NULL) {
            receipt->drop_slots_unloaded = 1;
        } else {
            int16_t gen_head = head;

            if (rng == NULL) {
                receipt->rng_unbound = 1;
                return 0;
            }
            if (!chain_terminates(pool_set, gen_head)) {
                receipt->tile_chain_corrupt = 1;
                return 0;
            }
            receipt->drops_placed = dm2_v1_drops_place_source_slots(
                pool_set, dungeon, map, drop_slots, rng,
                party_x, party_y, party_dir,
                x, y, &gen_head, generated_items,
                DM2_DROP_SLOT_COUNT,
                &receipt->drops_iterations);
            for (int i = 0; i < receipt->drops_iterations &&
                         i < DM2_DROP_SLOT_COUNT; ++i) {
                if (i == 0) {
                    receipt->generated_drop_first_itemspec =
                        generated_items[i].itemspec;
                    receipt->generated_drop_first_db =
                        generated_items[i].item_db >= 0
                            ? generated_items[i].item_db
                            : dm2_v1_get_itemdb_of_itemspec_actuator(
                                  generated_items[i].itemspec);
                }
                if (generated_items[i].alloc_failed)
                    ++receipt->generated_drop_alloc_failures;
                if (i == 0)
                    receipt->generated_drop_alloc_free_records =
                        generated_items[i].alloc_free_records;
                if (out_items && i < max_items)
                    out_items[i] = generated_items[i];
            }
            if (gen_head != head) {
                if (dm2_v1_dungeon_set_first_thing(
                        dungeon, map, x, y, (uint16_t)gen_head) != 0) {
                    return 0;
                }
                head = gen_head;
            }
            receipt->generated_drops_ran = 1;
        }
    }

    /* c_record.cpp:1682-1751: the possession chain walk. */
    if (cursor == DM2_V1_RECORD_HANDLE_END) {
        receipt->valid = 1;
        return 1;
    }

    /* The direction-randomization gate reads the creature's AI flags
     * (c_record.cpp:1689-1690); unresolved flags stop the walk BEFORE
     * its first RNG draw (fail-closed, no stream divergence). */
    if ((ai_flags_context_fn == NULL && ai_flags_fn == NULL) ||
        (ai_flags_context_fn != NULL
             ? !ai_flags_context_fn(ai_flags_context, (int)record[4],
                                    &ai_flags)
             : !ai_flags_fn((int)record[4], &ai_flags))) {
        receipt->ai_flags_unknown = 1;
        return 0;
    }
    receipt->ai_flags_known = 1;

    if (rng == NULL) {
        receipt->rng_unbound = 1;
        return 0;
    }
    if (!chain_terminates(pool_set, head)) {
        receipt->tile_chain_corrupt = 1;
        return 0;
    }

    receipt->possession_walk_ran = 1;
    budget = walk_budget_total(pool_set);
    while (cursor != DM2_V1_RECORD_HANDLE_END) {
        int16_t next;
        int16_t handle = cursor;
        int db;

        if (cursor == DM2_V1_RECORD_HANDLE_NULL || budget-- <= 0) {
            receipt->walk_corrupt = 1;
            return 0;
        }
        /* c_record.cpp:1685: the next link is prefetched BEFORE the
         * move rewrites the record's own link. */
        if (!dm2_v1_record_pool_next_link(pool_set, cursor, &next)) {
            receipt->walk_corrupt = 1;
            return 0;
        }

        /* c_record.cpp:1689-1710: AI flags bit0 clear randomizes the
         * item's direction bits, folded into the handle. */
        if ((ai_flags & 0x1u) == 0u) {
            int dir = draw_direction(rng, x, y,
                                     party_x, party_y, party_dir);
            handle = (int16_t)((cursor & 0x3fff) | (dir << 14));
            ++receipt->dir_draws;
        }

        db = dm2_v1_record_handle_pool(cursor);
        if (db != 0x0e) {
            /* c_record.cpp:1715-1721: MOVE_RECORD_TO from-nowhere onto
             * the drop cell — the tile-rooted append end state. */
            int16_t new_head = head;

            if (!dm2_v1_record_pool_append_to_list(pool_set, &new_head,
                                                   handle)) {
                receipt->walk_corrupt = 1;
                return 0;
            }
            if (new_head != head) {
                if (dm2_v1_dungeon_set_first_thing(
                        dungeon, map, x, y, (uint16_t)new_head) != 0) {
                    return 0;
                }
                head = new_head;
            }
            ++receipt->possession_dropped;
            if (noise_arg >= 0) {
                /* c_record.cpp:1727-1738: DM2_QUEUE_NOISE_GEN2 stays
                 * host-owned — receipted, never simulated. */
                ++receipt->noise_would_queue;
            }
        } else {
            /* c_record.cpp:1742-1744: DB 0x0e records are deallocated
             * (c_record.cpp:1205-1208). */
            uint8_t *item =
                dm2_v1_record_pool_address_mut(pool_set, cursor);
            if (item == NULL) {
                receipt->walk_corrupt = 1;
                return 0;
            }
            item[0] = 0xffu;
            item[1] = 0xffu;
            ++receipt->possession_dealloced;
        }
        ++receipt->possession_items;
        cursor = next;
    }

    receipt->valid = 1;
    return 1;
}

int dm2_v1_drop_creature_possession(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    int map,
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
    DM2_V1_DropPossessionReceipt *receipt)
{
    return dm2_v1_drop_creature_possession_impl(
        pool_set, dungeon, map, rng, ai_flags_fn, NULL, NULL,
        creature_record, x, y, mode, noise_arg, party_x, party_y, party_dir,
        drop_slots, out_items, max_items, receipt);
}

int dm2_v1_drop_creature_possession_with_context(
    DM2_V1_RecordPoolSet *pool_set,
    DM2_V1_DungeonData *dungeon,
    int map,
    DM2_V1_DropRng *rng,
    DM2_V1_DropPossessionAiFlagsContextFn ai_flags_fn,
    void *ai_flags_context,
    int16_t creature_record,
    int x, int y,
    int mode,
    int noise_arg,
    int party_x, int party_y, int party_dir,
    const uint16_t drop_slots[DM2_DROP_SLOT_COUNT],
    DM2_V1_DropPlacedItem *out_items,
    int max_items,
    DM2_V1_DropPossessionReceipt *receipt)
{
    if (!ai_flags_fn) return 0;
    return dm2_v1_drop_creature_possession_impl(
        pool_set, dungeon, map, rng, NULL, ai_flags_fn, ai_flags_context,
        creature_record, x, y, mode, noise_arg, party_x, party_y, party_dir,
        drop_slots, out_items, max_items, receipt);
}

const char *dm2_v1_drop_possession_source_evidence(void)
{
    return "skproject c_record.cpp:1537-1752 DM2_DROP_CREATURE_POSSESSION";
}
