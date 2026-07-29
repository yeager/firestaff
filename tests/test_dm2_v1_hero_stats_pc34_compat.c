#include "dm2_v1_hero_stats_pc34_compat.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static DM2_V1_HeroStats make_default_hero(void) {
    DM2_V1_HeroStats h;
    memset(&h, 0, sizeof(h));
    for (int i = 0; i < DM2_V1_HERO_NUM_ABILITIES; i++) {
        h.ability[i].current = 50;
        h.ability[i].maximum = 60;
        h.eability[i] = 0;
    }
    h.cur_stamina = 200;
    h.max_stamina = 200;
    return h;
}

static void test_adj_ability1_basic(void) {
    DM2_V1_HeroStats h = make_default_hero();
    DM2_V1_HeroGetAdjAbility1Receipt r;

    dm2_v1_hero_get_adj_ability1(&h, DM2_V1_HERO_ABILITY_STRENGTH, 1, 0, &r);
    assert(r.valid);
    assert(r.value == 50);

    dm2_v1_hero_get_adj_ability1(&h, DM2_V1_HERO_ABILITY_STRENGTH, 0, 0, &r);
    assert(r.valid);
    assert(r.value == 60);
}

static void test_adj_ability1_with_enchantment(void) {
    DM2_V1_HeroStats h = make_default_hero();
    h.ench_aura = 5;
    h.ench_power = 80;

    DM2_V1_HeroGetAdjAbility1Receipt r;
    dm2_v1_hero_get_adj_ability1(&h, DM2_V1_HERO_ABILITY_WIZARD, 1, 10, &r);
    assert(r.valid);
    assert(r.value > 50);
}

static void test_adj_ability1_eability_offset(void) {
    DM2_V1_HeroStats h = make_default_hero();
    h.eability[DM2_V1_HERO_ABILITY_STRENGTH] = 10;

    DM2_V1_HeroGetAdjAbility1Receipt r;
    dm2_v1_hero_get_adj_ability1(&h, DM2_V1_HERO_ABILITY_STRENGTH, 1, 0, &r);
    assert(r.valid);
    assert(r.value == 60);
}

static void test_adj_ability1_clamp_low(void) {
    DM2_V1_HeroStats h = make_default_hero();
    h.ability[DM2_V1_HERO_ABILITY_STRENGTH].current = 1;
    h.eability[DM2_V1_HERO_ABILITY_STRENGTH] = -20;

    DM2_V1_HeroGetAdjAbility1Receipt r;
    dm2_v1_hero_get_adj_ability1(&h, DM2_V1_HERO_ABILITY_STRENGTH, 1, 0, &r);
    assert(r.valid);
    assert(r.value == 10);
}

static void test_adj_ability1_clamp_high(void) {
    DM2_V1_HeroStats h = make_default_hero();
    h.ability[DM2_V1_HERO_ABILITY_STRENGTH].current = 200;
    h.eability[DM2_V1_HERO_ABILITY_STRENGTH] = 50;

    DM2_V1_HeroGetAdjAbility1Receipt r;
    dm2_v1_hero_get_adj_ability1(&h, DM2_V1_HERO_ABILITY_STRENGTH, 1, 0, &r);
    assert(r.valid);
    assert(r.value == 220);
}

static void test_adj_ability2_basic(void) {
    DM2_V1_HeroStats h = make_default_hero();
    DM2_V1_HeroGetAdjAbility2Receipt r;

    dm2_v1_hero_get_adj_ability2(&h, DM2_V1_HERO_ABILITY_STRENGTH, 100, 0, &r);
    assert(r.valid);
    /* 170 - 50 = 120 >= 16, so result = (100 * 7) >> 120 = 0 (shift too large) */
    /* Actually shift of 120 would be >= 32 so result = 0 */
    assert(r.value == 0);
}

static void test_adj_ability2_high_ability(void) {
    DM2_V1_HeroStats h = make_default_hero();
    h.ability[DM2_V1_HERO_ABILITY_STRENGTH].current = 160;
    DM2_V1_HeroGetAdjAbility2Receipt r;

    dm2_v1_hero_get_adj_ability2(&h, DM2_V1_HERO_ABILITY_STRENGTH, 100, 0, &r);
    assert(r.valid);
    /* 170 - 160 = 10 < 16, so result = 100 / 8 = 12 */
    assert(r.value == 12);
}

static void test_stamina_adj_full(void) {
    DM2_V1_HeroStats h = make_default_hero();
    DM2_V1_HeroGetStaminaAdjReceipt r;

    dm2_v1_hero_get_stamina_adj(&h, 100, &r);
    assert(r.valid);
    assert(r.value == 100);
}

static void test_stamina_adj_half(void) {
    DM2_V1_HeroStats h = make_default_hero();
    h.cur_stamina = 50; /* < max/2 = 100 */
    DM2_V1_HeroGetStaminaAdjReceipt r;

    dm2_v1_hero_get_stamina_adj(&h, 100, &r);
    assert(r.valid);
    /* half_input = 50, (50 * 50) / 100 + 50 = 25 + 50 = 75 */
    assert(r.value == 75);
}

static void test_max_load_basic(void) {
    DM2_V1_HeroStats h = make_default_hero();
    DM2_V1_HeroGetMaxLoadReceipt r;

    dm2_v1_hero_get_max_load(&h, 0, &r);
    assert(r.valid);
    assert(r.value > 0);
    assert(r.value % 10 == 0);
}

static void test_max_load_with_body_flag(void) {
    DM2_V1_HeroStats h = make_default_hero();
    h.body_flag = 0x10;

    DM2_V1_HeroGetMaxLoadReceipt r1;
    dm2_v1_hero_get_max_load(&h, 0, &r1);

    h.body_flag = 0;
    DM2_V1_HeroGetMaxLoadReceipt r2;
    dm2_v1_hero_get_max_load(&h, 0, &r2);

    assert(r1.valid && r2.valid);
    assert(r1.value < r2.value);
}

static void test_null_inputs(void) {
    DM2_V1_HeroGetAdjAbility1Receipt r1;
    assert(!dm2_v1_hero_get_adj_ability1(NULL, DM2_V1_HERO_ABILITY_STRENGTH, 1, 0, &r1));

    DM2_V1_HeroGetStaminaAdjReceipt r2;
    assert(!dm2_v1_hero_get_stamina_adj(NULL, 100, &r2));

    DM2_V1_HeroGetMaxLoadReceipt r3;
    assert(!dm2_v1_hero_get_max_load(NULL, 0, &r3));
}

int main(void) {
    test_adj_ability1_basic();
    test_adj_ability1_with_enchantment();
    test_adj_ability1_eability_offset();
    test_adj_ability1_clamp_low();
    test_adj_ability1_clamp_high();
    test_adj_ability2_basic();
    test_adj_ability2_high_ability();
    test_stamina_adj_full();
    test_stamina_adj_half();
    test_max_load_basic();
    test_max_load_with_body_flag();
    test_null_inputs();
    printf("All dm2_v1_hero_stats tests passed.\n");
    return 0;
}
