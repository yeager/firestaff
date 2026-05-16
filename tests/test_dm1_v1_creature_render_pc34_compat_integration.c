/*
 * CTest integration: DM1 V1 Creature Viewport Rendering source lock.
 *
 * Verifies the M10 creature render module against ReDMCSB source data:
 *
 *   1. Aspect table integrity: 27 entries, source-locked firstNative,
 *      firstDerived, coordinateSet, transparentColor, graphicInfo.
 *   2. Native bitmap index computation for all poses (front/side/back/attack).
 *   3. Direction delta calculation (M021_NORMALIZE).
 *   4. Pose selection from direction delta (F0115 logic).
 *   5. Horizontal flip determination (F0115 flip flags).
 *   6. Creature palette tables (D3, D2).
 *   7. Render list sort order (back-to-front depth, then column).
 *   8. Type name table completeness.
 *   9. Legacy API backward compatibility.
 *
 * Source references:
 *   ReDMCSB DUNVIEW.C: G0219 (line 1656), G0221-G0222 (lines 1821-1822),
 *     F0115 (line 4547), creature draw section (lines 5201-5520)
 *   ReDMCSB DEFS.H: M618=584, masks lines 1618-1629
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dm1_v1_creature_render_pc34_compat.h"

static int g_pass = 0;
static int g_fail = 0;

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) == (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: got %d, expected %d\n", msg, (int)(a), (int)(b)); } \
} while(0)

#define ASSERT_NEQ(a, b, msg) do { \
    if ((a) != (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: got %d, did not expect %d\n", msg, (int)(a), (int)(b)); } \
} while(0)

#define ASSERT_STR_EQ(a, b, msg) do { \
    if (strcmp((a), (b)) == 0) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "FAIL: %s: got '%s', expected '%s'\n", msg, (a), (b)); } \
} while(0)

/* ── Test 1: Aspect table integrity ── */
static void test_aspect_table(void) {
    const DM1_CreatureAspect* a = dm1_creature_aspects();

    /* Giant Scorpion (type 0) — ReDMCSB line 1627 */
    ASSERT_EQ(a[0].firstNativeBitmapRelativeIndex, 0,
              "GiantScorpion firstNative");
    ASSERT_EQ(a[0].coordinateSet_transparentColor, 0x1D,
              "GiantScorpion coordSet_trans");
    ASSERT_EQ(a[0].replacementColorSetIndices, 0x01,
              "GiantScorpion replColors");

    /* Swamp Slime (type 1) */
    ASSERT_EQ(a[1].firstNativeBitmapRelativeIndex, 4,
              "SwampSlime firstNative");
    ASSERT_EQ(a[1].graphicInfo, 0x0480,
              "SwampSlime graphicInfo");

    /* Mummy (type 10) — ReDMCSB line 1637 */
    ASSERT_EQ(a[10].firstNativeBitmapRelativeIndex, 29,
              "Mummy firstNative");

    /* Red Dragon (type 24) — ReDMCSB line 1651 */
    ASSERT_EQ(a[24].firstNativeBitmapRelativeIndex, 81,
              "RedDragon firstNative");
    ASSERT_EQ(a[24].graphicInfo, 0x068A,
              "RedDragon graphicInfo");

    /* Grey Lord (type 26) — last entry */
    ASSERT_EQ(a[26].firstNativeBitmapRelativeIndex, 86,
              "GreyLord firstNative");

    /* Verify coordinate set extraction */
    ASSERT_EQ(dm1_creature_coordinate_set(0), 1,
              "GiantScorpion coordSet");
    ASSERT_EQ(dm1_creature_transparent_color(0), 0x0D,
              "GiantScorpion transparentColor");
    ASSERT_EQ(dm1_creature_coordinate_set(1), 0,
              "SwampSlime coordSet");
    ASSERT_EQ(dm1_creature_transparent_color(1), 0x0B,
              "SwampSlime transparentColor");

    /* Bounds checking */
    ASSERT_EQ(dm1_creature_coordinate_set(-1), 0,
              "negativeType coordSet");
    ASSERT_EQ(dm1_creature_coordinate_set(27), 0,
              "outOfBoundsType coordSet");
}

/* ── Test 2: Native bitmap index computation ── */
static void test_native_bitmap_index(void) {
    /* Giant Scorpion: firstNative=0 → FRONT = 584+0 = 584 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(0, DM1_CREATURE_POSE_FRONT),
              584, "GiantScorpion front gfx");

    /* Giggler (type 2): firstNative=6, has BACK (0x0010), no SIDE
     * FRONT=584+6=590, BACK=590+1=591 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(2, DM1_CREATURE_POSE_FRONT),
              590, "Giggler front gfx");
    ASSERT_EQ(dm1_creature_native_bitmap_index(2, DM1_CREATURE_POSE_BACK),
              591, "Giggler back gfx");

    /* Pain Rat (type 4): firstNative=12, GI=0x0701
     * Has SIDE(0x0008)? NO — 0x0701 & 0x0008 = 0
     * Actually 0x0701 = ...0111 0000 0001 — let me check
     * 0x0701 = 0000 0111 0000 0001
     * ADDITIONAL=0x01, FLIP_NON_ATTACK=0 (bit2=0), SIDE=0 (bit3=0)
     * BACK=0 (bit4=0), ATTACK=0 (bit5=0)
     * Wait, that's the m11_game_view.c version... let me check more carefully.
     * Type 4 is mapped to m11 type 4 (Ruster in header ordering).
     *
     * Wait — there's a mismatch in type ordering between the header and
     * m11_game_view.c. Let me verify against the ReDMCSB G0219 comment:
     * ReDMCSB line 1627: Creature #00 Giant Scorpion → firstNative=0
     * ReDMCSB line 1628: Creature #01 Swamp Slime → firstNative=4
     * ReDMCSB line 1629: Creature #02 Giggler → firstNative=6
     * ReDMCSB line 1630: Creature #03 Wizard Eye → firstNative=10
     * ReDMCSB line 1631: Creature #04 Pain Rat → firstNative=12
     *
     * Pain Rat: firstNative=12, in s_aspects[4]: GI=0x0701
     * 0x0701 bits: 0000 0111 0000 0001
     *   ADDITIONAL = 1 (bits 0-1)
     *   no FLIP_NON_ATTACK (bit 2 = 0)
     *   no SIDE (bit 3 = 0)
     *   no BACK (bit 4 = 0)
     *   no ATTACK (bit 5 = 0)
     *
     * Hmm — but the m11_game_view.c type 4 comment says "Ruster" which
     * matches the HEADER enum DM1_CREATURE_RUSTER=5. The creature types
     * in the aspects table follow the G0219 order which IS the creature
     * type index from G0243.
     *
     * Actually looking back: the header enum has:
     *   DM1_CREATURE_PAIN_RAT = 4
     *   DM1_CREATURE_RUSTER = 5
     * But G0219 line 1631: Creature #04 Pain Rat = firstNative 12
     * And G0219 line 1632: Creature #05 Ruster = firstNative 16
     * And s_aspects[4] = {12, 543, 0x14, 0x34, 0x0701}
     *
     * So s_aspects[4] maps to DM1_CREATURE_PAIN_RAT. GI=0x0701:
     * FRONT = 584+12 = 596
     */
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_PAIN_RAT,
              DM1_CREATURE_POSE_FRONT), 596, "PainRat front gfx");

    /* Vexirk (type 14): firstNative=43, GI=0x05B8
     * 0x05B8 = 0000 0101 1011 1000
     *   ADDITIONAL=0 (bits 0-1)
     *   no FLIP_NON_ATTACK (bit 2 = 0... wait 0x8=1000, bit3=1)
     *   SIDE=1 (bit 3), BACK=1 (bit 4), ATTACK=1 (bit 5)
     * Sequence: FRONT(0), SIDE(+1), BACK(+2), ATTACK(+3)
     * FRONT = 584+43 = 627
     * SIDE  = 628
     * BACK  = 629
     * ATTACK = 630
     */
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_VEXIRK,
              DM1_CREATURE_POSE_FRONT), 627, "Vexirk front gfx");
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_VEXIRK,
              DM1_CREATURE_POSE_SIDE), 628, "Vexirk side gfx");
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_VEXIRK,
              DM1_CREATURE_POSE_BACK), 629, "Vexirk back gfx");
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_VEXIRK,
              DM1_CREATURE_POSE_ATTACK), 630, "Vexirk attack gfx");

    /* Bounds */
    ASSERT_EQ(dm1_creature_native_bitmap_index(-1, DM1_CREATURE_POSE_FRONT),
              0, "negativeType native idx");
    ASSERT_EQ(dm1_creature_native_bitmap_index(27, DM1_CREATURE_POSE_FRONT),
              0, "outOfBoundsType native idx");
}

/* ── Test 3: Direction delta ── */
static void test_direction_delta(void) {
    /* Party north(0), creature north(0) → delta=0 (back) */
    ASSERT_EQ(dm1_creature_direction_delta(0, 0), 0, "same dir = back");
    /* Party north(0), creature south(2) → delta=2 (front) ← (0-2)&3=2 */
    ASSERT_EQ(dm1_creature_direction_delta(0, 2), 2, "opposite = front");
    /* Party north(0), creature west(3) → (0-3)&3=1 (side-right) */
    ASSERT_EQ(dm1_creature_direction_delta(0, 3), 1, "right side");
    /* Party north(0), creature east(1) → (0-1)&3=3 (side-left) */
    ASSERT_EQ(dm1_creature_direction_delta(0, 1), 3, "left side");
    /* Party east(1), creature west(3) → (1-3)&3=2 (front) */
    ASSERT_EQ(dm1_creature_direction_delta(1, 3), 2, "east vs west = front");
}

/* ── Test 4: Pose selection ── */
static void test_pose_selection(void) {
    /* Vexirk GI=0x05B8: has SIDE, BACK, ATTACK */
    uint16_t vexirkGI = 0x05B8;

    /* delta=2 (facing party), not attacking → front */
    ASSERT_EQ(dm1_creature_pose_from_delta(2, 0, vexirkGI),
              DM1_CREATURE_POSE_FRONT, "facing party no attack = front");

    /* delta=1 (side right), has SIDE → side */
    ASSERT_EQ(dm1_creature_pose_from_delta(1, 0, vexirkGI),
              DM1_CREATURE_POSE_SIDE, "delta1 SIDE = side");

    /* delta=3 (side left), has SIDE → side */
    ASSERT_EQ(dm1_creature_pose_from_delta(3, 0, vexirkGI),
              DM1_CREATURE_POSE_SIDE, "delta3 SIDE = side");

    /* delta=0 (back), has BACK → back */
    ASSERT_EQ(dm1_creature_pose_from_delta(0, 0, vexirkGI),
              DM1_CREATURE_POSE_BACK, "delta0 BACK = back");

    /* delta=2 (front), attacking, has ATTACK → attack */
    ASSERT_EQ(dm1_creature_pose_from_delta(2, 1, vexirkGI),
              DM1_CREATURE_POSE_ATTACK, "facing attack = attack");

    /* delta=0 (back), attacking → still back (attack requires delta != 0) */
    ASSERT_EQ(dm1_creature_pose_from_delta(0, 1, vexirkGI),
              DM1_CREATURE_POSE_BACK, "back overrides attack");

    /* Creature without SIDE (e.g., Swamp Slime GI=0x0480):
     * 0x0480 = 0000 0100 1000 0000: no SIDE, no BACK, no ATTACK
     * delta=1 → falls to front */
    ASSERT_EQ(dm1_creature_pose_from_delta(1, 0, 0x0480),
              DM1_CREATURE_POSE_FRONT, "no SIDE → front on delta1");

    /* Creature without ATTACK (e.g., GI with only SIDE):
     * GI=0x0008 → SIDE only
     * delta=2, attacking → front (no ATTACK flag) */
    ASSERT_EQ(dm1_creature_pose_from_delta(2, 1, 0x0008),
              DM1_CREATURE_POSE_FRONT, "no ATTACK → front");
}

/* ── Test 5: Flip determination ── */
static void test_flip_logic(void) {
    /* Side pose: flip when directionDelta == 1 (from right) */
    ASSERT_EQ(dm1_creature_should_flip(1, DM1_CREATURE_POSE_SIDE, 0,
              DM1_GI_MASK_SIDE, 0), 1, "side delta1 = flip");
    ASSERT_EQ(dm1_creature_should_flip(3, DM1_CREATURE_POSE_SIDE, 0,
              DM1_GI_MASK_SIDE, 0), 0, "side delta3 = no flip");

    /* Front with FLIP_NON_ATTACK + FLIP_BITMAP → flip */
    ASSERT_EQ(dm1_creature_should_flip(2, DM1_CREATURE_POSE_FRONT, 0,
              DM1_GI_MASK_FLIP_NON_ATTACK, 0x40), 1,
              "front flip_non_attack + aspect 0x40 = flip");

    /* Front with FLIP_NON_ATTACK but no FLIP_BITMAP → no flip */
    ASSERT_EQ(dm1_creature_should_flip(2, DM1_CREATURE_POSE_FRONT, 0,
              DM1_GI_MASK_FLIP_NON_ATTACK, 0x00), 0,
              "front flip_non_attack no aspect = no flip");

    /* Attack + FLIP_BITMAP → flip */
    ASSERT_EQ(dm1_creature_should_flip(2, DM1_CREATURE_POSE_ATTACK, 1,
              DM1_GI_MASK_ATTACK, 0x40), 1,
              "attack flip_bitmap = flip");

    /* Back → never flip */
    ASSERT_EQ(dm1_creature_should_flip(0, DM1_CREATURE_POSE_BACK, 0,
              DM1_GI_MASK_BACK, 0x40), 0,
              "back never flip");
}

/* ── Test 6: Palette tables ── */
static void test_palette_tables(void) {
    const unsigned char* d3 = dm1_creature_palette_d3();
    const unsigned char* d2 = dm1_creature_palette_d2();

    /* ReDMCSB DUNVIEW.C line 1821:
     * {0, 12, 1, 3, 4, 3, 0, 6, 3, 0, 0, 11, 0, 2, 0, 13} */
    ASSERT_EQ(d3[0], 0, "D3 pal[0]");
    ASSERT_EQ(d3[1], 12, "D3 pal[1]");
    ASSERT_EQ(d3[2], 1, "D3 pal[2]");
    ASSERT_EQ(d3[7], 6, "D3 pal[7]");
    ASSERT_EQ(d3[15], 13, "D3 pal[15]");

    /* ReDMCSB DUNVIEW.C line 1822:
     * {0, 1, 2, 3, 4, 3, 6, 7, 5, 0, 0, 11, 12, 13, 14, 15} */
    ASSERT_EQ(d2[0], 0, "D2 pal[0]");
    ASSERT_EQ(d2[1], 1, "D2 pal[1]");
    ASSERT_EQ(d2[8], 5, "D2 pal[8]");
    ASSERT_EQ(d2[15], 15, "D2 pal[15]");
}

/* ── Test 7: Render list sort ── */
static void test_render_sort(void) {
    DM1_CreatureRenderList list;
    dm1_creature_render_init(&list);

    /* Add entries: near center, far left, far right, near right */
    list.entries[0].viewDepth = 0; list.entries[0].viewColumn = 0;
    list.entries[1].viewDepth = 3; list.entries[1].viewColumn = -1;
    list.entries[2].viewDepth = 3; list.entries[2].viewColumn = 1;
    list.entries[3].viewDepth = 0; list.entries[3].viewColumn = 1;
    list.count = 4;

    dm1_creature_render_sort(&list);

    /* Expected order: depth 3 left, depth 3 right, depth 0 center, depth 0 right */
    ASSERT_EQ(list.entries[0].viewDepth, 3, "sort[0] depth=3");
    ASSERT_EQ(list.entries[0].viewColumn, -1, "sort[0] col=-1");
    ASSERT_EQ(list.entries[1].viewDepth, 3, "sort[1] depth=3");
    ASSERT_EQ(list.entries[1].viewColumn, 1, "sort[1] col=1");
    ASSERT_EQ(list.entries[2].viewDepth, 0, "sort[2] depth=0");
    ASSERT_EQ(list.entries[2].viewColumn, 0, "sort[2] col=0");
    ASSERT_EQ(list.entries[3].viewDepth, 0, "sort[3] depth=0");
    ASSERT_EQ(list.entries[3].viewColumn, 1, "sort[3] col=1");
}

/* ── Test 8: Type names ── */
static void test_type_names(void) {
    ASSERT_STR_EQ(dm1_creature_type_name(0), "Giant Scorpion", "name[0]");
    ASSERT_STR_EQ(dm1_creature_type_name(14), "Vexirk", "name[14]");
    ASSERT_STR_EQ(dm1_creature_type_name(26), "Grey Lord", "name[26]");
    ASSERT_STR_EQ(dm1_creature_type_name(-1), "Unknown", "name[-1]");
    ASSERT_STR_EQ(dm1_creature_type_name(27), "Unknown", "name[27]");
}

/* ── Test 9: Legacy API ── */
static void test_legacy_api(void) {
    DM1_CreatureRenderList list;
    m11_creature_render_init(&list);
    ASSERT_EQ(list.count, 0, "legacy init count=0");

    m11_creature_render_collect(&list, 5, 5, 0, NULL);
    ASSERT_EQ(list.count, 0, "legacy collect stub count=0");

    /* Legacy get_graphic should use aspect table, not type*18 */
    int gfx = m11_creature_get_graphic(0, 0, 0);
    ASSERT_EQ(gfx, 584, "legacy get_graphic scorpion front=584");

    gfx = m11_creature_get_graphic(0, 1, 0);
    /* Scorpion GI=0x0482: ATTACK=0 (bit5=0), so attack falls to front */
    ASSERT_EQ(gfx, 584, "legacy get_graphic scorpion attack→front=584");

    ASSERT_STR_EQ(m11_creature_type_name(22), "Demon", "legacy name demon");
}

/* ── Test 10: Cross-check M11 aspect data agreement ──
 * The s_creatureAspects in m11_game_view.c and the s_aspects in this
 * M10 module MUST produce the same native bitmap indices and coordinate
 * sets. This test spot-checks known critical creatures. */
static void test_m11_cross_check(void) {
    /* Skeleton (type 12): firstNative=35, GI includes no SIDE, no ATTACK
     * FRONT = 584+35 = 619 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(12, DM1_CREATURE_POSE_FRONT),
              619, "Skeleton front = 619");

    /* Lord Chaos (type 23): firstNative=77
     * FRONT = 584+77 = 661 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(23, DM1_CREATURE_POSE_FRONT),
              661, "LordChaos front = 661");

    /* Demon (type 22): firstNative=73, GI=0x1480
     * 0x1480: ADDITIONAL=0, no FLIP_NON_ATTACK, no SIDE, no BACK, no ATTACK
     * Just FRONT → 584+73 = 657 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(22, DM1_CREATURE_POSE_FRONT),
              657, "Demon front = 657");
}

int main(void) {
    printf("=== DM1 V1 Creature Render Source Lock Test ===\n");

    test_aspect_table();
    test_native_bitmap_index();
    test_direction_delta();
    test_pose_selection();
    test_flip_logic();
    test_palette_tables();
    test_render_sort();
    test_type_names();
    test_legacy_api();
    test_m11_cross_check();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
