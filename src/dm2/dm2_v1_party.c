#include "dm2_v1_party.h"
#include <string.h>

static int16_t dm2_between_value(int16_t lo, int16_t hi, int16_t v) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static int32_t dm2_atimesb_rshiftc(int16_t a, int16_t b, int16_t c) {
    int32_t product = (int32_t)a * (int32_t)b;
    if (c <= 0) return product;
    if (c >= 32) return 0;
    return product >> c;
}

void dm2_v1_hero_init(DM2_V1_Hero *hero) {
    memset(hero, 0, sizeof(*hero));
}

void dm2_v1_party_init(DM2_V1_Party *party) {
    int i;
    for (i = 0; i < DM2_MAX_HEROES; ++i)
        dm2_v1_hero_init(&party->hero[i]);
    party->heros_in_party = 0;
    party->absdir = 0;
    party->curactevhero = DM2_HERO_NONE;
    memset(party->hand_container, 0, sizeof(party->hand_container));
    party->curactmode = 0;
    party->curacthero = 0;
    memset(party->handitems, -1, sizeof(party->handitems));
}

/* skproject c_party::set_hero_flags (c_hero.cpp:190) */
void dm2_v1_party_set_hero_flags(DM2_V1_Party *party) {
    int i;
    for (i = 0; i < party->heros_in_party; ++i)
        party->hero[i].heroflag |= 0x4000;
}

/* skproject c_hero::get_adj_ability1 (c_hero.cpp:81) */
int16_t dm2_v1_hero_get_adj_ability1_raw(
    DM2_V1_Hero *hero, DM2_Ability abi, DM2_CurMax curmax,
    int16_t rand16_value)
{
    int16_t a = hero->ability[abi][curmax];
    if (curmax == DM2_CUR) {
        if (hero->ench_power != 0) {
            int16_t w = (uint8_t)hero->ench_aura;
            if (w >= 3 && w <= 6) {
                w -= 2;
                if ((int)abi == w) {
                    int16_t ep = hero->ench_power;
                    if (ep > 100) ep = 100;
                    int16_t range = (int16_t)(((ep * a) >> 7) + 1);
                    int16_t bonus = (rand16_value % range) + 4;
                    a = (int16_t)(a + bonus);
                }
            }
        }
    }
    return dm2_between_value(10, 220,
        (int16_t)((int16_t)hero->eability[abi] + a));
}

/* skproject c_hero::get_adj_ability2 (c_hero.cpp:101) */
int32_t dm2_v1_hero_get_adj_ability2_raw(
    DM2_V1_Hero *hero, DM2_Ability abi, int16_t input,
    int16_t rand16_value)
{
    int16_t w = (int16_t)(170 - dm2_v1_hero_get_adj_ability1_raw(
        hero, abi, DM2_CUR, rand16_value));
    if (w < 16)
        return (uint16_t)input / 8;
    return dm2_atimesb_rshiftc(input, 7, w);
}

/* skproject c_hero::get_stamina_adj (c_hero.cpp:110) */
int16_t dm2_v1_hero_get_stamina_adj_raw(DM2_V1_Hero *hero, int16_t input) {
    int16_t mhalf = (int16_t)(hero->maxStamina >> 1);
    if (mhalf <= 0) return input;
    if (hero->curStamina < mhalf) {
        input >>= 1;
        return (int16_t)(((hero->curStamina * input) / mhalf) + input);
    }
    return input;
}

/* skproject c_hero::get_max_load (c_hero.cpp:126) */
int16_t dm2_v1_hero_get_max_load_raw(DM2_V1_Hero *hero, int16_t rand16_value) {
    int16_t base = dm2_v1_hero_get_adj_ability1_raw(
        hero, DM2_ABILITY_STRENGTH, DM2_CUR, rand16_value);
    int16_t s = dm2_v1_hero_get_stamina_adj_raw(hero,
        (int16_t)(8 * base + 100));
    if (hero->bodyflag != 0)
        s = (int16_t)(s - (s >> (((hero->bodyflag & 0x10) == 0 ? 1 : 0) + 2)));
    s = (int16_t)(s + 9);
    return (int16_t)(s - (s % 10));
}

/* skproject c_hero::use_luck (c_hero.cpp:136)
 * Deterministic: caller provides RNG values. */
int dm2_v1_hero_use_luck_raw(
    DM2_V1_Hero *hero, int16_t input,
    int randbit, int16_t rand16_ability, int16_t rand16_luck)
{
    if (randbit) {
        if ((rand16_ability % 100) > input)
            return 1;
    }

    int16_t adj = dm2_v1_hero_get_adj_ability1_raw(
        hero, DM2_ABILITY_LUCK, DM2_CUR, rand16_luck);
    int result = ((rand16_luck % (2 * adj)) > input) ? 1 : 0;

    int16_t w2 = dm2_between_value(10, 220,
        dm2_v1_hero_get_adj_ability1_raw(hero, DM2_ABILITY_LUCK, DM2_MAX, 0));
    int16_t w1 = (int16_t)((result ? -2 : 2) +
        (uint8_t)hero->ability[DM2_ABILITY_LUCK][DM2_CUR]);
    hero->ability[DM2_ABILITY_LUCK][DM2_CUR] =
        (int8_t)dm2_between_value(10, w2, w1);

    return result;
}

/* skproject DM2_hero_2c1d_132c (c_hero.cpp:1383)
 * Pure arithmetic helper for wound/defense computation. */
int16_t dm2_v1_hero_2c1d_132c(int16_t a, int16_t b) {
    int16_t result = (int16_t)(a & 0xFF);
    if (b != 0) {
        uint8_t alo = (uint8_t)(a & 0xFF);
        uint8_t xor_val = (uint8_t)((a ^ alo) & 0xFF);
        int16_t hi = (int16_t)((xor_val >> 8) & 0x07);
        int16_t shift = (int16_t)(((uint16_t)hi) + 4);
        result = (int16_t)dm2_atimesb_rshiftc(result, 3, shift);
    }
    return result;
}

/* skproject DM2_timproc_3a15_1da8 (c_tim_proc.cpp:1488)
 * Ornate animator toggle helper. Returns:
 *   input 0 → 1, input 1 → 0, input 2 → toggle bit 0 of second param,
 *   anything else → 0. */
int32_t dm2_v1_timproc_3a15_1da8(int32_t a, int32_t b) {
    uint16_t au = (uint16_t)a;
    if (au < 1)
        return (int16_t)a == 0 ? 1 : 0;
    if (au <= 1)
        return 0;
    if ((int16_t)a != 2)
        return 0;
    return (int32_t)(((uint8_t)b ^ 1) | (b & ~0xFF));
}

/* skproject DM2_hero_2c1d_0300 (c_hero.cpp:652)
 * Adjusts hero ability[idx][CUR] toward MAX by delta with diminishing curve. */
void dm2_v1_hero_2c1d_0300(DM2_V1_Hero *hero, int16_t ability_idx, int16_t delta) {
    int16_t diff = (int16_t)(hero->ability[ability_idx][DM2_CUR]
                           - hero->ability[ability_idx][DM2_MAX] + delta);
    int16_t sign_diff = diff < 0;
    int16_t sign_delta = delta < 0;
    if (sign_diff == sign_delta) {
        int16_t abs_diff = diff < 0 ? (int16_t)-diff : diff;
        for (;;) {
            if (abs_diff <= 0x14)
                break;
            int16_t quarter = (int16_t)(delta / 4);
            delta = (int16_t)(delta - quarter);
            abs_diff = (int16_t)(abs_diff - 0x14);
        }
    }
    int16_t result = (int16_t)((uint8_t)hero->ability[ability_idx][DM2_CUR] + delta);
    hero->ability[ability_idx][DM2_CUR] =
        (int8_t)dm2_between_value(10, 0xdc, result);
}

/* skproject DM2_hero_37BEA (c_hero.cpp:2383)
 * Per-hero special force contribution. carried_weight is hero's total weight. */
int32_t dm2_v1_hero_37bea(DM2_V1_Party *party, int16_t hero_idx, int16_t carried_weight) {
    if (party->hero[hero_idx].curHP == 0)
        return 0;
    int16_t flag_bit = (int16_t)(party->hero[hero_idx].heroflag & 0x10);
    int32_t w = (uint16_t)carried_weight;
    int32_t div10 = w / 10;
    int32_t combined = div10 + (uint16_t)flag_bit;
    return combined != 0 ? 0x32 : 0x28;
}

/* skproject DM2_GET_PARTY_SPECIAL_FORCE (c_hero.cpp:2407)
 * Sums hero special-force contributions. carried_weights[i] = weight for hero i. */
int32_t dm2_v1_get_party_special_force(DM2_V1_Party *party, const int16_t *carried_weights) {
    int32_t total = 0;
    for (int16_t i = 0; i < party->heros_in_party; i++)
        total += dm2_v1_hero_37bea(party, i, carried_weights[i]);
    return total;
}

/* skproject DM2_RESET_SQUAD_DIR (c_hero.cpp:2939) */
void dm2_v1_party_reset_squad_dir(DM2_V1_Party *party, int8_t facing_dir) {
    for (int i = 0; i < party->heros_in_party; i++)
        party->hero[i].absdir = facing_dir;
}

/* skproject DM2_SELECT_CHAMPION_LEADER (c_hero.cpp:2325)
 * current_leader = eventqueue.event_heroidx, next_champion_number = ddat.v1e0288 */
void dm2_v1_party_select_champion_leader(
    DM2_V1_Party *party, int16_t new_leader,
    int16_t current_leader, int16_t next_champion_number)
{
    if (new_leader == current_leader)
        return;
    if (new_leader != -1) {
        if (party->hero[new_leader].curHP == 0)
            return;
    }
    if (current_leader != -1)
        party->hero[current_leader].heroflag |= 0x1400;
    party->curactevhero = (DM2_HeroIndex)new_leader;
    if (new_leader == -1)
        return;
    if (new_leader + 1 == next_champion_number)
        return;
    party->hero[new_leader].heroflag |= 0x1400;
}

/* skproject DM2_ADJUST_HAND_COOLDOWN (c_hero.cpp:2432)
 * hand_idx: -1 means slots 0..2, else single slot hand_idx.
 * savegames1_b04: ddat.savegames1.b_04 flag. */
void dm2_v1_hero_adjust_hand_cooldown(
    DM2_V1_Hero *hero, int16_t hand_idx, int16_t base_delay,
    int savegames1_b04)
{
    uint16_t delay = (uint16_t)base_delay;
    delay += delay / 4;
    int16_t start, count;
    if (hand_idx != -1) {
        start = hand_idx;
        count = 1;
    } else {
        start = 0;
        count = 3;
    }
    if (savegames1_b04)
        delay >>= 2;
    delay += 2;
    for (int16_t i = start; count > 0; i++, count--) {
        uint16_t cur = (uint8_t)hero->handcooldown[i];
        uint16_t add;
        if (delay <= cur)
            add = delay / 2;
        else
            add = cur / 2;
        uint16_t result = (delay > cur ? delay : cur) + add;
        if (result > 0xff)
            result = 0xff;
        hero->handcooldown[i] = (int8_t)result;
    }
}

/* skproject DM2_USE_DEXTERITY_ATTRIBUTE (c_hero.cpp:2026)
 * Deterministic: caller provides RNG values (rand & 0x7 each). */
int16_t dm2_v1_hero_use_dexterity_attribute_raw(
    DM2_V1_Hero *hero, int16_t carried_weight, int16_t max_load,
    int sleep_flag, int16_t rand7_1, int16_t rand7_2, int16_t rand7_3)
{
    int16_t dex = dm2_v1_hero_get_adj_ability1_raw(
        hero, DM2_ABILITY_DEXTERITY, DM2_CUR, 0);
    int16_t r2 = (int16_t)(rand7_1 + dex);
    int32_t product = (int32_t)((int16_t)(r2 / 2)) * (int32_t)carried_weight;
    int32_t load = (int32_t)max_load;
    if (load <= 0) load = 1;
    int16_t sub = (int16_t)(product / load);
    r2 = (int16_t)(r2 - sub);
    int16_t val = r2 < 2 ? 2 : r2;
    if (sleep_flag)
        val = (int16_t)(val >> 1);
    int16_t hi = (int16_t)(100 - rand7_2);
    int16_t mid = (int16_t)(val / 2);
    int16_t lo = (int16_t)(rand7_3 + 1);
    return dm2_between_value(lo, hi, mid);
}

/* skproject DM2_2c1d_0e23 (c_hero.cpp:3879)
 * Stamina cost from item weight. Standalone arithmetic. */
int16_t dm2_v1_hero_2c1d_0e23(int16_t weight) {
    int16_t half = (int16_t)(weight / 2);
    int16_t clamped = dm2_between_value(1, 10, half);
    int16_t result = clamped;
    for (;;) {
        half = (int16_t)(half - 10);
        if (half <= 0)
            return result;
        result = (int16_t)(result + half / 2);
    }
}
