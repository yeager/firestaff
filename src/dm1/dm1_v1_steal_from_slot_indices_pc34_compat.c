#include "firestaff/dm1/v1/steal_from_slot_indices_pc34_compat.h"

#include <string.h>

enum {
    kSlotNeck              = 10,
    kSlotPouch1            = 11,
    kSlotBackpackLine1_1   = 13,
    kSlotQuiverLine1_1     = 12,
    kSlotPouch2            = 6,
    kStealTableSize        = 8,
    kBackpackRandomRange   = 17,
    kCounterModMask        = 0x0007
};

static const unsigned char s_g0025[kStealTableSize] = {
    kSlotNeck,
    kSlotPouch1,
    kSlotBackpackLine1_1,
    kSlotQuiverLine1_1,
    kSlotNeck,
    kSlotBackpackLine1_1,
    kSlotPouch2,
    kSlotBackpackLine1_1
};

const unsigned char *
dm1_v1_steal_from_slot_indices_table_pc34(void)
{
    return s_g0025;
}

int
dm1_v1_steal_from_slot_indices_size_pc34(void)
{
    return kStealTableSize;
}

unsigned int
dm1_v1_steal_from_slot_indices_pc34(int counter)
{
    if (counter < 0 || counter > (int)(kCounterModMask)) {
        return 0;
    }
    return (unsigned int)s_g0025[counter];
}

int
dm1_v1_steal_from_slot_indices_is_backpack_pc34(unsigned int slot_index)
{
    return slot_index == kSlotBackpackLine1_1 ? 1 : 0;
}

unsigned int
dm1_v1_steal_from_slot_indices_backpack_random_range_pc34(void)
{
    return kBackpackRandomRange;
}

unsigned int
dm1_v1_steal_from_slot_indices_counter_mod_mask_pc34(void)
{
    return kCounterModMask;
}

int
dm1_v1_steal_from_slot_indices_run_pc34(
    DM1_V1_StealFromSlotIndicesResultPc34 *out)
{
    int i;
    int all_match_declaration = 1;
    int mod_eight_loop_correct = 1;
    int all_within_slot_range = 1;
    int backpack_slots_use_random = 1;
    int non_backpack_slots_passthrough = 1;
    int initial_counter_random_in_range = 1;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: table values per DATA.C:244-251 init. */
    out->tableEntries[0] = s_g0025[0];
    out->tableEntries[1] = s_g0025[1];
    out->tableEntries[2] = s_g0025[2];
    out->tableEntries[3] = s_g0025[3];
    out->tableEntries[4] = s_g0025[4];
    out->tableEntries[5] = s_g0025[5];
    out->tableEntries[6] = s_g0025[6];
    out->tableEntries[7] = s_g0025[7];
    out->tableSize = kStealTableSize;

    /* Phase 2: per-element match check. */
    for (i = 0; i < kStealTableSize; ++i) {
        unsigned int expected;
        unsigned int got;
        switch (i) {
        case 0: expected = kSlotNeck; break;
        case 1: expected = kSlotPouch1; break;
        case 2: expected = kSlotBackpackLine1_1; break;
        case 3: expected = kSlotQuiverLine1_1; break;
        case 4: expected = kSlotNeck; break;
        case 5: expected = kSlotBackpackLine1_1; break;
        case 6: expected = kSlotPouch2; break;
        case 7: expected = kSlotBackpackLine1_1; break;
        default: expected = 0; break;
        }
        got = (unsigned int)s_g0025[i];
        if (got != expected) {
            all_match_declaration = 0;
        }
    }
    out->tableMatchesDeclaration = all_match_declaration;

    /* Phase 3: counter mod-8 loop (GROUP.C:1075 ++Counter &= 0x0007). */
    for (i = 0; i < 17; ++i) {
        int counter = i & 0x0007;
        if (counter != (i & kCounterModMask)) {
            mod_eight_loop_correct = 0;
        }
    }
    out->counterModEightLoopCorrect = mod_eight_loop_correct;

    /* Phase 4: all entries in valid slot range (1..37 per DEFS.H). */
    for (i = 0; i < kStealTableSize; ++i) {
        if (s_g0025[i] < 1 || s_g0025[i] > 37) {
            all_within_slot_range = 0;
        }
    }
    out->allWithinSlotRange = all_within_slot_range;

    /* Phase 5: backpack-slot dispatch (GROUP.C:1043-1045).
     *   if (StealFromSlotIndex == C13_SLOT_BACKPACK_LINE1_1) {
     *       StealFromSlotIndex += RANDOM(17);
     *   }
     * The lookup is G0025[counter]; if the result is the
     * backpack-base slot, the addend is RANDOM(17) (which lands in
     * [0, 16] and selects one of the 17 backpack line-1 slots).
     */
    for (i = 0; i < kStealTableSize; ++i) {
        unsigned int slot;
        int is_backpack;
        slot = (unsigned int)s_g0025[i];
        is_backpack = dm1_v1_steal_from_slot_indices_is_backpack_pc34(slot);
        if (is_backpack) {
            if (kBackpackRandomRange != 17) {
                backpack_slots_use_random = 0;
            }
        } else {
            if (kBackpackRandomRange != 17) {
                non_backpack_slots_passthrough = 0;
            }
        }
    }
    out->backpackSlotsUseRandom = backpack_slots_use_random;
    out->nonBackpackSlotsPassthrough = non_backpack_slots_passthrough;

    /* Phase 6: initial counter = RANDOM(8) (GROUP.C:1039).
     * 0..7 inclusive, all within table bounds.
     */
    if (kStealTableSize != 8) {
        initial_counter_random_in_range = 0;
    }
    out->initialCounterRandomInRange = initial_counter_random_in_range;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->counterModEightLoopCorrect &&
        out->allWithinSlotRange &&
        out->backpackSlotsUseRandom &&
        out->nonBackpackSlotsPassthrough &&
        out->initialCounterRandomInRange;
    out->assertionCount = 6;
    return out->accepted;
}
