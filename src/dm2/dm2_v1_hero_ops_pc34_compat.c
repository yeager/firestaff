/* DM2 V1 hero/champion operations — skproject c_hero.cpp + SkWinCore2.cpp. */

#include "dm2_v1_hero_ops_pc34_compat.h"
#include <stddef.h>

static int16_t dm2_abs16(int16_t v)
{
    return v < 0 ? (int16_t)-v : v;
}

int16_t dm2_v1_adjust_stamina(
    int hero_idx, int16_t stamina_cost,
    const DM2_V1_StaminaCallbacks *cb, void *ctx)
{
    if (!cb || hero_idx == -1)
        return -1;
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (!hero)
        return -1;
    int16_t cur = hero->cur_stamina;
    cur = (int16_t)(cur - stamina_cost);
    if (cur > 0) {
        if (cur > hero->max_stamina)
            cur = hero->max_stamina;
        hero->cur_stamina = cur;
    } else {
        hero->cur_stamina = 0;
        int16_t wound = (int16_t)(-cur / 2);
        cb->wound_player(ctx, hero_idx, wound, 0, 0);
    }
    int16_t abs_cost = dm2_abs16(stamina_cost);
    if (abs_cost >= 10)
        hero->hero_flag |= DM2_V1_HERO_FLAG_0800;
    return abs_cost;
}

int dm2_v1_cure_poison(
    int hero_idx,
    const DM2_V1_CurePoisonCallbacks *cb, void *ctx)
{
    if (!cb || hero_idx == -1)
        return 0;
    int count = cb->get_timer_count(ctx);
    for (int i = count - 1; i >= 0; i--) {
        if (cb->get_timer_type(ctx, i) == 0x4B) {
            if (cb->get_timer_actor(ctx, i) == (uint8_t)hero_idx) {
                cb->delete_timer(ctx, i);
            }
        }
    }
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (hero)
        hero->poison_value = 0;
    return 1;
}

int dm2_v1_cure_plague(
    int hero_idx, uint8_t plague_timer_type,
    const DM2_V1_CurePoisonCallbacks *cb, void *ctx)
{
    if (!cb || hero_idx == -1)
        return 0;
    int count = cb->get_timer_count(ctx);
    for (int i = count - 1; i >= 0; i--) {
        if (cb->get_timer_type(ctx, i) == plague_timer_type) {
            if (cb->get_timer_actor(ctx, i) == (uint8_t)hero_idx) {
                cb->delete_timer(ctx, i);
            }
        }
    }
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (hero)
        hero->plague_value = 0;
    return 1;
}

int dm2_v1_process_poison(
    int hero_idx, int16_t counters,
    const DM2_V1_ProcessPoisonCallbacks *cb, void *ctx)
{
    if (!cb || hero_idx == -1)
        return 0;
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (!hero || hero->cur_hp <= 0)
        return 0;
    cb->wound_player(ctx, hero_idx, 1, 0, 0);
    int16_t stam_cost = counters;
    if (stam_cost < 1)
        stam_cost = 1;
    stam_cost = (int16_t)(stam_cost << 4);
    cb->adjust_stamina(ctx, hero_idx, stam_cost);
    hero->cur_water = (int16_t)(hero->cur_water - 100);
    if (hero->cur_water < DM2_V1_WATER_MIN)
        hero->cur_water = DM2_V1_WATER_MIN;
    hero->hero_flag |= DM2_V1_HERO_FLAG_0800;
    hero->hero_flag |= DM2_V1_HERO_FLAG_2000;
    counters--;
    if (counters <= 0)
        return 1;
    hero->poison_value++;
    cb->queue_poison_timer(ctx, hero_idx, counters, 0x24);
    return 1;
}

int dm2_v1_add_coin_to_wallet(int16_t *wallet, int wallet_size,
                               int16_t coin_type)
{
    if (!wallet || coin_type < 0 || coin_type >= wallet_size)
        return 0;
    wallet[coin_type]++;
    return 1;
}

int dm2_v1_take_coin_from_wallet(int16_t *wallet, int wallet_size,
                                  int16_t coin_type)
{
    if (!wallet || coin_type < 0 || coin_type >= wallet_size)
        return 0;
    if (wallet[coin_type] <= 0)
        return 0;
    wallet[coin_type]--;
    return 1;
}

void dm2_v1_perform_turn_squad(
    int delta,
    const DM2_V1_SquadCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    for (int i = 0; i < cb->hero_count; i++) {
        DM2_V1_HeroState *hero = cb->get_hero(ctx, i);
        if (!hero)
            continue;
        hero->party_pos = (uint8_t)((hero->party_pos + delta) & 0x3);
    }
}

void dm2_v1_set_spelling_champion(DM2_V1_SpellingState *state, int hero_idx)
{
    if (!state)
        return;
    *state->spelling_champion = (int16_t)hero_idx;
}
