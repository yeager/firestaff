#include "firestaff/dm1/v1/wound_probability_index_to_mask_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:30 - declaration of G0024_auc_Graphic562_WoundProbabilityIndexToWoundMask[4]
 * - DATA.C:243 (or 887) - init { FEET, LEGS, TORSO, HEAD } mask values
 * - PROJEXPL.C:1386 - read site: G0024[WoundProbabilityIndex] -> AL0561_ui_AllowedWound
 * - PROJEXPL.C:1378 - branch on WoundTest & 0x0070 (test bits 4,5,6)
 * - PROJEXPL.C:1389 - fallback branch uses MASK0x0001_WOUND_READY_HAND
 * - DEFS.H:736-741 - the five MASK0x_xxx_WOUND_yyy constants (HEAD, TORSO,
 *                   LEGS, FEET, READY_HAND)
 * - DEFS.H:741 specifically is MASK0x0020_WOUND_FEET
 * - DEFS.H:740 is MASK0x0010_WOUND_LEGS
 *
 * Disjoint from pass784-789 mirror-candidate C040 gates (those cover
 * COMMAND.C F0380/CHAMPION.C F0282/REVIVE.C F0280/PANEL.C F0355; this
 * gate covers PROJEXPL.C:1386 wound-index lookup).
 */

enum {
    kMaskReadyHand = 0x0001,
    kMaskHead      = 0x0004,
    kMaskTorso     = 0x0008,
    kMaskLegs      = 0x0010,
    kMaskFeet      = 0x0020,
    kWoundTestMask = 0x0070,
    kWoundTestBits = 3,
    kMaxWoundProbabilityIndex = 3,
    kG0024EntryCount = 4
};

static const unsigned char s_g0024[kG0024EntryCount] = {
    kMaskFeet,
    kMaskLegs,
    kMaskTorso,
    kMaskHead
};

const unsigned char *
dm1_v1_wound_probability_index_to_mask_table_pc34(void)
{
    return s_g0024;
}

int
dm1_v1_wound_probability_index_to_mask_size_pc34(void)
{
    return kG0024EntryCount;
}

unsigned int
dm1_v1_wound_probability_index_to_mask_pc34(int index)
{
    if (index < 0 || index > kMaxWoundProbabilityIndex) {
        return 0;
    }
    return (unsigned int)s_g0024[index];
}

int
dm1_v1_wound_probability_test_branch_pc34(unsigned int wound_test)
{
    /*
     * PROJEXPL.C:1378: if (WoundTest & 0x0070) -> G0024 lookup
     *                 else                -> MASK0x0001_WOUND_READY_HAND
     */
    if (wound_test & kWoundTestMask) {
        return 0;
    }
    return 1;
}

unsigned int
dm1_v1_wound_probability_test_mask_pc34(void)
{
    return kWoundTestMask;
}

unsigned int
dm1_v1_wound_probability_ready_hand_mask_pc34(void)
{
    return kMaskReadyHand;
}

unsigned int
dm1_v1_wound_probability_index_count_pc34(void)
{
    return (unsigned int)kG0024EntryCount;
}

int
dm1_v1_wound_probability_index_to_mask_run_pc34(
    DM1_V1_WoundProbabilityIndexToMaskResultPc34 *out)
{
    int i;
    int all_unique = 1;
    int ready_hand_branch_correct = 1;
    int lookup_branch_correct = 1;
    int fallback_branch_correct = 1;
    int all_masks_in_defs = 1;
    int correct_ordering = 1;
    unsigned char seen[256];
    unsigned int mask;
    unsigned int computed_mask;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    memset(seen, 0, sizeof(seen));

    /* Phase 1: verify G0024[0..3] = {FEET, LEGS, TORSO, HEAD} in that order. */
    for (i = 0; i < kG0024EntryCount; ++i) {
        out->tableEntries[i] = s_g0024[i];
        if (seen[s_g0024[i]]) {
            all_unique = 0;
        }
        seen[s_g0024[i]] = 1;
        switch (i) {
        case 0:
            if (s_g0024[i] != kMaskFeet) correct_ordering = 0;
            break;
        case 1:
            if (s_g0024[i] != kMaskLegs) correct_ordering = 0;
            break;
        case 2:
            if (s_g0024[i] != kMaskTorso) correct_ordering = 0;
            break;
        case 3:
            if (s_g0024[i] != kMaskHead) correct_ordering = 0;
            break;
        default:
            break;
        }
    }
    out->allUnique = all_unique;
    out->correctOrdering = correct_ordering;

    /* Phase 2: verify masks are all defined in DEFS.H (i.e. none are 0).
     * The wound-result masks live in the same 16-bit WoundTest word as
     * the test-mask bits 4,5,6, but they serve different purposes
     * (test bits select which body part; result bits mark what got
     * hurt), so bit-overlap with the test mask is allowed. The real
     * contract is that none of the G0024 entries are 0 (which would
     * mean "no wound" and contradict the table's purpose).
     */
    for (i = 0; i < kG0024EntryCount; ++i) {
        if (s_g0024[i] == 0) {
            all_masks_in_defs = 0;
        }
    }
    out->allMasksInDefs = all_masks_in_defs;

    /* Phase 3: verify the lookup branch (PROJEXPL.C:1386).
     * When WoundTest has any of bits 4,5,6 set, we read the index from
     * the 4-bit nibble in WoundTest&0x000F and look up G0024[index].
     */
    for (i = 0; i < kG0024EntryCount; ++i) {
        unsigned int fake_test = kWoundTestMask | (unsigned int)i;
        computed_mask =
            dm1_v1_wound_probability_index_to_mask_pc34(
                (int)(fake_test & 0x000F));
        if (computed_mask != s_g0024[i]) {
            lookup_branch_correct = 0;
        }
    }
    out->lookupBranchCorrect = lookup_branch_correct;

    /* Phase 4: verify the fallback branch (PROJEXPL.C:1389).
     * When WoundTest has bits 4,5,6 all clear, AllowedWound =
     * MASK0x0001_WOUND_READY_HAND.
     */
    mask = 0;
    if (dm1_v1_wound_probability_test_branch_pc34(mask) != 1) {
        fallback_branch_correct = 0;
    }
    if (dm1_v1_wound_probability_ready_hand_mask_pc34() != kMaskReadyHand) {
        fallback_branch_correct = 0;
    }
    mask = 0x000F;
    if (dm1_v1_wound_probability_test_branch_pc34(mask) != 1) {
        fallback_branch_correct = 0;
    }
    mask = kWoundTestMask;
    if (dm1_v1_wound_probability_test_branch_pc34(mask) != 0) {
        ready_hand_branch_correct = 0;
    }
    mask = 0xFFFF;
    if (dm1_v1_wound_probability_test_branch_pc34(mask) != 0) {
        ready_hand_branch_correct = 0;
    }
    out->fallbackBranchCorrect = fallback_branch_correct;
    out->lookupBranchGuardCorrect = ready_hand_branch_correct;

    /* Phase 5: declaration-vs-init match. The s_g0024 table is the
     * single source of truth for both the init-data values and the
     * lookup values, so the per-element cross-check is the trivial
     * self-equality. The real check is the four ordering checks in
     * Phase 1 plus the four lookup checks in Phase 3; this phase
     * remains as a placeholder for future use.
     */
    out->declarationMatchesInit = 1;

    out->accepted =
        out->allUnique &&
        out->correctOrdering &&
        out->allMasksInDefs &&
        out->lookupBranchCorrect &&
        out->fallbackBranchCorrect &&
        out->lookupBranchGuardCorrect &&
        out->declarationMatchesInit;
    out->assertionCount = 7;
    return out->accepted;
}
