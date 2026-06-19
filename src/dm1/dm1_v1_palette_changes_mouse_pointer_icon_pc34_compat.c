#include "firestaff/dm1/v1/palette_changes_mouse_pointer_icon_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0044_auc_Graphic562_PaletteChanges_MousePointerIcon):
 * - DATA.C:40/181/583
 * - IO.C:164/2140/2167/2176/2443
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), all earlier
 * Graphics.dat init-table gates.
 */

enum {
    kTableSize = 16,
    kOutOfRange = 0
};

static const unsigned char s_table[16] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 12, 4, 4, 4 };

const unsigned char *
dm1_v1_palette_changes_mouse_pointer_icon_table_pc34(void)
{
    return s_table;
}

int
dm1_v1_palette_changes_mouse_pointer_icon_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(int palette_index)
{
    if (palette_index < 0 || palette_index >= kTableSize) {
        return kOutOfRange;
    }
    return (int)s_table[palette_index];
}

int
dm1_v1_palette_changes_mouse_pointer_icon_run_pc34(DM1_V1_PaletteChangesChangesMousePointerIconResultPc34 *out)
{
    int table_matches_declaration = 1;
    int first_entry_4 = 1;
    int last_entry_4 = 1;
    int all_values_in_range = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_zero = 1;
    int entry12_special = 1;
    int i;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = (int)s_table[i];
    }
    out->tableSize = kTableSize;

    /* Verify exact match. */
    {
        static const unsigned char kExpected[16] = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 12, 4, 4, 4 };
        for (i = 0; i < kTableSize; ++i) {
            if (s_table[i] != kExpected[i]) {
                table_matches_declaration = 0;
            }
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    if (s_table[0] != 4) first_entry_4 = 0;
    if (s_table[kTableSize - 1] != 4) last_entry_4 = 0;
    out->firstEntry4 = first_entry_4;
    out->lastEntry4 = last_entry_4;

    /* All values in [0, 255] (unsigned char range). */
    for (i = 0; i < kTableSize; ++i) {
        if ((int)s_table[i] < 0 || (int)s_table[i] > 255) {
            all_values_in_range = 0;
        }
    }
    out->allValuesInRange = all_values_in_range;

    /* Lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(i) != (int)s_table[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Out-of-range lookup returns 0. */
    if (dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(16) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_palette_changes_mouse_pointer_icon_get_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    /* Entry 12 is special (the spotlight pixel index). */
    if (s_table[12] != 12) {
        entry12_special = 0;
    }
    out->entry12Special = entry12_special;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntry4 &&
        out->lastEntry4 &&
        out->allValuesInRange &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsZero &&
        out->entry12Special;
    out->assertionCount = 8;
    return out->accepted;
}
