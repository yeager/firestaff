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

void dm2_v1_proceed_enchantment_self(
    uint16_t hero_mask, uint8_t aura_type, int16_t power, uint16_t duration,
    const DM2_V1_EnchantmentCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    int halve_power = 0;
    uint16_t mask = hero_mask;

    /* Pass 1: check aura mismatch, clean up old timers, detect halving */
    for (int i = 0; i < 4 && i < cb->hero_count; i++) {
        uint16_t hero_bit = (uint16_t)(1 << i);
        if ((mask & hero_bit) == 0)
            continue;
        DM2_V1_HeroState *hero = cb->get_hero(ctx, i);
        if (!hero)
            continue;
        if (hero->cur_hp == 0) {
            mask &= ~hero_bit;
            continue;
        }
        if (aura_type != hero->ench_aura || hero->cur_hp == 0) {
            hero->ench_power = 0;
            int timer_count = cb->get_timer_count(ctx);
            for (int t = 0; t < timer_count; t++) {
                if (cb->get_timer_type(ctx, t) != 0x48)
                    continue;
                uint8_t actor = cb->get_timer_actor(ctx, t);
                if ((actor & mask) == 0)
                    continue;
                uint8_t remaining = actor & ~(uint8_t)mask;
                if (remaining != 0)
                    cb->set_timer_actor(ctx, t, (uint8_t)(actor & ~(uint8_t)(mask & 0xFF)));
                else
                    cb->delete_timer(ctx, t);
            }
        }
        if (hero->ench_power > 50)
            halve_power = 1;
    }

    if (halve_power)
        power >>= 2;

    /* Pass 2: apply enchantment to affected heroes */
    for (int i = 0; i < cb->hero_count; i++) {
        uint16_t hero_bit = (uint16_t)(1 << i);
        if ((mask & hero_bit) == 0)
            continue;
        DM2_V1_HeroState *hero = cb->get_hero(ctx, i);
        if (!hero)
            continue;
        hero->ench_aura = aura_type;
        hero->ench_power = (uint8_t)(hero->ench_power + power);
    }

    cb->queue_enchant_timer(ctx, (uint8_t)(mask & 0xFF), power, duration);
}

void dm2_v1_proceed_global_effect_timers(
    DM2_V1_GlobalSpellEffects *effects,
    const DM2_V1_GlobalEffectCallbacks *cb, void *ctx)
{
    if (!effects || !cb)
        return;
    effects->light = 0;
    effects->invisibility = 0;
    effects->see_thru_walls = 0;

    for (int i = 0; i < cb->timer_count; i++) {
        uint8_t ttype = cb->get_timer_type(ctx, i);
        uint16_t actor = cb->get_timer_actor(ctx, i);
        int16_t value = cb->get_timer_value(ctx, i);

        switch (ttype) {
        case 0x0E: /* ttyItemBonus */
            if (cb->process_timer_0e)
                cb->process_timer_0e(ctx, i);
            break;
        case 0x46: /* ttyLight */
            if (value == 0 || value < -15 || value > 15)
                break;
            if (value < 0) {
                int idx = -value;
                if (cb->light_level_table && idx < cb->light_level_table_size)
                    effects->light = (int16_t)(effects->light +
                        cb->light_level_table[idx]);
            } else {
                if (cb->light_level_table && value < cb->light_level_table_size)
                    effects->light = (int16_t)(effects->light -
                        (cb->light_level_table[value] << 1));
            }
            break;
        case 0x47: /* ttyInvisibility */
            effects->invisibility++;
            break;
        case 0x49: /* ttySeeThruWalls (DM2 extended) */
            effects->see_thru_walls++;
            break;
        case 0x48: /* ttyEnchantment */
            for (int h = 0; h < cb->hero_count; h++) {
                if ((actor & (1 << h)) == 0)
                    continue;
                DM2_V1_HeroState *hero = cb->get_hero(ctx, h);
                if (!hero || hero->cur_hp == 0)
                    continue;
                hero->ench_power = (uint8_t)(hero->ench_power + value);
            }
            break;
        case 0x4B: /* ttyPoison */
            {
                DM2_V1_HeroState *hero = cb->get_hero(ctx, (int)actor);
                if (hero)
                    hero->poison_value++;
            }
            break;
        }
    }
}

int dm2_v1_process_timer_0c(DM2_V1_HeroState *hero)
{
    if (!hero)
        return 0;
    hero->timer_idx = -1;
    if (hero->cur_hp != 0)
        hero->hero_flag |= DM2_V1_HERO_FLAG_0800;
    return 1;
}

int dm2_v1_resume_from_wake(const DM2_V1_WakeCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    *cb->wake_flag = 1;
    *cb->sleep_flag = 0;
    *cb->tick_trigger = 0x8;
    cb->init_backbuff(ctx);
    return cb->display_mode(ctx, 5);
}

void dm2_v1_champion_squad_recompute_position(
    const DM2_V1_SquadRecomputeCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    if (cb->pending_flag != 0)
        cb->change_player_pos(ctx, (int16_t)(cb->pending_pos | (int16_t)0x8000));
}
