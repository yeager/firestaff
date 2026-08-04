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
    int hero_idx, int16_t cure_amount,
    const DM2_V1_CurePoisonCallbacks *cb, void *ctx)
{
    if (!cb || hero_idx == -1)
        return 0;
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (!hero)
        return 0;
    int count = cb->get_timer_count(ctx);
    for (int i = 0; i < count; i++) {
        if (cb->get_timer_type(ctx, i) != 0x4B)
            continue;
        if (cb->get_timer_actor(ctx, i) != (uint8_t)hero_idx)
            continue;
        int16_t timer_counter = cb->get_timer_value(ctx, i);
        if (cure_amount < timer_counter) {
            cb->set_timer_value(ctx, i, (int16_t)(timer_counter - cure_amount));
            hero->poison_value = (int16_t)(hero->poison_value - cure_amount);
            return 1;
        }
        cure_amount = (int16_t)(cure_amount - timer_counter);
        hero->poison_value = (int16_t)(hero->poison_value - timer_counter);
        cb->delete_timer(ctx, i);
        hero->poisoned_count--;
        if (hero->poisoned_count == 0)
            return 1;
        if (cure_amount <= 0)
            return 1;
    }
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
    if (cb->is_last_hero && cb->is_last_hero(ctx, hero_idx))
        return 0;
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (!hero)
        return 0;
    int16_t wound_amount = (int16_t)((counters + 0x1E) >> 6);
    if (wound_amount < 1)
        wound_amount = 1;
    cb->wound_player(ctx, hero_idx, wound_amount, 0, 0);
    hero->hero_flag |= DM2_V1_HERO_FLAG_0800 | DM2_V1_HERO_FLAG_2000;
    counters--;
    if (counters <= 0)
        return 1;
    int16_t accum = (int16_t)(hero->poison_value + counters);
    if (accum > 0xC00) {
        counters = (int16_t)(0xC00 - hero->poison_value);
        if (counters <= 0)
            return 1;
    }
    hero->poison_value = (int16_t)(hero->poison_value + counters);
    hero->poisoned_count++;
    cb->queue_poison_timer(ctx, hero_idx, counters, 0x24);
    return 1;
}

int dm2_v1_add_coin_to_wallet(uint16_t container_ref, uint16_t coin_item_ref,
                               const DM2_V1_WalletCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    if (!cb->is_container_moneybox(ctx, container_ref))
        return 0;
    if (!cb->is_item_currency(ctx, coin_item_ref))
        return 0;

    uint16_t denom = cb->get_distinctive_item_type(ctx, coin_item_ref);
    uint16_t item = cb->get_container_head(ctx, container_ref);
    while (item != DM2_V1_OBJECT_END) {
        uint16_t db_type = cb->get_item_db_type(ctx, item);
        if (db_type != DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY)
            return db_type != 0;
        if (cb->get_distinctive_item_type(ctx, item) == denom &&
            cb->get_item_charge(ctx, item) < DM2_V1_COIN_CHARGE_MAX) {
            cb->set_item_charge(ctx, item,
                (uint8_t)(cb->get_item_charge(ctx, item) + 1));
            cb->dealloc_record(ctx, coin_item_ref);
            return 1;
        }
        item = cb->get_next_item_in_list(ctx, item);
    }
    cb->set_item_charge(ctx, coin_item_ref, 0);
    cb->append_item_to_container(ctx, coin_item_ref, container_ref);
    return 1;
}

uint16_t dm2_v1_take_coin_from_wallet(uint16_t container_ref,
                                       int16_t coin_type_index,
                                       const DM2_V1_WalletCallbacks *cb,
                                       void *ctx)
{
    if (!cb || coin_type_index < 0)
        return DM2_V1_OBJECT_NULL;
    if (!cb->coin_type_table || coin_type_index >= cb->coin_type_table_size)
        return DM2_V1_OBJECT_NULL;
    uint16_t denom = cb->coin_type_table[coin_type_index];

    uint16_t found = DM2_V1_OBJECT_NULL;
    uint16_t item = cb->get_container_head(ctx, container_ref);
    for (;;) {
        if (item == DM2_V1_OBJECT_END) {
            if (found != DM2_V1_OBJECT_NULL) {
                uint16_t stack = found;
                if (cb->get_item_charge(ctx, stack) != 0) {
                    cb->set_item_charge(ctx, stack,
                        (uint8_t)(cb->get_item_charge(ctx, stack) - 1));
                    found = cb->alloc_new_dbitem(ctx, denom);
                } else {
                    cb->cut_item_from_container(ctx, stack, container_ref);
                }
            }
            return found;
        }
        uint16_t db_type = cb->get_item_db_type(ctx, item);
        if (db_type != DM2_V1_ITEM_DB_TYPE_MISC_CURRENCY)
            return DM2_V1_OBJECT_NULL;
        if (cb->get_distinctive_item_type(ctx, item) == denom)
            found = item;
        item = cb->get_next_item_in_list(ctx, item);
    }
}

void dm2_v1_perform_turn_squad(
    int delta,
    const DM2_V1_SquadCallbacks *cb, void *ctx)
{
    if (!cb || delta == 0)
        return;
    cb->reset_squad_dir(ctx);
    uint8_t tile_val = cb->get_tile_value(ctx, cb->party_x, cb->party_y);
    uint8_t tile_type = (tile_val >> 5) & 0x7;
    if (tile_type == 3) {
        int stair_flag = tile_val & 0x4;
        cb->move_stairs(ctx, stair_flag);
        return;
    }
    uint8_t b1, b2, b3, b4;
    if (cb->get_teleporter_detail(ctx, cb->party_x, cb->party_y,
                                   &b1, &b2, &b3, &b4)) {
        cb->map_teleport(ctx, b2, b3, b4, b1);
        return;
    }
    cb->moverec(ctx, cb->party_x, cb->party_y, -1, 1, 0, 0);
    int16_t rotation;
    if (delta != 2)
        rotation = 3;
    else
        rotation = 1;
    int16_t new_dir = (int16_t)((rotation + cb->party_dir) & 0x3);
    cb->rotate_party(ctx, new_dir);
    cb->moverec(ctx, cb->party_x, cb->party_y, -1, 1, 1, 0);
}

void dm2_v1_set_spelling_champion(
    DM2_V1_SpellingState *state, int hero_idx,
    const DM2_V1_SquadCallbacks *cb, void *ctx)
{
    if (!state || !cb)
        return;
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (!hero || hero->cur_hp == 0)
        return;
    *state->spelling_champion = (int16_t)(hero_idx + 1);
    state->curactmode = 2;
    state->spell_active = 1;
    state->spell_symbol_count = 0;
    if (cb->rotate_party)
        cb->rotate_party(ctx, -1);
    if (state->update_panel)
        state->update_panel(state->panel_ctx, 0);
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
        if (hero->cur_hp == 0)
            mask &= ~hero_bit;
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

static int16_t dm2_min16(int16_t a, int16_t b)
{
    return a < b ? a : b;
}

static int16_t dm2_max16(int16_t a, int16_t b)
{
    return a > b ? a : b;
}

int dm2_v1_hero_cast_enchantment(
    DM2_V1_HeroState *hero, uint8_t enchant_type,
    int16_t power, int requires_mp,
    const DM2_V1_CastEnchantCallbacks *cb, void *ctx)
{
    if (!cb || !hero)
        return 0;
    int full_cost = 1;
    if (requires_mp) {
        if (hero->cur_mp == 0)
            return 0;
        if (hero->cur_mp >= 4)
            hero->cur_mp = (int16_t)(hero->cur_mp - 4);
        else {
            power >>= 1;
            hero->cur_mp = 0;
            full_cost = 0;
        }
    }
    uint16_t duration = (uint16_t)power;
    int16_t ench_power = (int16_t)(power >> 5);
    cb->proceed_enchantment_self(ctx, 0x0F, enchant_type, ench_power, duration);
    return full_cost;
}

void dm2_v1_shoot_champion_missile(
    const DM2_V1_ShootMissileState *state,
    uint16_t item, int16_t damage, int16_t kinetic_energy, int16_t spell_power,
    const DM2_V1_ShootMissileCallbacks *cb, void *ctx)
{
    if (!cb || !state)
        return;
    uint8_t clamped_power = (uint8_t)(dm2_min16(spell_power, 255) & 0xFF);
    uint8_t clamped_ke = (uint8_t)(dm2_min16(kinetic_energy, 255) & 0xFF);
    uint8_t clamped_dmg = (uint8_t)(dm2_min16(damage, 255) & 0xFF);
    int direction = state->abs_dir;
    uint8_t pos_offset = (uint8_t)((state->party_pos - state->abs_dir + 1) & 0x2);
    int facing = (direction + (pos_offset >> 1)) & 0x3;
    cb->shoot_item(ctx, item, state->party_x, state->party_y,
                   direction, facing, clamped_dmg, clamped_ke, clamped_power);
    if (cb->set_v1e025e)
        cb->set_v1e025e(ctx, 4);
    if (cb->set_v1e0274)
        cb->set_v1e0274(ctx, (uint8_t)direction);
}

int dm2_v1_cast_champion_missile_spell(
    DM2_V1_HeroState *hero, const DM2_V1_ShootMissileState *state,
    uint16_t spell_item, int16_t spell_power, int16_t mp_cost,
    const DM2_V1_ShootMissileCallbacks *cb, void *ctx)
{
    if (!hero || !cb || !state)
        return 0;
    if (hero->cur_mp < mp_cost)
        return 0;
    hero->cur_mp = (int16_t)(hero->cur_mp - mp_cost);
    hero->hero_flag |= DM2_V1_HERO_FLAG_0800;
    int16_t mp_bonus = (int16_t)(hero->cur_mp >> 5);
    mp_bonus = dm2_min16(6, mp_bonus);
    int16_t accuracy = (int16_t)(10 - mp_bonus);
    int16_t power = spell_power;
    int16_t threshold = (int16_t)(accuracy << 2);
    if (power < threshold) {
        power = (int16_t)(power + 4);
        accuracy = (int16_t)(power / 4);
    }
    dm2_v1_shoot_champion_missile(state, spell_item, power, 0x5A,
                                   accuracy, cb, ctx);
    return 1;
}

void dm2_v1_hero_bring_to_life(
    int hero_idx,
    const DM2_V1_ResurrectCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    DM2_V1_HeroState *hero = cb->get_hero(ctx, hero_idx);
    if (!hero)
        return;
    /* R_36EFE: find free party position */
    int16_t saved_hp = hero->cur_hp;
    hero->cur_hp = 0;
    int need_scan = 1;
    if (hero->party_pos != 0xFF) {
        int16_t at = cb->get_player_at_position(ctx, hero->party_pos);
        if (at == -1)
            need_scan = 0;
    }
    if (need_scan) {
        for (int k = 0; k < 4; k++) {
            uint8_t pos = (uint8_t)((cb->formation_start + k) & 0x3);
            if (cb->get_player_at_position(ctx, pos) == -1) {
                hero->party_pos = pos;
                break;
            }
        }
    }
    hero->absdir = cb->formation_start;
    hero->cur_hp = saved_hp;

    /* BRING_CHAMPION_TO_LIFE */
    if (cb->clear_hero_items)
        cb->clear_hero_items(ctx, hero_idx);
    int16_t max_hp = hero->max_hp;
    int16_t reduction = (int16_t)(max_hp >> 6);
    max_hp = (int16_t)(max_hp - reduction - 1);
    max_hp = dm2_max16(25, max_hp);
    hero->max_hp = max_hp;
    hero->cur_hp = (int16_t)(max_hp / 2);
    hero->hero_flag |= DM2_V1_HERO_FLAG_4000;
    hero->ench_aura = 0;
    hero->ench_power = 0;
    if (cb->post_resurrect)
        cb->post_resurrect(ctx, hero_idx);
}

int dm2_v1_add_item_to_player(
    int hero_idx, int16_t *hero_items, int item_count,
    uint16_t item_record, uint16_t item_db_type,
    const DM2_V1_SlotGroup *groups, int group_count,
    const DM2_V1_AddItemCallbacks *cb, void *ctx)
{
    (void)item_db_type;
    if (!cb || !hero_items || !groups)
        return 0;
    for (int g = 0; g < group_count; g++) {
        for (uint16_t s = groups[g].slot_start; s <= groups[g].slot_end && s < (uint16_t)item_count; s++) {
            if (hero_items[s] != -1)
                continue;
            if (!cb->is_item_fit(ctx, item_record, (int)s, 0))
                continue;
            if (groups[g].category_filter != -1) {
                uint16_t cat = (item_record & 0x3C00) >> 10;
                if ((int16_t)cat != groups[g].category_filter)
                    continue;
            }
            cb->equip_item(ctx, hero_idx, item_record, (int)s);
            return 1;
        }
    }
    return 0;
}

int dm2_v1_burn_player_lighting_items(
    const DM2_V1_BurnLightCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int any_burned = 0;
    int last_idx = cb->hero_count - 1;
    if (cb->is_last_hero && cb->is_last_hero(ctx, last_idx))
        last_idx--;
    for (int h = last_idx; h >= 0; h--) {
        for (int hand = 1; hand >= 0; hand--) {
            int16_t item = cb->get_hero_hand_item(ctx, h, hand);
            if (item == -1)
                continue;
            uint16_t flags = cb->query_item_flags(ctx, (uint16_t)item);
            if ((flags & 0x10) == 0)
                continue;
            int16_t charge = cb->add_item_charge(ctx, (uint16_t)item, 0);
            if (charge == 0)
                continue;
            charge = cb->add_item_charge(ctx, (uint16_t)item, -1);
            if (charge == 0)
                cb->set_item_importance(ctx, (uint16_t)item, 0);
            any_burned = 1;
        }
    }
    if (any_burned)
        cb->recalc_light(ctx);
    return any_burned;
}

int dm2_v1_drop_player_items(
    int hero_idx, uint8_t party_pos,
    int16_t tile_x, int16_t tile_y,
    const uint8_t *slot_order, int slot_count,
    const DM2_V1_DropItemsCallbacks *cb, void *ctx)
{
    if (!cb || !slot_order)
        return 0;
    int dropped = 0;
    for (int i = 0; i < slot_count; i++) {
        int slot = slot_order[i];
        int16_t item = cb->remove_possession(ctx, hero_idx, slot);
        if (item == -1)
            continue;
        uint16_t packed = (uint16_t)((uint16_t)(party_pos << 14) |
                          ((uint16_t)item & 0x3FFF));
        cb->move_record_to(ctx, packed, tile_x, tile_y, party_pos);
        dropped++;
    }
    return dropped;
}

int dm2_v1_put_item_to_player(
    int hero_idx, DM2_V1_HeroState *hero,
    int16_t *hero_items, int item_count,
    const DM2_V1_PutItemCallbacks *cb, void *ctx)
{
    if (!cb || !hero || !hero_items)
        return 0;
    if (cb->cursor_item == -1)
        return 0;
    if (hero->cur_hp == 0)
        return 0;
    for (int slot = 13; slot < item_count && slot < 30; slot++) {
        if (hero_items[slot] != -1)
            continue;
        int16_t item = cb->remove_object_from_hand(ctx);
        if (item == -1)
            return 0;
        cb->equip_item(ctx, hero_idx, (uint16_t)item, slot);
        return 1;
    }
    return 0;
}
