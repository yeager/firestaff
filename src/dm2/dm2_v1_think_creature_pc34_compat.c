/*
 * dm2_v1_think_creature_pc34_compat.c — per-cell DM2_THINK_CREATURE binding
 * over the DM2-002 record pool.
 *
 * Source: skproject/SKULLWIN/c_tim_proc.cpp:4079-4088 (0x21/0x22 dispatch)
 *         skproject/SKULLWIN/c_ai.cpp:5649-5677      (DM2_THINK_CREATURE
 *                                                     prologue)
 *         skproject/SKULLWIN/c_querydb.cpp:1486-1507 (DM2_GET_CREATURE_AT)
 *         skproject/SKULLWIN/c_map.cpp:44-69         (GET_OBJECT_INDEX_FROM_
 *                                                     TILE / GET_TILE_RECORD_
 *                                                     LINK)
 *         skproject/SKULLWIN/c_timer.h:78-80         (getxA/getyA bytes)
 */

#include "dm2_v1_think_creature_pc34_compat.h"

#include <string.h>

/* skproject dbCreature pool index (c_record.cpp GET_ITEMDB... case 3 branch
 * and every DB4 dereference in c_ai.cpp/c_creature.cpp). */
#define DM2_V1_DB_CREATURE 4

/* DB4 Creature record byte@4 carries the creature type
 * (c_ai.cpp:5849-5858: setactor(byte_at(record, 4)) and the
 * DM2_QUERY_CREATURE_AI_SPEC_FROM_RECORD call in
 * DM2_PREPARE_LOCAL_CREATURE_VAR). */
#define DM2_V1_CREATURE_TYPE_OFFSET 4

int16_t dm2_v1_get_creature_at(const DM2_V1_RecordPoolSet *set,
                               const DM2_V1_DungeonData *dungeon,
                               int map, int x, int y)
{
    int16_t cursor;
    long walk_budget = 1;

    if (set == NULL || !set->valid || dungeon == NULL) {
        return DM2_V1_RECORD_HANDLE_NULL;
    }

    /* DM2_GET_TILE_RECORD_LINK (c_map.cpp:61-69) via the proven Firestaff
     * equivalent: byte-square bit-0x10 object flag + column-index
     * ground-stack table (or the word-square inline encoding).  The loader
     * returns -1 when the cell carries no objects, matching the source's
     * OBJECT_END_MARKER head. */
    cursor = (int16_t)dm2_v1_dungeon_get_first_thing(dungeon, map, x, y);

    /* Bound the walk so a corrupt chain can never spin: no source chain is
     * longer than the total declared record count. */
    for (int i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        walk_budget += set->pools[i].record_count;
    }

    /* c_querydb.cpp:1493-1506 — walk next-links until OBJECT_END_MARKER;
     * the first record whose DB index (bits 10-13, direction bits ignored)
     * is dbCreature wins. */
    for (;;) {
        int16_t next;

        if (cursor == DM2_V1_RECORD_HANDLE_END ||
            cursor == DM2_V1_RECORD_HANDLE_NULL) {
            return DM2_V1_RECORD_HANDLE_NULL;
        }
        if (walk_budget-- <= 0) {
            return DM2_V1_RECORD_HANDLE_NULL; /* corrupt chain: fail-closed */
        }
        if (dm2_v1_record_pool_address(set, cursor) == NULL) {
            return DM2_V1_RECORD_HANDLE_NULL;
        }
        if (dm2_v1_record_handle_pool(cursor) == DM2_V1_DB_CREATURE) {
            return cursor;
        }
        if (!dm2_v1_record_pool_next_link(set, cursor, &next)) {
            return DM2_V1_RECORD_HANDLE_NULL;
        }
        cursor = next;
    }
}

void dm2_v1_think_creature_binding_init(
    DM2_V1_ThinkCreatureBinding *binding,
    const DM2_V1_RecordPoolSet *pool_set,
    const DM2_V1_DungeonData *dungeon)
{
    if (binding == NULL) {
        return;
    }
    memset(binding, 0, sizeof(*binding));
    binding->pool_set = pool_set;
    binding->dungeon = dungeon;
    binding->receipt.valid = 1;
    binding->receipt.last_record = DM2_V1_RECORD_HANDLE_NULL;
    binding->receipt.last_creature_type = -1;
}

int dm2_v1_think_creature_timer_handler(
    void *context,
    const DM2_V1_SourceTimer *timer,
    uint16_t source_index,
    DM2_V1_ProceedTimersReceipt *receipt)
{
    DM2_V1_ThinkCreatureBinding *binding =
        (DM2_V1_ThinkCreatureBinding *)context;
    DM2_V1_ThinkCreatureReceipt *think_receipt;
    int map;
    int x;
    int y;
    int16_t creature;
    const uint8_t *record_addr;

    (void)source_index;
    (void)receipt;

    if (binding == NULL || timer == NULL ||
        binding->pool_set == NULL || binding->dungeon == NULL) {
        return 0; /* incomplete binding: fail-closed */
    }
    if (timer->type != DM2_V1_TIMER_THINK_CREATURE_A &&
        timer->type != DM2_V1_TIMER_THINK_CREATURE_B) {
        return 0;
    }

    think_receipt = &binding->receipt;

    /* c_tim_proc.cpp:4079-4088 — the dispatch unpacks the c_tim payload:
     * x = getxA(), y = getyA() (c_timer.h:78-80), the think type is the
     * timer type word, and the map switch already happened (c_timer.h:64
     * getmap = high byte of l_00). */
    map = (int)((timer->ticks_and_map >> 24) & 0xffu);
    x = (int)(int8_t)(timer->value_a & 0xff);
    y = (int)(int8_t)((timer->value_a >> 8) & 0xff);

    think_receipt->think_timers++;
    think_receipt->last_map = map;
    think_receipt->last_x = x;
    think_receipt->last_y = y;
    think_receipt->last_type = timer->type;

    /* c_ai.cpp:5670-5673 — DM2_GET_CREATURE_AT(x, y); no DB4 record on the
     * cell is the source's immediate return (timer consumed, no think). */
    creature = dm2_v1_get_creature_at(binding->pool_set, binding->dungeon,
                                      map, x, y);
    think_receipt->last_record = creature;
    if (creature == DM2_V1_RECORD_HANDLE_NULL) {
        think_receipt->no_creature_at_cell++;
        return 1;
    }

    think_receipt->resolved++;
    record_addr = dm2_v1_record_pool_address(binding->pool_set, creature);
    think_receipt->last_creature_type =
        (record_addr != NULL)
            ? (int)record_addr[DM2_V1_CREATURE_TYPE_OFFSET]
            : -1;

    /* The think body (DM2_PREPARE_LOCAL_CREATURE_VAR + the c_ai.cpp body)
     * stays fail-closed until the CCM stream owner/grammar is proven
     * (DM2-005).  With no DM2-owned body bound, nothing is simulated. */
    if (binding->think_body == NULL) {
        think_receipt->body_unbound++;
        return 1;
    }
    if (binding->think_body(binding->think_body_context, creature, timer,
                            map, x, y, timer->type)) {
        think_receipt->body_consumed++;
    } else {
        think_receipt->body_rejected++;
    }
    return 1;
}

const char *dm2_v1_think_creature_source_evidence(void)
{
    return
        "DM2 V1 per-cell DM2_THINK_CREATURE binding — skproject source-lock\n"
        "Source: skproject/SKULLWIN/c_tim_proc.cpp:4079-4088 (0x21/0x22 dispatch: x=getxA, y=getyA, type word)\n"
        "Source: skproject/SKULLWIN/c_ai.cpp:5649-5677 (DM2_THINK_CREATURE prologue, early return without creature)\n"
        "Source: skproject/SKULLWIN/c_querydb.cpp:1486-1507 (DM2_GET_CREATURE_AT chain walk, DB index bits 10-13)\n"
        "Source: skproject/SKULLWIN/c_map.cpp:44-69 (GET_OBJECT_INDEX_FROM_TILE / GET_TILE_RECORD_LINK)\n"
        "Source: skproject/SKULLWIN/c_timer.h:78-80 (getxA/getyA value_a bytes)\n"
        "Source: skproject/SKULLWIN/c_timer.h:64 (getmap = high byte of l_00)\n";
}
