/* DM2 V1 runtime narrow ops — callback-based stubs for the remaining
 * skproject SKULLWIN functions (timer/hero/creature/item/moverec/record/
 * light). See dm2_v1_runtime_narrow_pc34_compat.h for source references. */

#include "dm2_v1_runtime_narrow_pc34_compat.h"
#include <stddef.h>

/* =====================================================================
 * c_tim_proc.cpp
 * ===================================================================== */

void dm2_v1_process_timer_resurrection(
    int timer_idx, const DM2_V1_TimerResurrectionCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t hero_idx = cb->get_timer_actor(ctx, timer_idx);
    (void)cb->get_timer_value(ctx, timer_idx);
    cb->bring_champion_to_life(ctx, (int)hero_idx);
    cb->delete_timer(ctx, timer_idx);
}

void dm2_v1_continue_ornate_noise(
    int timer_idx, int16_t x, int16_t y, uint32_t delay,
    const DM2_V1_ContinueOrnateNoiseCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    int16_t sample = cb->get_timer_value(ctx, timer_idx);
    if (sample < 0) {
        cb->delete_timer(ctx, timer_idx);
        return;
    }
    cb->play_sound(ctx, x, y, (int)sample);
    cb->requeue_timer(ctx, timer_idx, delay);
}

void dm2_v1_activate_ornate_animator(
    int16_t x, int16_t y, int frame_count,
    const DM2_V1_ActivateOrnateAnimatorCallbacks *cb, void *ctx)
{
    if (!cb || frame_count <= 0)
        return;
    int16_t frame = cb->get_ornament_frame(ctx, x, y);
    frame++;
    if (frame >= frame_count)
        frame = 0;
    cb->set_ornament_frame(ctx, x, y, frame);
    cb->invalidate_view(ctx, x, y);
}

void dm2_v1_activate_continuous_ornate_animator(
    int timer_idx, int16_t x, int16_t y, int frame_count, uint32_t delay,
    const DM2_V1_ActivateContinuousOrnateAnimatorCallbacks *cb, void *ctx)
{
    if (!cb || frame_count <= 0)
        return;
    int16_t frame = cb->get_ornament_frame(ctx, x, y);
    frame = (int16_t)((frame + 1) % frame_count);
    cb->set_ornament_frame(ctx, x, y, frame);
    cb->requeue_timer(ctx, timer_idx, delay);
}

void dm2_v1_activate_shooter(
    int timer_idx, int16_t x, int16_t y, uint16_t item, int direction,
    const DM2_V1_ActivateShooterCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    (void)cb->get_timer_actor(ctx, timer_idx);
    cb->shoot_item(ctx, x, y, item, direction);
}

int dm2_v1_try_ornate_noise(
    int16_t x, int16_t y, int chance, uint32_t delay,
    const DM2_V1_TryOrnateNoiseCallbacks *cb, void *ctx)
{
    if (!cb || chance <= 0)
        return 0;
    int roll = cb->random(ctx, chance);
    if (roll != 0)
        return 0;
    cb->queue_timer(ctx, x, y, delay);
    return 1;
}

void dm2_v1_animate_creature(
    DM2_V1_CreatureAnimState *anim, int16_t direction, int16_t step)
{
    if (!anim)
        return;
    (void)direction;
    if (anim->anim_max <= 0)
        return;
    anim->anim_frame = (int16_t)(anim->anim_frame + step);
    if (anim->anim_frame >= anim->anim_max)
        anim->anim_frame = (int16_t)(anim->anim_frame % anim->anim_max);
    if (anim->anim_frame < 0)
        anim->anim_frame = 0;
}

void dm2_v1_advance_tiles_time(
    int16_t x, int16_t y, int16_t delta,
    const DM2_V1_AdvanceTilesTimeCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->advance_tile_state(ctx, x, y, delta);
}

void dm2_v1_process_actuator_tick_generator(
    const DM2_V1_ActuatorTickGeneratorCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    for (int i = 0; i < cb->generator_count; i++) {
        if (cb->get_generator_active(ctx, i))
            cb->tick_generator(ctx, i);
    }
}

int dm2_v1_operate_pit_tele_tile(
    int16_t x, int16_t y, int tele_mode,
    const DM2_V1_OperatePitTeleTileCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int count = cb->get_things_on_tile(ctx, x, y);
    if (count <= 0)
        return 0;
    for (int i = 0; i < count; i++)
        cb->teleport_thing(ctx, x, y, i);
    (void)tele_mode;
    return 1;
}

/* =====================================================================
 * c_hero.cpp
 * ===================================================================== */

int16_t dm2_v1_calc_player_attack_damage(
    const DM2_V1_AttackDamageHero *hero, int16_t weapon_damage,
    int16_t action_strength, int16_t to_hit_bonus, int16_t stealth_bonus)
{
    if (!hero || hero->cur_hp <= 0)
        return 0;
    int32_t dmg = weapon_damage;
    dmg += (action_strength * (hero->skill_fighter + 1)) / 32;
    dmg += (to_hit_bonus + stealth_bonus + hero->skill_ninja) / 8;
    if (dmg < 0)
        dmg = 0;
    if (dmg > 0x7FFF)
        dmg = 0x7FFF;
    return (int16_t)dmg;
}

void dm2_v1_hero_39796(DM2_V1_HeroRegenState *hero)
{
    if (!hero || hero->cur_hp <= 0)
        return;
    if (hero->cur_stamina < hero->max_stamina) {
        hero->cur_stamina++;
        if (hero->cur_stamina > hero->max_stamina)
            hero->cur_stamina = hero->max_stamina;
    }
    hero->hero_flag |= 0x0800;
}

int dm2_v1_hero_2c1d_135d(int16_t skill_value, int16_t action_difficulty)
{
    int32_t gain = action_difficulty - (skill_value / 4);
    if (gain < 1)
        gain = 1;
    return (int)gain;
}

int32_t dm2_v1_adjust_skills(
    int hero_idx, DM2_V1_AdjustSkillsState *state, int32_t exp_gain)
{
    if (!state || !state->skill_exp || !state->skill_level)
        return -1;
    (void)hero_idx;
    state->skill_exp[hero_idx] += (int16_t)exp_gain;
    int32_t level = 0;
    if (state->level_thresholds) {
        for (int i = 0; i < state->level_threshold_count; i++) {
            if (state->skill_exp[hero_idx] >= state->level_thresholds[i])
                level = i + 1;
            else
                break;
        }
    }
    state->skill_level[hero_idx] = (int16_t)level;
    return level;
}

int16_t dm2_v1_wound_player(
    int hero_idx, DM2_V1_WoundPlayerHero *hero, int16_t damage, int flags,
    const DM2_V1_WoundPlayerCallbacks *cb, void *ctx)
{
    (void)flags;
    if (!hero || hero->cur_hp <= 0)
        return 0;
    int16_t applied = damage;
    if (applied > hero->cur_hp)
        applied = hero->cur_hp;
    hero->cur_hp = (int16_t)(hero->cur_hp - applied);
    hero->hero_flag |= 0x0800;
    if (hero->cur_hp <= 0) {
        hero->cur_hp = 0;
        if (cb && cb->player_defeated)
            cb->player_defeated(ctx, hero_idx);
    }
    return applied;
}

void dm2_v1_update_champions_stats(
    const DM2_V1_UpdateChampionsStatsCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    for (int i = 0; i < cb->hero_count; i++)
        cb->recompute_hero_stats(ctx, i);
}

int dm2_v1_wield_weapon(
    int hero_idx, uint16_t item, int hand_slot, int forced,
    const DM2_V1_WieldWeaponCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    if (!forced && !cb->is_item_fit(ctx, item, hand_slot))
        return 0;
    cb->equip_item(ctx, hero_idx, item, hand_slot);
    return 1;
}

int32_t dm2_v1_remove_object_from_hand(DM2_V1_RemoveFromHandState *state)
{
    if (!state || !state->hand_item)
        return -1;
    int32_t item = *state->hand_item;
    *state->hand_item = -1;
    return item;
}

void dm2_v1_player_defeated(
    int hero_idx, const DM2_V1_PlayerDefeatedCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->drop_player_items(ctx, hero_idx);
    cb->mark_hero_dead(ctx, hero_idx);
    if (cb->count_living_heroes(ctx) <= 0)
        cb->trigger_game_over(ctx);
}

void dm2_v1_process_players_damage(
    const DM2_V1_ProcessPlayersDamageCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    for (int i = 0; i < cb->hero_count; i++) {
        int16_t dmg = cb->get_pending_damage(ctx, i);
        if (dmg == 0)
            continue;
        cb->wound_player(ctx, i, dmg);
        cb->clear_pending_damage(ctx, i);
    }
}

int dm2_v1_player_consume_object(
    int hero_idx, uint16_t item,
    const DM2_V1_PlayerConsumeObjectCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int category = cb->get_item_category(ctx, item);
    return cb->dispatch(ctx, hero_idx, item, category);
}

void dm2_v1_change_player_pos(
    int16_t packed_pos, const DM2_V1_ChangePlayerPosCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    int hero_idx = (packed_pos >> 8) & 0xFF;
    uint8_t new_pos = (uint8_t)(packed_pos & 0xFF);
    cb->swap_positions(ctx, hero_idx, new_pos);
}

int32_t dm2_v1_hero_2c1d_1de2(int hero_idx, int16_t resist_stat, int16_t power)
{
    (void)hero_idx;
    if (resist_stat >= power)
        return 1;
    return 0;
}

void dm2_v1_revive_player(
    int8_t hero_type, int8_t direction,
    const DM2_V1_RevivePlayerCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->init_hero_stats(ctx, hero_type);
    cb->set_hero_direction(ctx, direction);
}

void dm2_v1_select_champion(
    int hero_idx, int panel_slot, int mode,
    const DM2_V1_SelectChampionCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    (void)panel_slot;
    cb->set_selected_champion(ctx, hero_idx);
    cb->refresh_panel(ctx, mode);
}

/* =====================================================================
 * c_engage.cpp
 * ===================================================================== */

int32_t dm2_v1_engage_command(
    int hero_idx, int command_id,
    const DM2_V1_EngageCommandCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch)
        return -1;
    return cb->dispatch(ctx, hero_idx, command_id);
}

/* =====================================================================
 * c_creature.cpp
 * ===================================================================== */

int32_t dm2_v1_wound_creature(
    int creature_idx, DM2_V1_WoundCreatureState *creature, int16_t damage,
    const DM2_V1_WoundCreatureCallbacks *cb, void *ctx)
{
    if (!creature || creature->cur_hp <= 0)
        return 0;
    int16_t applied = damage;
    if (applied > creature->cur_hp)
        applied = creature->cur_hp;
    creature->cur_hp = (int16_t)(creature->cur_hp - applied);
    if (creature->cur_hp <= 0) {
        creature->cur_hp = 0;
        if (cb && cb->creature_defeated)
            cb->creature_defeated(ctx, creature_idx);
    }
    return applied;
}

int32_t dm2_v1_creature_attacks_player(
    int creature_idx, int hero_idx, int16_t damage,
    const DM2_V1_CreatureAttacksPlayerCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    if (!cb->roll_to_hit(ctx, creature_idx, hero_idx))
        return 0;
    cb->wound_player(ctx, hero_idx, damage);
    return 1;
}

int32_t dm2_v1_creature_attacks_creature(
    int attacker_idx, int defender_idx, int16_t damage,
    const DM2_V1_CreatureAttacksCreatureCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    if (!cb->roll_to_hit(ctx, attacker_idx, defender_idx))
        return 0;
    cb->wound_creature(ctx, defender_idx, damage);
    return 1;
}

int32_t dm2_v1_creature_can_handle_it(
    const DM2_V1_CreatureHandleCaps *caps, int item_class)
{
    if (!caps || item_class < 0 || item_class >= 32)
        return 0;
    return (caps->handle_mask & ((uint32_t)1 << item_class)) != 0;
}

int32_t dm2_v1_creature_cast_spell(
    int creature_idx, int spell_id,
    const DM2_V1_CreatureCastSpellCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    cb->cast_spell(ctx, creature_idx, spell_id);
    return 1;
}

int32_t dm2_v1_creature_steal_from_champion(
    int creature_idx, int hero_idx,
    const DM2_V1_CreatureStealCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int32_t slot = cb->pick_random_item_slot(ctx, hero_idx);
    if (slot < 0)
        return 0;
    int32_t item = cb->remove_possession(ctx, hero_idx, slot);
    if (item < 0)
        return 0;
    cb->give_creature_item(ctx, creature_idx, item);
    return 1;
}

int32_t dm2_v1_creature_ccm0b(
    int creature_idx, const DM2_V1_CreatureCcm0bCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;
    return cb->step(ctx, creature_idx);
}

int32_t dm2_v1_creature_ccm0c(
    int creature_idx, const DM2_V1_CreatureCcm0cCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;
    return cb->step(ctx, creature_idx);
}

int32_t dm2_v1_creature_uses_ladder_hole(
    int creature_idx, int16_t x, int16_t y,
    const DM2_V1_CreatureUsesLadderHoleCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int tile = cb->get_tile_type(ctx, x, y);
    if (tile <= 0)
        return 0;
    int level_delta = (tile == 1) ? 1 : -1;
    cb->move_creature(ctx, creature_idx, x, y, level_delta);
    return 1;
}

int32_t dm2_v1_creature_walk_now(DM2_V1_CreatureWalkState *state)
{
    if (!state || state->speed <= 0)
        return 0;
    state->move_counter += state->speed;
    if (state->move_counter < 256)
        return 0;
    state->move_counter -= 256;
    return 1;
}

int32_t dm2_v1_creature_activates_wall(
    int16_t x, int16_t y, int dir,
    const DM2_V1_CreatureActivatesWallCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    cb->activate_wall_item(ctx, x, y, dir);
    return 1;
}

int32_t dm2_v1_place_merchandise(
    int creature_idx, int16_t x, int16_t y, uint16_t item,
    const DM2_V1_PlaceMerchandiseCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    (void)creature_idx;
    cb->place_item_on_tile(ctx, x, y, item);
    return 1;
}

int32_t dm2_v1_take_merchandise(
    int creature_idx, int16_t x, int16_t y,
    const DM2_V1_TakeMerchandiseCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;
    (void)creature_idx;
    return cb->remove_item_from_tile(ctx, x, y);
}

/* =====================================================================
 * c_item.cpp
 * ===================================================================== */

void dm2_v1_move_item_to(
    uint16_t item, int16_t x, int16_t y, int16_t flags,
    const DM2_V1_MoveItemToCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    (void)flags;
    cb->unlink_from_source(ctx, item);
    cb->link_to_tile(ctx, item, x, y);
}

int32_t dm2_v1_activate_item_teleport(
    uint16_t item, int16_t src_x, int16_t src_y,
    int16_t dst_x, int16_t dst_y, int32_t mode,
    const DM2_V1_ActivateItemTeleportCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch)
        return -1;
    return cb->dispatch(ctx, item, src_x, src_y, dst_x, dst_y, mode);
}

void dm2_v1_shoot_item(
    uint16_t item, int16_t x, int16_t y, int direction,
    int8_t damage, int8_t speed, int8_t power,
    const DM2_V1_ShootItemCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->spawn_missile(ctx, item, x, y, direction, damage, speed, power);
}

/* =====================================================================
 * c_moverec.cpp
 * ===================================================================== */

int32_t dm2_v1_try_push_object_to(
    int32_t record, int16_t dst_x, int16_t dst_y,
    int16_t *out_x, int16_t *out_y,
    const DM2_V1_TryPushObjectToCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    if (!cb->is_tile_free(ctx, dst_x, dst_y))
        return 0;
    cb->move_record_to(ctx, record, dst_x, dst_y);
    if (out_x)
        *out_x = dst_x;
    if (out_y)
        *out_y = dst_y;
    return 1;
}

void dm2_v1_moverec_2fcf_0234(
    int32_t record, int16_t src_x, int16_t src_y,
    int16_t dst_x, int16_t dst_y,
    const DM2_V1_Moverec2fcf0234Callbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->unlink_record(ctx, record, src_x, src_y);
    cb->link_record(ctx, record, dst_x, dst_y);
}

void dm2_v1_moverec_3ce7d(
    int32_t record, int16_t x, int16_t y, int32_t kind, int32_t flags,
    const DM2_V1_Moverec3ce7dCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch)
        return;
    (void)cb->dispatch(ctx, record, x, y, kind, flags);
}

/* =====================================================================
 * c_record.cpp
 * ===================================================================== */

int32_t dm2_v1_recycle_a_record_from_the_world(
    int32_t requested_kind, const DM2_V1_RecycleRecordCallbacks *cb, void *ctx)
{
    (void)requested_kind;
    if (!cb)
        return -1;
    int best_idx = -1;
    int best_importance = 0x7FFFFFFF;
    for (int i = 0; i < cb->record_count; i++) {
        int importance = cb->get_record_importance(ctx, i);
        if (importance < best_importance) {
            best_importance = importance;
            best_idx = i;
        }
    }
    if (best_idx < 0)
        return -1;
    cb->free_record(ctx, best_idx);
    return best_idx;
}

void dm2_v1_delete_missile_record(
    int32_t record, int16_t x, int16_t y,
    const DM2_V1_DeleteMissileRecordCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->unlink_missile(ctx, record, x, y);
    cb->free_record(ctx, record);
}

/* =====================================================================
 * c_light.cpp
 * ===================================================================== */

void dm2_v1_add_background_light_from_tile(
    int16_t x, int16_t y, int16_t radius,
    const DM2_V1_AddBackgroundLightCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    int16_t light = cb->get_tile_light(ctx, x, y);
    if (light == 0)
        return;
    int16_t amount = (int16_t)(light - radius);
    if (amount < 0)
        amount = 0;
    cb->add_light(ctx, x, y, amount);
}

int32_t dm2_v1_check_recompute_light(
    const DM2_V1_CheckRecomputeLightCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    if (!cb->is_light_dirty(ctx))
        return 0;
    cb->recompute_light_map(ctx);
    cb->clear_light_dirty(ctx);
    return 1;
}
