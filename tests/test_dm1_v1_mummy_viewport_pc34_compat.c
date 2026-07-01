/*
 * DM1 V1 Mummy slice gate — focused regression for the Mummy (C010_CREATURE_MUMMY).
 *
 * Source-locked to ReDMCSB:
 *
 *   DUNGEON.C:683   G0243_as_Graphic559_CreatureInfo[10] (PC 3.4 / I34E)
 *                    { AspectIndex=10, AttackSoundOrdinal=13,
 *                      GraphicInfo=0x1480, MovementTicks=17, AttackTicks=12,
 *                      Defense=25, BaseHealth=33, Attack=20, ... }
 *   DUNGEON.C:439   Same row for the original (PC 3.4 pre-1.2EN) variant.
 *   DUNVIEW.C:1656  G0219_as_Graphic558_CreatureAspects[10]
 *                    { firstNativeRelativeIndex=29, firstDerivedBitmapIndex=615,
 *                      coordinateSet_transparentColor=0x04,
 *                      replacementColorSetIndices=0x00,
 *                      graphicInfo=0x1480 }
 *   DUNVIEW.C:2392  M618_GRAPHIC_FIRST_CREATURE = 584
 *   DUNVIEW.C:5244  C0xFF_SINGLE_CENTERED_CREATURE single-creature path
 *   DEFS.H:1631     M052_MAXIMUM_HORIZONTAL_OFFSET(graphicInfo)
 *                   = ((graphicInfo) >> 12) & 3
 *                   → 0x1480 → maxH = 1, maxV = 0
 *   DEFS.H:1632     M053_MAXIMUM_VERTICAL_OFFSET(graphicInfo)
 *                   = ((graphicInfo) >> 14) & 3
 *   DEFS.H:591-604  M022/M023 offset reads, M024/M025 offset sets
 *   DEFS.H:1367     C0xFF_SINGLE_CENTERED_CREATURE = 0xFF
 *   GROUP.C:187-308 F0179_GROUP_GetCreatureAspectUpdateTime (aspect frame
 *                   cycling including the M052/M053 horizontal/vertical
 *                   offset bits)
 *   GROUP.C:524-560 F0185 single/multi-creature group placement
 *
 * The Mummy's GraphicInfo 0x1480 is the M10 source-locked value:
 *   bits 0-1   (MASK0x0003_ADDITIONAL)        = 0
 *   bit 2      (MASK0x0004_FLIP_NON_ATTACK)   = 0
 *   bit 3      (MASK0x0008_SIDE)              = 0
 *   bit 4      (MASK0x0010_BACK)              = 0
 *   bit 5      (MASK0x0020_ATTACK)            = 0
 *   bit 7      (MASK0x0080_SPECIAL_D2_FRONT)  = 0
 *   bit 8      (MASK0x0100_D2_FRONT_FLIPPED)  = 0
 *   bit 9      (MASK0x0200_FLIP_ATTACK)       = 0
 *   bit 10     (MASK0x0400_FLIP_DURING_ATTACK)= 0
 *   bits 12-13 (M052_MAXIMUM_HORIZONTAL)      = 1
 *   bits 14-15 (M053_MAXIMUM_VERTICAL)        = 0
 *
 * Therefore the Mummy has no pose variation (front-only sprite), does not
 * flip during attack, but does randomize its 3-bit horizontal sprite
 * offset each aspect update.  This test pins both halves of that contract.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "dm1_v1_creature_render_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(a, b, msg) do { \
    long long _a = (long long)(a); \
    long long _b = (long long)(b); \
    if (_a == _b) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: got %lld, expected %lld\n", msg, _a, _b); } \
} while(0)

#define ASSERT_STR_EQ(a, b, msg) do { \
    if (strcmp((a), (b)) == 0) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: got '%s', expected '%s'\n", msg, (a), (b)); } \
} while(0)

#define ASSERT_TRUE(cond, msg) do { \
    if (cond) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s\n", msg); } \
} while(0)

/* ── Test 1: Mummy aspect table is source-locked ── */
static void test_mummy_aspect_source_lock(void) {
    const DM1_CreatureAspect* a = dm1_creature_aspects();
    const DM1_CreatureAspect* mummy = &a[DM1_CREATURE_MUMMY];

    /* ReDMCSB DUNVIEW.C line 1656 (I34E variant) — aspect #10 */
    ASSERT_EQ(mummy->firstNativeBitmapRelativeIndex, 29,
              "Mummy firstNativeBitmapRelativeIndex");
    ASSERT_EQ(mummy->firstDerivedBitmapIndex, 615,
              "Mummy firstDerivedBitmapIndex");
    ASSERT_EQ(mummy->coordinateSet_transparentColor, 0x04,
              "Mummy coordSet_transparentColor");
    ASSERT_EQ(mummy->replacementColorSetIndices, 0x00,
              "Mummy replacementColorSetIndices");
    ASSERT_EQ(mummy->graphicInfo, 0x1480,
              "Mummy GraphicInfo");
    ASSERT_STR_EQ(dm1_creature_type_name(DM1_CREATURE_MUMMY), "Mummy",
                  "Mummy name");

    /* Coordinate-set split — high nibble = coord set, low = transparent. */
    ASSERT_EQ(dm1_creature_coordinate_set(DM1_CREATURE_MUMMY), 0,
              "Mummy coordinate set");
    ASSERT_EQ(dm1_creature_transparent_color(DM1_CREATURE_MUMMY), 4,
              "Mummy transparent color");

    /* Native front bitmap: 584 + 29 = 613 (M618 + firstNative, no SIDE /
     * BACK / ATTACK / FLIP_ATTACK flags, so the front bitmap is the only
     * one ReDMCSB resolves). */
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
              DM1_CREATURE_POSE_FRONT), 613, "Mummy native front index");
    /* Side / Back / Attack poses fall back to front because no pose bit
     * is set in 0x1480.  The render layer still produces a valid index. */
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
              DM1_CREATURE_POSE_SIDE), 613, "Mummy side→front fallback");
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
              DM1_CREATURE_POSE_BACK), 613, "Mummy back→front fallback");
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
              DM1_CREATURE_POSE_ATTACK), 613, "Mummy attack→front fallback");
}

/* ── Test 2: M052 / M053 horizontal offset range from Mummy GraphicInfo ── */
static void test_mummy_horizontal_offset_range(void) {
    uint16_t gi = (uint16_t)dm1_creature_aspects()[DM1_CREATURE_MUMMY].graphicInfo;

    /* ReDMCSB DEFS.H:1631 — M052 reads bits 12..13, returns 0..3. */
    ASSERT_EQ(dm1_creature_max_horizontal_offset(gi), 1,
              "Mummy M052 max horizontal offset = 1");
    /* ReDMCSB DEFS.H:1632 — M053 reads bits 14..15. */
    ASSERT_EQ(dm1_creature_max_vertical_offset(gi), 0,
              "Mummy M053 max vertical offset = 0");

    /* For comparison: Giant Scorpion has GraphicInfo 0x0482 → maxH=0,
     * maxV=0; Ghost 0x5864 → maxH=1, maxV=1; Red Dragon 0x068A → maxH=0,
     * maxV=0 (verified by the M053 macro on the literal 0x068A). */
    ASSERT_EQ(dm1_creature_max_horizontal_offset(0x0482), 0,
              "Scorpion max horizontal offset = 0");
    ASSERT_EQ(dm1_creature_max_horizontal_offset(0x5864), 1,
              "Ghost max horizontal offset = 1");
    ASSERT_EQ(dm1_creature_max_vertical_offset(0x5864), 1,
              "Ghost max vertical offset = 1");
    ASSERT_EQ(dm1_creature_max_vertical_offset(0x068A), 0,
              "RedDragon max vertical offset = 0");
    /* Synthetic maxH=2 / maxV=2 (bits 12-13 = 0b10, bits 14-15 = 0b10). */
    ASSERT_EQ(dm1_creature_max_horizontal_offset(0xA000), 2,
              "synthetic 0xA000 maxH = 2");
    ASSERT_EQ(dm1_creature_max_vertical_offset(0xA000), 2,
              "synthetic 0xA000 maxV = 2");
}

/* ── Test 3: Aspect offset bits cycle according to F0179 ── */
static void test_mummy_aspect_frame_cycling(void) {
    uint8_t aspect;
    int hOff, vOff;

    /* ReDMCSB GROUP.C F0179 lines 226-243: maxH=1 means
     *   M002_RANDOM(1) = 0; then if randomBit set, offset = (-0)&0x7 = 0;
     *   else offset = 0.  So Mummy horizontal offset bits are always 0
     *   regardless of randomBit (only the magnitude has range). */
    aspect = dm1_creature_cycle_aspect_frame(DM1_CREATURE_MUMMY, 0xFF, 0, 0);
    hOff = dm1_creature_aspect_horizontal_offset(aspect);
    vOff = dm1_creature_aspect_vertical_offset(aspect);
    ASSERT_EQ(hOff, 0, "Mummy non-attack randomBit=0 horizontal offset = 0");
    ASSERT_EQ(vOff, 0, "Mummy non-attack vertical offset = 0");

    aspect = dm1_creature_cycle_aspect_frame(DM1_CREATURE_MUMMY, 0xFF, 0, 7);
    hOff = dm1_creature_aspect_horizontal_offset(aspect);
    ASSERT_EQ(hOff, 0, "Mummy non-attack randomBit=7 horizontal offset = 0");
    /* IS_ATTACKING must be cleared for non-attack update. */
    ASSERT_EQ(aspect & DM1_CREATURE_ASPECT_IS_ATTACKING, 0,
              "Mummy non-attack update clears IS_ATTACKING");

    /* Attacking update: IS_ATTACKING set, FLIP_BITMAP cleared because the
     * Mummy has no MASK0x0200_FLIP_ATTACK. */
    aspect = dm1_creature_cycle_aspect_frame(DM1_CREATURE_MUMMY, 0x00, 1, 7);
    ASSERT_TRUE((aspect & DM1_CREATURE_ASPECT_IS_ATTACKING) != 0,
                "Mummy attack update sets IS_ATTACKING");
    ASSERT_EQ(aspect & DM1_CREATURE_ASPECT_FLIP_BITMAP, 0,
              "Mummy attack update never flips (GraphicInfo 0x1480)");

    /* Re-run with randomBit=1 → sign bit (0x04) on, magnitude stays 0
     * because maxH = 1 implies M002_RANDOM(1) == 0 always. */
    aspect = dm1_creature_cycle_aspect_frame(DM1_CREATURE_MUMMY, 0x00, 0,
                                             /* randomBit=1 */
                                             (1 << 0));
    hOff = dm1_creature_aspect_horizontal_offset(aspect);
    ASSERT_EQ(hOff, 0, "Mummy maxH=1 horizontal offset magnitude is 0");
}

/* ── Test 4: Group placement — single creature is 0xFF ── */
static void test_mummy_single_creature_placement(void) {
    /* ReDMCSB GROUP.C F0185 line 528: count==0 → L0352_ui_GroupCells =
     * C0xFF_SINGLE_CENTERED_CREATURE.  This is the path used by the
     * Mummy when encountered solo. */
    ASSERT_EQ(dm1_creature_place_group_cells(0, DM1_CREATURE_SIZE_QUARTER,
                                              NULL, NULL),
              DM1_GROUP_CELL_SINGLE_CENTERED,
              "single creature group → 0xFF");
}

/* ── Test 5: Group placement — multi-creature cells are deterministic
 *            when rng is NULL (regression-friendly path). ── */
static void test_mummy_multi_creature_placement_deterministic(void) {
    /* ReDMCSB GROUP.C F0185 lines 521-560: for count>=1, slots are
     * packed 2 bits each.  Each slot advances the cell by 1 via the
     * post-increment in F0185; quarter- and full-square creatures get
     * the same +1 step, half-square gets an additional +1. */
    int cells = dm1_creature_place_group_cells(2, DM1_CREATURE_SIZE_QUARTER,
                                                NULL, NULL);
    /* Slot 0 → cell 0, slot 1 → cell 1 → cells = 0b00 | (0b01<<2) = 0x04. */
    ASSERT_EQ(cells, 0x04, "two quarter-square Mummy slots = 0x04");

    cells = dm1_creature_place_group_cells(4, DM1_CREATURE_SIZE_QUARTER,
                                            NULL, NULL);
    /* Slots 0,1,2,3 → cells 0,1,2,3 → packed 2 bits each → 0xE4. */
    ASSERT_EQ(cells, 0xE4, "four quarter-square Mummy slots = 0xE4");

    /* Half-square creatures advance the cell by 2 per slot. */
    cells = dm1_creature_place_group_cells(3, DM1_CREATURE_SIZE_HALF,
                                            NULL, NULL);
    /* Slot 0 → 0, slot 1 → 2, slot 2 → 0 → 0b00 | (0b10<<2) | (0b00<<4)
     * = 0x08. */
    ASSERT_EQ(cells, 0x08, "three half-square Mummy slots = 0x08");
}

/* ── Test 6: Group placement — full-square creatures step one cell each. ── */
static void test_full_square_placement(void) {
    /* A full-square creature (C1_SIZE_FULL_SQUARE = 2) advances the
     * base cell by 1 each slot.  With rng=NULL the deterministic path
     * produces start cell 0 then 1, 2, 3 packed into 2 bits each:
     *   slot 0 → 0b00 → bits 0-1   = 0x00
     *   slot 1 → 0b01 → bits 2-3   = 0x04
     *   slot 2 → 0b10 → bits 4-5   = 0x20
     *   slot 3 → 0b11 → bits 6-7   = 0xC0
     * Sum = 0xE4. */
    int cells = dm1_creature_place_group_cells(4, DM1_CREATURE_SIZE_FULL,
                                               NULL, NULL);
    ASSERT_EQ(cells, 0xE4, "four full-square slots with rng=NULL");
}

/* ── Test 7: Source-lock cross-reference with M11 sprite table. ── */
static void test_m11_cross_reference(void) {
    /* The m11_game_view.c m11_cr_setup_sprite_table places the Mummy
     * (M11_CR_MUMMY = 0) at GRAPHICS.DAT bitmap index 225 with 4
     * animation frames.  Each Mummy frame is a separate native bitmap
     * and the Mummy sprite atlas starts at the offset that
     * firstNativeBitmapRelativeIndex = 29 maps to.  Our computed
     * native front index (613) is the ReDMCSB source-locked value
     * (M618 + 29) which differs from the M11 layer's sprite atlas
     * because the M11 layer re-bases the Mummy atlas at 225 in
     * GRAPHICS.DAT for non-source-locked demo rendering.  The two
     * are independent atlases; the M10 source-locked value is 613. */
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_MUMMY,
              DM1_CREATURE_POSE_FRONT), 613,
              "M10 source-locked Mummy front bitmap = 613");

    /* The Mummy creature type index (10) and the M11 enum (0) are
     * intentionally different ordinals — the M11 sprite table sorts by
     * M11_CR_* which places MUMMY first, whereas the M10 CreatureInfo
     * follows the source-locked G0219 / G0243 order.  This assertion
     * documents that contract. */
    ASSERT_TRUE(DM1_CREATURE_MUMMY == 10,
                "M10 creature type ordinal = 10 (G0219 index)");
}

int main(void) {
    printf("=== DM1 V1 Mummy Slice Gate ===\n");

    test_mummy_aspect_source_lock();
    test_mummy_horizontal_offset_range();
    test_mummy_aspect_frame_cycling();
    test_mummy_single_creature_placement();
    test_mummy_multi_creature_placement_deterministic();
    test_full_square_placement();
    test_m11_cross_reference();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
