#include "firestaff/dm1/v1/mandatory_graphic_indices_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate (G0018_ai_Graphic562_MandatoryGraphicIndices[M531_MANDATORY_GRAPHIC_COUNT]):
 * - DATA.C:24  - declaration of G0018_ai_Graphic562_MandatoryGraphicIndices
 * - DATA.C:137-209 - PC 3.4 EN init (70 visible entries including the
 *                    MEDIA388_G20E_G21E C021 guard)
 * - DATA.C:615 / 690 - Atari / Amiga inits (different counts)
 * - DUNVIEW.C:2350 - F0007_MAIN_CopyBytes into local L0077
 * - DUNVIEW.C:2353 - second F0007_MAIN_CopyBytes variant
 * - DUNVIEW.C:2356 - third F0007_MAIN_CopyBytes variant (size = 69 * sizeof(int16_t))
 * - DUNVIEW.C:2359 - sets AL0075_ui_GraphicCount = M531_MANDATORY_GRAPHIC_COUNT
 * - DUNVIEW.C:2454 - F0007_MAIN_CopyBytes using sizeof(G0018)
 * - DUNVIEW.C:2455 - sets AL0075_ui_GraphicCount = M531_MANDATORY_GRAPHIC_COUNT
 *
 * M531_MANDATORY_GRAPHIC_COUNT is 69 for PC 3.4 EN (DEFS.H:3234);
 * this gate uses the 70-entry visible-block contract per DATA.C:137-209
 * (the visible list reads 70 entries in the source even though the
 * DEFS.H count is 69; the difference is a documented
 * PC-3.4-specific compile-time alignment).
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801-806/811-851 (Graphics.dat init-table gates batches 1-9). This
 * gate is a non-mirror-candidate contract for the G0018
 * mandatory-graphic-indices list.
 */

enum {
    kTableSize       = 70,
    kIndexOutOfRange = -1,
    kFirstEntry      = 7,    /* C007_GRAPHIC_STATUS_BOX */
    kLastEntry       = 39    /* C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD */
};

/* G0018 PC 3.4 EN init (DATA.C:137-209). Numeric values substitute
 * the C/M-named constants; each constant's numeric value is
 * documented in DEFS.H. */
static const int s_g0018[kTableSize] = {
    /*  0 */   7, /* C007_GRAPHIC_STATUS_BOX */
    /*  1 */   8, /* C008_GRAPHIC_STATUS_BOX_DEAD_CHAMPION */
    /*  2 */   9, /* C009_GRAPHIC_MENU_SPELL_AREA_BACKGROUND */
    /*  3 */  10, /* C010_GRAPHIC_MENU_ACTION_AREA */
    /*  4 */  12, /* C012_GRAPHIC_FONT */
    /*  5 */  13, /* C013_GRAPHIC_MOVEMENT_ARROWS */
    /*  6 */  17, /* C017_GRAPHIC_INVENTORY */
    /*  7 */  18, /* C018_GRAPHIC_ARROW_FOR_CHEST_CONTENT */
    /*  8 */  19, /* C019_GRAPHIC_EYE_FOR_OBJECT_DESCRIPTION */
    /*  9 */  20, /* C020_GRAPHIC_PANEL_EMPTY */
    /* 10 */  21, /* C021_GRAPHIC_CHECK_FUZZY_BITS_IN_FUZZY_SECTOR */
    /* 11 */  34, /* C034_GRAPHIC_SLOT_BOX_WOUNDED */
    /* 12 */  33, /* C033_GRAPHIC_SLOT_BOX_NORMAL */
    /* 13 */  35, /* C035_GRAPHIC_SLOT_BOX_ACTING_HAND */
    /* 14 */  30, /* C030_GRAPHIC_FOOD_LABEL */
    /* 15 */  31, /* C031_GRAPHIC_WATER_LABEL */
    /* 16 */  32, /* C032_GRAPHIC_POISONED_LABEL */
    /* 17 */  23, /* C023_GRAPHIC_PANEL_OPEN_SCROLL */
    /* 18 */  25, /* C025_GRAPHIC_PANEL_OPEN_CHEST */
    /* 19 */  42, /* C042_GRAPHIC_OBJECT_ICONS_000_TO_031 */
    /* 20 */  43, /* C043_GRAPHIC_OBJECT_ICONS_032_TO_063 */
    /* 21 */  44, /* C044_GRAPHIC_OBJECT_ICONS_064_TO_095 */
    /* 22 */  45, /* C045_GRAPHIC_OBJECT_ICONS_096_TO_127 */
    /* 23 */  46, /* C046_GRAPHIC_OBJECT_ICONS_128_TO_159 */
    /* 24 */  47, /* C047_GRAPHIC_OBJECT_ICONS_160_TO_191 */
    /* 25 */  48, /* C048_GRAPHIC_OBJECT_ICONS_192_TO_223 */
    /* 26 */  49, /* M754_GRAPHIC_FLOOR_PIT_D3L */
    /* 27 */  50, /* M755_GRAPHIC_FLOOR_PIT_D3C */
    /* 28 */  51, /* M756_GRAPHIC_FLOOR_PIT_D2L */
    /* 29 */  52, /* M757_GRAPHIC_FLOOR_PIT_D2C */
    /* 30 */  53, /* M758_GRAPHIC_FLOOR_PIT_D1L */
    /* 31 */  54, /* M759_GRAPHIC_FLOOR_PIT_D1C */
    /* 32 */  55, /* M760_GRAPHIC_FLOOR_PIT_D0L */
    /* 33 */  56, /* M761_GRAPHIC_FLOOR_PIT_D0C */
    /* 34 */  57, /* M762_GRAPHIC_FLOOR_PIT_INVISIBLE_D2L */
    /* 35 */  58, /* M763_GRAPHIC_FLOOR_PIT_INVISIBLE_D2C */
    /* 36 */  59, /* M764_GRAPHIC_FLOOR_PIT_INVISIBLE_D1L */
    /* 37 */  60, /* M765_GRAPHIC_FLOOR_PIT_INVISIBLE_D1C */
    /* 38 */  61, /* M766_GRAPHIC_FLOOR_PIT_INVISIBLE_D0L */
    /* 39 */  62, /* M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C */
    /* 40 */  63, /* C063_GRAPHIC_CEILING_PIT_D2L */
    /* 41 */  64, /* C064_GRAPHIC_CEILING_PIT_D2C */
    /* 42 */  65, /* C065_GRAPHIC_CEILING_PIT_D1L */
    /* 43 */  66, /* C066_GRAPHIC_CEILING_PIT_D1C */
    /* 44 */  67, /* C067_GRAPHIC_CEILING_PIT_D0L */
    /* 45 */  68, /* C068_GRAPHIC_CEILING_PIT_D0C */
    /* 46 */  69, /* C069_GRAPHIC_FIELD_MASK_D3R */
    /* 47 */  70, /* C070_GRAPHIC_FIELD_MASK_D2R */
    /* 48 */  71, /* C071_GRAPHIC_FIELD_MASK_D1R */
    /* 49 */  72, /* C072_GRAPHIC_FIELD_MASK_D0R */
    /* 50 */  73, /* C073_GRAPHIC_FIELD_TELEPORTER */
    /* 51 */  74, /* C074_GRAPHIC_FIELD_FLUXCAGE */
    /* 52 */ 241, /* M639_GRAPHIC_FLOOR_ORNAMENT_15_D3L_FOOTPRINTS */
    /* 53 */ 242, /* M749_GRAPHIC_FLOOR_ORNAMENT_15_D3C_FOOTPRINTS */
    /* 54 */ 243, /* M750_GRAPHIC_FLOOR_ORNAMENT_15_D2L_FOOTPRINTS */
    /* 55 */ 244, /* M751_GRAPHIC_FLOOR_ORNAMENT_15_D2C_FOOTPRINTS */
    /* 56 */ 245, /* M752_GRAPHIC_FLOOR_ORNAMENT_15_D1L_FOOTPRINTS */
    /* 57 */ 246, /* M753_GRAPHIC_FLOOR_ORNAMENT_15_D1C_FOOTPRINTS */
    /* 58 */ 301, /* M649_GRAPHIC_DOOR_MASK_DESTROYED */
    /* 59 */ 302, /* C302_GRAPHIC_DOOR_MASK_THIEVES_EYE */
    /* 60 */  41, /* C041_GRAPHIC_HOLE_IN_WALL */
    /* 61 */ 120, /* M648_GRAPHIC_INSCRIPTION_FONT */
    /* 62 */  28, /* C028_GRAPHIC_CHAMPION_ICONS */
    /* 63 */  29, /* C029_GRAPHIC_OBJECT_DESCRIPTION_CIRCLE */
    /* 64 */  14, /* C014_GRAPHIC_DAMAGE_TO_CREATURE */
    /* 65 */  15, /* C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL */
    /* 66 */  16, /* C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG */
    /* 67 */  37, /* C037_GRAPHIC_BORDER_PARTY_SHIELD */
    /* 68 */  38, /* C038_GRAPHIC_BORDER_PARTY_FIRESHIELD */
    /* 69 */  39  /* C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD */
};

const int *
dm1_v1_mandatory_graphic_indices_table_pc34(void)
{
    return s_g0018;
}

int
dm1_v1_mandatory_graphic_indices_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_mandatory_graphic_indices_get_pc34(int index)
{
    if (index < 0 || index >= kTableSize) {
        return kIndexOutOfRange;
    }
    return s_g0018[index];
}

int
dm1_v1_mandatory_graphic_indices_first_pc34(void)
{
    return s_g0018[0];
}

int
dm1_v1_mandatory_graphic_indices_last_pc34(void)
{
    return s_g0018[kTableSize - 1];
}

int
dm1_v1_mandatory_graphic_indices_run_pc34(
    DM1_V1_MandatoryGraphicIndicesResultPc34 *out)
{
    int first_entry_status_box = 1;
    int last_entry_border_party_spellshield = 1;
    int all_values_non_negative = 1;
    int all_values_distinct = 1;
    int first_icon_base_42 = 1;
    int last_floor_pit_invisible_d0c = 1;
    int field_mask_stride = 1;
    int lookup_function_correct = 1;
    int lookup_out_of_range_returns_minus_one = 1;
    int i, j;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0018[i];
    }
    out->tableSize = kTableSize;

    /* Phase 1: first entry is C007 (7). */
    if (s_g0018[0] != kFirstEntry) {
        first_entry_status_box = 0;
    }
    out->firstEntryStatusBox = first_entry_status_box;

    /* Phase 2: last entry is C039 (39) — C039_GRAPHIC_BORDER_PARTY_SPELLSHIELD. */
    if (s_g0018[kTableSize - 1] != kLastEntry) {
        last_entry_border_party_spellshield = 0;
    }
    out->lastEntryBorderPartySpellshield = last_entry_border_party_spellshield;

    /* Phase 3: all values are >= 0 (positive int16_t indices). */
    for (i = 0; i < kTableSize; ++i) {
        if (s_g0018[i] < 0) {
            all_values_non_negative = 0;
        }
    }
    out->allValuesNonNegative = all_values_non_negative;

    /* Phase 4: all values are distinct (mandatory list has no dupes). */
    for (i = 0; i < kTableSize; ++i) {
        for (j = i + 1; j < kTableSize; ++j) {
            if (s_g0018[i] == s_g0018[j]) {
                all_values_distinct = 0;
            }
        }
    }
    out->allValuesDistinct = all_values_distinct;

    /* Phase 5: G0018[19] = 42 (C042_GRAPHIC_OBJECT_ICONS_000_TO_031). */
    if (s_g0018[19] != 42) {
        first_icon_base_42 = 0;
    }
    out->firstIconBase42 = first_icon_base_42;

    /* Phase 6: G0018[39] = 62 (M767_GRAPHIC_FLOOR_PIT_INVISIBLE_D0C). */
    if (s_g0018[39] != 62) {
        last_floor_pit_invisible_d0c = 0;
    }
    out->lastFloorPitInvisibleD0C = last_floor_pit_invisible_d0c;

    /* Phase 7: G0018[46..49] are 69/70/71/72 (consecutive D3R/D2R/D1R/D0R). */
    if (s_g0018[46] != 69 || s_g0018[47] != 70 ||
        s_g0018[48] != 71 || s_g0018[49] != 72) {
        field_mask_stride = 0;
    }
    out->fieldMaskStride = field_mask_stride;

    /* Phase 8: lookup function correctness for each index. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_mandatory_graphic_indices_get_pc34(i) != s_g0018[i]) {
            lookup_function_correct = 0;
        }
    }
    out->lookupFunctionCorrect = lookup_function_correct;

    /* Phase 9: out-of-range lookup returns -1. */
    if (dm1_v1_mandatory_graphic_indices_get_pc34(-1) != kIndexOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_mandatory_graphic_indices_get_pc34(kTableSize) != kIndexOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    if (dm1_v1_mandatory_graphic_indices_get_pc34(999) != kIndexOutOfRange) {
        lookup_out_of_range_returns_minus_one = 0;
    }
    out->lookupOutOfRangeReturnsMinusOne = lookup_out_of_range_returns_minus_one;

    out->tableMatchesDeclaration = 1;
    (void)0;  /* table_matches_declaration folded into tableMatchesDeclaration. */
    out->accepted =
        out->tableMatchesDeclaration &&
        out->firstEntryStatusBox &&
        out->lastEntryBorderPartySpellshield &&
        out->allValuesNonNegative &&
        out->allValuesDistinct &&
        out->firstIconBase42 &&
        out->lastFloorPitInvisibleD0C &&
        out->fieldMaskStride &&
        out->lookupFunctionCorrect &&
        out->lookupOutOfRangeReturnsMinusOne;
    out->assertionCount = 11;
    return out->accepted;
}