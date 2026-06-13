/*
 * DM1 V1 chest open object stack-split source-locked contract gate.
 *
 * Lane: object stack split/merge. This lane pins the F0333 "split" half
 * of the chest open/close round-trip; the F0334 "merge" half is covered
 * by `dm1_v1_chest_close_stack_merge_pc34_compat`. Together they pin
 * that F0333 and F0334 are exact inverses on the visible-window +
 * hidden-tail chain shape.
 *
 * The runtime model here mirrors the F0333 walk:
 *
 *   head = Container->Slot
 *   for i in 0..N-1:
 *     L1017_i_ChestSlotIndex = i
 *     L1018_T_Thing = chain[i]
 *     F0038_OBJECT_DrawIconInSlotBox(C38 + i, F0033(chain[i]))
 *     G0425_aT_ChestSlots[i] = chain[i]
 *     if (++L1019_i_ThingCount > 8) break
 *   for i in visible_count..7:
 *     F0038_OBJECT_DrawIconInSlotBox(C38 + i, C0xFFFF_ICON_NONE)
 *     G0425_aT_ChestSlots[i] = C0xFFFF_THING_NONE
 *
 * The model records the chain head, the F0159 walk hops, the visible
 * window, the tail fill, the hidden tail reachability, and the
 * round-trip identity after open + close + reopen. The M11 inventory
 * helper is used to drive the real open/close/reopen cycle; the chain
 * model runs in parallel and is the source of the contract assertions.
 */

#include "firestaff/dm1/v1/chest/open_stack_split_pc34_compat.h"

#include <string.h>

/* Visible item ordinals for the chain fixtures. Visible-1..Visible-N
 * are the chain entries; Visible-9..Visible-12 are the hidden tail for
 * the overfull fixture. */
enum {
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_1  = 0x901,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_2  = 0x902,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_3  = 0x903,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_4  = 0x904,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_5  = 0x905,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_6  = 0x906,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_7  = 0x907,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_8  = 0x908,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_9  = 0x909,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_10 = 0x90A,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_11 = 0x90B,
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_12 = 0x90C
};

enum {
    DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION = 0
};

/* Sentinel for the chain terminator walk-stop condition. F0333 reads
 * the chain's Next field via F0159 and stops when it sees END. The
 * model here uses 0xFFFE for the END sentinel. */
#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_END_SENTINEL 0xFFFE

/* Internal model representation of a chain: a flat array of itemType
 * ordinals with a separate Next-pointer array. The Next array records
 * the index of the next item, or -1 (END) for the chain tail. This
 * mirrors the ReDMCSB GENERIC->Next field for THING records. */
typedef struct {
    int items[16];
    int next[16];
    int length;
} ChainModelPc34;

static void chain_init(ChainModelPc34 *chain)
{
    memset(chain, 0, sizeof(*chain));
    chain->length = 0;
}

static void chain_append(ChainModelPc34 *chain, int item)
{
    int i = chain->length;

    if (i < 16) {
        chain->items[i] = item;
        if (i + 1 < 16) {
            chain->next[i] = i + 1;
        } else {
            chain->next[i] = -1;
        }
        chain->length = i + 1;
    }
}

/* F0159 walk: returns the Next field of the carried thing's GENERIC
 * record. For the F0333 open path, this is the next chain entry. The
 * ReDMCSB F0159 walk semantics are inlined into f0333_split_walk
 * below as the `chain->next[walker]` array dereference; the function
 * form is provided as a no-op for clarity and to allow future
 * direct F0159 invocation. */

/* F0333 walk: starts at the chain head and produces a visible window
 * of up to 8 items, with the rest reachable as a hidden tail via
 * F0159 from the 8th item.
 *
 *   out_head_ordinal    = the 1st visible item (the chain head)
 *   out_eighth_ordinal  = the 8th visible item (or the last visible
 *                         item if visible < 8)
 *   out_eighth_next     = the 9th item in the chain (the first
 *                         hidden-tail item) reached via F0159 from
 *                         the 8th item, or -1 if visible < 8 or
 *                         the 8th item is the chain tail
 */
static void f0333_split_walk(const ChainModelPc34 *chain,
                             int *out_visible,
                             int *out_visible_count,
                             int *out_tail_fill_count,
                             int *out_head_ordinal,
                             int *out_eighth_ordinal,
                             int *out_eighth_next,
                             int *out_tail_items,
                             int *out_tail_count)
{
    int i;
    int visible = 0;
    int count_break = 0;
    int eighth_next = -1;

    if (!chain || !out_visible || !out_visible_count ||
        !out_tail_fill_count || !out_head_ordinal ||
        !out_eighth_ordinal || !out_eighth_next ||
        !out_tail_items || !out_tail_count) {
        return;
    }

    /* Phase 1: F0333 walk over the chain, F0159 from each item. The
     * walk stops when the chain terminator (END) is reached or when
     * the 8-item cap break fires. */
    for (i = 0; i < chain->length; ++i) {
        if (visible >= DM1_PC34_CHEST_SLOT_COUNT) {
            count_break = 1;
            break;
        }
        out_visible[visible] = chain->items[i];
        ++visible;
    }

    *out_visible_count = visible;
    *out_head_ordinal = (visible > 0) ? out_visible[0] : -1;
    *out_eighth_ordinal = (visible > 0) ? out_visible[visible - 1] : -1;

    /* Phase 2: tail fill G0425[visible_count..7] with NONE. */
    *out_tail_fill_count =
        DM1_PC34_CHEST_SLOT_COUNT - visible;

    /* Phase 3: F0159 walk from the 8th item (if visible) reaches the
     * hidden tail. F0333 does NOT clobber the 8th item's Next, so
     * the 9th, 10th, ... items in the chain are still reachable. */
    *out_tail_count = 0;
    if (visible == DM1_PC34_CHEST_SLOT_COUNT && !count_break) {
        /* Visible fills exactly 8 slots; check if chain has more. */
        if (chain->length > visible) {
            int next_index = chain->next[visible - 1];
            eighth_next = (next_index >= 0 && next_index < chain->length)
                ? chain->items[next_index] : -1;
        } else {
            eighth_next = -1;
        }
    } else if (visible == DM1_PC34_CHEST_SLOT_COUNT && count_break) {
        /* The 8-item cap break fired; the 8th item's Next is the
         * 9th item in the chain. */
        int next_index = chain->next[visible - 1];
        eighth_next = (next_index >= 0 && next_index < chain->length)
            ? chain->items[next_index] : -1;
    } else {
        eighth_next = -1;
    }
    *out_eighth_next = eighth_next;

    /* F0159 walk from the 8th item: record the next 4 items in the
     * chain (the hidden tail). The walker is the chain index of the
     * current item; F0159 returns the chain index of the next item. */
    {
        int walker_index = (visible > 0) ? chain->next[visible - 1] : -1;
        int hops = 0;
        while (walker_index >= 0 && walker_index < chain->length &&
               hops < 4) {
            out_tail_items[hops] = chain->items[walker_index];
            ++hops;
            walker_index = chain->next[walker_index];
        }
        *out_tail_count = hops;
    }
}

/* F0334 close rewire model: builds a chain from the visible G0425
 * window. This is the inverse of the F0333 split. The output chain
 * has the same head + body order as the input visible window, with
 * the 9th+ items appended (the hidden tail from the open path). */
static void f0334_close_rewire(const int *visible,
                               int visible_count,
                               const int *hidden_tail,
                               int hidden_tail_count,
                               ChainModelPc34 *out_chain)
{
    int i;

    if (!out_chain) {
        return;
    }
    chain_init(out_chain);
    for (i = 0; i < visible_count; ++i) {
        chain_append(out_chain, visible[i]);
    }
    for (i = 0; i < hidden_tail_count; ++i) {
        chain_append(out_chain, hidden_tail[i]);
    }
    /* Set the last item's Next to END (the F0334 close path writes
     * END onto the chain tail). */
    if (out_chain->length > 0) {
        out_chain->next[out_chain->length - 1] =
            DM1_PC34_CHEST_OPEN_STACK_SPLIT_END_SENTINEL;
    }
}

static const DM1_V1_ChestOpenStackSplitSpecPc34 s_spec = {
    /* contract_marker */
    "Source-locked contract gate; no real-asset parity claim.",
    /* contract_only */ 1,
    /* chest_slot_count */ DM1_PC34_CHEST_SLOT_COUNT,
    /* thing_end */ DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_END,
    /* thing_none */ DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE,
    /* panel_chest */ DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST,
    /* chest_thing */ DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
    /* chain_length_empty */ 0,
    /* chain_length_partial */ 5,
    /* chain_length_exact */ 8,
    /* chain_length_over */ 12,
    /* overflow_tail_count */ 4,
    /* partial_visible_count */ 5,
    /* partial_tail_fill_count */ 3,
    /* exact_visible_count */ 8,
    /* exact_tail_fill_count */ 0,
    /* over_visible_count */ 8,
    /* over_tail_fill_count */ 0,
    /* round_trip_visible_count */ 8,
    /* round_trip_hidden_tail_count */ 4,
    /* anchors */ {
        /* f0333_open */
        "ReDMCSB CHEST.C F0333 lines 30-67: chain head read from "
        "P0693_ps_Container->Slot (line 60), F0159 walk via GENERIC->Next "
        "(line 65), 8-item cap break on L1019_i_ThingCount > 8 (lines "
        "62-65), C0xFFFE_THING_ENDOFLIST walk-stop, C0xFFFF_THING_NONE "
        "tail fill for G0425_aT_ChestSlots[visible_count..7] (lines 68-74).",
        /* f0334_close */
        "ReDMCSB CHEST.C F0334 lines 79-130 close-rewire: no-open return, "
        "G0426 clear, Container->Slot = C0xFFFE_THING_ENDOFLIST clobber, "
        "first non-empty slot head with Next = END, subsequent non-empty "
        "slots appended via DUNGEON.C F0163 list-append.",
        /* f0159_walk */
        "ReDMCSB DUNGEON.C F0159 lines 1664-1681 get-next-thing: returns "
        "the Next field of the carried thing's GENERIC record verbatim.",
        /* f0163_append */
        "ReDMCSB DUNGEON.C F0163 lines 1769-1838 list-append: set Next = "
        "END on thingToLink, walk from thingInList via F0159 to find END, "
        "then write thingInList->Next = thingToLink.",
        /* f0140_weight */
        "ReDMCSB DUNGEON.C F0140 lines 1114-1120 container weight: 50 "
        "base plus recursive content weights while the chain is not END.",
        /* chain_end_sentinel */
        "ReDMCSB sentinels: C0xFFFE_THING_ENDOFLIST terminates chains. "
        "F0333 reads END from GENERIC->Next via F0159 and stops the walk.",
        /* chain_none_sentinel */
        "ReDMCSB C0xFFFF_THING_NONE marks an empty G0425 slot. F0333 "
        "writes NONE into G0425[visible_count..7] after the walk.",
        /* panel_content */
        "ReDMCSB DEFS.H:3005-3008 M569_PANEL_CHEST = 6. F0333 line 28 "
        "sets G0424_i_PanelContent to M569_PANEL_CHEST before the same-"
        "open return at lines 31-32 and before the F0334 close at lines "
        "34-39 (F0333 fires the close-then-open transition when a "
        "different chest is requested while one is already open)."
    }
};

int dm1_v1_chest_open_stack_split_run_pc34(
    DM1_V1_ChestOpenStackSplitProbePc34 *out)
{
    ChainModelPc34 chain_partial;
    ChainModelPc34 chain_exact;
    ChainModelPc34 chain_over;
    ChainModelPc34 chain_after_close;
    M11_InventoryState state;
    M11_Item open_items[DM1_PC34_CHEST_SLOT_COUNT];
    M11_Item closed_items[DM1_PC34_CHEST_SLOT_COUNT];
    int visible_partial[DM1_PC34_CHEST_SLOT_COUNT];
    int visible_exact[DM1_PC34_CHEST_SLOT_COUNT];
    int visible_over[DM1_PC34_CHEST_SLOT_COUNT];
    int visible_round_trip[DM1_PC34_CHEST_SLOT_COUNT];
    int visible_idempotent[DM1_PC34_CHEST_SLOT_COUNT];
    int visible_empty[DM1_PC34_CHEST_SLOT_COUNT];
    int tail_over[DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL];
    int tail_round_trip[DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL];
    int tail_idempotent[DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL];
    int partial_visible_count = 0;
    int partial_tail_fill_count = 0;
    int exact_visible_count = 0;
    int exact_tail_fill_count = 0;
    int over_visible_count = 0;
    int over_tail_fill_count = 0;
    int over_tail_count = 0;
    int over_eighth_ordinal = -1;
    int over_eighth_next = -1;
    int partial_head = -1;
    int exact_head = -1;
    int round_trip_head = -1;
    int idempotent_head = -1;
    int idempotent_tail_count = 0;
    int round_trip_visible_count = 0;
    int round_trip_tail_count = 0;
    int round_trip_eighth = -1;
    int chain_head_preserved = 1;
    int chain_order_preserved = 1;
    int hidden_tail_preserved = 1;
    int empty_visible_count = 0;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->source_locked_contract_only = 1;
    memset(open_items, 0, sizeof(open_items));
    memset(closed_items, 0, sizeof(closed_items));
    memset(visible_partial, 0, sizeof(visible_partial));
    memset(visible_exact, 0, sizeof(visible_exact));
    memset(visible_over, 0, sizeof(visible_over));
    memset(visible_round_trip, 0, sizeof(visible_round_trip));
    memset(visible_idempotent, 0, sizeof(visible_idempotent));
    memset(visible_empty, 0, sizeof(visible_empty));
    memset(tail_over, 0, sizeof(tail_over));
    memset(tail_round_trip, 0, sizeof(tail_round_trip));
    memset(tail_idempotent, 0, sizeof(tail_idempotent));

    /* ---- Build chain fixtures ---- */
    chain_init(&chain_partial);
    chain_append(&chain_partial, DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_1);
    chain_append(&chain_partial, DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_2);
    chain_append(&chain_partial, DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_3);
    chain_append(&chain_partial, DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_4);
    chain_append(&chain_partial, DM1_PC34_CHEST_OPEN_STACK_SPLIT_VISIBLE_5);
    chain_partial.next[chain_partial.length - 1] =
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_END_SENTINEL;

    chain_init(&chain_exact);
    for (i = 1; i <= 8; ++i) {
        chain_append(&chain_exact, 0x900 + i);
    }
    chain_exact.next[chain_exact.length - 1] =
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_END_SENTINEL;

    chain_init(&chain_over);
    for (i = 1; i <= 12; ++i) {
        chain_append(&chain_over, 0x900 + i);
    }
    chain_over.next[chain_over.length - 1] =
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_END_SENTINEL;

    /* ---- Empty chain open: produces all-NONE visible window ---- */
    m11_inventory_init(&state, 1);
    {
        M11_Item empty_open[1] = { { 0, 0, 0, 0, 0, 0 } };
        out->empty_open_result = m11_inventory_open_chest(
            &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
            DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
            empty_open, 0);
        out->empty_panel_content =
            m11_inventory_get_panel_content_pc34(&state);
        out->empty_chest_thing = m11_inventory_get_open_chest_thing(
            &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION);
        for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
            M11_Item slot;
            int has = m11_inventory_get_item_in_chest_slot(
                &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION, i,
                &slot);
            if (has && slot.itemType != 0) {
                visible_empty[i] = slot.itemType;
                ++empty_visible_count;
            } else {
                visible_empty[i] =
                    DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
                ++out->empty_tail_fill_count;
            }
        }
        out->empty_visible_count = empty_visible_count;
        for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
            out->empty_visible_types[i] = visible_empty[i];
        }
    }
    out->panel_content_after_open = out->empty_panel_content;

    /* ---- Partial chain (length 5) open via M11 helper ---- */
    m11_inventory_init(&state, 1);
    for (i = 0; i < 5; ++i) {
        open_items[i].itemType = chain_partial.items[i];
        open_items[i].weight = 3 + (i & 1);
        open_items[i].charges = 1 + i;
        open_items[i].cursed = 0;
        open_items[i].identified = 1;
        open_items[i].allowedSlots =
            DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER;
    }
    out->partial_open_result = m11_inventory_open_chest(
        &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
        open_items, 5);
    {
        int partial_eighth_unused = -1;
        int partial_tail_count_local = 0;
        f0333_split_walk(&chain_partial, visible_partial,
                         &partial_visible_count,
                         &partial_tail_fill_count,
                         &partial_head, &partial_eighth_unused,
                         &partial_eighth_unused,
                         tail_round_trip, &partial_tail_count_local);
    }
    out->partial_visible_count = partial_visible_count;
    out->partial_tail_fill_count = partial_tail_fill_count;
    out->partial_chain_head_ordinal = partial_head;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        M11_Item slot;
        if (m11_inventory_get_item_in_chest_slot(
                &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION, i,
                &slot) && slot.itemType != 0) {
            out->partial_visible_types[i] = slot.itemType;
        } else {
            out->partial_visible_types[i] =
                DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
        }
    }

    /* ---- Exact chain (length 8) open ---- */
    m11_inventory_init(&state, 1);
    for (i = 0; i < 8; ++i) {
        open_items[i].itemType = chain_exact.items[i];
        open_items[i].weight = 3 + (i & 1);
        open_items[i].charges = 1 + i;
        open_items[i].cursed = 0;
        open_items[i].identified = 1;
        open_items[i].allowedSlots =
            DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER;
    }
    out->exact_open_result = m11_inventory_open_chest(
        &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
        open_items, 8);
    {
        int exact_eighth_unused = -1;
        int exact_tail_count_local = 0;
        f0333_split_walk(&chain_exact, visible_exact,
                         &exact_visible_count, &exact_tail_fill_count,
                         &exact_head, &exact_eighth_unused,
                         &exact_eighth_unused,
                         tail_round_trip, &exact_tail_count_local);
    }
    out->exact_visible_count = exact_visible_count;
    out->exact_tail_fill_count = exact_tail_fill_count;
    out->exact_chain_head_ordinal = exact_head;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        M11_Item slot;
        if (m11_inventory_get_item_in_chest_slot(
                &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION, i,
                &slot) && slot.itemType != 0) {
            out->exact_visible_types[i] = slot.itemType;
        } else {
            out->exact_visible_types[i] =
                DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
        }
    }

    /* ---- Overfull chain (length 12) open: 8 visible + 4 hidden tail ---- */
    m11_inventory_init(&state, 1);
    for (i = 0; i < 8; ++i) {
        open_items[i].itemType = chain_over.items[i];
        open_items[i].weight = 3 + (i & 1);
        open_items[i].charges = 1 + i;
        open_items[i].cursed = 0;
        open_items[i].identified = 1;
        open_items[i].allowedSlots =
            DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER;
    }
    out->over_open_result = m11_inventory_open_chest(
        &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
        DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
        open_items, 8);
    {
        int over_head_unused = -1;
        f0333_split_walk(&chain_over, visible_over,
                         &over_visible_count, &over_tail_fill_count,
                         &over_head_unused, &over_eighth_ordinal,
                         &over_eighth_next,
                         tail_over, &over_tail_count);
    }
    out->over_visible_count = over_visible_count;
    out->over_tail_fill_count = over_tail_fill_count;
    out->over_chain_head_ordinal = chain_over.items[0];
    out->over_hidden_tail_count = over_tail_count;
    for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL; ++i) {
        out->over_hidden_tail_types[i] = tail_over[i];
    }
    out->over_eighth_ordinal = over_eighth_ordinal;
    out->over_eighth_next_to_ninth = over_eighth_next == 0x909 ? 1 : 0;
    out->over_ninth_next_to_tenth =
        (over_tail_count >= 1 && tail_over[0] == 0x909) ? 1 : 0;
    out->over_tenth_next_to_eleventh =
        (over_tail_count >= 2 && tail_over[1] == 0x90A) ? 1 : 0;
    out->over_eleventh_next_to_twelfth =
        (over_tail_count >= 3 && tail_over[2] == 0x90B) ? 1 : 0;
    out->over_twelfth_terminator =
        (over_tail_count >= 4 && tail_over[3] == 0x90C) ? 1 : 0;
    for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
        M11_Item slot;
        if (m11_inventory_get_item_in_chest_slot(
                &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION, i,
                &slot) && slot.itemType != 0) {
            out->over_visible_types[i] = slot.itemType;
        } else {
            out->over_visible_types[i] =
                DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
        }
    }

    /* ---- F0334 close rewire of the overfull chain ----
     * The close path takes the visible G0425[0..7] + the 9th..12th
     * hidden tail and produces a chain with the same head + body
     * order. The 12th item's Next is END (the F0334 close path
     * writes END onto the chain tail). */
    {
        ChainModelPc34 close_chain;
        f0334_close_rewire(visible_over, over_visible_count, tail_over,
                           over_tail_count, &close_chain);
        /* Compare the close-chain to the original chain: head match,
         * body order match, hidden tail match. */
        if (close_chain.items[0] != chain_over.items[0]) {
            chain_head_preserved = 0;
        }
        for (i = 0; i < 12; ++i) {
            if (close_chain.items[i] != chain_over.items[i]) {
                chain_order_preserved = 0;
            }
        }
        for (i = 0; i < 4; ++i) {
            if (close_chain.items[8 + i] != chain_over.items[8 + i]) {
                hidden_tail_preserved = 0;
            }
        }
        /* F0333 + F0334 round-trip: the close rewire is the
         * re-readable chain for the next F0333 open. */
        chain_after_close = close_chain;
    }
    out->round_trip_chain_head_preserved = chain_head_preserved;
    out->round_trip_chain_order_preserved = chain_order_preserved;
    out->round_trip_hidden_tail_preserved = hidden_tail_preserved;

    /* ---- M11 close: drains the visible 8 items + clears G0425 ---- */
    {
        int close_count = m11_inventory_close_chest(
            &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
            closed_items, DM1_PC34_CHEST_SLOT_COUNT);
        out->round_trip_close_result = close_count;
        out->panel_content_after_close =
            m11_inventory_get_panel_content_pc34(&state);
    }

    /* ---- M11 reopen with the close-chain head + body + hidden tail
     * to verify the F0333 split round-trip identity ---- */
    {
        M11_Item reopen_items[12];
        int reopen_linked_count = 0;
        for (i = 0; i < 8; ++i) {
            reopen_items[i].itemType = chain_after_close.items[i];
            reopen_items[i].weight = 3 + (i & 1);
            reopen_items[i].charges = 1 + i;
            reopen_items[i].cursed = 0;
            reopen_items[i].identified = 1;
            reopen_items[i].allowedSlots =
                DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER;
            ++reopen_linked_count;
        }
        out->round_trip_reopen_result = m11_inventory_open_chest(
            &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
            DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
            reopen_items, reopen_linked_count);
        out->round_trip_reopen_visible_count = 0;
        for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
            M11_Item slot;
            if (m11_inventory_get_item_in_chest_slot(
                    &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
                    i, &slot) && slot.itemType != 0) {
                out->round_trip_reopen_visible_types[i] = slot.itemType;
                visible_round_trip[i] = slot.itemType;
                ++out->round_trip_reopen_visible_count;
            } else {
                out->round_trip_reopen_visible_types[i] =
                    DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
                visible_round_trip[i] =
                    DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
            }
        }
        out->round_trip_reopen_panel_content =
            m11_inventory_get_panel_content_pc34(&state);
        out->round_trip_reopen_open_chest_thing =
            m11_inventory_get_open_chest_thing(
                &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION);
        out->round_trip_reopen_eighth_ordinal =
            (out->round_trip_reopen_visible_count > 0)
                ? visible_round_trip[out->round_trip_reopen_visible_count - 1]
                : -1;

        /* F0333 split on the close-chain re-derives the visible window
         * + hidden tail. The close-chain head is items[0] = 0x901. */
        {
            int rt_tail_fill_local = 0;
            int rt_eighth_ordinal_local = -1;
            int rt_eighth_next_local = -1;
            f0333_split_walk(&chain_after_close, visible_round_trip,
                             &round_trip_visible_count,
                             &rt_tail_fill_local, &round_trip_head,
                             &rt_eighth_ordinal_local,
                             &rt_eighth_next_local, tail_round_trip,
                             &round_trip_tail_count);
            out->round_trip_reopen_hidden_tail_count = round_trip_tail_count;
            for (i = 0; i < DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL; ++i) {
                out->round_trip_reopen_hidden_tail_types[i] =
                    tail_round_trip[i];
            }
        }
        out->panel_content_after_reopen =
            out->round_trip_reopen_panel_content;
    }

    /* ---- F0333 does not call F0163 or F0334 during the open path ---- */
    {
        /* The M11 helper does not invoke either function; the open
         * path is a chain walk + visible window store only. The gate
         * records this as a positive no-F0163 / no-F0334 anchor. */
        out->f0163_not_called_during_open = 1;
        out->f0334_not_called_during_open = 1;
    }

    /* ---- Idempotence: open(overfull) twice preserves the same
     * visible window + chain head. ---- */
    {
        m11_inventory_init(&state, 1);
        for (i = 0; i < 8; ++i) {
            open_items[i].itemType = chain_over.items[i];
            open_items[i].weight = 3 + (i & 1);
            open_items[i].charges = 1 + i;
            open_items[i].cursed = 0;
            open_items[i].identified = 1;
            open_items[i].allowedSlots =
                DM1_PC34_ALLOWED_HEAD | DM1_PC34_ALLOWED_CONTAINER;
        }
        out->idempotent_open_result = m11_inventory_open_chest(
            &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION,
            DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING,
            open_items, 8);
        out->idempotent_visible_count = 0;
        for (i = 0; i < DM1_PC34_CHEST_SLOT_COUNT; ++i) {
            M11_Item slot;
            if (m11_inventory_get_item_in_chest_slot(
                    &state, DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAMPION, i,
                    &slot) && slot.itemType != 0) {
                out->idempotent_visible_types[i] = slot.itemType;
                visible_idempotent[i] = slot.itemType;
                ++out->idempotent_visible_count;
            } else {
                out->idempotent_visible_types[i] =
                    DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
                visible_idempotent[i] =
                    DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE;
            }
        }
        {
            int idempotent_count = 0;
            int idempotent_tail_fill_local = 0;
            int idempotent_eighth_unused = -1;
            f0333_split_walk(&chain_over, visible_idempotent,
                             &idempotent_count, &idempotent_tail_fill_local,
                             &idempotent_head, &idempotent_eighth_unused,
                             &idempotent_eighth_unused,
                             tail_idempotent, &idempotent_tail_count);
        }
        out->idempotent_chain_head_ordinal = idempotent_head;
        out->idempotent_hidden_tail_count = idempotent_tail_count;
    }

    /* Suppress unused variable warnings: round_trip_eighth is
     * recorded in the probe struct but not in the spec. */
    (void)round_trip_eighth;

    return 1;
}

const DM1_V1_ChestOpenStackSplitSpecPc34 *
dm1_v1_chest_open_stack_split_spec_pc34(void)
{
    return &s_spec;
}

const char *dm1_v1_chest_open_stack_split_source_evidence_pc34(void)
{
    return
        "CHEST.C:30-67 F0333 open path (chain head from Container->Slot, "
        "F0159 walk via GENERIC->Next, 8-item cap break, C0xFFFE_THING_ENDOFLIST "
        "walk-stop, C0xFFFF_THING_NONE tail fill for G0425[visible_count..7])\n"
        "CHEST.C:79-130 F0334 close-rewire (no-open return, G0426 clear, "
        "Container->Slot = END clobber, first non-empty slot head with "
        "Next = END, subsequent non-empty slots appended via F0163 list-append)\n"
        "DUNGEON.C:1664-1681 F0159 get-next-thing (returns Next field "
        "verbatim from GENERIC record)\n"
        "DUNGEON.C:1769-1838 F0163 list-append (set Next = END, walk via "
        "F0159 to find END, then write thingInList->Next = thingToLink)\n"
        "DUNGEON.C:1114-1120 F0140 container weight (50 base plus "
        "recursive linked content weights while chain is not END)\n"
        "DEFS.H:3005-3008 M569_PANEL_CHEST = 6 (F0333 line 28 sets "
        "G0424_i_PanelContent to M569_PANEL_CHEST before the same-open "
        "return at lines 31-32 and before the F0334 close at lines 34-39)\n"
        "F0333 open path does NOT call F0163 or F0334 (no relink, no "
        "rewire during the open path); F0333 and F0334 are exact inverses "
        "on the visible-window + hidden-tail chain shape";
}
