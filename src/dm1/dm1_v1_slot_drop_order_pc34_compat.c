#include "firestaff/dm1/v1/slot_drop_order_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0057_ai_Graphic562_SlotDropOrder):
 * - DATA.C:105 - declaration of G0057_ai_Graphic562_SlotDropOrder[30]
 * - DATA.C:436-466 - PC 3.4 EN init (30 C_SLOT_* ordinals, see comments)
 * - DATA.C:466 - last entry of slot-drop-order init block
 * - DATA.C:1119 - post-1.3 Atari init (same values)
 * - CHAMPION.C:1546 - F0300_CHAMPION_GetObjectRemovedFromSlot(
 *                       P0660_ui_ChampionIndex, G0057[L0961_ui_SlotIndex])
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-859 (Graphics.dat init-table gates batches 1-11). This
 * gate is a non-mirror-candidate contract for the G0057
 * slot drop-order priority table.
 */

enum {
    kTableSize    = 30,
    kIndexOOR     = -1,
    kFirstEntry   = 5,    /* C05_SLOT_FEET */
    kLastEntry    = 1     /* C01_SLOT_ACTION_HAND */
};

/* G0057 PC 3.4 EN init (DATA.C:436-466). Numeric values substitute
 * the C_NNN_SLOT_* constants: each constant's numeric value is
 * documented in DEFS.H (see header comments). */
static const int s_g0057[kTableSize] = {
    /*  0 */   5, /* C05_SLOT_FEET */
    /*  1 */   4, /* C04_SLOT_LEGS */
    /*  2 */   9, /* C09_SLOT_QUIVER_LINE2_2 */
    /*  3 */   8, /* C08_SLOT_QUIVER_LINE1_2 */
    /*  4 */   7, /* C07_SLOT_QUIVER_LINE2_1 */
    /*  5 */  12, /* C12_SLOT_QUIVER_LINE1_1 */
    /*  6 */   6, /* C06_SLOT_POUCH_2 */
    /*  7 */  11, /* C11_SLOT_POUCH_1 */
    /*  8 */   3, /* C03_SLOT_TORSO */
    /*  9 */  13, /* C13_SLOT_BACKPACK_LINE1_1 */
    /* 10 */  14, /* C14_SLOT_BACKPACK_LINE2_2 */
    /* 11 */  15, /* C15_SLOT_BACKPACK_LINE2_3 */
    /* 12 */  16, /* C16_SLOT_BACKPACK_LINE2_4 */
    /* 13 */  17, /* C17_SLOT_BACKPACK_LINE2_5 */
    /* 14 */  18, /* C18_SLOT_BACKPACK_LINE2_6 */
    /* 15 */  19, /* C19_SLOT_BACKPACK_LINE2_7 */
    /* 16 */  20, /* C20_SLOT_BACKPACK_LINE2_8 */
    /* 17 */  21, /* C21_SLOT_BACKPACK_LINE2_9 */
    /* 18 */  22, /* C22_SLOT_BACKPACK_LINE1_2 */
    /* 19 */  23, /* C23_SLOT_BACKPACK_LINE1_3 */
    /* 20 */  24, /* C24_SLOT_BACKPACK_LINE1_4 */
    /* 21 */  25, /* C25_SLOT_BACKPACK_LINE1_5 */
    /* 22 */  26, /* C26_SLOT_BACKPACK_LINE1_6 */
    /* 23 */  27, /* C27_SLOT_BACKPACK_LINE1_7 */
    /* 24 */  28, /* C28_SLOT_BACKPACK_LINE1_8 */
    /* 25 */  29, /* C29_SLOT_BACKPACK_LINE1_9 */
    /* 26 */  10, /* C10_SLOT_NECK */
    /* 27 */   2, /* C02_SLOT_HEAD */
    /* 28 */   0, /* C00_SLOT_READY_HAND */
    /* 29 */   1  /* C01_SLOT_ACTION_HAND (last) */
};

const int *
dm1_v1_slot_drop_order_table_pc34(void)
{
    return s_g0057;
}

int
dm1_v1_slot_drop_order_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_slot_drop_order_get_pc34(int slot_index)
{
    if (slot_index < 0 || slot_index >= kTableSize) {
        return kIndexOOR;
    }
    return s_g0057[slot_index];
}

int
dm1_v1_slot_drop_order_first_pc34(void)
{
    return s_g0057[0];
}

int
dm1_v1_slot_drop_order_last_pc34(void)
{
    return s_g0057[kTableSize - 1];
}

int
dm1_v1_slot_drop_order_run_pc34(
    DM1_V1_SlotDropOrderResultPc34 *out)
{
    int table_matches_declaration = 1;
    int first_entry_feet_slot_5 = 1;
    int last_entry_action_hand_slot_1 = 1;
    int all_values_in_byte_range = 1;
    int all_values_distinct = 1;
    int all_backpack_slots_covered = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i, j;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0057[i];
    }
    out->tableSize = kTableSize;

    /* Phase 1: first entry is C05_SLOT_FEET = 5. */
    if (s_g0057[0] != kFirstEntry) {
        first_entry_feet_slot_5 = 0;
    }
    out->firstEntryFeetSlot5 = first_entry_feet_slot_5;

    /* Phase 2: last entry is C01_SLOT_ACTION_HAND = 1. */
    if (s_g0057[kTableSize - 1] != kLastEntry) {
        last_entry_action_hand_slot_1 = 0;
    }
    out->lastEntryActionHandSlot1 = last_entry_action_hand_slot_1;

    /* Phase 3: all values fit in int16_t. */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0057[i] < 0 || s_g0057[i] > 32767) {
            all_values_in_byte_range = 0;
        }
    }
    out->allValuesInByteRange = all_values_in_byte_range;

    /* Phase 4: all values are distinct. */
    for (i = 0; i < kTableSize; ++i) {
        for (j = i + 1; j < kTableSize; ++j) {
            if (s_g0057[i] == s_g0057[j]) {
                all_values_distinct = 0;
            }
        }
    }
    out->allValuesDistinct = all_values_distinct;

    /* Phase 5: all backpack slots (13..29) appear in the table. */
    {
        int slot;
        for (slot = 13; slot <= 29; ++slot) {
            int found = 0;
            for (i = 0; i < kTableSize; ++i) {
                if (s_g0057[i] == slot) {
                    found = 1;
                    break;
                }
            }
            if (!found) {
                all_backpack_slots_covered = 0;
            }
        }
    }
    out->allBackpackSlotsCovered = all_backpack_slots_covered;

    /* Phase 6: table matches declared order. */
    {
        static const int kExpected[kTableSize] = {
            5, 4, 9, 8, 7, 12, 6, 11, 3,
            13, 14, 15, 16, 17, 18, 19, 20, 21,
            22, 23, 24, 25, 26, 27, 28, 29,
            10, 2, 0, 1
        };
        for (i = 0; i < kTableSize; ++i) {
            if (s_g0057[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 7: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_slot_drop_order_get_pc34(i) != s_g0057[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 8: out-of-range lookup returns -1. */
    if (dm1_v1_slot_drop_order_get_pc34(-1) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_slot_drop_order_get_pc34(kTableSize) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_slot_drop_order_get_pc34(999) != kIndexOOR) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntryFeetSlot5 &&
        out->lastEntryActionHandSlot1 &&
        out->allValuesInByteRange &&
        out->allValuesDistinct &&
        out->allBackpackSlotsCovered &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 9;
    return out->accepted;
}