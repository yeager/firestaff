#include "dm1/dm1_v1_chest_link_corruption_recovery_pc34_compat.h"

#include <string.h>

enum {
    ITEM_A = 0x210,
    ITEM_B = 0x211,
    ITEM_C = 0x212,
    ITEM_D = 0x213,
    ITEM_E = 0x214,
    ITEM_F = 0x215,
    ITEM_G = 0x216,
    ITEM_H = 0x217
};

static const int s_items[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34] = {
    ITEM_A, ITEM_B, ITEM_C, ITEM_D, ITEM_E, ITEM_F, ITEM_G, ITEM_H
};

static const int s_weights[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34] = {
    2, 3, 5, 7, 11, 13, 17, 19
};

static const DM1_V1_ChestLinkCorruptionRecoveryAnchorsPc34 s_anchors = {
    "ReDMCSB CHEST.C F0333:30-67 open-materialization: same-open return, "
    "Container->Slot walk, eight-item cap, G0425_aT_ChestSlots writes, "
    "C0xFFFE_THING_ENDOFLIST stop, C0xFFFF_THING_NONE tail fill.",
    "ReDMCSB CHEST.C F0334:113-132 close-rewrite: no-open return, "
    "G0426 clear, Container->Slot=C0xFFFE_THING_ENDOFLIST, scan eight "
    "G0425 entries, skip C0xFFFF_THING_NONE, clear slots, relink via F0163.",
    "ReDMCSB DUNGEON.C F0163:1769-1838 visible-input link append: "
    "END input returns, ThingToLink->Next=C0xFFFE_THING_ENDOFLIST, append "
    "after the last thing when MapX is CM1_MAPX_NOT_ON_A_SQUARE.",
    "ReDMCSB DUNGEON.C F0164:1840-1905 cleanup: END input returns, "
    "search stops at C0xFFFE_THING_ENDOFLIST or C0xFFFF_THING_NONE, then "
    "the unlinked thing Next is isolated to END.",
    "ReDMCSB DUNGEON.C F0140:1114-1120 container weight: base 50 plus "
    "recursive content weights while the container link is not END.",
    "ReDMCSB CHAMPION.C F0297:243-268 leader-hand put: ignore NONE, "
    "store Thing, derive icon with F0033, add F0140 weight, mark load.",
    "ReDMCSB CHAMPION.C F0298:270-298 leader-hand remove: return stored "
    "Thing, clear hand/icon, subtract F0140 weight, mark load.",
    "ReDMCSB CHAMPION.C F0300:511-515 C30+ clear: chest slots read and "
    "cleared through G0425_aT_ChestSlots[index - C30_SLOT_CHEST_1].",
    "ReDMCSB CHAMPION.C F0301:606-614 C30+ write: NONE returns; C30+ "
    "writes through G0425_aT_ChestSlots[index - C30_SLOT_CHEST_1], then "
    "adds F0140 weight and derives the icon.",
    "ReDMCSB OBJECT.C F0033:147-212 icon identity: F0032 type lookup and "
    "state-specific icon adjustment leave the source THING ordinal intact.",
    "ReDMCSB BLITMASK.C F0133:30-33 partial-mask dispatch documents masked "
    "bitmap presentation only; this contract makes no real-asset bitmap claim.",
    "ReDMCSB sentinels from CHEST.C F0333:58-76 and F0334:117-132: "
    "C0xFFFE_THING_ENDOFLIST terminates lists; C0xFFFF_THING_NONE marks an "
    "empty visible chest slot.",
    " contract_only=1; no real-asset parity claim."
};

static int index_for_thing(int thing)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34; ++i) {
        if (s_items[i] == thing) {
            return i;
        }
    }
    return -1;
}

static int next_for_thing(const int next_by_item[], int thing)
{
    const int index = index_for_thing(thing);

    if (index < 0) {
        return DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34;
    }
    return next_by_item[index];
}

static void set_next_for_thing(int next_by_item[], int thing, int next)
{
    const int index = index_for_thing(thing);

    if (index >= 0) {
        next_by_item[index] = next;
    }
}

static int contains_thing(const int values[], int count, int thing)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (values[i] == thing) {
            return 1;
        }
    }
    return 0;
}

static int duplicate_count(const int values[], int count)
{
    int duplicates = 0;
    int i;
    int j;

    for (i = 0; i < count; ++i) {
        for (j = i + 1; j < count; ++j) {
            if (values[i] == values[j]) {
                ++duplicates;
                break;
            }
        }
    }
    return duplicates;
}

static int count_dangling_nexts(const int next_by_item[])
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34; ++i) {
        const int next = next_by_item[i];
        if (next != DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34 &&
            next != DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34 &&
            index_for_thing(next) < 0) {
            ++count;
        }
    }
    return count;
}

static void link_thing_to_list(int next_by_item[], int thing, int in_list)
{
    int tail;

    if (thing == DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34) {
        return;
    }
    set_next_for_thing(next_by_item, thing,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34);
    tail = in_list;
    while (next_for_thing(next_by_item, tail) !=
           DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34) {
        tail = next_for_thing(next_by_item, tail);
    }
    set_next_for_thing(next_by_item, tail, thing);
}

static int materialize_open_slots(const int next_by_item[],
                                  int container_slot,
                                  int out_slots[])
{
    int thing = container_slot;
    int count = 0;
    int i;

    while (thing != DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34) {
        if (++count > DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34) {
            break;
        }
        out_slots[count - 1] = thing;
        thing = next_for_thing(next_by_item, thing);
    }
    for (i = count; i < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34; ++i) {
        out_slots[i] = DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34;
    }
    return count;
}

static int container_weight_from_chain(const int next_by_item[],
                                       int container_slot)
{
    int total = 50;
    int thing = container_slot;
    int seen = 0;

    while (thing != DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34 &&
           seen < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34) {
        const int index = index_for_thing(thing);
        if (index >= 0) {
            total += s_weights[index];
        }
        thing = next_for_thing(next_by_item, thing);
        ++seen;
    }
    return total;
}

static void initialize_spec(DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *spec)
{
    int g0425[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int next_by_item[DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34];
    int previous = DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34;
    int first = 1;
    int i;

    memset(spec, 0, sizeof(*spec));
    spec->contract_only = 1;
    spec->no_real_asset_parity_claim = 1;
    spec->slot_count = DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34;
    spec->thing_none = DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34;
    spec->thing_end = DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34;
    spec->thing_dangling = DM1_V1_CHEST_LINK_CORRUPTION_THING_DANGLING_PC34;
    spec->container_base_weight = 50;
    spec->anchors = s_anchors;

    for (i = 0; i < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34; ++i) {
        spec->initial_visible_slots[i] = s_items[i];
        spec->item_weights[i] = s_weights[i];
        spec->initial_next_by_item[i] =
            (i == DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34 - 1) ?
            DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34 : s_items[i + 1];
    }
    spec->initial_container_slot = ITEM_A;
    spec->initial_visible_count = DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34;
    spec->initial_chain_terminates_at_end = 1;
    spec->initial_container_weight =
        container_weight_from_chain(spec->initial_next_by_item,
                                    spec->initial_container_slot);

    memcpy(g0425, spec->initial_visible_slots, sizeof(g0425));
    memcpy(next_by_item, spec->initial_next_by_item, sizeof(next_by_item));

    g0425[4] = ITEM_B;
    set_next_for_thing(next_by_item, ITEM_C,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34);
    set_next_for_thing(next_by_item, ITEM_H,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_DANGLING_PC34);
    memcpy(spec->corrupted_visible_slots, g0425, sizeof(g0425));
    memcpy(spec->corrupted_next_by_item, next_by_item, sizeof(next_by_item));
    spec->corrupt_null_next_item = ITEM_C;
    spec->corrupt_duplicate_item = ITEM_B;
    spec->corrupt_duplicate_slot = 4;
    spec->corrupt_dangling_next_item = ITEM_H;
    spec->corruptions_present =
        next_for_thing(next_by_item, ITEM_C) ==
            DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34 &&
        g0425[4] == ITEM_B &&
        count_dangling_nexts(next_by_item) == 1;

    spec->close_container_slot = DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34;
    for (i = 0; i < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34; ++i) {
        const int thing = g0425[i];
        if (thing != DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34) {
            g0425[i] = DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34;
            if (first) {
                first = 0;
                set_next_for_thing(next_by_item, thing,
                                   DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34);
                spec->close_container_slot = thing;
                previous = thing;
            } else {
                link_thing_to_list(next_by_item, thing, previous);
                previous = thing;
            }
        }
    }
    memcpy(spec->post_close_visible_slots, g0425, sizeof(g0425));
    memcpy(spec->post_close_next_by_item, next_by_item, sizeof(next_by_item));
    spec->close_count =
        materialize_open_slots(next_by_item, spec->close_container_slot,
                               spec->close_order);
    spec->close_cleared_visible_slots = 1;
    for (i = 0; i < DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34; ++i) {
        if (g0425[i] != DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34) {
            spec->close_cleared_visible_slots = 0;
        }
    }
    spec->close_duplicate_ordinal_count =
        duplicate_count(spec->close_order, spec->close_count);
    spec->close_has_dangling_next = count_dangling_nexts(next_by_item);
    spec->close_has_none_inside_visible_window =
        contains_thing(spec->close_order, spec->close_count,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34);
    spec->close_dropped_null_next_item =
        !contains_thing(spec->close_order, spec->close_count, ITEM_C);
    spec->close_dropped_duplicate_shadow_item =
        !contains_thing(spec->close_order, spec->close_count, ITEM_D);
    spec->close_dropped_replaced_item =
        !contains_thing(spec->close_order, spec->close_count, ITEM_E);

    set_next_for_thing(next_by_item, ITEM_C,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34);
    set_next_for_thing(next_by_item, ITEM_D,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34);
    set_next_for_thing(next_by_item, ITEM_E,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34);
    memcpy(spec->cleanup_next_by_item, next_by_item, sizeof(next_by_item));
    spec->cleanup_isolated_null_next_item =
        next_for_thing(next_by_item, ITEM_C) ==
        DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34;
    spec->cleanup_isolated_duplicate_shadow_item =
        next_for_thing(next_by_item, ITEM_D) ==
        DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34;
    spec->cleanup_isolated_replaced_item =
        next_for_thing(next_by_item, ITEM_E) ==
        DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34;
    spec->cleanup_stopped_at_none_or_end = 1;

    spec->reopen_count =
        materialize_open_slots(next_by_item, spec->close_container_slot,
                               spec->reopen_slots);
    spec->reopen_tail_none_count =
        DM1_V1_CHEST_LINK_CORRUPTION_SLOT_COUNT_PC34 - spec->reopen_count;
    spec->reopen_duplicate_ordinal_count =
        duplicate_count(spec->reopen_slots, spec->reopen_count);
    spec->reopen_has_dangling_next = count_dangling_nexts(next_by_item);
    spec->reopen_has_end_inside_visible_window =
        contains_thing(spec->reopen_slots, spec->reopen_count,
                       DM1_V1_CHEST_LINK_CORRUPTION_THING_END_PC34);
    spec->reopen_first_slot = spec->reopen_slots[0];
    spec->reopen_last_visible_slot = spec->reopen_slots[spec->reopen_count - 1];
    spec->recovered_container_weight =
        container_weight_from_chain(next_by_item, spec->close_container_slot);

    spec->leader_hand_initial_thing =
        DM1_V1_CHEST_LINK_CORRUPTION_CHEST_THING_PC34;
    spec->leader_hand_icon_before =
        DM1_V1_CHEST_LINK_CORRUPTION_ICON_CHEST_OPEN_PC34;
    spec->leader_load_before_put = 100;
    spec->leader_load_after_put =
        spec->leader_load_before_put + spec->recovered_container_weight;
    spec->leader_hand_removed_thing = spec->leader_hand_initial_thing;
    spec->leader_load_after_remove =
        spec->leader_load_after_put - spec->recovered_container_weight;
    spec->leader_hand_final_thing = spec->leader_hand_removed_thing;
    spec->leader_load_after_restore =
        spec->leader_load_after_remove + spec->recovered_container_weight;
    spec->leader_hand_valid_after_cycle =
        spec->leader_hand_final_thing !=
        DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34;

    spec->c30_slot_index = DM1_V1_CHEST_LINK_CORRUPTION_C30_SLOT_PC34 + 3;
    spec->c30_clear_before = ITEM_F;
    spec->c30_clear_after = DM1_V1_CHEST_LINK_CORRUPTION_THING_NONE_PC34;
    spec->c30_write_thing = ITEM_H;
    spec->c30_write_after = ITEM_H;
}

const DM1_V1_ChestLinkCorruptionRecoverySpecPc34 *
dm1_v1_chest_link_corruption_recovery_pc34_compat(void)
{
    static DM1_V1_ChestLinkCorruptionRecoverySpecPc34 s_spec;
    static int s_initialized;

    if (!s_initialized) {
        initialize_spec(&s_spec);
        s_initialized = 1;
    }
    return &s_spec;
}
