/**
 * DM1 V1 Light & Torch System — CTest gate
 *
 * Tests are source-locked to ReDMCSB DATA.C lookup tables and
 * PANEL.C F0337/F0338, TIMELINE.C F0257, DARKCOLR.C F0431.
 */

#include "dm1_v1_light_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  [TEST] %s ... ", #name); \
    name(); \
    tests_passed++; \
    printf("PASS\n"); \
} while(0)

#define ASSERT_EQ(a, b) do { \
    int _a = (a), _b = (b); \
    if (_a != _b) { \
        printf("FAIL\n    %s:%d: %s=%d, expected %d\n", __FILE__, __LINE__, #a, _a, _b); \
        exit(1); \
    } \
} while(0)

#define ASSERT_TRUE(x) do { \
    if (!(x)) { \
        printf("FAIL\n    %s:%d: %s was false\n", __FILE__, __LINE__, #x); \
        exit(1); \
    } \
} while(0)

/* ── Test: Lookup table values match ReDMCSB DATA.C ─────────────────── */

static void test_lookup_tables(void) {
    /* ReDMCSB DATA.C line 359: LightPowerToLightAmount */
    ASSERT_EQ(dm1_light_power_to_amount[0],   0);
    ASSERT_EQ(dm1_light_power_to_amount[1],   5);
    ASSERT_EQ(dm1_light_power_to_amount[2],  12);
    ASSERT_EQ(dm1_light_power_to_amount[3],  24);
    ASSERT_EQ(dm1_light_power_to_amount[7],  51);
    ASSERT_EQ(dm1_light_power_to_amount[15], 100);

    /* ReDMCSB DATA.C line 360: PaletteIndexToLightAmount */
    ASSERT_EQ(dm1_palette_index_to_light_amount[0], 99);
    ASSERT_EQ(dm1_palette_index_to_light_amount[1], 75);
    ASSERT_EQ(dm1_palette_index_to_light_amount[2], 50);
    ASSERT_EQ(dm1_palette_index_to_light_amount[3], 25);
    ASSERT_EQ(dm1_palette_index_to_light_amount[4],  1);
    ASSERT_EQ(dm1_palette_index_to_light_amount[5],  0);

    /* ReDMCSB DATA.C line 263: ChargeCountToTorchType */
    ASSERT_EQ(dm1_charge_count_to_torch_type[0],  0);
    ASSERT_EQ(dm1_charge_count_to_torch_type[1],  1);
    ASSERT_EQ(dm1_charge_count_to_torch_type[4],  2);
    ASSERT_EQ(dm1_charge_count_to_torch_type[8],  3);
    ASSERT_EQ(dm1_charge_count_to_torch_type[15], 3);
}

/* ── Test: Init state ───────────────────────────────────────────────── */

static void test_init(void) {
    DM1_LightState s;
    dm1_light_init(&s);

    /* Full darkness: palette 5, no light */
    ASSERT_EQ(s.dungeon_view_palette_idx, DM1_LIGHT_PALETTE_DARKEST);
    ASSERT_EQ(s.total_light_amount, 0);
    ASSERT_EQ(s.magical_light_amount, 0);
    ASSERT_EQ(s.champion_count, 0);
    ASSERT_EQ(s.game_time, 0);
    ASSERT_TRUE(s.refresh_palette_requested);
}

/* ── Test: Single torch palette calculation ──────────────────────────── */

static void test_single_torch_max_charge(void) {
    /*
     * ReDMCSB F0337: single torch with charge 15
     * LightPowerToLightAmount[15] = 100
     * multiplier = 6: (100 << 6) >> 6 = 100
     * total = 100 > 99 (threshold[0]) → palette 0 (brightest)
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 15, true);

    ASSERT_EQ(s.total_light_amount, 100);
    ASSERT_EQ(s.dungeon_view_palette_idx, DM1_LIGHT_PALETTE_BRIGHTEST);
}

static void test_single_torch_mid_charge(void) {
    /*
     * Torch charge 7: LightPowerToLightAmount[7] = 51
     * multiplier 6: (51 << 6) >> 6 = 51
     * ReDMCSB: while (*threshold > total) idx++
     *   threshold[0]=99 > 51 → idx 1
     *   threshold[1]=75 > 51 → idx 2
     *   threshold[2]=50 > 51 → NO → stop at idx 2
     * → palette index 2
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 7, true);

    ASSERT_EQ(s.total_light_amount, 51);
    ASSERT_EQ(s.dungeon_view_palette_idx, 2);
}

static void test_single_torch_low_charge(void) {
    /*
     * Torch charge 1: LightPowerToLightAmount[1] = 5
     * multiplier 6: (5 << 6) >> 6 = 5
     * ReDMCSB: while (*threshold > total) idx++
     *   99>5 → 1, 75>5 → 2, 50>5 → 3, 25>5 → 4, 1>5? No → idx 4
     * → palette index 4
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 1, true);

    ASSERT_EQ(s.total_light_amount, 5);
    ASSERT_EQ(s.dungeon_view_palette_idx, 4);
}

/* ── Test: No light → darkest palette ───────────────────────────────── */

static void test_no_light_darkest(void) {
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 4);
    dm1_light_recalculate_palette(&s);

    ASSERT_EQ(s.total_light_amount, 0);
    ASSERT_EQ(s.dungeon_view_palette_idx, DM1_LIGHT_PALETTE_DARKEST);
}

/* ── Test: Unlit torch contributes nothing ──────────────────────────── */

static void test_unlit_torch_no_light(void) {
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    /* Torch with charge but NOT lit */
    dm1_light_set_torch(&s, 0, 1, 15, false);

    ASSERT_EQ(s.total_light_amount, 0);
    ASSERT_EQ(s.dungeon_view_palette_idx, DM1_LIGHT_PALETTE_DARKEST);
}

/* ── Test: Multiple torches weighted sum ────────────────────────────── */

static void test_two_torches_weighted(void) {
    /*
     * ReDMCSB F0337: Two torches, charge 15 and charge 10
     * Sorted: [15, 10, 0, 0, 0, 0, 0, 0]
     * Top 5: torch[0]=15 mult=6, torch[1]=10 mult=5
     *
     * LightPowerToLightAmount[15]=100, [10]=76
     * (100 << 6) >> 6 = 100
     * (76 << 5) >> 6 = (2432) >> 6 = 38
     * total = 100 + 38 = 138
     * 138 > 99 → palette 0
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 2);
    dm1_light_set_torch(&s, 0, 1, 15, true);
    dm1_light_set_torch(&s, 1, 1, 10, true);

    ASSERT_EQ(s.total_light_amount, 138);
    ASSERT_EQ(s.dungeon_view_palette_idx, 0);
}

/* ── Test: Torch fuel depletion every 512 ticks ─────────────────────── */

static void test_torch_depletion_interval(void) {
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 3, true);

    /* Tick 511 times — no depletion yet (depletion at tick 512) */
    for (int i = 0; i < 511; i++) {
        dm1_light_tick(&s);
    }
    ASSERT_EQ(s.torch_slots[1].charge_count, 3);

    /* Tick 512 — depletion fires */
    dm1_light_tick(&s);
    ASSERT_EQ(s.torch_slots[1].charge_count, 2);

    /* Another 512 ticks → charge 1 */
    for (int i = 0; i < 512; i++) {
        dm1_light_tick(&s);
    }
    ASSERT_EQ(s.torch_slots[1].charge_count, 1);
}

/* ── Test: Torch burns out and DoNotDiscard clears ──────────────────── */

static void test_torch_burnout(void) {
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 1, true);

    ASSERT_TRUE(s.torch_slots[1].do_not_discard);

    /* Burn out: 512 ticks */
    for (int i = 0; i < 512; i++) {
        dm1_light_tick(&s);
    }

    ASSERT_EQ(s.torch_slots[1].charge_count, 0);
    ASSERT_TRUE(!s.torch_slots[1].do_not_discard);
    /* Palette should now be darkest (no light) */
    ASSERT_EQ(s.dungeon_view_palette_idx, DM1_LIGHT_PALETTE_DARKEST);
}

/* ── Test: Magical light (FUL spell / ACTION_LIGHT) ─────────────────── */

static void test_magical_light_action(void) {
    /*
     * ReDMCSB MENU.C C038_ACTION_LIGHT (Yew Staff):
     *   MagicalLightAmount += LightPowerToLightAmount[2] = 12
     *   CreateEvent70_Light(-2, 2500)
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_add_magical(&s, 2, 2500);

    ASSERT_EQ(s.magical_light_amount, 12);
    ASSERT_EQ(s.total_light_amount, 12);
    /* 99>12→1, 75>12→2, 50>12→3, 25>12→4, 1>12?No → palette 4 */
    ASSERT_EQ(s.dungeon_view_palette_idx, 4);
    ASSERT_EQ(s.light_event_count, 1);
    ASSERT_EQ(s.light_events[0].light_power, -2);
}

/* ── Test: Magical light with torch combined ────────────────────────── */

static void test_magical_plus_torch(void) {
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 7, true); /* 51 light */
    dm1_light_add_magical(&s, 2, 2500);      /* +12 magical */

    /* Total = 51 + 12 = 63 */
    ASSERT_EQ(s.total_light_amount, 63);
    /* 99>63→1, 75>63→2, 50>63?No → palette 2 */
    ASSERT_EQ(s.dungeon_view_palette_idx, 2);
}

/* ── Test: Darkness spell ───────────────────────────────────────────── */

static void test_darkness_spell(void) {
    /*
     * ReDMCSB MENU.C C1_SPELL_TYPE_OTHER_DARKNESS:
     *   power >>= 2; power = (say) 2
     *   MagicalLightAmount -= LightPowerToLightAmount[2] = 12
     *   CreateEvent70_Light(+2, 98) — positive = darkness fading back
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 7, true); /* 51 light */

    dm1_light_add_darkness(&s, 2); /* -12 */

    ASSERT_EQ(s.magical_light_amount, -12);
    ASSERT_EQ(s.total_light_amount, 51 - 12); /* 39 */
    /* 99>39→1, 75>39→2, 50>39→3, 25>39?No → palette 3 */
    ASSERT_EQ(s.dungeon_view_palette_idx, 3);
}

/* ── Test: Light event fade (Event70_Light) ─────────────────────────── */

static void test_light_event_fade(void) {
    /*
     * ReDMCSB TIMELINE.C F0257:
     * Add light power 3. LightPowerToLightAmount[3] = 24.
     * Event scheduled at GameTime + 100 with power = -3.
     * At expiry:
     *   step 1: delta = Amount[3]-Amount[2] = 24-12 = 12; magical -= 12 → 12
     *           reschedule -2 at +4
     *   step 2: delta = Amount[2]-Amount[1] = 12-5 = 7; magical -= 7 → 5
     *           reschedule -1 at +4
     *   step 3: delta = Amount[1]-Amount[0] = 5-0 = 5; magical -= 5 → 0
     *           weaker=0, done
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_add_magical(&s, 3, 100);

    ASSERT_EQ(s.magical_light_amount, 24);

    /* Advance to tick 100 */
    for (int i = 0; i < 100; i++) {
        dm1_light_tick(&s);
    }

    /* After tick 100: first fade step fires. magical = 24 - 12 = 12 */
    ASSERT_EQ(s.magical_light_amount, 12);

    /* Advance 4 more ticks for next step */
    for (int i = 0; i < 4; i++) {
        dm1_light_tick(&s);
    }
    /* magical = 12 - 7 = 5 */
    ASSERT_EQ(s.magical_light_amount, 5);

    /* Advance 4 more ticks for final step */
    for (int i = 0; i < 4; i++) {
        dm1_light_tick(&s);
    }
    /* magical = 5 - 5 = 0 */
    ASSERT_EQ(s.magical_light_amount, 0);
    ASSERT_EQ(s.light_event_count, 0); /* All events consumed */
}

/* ── Test: Torch type from charge ───────────────────────────────────── */

static void test_torch_type(void) {
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);

    dm1_light_set_torch(&s, 0, 1, 0, false);
    ASSERT_EQ(dm1_light_get_torch_type(&s, 0, 1), 0);

    dm1_light_set_torch(&s, 0, 1, 2, true);
    ASSERT_EQ(dm1_light_get_torch_type(&s, 0, 1), 1);

    dm1_light_set_torch(&s, 0, 1, 5, true);
    ASSERT_EQ(dm1_light_get_torch_type(&s, 0, 1), 2);

    dm1_light_set_torch(&s, 0, 1, 10, true);
    ASSERT_EQ(dm1_light_get_torch_type(&s, 0, 1), 3);
}

/* ── Test: F0431_STARTEND_GetDarkenedColor (DARKCOLR.C) ─────────────── */

static void test_darken_rgb444(void) {
    /*
     * ReDMCSB DARKCOLR.C F0431:
     * 0x0FFF (white) → each component -1 → 0x0EEE
     * 0x0000 (black) → no change → 0x0000
     * 0x0F00 (red) → red-1, green=0 no change, blue=0 no change → 0x0E00
     * 0x0421 → R:4→3, G:2→1, B:1→0 → 0x0310
     */
    ASSERT_EQ(dm1_light_darken_rgb444(0x0FFF), 0x0EEE);
    ASSERT_EQ(dm1_light_darken_rgb444(0x0000), 0x0000);
    ASSERT_EQ(dm1_light_darken_rgb444(0x0F00), 0x0E00);
    ASSERT_EQ(dm1_light_darken_rgb444(0x0421), 0x0310);
    ASSERT_EQ(dm1_light_darken_rgb444(0x0111), 0x0000);
}

/* ── Test: Palette boundary exact thresholds ─────────────────────────── */

static void test_palette_boundaries(void) {
    /*
     * ReDMCSB F0337: threshold walk
     * total > 99 → palette 0 (brightest)
     * 75 < total <= 99 → palette 1
     * 50 < total <= 75 → palette 2  (threshold[1]=75 > 50, so stop at idx=2)
     * ... etc.
     * Exactly: while (*threshold > total) { idx++; }
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);

    /* Exactly 100 light → palette 0 */
    s.magical_light_amount = 100;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 0);

    /* Exactly 99 light: threshold[0]=99 > 99? No. → palette 0 */
    s.magical_light_amount = 99;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 0);

    /* 75 light: threshold[0]=99>75 yes→idx1; threshold[1]=75>75? No. → palette 1 */
    s.magical_light_amount = 75;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 1);

    /* 50: threshold[0]=99>50 yes→1; [1]=75>50 yes→2; [2]=50>50? No → palette 2 */
    s.magical_light_amount = 50;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 2);

    /* 25: ...→3; [3]=25>25? No → palette 3 */
    s.magical_light_amount = 25;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 3);

    /* 1: ...→4; [4]=1>1? No → palette 4 */
    s.magical_light_amount = 1;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 4);

    /* 0 → palette 5 (total <= 0) */
    s.magical_light_amount = 0;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 5);

    /* Negative → palette 5 */
    s.magical_light_amount = -10;
    dm1_light_recalculate_palette(&s);
    ASSERT_EQ(s.dungeon_view_palette_idx, 5);
}

/* ── Test: Legacy API compatibility ─────────────────────────────────── */

static void test_legacy_api(void) {
    M11_LightState s;
    m11_light_init(&s);

    ASSERT_EQ(s.dungeon_view_palette_idx, DM1_LIGHT_PALETTE_DARKEST);

    m11_light_apply_torch(&s, 10);
    ASSERT_TRUE(s.total_light_amount > 0);
    ASSERT_TRUE(s.dungeon_view_palette_idx < DM1_LIGHT_PALETTE_DARKEST);

    /* Level at depth 0 should be non-zero */
    ASSERT_TRUE(m11_light_get_level_at_depth(&s, 0) > 0);
    /* Darkness mask at depth 0 should be < 100 */
    ASSERT_TRUE(m11_light_get_darkness_mask(&s, 0) < 100);
}

/* ── Test: Four torches weighted sum matches ReDMCSB ────────────────── */

static void test_four_torches_formula(void) {
    /*
     * 4 torches all at charge 15:
     * Sorted: [15, 15, 15, 15, 0, 0, 0, 0]
     * Top 5 (only 4 non-zero):
     *   t0: (100 << 6) >> 6 = 100, mult→5
     *   t1: (100 << 5) >> 6 = 3200>>6 = 50, mult→4
     *   t2: (100 << 4) >> 6 = 1600>>6 = 25, mult→3
     *   t3: (100 << 3) >> 6 = 800>>6 = 12, mult→2
     * total = 100+50+25+12 = 187
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 4);
    dm1_light_set_torch(&s, 0, 1, 15, true);
    dm1_light_set_torch(&s, 1, 1, 15, true);
    dm1_light_set_torch(&s, 2, 1, 15, true);
    dm1_light_set_torch(&s, 3, 1, 15, true);

    ASSERT_EQ(s.total_light_amount, 187);
    ASSERT_EQ(s.dungeon_view_palette_idx, 0);
}

/* ── Test: Clearing torch updates palette ───────────────────────────── */

static void test_clear_torch(void) {
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_set_torch(&s, 0, 1, 15, true);
    ASSERT_EQ(s.dungeon_view_palette_idx, 0);

    dm1_light_clear_torch(&s, 0, 1);
    ASSERT_EQ(s.dungeon_view_palette_idx, DM1_LIGHT_PALETTE_DARKEST);
    ASSERT_EQ(s.total_light_amount, 0);
}

/* ── Test: Darkness spell recovery event ────────────────────────────── */

static void test_darkness_recovery(void) {
    /*
     * ReDMCSB: Darkness spell creates Event70 with positive power, 98 tick duration.
     * The event gradually restores light.
     * power 2: immediate: magical -= 12
     * At tick 98: delta = Amount[2]-Amount[1] = 12-5 = 7; magical += 7 → -5
     *             reschedule +1 at +4
     * At tick 102: delta = Amount[1]-Amount[0] = 5-0 = 5; magical += 5 → 0
     */
    DM1_LightState s;
    dm1_light_init(&s);
    dm1_light_set_champion_count(&s, 1);
    dm1_light_add_darkness(&s, 2);

    ASSERT_EQ(s.magical_light_amount, -12);

    /* Advance to tick 98 */
    for (int i = 0; i < 98; i++) {
        dm1_light_tick(&s);
    }
    ASSERT_EQ(s.magical_light_amount, -5);

    /* Advance 4 more ticks */
    for (int i = 0; i < 4; i++) {
        dm1_light_tick(&s);
    }
    ASSERT_EQ(s.magical_light_amount, 0);
    ASSERT_EQ(s.light_event_count, 0);
}

/* ── Test: NULL safety ──────────────────────────────────────────────── */

static void test_null_safety(void) {
    dm1_light_init(NULL);
    dm1_light_set_champion_count(NULL, 1);
    dm1_light_set_torch(NULL, 0, 0, 5, true);
    dm1_light_clear_torch(NULL, 0, 0);
    dm1_light_add_magical(NULL, 2, 100);
    dm1_light_add_darkness(NULL, 2);
    dm1_light_tick(NULL);
    dm1_light_recalculate_palette(NULL);
    dm1_light_decrease_torches(NULL);
    dm1_light_process_events(NULL);
    ASSERT_EQ(dm1_light_get_palette_index(NULL), DM1_LIGHT_PALETTE_DARKEST);
    ASSERT_EQ(dm1_light_get_total_amount(NULL), 0);
    ASSERT_EQ(dm1_light_get_torch_type(NULL, 0, 0), 0);
    /* If we get here, no crashes — pass */
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void) {
    printf("=== DM1 V1 Light & Torch System — source-lock CTest gate ===\n");

    TEST(test_lookup_tables);
    TEST(test_init);
    TEST(test_single_torch_max_charge);
    TEST(test_single_torch_mid_charge);
    TEST(test_single_torch_low_charge);
    TEST(test_no_light_darkest);
    TEST(test_unlit_torch_no_light);
    TEST(test_two_torches_weighted);
    TEST(test_torch_depletion_interval);
    TEST(test_torch_burnout);
    TEST(test_magical_light_action);
    TEST(test_magical_plus_torch);
    TEST(test_darkness_spell);
    TEST(test_light_event_fade);
    TEST(test_torch_type);
    TEST(test_darken_rgb444);
    TEST(test_palette_boundaries);
    TEST(test_legacy_api);
    TEST(test_four_torches_formula);
    TEST(test_clear_torch);
    TEST(test_darkness_recovery);
    TEST(test_null_safety);

    printf("\n=== %d/%d tests passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
