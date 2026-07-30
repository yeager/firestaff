#include "dm2_v1_party.h"
#include <assert.h>
#include <string.h>

int main(void) {
    DM2_V1_Hero hero;
    DM2_V1_Party party;

    /* Hero struct is 263 bytes (savegame layout) */
    assert(sizeof(DM2_V1_Hero) == 263);

    /* Init zeroes all fields */
    dm2_v1_hero_init(&hero);
    assert(hero.curHP == 0);
    assert(hero.maxHP == 0);
    assert(hero.heroflag == 0);
    assert(hero.ench_power == 0);

    /* Party init */
    dm2_v1_party_init(&party);
    assert(party.heros_in_party == 0);
    assert(party.curactevhero == DM2_HERO_NONE);
    assert(party.handitems[0] == -1);

    /* set_hero_flags sets 0x4000 on all heroes */
    party.heros_in_party = 2;
    party.hero[0].heroflag = 0x0001;
    party.hero[1].heroflag = 0x0002;
    dm2_v1_party_set_hero_flags(&party);
    assert(party.hero[0].heroflag == 0x4001);
    assert(party.hero[1].heroflag == 0x4002);

    /* get_adj_ability1 basic: no enchantment */
    dm2_v1_hero_init(&hero);
    hero.ability[DM2_ABILITY_STRENGTH][DM2_CUR] = 50;
    hero.ability[DM2_ABILITY_STRENGTH][DM2_MAX] = 60;
    hero.eability[DM2_ABILITY_STRENGTH] = 5;
    int16_t adj = dm2_v1_hero_get_adj_ability1_raw(
        &hero, DM2_ABILITY_STRENGTH, DM2_CUR, 0);
    assert(adj == 55);

    /* get_adj_ability1 clamped to [10, 220] */
    hero.ability[DM2_ABILITY_LUCK][DM2_CUR] = 5;
    hero.eability[DM2_ABILITY_LUCK] = 0;
    adj = dm2_v1_hero_get_adj_ability1_raw(
        &hero, DM2_ABILITY_LUCK, DM2_CUR, 0);
    assert(adj == 10);

    /* get_stamina_adj: full stamina passes through */
    dm2_v1_hero_init(&hero);
    hero.curStamina = 100;
    hero.maxStamina = 100;
    int16_t sa = dm2_v1_hero_get_stamina_adj_raw(&hero, 80);
    assert(sa == 80);

    /* get_stamina_adj: half stamina reduces */
    hero.curStamina = 25;
    hero.maxStamina = 100;
    sa = dm2_v1_hero_get_stamina_adj_raw(&hero, 80);
    assert(sa == 60);

    /* get_max_load: basic computation */
    dm2_v1_hero_init(&hero);
    hero.ability[DM2_ABILITY_STRENGTH][DM2_CUR] = 50;
    hero.ability[DM2_ABILITY_STRENGTH][DM2_MAX] = 60;
    hero.curStamina = 100;
    hero.maxStamina = 100;
    int16_t ml = dm2_v1_hero_get_max_load_raw(&hero, 0);
    assert(ml > 0);
    assert(ml % 10 == 0);

    /* 2c1d_0e23: stamina cost from weight */
    assert(dm2_v1_hero_2c1d_0e23(0) == 1);
    assert(dm2_v1_hero_2c1d_0e23(4) == 2);
    assert(dm2_v1_hero_2c1d_0e23(20) == 10);

    /* hero_2c1d_0300: ability adjustment with diminishing curve */
    dm2_v1_hero_init(&hero);
    hero.ability[DM2_ABILITY_STRENGTH][DM2_CUR] = 50;
    hero.ability[DM2_ABILITY_STRENGTH][DM2_MAX] = 60;
    dm2_v1_hero_2c1d_0300(&hero, DM2_ABILITY_STRENGTH, 5);
    assert(hero.ability[DM2_ABILITY_STRENGTH][DM2_CUR] == 55);

    /* Small delta within 20 of max: no diminishing */
    dm2_v1_hero_init(&hero);
    hero.ability[DM2_ABILITY_DEXTERITY][DM2_CUR] = 45;
    hero.ability[DM2_ABILITY_DEXTERITY][DM2_MAX] = 60;
    dm2_v1_hero_2c1d_0300(&hero, DM2_ABILITY_DEXTERITY, 10);
    assert(hero.ability[DM2_ABILITY_DEXTERITY][DM2_CUR] == 55);

    /* Clamp to [10, 220] */
    dm2_v1_hero_init(&hero);
    hero.ability[DM2_ABILITY_LUCK][DM2_CUR] = 5;
    hero.ability[DM2_ABILITY_LUCK][DM2_MAX] = 5;
    dm2_v1_hero_2c1d_0300(&hero, DM2_ABILITY_LUCK, -10);
    assert(hero.ability[DM2_ABILITY_LUCK][DM2_CUR] == 10);

    /* hero_37bea: special force per hero */
    dm2_v1_party_init(&party);
    party.heros_in_party = 2;
    party.hero[0].curHP = 50;
    party.hero[0].heroflag = 0;
    party.hero[1].curHP = 0;
    assert(dm2_v1_hero_37bea(&party, 0, 30) == 0x32);
    assert(dm2_v1_hero_37bea(&party, 1, 30) == 0);
    /* No weight, no flag -> 0x28 */
    assert(dm2_v1_hero_37bea(&party, 0, 0) == 0x28);

    /* get_party_special_force: sum of contributions */
    dm2_v1_party_init(&party);
    party.heros_in_party = 2;
    party.hero[0].curHP = 50;
    party.hero[0].heroflag = 0;
    party.hero[1].curHP = 40;
    party.hero[1].heroflag = 0x10;
    int16_t weights[4] = {30, 0, 0, 0};
    assert(dm2_v1_get_party_special_force(&party, weights) == (0x32 + 0x32));

    /* reset_squad_dir: all heroes face same direction */
    dm2_v1_party_init(&party);
    party.heros_in_party = 3;
    party.hero[0].absdir = 0;
    party.hero[1].absdir = 1;
    party.hero[2].absdir = 2;
    dm2_v1_party_reset_squad_dir(&party, 3);
    assert(party.hero[0].absdir == 3);
    assert(party.hero[1].absdir == 3);
    assert(party.hero[2].absdir == 3);

    /* select_champion_leader: basic selection */
    dm2_v1_party_init(&party);
    party.heros_in_party = 2;
    party.hero[0].curHP = 50;
    party.hero[0].heroflag = 0;
    party.hero[1].curHP = 40;
    party.hero[1].heroflag = 0;
    dm2_v1_party_select_champion_leader(&party, 1, -1, 3);
    assert(party.curactevhero == DM2_HERO_1);
    assert((party.hero[1].heroflag & 0x1400) == 0x1400);

    /* select_champion_leader: dead hero rejected */
    dm2_v1_party_init(&party);
    party.heros_in_party = 2;
    party.hero[0].curHP = 0;
    dm2_v1_party_select_champion_leader(&party, 0, -1, 2);
    assert(party.curactevhero == DM2_HERO_NONE);

    /* select_champion_leader: same leader is no-op */
    dm2_v1_party_init(&party);
    party.heros_in_party = 1;
    party.hero[0].curHP = 50;
    party.curactevhero = DM2_HERO_0;
    dm2_v1_party_select_champion_leader(&party, 0, 0, 1);
    assert(party.hero[0].heroflag == 0);

    /* adjust_hand_cooldown: single hand */
    dm2_v1_hero_init(&hero);
    dm2_v1_hero_adjust_hand_cooldown(&hero, 0, 10, 0);
    assert(hero.handcooldown[0] > 0);
    assert(hero.handcooldown[1] == 0);

    /* adjust_hand_cooldown: all hands (hand_idx = -1) */
    dm2_v1_hero_init(&hero);
    dm2_v1_hero_adjust_hand_cooldown(&hero, -1, 20, 0);
    assert(hero.handcooldown[0] > 0);
    assert(hero.handcooldown[1] > 0);
    assert(hero.handcooldown[2] > 0);

    /* adjust_hand_cooldown: savegames1_b04 reduces delay */
    dm2_v1_hero_init(&hero);
    dm2_v1_hero_adjust_hand_cooldown(&hero, 0, 40, 0);
    int8_t cd_normal = hero.handcooldown[0];
    dm2_v1_hero_init(&hero);
    dm2_v1_hero_adjust_hand_cooldown(&hero, 0, 40, 1);
    assert(hero.handcooldown[0] < cd_normal);

    /* use_dexterity_attribute: basic range check */
    dm2_v1_hero_init(&hero);
    hero.ability[DM2_ABILITY_DEXTERITY][DM2_CUR] = 50;
    hero.ability[DM2_ABILITY_DEXTERITY][DM2_MAX] = 60;
    int16_t dex_result = dm2_v1_hero_use_dexterity_attribute_raw(
        &hero, 30, 500, 0, 3, 3, 3);
    assert(dex_result >= 1 && dex_result <= 100);

    /* use_dexterity_attribute: sleep flag halves */
    int16_t dex_awake = dm2_v1_hero_use_dexterity_attribute_raw(
        &hero, 30, 500, 0, 0, 0, 0);
    int16_t dex_sleep = dm2_v1_hero_use_dexterity_attribute_raw(
        &hero, 30, 500, 1, 0, 0, 0);
    assert(dex_sleep <= dex_awake);

    /* timproc_3a15_1da8: ornate animator toggle */
    assert(dm2_v1_timproc_3a15_1da8(0, 0) == 1);
    assert(dm2_v1_timproc_3a15_1da8(1, 0) == 0);
    assert(dm2_v1_timproc_3a15_1da8(2, 0) == 1);
    assert(dm2_v1_timproc_3a15_1da8(2, 1) == 0);
    assert(dm2_v1_timproc_3a15_1da8(3, 0) == 0);
    assert(dm2_v1_timproc_3a15_1da8(5, 99) == 0);

    return 0;
}
