/*
 * test_dm1_v1_food_water_pc34_compat.c
 *
 * DM1 V1 food/water state regression. Pins the source-locked
 * m11_fw_* API used by the fountain interaction (DM1V1_Fountain_*)
 * and by the food/water consume path on the inventory side
 * (dm1_inventory_consume_food_junk_pc34 / dm1_inventory_consume_water_junk_pc34).
 *
 * Source-locked against:
 *   - ReDMCSB CHAMPION.C F0331_CHAMPION_ApplyTimeEffects_CPSF
 *     (lines 2360..2418) — per-tick food/water decay, F0331:2395
 *     food-non-negative clamp, F0331:2406 water-non-negative clamp,
 *     F0331:2413/2416 floor at -1024.
 *   - ReDMCSB PANEL.C F0349_CHAMPION_HandleMouthUseLocal (lines
 *     1743..1785 + 1824..1945) — leader-hand food/water consume and
 *     the DUNGEON.C:428..436 G0242 food amount table.
 *   - include/dm1_v1_food_water_pc34_compat.h — M11_FOOD_DECAY_PER_TICK,
 *     M11_WATER_DECAY_PER_TICK, M11_MAX_CHAMPIONS, M11_FoodWaterState.
 *
 * This is a data-free slice. It does NOT load GRAPHICS.DAT / DUNGEON.DAT,
 * drive M11 graphics, claim original-vs-Firestaff parity, or wire the
 * survival tick into M11_GameView (the M11 path uses
 * dm1_needs_apply_time_effects in src/dm1/dm1_v1_champion_needs_pc34_compat.c).
 */

#include "dm1_v1_food_water_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void check_int(const char* label, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d\n", label, got, want);
    } else {
        printf("PASS %s == %d\n", label, want);
    }
}

static void check_int_ge(const char* label, int got, int wantMin)
{
    ++g_assertions;
    if (got < wantMin) {
        ++g_failures;
        printf("FAIL %s got=%d want>=%d\n", label, got, wantMin);
    } else {
        printf("PASS %s == %d (>= %d)\n", label, got, wantMin);
    }
}

static void test_init(void)
{
    /* ReDMCSB PANEL.C F0349 + CHAMPION.C F0331: freshly initialised
     * food/water state has full food=1000, full water=1000, no
     * starvation/thirst, lastEat/lastDrink at 0. Count is clamped to
     * [0, M11_MAX_CHAMPIONS]. */
    M11_FoodWaterState s;
    memset(&s, 0x7A, sizeof(s));
    m11_fw_init(&s, 4);
    check_int("init.count=4", s.count, 4);
    for (int i = 0; i < 4; ++i) {
        check_int("init.food full", s.champions[i].food, 1000);
        check_int("init.water full", s.champions[i].water, 1000);
        check_int("init.lastEatMs", s.champions[i].lastEatMs, 0);
        check_int("init.lastDrinkMs", s.champions[i].lastDrinkMs, 0);
        check_int("init.starved", s.champions[i].starved, 0);
        check_int("init.thirsty", s.champions[i].thirsty, 0);
    }

    /* Clamp count to M11_MAX_CHAMPIONS. */
    M11_FoodWaterState sBig;
    memset(&sBig, 0, sizeof(sBig));
    m11_fw_init(&sBig, 99);
    check_int("init.clamp_high", sBig.count, M11_MAX_CHAMPIONS);

    /* Negative count clamps to 0; no champion is touched. */
    M11_FoodWaterState sNeg;
    memset(&sNeg, 0, sizeof(sNeg));
    m11_fw_init(&sNeg, -5);
    check_int("init.clamp_neg", sNeg.count, 0);

    /* NULL state is a no-op. */
    m11_fw_init(0, 4);
    printf("PASS init.null_state\n");
}

static void test_tick_decay(void)
{
    /* ReDMCSB CHAMPION.C F0331:2395/2406 food/water stay non-negative
     * after a tick; m11_fw_tick clamps to 0. The decay rate in the
     * shim is M11_FOOD_DECAY_PER_TICK (3) / M11_WATER_DECAY_PER_TICK (3). */
    M11_FoodWaterState s;
    m11_fw_init(&s, 2);
    s.champions[0].food = 1000;
    s.champions[0].water = 1000;
    s.champions[1].food = 5;
    s.champions[1].water = 4;

    m11_fw_tick(&s, 1);
    check_int("tick.food_normal", s.champions[0].food, 997);
    check_int("tick.water_normal", s.champions[0].water, 997);

    /* Champion 1: 5 food / 4 water after one tick of 3/3 decay. */
    check_int("tick.food_low", s.champions[1].food, 2);
    check_int("tick.water_low", s.champions[1].water, 1);
    /* Both still above zero — no starve/thirst yet. */
    check_int("tick.starved_low", s.champions[1].starved, 0);
    check_int("tick.thirsty_low", s.champions[1].thirsty, 0);

    /* Tick again: champion 1 hits zero food/water; starved/thirsty
     * flags go high. */
    m11_fw_tick(&s, 2);
    check_int("tick.food_floor", s.champions[1].food, 0);
    check_int("tick.water_floor", s.champions[1].water, 0);
    check_int("tick.starved", s.champions[1].starved, 1);
    check_int("tick.thirsty", s.champions[1].thirsty, 1);
    check_int("tick.normal_still_ok", s.champions[0].starved, 0);

    /* Repeated ticks must NOT push food/water negative; the clamp
     * guards against the F0331:2413 floor at -1024 for the source
     * (decay stops at 0 in the shim). */
    for (int i = 0; i < 50; ++i) {
        m11_fw_tick(&s, 100 + i);
    }
    check_int("tick.food_no_underflow", s.champions[1].food, 0);
    check_int("tick.water_no_underflow", s.champions[1].water, 0);
    check_int("tick.starved_sticky", s.champions[1].starved, 1);
    check_int("tick.thirsty_sticky", s.champions[1].thirsty, 1);

    /* NULL state must not crash. */
    m11_fw_tick(0, 1);
    printf("PASS tick.null_state\n");
}

static void test_eat_drink(void)
{
    /* ReDMCSB PANEL.C F0349:1824..1844 mouth consume updates food/water
     * and refreshes the lastEat/lastDrink timestamps; F0349:1918..1919
     * food is capped at 2048 (CHAMPION.C clamp). The shim caps at 1000
     * for the M11_FoodWaterState side, matching the existing API contract
     * used by dm1_v1_fountain_interaction_pc34_compat. */
    M11_FoodWaterState s;
    m11_fw_init(&s, 1);
    s.champions[0].food = 500;
    s.champions[0].water = 500;
    s.champions[0].starved = 1;
    s.champions[0].thirsty = 1;

    /* Eat 400 food → 900. */
    int rc = m11_fw_eat(&s, 0, 400, 1234);
    check_int("eat.rc", rc, 1);
    check_int("eat.food_after_add", s.champions[0].food, 900);
    check_int("eat.lastEatMs", s.champions[0].lastEatMs, 1234);
    check_int("eat.clears_starved", s.champions[0].starved, 0);

    /* Eat past the cap → clamped to 1000, NOT 1100. */
    m11_fw_eat(&s, 0, 300, 1500);
    check_int("eat.food_capped", s.champions[0].food, 1000);

    /* Drink 400 water → 900. */
    rc = m11_fw_drink(&s, 0, 400, 2222);
    check_int("drink.rc", rc, 1);
    check_int("drink.water_after_add", s.champions[0].water, 900);
    check_int("drink.lastDrinkMs", s.champions[0].lastDrinkMs, 2222);
    check_int("drink.clears_thirsty", s.champions[0].thirsty, 0);

    /* Drink past the cap → clamped to 1000. */
    m11_fw_drink(&s, 0, 300, 2500);
    check_int("drink.water_capped", s.champions[0].water, 1000);

    /* Eat/drink with NULL state return 0. */
    check_int("eat.null_state", m11_fw_eat(0, 0, 100, 0), 0);
    check_int("drink.null_state", m11_fw_drink(0, 0, 100, 0), 0);

    /* Eat/drink with out-of-range champion returns 0 and does not
     * mutate any champion. */
    check_int("eat.oor_neg", m11_fw_eat(&s, -1, 100, 0), 0);
    check_int("eat.oor_high", m11_fw_eat(&s, 5, 100, 0), 0);
    check_int("drink.oor_neg", m11_fw_drink(&s, -1, 100, 0), 0);
    check_int("drink.oor_high", m11_fw_drink(&s, 5, 100, 0), 0);
    check_int("eat.oor_preserves_food", s.champions[0].food, 1000);
    check_int("drink.oor_preserves_water", s.champions[0].water, 1000);
}

static void test_getters(void)
{
    /* Getter parity: m11_fw_get_food/_get_water mirror champion[i].food
     * and champion[i].water; bounds checks return 0. */
    M11_FoodWaterState s;
    m11_fw_init(&s, 2);
    s.champions[0].food = 700;
    s.champions[0].water = 800;
    s.champions[1].food = 100;
    s.champions[1].water = 200;

    check_int("get.food[0]", m11_fw_get_food(&s, 0), 700);
    check_int("get.water[0]", m11_fw_get_water(&s, 0), 800);
    check_int("get.food[1]", m11_fw_get_food(&s, 1), 100);
    check_int("get.water[1]", m11_fw_get_water(&s, 1), 200);

    /* Bounds. */
    check_int("get.food.oor_neg", m11_fw_get_food(&s, -1), 0);
    check_int("get.food.oor_high", m11_fw_get_food(&s, 99), 0);
    check_int("get.water.oor_neg", m11_fw_get_water(&s, -1), 0);
    check_int("get.water.oor_high", m11_fw_get_water(&s, 99), 0);

    /* NULL state. */
    check_int("get.food.null", m11_fw_get_food(0, 0), 0);
    check_int("get.water.null", m11_fw_get_water(0, 0), 0);

    /* Starved/thirsty getters track the same flag the tick sets. */
    check_int("is.starved[0]", m11_fw_is_starved(&s, 0), 0);
    check_int("is.thirsty[0]", m11_fw_is_thirsty(&s, 0), 0);
    s.champions[1].starved = 1;
    s.champions[1].thirsty = 1;
    check_int("is.starved[1]", m11_fw_is_starved(&s, 1), 1);
    check_int("is.thirsty[1]", m11_fw_is_thirsty(&s, 1), 1);
    check_int("is.starved.oor", m11_fw_is_starved(&s, 99), 0);
    check_int("is.thirsty.oor", m11_fw_is_thirsty(&s, 99), 0);
    check_int("is.starved.null", m11_fw_is_starved(0, 0), 0);
    check_int("is.thirsty.null", m11_fw_is_thirsty(0, 0), 0);
}

static void test_starvation_damage(void)
{
    /* ReDMCSB CHAMPION.C F0325_DecrementStamina: when stamina underflows
     * the F0331 food/water starvation path adds HP damage. The shim's
     * m11_fw_starvation_damage returns 2 per active starve/thirst flag
     * so callers can feed it back into the M11 combat HP delta path. */
    M11_FoodWaterState s;
    m11_fw_init(&s, 4);
    /* All champions start full: no damage. */
    for (int i = 0; i < 4; ++i) {
        check_int("starve_dmg.normal", m11_fw_starvation_damage(&s, i), 0);
    }
    /* Champion 0 starving only → +2 damage. */
    s.champions[0].starved = 1;
    check_int("starve_dmg.food_only", m11_fw_starvation_damage(&s, 0), 2);
    /* Champion 1 thirsty only → +2 damage. */
    s.champions[0].starved = 0;
    s.champions[1].thirsty = 1;
    check_int("starve_dmg.water_only", m11_fw_starvation_damage(&s, 1), 2);
    /* Champion 2 both → +4 damage. */
    s.champions[2].starved = 1;
    s.champions[2].thirsty = 1;
    check_int("starve_dmg.both", m11_fw_starvation_damage(&s, 2), 4);
    /* Champion 3 neither → 0. */
    check_int("starve_dmg.none", m11_fw_starvation_damage(&s, 3), 0);
    /* Bounds: OOR / NULL → 0. */
    check_int("starve_dmg.oor_neg", m11_fw_starvation_damage(&s, -1), 0);
    check_int("starve_dmg.oor_high", m11_fw_starvation_damage(&s, 99), 0);
    check_int("starve_dmg.null", m11_fw_starvation_damage(0, 0), 0);
}

static void test_inventory_handoff_lifecycle(void)
{
    /* Lifecycle check that the inventory-consume side actually feeds
     * the m11_fw_* state with non-zero food amounts. Mirrors what the
     * leader-hand food/water consume path does: pick an icon, look up
     * the food amount from the DUNGEON.C:428..436 G0242 table via
     * dm1_inventory_food_amount_from_icon_pc34, then credit it through
     * m11_fw_eat / m11_fw_drink. The 8-entry G0242 apple..dragon-steak
     * table covers icons 168..175; outside that range the amount is 0
     * (no credit). */
    M11_FoodWaterState s;
    m11_fw_init(&s, 1);
    /* Drain to a low value to make the credit observable. */
    s.champions[0].food = 100;
    s.champions[0].water = 100;
    s.champions[0].starved = 1;
    s.champions[0].thirsty = 1;

    /* Drive the table to confirm source-locked amounts. The static
     * helper is intentionally a one-shot, so we drive it through the
     * public consume helpers when wiring from inventory to state. */

    /* Apple (icon 168) → +500 food. */
    int appleAmt = 0;
    {
        extern int dm1_inventory_food_amount_from_icon_pc34(int iconIndex);
        appleAmt = dm1_inventory_food_amount_from_icon_pc34(168);
    }
    check_int("handoff.apple_amount", appleAmt, 500);
    m11_fw_eat(&s, 0, appleAmt, 9001);
    check_int("handoff.apple_credit", s.champions[0].food, 600);
    check_int("handoff.apple_clears_starved", s.champions[0].starved, 0);

    /* Dragon steak (icon 175) → +1400 food, but capped at 1000. */
    int steakAmt = 0;
    {
        extern int dm1_inventory_food_amount_from_icon_pc34(int iconIndex);
        steakAmt = dm1_inventory_food_amount_from_icon_pc34(175);
    }
    check_int("handoff.steak_amount", steakAmt, 1400);
    m11_fw_eat(&s, 0, steakAmt, 9002);
    check_int("handoff.steak_capped", s.champions[0].food, 1000);

    /* Out-of-range icon → 0 amount, no credit applied. */
    int junkAmt = 0;
    {
        extern int dm1_inventory_food_amount_from_icon_pc34(int iconIndex);
        junkAmt = dm1_inventory_food_amount_from_icon_pc34(50);
    }
    check_int("handoff.junk_amount", junkAmt, 0);
    int beforeFood = s.champions[0].food;
    m11_fw_eat(&s, 0, junkAmt, 9003);
    check_int("handoff.junk_no_credit", s.champions[0].food, beforeFood);

    /* Waterskin drink: F0349:1824..1844 waterskin adds 800 water per
     * charge. Starting from 100, two charges overshoot the 1000 cap,
     * so the post-state is capped at 1000. */
    m11_fw_drink(&s, 0, 800, 9100);
    m11_fw_drink(&s, 0, 800, 9101);
    check_int("handoff.waterskin_two_charges_capped", s.champions[0].water, 1000);
    check_int("handoff.waterskin_clears_thirsty", s.champions[0].thirsty, 0);

    /* Drain again, then verify a single in-range charge sits at 600. */
    s.champions[0].water = 0;
    s.champions[0].thirsty = 1;
    m11_fw_drink(&s, 0, 800, 9200);
    check_int("handoff.waterskin_one_charge", s.champions[0].water, 800);
    check_int("handoff.waterskin_one_charge_no_thirsty", s.champions[0].thirsty, 0);
}

static void test_decay_then_consume_round_trip(void)
{
    /* End-to-end lifecycle: a champion starts full, decays to zero,
     * becomes starved/thirsty, then consumes an inventory apple and
     * a waterskin-charge-worth of water. After the consume the starve
     * and thirsty flags must be cleared and food/water must reflect
     * the credited amount. */
    M11_FoodWaterState s;
    m11_fw_init(&s, 1);
    /* Drive 400 ticks at 3 food / 3 water each: 400*3 = 1200 — well
     * past the 1000 cap, so we are guaranteed to be at 0,0 + starved. */
    for (int i = 0; i < 400; ++i) {
        m11_fw_tick(&s, i);
    }
    check_int("rt.food_floor", s.champions[0].food, 0);
    check_int("rt.water_floor", s.champions[0].water, 0);
    check_int("rt.starved", s.champions[0].starved, 1);
    check_int("rt.thirsty", s.champions[0].thirsty, 1);
    check_int("rt.starve_dmg", m11_fw_starvation_damage(&s, 0), 4);

    /* Consume an apple (500 food) and a water flask (1600 water,
     * clamped to 1000 in m11_fw_drink). */
    m11_fw_eat(&s, 0, 500, 12345);
    m11_fw_drink(&s, 0, 1600, 12346);
    check_int("rt.food_after_eat", s.champions[0].food, 500);
    check_int("rt.water_after_drink", s.champions[0].water, 1000);
    check_int("rt.clears_starved", s.champions[0].starved, 0);
    check_int("rt.clears_thirsty", s.champions[0].thirsty, 0);
    check_int("rt.starve_dmg_after_eat", m11_fw_starvation_damage(&s, 0), 0);

    /* Tick once after eat/drink: decay resumes. */
    m11_fw_tick(&s, 99999);
    check_int("rt.food_decay_resume", s.champions[0].food, 497);
    check_int("rt.water_decay_resume", s.champions[0].water, 997);
}

int main(void)
{
    printf("== DM1 V1 food/water state slice ==\n");
    printf("source=F0331_CHAMPION_ApplyTimeEffects_CPSF@CHAMPION.C:2360..2418 "
           "F0349_CHAMPION_HandleMouthUseLocal@PANEL.C:1743..1945 "
           "DUNGEON.C:428..436 G0242 food amounts\n");
    test_init();
    test_tick_decay();
    test_eat_drink();
    test_getters();
    test_starvation_damage();
    test_inventory_handoff_lifecycle();
    test_decay_then_consume_round_trip();

    /* Total assertions should be at least 75 across all subtests. */
    check_int_ge("summary.assertions_min", g_assertions, 75);

    if (g_failures != 0) {
        printf("FAIL dm1_v1_food_water_pc34_compat failures=%d assertions=%d\n",
               g_failures, g_assertions);
        return 1;
    }
    printf("PASS dm1_v1_food_water_pc34_compat assertions=%d\n", g_assertions);
    return 0;
}
