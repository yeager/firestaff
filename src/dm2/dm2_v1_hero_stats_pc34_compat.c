#include "dm2_v1_hero_stats_pc34_compat.h"

/*
 * DM2 hero stat computation functions.
 * Source: skproject c_hero.cpp:80-133
 */

static int16_t clamp_i16(int16_t minv, int16_t maxv, int16_t value) {
    if (value < minv) return minv;
    if (value > maxv) return maxv;
    return value;
}

/* skproject c_hero.cpp:81-98  get_adj_ability1 (DM2_GET_PLAYER_ABILITY) */
int dm2_v1_hero_get_adj_ability1(
    const DM2_V1_HeroStats *hero,
    DM2_V1_HeroAbility ability,
    int use_current,
    int16_t rand16_value,
    DM2_V1_HeroGetAdjAbility1Receipt *out)
{
    if (!hero || !out || (unsigned)ability >= DM2_V1_HERO_NUM_ABILITIES) {
        if (out) out->valid = 0;
        return 0;
    }

    int16_t a = use_current
        ? hero->ability[ability].current
        : hero->ability[ability].maximum;

    if (use_current && hero->ench_power != 0) {
        uint16_t w = (uint16_t)hero->ench_aura;
        if (w >= 3 && w <= 6) {
            int adj_ability = (int)w - 2;
            if ((int)ability == adj_ability) {
                int16_t ep = hero->ench_power;
                if (ep > 100) ep = 100;
                int16_t limit = (int16_t)(((int32_t)ep * (int32_t)a) >> 7) + 1;
                int16_t rand_mod = (limit > 0) ? (rand16_value % limit) : 0;
                a += rand_mod + 4;
            }
        }
    }

    out->valid = 1;
    out->value = clamp_i16(10, 220,
        (int16_t)((int16_t)hero->eability[ability] + a));
    return 1;
}

/* skproject c_hero.cpp:100-107  get_adj_ability2 */
int dm2_v1_hero_get_adj_ability2(
    const DM2_V1_HeroStats *hero,
    DM2_V1_HeroAbility ability,
    int16_t input,
    int16_t rand16_value,
    DM2_V1_HeroGetAdjAbility2Receipt *out)
{
    if (!hero || !out || (unsigned)ability >= DM2_V1_HERO_NUM_ABILITIES) {
        if (out) out->valid = 0;
        return 0;
    }

    DM2_V1_HeroGetAdjAbility1Receipt a1;
    dm2_v1_hero_get_adj_ability1(hero, ability, 1, rand16_value, &a1);
    if (!a1.valid) {
        out->valid = 0;
        return 0;
    }

    int16_t w = 170 - a1.value;
    out->valid = 1;
    if (w < 16) {
        out->value = (int32_t)((uint16_t)input) / 8;
    } else {
        /* DM2_ATIMESB_RSHIFTC(input, 7, w) = (input * 7) >> w
         * but clamped: skproject atimesb_rshiftc does (a * b) >> c */
        int32_t product = (int32_t)input * 7;
        if (w >= 32)
            out->value = 0;
        else
            out->value = product >> w;
    }
    return 1;
}

/* skproject c_hero.cpp:109-123  get_stamina_adj (DM2_STAMINA_ADJUSTED_ATTR) */
int dm2_v1_hero_get_stamina_adj(
    const DM2_V1_HeroStats *hero,
    int16_t input,
    DM2_V1_HeroGetStaminaAdjReceipt *out)
{
    if (!hero || !out) {
        if (out) out->valid = 0;
        return 0;
    }

    int16_t mhalf = (int16_t)(hero->max_stamina >> 1);
    out->valid = 1;

    if ((int16_t)hero->cur_stamina < mhalf) {
        int16_t half_input = input >> 1;
        if (mhalf != 0)
            out->value = (int16_t)(((int32_t)hero->cur_stamina * (int32_t)half_input) / mhalf) + half_input;
        else
            out->value = half_input;
    } else {
        out->value = input;
    }
    return 1;
}

/* skproject c_hero.cpp:125-133  get_max_load (DM2_MAX_LOAD) */
int dm2_v1_hero_get_max_load(
    const DM2_V1_HeroStats *hero,
    int16_t rand16_value,
    DM2_V1_HeroGetMaxLoadReceipt *out)
{
    if (!hero || !out) {
        if (out) out->valid = 0;
        return 0;
    }

    DM2_V1_HeroGetAdjAbility1Receipt a1;
    dm2_v1_hero_get_adj_ability1(hero, DM2_V1_HERO_ABILITY_STRENGTH, 1,
                                  rand16_value, &a1);
    if (!a1.valid) {
        out->valid = 0;
        return 0;
    }

    DM2_V1_HeroGetStaminaAdjReceipt sa;
    int16_t raw = (int16_t)(8 * a1.value + 100);
    dm2_v1_hero_get_stamina_adj(hero, raw, &sa);
    if (!sa.valid) {
        out->valid = 0;
        return 0;
    }

    int16_t s = sa.value;
    if (hero->body_flag != 0) {
        int shift = ((hero->body_flag & 0x10) == 0 ? 1 : 0) + 2;
        s = s - (s >> shift);
    }
    s += 9;
    s = s - (s % 10);

    out->valid = 1;
    out->value = s;
    return 1;
}

/* skproject c_hero.cpp:136-147  use_luck (DM2_USE_LUCK_ATTRIBUTE) */
int dm2_v1_hero_use_luck(
    DM2_V1_HeroStats *hero,
    int16_t threshold,
    int randbit,
    int16_t rand16_100,
    int16_t rand16_ability,
    DM2_V1_HeroUseLuckResult *out)
{
    if (!hero || !out) {
        if (out) out->valid = 0;
        return 0;
    }

    int result;
    if (randbit && rand16_100 > threshold) {
        result = 1;
    } else {
        result = rand16_ability > threshold ? 1 : 0;
    }

    int16_t luck_cur = (int16_t)hero->ability[DM2_V1_HERO_ABILITY_LUCK].current;
    int16_t luck_max = (int16_t)hero->ability[DM2_V1_HERO_ABILITY_LUCK].maximum;
    int16_t max_clamp = luck_max;
    if (max_clamp > 220) max_clamp = 220;

    if (result)
        luck_cur -= 2;
    else
        luck_cur += 2;

    luck_cur = clamp_i16(10, max_clamp, luck_cur);
    hero->ability[DM2_V1_HERO_ABILITY_LUCK].current = (uint8_t)luck_cur;

    out->valid = 1;
    out->result = result;
    out->new_luck = (uint8_t)luck_cur;
    return 1;
}

/* skproject c_hero.cpp:652-684  hero_2c1d_0300 */
void dm2_v1_hero_adjust_ability(
    DM2_V1_HeroStats *hero,
    int ability_idx,
    int16_t delta)
{
    if (!hero || (unsigned)ability_idx >= DM2_V1_HERO_NUM_ABILITIES)
        return;

    int16_t cur = (int16_t)hero->ability[ability_idx].current;
    int16_t max = (int16_t)hero->ability[ability_idx].maximum;

    /* Check if delta moves cur further from max (same sign as cur-max+delta) */
    int16_t gap = (int16_t)(cur - max + delta);
    int gap_neg = gap < 0;
    int delta_neg = delta < 0;

    if (gap_neg == delta_neg) {
        /* Diminishing returns: for every 20 units of |gap|, reduce delta by 3/4 */
        int16_t abs_gap = gap < 0 ? (int16_t)-gap : gap;
        while (abs_gap > 20) {
            int16_t quarter = (int16_t)(delta / 4);
            delta = (int16_t)(delta - quarter);
            abs_gap = (int16_t)(abs_gap - 20);
        }
    }

    int16_t result = (int16_t)(cur + delta);
    result = clamp_i16(10, 220, result);
    hero->ability[ability_idx].current = (uint8_t)result;
}
