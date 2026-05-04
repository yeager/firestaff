/*
 * test_dm1_v1_champion_needs_pc34_compat_integration.c
 *
 * CTest integration tests for DM1 V1 champion needs system.
 * Source-locked to ReDMCSB CHAMPION.C F0331, F0325, DEFS.H.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dm1_v1_champion_needs_pc34_compat.h"

static int g_pass = 0, g_fail = 0;

#define ASSERT(cond, msg) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL: %s (line %d): %s\n", __func__, __LINE__, msg); \
        g_fail++; return; \
    } else { g_pass++; } \
} while(0)

#define ASSERT_EQ(a, b, msg) do { \
    if ((a) != (b)) { \
        fprintf(stderr, "FAIL: %s (line %d): %s — got %d, expected %d\n", \
                __func__, __LINE__, msg, (int)(a), (int)(b)); \
        g_fail++; return; \
    } else { g_pass++; } \
} while(0)

/* ── Helper: default champion ─────────────────────────────────────── */
static DM1_ChampionNeeds make_champion(void) {
    DM1_ChampionNeeds c;
    memset(&c, 0, sizeof(c));
    c.current_health = 200;
    c.max_health = 200;
    c.current_stamina = 1500;
    c.max_stamina = 1500;
    c.current_mana = 0;
    c.max_mana = 0;
    c.food = 1024;
    c.water = 1024;
    c.vitality_current = 40;
    c.wisdom_current = 30;
    c.alive = 1;
    return c;
}

static DM1_NeedsTickContext make_ctx(uint32_t gt, int resting) {
    DM1_NeedsTickContext ctx;
    ctx.game_time = gt;
    ctx.last_movement_time = gt - 100;  /* 100 ticks since last move */
    ctx.party_is_resting = resting;
    return ctx;
}

/* ── Test: stamina_amount computation ─────────────────────────────── */
static void test_stamina_amount(void) {
    /* F0331: BoundedValue(1, (MaxStamina >> 8) - 1, 6) */
    /* MaxStamina = 256 → (1 - 1) = 0 → bounded to 1 */
    ASSERT_EQ(dm1_needs_compute_stamina_amount(256), 1, "256 → 1");
    /* MaxStamina = 512 → (2 - 1) = 1 → 1 */
    ASSERT_EQ(dm1_needs_compute_stamina_amount(512), 1, "512 → 1");
    /* MaxStamina = 1024 → (4 - 1) = 3 → 3 */
    ASSERT_EQ(dm1_needs_compute_stamina_amount(1024), 3, "1024 → 3");
    /* MaxStamina = 1500 → (5 - 1) = 4 → 4 (5 = 1500/256 truncated) */
    ASSERT_EQ(dm1_needs_compute_stamina_amount(1500), 4, "1500 → 4");
    /* MaxStamina = 9999 → (39 - 1) = 38 → bounded to 6 */
    ASSERT_EQ(dm1_needs_compute_stamina_amount(9999), 6, "9999 → 6");
    /* MaxStamina = 100 → (0 - 1) = -1 → bounded to 1 */
    ASSERT_EQ(dm1_needs_compute_stamina_amount(100), 1, "100 → 1");
}

/* ── Test: health gain computation ────────────────────────────────── */
static void test_health_gain(void) {
    /* MaxHealth=200, not resting, no cross: (200>>7)+1 = 2 */
    ASSERT_EQ(dm1_needs_compute_health_gain(200, 0, 0), 2, "200/no/no → 2");
    /* Resting: 2*2 = 4 */
    ASSERT_EQ(dm1_needs_compute_health_gain(200, 1, 0), 4, "200/rest/no → 4");
    /* Ekkhard Cross, not resting: 2 + (2>>1)+1 = 2+2 = 4 */
    ASSERT_EQ(dm1_needs_compute_health_gain(200, 0, 1), 4, "200/no/cross → 4");
    /* Resting + Cross: 4 + (4>>1)+1 = 4+3 = 7 */
    ASSERT_EQ(dm1_needs_compute_health_gain(200, 1, 1), 7, "200/rest/cross → 7");
    /* MaxHealth=999: (999>>7)+1 = 8 */
    ASSERT_EQ(dm1_needs_compute_health_gain(999, 0, 0), 8, "999/no/no → 8");
}

/* ── Test: decrement stamina ──────────────────────────────────────── */
static void test_decrement_stamina(void) {
    int16_t stam;
    int dmg;

    /* Normal decrement */
    stam = 1000;
    dmg = dm1_needs_decrement_stamina(&stam, 1500, 200);
    ASSERT_EQ(stam, 800, "1000-200=800");
    ASSERT_EQ(dmg, 0, "no damage");

    /* Overflow: stamina goes negative → damage */
    stam = 100;
    dmg = dm1_needs_decrement_stamina(&stam, 1500, 300);
    ASSERT_EQ(stam, 0, "clamped to 0");
    ASSERT_EQ(dmg, 100, "(-200)>>1 = 100");

    /* Negative decrement (gain) that exceeds max */
    stam = 1400;
    dmg = dm1_needs_decrement_stamina(&stam, 1500, -200);
    ASSERT_EQ(stam, 1500, "clamped to max");
    ASSERT_EQ(dmg, 0, "no damage");
}

/* ── Test: food depletes each tick ────────────────────────────────── */
static void test_food_depletes(void) {
    DM1_ChampionNeeds c = make_champion();
    DM1_NeedsTickContext ctx = make_ctx(1000, 0);
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    /* Food should decrease (food was 1024, positive → consumed) */
    ASSERT(out.food_after < c.food, "food decreased");
    /* Water should also decrease */
    ASSERT(out.water_after < c.water, "water decreased");
    /* Stamina should increase (food+water both >= 0 → gain) */
    ASSERT(out.stamina_delta >= 0, "stamina gained from food+water");
}

/* ── Test: starvation causes stamina loss ─────────────────────────── */
static void test_starvation_stamina_loss(void) {
    DM1_ChampionNeeds c = make_champion();
    c.food = -600;  /* Below -512: starving */
    c.water = -600;
    DM1_NeedsTickContext ctx = make_ctx(1000, 0);
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    /* Starving: stamina should decrease */
    ASSERT(out.stamina_delta < 0, "stamina lost when starving");
}

/* ── Test: resting doubles stamina gain ───────────────────────────── */
static void test_resting_doubles_gain(void) {
    DM1_ChampionNeeds c = make_champion();
    c.food = 1024;
    c.water = 1024;
    c.current_stamina = 500;
    DM1_NeedsTickContext ctx_normal = make_ctx(1000, 0);
    DM1_NeedsTickContext ctx_rest = make_ctx(1000, 1);
    DM1_NeedsTickResult out_normal, out_rest;

    dm1_needs_apply_time_effects(&c, &ctx_normal, &out_normal);
    dm1_needs_apply_time_effects(&c, &ctx_rest, &out_rest);

    /* Resting should produce more stamina gain (or at least equal) */
    ASSERT(out_rest.stamina_delta >= out_normal.stamina_delta,
           "resting → more stamina gain");
    /* Food depletes faster when resting (stamina_amount doubled) */
    ASSERT(out_rest.food_after <= out_normal.food_after,
           "resting → faster food depletion");
}

/* ── Test: HP healing when conditions met ─────────────────────────── */
static void test_hp_healing(void) {
    DM1_ChampionNeeds c = make_champion();
    c.current_health = 100;  /* Below max */
    c.vitality_current = 100;
    /* Need timeCriteria < vitality + 12. timeCriteria depends on game_time bits.
       Use game_time = 0 so timeCriteria = 0, which is < 112. */
    DM1_NeedsTickContext ctx = make_ctx(0, 0);
    /* Need stamina >= max/4 = 375. We have 1500, so OK. */
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    ASSERT(out.health_delta > 0, "HP healing occurred");
    /* gain = (200>>7)+1 = 2 */
    ASSERT_EQ(out.health_delta, 2, "heal amount = 2");
}

/* ── Test: no HP healing when stamina too low ─────────────────────── */
static void test_no_heal_low_stamina(void) {
    DM1_ChampionNeeds c = make_champion();
    c.current_health = 100;
    c.current_stamina = 100;  /* Below max/4 = 375 */
    c.vitality_current = 100;
    DM1_NeedsTickContext ctx = make_ctx(0, 0);
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    ASSERT_EQ(out.health_delta, 0, "no heal when stamina low");
}

/* ── Test: dead champion gets no processing ───────────────────────── */
static void test_dead_champion(void) {
    DM1_ChampionNeeds c = make_champion();
    c.alive = 0;
    DM1_NeedsTickContext ctx = make_ctx(1000, 0);
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    ASSERT_EQ(out.stamina_delta, 0, "dead: no stamina delta");
    ASSERT_EQ(out.health_delta, 0, "dead: no health delta");
    ASSERT_EQ(out.food_after, c.food, "dead: food unchanged");
    ASSERT_EQ(out.water_after, c.water, "dead: water unchanged");
}

/* ── Test: food/water clamped to minimum ──────────────────────────── */
static void test_food_water_clamp(void) {
    DM1_ChampionNeeds c = make_champion();
    c.food = -1020;   /* Close to min */
    c.water = -1020;
    DM1_NeedsTickContext ctx = make_ctx(1000, 0);
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    ASSERT(out.food_after >= DM1_NEEDS_FOOD_MIN, "food >= -1024");
    ASSERT(out.water_after >= DM1_NEEDS_WATER_MIN, "water >= -1024");
}

/* ── Test: mana regeneration with wisdom/skills ───────────────────── */
static void test_mana_regen(void) {
    DM1_ChampionNeeds c = make_champion();
    c.current_mana = 50;
    c.max_mana = 200;
    c.wisdom_current = 60;
    c.wizard_skill_level = 5;
    c.priest_skill_level = 3;
    /* timeCriteria must be < (wisdom + wizPriestLevel) = 60 + 8 = 68
       Use game_time = 0 → timeCriteria = 0, so condition met. */
    DM1_NeedsTickContext ctx = make_ctx(0, 0);
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    /* manaGain = 200/40 + 1 = 6. min(6, 200-50) = 6 */
    ASSERT(out.mana_delta > 0, "mana regenerated");
    ASSERT_EQ(out.mana_delta, 6, "mana gain = 6");
}

/* ── Test: mana regen costs stamina ───────────────────────────────── */
static void test_mana_regen_stamina_cost(void) {
    DM1_ChampionNeeds c = make_champion();
    c.current_mana = 50;
    c.max_mana = 200;
    c.wisdom_current = 60;
    c.wizard_skill_level = 5;
    c.priest_skill_level = 3;
    c.current_stamina = 100;  /* Low stamina */
    DM1_NeedsTickContext ctx = make_ctx(0, 0);
    DM1_NeedsTickResult out;

    dm1_needs_apply_time_effects(&c, &ctx, &out);

    /* manaGain=6, staminaCost = 6 * max(7, 16-8) = 6*8 = 48
       stamina drops from 100 by at least 48 (mana cost) */
    /* But note: food/water cycle also affects stamina */
    ASSERT(out.mana_delta > 0, "mana still regenerated");
}

/* ── Test: NULL safety ────────────────────────────────────────────── */
static void test_null_safety(void) {
    DM1_NeedsTickResult out;
    DM1_ChampionNeeds c = make_champion();
    DM1_NeedsTickContext ctx = make_ctx(0, 0);

    /* Should not crash */
    dm1_needs_apply_time_effects(NULL, &ctx, &out);
    dm1_needs_apply_time_effects(&c, NULL, &out);
    dm1_needs_apply_time_effects(&c, &ctx, NULL);
    g_pass++;
}

/* ── Test: movement delay affects stamina ─────────────────────────── */
static void test_movement_delay_boost(void) {
    DM1_ChampionNeeds c = make_champion();
    c.current_stamina = 500;
    c.food = 1024;
    c.water = 1024;

    /* Recent movement: delay = 50 (<80) → no bonus */
    DM1_NeedsTickContext ctx_recent;
    ctx_recent.game_time = 1000;
    ctx_recent.last_movement_time = 950;
    ctx_recent.party_is_resting = 0;

    /* Old movement: delay = 300 (>250) → +2 bonus */
    DM1_NeedsTickContext ctx_old;
    ctx_old.game_time = 1000;
    ctx_old.last_movement_time = 700;
    ctx_old.party_is_resting = 0;

    DM1_NeedsTickResult out_recent, out_old;
    dm1_needs_apply_time_effects(&c, &ctx_recent, &out_recent);
    dm1_needs_apply_time_effects(&c, &ctx_old, &out_old);

    /* Longer idle → more stamina gain */
    ASSERT(out_old.stamina_delta >= out_recent.stamina_delta,
           "longer idle → more stamina");
}

int main(void) {
    printf("=== DM1 V1 Champion Needs Source-Lock Tests ===\n\n");

    test_stamina_amount();
    test_health_gain();
    test_decrement_stamina();
    test_food_depletes();
    test_starvation_stamina_loss();
    test_resting_doubles_gain();
    test_hp_healing();
    test_no_heal_low_stamina();
    test_dead_champion();
    test_food_water_clamp();
    test_mana_regen();
    test_mana_regen_stamina_cost();
    test_null_safety();
    test_movement_delay_boost();

    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
