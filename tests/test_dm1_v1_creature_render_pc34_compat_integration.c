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
 *   9. Legacy render API delegation.
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
    ASSERT_EQ(a[1].graphicInfo, 0xA625,
              "SwampSlime graphicInfo");

    /* Mummy (type 10) — ReDMCSB line 1637 */
    ASSERT_EQ(a[10].firstNativeBitmapRelativeIndex, 29,
              "Mummy firstNative");

    /* Red Dragon (type 24) — ReDMCSB line 1651 */
    ASSERT_EQ(a[24].firstNativeBitmapRelativeIndex, 81,
              "RedDragon firstNative");
    ASSERT_EQ(a[24].graphicInfo, 0x97BD,
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

    /* Giggler (type 2): firstNative=6, GI=0x6198 → has SIDE and BACK
     * FRONT=584+6=590, SIDE=591, BACK=592 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(2, DM1_CREATURE_POSE_FRONT),
              590, "Giggler front gfx");
    ASSERT_EQ(dm1_creature_native_bitmap_index(2, DM1_CREATURE_POSE_BACK),
              592, "Giggler back gfx");

    /* Pain Rat (type 4): firstNative=12, GI=0xA3B8
     * SIDE=1, BACK=1, ATTACK=1 → FRONT = 584+12 = 596 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(DM1_CREATURE_PAIN_RAT,
              DM1_CREATURE_POSE_FRONT), 596, "PainRat front gfx");

    /* Vexirk (type 14): firstNative=43, GI=0x1638 → SIDE+BACK+ATTACK
     * FRONT=627, SIDE=628, BACK=629, ATTACK=630 */
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
    /* Test with a GI that has SIDE, BACK, ATTACK (like Vexirk 0x1638) */
    uint16_t vexirkGI = 0x1638;

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

    /* GI with no SIDE, no BACK, no ATTACK → delta=1 falls to front */
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

/* ── Test 9: Legacy render API delegation ── */
static void test_legacy_api(void) {
    DM1_CreatureRenderList list;
    DM1_V1_CreatureRender_InitPc34Compat(&list);
    ASSERT_EQ(list.count, 0, "legacy init count=0");

    list.count = 2;
    list.entries[0].viewDepth = 1;
    list.entries[0].viewColumn = 1;
    list.entries[1].viewDepth = 3;
    list.entries[1].viewColumn = -1;
    DM1_V1_CreatureRender_SortPc34Compat(&list);
    ASSERT_EQ(list.entries[0].viewDepth, 3, "legacy sort uses DM1 depth order");
    ASSERT_EQ(list.entries[0].viewColumn, -1, "legacy sort keeps DM1 column order");

    /* Legacy get_graphic should use aspect table, not type*18 */
    int gfx = DM1_V1_CreatureRender_GetGraphicPc34Compat(0, 0, 0);
    ASSERT_EQ(gfx, 584, "legacy get_graphic scorpion front=584");

    gfx = DM1_V1_CreatureRender_GetGraphicPc34Compat(0, 1, 0);
    /* Scorpion GI=0x623D: ATTACK set (bit5), SIDE+BACK too → offset=3 */
    ASSERT_EQ(gfx, 587, "legacy get_graphic scorpion attack=587");

    ASSERT_STR_EQ(DM1_V1_CreatureRender_TypeNamePc34Compat(22), "Demon", "legacy name demon");
}

/* ── Test 10: Cross-check M11 aspect data agreement ──
 * The s_creatureAspects in m11_game_view.c and the s_aspects in this
 * M10 module MUST produce the same native bitmap indices and coordinate
 * sets. This test spot-checks known critical creatures. */
static void test_m11_cross_check(void) {
    /* Skeleton (type 12): firstNative=35, GI=0x6038 → SIDE+BACK+ATTACK
     * FRONT = 584+35 = 619 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(12, DM1_CREATURE_POSE_FRONT),
              619, "Skeleton front = 619");

    /* Lord Chaos (type 23): firstNative=77
     * FRONT = 584+77 = 661 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(23, DM1_CREATURE_POSE_FRONT),
              661, "LordChaos front = 661");

    /* Demon (type 22): firstNative=73, GI=0x53BD → SIDE+BACK+ATTACK
     * FRONT = 584+73 = 657 */
    ASSERT_EQ(dm1_creature_native_bitmap_index(22, DM1_CREATURE_POSE_FRONT),
              657, "Demon front = 657");
}

/* Test 11: M11 creature query delegation surface. */
static void test_m11_query_surface(void) {
    int mirror = -1;
    int repl9 = 0;
    int repl10 = 0;

    ASSERT_EQ(dm1_creature_sprite_for_depth(0, 0), 584,
              "query depth scorpion native front");
    ASSERT_EQ(dm1_creature_sprite_for_depth(0, 1), 763,
              "query depth scorpion derived D2 front");
    ASSERT_EQ(dm1_creature_sprite_for_view(3, 0, 0, 0, 0, &mirror), 594,
              "query view WizardEye back keeps M11 native offset");
    ASSERT_EQ(mirror, 0, "query view back mirror off");
    ASSERT_EQ(dm1_creature_sprite_for_view(14, 0, 1, 0, 0, &mirror), 628,
              "query view Vexirk side native");
    ASSERT_EQ(mirror, 1, "query view side mirror on");
    ASSERT_EQ(dm1_creature_sprite_for_view(14, 0, 2, 0, 1, &mirror), 630,
              "query view Vexirk attack native");

    ASSERT_EQ(dm1_creature_graphic_info(14), 0x1638,
              "query graphic info Vexirk");
    ASSERT_EQ(dm1_creature_native_bitmap_count(14), 4,
              "query native bitmap count Vexirk");
    ASSERT_EQ(dm1_creature_derived_bitmap_count(14), 8,
              "query derived bitmap count Vexirk");
    ASSERT_EQ(dm1_creature_has_side_bitmap(14), 1,
              "query has side Vexirk");
    ASSERT_EQ(dm1_creature_has_back_bitmap(14), 1,
              "query has back Vexirk");
    ASSERT_EQ(dm1_creature_has_attack_bitmap(14), 1,
              "query has attack Vexirk");
    ASSERT_EQ(dm1_creature_has_flip_during_attack(3), 0,
              "query WizardEye no flip during attack");
    ASSERT_EQ(dm1_creature_replacement_colors(0, &repl9, &repl10), 1,
              "query replacement colors scorpion");
    ASSERT_EQ(repl9, 4, "query replacement color9 scorpion");
    ASSERT_EQ(repl10, 10, "query replacement color10 scorpion fallback");
    ASSERT_EQ(dm1_creature_replacement_colors(2, &repl9, &repl10), 0,
              "query no replacement colors Giggler");
}

/* Test 12: Creature draw placement helper surface. */
static void test_draw_placement_surface(void) {
    DM1_CreatureDrawPlacement p;
    DM1_CreatureDrawPlan plan;
    int types[2] = { 14, 10 };
    int counts[2] = { 2, 1 };
    int dirs[2] = { 2, 0 };
    ASSERT_EQ(dm1_creature_center_draw_placement(14, 0, 32, 42, 160, 90,
              1, 0, 1, 0, &p), 1, "center placement returns row");
    ASSERT_EQ(p.side_hint, 0, "center placement side hint");
    ASSERT_EQ(p.source_anchor_valid, 1, "center placement retains G0224 anchor");
    ASSERT_EQ(p.source_depth_index, 0, "center placement retains D1 source row");
    ASSERT_EQ(p.w > 0, 1, "center placement width positive");
    ASSERT_EQ(p.h > 0, 1, "center placement height positive");
    ASSERT_EQ(p.x >= 32, 1, "center placement x in face");
    ASSERT_EQ(p.y >= 42, 1, "center placement y in face");

    ASSERT_EQ(dm1_creature_side_draw_placement(14, 1, -1, 4, 50, 48, 70,
              1, 0, 2, 1, &p), 1, "side placement returns row");
    ASSERT_EQ(p.side_hint, 0, "side placement binds source zone");
    ASSERT_EQ(p.source_anchor_valid, 1, "side placement retains G0224 anchor");
    ASSERT_EQ(p.source_depth_index, 1, "side placement retains D2 source row");
    ASSERT_EQ(p.w > 0, 1, "side placement width positive");
    ASSERT_EQ(p.h > 0, 1, "side placement height positive");

    ASSERT_EQ(dm1_creature_center_draw_plan(types, counts, dirs, 2,
              0, 32, 42, 160, 90, &plan), 3,
              "center draw plan expands groups");
    ASSERT_EQ(plan.count, 3, "center draw plan count");
    ASSERT_EQ(plan.entries[0].group_index, 0, "center plan group 0 first");
    ASSERT_EQ(plan.entries[0].duplicate_index, 0, "center plan dup 0");
    ASSERT_EQ(plan.entries[0].first_in_group, 1, "center plan first flag");
    ASSERT_EQ(plan.entries[0].creature_count, 2, "center plan count field");
    ASSERT_EQ(plan.entries[0].creature_direction, 2, "center plan direction");
    ASSERT_EQ(plan.entries[1].group_index, 0, "center plan group 0 second");
    ASSERT_EQ(plan.entries[1].duplicate_index, 1, "center plan dup 1");
    ASSERT_EQ(plan.entries[1].first_in_group, 0, "center plan second flag");
    ASSERT_EQ(plan.entries[2].group_index, 1, "center plan group 1");
    ASSERT_EQ(plan.entries[2].creature_type, 10, "center plan creature type");

    ASSERT_EQ(dm1_creature_side_draw_plan(types, counts, dirs, 2,
              1, -1, 4, 50, 48, 70, &plan), 3,
              "side draw plan expands groups");
    ASSERT_EQ(plan.count, 3, "side draw plan count");
    ASSERT_EQ(plan.entries[0].placement.w > 0, 1, "side plan width positive");
    ASSERT_EQ(dm1_creature_center_draw_plan(NULL, counts, dirs, 2,
              0, 32, 42, 160, 90, &plan), 0,
              "center draw plan rejects null types");
}

/* Test 13: Source-locked creature Aspect frame cycling.
 * ReDMCSB GROUP.C F0179 lines 222-305 stores frame state in the
 * active-group Aspect bits, and DUNVIEW.C F0115 lines 5331-5418 consumes
 * those bits to select attack/front flip state. */
static void test_aspect_frame_cycling(void) {
    ASSERT_EQ(dm1_creature_next_aspect_update_delay(0x0A50, 0, 0),
              5, "non-attack aspect delay low random");
    ASSERT_EQ(dm1_creature_next_aspect_update_delay(0x0A50, 0, 1),
              6, "non-attack aspect delay high random");
    ASSERT_EQ(dm1_creature_next_aspect_update_delay(0x0A50, 1, 0),
              10, "attack aspect delay");

    /* Ghost has FLIP_NON_ATTACK (GI=0x5225), so non-attack update chooses
     * the FLIP_BITMAP frame bit from M005_RANDOM(2). */
    ASSERT_EQ(dm1_creature_cycle_aspect_frame(DM1_CREATURE_GHOST, 0x00, 0, 1),
              DM1_CREATURE_ASPECT_FLIP_BITMAP,
              "ghost non-attack random=1 sets flip frame");
    ASSERT_EQ(dm1_creature_cycle_aspect_frame(DM1_CREATURE_GHOST, 0xC0, 0, 0),
              0, "ghost non-attack random=0 clears attack and flip");

    /* Giant Scorpion has FLIP_NON_ATTACK (GI=0x623D), so idle cycling
     * with random=1 sets the flip frame bit. */
    ASSERT_EQ(dm1_creature_cycle_aspect_frame(DM1_CREATURE_GIANT_SCORPION,
              DM1_CREATURE_ASPECT_FLIP_BITMAP, 0, 1),
              DM1_CREATURE_ASPECT_FLIP_BITMAP,
              "scorpion idle random=1 sets flip frame");

    /* Pain Rat has FLIP_ATTACK but not FLIP_DURING_ATTACK (GI=0xA3B8).
     * First attack with random=1 sets flip; subsequent attack with random=0
     * clears flip since FLIP_DURING_ATTACK is not set. */
    ASSERT_EQ(dm1_creature_cycle_aspect_frame(DM1_CREATURE_PAIN_RAT, 0x00, 1, 1),
              DM1_CREATURE_ASPECT_IS_ATTACKING | DM1_CREATURE_ASPECT_FLIP_BITMAP,
              "pain rat first attack update sets attack and flip");
    ASSERT_EQ(dm1_creature_cycle_aspect_frame(DM1_CREATURE_PAIN_RAT, 0xC0, 1, 0),
              DM1_CREATURE_ASPECT_IS_ATTACKING,
              "pain rat no FLIP_DURING_ATTACK clears flip on random=0");

    /* Animated Armour/Deth Knight toggles the attack flip bit when already
     * attacking, matching GROUP.C lines 239-242. */
    ASSERT_EQ(dm1_creature_cycle_aspect_frame(DM1_CREATURE_ANIMATED_ARMOUR, 0xC0, 1, 1),
              DM1_CREATURE_ASPECT_IS_ATTACKING,
              "animated armour attack update toggles flip off");
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
    test_m11_query_surface();
    test_draw_placement_surface();
    test_aspect_frame_cycling();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail > 0 ? 1 : 0;
}
