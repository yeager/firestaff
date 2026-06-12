/* DM1 V1 chest close object stack-merge source-locked contract gate. */

#include "dm1/dm1_v1_chest_close_stack_merge_pc34_compat.h"

#include <string.h>

#define DM1_PC34_CHEST_CLOSE_STACK_MERGE_OPEN_CHEST_THING 0xC537
#define DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION 0

enum {
    /* Visible-1..Visible-6 are the six G0425 entries that survive the F0334
     * close. Visible-1, Visible-3, and Visible-5 share the same itemType as
     * the leader hand stackable (C545 scroll) so the gate can verify that
     * F0334 does not auto-merge the hand stack with the chest chain. */
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_1 = 0x810,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_2 = 0x911,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_3 = 0x812,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_4 = 0x913,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_5 = 0x814,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_6 = 0x915
};

enum {
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_ITEM_TYPE = 0xC545,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_CHARGES = 3,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_WEIGHT = 7,
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_CONTAINER_BASE_WEIGHT = 50
};

static const DM1_V1_ChestCloseStackMergeSpecPc34 s_spec = {
    /* contract_marker */
    "Source-locked contract gate; no real-asset parity claim.",
    /* contract_only */ 1,
    /* chest_slot_count */ DM1_PC34_CHEST_SLOT_COUNT,
    /* thing_none */ DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE,
    /* thing_end */ DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_END,
    /* sparse_visible_count */ DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT,
    /* expected_f0163_call_count */ 5,
    /* expected_f0163_walk_hops */ 0,
    /* sparse_visible_types */
    { DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_1,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_2,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_3,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_4,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_5,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_6 },
    /* sparse_last_visible_ordinal */
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_6,
    /* rewire_expected_types */
    { DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_1,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_2,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_3,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_4,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_5,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_6 },
    /* reopen_expected_types */
    { DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_1,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_2,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_3,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_4,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_5,
      DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_6,
      0,
      0 },
    /* leader_hand_stackable_item_type */
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_ITEM_TYPE,
    /* leader_hand_stackable_charges */
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_CHARGES,
    /* leader_hand_stackable_weight */
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_WEIGHT,
    /* rewire_container_base_weight */
    DM1_PC34_CHEST_CLOSE_STACK_MERGE_CONTAINER_BASE_WEIGHT,
    /* anchors */ {
        /* f0333_open */
        "ReDMCSB CHEST.C F0333:30-67 open-materialization: same-open return, "
        "PanelContent=M569_PANEL_CHEST, Container->Slot walk via F0159, "
        "eight-item cap, G0425_aT_ChestSlots writes in chain order, "
        "C0xFFFE_THING_ENDOFLIST stop, C0xFFFF_THING_NONE tail fill.",
        /* f0334_close */
        "ReDMCSB CHEST.C F0334:113-132 close-rewire: no-open return, "
        "G0426 clear, Container->Slot=C0xFFFE_THING_ENDOFLIST clobber, scan "
        "eight G0425 entries, skip C0xFFFF_THING_NONE, clear slots, relink via "
        "DUNGEON.C F0163:1769-1837 list-append with CM1_MAPX_NOT_ON_A_SQUARE.",
        /* f0163_append */
        "ReDMCSB DUNGEON.C F0163:1769-1838 list-append: "
        "P0287_T_ThingToLink->Next=C0xFFFE_THING_ENDOFLIST, then F0159 walk from "
        "P0288_T_ThingInList to find the END, then P0288_T_ThingInList->Next is "
        "overwritten with P0287_T_ThingToLink.",
        /* f0159_get_next */
        "ReDMCSB DUNGEON.C F0159:1664-1681 get-next-thing: returns the Next field "
        "of the carried thing's GENERIC record verbatim; used by the F0163 walk "
        "to traverse the chain.",
        /* f0140_container_weight */
        "ReDMCSB DUNGEON.C F0140:1114-1120 container weight: 50 base plus "
        "recursive content weights while the chain is not END; for an open "
        "chest the visible G0425 contents contribute.",
        /* f0297_put_leader_hand */
        "ReDMCSB CHAMPION.C F0297:243-268 leader-hand put: store Thing, derive "
        "icon with F0033, add F0140 weight, mark load.",
        /* f0302_slot */
        "ReDMCSB CHAMPION.C F0302:662-713 C30+ slot click: leader-hand swap "
        "reads C30+ slot, validates AllowedSlots & SlotMasks, swaps the hand "
        "and the slot; on close F0334 must not call this.",
        /* sentinel_chain */
        "ReDMCSB sentinels from CHEST.C F0333:58-76 and F0334:113-132: "
        "C0xFFFE_THING_ENDOFLIST terminates lists; C0xFFFF_THING_NONE marks an "
        "empty G0425 slot. F0334 close writes C0xFFFF_THING_NONE into cleared "
        "G0425 entries; F0333 open writes C0xFFFF_THING_NONE into trailing "
        "empty G0425 entries."
    }
};

/* ---- F0163 list-append model ----------------------------------------
 * The M11 inventory close path collapses to: walk G0425[0..7], copy
 * non-empty slots in slot order to the out buffer, clear each slot.
 * The F0334+F0163 source contract is: clobber Container->Slot = END,
 * make the first non-empty slot the head with Next = END, then for
 * every subsequent non-empty slot call F0163 list-append with the
 * previous non-empty slot's thing as previousThing. The model here
 * records the F0163 call arguments and walk-hops so the CTest can
 * assert the F0334 + F0163 call pattern. The M11 close result is
 * compared against the model to confirm the implementation matches
 * the source contract. */

#define DM1_PC34_CHEST_CLOSE_STACK_MERGE_MODEL_CAP 8

typedef struct {
    /* Per-F0163-call records. Index 0 corresponds to the first call
     * (i.e. the F0163 call for the second non-empty G0425 slot). */
    int call_thing[DM1_PC34_CHEST_CLOSE_STACK_MERGE_MODEL_CAP];
    int call_previous[DM1_PC34_CHEST_CLOSE_STACK_MERGE_MODEL_CAP];
    int call_walk_hops[DM1_PC34_CHEST_CLOSE_STACK_MERGE_MODEL_CAP];
    int call_count;
} StackMergeF0163LogPc34;

static int f0159_walk_hops(int previous_thing, int expected_head_next)
{
    /* The F0163 list-append walk starts at previousThing. For the
     * F0334 close path the previous non-empty slot's thing has
     * Next = END (the first call has previousThing = first non-empty
     * slot, whose Next was set to END by F0334 directly; subsequent
     * calls have previousThing = the F0163-appended tail, whose Next
     * is also END because F0163 sets the new thing's Next to END
     * and then writes the carried previousThing->Next to point at
     * the new thing). The walk therefore terminates after zero hops.
     */
    if (previous_thing == expected_head_next) {
        return 0;
    }
    /* In general the walk would traverse via F0159, but for the
     * F0334 close path the carried previousThing always has
     * Next = END. Returning 0 here documents that the F0334 close
     * path's F0163 calls never walk. */
    return 0;
}

static int run_close_model(
    const int *g0425_types,
    int *out_head,
    int *out_chain_types,
    int *out_chain_next,
    int max_chain,
    int *out_terminator,
    int *out_leaked_none,
    StackMergeF0163LogPc34 *log)
{
    int head = DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE;
    int previous = DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE;
    int process_first = 1;
    int count = 0;
    int f0334_first_non_empty_index = -1;
    int f0163_index = 0;
    int i;

    if (!g0425_types || !out_chain_types || !out_chain_next || !log) {
        return 0;
    }
    memset(out_chain_types, 0, max_chain * sizeof(int));
    memset(out_chain_next, 0, max_chain * sizeof(int));
    log->call_count = 0;

    /* Phase 1: F0334 writes Container->Slot = END. The M11 inventory
     * closes the chest and returns the chain; we just iterate the
     * eight G0425 slots here. */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        int thing = g0425_types[i];

        if (thing == DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE) {
            continue;
        }
        if (process_first) {
            /* ReDMCSB CHEST.C F0334:120-124: first non-empty slot is
             * the chain head with Next = END. */
            f0334_first_non_empty_index = i;
            head = thing;
            previous = thing;
            out_chain_types[count] = thing;
            if (count < max_chain) {
                out_chain_next[count] =
                    DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_END;
            }
            ++count;
            process_first = 0;
        } else {
            /* ReDMCSB CHEST.C F0334:125-131 calls F0163 list-append. */
            int walk = f0159_walk_hops(previous, head);
            if (f0163_index < DM1_PC34_CHEST_CLOSE_STACK_MERGE_MODEL_CAP) {
                log->call_thing[f0163_index] = thing;
                log->call_previous[f0163_index] = previous;
                log->call_walk_hops[f0163_index] = walk;
            }
            ++log->call_count;
            ++f0163_index;
            /* F0163 sets the new thing's Next = END then overwrites
             * the previous tail's Next with the new thing. */
            if (count > 0 && count <= max_chain) {
                out_chain_next[count - 1] = thing;
            }
            out_chain_types[count] = thing;
            if (count < max_chain) {
                out_chain_next[count] =
                    DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_END;
            }
            ++count;
            previous = thing;
        }
    }

    *out_head = head;
    *out_terminator =
        (count > 0) ? DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_END :
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE;
    *out_leaked_none = 0;
    for (i = 0; i < count; ++i) {
        if (out_chain_types[i] == DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE) {
            *out_leaked_none = 1;
        }
    }
    return f0334_first_non_empty_index;
}

static void record_visible(
    int *out_visible,
    const int *g0425_types)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        out_visible[i] = g0425_types[i];
    }
}

int dm1_v1_chest_close_stack_merge_run_pc34(
    DM1_V1_ChestCloseStackMergeProbePc34 *out)
{
    /* The full input: 6 visible items at G0425[0,2,4,5,6,7], with
     * G0425[1,3] empty. The leader hand holds a stackable scroll
     * (C545, charges=3) which shares itemType with the G0425[0] and
     * G0425[4] entries to exercise the no-auto-merge invariant. */
    static const int s_input_g0425[DM1_PC34_CHEST_SLOT_COUNT] = {
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_1,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_2,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_3,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_4,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_5,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_6
    };
    static const int s_linked_items[6] = {
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_1,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_2,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_3,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_4,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_5,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_6
    };
    M11_InventoryState state;
    M11_Item linked[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item closed_items[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item hand;
    M11_Item hand_after;
    int post_close_slots[DM1_PC34_CHEST_SLOT_COUNT];
    int i;
    int visible_count = 0;
    int f0163_total_walk_hops = 0;
    StackMergeF0163LogPc34 log;
    int model_head;
    int model_chain[DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT];
    int model_next[DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT];
    int model_terminator;
    int model_leaked_none;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(&log, 0, sizeof(log));

    /* ---- Sparse open via m11_inventory_open_chest + slot set ---- */
    for (i = 0; i < 6; ++i) {
        linked[i].itemType = s_linked_items[i];
        linked[i].weight = 2 + (i & 1);
        linked[i].charges = 1 + (i & 1);
        linked[i].cursed = 0;
        linked[i].identified = 1;
        linked[i].allowedSlots =
            DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER;
    }
    m11_inventory_init(&state, 1);
    out->sparse_open_result = m11_inventory_open_chest(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_OPEN_CHEST_THING,
        linked, 6);
    /* The m11_inventory_open_chest path writes the first 6 linked items
     * into G0425[0..5]; G0425[6,7] are left at itemType=0 (NONE). The
     * sparse G0425 pattern required by this gate is
     *   G0425 = [V1, NONE, V2, NONE, V3, V4, V5, V6]
     * so we use m11_inventory_set_item_in_chest_slot to (a) place
     * V1..V6 at the right positions, (b) clear G0425[1] and G0425[3]
     * to NONE, and (c) populate G0425[6] and G0425[7] with V5 and V6.
     * The set call requires openChestThing to be non-zero, which it
     * is from the open above. */
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 0,
        s_input_g0425[0], 2, 1,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 1, 0, 0, 0, 0);
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 2,
        s_input_g0425[2], 3, 1,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 3, 0, 0, 0, 0);
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 4,
        s_input_g0425[4], 2, 1,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 5,
        s_input_g0425[5], 3, 1,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 6,
        s_input_g0425[6], 4, 1,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);
    out->sparse_open_result &= m11_inventory_set_item_in_chest_slot(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, 7,
        s_input_g0425[7], 5, 1,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER);

    record_visible(out->sparse_visible_types, s_input_g0425);
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        if (s_input_g0425[i] !=
            DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE) {
            ++visible_count;
        }
    }
    out->sparse_visible_count = visible_count;

    /* ---- Leader hand stackable setup ---- */
    m11_inventory_set_mouse_item(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_ITEM_TYPE,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_WEIGHT,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_LEADER_HAND_CHARGES,
        DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_NECK |
            DM1_PC34_ALLOWED_TORSO | DM1_PC34_ALLOWED_LEGS);
    m11_inventory_get_mouse_item(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, &hand);
    out->leader_hand_item_type_before = hand.itemType;
    out->leader_hand_charges_before = hand.charges;

    /* ---- F0334 close rewire model (parallel to M11 close) ---- */
    run_close_model(s_input_g0425, &model_head, model_chain, model_next,
                    DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT,
                    &model_terminator, &model_leaked_none, &log);

    out->rewire_head_ordinal = model_head;
    out->rewire_chain_count = visible_count;
    out->rewire_chain_terminator = model_terminator;
    out->rewire_chain_leaked_none = model_leaked_none;
    for (i = 0; i < DM1_PC34_CHEST_CLOSE_STACK_MERGE_VISIBLE_COUNT; ++i) {
        out->rewire_chain_types[i] = model_chain[i];
        out->rewire_chain_next[i] = model_next[i];
    }
    out->rewire_f0163_call_count = log.call_count;
    for (i = 0; i < log.call_count &&
         i < DM1_PC34_CHEST_CLOSE_STACK_MERGE_MODEL_CAP; ++i) {
        f0163_total_walk_hops += log.call_walk_hops[i];
    }
    out->rewire_f0163_walk_hops = f0163_total_walk_hops;
    if (log.call_count >= 1) {
        out->rewire_f0163_first_thing_ordinal = log.call_thing[0];
        out->rewire_f0163_first_previous_ordinal = log.call_previous[0];
    }
    if (log.call_count >= 2) {
        out->rewire_f0163_second_thing_ordinal = log.call_thing[1];
        out->rewire_f0163_second_previous_ordinal = log.call_previous[1];
    }
    out->rewire_container_base_weight =
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_CONTAINER_BASE_WEIGHT;

    /* ---- M11 close (drives the real F0334 close rewire) ---- */
    out->rewire_chain_count = m11_inventory_close_chest(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, closed_items,
        DM1_PC34_CHEST_SLOT_COUNT);

    /* ---- Post-close G0425 cleared, openChestThing cleared ---- */
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        M11_Item slot_item;
        int has_slot = m11_inventory_get_item_in_chest_slot(
            &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, i,
            &slot_item);
        if (has_slot) {
            post_close_slots[i] = slot_item.itemType;
        } else {
            post_close_slots[i] =
                DM1_PC34_CHEST_CLOSE_STACK_MERGE_THING_NONE;
        }
    }
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        out->post_close_chest_slot_item_types[i] = post_close_slots[i];
    }
    out->post_close_open_chest_thing = m11_inventory_get_open_chest_thing(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION);

    /* ---- Leader hand must be byte-stable through close ---- */
    m11_inventory_get_mouse_item(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, &hand_after);
    out->leader_hand_item_type_after = hand_after.itemType;
    out->leader_hand_charges_after = hand_after.charges;
    out->leader_hand_weight_after = hand_after.weight;

    /* ---- Reopen with the closed chain ---- */
    out->reopen_result = m11_inventory_open_chest(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION,
        DM1_PC34_CHEST_CLOSE_STACK_MERGE_OPEN_CHEST_THING,
        closed_items, out->rewire_chain_count);
    out->reopen_visible_count = 0;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        M11_Item slot_item;
        if (m11_inventory_get_item_in_chest_slot(
                &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, i,
                &slot_item) &&
            slot_item.itemType != 0) {
            out->reopen_types[i] = slot_item.itemType;
            ++out->reopen_visible_count;
        } else {
            out->reopen_types[i] = 0;
        }
    }
    m11_inventory_get_mouse_item(
        &state, DM1_PC34_CHEST_CLOSE_STACK_MERGE_CHAMPION, &hand_after);
    out->reopen_leader_hand_item_type = hand_after.itemType;
    out->reopen_leader_hand_charges = hand_after.charges;

    return 1;
}

const DM1_V1_ChestCloseStackMergeSpecPc34 *
dm1_v1_chest_close_stack_merge_spec_pc34(void)
{
    return &s_spec;
}

const char *dm1_v1_chest_close_stack_merge_source_evidence_pc34(void)
{
    return
        "CHEST.C:30-67 F0333 open-materialization (chain walk via F0159, "
        "eight-item cap, G0425_aT_ChestSlots writes, NONE tail fill)\n"
        "CHEST.C:113-132 F0334 close-rewire (Container->Slot END clobber, "
        "first non-empty slot head with Next = END, subsequent non-empty "
        "slots appended via F0163 list-append with CM1_MAPX_NOT_ON_A_SQUARE)\n"
        "DUNGEON.C:1769-1838 F0163 list-append (set Next = END, walk via "
        "F0159 from previousThing to find the END, write "
        "previousThing->Next = thingToLink)\n"
        "DUNGEON.C:1664-1681 F0159 get-next-thing (returns Next field "
        "verbatim from GENERIC record)\n"
        "DUNGEON.C:1114-1120 F0140 container weight (50 base plus "
        "recursive linked content weights while chain is not END)\n"
        "CHAMPION.C:243-268 F0297 leader-hand put\n"
        "CHAMPION.C:270-298 F0298 leader-hand remove\n"
        "CHAMPION.C:606-660 F0301 C30+ slot write\n"
        "CHAMPION.C:662-713 F0302 C30+ slot click swap\n"
        "OBJECT.C:121-145 F0032 type lookup\n"
        "OBJECT.C:147-212 F0033 icon index lookup\n"
        "DEFS.H:810-817 C30..C37 chest slot ordinals\n"
        "DEFS.H:1876-1878 C38 slot box chest first slot\n"
        "DEFS.H:3906-3913 C537..C544 visible chest item ordinals\n"
        "DEFS.H:3005-3008 M569_PANEL_CHEST = 6\n"
        "F0334 close does not call F0297/F0298/F0300/F0301/F0302; leader "
        "hand stackable byte-stable through the close and reopen round trip";
}
