#include "firestaff/dm1/v1/slot_masks_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:44  - declaration of G0038_ai_Graphic562_SlotMasks[38]
 * - DATA.C:320-358 - PC 3.4 init
 * - DATA.C:1049 - post-1.3 Atari init (same values)
 * - CHAMPION.C:697 - leader-hand object placement check
 * - REVIVE.C:307/310/338 - resurrect placement check
 * - DEFS.H MASK0xFFFF_ANY_SLOT/HEAD/TORSO/LEGS/FEET/NECK/QUIVER_xx
 *   POUCH_xx and CONTAINER constants
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791-799 (champion-panel/leader/mirror + chest), pass798/800/
 * 801/802/803/804/805/806/807/808 (Graphics.dat init-table gates
 * batches 1+2+3+4). This gate is a non-mirror-candidate contract
 * for the G0038 slot-mask table.
 */

enum {
    kTableSize        = 38,
    kReadyHandIdx     = 0,
    kActionHandIdx    = 1,
    kHeadIdx          = 2,
    kTorsoIdx         = 3,
    kLegsIdx          = 4,
    kFeetIdx          = 5,
    kNeckIdx          = 10,
    kQuiverLine2_1Idx = 7,
    kQuiverLine1_2Idx = 8,
    kQuiverLine2_2Idx = 9,
    kPouch1Idx        = 11,
    kPouch2Idx        = 6,
    kQuiverLine1_1Idx = 12,
    kBackpackLine1_1Idx = 13,
    kBackpackLastIdx  = 29,   /* backpack line1 row 9 */
    kChest1Idx        = 30,
    kChestLastIdx     = 37,

    kMaskAnySlot      = 0xFFFF,
    kMaskHead         = 0x0002,
    kMaskTorso        = 0x0008,
    kMaskLegs         = 0x0010,
    kMaskFeet         = 0x0020,
    kMaskNeck         = 0x0004,
    kMaskQuiverLine1  = 0x0040,
    kMaskQuiverLine2  = 0x0080,
    kMaskPouch        = 0x0100,
    kMaskContainer    = 0x0400,

    kOutOfRange       = 0
};

/* G0038 PC 3.4 init (DATA.C:320-358). 38 entries: 8 status-box
 * hands + 30 inventory slots (matching G0030's inventory partition
 * but starting at offset 8 in G0038) + 8 chest slots.
 *
 * Index 0..7: status-box hands
 *   0: Ready Hand
 *   1: Action Hand
 *   2: Head
 *   3: Torso
 *   4: Legs
 *   5: Feet
 *   6: Pouch 2 (in panel)
 *   7: Quiver Line2 1 (in panel)
 *
 * Index 8..37: inventory (30) + chest (8), with G0038 indices
 * matching the G0030 inventory+chest offset directly.
 */
static const int s_g0038[kTableSize] = {
    /* 0  Ready Hand */       kMaskAnySlot,
    /* 1  Action Hand */      kMaskAnySlot,
    /* 2  Head */             kMaskHead,
    /* 3  Torso */            kMaskTorso,
    /* 4  Legs */             kMaskLegs,
    /* 5  Feet */             kMaskFeet,
    /* 6  Pouch 2 */          kMaskPouch,
    /* 7  Quiver Line2 1 */   kMaskQuiverLine2,
    /* 8  Quiver Line1 2 */   kMaskQuiverLine2,
    /* 9  Quiver Line2 2 */   kMaskQuiverLine2,
    /* 10 Neck */             kMaskNeck,
    /* 11 Pouch 1 */          kMaskPouch,
    /* 12 Quiver Line1 1 */   kMaskQuiverLine1,
    /* 13 Backpack Line1 1 */ kMaskAnySlot,
    /* 14 Backpack Line2 2 */ kMaskAnySlot,
    /* 15 Backpack Line2 3 */ kMaskAnySlot,
    /* 16 Backpack Line2 4 */ kMaskAnySlot,
    /* 17 Backpack Line2 5 */ kMaskAnySlot,
    /* 18 Backpack Line2 6 */ kMaskAnySlot,
    /* 19 Backpack Line2 7 */ kMaskAnySlot,
    /* 20 Backpack Line2 8 */ kMaskAnySlot,
    /* 21 Backpack Line2 9 */ kMaskAnySlot,
    /* 22 Backpack Line1 2 */ kMaskAnySlot,
    /* 23 Backpack Line1 3 */ kMaskAnySlot,
    /* 24 Backpack Line1 4 */ kMaskAnySlot,
    /* 25 Backpack Line1 5 */ kMaskAnySlot,
    /* 26 Backpack Line1 6 */ kMaskAnySlot,
    /* 27 Backpack Line1 7 */ kMaskAnySlot,
    /* 28 Backpack Line1 8 */ kMaskAnySlot,
    /* 29 Backpack Line1 9 */ kMaskAnySlot,
    /* 30 Chest 1 */          kMaskContainer,
    /* 31 Chest 2 */          kMaskContainer,
    /* 32 Chest 3 */          kMaskContainer,
    /* 33 Chest 4 */          kMaskContainer,
    /* 34 Chest 5 */          kMaskContainer,
    /* 35 Chest 6 */          kMaskContainer,
    /* 36 Chest 7 */          kMaskContainer,
    /* 37 Chest 8 */          kMaskContainer
};

const int *
dm1_v1_slot_masks_table_pc34(void)
{
    return s_g0038;
}

int
dm1_v1_slot_masks_size_pc34(void)
{
    return kTableSize;
}

int
dm1_v1_slot_masks_pc34(int slot_index)
{
    if (slot_index < 0 || slot_index >= kTableSize) {
        return kOutOfRange;
    }
    return s_g0038[slot_index];
}

/* CHAMPION.C:697 / REVIVE.C:307 semantic:
 *   compatible = (slot_mask & G0038[slot_index]) != 0
 *
 * The caller passes the thing's AllowedSlots bitmask; if any of
 * those bits intersect G0038[slot_index]'s bitmask, the thing is
 * compatible with that slot.
 */
int
dm1_v1_slot_masks_is_compatible_pc34(int slot_mask, int slot_index)
{
    if (slot_index < 0 || slot_index >= kTableSize) {
        return 0;
    }
    return (slot_mask & s_g0038[slot_index]) ? 1 : 0;
}

int
dm1_v1_slot_masks_run_pc34(
    DM1_V1_SlotMasksResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int ready_hand_mask_is_any = 1;
    int action_hand_mask_is_any = 1;
    int body_part_masks_single_bit = 1;
    int neck_mask_is_neck = 1;
    int quiver_line1_mask_is_quiver_line1 = 1;
    int quiver_line2_mask_is_quiver_line2 = 1;
    int pouch_mask_is_pouch = 1;
    int backpack_masks_are_any = 1;
    int chest_masks_are_container = 1;
    int lookup_function_in_range = 1;
    int lookup_out_of_range_returns_zero = 1;
    static const int kExpected[kTableSize] = {
        kMaskAnySlot,   kMaskAnySlot,   kMaskHead,      kMaskTorso,
        kMaskLegs,      kMaskFeet,      kMaskPouch,     kMaskQuiverLine2,
        kMaskQuiverLine2, kMaskQuiverLine2, kMaskNeck,  kMaskPouch,
        kMaskQuiverLine1,
        kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,
        kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,
        kMaskAnySlot,
        kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,
        kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,   kMaskAnySlot,
        kMaskContainer, kMaskContainer, kMaskContainer, kMaskContainer,
        kMaskContainer, kMaskContainer, kMaskContainer, kMaskContainer
    };

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table + per-entry cross-check. */
    for (i = 0; i < kTableSize; ++i) {
        out->tableEntries[i] = s_g0038[i];
        if (s_g0038[i] != kExpected[i]) {
            table_matches_declaration = 0;
        }
    }
    out->tableSize = kTableSize;
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 2: ready hand + action hand are MASK0xFFFF_ANY_SLOT. */
    if (s_g0038[kReadyHandIdx]  != kMaskAnySlot) ready_hand_mask_is_any  = 0;
    if (s_g0038[kActionHandIdx] != kMaskAnySlot) action_hand_mask_is_any = 0;
    out->readyHandMaskIsAny  = ready_hand_mask_is_any;
    out->actionHandMaskIsAny = action_hand_mask_is_any;

    /* Phase 3: body part slots (Head/Torso/Legs/Feet) have a single
     * bit set (powers of 2: 0x0002, 0x0008, 0x0010, 0x0020).
     */
    if (s_g0038[kHeadIdx]  != kMaskHead)  body_part_masks_single_bit = 0;
    if (s_g0038[kTorsoIdx] != kMaskTorso) body_part_masks_single_bit = 0;
    if (s_g0038[kLegsIdx]  != kMaskLegs)  body_part_masks_single_bit = 0;
    if (s_g0038[kFeetIdx]  != kMaskFeet)  body_part_masks_single_bit = 0;
    out->bodyPartMasksSingleBit = body_part_masks_single_bit;

    /* Phase 4: neck slot mask = MASK0x0004_NECK. */
    if (s_g0038[kNeckIdx] != kMaskNeck) neck_mask_is_neck = 0;
    out->neckMaskIsNeck = neck_mask_is_neck;

    /* Phase 5: quiver line1/line2 masks. */
    if (s_g0038[kQuiverLine1_1Idx] != kMaskQuiverLine1) quiver_line1_mask_is_quiver_line1 = 0;
    if (s_g0038[kQuiverLine2_1Idx] != kMaskQuiverLine2) quiver_line2_mask_is_quiver_line2 = 0;
    out->quiverLine1MaskIsQuiverLine1 = quiver_line1_mask_is_quiver_line1;
    out->quiverLine2MaskIsQuiverLine2 = quiver_line2_mask_is_quiver_line2;

    /* Phase 6: pouch slots (Pouch 1 + Pouch 2). */
    if (s_g0038[kPouch1Idx] != kMaskPouch) pouch_mask_is_pouch = 0;
    if (s_g0038[kPouch2Idx] != kMaskPouch) pouch_mask_is_pouch = 0;
    out->pouchMaskIsPouch = pouch_mask_is_pouch;

    /* Phase 7: backpack slots (indices 13..29) all = MASK0xFFFF_ANY_SLOT. */
    for (i = kBackpackLine1_1Idx; i <= kBackpackLastIdx; ++i) {
        if (s_g0038[i] != kMaskAnySlot) {
            backpack_masks_are_any = 0;
        }
    }
    out->backpackMasksAreAny = backpack_masks_are_any;

    /* Phase 8: chest slots (indices 30..37) all = MASK0x0400_CONTAINER. */
    for (i = kChest1Idx; i <= kChestLastIdx; ++i) {
        if (s_g0038[i] != kMaskContainer) {
            chest_masks_are_container = 0;
        }
    }
    out->chestMasksAreContainer = chest_masks_are_container;

    /* Phase 9: lookup function correctness. */
    for (i = 0; i < kTableSize; ++i) {
        if (dm1_v1_slot_masks_pc34(i) != kExpected[i]) {
            lookup_function_in_range = 0;
        }
    }
    out->lookupFunctionInRange = lookup_function_in_range;

    /* Phase 10: out-of-range lookup returns 0. */
    if (dm1_v1_slot_masks_pc34(-1) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_slot_masks_pc34(38) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    if (dm1_v1_slot_masks_pc34(999) != 0) {
        lookup_out_of_range_returns_zero = 0;
    }
    out->lookupOutOfRangeReturnsZero = lookup_out_of_range_returns_zero;

    out->accepted =
        out->tableMatchesDeclaration &&
        out->readyHandMaskIsAny &&
        out->actionHandMaskIsAny &&
        out->bodyPartMasksSingleBit &&
        out->neckMaskIsNeck &&
        out->quiverLine1MaskIsQuiverLine1 &&
        out->quiverLine2MaskIsQuiverLine2 &&
        out->pouchMaskIsPouch &&
        out->backpackMasksAreAny &&
        out->chestMasksAreContainer &&
        out->lookupFunctionInRange &&
        out->lookupOutOfRangeReturnsZero;
    out->assertionCount = 13;
    return out->accepted;
}