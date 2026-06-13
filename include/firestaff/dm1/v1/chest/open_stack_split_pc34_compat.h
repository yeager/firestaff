/*
 * DM1 V1 chest open object stack-split source-locked contract gate.
 *
 * Lane: object stack split/merge. This lane pins the F0333 "split" half
 * of the chest open/close round-trip; the F0334 "merge" half is covered
 * by `dm1_v1_chest_close_stack_merge_pc34_compat`. Together they pin
 * that F0333 and F0334 are exact inverses on the visible-window +
 * hidden-tail chain shape: open (split) produces 8 visible items in
 * chain order, leaves the 9th+ items reachable via F0159 from the 8th
 * item's Next, and close (merge) re-derives the same chain head +
 * body order.
 *
 * Source anchors (ReDMCSB):
 *   - CHEST.C F0333:30-67 open path: chain head read from
 *     P0693_ps_Container->Slot (F0333 line 60), F0159 walk via
 *     GENERIC->Next (F0333 line 65), 8-item cap with break on
 *     L1019_i_ThingCount > 8 (F0333 lines 62-65), C0xFFFE_THING_ENDOFLIST
 *     stop (F0333 line 60 / 65), C0xFFFF_THING_NONE tail fill for
 *     G0425_aT_ChestSlots[visible_count..7] (F0333 lines 68-74).
 *   - CHEST.C F0334:79-130 close path (negative no-F0334 anchor during
 *     the open path; asserted as the inverse "merge" operation after
 *     a follow-up close to verify round-trip).
 *   - DUNGEON.C F0159:1664-1681 get-next-thing: returns the Next field
 *     of the carried thing's GENERIC record verbatim. The F0333 walk
 *     relies on F0159 to read the chain.
 *   - DUNGEON.C F0163:1769-1838 list-append: the F0334 close path's
 *     list relink (this lane asserts it as a no-F0163 anchor during
 *     the open path and as a positive round-trip anchor after close).
 *   - DUNGEON.C F0140:1114-1120 container weight: 50 base + recursive
 *     chain weight; an open chest's visible G0425 contents contribute.
 *   - DEFS.H C0xFFFE_THING_ENDOFLIST, C0xFFFF_THING_NONE,
 *     C537..C544 chest slot ordinals, M569_PANEL_CHEST panel content.
 *
 * The split semantics pinned by this gate are:
 *
 *   1. F0333 reads the chain head from P0693_ps_Container->Slot
 *      directly. It does not synthesize a "first thing" from any
 *      derived source; the head is the Slot field written by F0334
 *      close (or by F0156 on chest creation), and F0333 reads it
 *      verbatim. This is the head contract the F0334 close path's
 *      re-merge must satisfy.
 *
 *   2. F0333 walks the chain via F0159. The walk produces the visible
 *      G0425 entries in the same order as the chain, so visible order
 *      == chain order for slots 0..7. The 8-item cap break at
 *      L1019_i_ThingCount > 8 (F0333 line 62-65) terminates the walk
 *      with 8 visible items, even if the chain has more.
 *
 *   3. The 8-item cap preserves the 8th item's Next pointer. F0333
 *      does NOT clobber Next on the 8th item; the 8th item's Next
 *      still points to the 9th item in the chain. F0159 walk from
 *      the 8th item therefore reaches the 9th, 10th, ... items in
 *      the chain (the "hidden tail"). This is the round-trip
 *      contract that the F0334 close path relies on to recover the
 *      full chain.
 *
 *   4. The C0xFFFF_THING_NONE tail fill for G0425[visible_count..7]
 *      means the visible window after open is a contiguous 8-entry
 *      array with the visible items in chain order at indices 0..N-1
 *      (N = min(chain length, 8)) and C0xFFFF_THING_NONE at indices
 *      N..7. The reopen-derivation from the chain produces the same
 *      shape.
 *
 *   5. The F0333 walk terminates on C0xFFFE_THING_ENDOFLIST, which is
 *      the chain terminator the F0334 close path writes onto the last
 *      chain item's Next. F0333 reads END verbatim and stops; it does
 *      not synthesize a terminator.
 *
 *   6. The F0333 open path does NOT call F0163 (no relink), so the
 *      chain order is preserved exactly. F0333 also does NOT call
 *      F0334 (no rewire during the open path).
 *
 *   7. Round-trip identity: open (F0333 split) + close (F0334 merge)
 *      + reopen (F0333 split) produces the same visible G0425 window
 *      as the first open. The chain head, body order, and hidden
 *      tail are preserved across the round trip.
 *
 *   8. The panel content transitions to M569_PANEL_CHEST (6) on open
 *      and stays at M569_PANEL_CHEST during the open + close cycle.
 *      F0333 sets the panel content at line 28 before the same-open
 *      return at lines 31-32 and before the F0334 close at lines
 *      34-39.
 *
 *   9. The split is idempotent on a chain of length 0 (no items):
 *      the visible window is all C0xFFFF_THING_NONE, the chain is
 *      already END, and F0333 produces no draws beyond the tail
 *      fill. The split is also idempotent on a chain of length 8:
 *      the visible window is full, and F0333 produces no tail fill.
 *
 *  10. The split is well-defined for chain lengths 1..7 and 9..N
 *      (the 8-item cap break behavior only matters for chain
 *      length >= 9; for chain length 1..7 the visible window is
 *      N items + 8-N tail fills, and for chain length 8 the
 *      visible window is 8 items + 0 tail fills).
 *
 * Reject anchors (negative contracts that the gate REJECTS):
 *   - open with chain length 0 must not produce any non-NONE visible
 *     item;
 *   - open with chain length 8 must produce exactly 8 visible items;
 *   - the chain head for the round-trip must equal the input chain
 *     head (no F0333-side rewriting);
 *   - the chain order after round-trip must equal the input chain
 *     order (no F0333-side shuffling);
 *   - the panel content must be M569_PANEL_CHEST across the open +
 *     close cycle.
 *
 * Source-locked contract-only; no real-asset or original-DOS pixel
 * parity claim. Deterministic, no game data, no GRAPHICS.DAT, no
 * DUNGEON.DAT, no M11 render path; uses the same in-memory inventory
 * helper that the chest_close_stack_merge gate uses.
 */
#ifndef FIRESTAFF_DM1_V1_CHEST_OPEN_STACK_SPLIT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_OPEN_STACK_SPLIT_PC34_COMPAT_H

#include "dm1_v1_inventory_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB: DUNGEON.C F0163:1796 + CHEST.C F0333:60 + F0334:113-114. */
#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_END  0xFFFE

/* ReDMCSB: CHEST.C F0333:72 + F0334:118 (G0425 empty slot sentinel). */
#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_THING_NONE 0xFFFF

#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_PANEL_CHEST 6

#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAIN_LEN_EMPTY   0
#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAIN_LEN_PARTIAL 5
#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAIN_LEN_EXACT   8
#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHAIN_LEN_OVER    12

#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL 4

#define DM1_PC34_CHEST_OPEN_STACK_SPLIT_CHEST_THING 0xA537

typedef struct {
    const char *f0333_open;
    const char *f0334_close;
    const char *f0159_walk;
    const char *f0163_append;
    const char *f0140_weight;
    const char *chain_end_sentinel;
    const char *chain_none_sentinel;
    const char *panel_content;
} DM1_V1_ChestOpenStackSplitAnchorsPc34;

typedef struct {
    const char *contract_marker;
    int contract_only;
    int chest_slot_count;
    int thing_end;
    int thing_none;
    int panel_chest;
    int chest_thing;
    int chain_length_empty;
    int chain_length_partial;
    int chain_length_exact;
    int chain_length_over;
    int overflow_tail_count;
    int partial_visible_count;
    int partial_tail_fill_count;
    int exact_visible_count;
    int exact_tail_fill_count;
    int over_visible_count;
    int over_tail_fill_count;
    int round_trip_visible_count;
    int round_trip_hidden_tail_count;
    DM1_V1_ChestOpenStackSplitAnchorsPc34 anchors;
} DM1_V1_ChestOpenStackSplitSpecPc34;

typedef struct {
    /* Spec-anchor contract strings. */
    int source_locked_contract_only;

    /* Empty chain (length 0) open. */
    int empty_open_result;
    int empty_panel_content;
    int empty_chest_thing;
    int empty_visible_count;
    int empty_tail_fill_count;
    int empty_visible_types[DM1_PC34_CHEST_SLOT_COUNT];

    /* Partial chain (length 5) open. */
    int partial_open_result;
    int partial_visible_count;
    int partial_tail_fill_count;
    int partial_visible_types[DM1_PC34_CHEST_SLOT_COUNT];
    int partial_chain_head_ordinal;

    /* Exact chain (length 8) open: full visible window, no tail fill. */
    int exact_open_result;
    int exact_visible_count;
    int exact_tail_fill_count;
    int exact_visible_types[DM1_PC34_CHEST_SLOT_COUNT];
    int exact_chain_head_ordinal;

    /* Overfull chain (length 12) open: 8 visible + 4 hidden tail. */
    int over_open_result;
    int over_visible_count;
    int over_tail_fill_count;
    int over_visible_types[DM1_PC34_CHEST_SLOT_COUNT];
    int over_chain_head_ordinal;
    int over_hidden_tail_count;
    int over_hidden_tail_types[DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL];

    /* Hidden tail reachability: F0159 walk from the 8th item reaches
     * the 9th, 10th, 11th, 12th items in the chain. */
    int over_eighth_next_to_ninth;
    int over_ninth_next_to_tenth;
    int over_tenth_next_to_eleventh;
    int over_eleventh_next_to_twelfth;
    int over_twelfth_terminator;
    int over_eighth_ordinal;

    /* Round-trip: open(overfull) -> close(merge) -> reopen(split)
     * preserves chain head, body order, and hidden tail. */
    int round_trip_close_result;
    int round_trip_reopen_result;
    int round_trip_reopen_visible_count;
    int round_trip_reopen_visible_types[DM1_PC34_CHEST_SLOT_COUNT];
    int round_trip_reopen_panel_content;
    int round_trip_reopen_open_chest_thing;
    int round_trip_reopen_eighth_ordinal;
    int round_trip_reopen_hidden_tail_count;
    int round_trip_reopen_hidden_tail_types
        [DM1_PC34_CHEST_OPEN_STACK_SPLIT_OVERFLOW_TAIL];
    int round_trip_chain_head_preserved;
    int round_trip_chain_order_preserved;
    int round_trip_hidden_tail_preserved;

    /* F0333 does not call F0163 / F0334 during the open path. */
    int f0163_not_called_during_open;
    int f0334_not_called_during_open;

    /* Panel content transitions on open. */
    int panel_content_after_open;
    int panel_content_after_close;
    int panel_content_after_reopen;

    /* Idempotence: open(overfull) twice in a row preserves the same
     * visible window + chain head. */
    int idempotent_open_result;
    int idempotent_visible_count;
    int idempotent_visible_types[DM1_PC34_CHEST_SLOT_COUNT];
    int idempotent_chain_head_ordinal;
    int idempotent_hidden_tail_count;
} DM1_V1_ChestOpenStackSplitProbePc34;

const char *dm1_v1_chest_open_stack_split_source_evidence_pc34(void);

const DM1_V1_ChestOpenStackSplitSpecPc34 *
dm1_v1_chest_open_stack_split_spec_pc34(void);

int dm1_v1_chest_open_stack_split_run_pc34(
    DM1_V1_ChestOpenStackSplitProbePc34 *out);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_CHEST_OPEN_STACK_SPLIT_PC34_COMPAT_H */
