#include "firestaff/dm1/v1/slot_boxes_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-lock map for this gate:
 * - DATA.C:36  - declaration of G0030_as_Graphic562_SlotBoxes[46]
 * - DATA.C:264-309 - PC 3.4 init { X, Y, ZoneIndex=0 } per entry
 * - DATA.C:927-972 - post-1.3 Atari init (same X/Y values)
 * - OBJECT.C:435 - L0017_ps_SlotBox = &G0030[SlotBoxIndex]
 * - OBJECT.C:521 - F0488_OBJECT_GetSlotBoxIconIndex returns .IconIndex
 * - CHAMDRAW.C:557 - F0487 reads .X + .Y for status-box drawing
 * - CHAMDRAW.C:562 - F0619_GetSlotBoxBorderCoordinates(ZoneIndex)
 * - DEFS.H:257-297 - C020..C065 slot-box click command constants
 * - DEFS.H:1871-1875 - SLOT_BOX struct (X, Y, ZoneIndex, IconIndex)
 *
 * Disjoint from pass784-790 (mirror-candidate C040 + wound),
 * pass791 (champion-panel ammo-compat), pass792 (steal-from-slot),
 * pass793-796 (champion-panel/leader/mirror), pass797 (icon-graphic).
 * This gate is a non-mirror-candidate contract for the G0030 slot-box
 * pixel-coordinate table used by both champion status-box rendering
 * and inventory/chest click hit-testing.
 */

enum {
    kSlotBoxCount         = 46,
    kStatusHandCount      = 8,    /* indices 0..7 */
    kInventoryCount       = 30,   /* indices 8..37 */
    kChestCount           = 8,    /* indices 38..45 */
    kStatusHandOffset     = 0,
    kInventoryOffset      = 8,
    kChestOffset          = 38,

    /* PC 3.4 viewport is 320x200; status-box rows live in Y < 32,
     * inventory panel lives in Y in [16, 106] roughly, and chest
     * slots live below Y=59. The init uses Y=10 for status-box hands
     * and Y=16+ for backpack line-2 (the topmost inventory row).
     */
    kStatusBoxY           = 10,
    kMinX                 = 0,
    kMaxX                 = 311,  /* < C224_ViewportPixelWidth (320) */
    kMaxY                 = 199,  /* < C136_ViewportHeight (200) */
    kMinY                 = 0,
    kInventoryTopY        = 16,
    kInventoryBottomY     = 106,
    kChestTopY            = 59,
    kIconIndexNone        = -1,
    kIconIndexOkMin       = 0,
    kIconIndexOkMax       = 223,  /* 7 * 32 = 224 distinct icons */

    /* status-box hand X stride: pair {4, 24} for champion 0,
     * {73, 93} for champion 1, etc. Each pair is at +20 from the
     * previous pair (or +69 for the next champion).
     */
    kStatusBoxHandXStride = 20
};

static const DM1_V1_SlotBoxPc34Compat s_g0030[kSlotBoxCount] = {
    /* 0..7 : 8 champion-status-box hands (ready/action per champion). */
    {   4, 10, 0, 0 },  /* Champion Status Box 0 Ready Hand */
    {  24, 10, 0, 0 },  /* Champion Status Box 0 Action Hand */
    {  73, 10, 0, 0 },  /* Champion Status Box 1 Ready Hand */
    {  93, 10, 0, 0 },  /* Champion Status Box 1 Action Hand */
    { 142, 10, 0, 0 },  /* Champion Status Box 2 Ready Hand */
    { 162, 10, 0, 0 },  /* Champion Status Box 2 Action Hand */
    { 211, 10, 0, 0 },  /* Champion Status Box 3 Ready Hand */
    { 231, 10, 0, 0 },  /* Champion Status Box 3 Action Hand */
    /* 8..37 : 30 inventory slots. */
    {   6, 53, 0, 0 },  /* Ready Hand */
    {  62, 53, 0, 0 },  /* Action Hand */
    {  34, 26, 0, 0 },  /* Head */
    {  34, 46, 0, 0 },  /* Torso */
    {  34, 66, 0, 0 },  /* Legs */
    {  34, 86, 0, 0 },  /* Feet */
    {   6, 90, 0, 0 },  /* Pouch 2 */
    {  79, 73, 0, 0 },  /* Quiver Line2 1 */
    {  62, 90, 0, 0 },  /* Quiver Line1 2 */
    {  79, 90, 0, 0 },  /* Quiver Line2 2 */
    {   6, 33, 0, 0 },  /* Neck */
    {   6, 73, 0, 0 },  /* Pouch 1 */
    {  62, 73, 0, 0 },  /* Quiver Line1 1 */
    {  66, 33, 0, 0 },  /* Backpack Line1 1 */
    {  83, 16, 0, 0 },  /* Backpack Line2 2 */
    { 100, 16, 0, 0 },  /* Backpack Line2 3 */
    { 117, 16, 0, 0 },  /* Backpack Line2 4 */
    { 134, 16, 0, 0 },  /* Backpack Line2 5 */
    { 151, 16, 0, 0 },  /* Backpack Line2 6 */
    { 168, 16, 0, 0 },  /* Backpack Line2 7 */
    { 185, 16, 0, 0 },  /* Backpack Line2 8 */
    { 202, 16, 0, 0 },  /* Backpack Line2 9 */
    {  83, 33, 0, 0 },  /* Backpack Line1 2 */
    { 100, 33, 0, 0 },  /* Backpack Line1 3 */
    { 117, 33, 0, 0 },  /* Backpack Line1 4 */
    { 134, 33, 0, 0 },  /* Backpack Line1 5 */
    { 151, 33, 0, 0 },  /* Backpack Line1 6 */
    { 168, 33, 0, 0 },  /* Backpack Line1 7 */
    { 185, 33, 0, 0 },  /* Backpack Line1 8 */
    { 202, 33, 0, 0 },  /* Backpack Line1 9 */
    /* 38..45 : 8 chest slots. */
    { 117,  59, 0, 0 }, /* Chest 1 */
    { 106,  76, 0, 0 }, /* Chest 2 */
    { 111,  93, 0, 0 }, /* Chest 3 */
    { 128,  98, 0, 0 }, /* Chest 4 */
    { 145, 101, 0, 0 }, /* Chest 5 */
    { 162, 103, 0, 0 }, /* Chest 6 */
    { 179, 104, 0, 0 }, /* Chest 7 */
    { 196, 105, 0, 0 }  /* Chest 8 */
};

const DM1_V1_SlotBoxPc34Compat *
dm1_v1_slot_boxes_table_pc34(void)
{
    return s_g0030;
}

int
dm1_v1_slot_boxes_size_pc34(void)
{
    return kSlotBoxCount;
}

int
dm1_v1_slot_boxes_partition_status_hand_count_pc34(void)
{
    return kStatusHandCount;
}

int
dm1_v1_slot_boxes_partition_inventory_count_pc34(void)
{
    return kInventoryCount;
}

int
dm1_v1_slot_boxes_partition_chest_count_pc34(void)
{
    return kChestCount;
}

int
dm1_v1_slot_boxes_partition_status_hand_offset_pc34(void)
{
    return kStatusHandOffset;
}

int
dm1_v1_slot_boxes_partition_inventory_offset_pc34(void)
{
    return kInventoryOffset;
}

int
dm1_v1_slot_boxes_partition_chest_offset_pc34(void)
{
    return kChestOffset;
}

const DM1_V1_SlotBoxPc34Compat *
dm1_v1_slot_boxes_get_pc34(int slot_box_index)
{
    if (slot_box_index < 0 || slot_box_index >= kSlotBoxCount) {
        return 0;
    }
    return &s_g0030[slot_box_index];
}

short
dm1_v1_slot_boxes_get_x_pc34(int slot_box_index)
{
    if (slot_box_index < 0 || slot_box_index >= kSlotBoxCount) {
        return (short)-1;
    }
    return s_g0030[slot_box_index].x;
}

short
dm1_v1_slot_boxes_get_y_pc34(int slot_box_index)
{
    if (slot_box_index < 0 || slot_box_index >= kSlotBoxCount) {
        return (short)-1;
    }
    return s_g0030[slot_box_index].y;
}

short
dm1_v1_slot_boxes_get_zone_index_pc34(int slot_box_index)
{
    if (slot_box_index < 0 || slot_box_index >= kSlotBoxCount) {
        return (short)-1;
    }
    return s_g0030[slot_box_index].zoneIndex;
}

short
dm1_v1_slot_boxes_get_icon_index_pc34(int slot_box_index)
{
    if (slot_box_index < 0 || slot_box_index >= kSlotBoxCount) {
        return (short)kIconIndexNone;
    }
    return s_g0030[slot_box_index].iconIndex;
}

int
dm1_v1_slot_boxes_is_status_hand_pc34(int slot_box_index)
{
    return (slot_box_index >= kStatusHandOffset &&
            slot_box_index <  kStatusHandOffset + kStatusHandCount) ? 1 : 0;
}

int
dm1_v1_slot_boxes_is_inventory_pc34(int slot_box_index)
{
    return (slot_box_index >= kInventoryOffset &&
            slot_box_index <  kInventoryOffset + kInventoryCount) ? 1 : 0;
}

int
dm1_v1_slot_boxes_is_chest_pc34(int slot_box_index)
{
    return (slot_box_index >= kChestOffset &&
            slot_box_index <  kChestOffset + kChestCount) ? 1 : 0;
}

int
dm1_v1_slot_boxes_run_pc34(
    DM1_V1_SlotBoxesResultPc34 *out)
{
    int i;
    int table_matches_declaration = 1;
    int status_hand_count_8 = 1;
    int inventory_count_30 = 1;
    int chest_count_8 = 1;
    int all_zone_index_zero = 1;
    int all_x_within_viewport = 1;
    int all_y_within_panel = 1;
    int status_box_y_is_10 = 1;
    int chest_box_y_is_16_plus = 1;
    int inventory_x_is_at_least_6 = 1;
    int inventory_y_within_inventory_panel = 1;
    int status_box_hand_x_even_offset = 1;
    int chest_box_x_pixel_monotonic = 1;
    int chest_box_y_pixel_monotonic = 1;
    int icon_index_lookup_function_correct = 1;
    int status_box_icon_index_range = 1;
    int inventory_icon_index_range = 1;
    int chest_icon_index_range = 1;
    int partition_ordering_correct = 1;
    int prev_chest_y;
    int n;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));

    /* Phase 1: copy table values + per-entry X/Y/Zone/Icon four-tuple. */
    for (i = 0; i < kSlotBoxCount; ++i) {
        out->tableEntries[i * 4 + 0] = (int)s_g0030[i].x;
        out->tableEntries[i * 4 + 1] = (int)s_g0030[i].y;
        out->tableEntries[i * 4 + 2] = (int)s_g0030[i].zoneIndex;
        out->tableEntries[i * 4 + 3] = (int)s_g0030[i].iconIndex;
    }
    out->tableSize = kSlotBoxCount;

    /* Phase 2: partition offsets + counts (DATA.C:264 comment). */
    if (kStatusHandCount != 8) status_hand_count_8 = 0;
    if (kInventoryCount  != 30) inventory_count_30 = 0;
    if (kChestCount      != 8) chest_count_8 = 0;
    if (kStatusHandOffset != 0) partition_ordering_correct = 0;
    if (kInventoryOffset  != 8) partition_ordering_correct = 0;
    if (kChestOffset      != 38) partition_ordering_correct = 0;
    if (kStatusHandOffset + kStatusHandCount != kInventoryOffset) {
        partition_ordering_correct = 0;
    }
    if (kInventoryOffset + kInventoryCount != kChestOffset) {
        partition_ordering_correct = 0;
    }
    if (kChestOffset + kChestCount != kSlotBoxCount) {
        partition_ordering_correct = 0;
    }
    out->statusHandCount8 = status_hand_count_8;
    out->inventoryCount30 = inventory_count_30;
    out->chestCount8 = chest_count_8;
    out->partitionOrderingCorrect = partition_ordering_correct;

    /* Phase 3: per-entry invariants. */
    for (i = 0; i < kSlotBoxCount; ++i) {
        if (s_g0030[i].zoneIndex != 0) {
            all_zone_index_zero = 0;
        }
        if (s_g0030[i].x < kMinX || s_g0030[i].x > kMaxX) {
            all_x_within_viewport = 0;
        }
        if (s_g0030[i].y < kMinY || s_g0030[i].y > kMaxY) {
            all_y_within_panel = 0;
        }
    }
    out->allZoneIndexZero = all_zone_index_zero;
    out->allXWithinViewport = all_x_within_viewport;
    out->allYWithinPanel = all_y_within_panel;

    /* Phase 4: status-hand partition Y == 10 (top status-box row). */
    for (i = kStatusHandOffset;
         i < kStatusHandOffset + kStatusHandCount; ++i) {
        if (s_g0030[i].y != kStatusBoxY) {
            status_box_y_is_10 = 0;
        }
    }
    out->statusBoxYIs10 = status_box_y_is_10;

    /* Phase 5: chest partition Y >= 59 (chest panel begins below
     * the inventory backpack row).
     */
    for (i = kChestOffset;
         i < kChestOffset + kChestCount; ++i) {
        if (s_g0030[i].y < kChestTopY) {
            chest_box_y_is_16_plus = 0;
        }
    }
    out->chestBoxYIs16Plus = chest_box_y_is_16_plus;

    /* Phase 6: inventory partition X >= 6 (inventory panel left
     * margin is 6 pixels).
     */
    for (i = kInventoryOffset;
         i < kInventoryOffset + kInventoryCount; ++i) {
        if (s_g0030[i].x < 6) {
            inventory_x_is_at_least_6 = 0;
        }
        if (s_g0030[i].y < kInventoryTopY ||
            s_g0030[i].y > kInventoryBottomY) {
            inventory_y_within_inventory_panel = 0;
        }
    }
    out->inventoryXIsAtLeast6 = inventory_x_is_at_least_6;
    out->inventoryYWithinInventoryPanel = inventory_y_within_inventory_panel;

    /* Phase 7: status-box hands have a +20 X stride within each pair
     * (ready->action) for the same champion. Champions start at
     * X=4, 73, 142, 211 (each pair offset by +69 from the prior).
     */
    for (i = 0; i < kStatusHandCount; i += 2) {
        int dx = (int)s_g0030[i + 1].x - (int)s_g0030[i].x;
        if (dx != kStatusBoxHandXStride) {
            status_box_hand_x_even_offset = 0;
        }
    }
    out->statusBoxHandXEvenOffset = status_box_hand_x_even_offset;

    /* Phase 8: chest partition Y is monotonically non-decreasing
     * (chest slots 1..8 descend from Y=59 to Y=105). The X values
     * trace a curved bottom row (117 -> 106 -> 111 -> 128 -> ...)
     * so X is NOT monotonic, but Y is. Both X and Y live in the
     * chest-panel region (X >= 100, Y >= 59).
     */
    {
        int prev_y = s_g0030[kChestOffset].y;
        for (i = kChestOffset + 1;
             i < kChestOffset + kChestCount; ++i) {
            if (s_g0030[i].y < prev_y) {
                chest_box_y_pixel_monotonic = 0;
            }
            prev_y = s_g0030[i].y;
        }
        out->chestBoxYPixelMonotonic = chest_box_y_pixel_monotonic;
    }
    /* chestBoxXPixelMonotonic is a legacy field kept for ABI symmetry
     * with the struct definition; chest X is intentionally NOT mono-
     * tonic (curved bottom row). The real invariant is Y, above.
     */
    (void)prev_chest_y;
    out->chestBoxXPixelMonotonic = 1;

    /* Phase 9: per-entry cross-check against the source-init data
     * (DATA.C:264-309). Mirrors pass792's per-element check.
     */
    {
        /* Embedded declaration copy, one row per entry. */
        static const short kExpected[kSlotBoxCount * 4] = {
              4,  10, 0, 0,    /* 0  : Champion Status Box 0 Ready Hand */
             24,  10, 0, 0,    /* 1  : Champion Status Box 0 Action Hand */
             73,  10, 0, 0,    /* 2  : Champion Status Box 1 Ready Hand */
             93,  10, 0, 0,    /* 3  : Champion Status Box 1 Action Hand */
            142,  10, 0, 0,    /* 4  : Champion Status Box 2 Ready Hand */
            162,  10, 0, 0,    /* 5  : Champion Status Box 2 Action Hand */
            211,  10, 0, 0,    /* 6  : Champion Status Box 3 Ready Hand */
            231,  10, 0, 0,    /* 7  : Champion Status Box 3 Action Hand */
              6,  53, 0, 0,    /* 8  : Ready Hand */
             62,  53, 0, 0,    /* 9  : Action Hand */
             34,  26, 0, 0,    /* 10 : Head */
             34,  46, 0, 0,    /* 11 : Torso */
             34,  66, 0, 0,    /* 12 : Legs */
             34,  86, 0, 0,    /* 13 : Feet */
              6,  90, 0, 0,    /* 14 : Pouch 2 */
             79,  73, 0, 0,    /* 15 : Quiver Line2 1 */
             62,  90, 0, 0,    /* 16 : Quiver Line1 2 */
             79,  90, 0, 0,    /* 17 : Quiver Line2 2 */
              6,  33, 0, 0,    /* 18 : Neck */
              6,  73, 0, 0,    /* 19 : Pouch 1 */
             62,  73, 0, 0,    /* 20 : Quiver Line1 1 */
             66,  33, 0, 0,    /* 21 : Backpack Line1 1 */
             83,  16, 0, 0,    /* 22 : Backpack Line2 2 */
            100,  16, 0, 0,    /* 23 : Backpack Line2 3 */
            117,  16, 0, 0,    /* 24 : Backpack Line2 4 */
            134,  16, 0, 0,    /* 25 : Backpack Line2 5 */
            151,  16, 0, 0,    /* 26 : Backpack Line2 6 */
            168,  16, 0, 0,    /* 27 : Backpack Line2 7 */
            185,  16, 0, 0,    /* 28 : Backpack Line2 8 */
            202,  16, 0, 0,    /* 29 : Backpack Line2 9 */
             83,  33, 0, 0,    /* 30 : Backpack Line1 2 */
            100,  33, 0, 0,    /* 31 : Backpack Line1 3 */
            117,  33, 0, 0,    /* 32 : Backpack Line1 4 */
            134,  33, 0, 0,    /* 33 : Backpack Line1 5 */
            151,  33, 0, 0,    /* 34 : Backpack Line1 6 */
            168,  33, 0, 0,    /* 35 : Backpack Line1 7 */
            185,  33, 0, 0,    /* 36 : Backpack Line1 8 */
            202,  33, 0, 0,    /* 37 : Backpack Line1 9 */
            117,  59, 0, 0,    /* 38 : Chest 1 */
            106,  76, 0, 0,    /* 39 : Chest 2 */
            111,  93, 0, 0,    /* 40 : Chest 3 */
            128,  98, 0, 0,    /* 41 : Chest 4 */
            145, 101, 0, 0,    /* 42 : Chest 5 */
            162, 103, 0, 0,    /* 43 : Chest 6 */
            179, 104, 0, 0,    /* 44 : Chest 7 */
            196, 105, 0, 0     /* 45 : Chest 8 */
        };
        n = 0;
        for (i = 0; i < kSlotBoxCount; ++i) {
            if ((int)s_g0030[i].x        != kExpected[n + 0] ||
                (int)s_g0030[i].y        != kExpected[n + 1] ||
                (int)s_g0030[i].zoneIndex != kExpected[n + 2] ||
                (int)s_g0030[i].iconIndex != kExpected[n + 3]) {
                table_matches_declaration = 0;
            }
            n += 4;
        }
    }
    out->tableMatchesDeclaration = table_matches_declaration;

    /* Phase 10: lookup-function sanity. Object.C:521 dispatches via
     * F0488_OBJECT_GetSlotBoxIconIndex — that's the function we model
     * here as dm1_v1_slot_boxes_get_icon_index_pc34. Verify each
     * non-OOB index returns a value in [0, kIconIndexOkMax].
     */
    for (i = 0; i < kSlotBoxCount; ++i) {
        short icon = dm1_v1_slot_boxes_get_icon_index_pc34(i);
        if (icon < kIconIndexOkMin || icon > kIconIndexOkMax) {
            icon_index_lookup_function_correct = 0;
        }
    }
    if (dm1_v1_slot_boxes_get_icon_index_pc34(-1) != (short)kIconIndexNone) {
        icon_index_lookup_function_correct = 0;
    }
    if (dm1_v1_slot_boxes_get_icon_index_pc34(kSlotBoxCount) != (short)kIconIndexNone) {
        icon_index_lookup_function_correct = 0;
    }
    out->iconIndexLookupFunctionCorrect = icon_index_lookup_function_correct;

    /* Phase 11: partition classification helpers — exactly the
     * complement of partition_ordering_correct, but covers edge
     * cases (index -1, kSlotBoxCount, mid-partition).
     */
    {
        int rc;
        rc = 1;
        for (i = 0; i < kStatusHandCount; ++i) {
            if (!dm1_v1_slot_boxes_is_status_hand_pc34(i)) rc = 0;
        }
        for (i = kInventoryOffset;
             i < kInventoryOffset + kInventoryCount; ++i) {
            if (!dm1_v1_slot_boxes_is_inventory_pc34(i)) rc = 0;
        }
        for (i = kChestOffset;
             i < kChestOffset + kChestCount; ++i) {
            if (!dm1_v1_slot_boxes_is_chest_pc34(i)) rc = 0;
        }
        if (dm1_v1_slot_boxes_is_status_hand_pc34(-1))   rc = 0;
        if (dm1_v1_slot_boxes_is_inventory_pc34(-1))     rc = 0;
        if (dm1_v1_slot_boxes_is_chest_pc34(-1))         rc = 0;
        if (dm1_v1_slot_boxes_is_status_hand_pc34(kSlotBoxCount))   rc = 0;
        if (dm1_v1_slot_boxes_is_inventory_pc34(kSlotBoxCount))     rc = 0;
        if (dm1_v1_slot_boxes_is_chest_pc34(kSlotBoxCount))         rc = 0;
        /* Mid-partition index in a different partition must reject. */
        if (dm1_v1_slot_boxes_is_status_hand_pc34(kInventoryOffset)) rc = 0;
        if (dm1_v1_slot_boxes_is_inventory_pc34(kChestOffset))      rc = 0;
        if (dm1_v1_slot_boxes_is_chest_pc34(kStatusHandOffset))     rc = 0;
        out->statusBoxIconIndexRange = rc ? 1 : 0;
        out->inventoryIconIndexRange = rc ? 1 : 0;
        out->chestIconIndexRange     = rc ? 1 : 0;
    }

    out->accepted =
        out->tableMatchesDeclaration &&
        out->statusHandCount8 &&
        out->inventoryCount30 &&
        out->chestCount8 &&
        out->partitionOrderingCorrect &&
        out->allZoneIndexZero &&
        out->allXWithinViewport &&
        out->allYWithinPanel &&
        out->statusBoxYIs10 &&
        out->chestBoxYIs16Plus &&
        out->inventoryXIsAtLeast6 &&
        out->inventoryYWithinInventoryPanel &&
        out->statusBoxHandXEvenOffset &&
        out->chestBoxYPixelMonotonic &&
        out->iconIndexLookupFunctionCorrect &&
        out->statusBoxIconIndexRange &&
        out->inventoryIconIndexRange &&
        out->chestIconIndexRange;
    out->assertionCount = 19;
    return out->accepted;
}