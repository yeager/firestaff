/* DM2 V1 runtime narrow ops — callback-based stubs for the remaining
 * skproject SKULLWIN functions (timer/hero/creature/item/moverec/record/
 * light). See dm2_v1_runtime_narrow_pc34_compat.h for source references. */

#include "dm2_v1_runtime_narrow_pc34_compat.h"
#include <stddef.h>

/* =====================================================================
 * c_tim_proc.cpp
 * ===================================================================== */

void dm2_v1_continue_ornate_noise_cb(
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

void dm2_v1_activate_shooter_narrow(
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

/* belongs to DM2_hero_2c1d_135d — c_hero.cpp:1352 (DM2_hero_2c1d_132c):
 * halves an armour class value when the "used/worn-out" flag is set. */
static int32_t dm2_v1_hero_2c1d_132c_helper(int16_t armour_class, int is_used)
{
    int32_t v = armour_class;
    if (is_used)
        v = v >> 3;
    return v;
}

/* DM2_CALC_PLAYER_ATTACK_DAMAGE — c_hero.cpp:232 */
int16_t dm2_v1_calc_player_attack_damage(
    int hero_idx, const DM2_V1_AttackDamageHero *hero, int creature_idx,
    int16_t action_strength, int32_t skill_id,
    const DM2_V1_AttackDamageCallbacks *cb, void *ctx)
{
    if (!hero || !cb || hero->cur_hp <= 0)
        return 0;

    int16_t result = 0;
    int hit = 0;

    /* skproject bails the whole roll immediately if the target has no
     * defense-class spec (0xff == "no creature/no defense record"). */
    if (hero->weapon_poison_class != 0xff) {
        int creature_present = (creature_idx >= 0);
        if (creature_present) {
            int16_t def_class = cb->creature_defense_class
                ? cb->creature_defense_class(ctx, creature_idx) : 0;
            int16_t armour_class = cb->creature_armour_class
                ? cb->creature_armour_class(ctx, creature_idx) : 0;
            int asleep = cb->is_creature_asleep
                ? cb->is_creature_asleep(ctx, creature_idx) : 0;

            /* dexterity-vs-defense to-hit gate (m_8BA8 in the source) */
            int r = cb->random ? cb->random(ctx, 32) : 0;
            int16_t threshold = (int16_t)((2 * def_class + def_class + r - 16) / 2);
            (void)armour_class;
            int16_t dex = hero->dexterity;
            if (!(asleep) && dex <= threshold) {
                int rd = cb->random_dir ? cb->random_dir(ctx) : 0;
                if (rd != 0 || (cb->use_luck && cb->use_luck(ctx, hero_idx, (int16_t)(75 - action_strength))))
                    hit = 1;
            }
        }

        if (hit) {
            /* strength-derived base roll, m_8C2A..m_8CF7 */
            int32_t strength_roll = action_strength;
            int rr = cb->random ? cb->random(ctx, action_strength / 2 + 1) : 0;
            strength_roll += rr;
            int32_t dmg = ((int32_t)strength_roll * hero->skill_fighter) >> 5;

            int r1 = cb->random ? cb->random(ctx, 32) : 0;
            int r2 = cb->random ? cb->random(ctx, 32) : 0;
            dmg += r1 - r2;

            int rd1 = cb->random_dir ? cb->random_dir(ctx) : 0;
            dmg += rd1 + 1;
            int rd2 = cb->random_dir ? cb->random_dir(ctx) : 0;
            dmg += rd2;

            /* fighter skill-threshold bonus */
            int roll32 = cb->random ? cb->random(ctx, 64) : 0;
            if (hero->skill_fighter > roll32)
                dmg += 10;

            /* poison-resistance roll */
            if (hero->weapon_poison_class != 0) {
                int roll20 = cb->random ? cb->random(ctx, 32) : 0;
                if (dmg > roll20 && cb->apply_creature_poison_resistance)
                    dmg += cb->apply_creature_poison_resistance(
                        ctx, creature_idx, (int16_t)hero->weapon_poison_class);
            }

            if (dmg < 0)
                dmg = 0;
            if (dmg > 0x7FFF)
                dmg = 0x7FFF;
            result = (int16_t)dmg;

            /* skill exp gain proportional to the damage dealt */
            if (cb->adjust_skills)
                cb->adjust_skills(ctx, hero_idx, skill_id, (dmg * 3) / 16 + 3);
        }
    }

    if (!hit) {
        /* miss: still a small residual roll (m_8DE8) */
        int rb = cb->random_bit ? cb->random_bit(ctx) : 0;
        result = (int16_t)(2 + rb);
    }

    if (cb->adjust_stamina)
        cb->adjust_stamina(ctx, hero_idx, action_strength);

    return result;
}

/* DM2_hero_39796 — c_hero.cpp:464. See header note: the real skproject
 * function at this identifier is the champion name-entry UI loop, which
 * has no non-UI state to port. We keep the tick-regen contract the
 * narrow API previously exposed for callers that need a per-tick hook. */
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

/* DM2_hero_2c1d_135d — c_hero.cpp:1403 (armour defense percentage) */
int32_t dm2_v1_hero_2c1d_135d(
    int hero_idx, uint16_t body_item_slot[6], uint16_t hand_item[2],
    int is_used, uint32_t body_flag,
    const DM2_V1_DefenseCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    (void)hand_item;

    int32_t defense = 0;

    /* body slots 0-5 */
    for (int slot = 0; slot < 6; slot++) {
        if (!(body_flag & (1u << slot)))
            continue;
        int16_t ac = cb->item_armour_class
            ? cb->item_armour_class(ctx, body_item_slot[slot]) : 0;
        int32_t shield_flag = (slot == 4) ? 0x8000 : 0;
        int32_t contribution = dm2_v1_hero_2c1d_132c_helper(ac, is_used) | shield_flag;
        defense += contribution & 0xffff;
    }

    /* hand slots 0-1 */
    int32_t hand_bonus = 0;
    for (int hand = 0; hand < 2; hand++) {
        int16_t dc = cb->hand_defense_class
            ? cb->hand_defense_class(ctx, hero_idx, hand) : 0;
        hand_bonus += dc;
    }

    int16_t adj_dex = cb->ability_defense_bonus
        ? cb->ability_defense_bonus(ctx, hero_idx) : 0;
    int r = cb->random ? cb->random(ctx, adj_dex / 8 + 1) : 0;
    defense += r;
    if (is_used)
        defense >>= 1;
    defense += hand_bonus;

    if (cb->is_asleep && cb->is_asleep(ctx, hero_idx))
        defense >>= 1;

    defense /= 2;
    if (defense < 0)
        defense = 0;
    if (defense > 100)
        defense = 100;
    return defense;
}

/* DM2_ADJUST_SKILLS — c_hero.cpp:1166 */
int32_t dm2_v1_adjust_skills(
    int hero_idx, int32_t skill_id, int32_t exp_gain,
    const DM2_V1_AdjustSkillsCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;

    /* time-window halving for combat (4..11) skill ids */
    if (skill_id >= 4 && skill_id <= 11) {
        int16_t tick = cb->get_gametick ? cb->get_gametick(ctx) : 0;
        int16_t window = cb->get_exp_window_tick ? cb->get_exp_window_tick(ctx) : 0;
        if ((int32_t)(tick - 150) > window)
            exp_gain >>= 1;
    }
    if (exp_gain == 0)
        return 0;

    int16_t scale = cb->get_exp_scale ? cb->get_exp_scale(ctx) : 0;
    if (scale != 0)
        exp_gain *= scale;

    int group = (int)skill_id;
    if (group >= 4) {
        group -= 4;
        group >>= 2;
    }
    int16_t level_before = cb->get_skill_lv ? cb->get_skill_lv(ctx, hero_idx, group, 0) : 0;

    if (skill_id >= 4) {
        int16_t tick = cb->get_gametick ? cb->get_gametick(ctx) : 0;
        int16_t window = cb->get_exp_window_tick ? cb->get_exp_window_tick(ctx) : 0;
        if (tick - 40 < window)
            exp_gain *= 2;
    }

    if (cb->add_skill_exp) {
        cb->add_skill_exp(ctx, hero_idx, (int)skill_id, exp_gain);
        if (skill_id >= 4)
            cb->add_skill_exp(ctx, hero_idx, group, exp_gain);
    }

    int16_t level_after = cb->get_skill_lv ? cb->get_skill_lv(ctx, hero_idx, group, 0) : 0;

    for (int32_t pass = level_before; (uint16_t)pass < (uint16_t)level_after; pass++) {
        int rb0 = cb->random_bit ? cb->random_bit(ctx) : 0;
        int rb1 = cb->random_bit ? cb->random_bit(ctx) : 0;
        int8_t vit_delta = (int8_t)(rb1 + 1);
        int rb2 = cb->random_bit ? cb->random_bit(ctx) : 0;
        int32_t vit_gain = rb2;
        if (group != 2)
            vit_gain &= rb0;
        if (cb->add_ability_max)
            cb->add_ability_max(ctx, hero_idx, /*E_VITALITY*/ 0, (int16_t)vit_gain);

        int rb3 = cb->random_bit ? cb->random_bit(ctx) : 0;
        int16_t antifire_gain = (int16_t)(rb3 & ~rb0);
        if (cb->add_ability_max)
            cb->add_ability_max(ctx, hero_idx, /*E_ANTIFIRE*/ 1, antifire_gain);

        int16_t hp_gain, sta_gain;
        int16_t maxhp, maxsta, maxmp;
        switch (group) {
        case 0: /* fighter */
            if (cb->add_ability_max) {
                cb->add_ability_max(ctx, hero_idx, /*E_STRENGTH*/ 2, vit_delta);
                cb->add_ability_max(ctx, hero_idx, /*E_DEXTERITY*/ 3, (int16_t)rb2);
            }
            break;
        case 1: /* ninja */
            if (cb->add_ability_max) {
                cb->add_ability_max(ctx, hero_idx, /*E_STRENGTH*/ 2, (int16_t)rb2);
                cb->add_ability_max(ctx, hero_idx, /*E_DEXTERITY*/ 3, vit_delta);
            }
            break;
        case 2: /* wizard */
            maxmp = cb->get_max_mp ? cb->get_max_mp(ctx, hero_idx) : 0;
            if (cb->set_max_mp)
                cb->set_max_mp(ctx, hero_idx, (int16_t)(maxmp + 1));
            if (cb->add_ability_max)
                cb->add_ability_max(ctx, hero_idx, /*E_WIZARDRY*/ 4, (int16_t)rb2);
            break;
        case 3: /* priest */
            maxmp = cb->get_max_mp ? cb->get_max_mp(ctx, hero_idx) : 0;
            if (cb->set_max_mp)
                cb->set_max_mp(ctx, hero_idx, (int16_t)(maxmp + 1));
            if (cb->add_ability_max)
                cb->add_ability_max(ctx, hero_idx, /*E_WIZARDRY*/ 4, vit_delta);
            break;
        default:
            break;
        }
        if (group == 2 || group == 3) {
            maxmp = cb->get_max_mp ? cb->get_max_mp(ctx, hero_idx) : 0;
            int rd = 0; /* DM2_RANDDIR() range placeholder */
            maxmp = (int16_t)(maxmp + rd);
            if (maxmp > 0x384)
                maxmp = 0x384;
            if (cb->set_max_mp)
                cb->set_max_mp(ctx, hero_idx, maxmp);
            if (cb->add_ability_max)
                cb->add_ability_max(ctx, hero_idx, /*E_ANTIMAGIC*/ 5, 0);
        }

        hp_gain = (int16_t)(1 + rb2);
        maxhp = cb->get_max_hp ? cb->get_max_hp(ctx, hero_idx) : 0;
        maxhp = (int16_t)(maxhp + hp_gain);
        if (maxhp > 999)
            maxhp = 999;
        if (cb->set_max_hp)
            cb->set_max_hp(ctx, hero_idx, maxhp);

        sta_gain = (int16_t)(1 + rb2);
        maxsta = cb->get_max_stamina ? cb->get_max_stamina(ctx, hero_idx) : 0;
        maxsta = (int16_t)(maxsta + sta_gain);
        if (maxsta > 9999)
            maxsta = 9999;
        if (cb->set_max_stamina)
            cb->set_max_stamina(ctx, hero_idx, maxsta);

        if (cb->mark_hero_dirty)
            cb->mark_hero_dirty(ctx, hero_idx);
        if (cb->display_level_up_hint)
            cb->display_level_up_hint(ctx, hero_idx, group);
    }

    return level_after;
}

/* DM2_WOUND_PLAYER — c_hero.cpp:1496 */
int16_t dm2_v1_wound_player(
    int hero_idx, DM2_V1_WoundPlayerHero *hero, int16_t wound,
    uint32_t body_mask,
    const DM2_V1_WoundPlayerCallbacks *cb, void *ctx)
{
    if (!hero || !cb)
        return 0;
    if (cb->hero_exists && !cb->hero_exists(ctx, hero_idx))
        return 0;
    if (cb->is_sleeping && cb->is_sleeping(ctx))
        return 0;
    if (wound <= 0)
        return 0;
    if (hero->cur_hp == 0)
        return 0;

    int ignore_armour = (body_mask & 0x8000) != 0;
    uint32_t mask = body_mask & 0x7fff;

    if (mask != 0) {
        int is_used = ignore_armour;
        int32_t defense_pct = cb->hero_defense_pct
            ? cb->hero_defense_pct(ctx, hero_idx, mask, is_used) : 0;

        /* skill-based partial mitigation for a "shocking" body-part hit */
        {
            int16_t lv = cb->skill_lv ? cb->skill_lv(ctx, hero_idx, 7, 1) : 0;
            int roll = cb->random ? cb->random(ctx, 16) : 0;
            if ((int32_t)lv + (int32_t)(mask / 8) > roll) {
                if (ignore_armour) {
                    wound = (int16_t)(wound - (int16_t)mask);
                    if (wound <= 0)
                        return 0;
                }
                defense_pct += (int32_t)mask / 4;
            }
        }

        wound = (int16_t)((int32_t)wound * (130 - defense_pct) / 64);
        if (wound <= 0)
            return 0;

        if (cb->is_sleeping && cb->resume_from_wake)
            cb->resume_from_wake(ctx);

        if (wound > 10) {
            int roll = cb->random ? cb->random(ctx, 128) : 0;
            if (cb->add_body_status)
                cb->add_body_status(ctx, hero_idx, (uint16_t)(1u << (roll & 7)));
        }
    }

    if (wound <= 0)
        return 0;

    hero->hero_flag |= 0x0800;
    if (cb->accumulate_pending_damage)
        cb->accumulate_pending_damage(ctx, hero_idx, wound);
    return wound;
}

/* DM2_UPDATE_CHAMPIONS_STATS — c_hero.cpp:1757 */
void dm2_v1_update_champions_stats(
    const DM2_V1_UpdateChampionsStatsCallbacks *cb, void *ctx)
{
    if (!cb || cb->hero_count == 0)
        return;

    int16_t accum = cb->get_stat_tick_accum ? cb->get_stat_tick_accum(ctx) : 0;
    accum = (int16_t)(accum + 0x38);
    if (accum > 0x80)
        accum = (int16_t)(accum - 0x80);
    if (cb->set_stat_tick_accum)
        cb->set_stat_tick_accum(ctx, accum);

    for (int i = 0; i < cb->hero_count; i++) {
        if (cb->is_held_item_slot && cb->is_held_item_slot(ctx, i))
            continue;
        if (cb->hero_alive && !cb->hero_alive(ctx, i))
            continue;
        cb->recompute_hero_stats(ctx, i);
    }
}

/* DM2_WIELD_WEAPON — c_hero.cpp:2064 */
int dm2_v1_wield_weapon(
    int hero_idx, int16_t x, int16_t y, int hero_partypos, int hero_absdir,
    int16_t action_strength, int require_melee,
    const DM2_V1_WieldWeaponCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;

    int creature_idx = cb->get_creature_at ? cb->get_creature_at(ctx, x, y) : -1;
    if (creature_idx < 0)
        return 0;

    /* refuse to strike through an ally standing in the way */
    int rel = (hero_partypos + 4 - hero_absdir) & 3;
    if (rel >= 2) {
        int side = (rel <= 2) ? 3 : 1;
        int target_pos = (side + hero_partypos) & 3;
        if (cb->get_player_at_position &&
            cb->get_player_at_position(ctx, target_pos) != -1) {
            return 0;
        }
    }

    if (require_melee && cb->creature_ai_throw_only &&
        cb->creature_ai_throw_only(ctx, creature_idx)) {
        return 0;
    }

    int16_t dmg = cb->calc_attack_damage
        ? cb->calc_attack_damage(ctx, hero_idx, creature_idx, action_strength, 0)
        : 0;
    if (cb->set_pending_combat_damage)
        cb->set_pending_combat_damage(ctx, dmg);
    return 1;
}

/* DM2_REMOVE_OBJECT_FROM_HAND — c_hero.cpp:2354 */
int32_t dm2_v1_remove_object_from_hand_ex(
    DM2_V1_RemoveFromHandState *state, int event_hero_idx,
    const DM2_V1_RemoveFromHandCallbacks *cb, void *ctx)
{
    if (!state)
        return -1;
    int16_t item = state->hand_item;
    if (item == -1)
        return -1;

    state->hand_weight = 0;
    state->hand_flags = -1;
    state->hand_item = -1;

    if (cb) {
        if (cb->hide_mouse)
            cb->hide_mouse(ctx);
        if (cb->show_mouse)
            cb->show_mouse(ctx);
        if (cb->process_item_bonus_release)
            cb->process_item_bonus_release(ctx, event_hero_idx, (uint16_t)item);
        if (cb->relink_item_to_view_tile)
            cb->relink_item_to_view_tile(ctx, (uint16_t)item);
    }
    return item;
}

/* DM2_PLAYER_DEFEATED — c_hero.cpp:2636 */
void dm2_v1_player_defeated(
    const DM2_V1_PlayerDefeatedInfo *info,
    const DM2_V1_PlayerDefeatedCallbacks *cb, void *ctx)
{
    if (!info || !cb)
        return;

    if (info->is_active_hero && cb->refresh_squad_hands_panel)
        cb->refresh_squad_hands_panel(ctx);

    if (info->is_last_visible_champion) {
        if (info->is_mouse_drag_capture) {
            if (cb->release_mouse_capture)
                cb->release_mouse_capture(ctx);
            if (cb->get_hand_item) {
                int16_t item = cb->get_hand_item(ctx);
                if (item != -1 && cb->display_taken_item_name)
                    cb->display_taken_item_name(ctx, (uint16_t)item);
            }
            if (cb->show_mouse)
                cb->show_mouse(ctx);
        }
        if (info->is_mouse_select_capture) {
            if (cb->release_mouse_capture)
                cb->release_mouse_capture(ctx);
            if (cb->show_mouse)
                cb->show_mouse(ctx);
        }
        if (cb->refresh_champion_display)
            cb->refresh_champion_display(ctx, 4);
    }

    if (info->is_event_capture) {
        if (cb->release_mouse_capture)
            cb->release_mouse_capture(ctx);
        if (cb->show_mouse)
            cb->show_mouse(ctx);
    }

    if (cb->drop_player_items)
        cb->drop_player_items(ctx, info->hero_idx);
    if (cb->mark_hero_dead)
        cb->mark_hero_dead(ctx, info->hero_idx);
}

/* DM2_PROCESS_PLAYERS_DAMAGE — c_hero.cpp:2777 */
void dm2_v1_process_players_damage(
    const DM2_V1_ProcessPlayersDamageCallbacks *cb, void *ctx)
{
    if (!cb)
        return;

    for (int i = 0; i < cb->hero_count; i++) {
        if (cb->take_pending_body_status && cb->merge_body_status) {
            uint16_t status = cb->take_pending_body_status(ctx, i);
            if (status != 0)
                cb->merge_body_status(ctx, i, status);
        }

        int16_t wound = cb->take_pending_damage ? cb->take_pending_damage(ctx, i) : 0;
        if (wound == 0)
            continue;

        int16_t hp = cb->get_cur_hp ? cb->get_cur_hp(ctx, i) : 0;
        if (hp == 0)
            continue;

        hp = (int16_t)(hp - wound);
        if (hp > 0) {
            if (cb->set_cur_hp)
                cb->set_cur_hp(ctx, i, hp);
            if (cb->set_damage_suffered)
                cb->set_damage_suffered(ctx, i, wound);
            if (cb->mark_hero_dirty)
                cb->mark_hero_dirty(ctx, i);

            int timer_idx = cb->get_hero_timer ? cb->get_hero_timer(ctx, i) : -1;
            if (timer_idx != -1) {
                if (cb->rearm_hero_timer)
                    cb->rearm_hero_timer(ctx, i, timer_idx, 5);
            } else if (cb->queue_hero_hit_timer) {
                cb->queue_hero_hit_timer(ctx, i, 5);
            }
        } else {
            if (cb->player_defeated)
                cb->player_defeated(ctx, i);
        }
    }
}

/* DM2_PLAYER_CONSUME_OBJECT — c_hero.cpp:2998 */
int dm2_v1_player_consume_object(
    int hero_idx, uint16_t item,
    const DM2_V1_PlayerConsumeObjectCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int category = cb->get_item_category ? cb->get_item_category(ctx, item) : -1;
    if (cb->item_is_stackable_charge && cb->item_is_stackable_charge(ctx, item)) {
        if (cb->decrement_item_charge)
            cb->decrement_item_charge(ctx, item);
    }
    return cb->dispatch ? cb->dispatch(ctx, hero_idx, item, category) : 0;
}

/* DM2_CHANGE_PLAYER_POS — c_hero.cpp:4037 */
void dm2_v1_change_player_pos(
    int16_t packed_pos, const DM2_V1_ChangePlayerPosState *state,
    const DM2_V1_ChangePlayerPosCallbacks *cb, void *ctx)
{
    if (!cb || !state)
        return;

    int target_slot = packed_pos & 0x7fff;
    int also_dir = state->also_swap_direction;

    int target_hero = cb->get_player_at_position
        ? cb->get_player_at_position(ctx, target_slot) : -1;

    if (state->drag_in_progress) {
        int src_hero = cb->get_player_at_position
            ? cb->get_player_at_position(ctx, state->drag_source_position) : -1;
        if (also_dir && src_hero != -1 && cb->set_direction && cb->get_party_facing) {
            cb->set_direction(ctx, src_hero, cb->get_party_facing(ctx));
        }
        if (state->drag_source_position != target_slot) {
            if (target_hero != -1 && cb->swap_positions)
                cb->swap_positions(ctx, target_hero, src_hero);
        }
    } else if (target_hero != -1 && cb->swap_positions) {
        cb->swap_positions(ctx, target_hero, target_slot);
    }
}

/* DM2_hero_2c1d_1de2 — c_hero.cpp:3903 (throw item) */
int32_t dm2_v1_hero_2c1d_1de2(
    int hero_idx, int slot, int16_t x, int16_t y, int direction,
    const DM2_V1_ThrowItemCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;

    int16_t item;
    if (slot < 0) {
        item = cb->remove_from_hand ? cb->remove_from_hand(ctx) : -1;
        if (item == -1)
            return 0;
        if (cb->set_hand_slot1)
            cb->set_hand_slot1(ctx, hero_idx, item);
        slot = 1;
    } else {
        item = cb->remove_possession ? cb->remove_possession(ctx, hero_idx, slot) : -1;
        if (item == -1)
            return 0;
    }

    int16_t strength = cb->compute_throw_strength
        ? cb->compute_throw_strength(ctx, hero_idx, slot) : 0;

    if (cb->queue_launch_noise)
        cb->queue_launch_noise(ctx, (uint16_t)item, x, y);
    if (cb->shoot_item)
        cb->shoot_item(ctx, (uint16_t)item, x, y, direction, strength);
    return 1;
}

/* DM2_REVIVE_PLAYER — c_hero.cpp:957 */
void dm2_v1_revive_player(
    DM2_V1_HeroRecord *hero, int8_t hero_type, int8_t direction,
    const DM2_V1_RevivePlayerCallbacks *cb, void *ctx)
{
    if (!hero || !cb)
        return;

    for (int i = 0; i < 30; i++)
        hero->item[i] = (uint16_t)-1;

    int free_pos = cb->find_free_position ? cb->find_free_position(ctx, direction) : 0;
    hero->partypos = (int8_t)free_pos;
    hero->absdir = direction;

    hero->cur_hp = hero->max_hp = (int16_t)(cb->hp_base * 10);
    hero->cur_stamina = hero->max_stamina = (int16_t)(cb->stamina_base * 10);
    hero->cur_mp = hero->max_mp = (int16_t)(cb->mp_base * 10);

    for (int i = 0; i < 7; i++) {
        int16_t v = cb->ability_base[i];
        if (v < 30)
            v = 30;
        hero->ability_cur[i] = hero->ability_max[i] = v;
    }

    for (int i = 0; i < 16; i++) {
        int32_t v = 0;
        if (cb->skill_level[i] != 0)
            v = 0x40 << (uint8_t)cb->skill_level[i];
        hero->skill[i / 4][i % 4] = v;
    }
    for (int g = 0; g < 4; g++) {
        int32_t sum = 0;
        for (int i = 0; i < 4; i++)
            sum += hero->skill[g][i];
        /* group total stored alongside sub-skills, matching the
         * skill[0][group] layout used by DM2_QUERY_PLAYER_SKILL_LV */
        hero->skill[0][g] = sum;
    }

    int r1 = cb->random ? cb->random(ctx) : 0;
    int r2 = cb->random ? cb->random(ctx) : 0;
    hero->food = (uint16_t)((r1 & 0xff) + 1500);
    hero->water = (uint16_t)((r2 & 0xff) + 1500);
    (void)hero_type;
}

/* DM2_SELECT_CHAMPION — c_hero.cpp:1052 */
void dm2_v1_select_champion_narrow(
    const DM2_V1_SelectChampionState *state, int creation_map_id,
    int previous_map_id, int new_hero_idx,
    const DM2_V1_SelectChampionCallbacks *cb, void *ctx)
{
    if (!state || !cb)
        return;
    if (state->hand_item_captured || state->already_full)
        return;

    if (cb->change_current_map)
        cb->change_current_map(ctx, creation_map_id);

    if (cb->revive_player)
        cb->revive_player(ctx, state->hero_type, state->direction);

    if (new_hero_idx == 0 && cb->select_leader)
        cb->select_leader(ctx, 0);

    if (cb->scavenge_creation_tile_items)
        cb->scavenge_creation_tile_items(ctx, new_hero_idx);

    if (cb->refresh_champion_strip)
        cb->refresh_champion_strip(ctx);

    if (cb->change_current_map)
        cb->change_current_map(ctx, previous_map_id);

    if (cb->recompute_player_weight)
        cb->recompute_player_weight(ctx, new_hero_idx);
}

/* =====================================================================
 * c_engage.cpp
 * ===================================================================== */

int32_t dm2_v1_engage_command_narrow(
    int hero_idx, int command_id,
    const DM2_V1_EngageCommandCallbacks *cb, void *ctx)
{
    if (!cb || !cb->dispatch)
        return -1;
    if (cb->hero_is_dead && cb->hero_is_dead(ctx, hero_idx))
        return 0;

    int alt_flag = (command_id & DM2_V1_ENGAGE_CMD_ALT_FLAG) != 0;
    int raw_cmd = command_id & DM2_V1_ENGAGE_CMD_MASK;
    int resolved_cmd = cb->set_handcmd_and_resolve
        ? cb->set_handcmd_and_resolve(ctx, hero_idx, raw_cmd) : raw_cmd;

    if ((uint16_t)(resolved_cmd - DM2_V1_ENGAGE_CMD_MIN) > (DM2_V1_ENGAGE_CMD_MAX - DM2_V1_ENGAGE_CMD_MIN))
        return 0;

    return cb->dispatch(ctx, hero_idx, resolved_cmd, alt_flag);
}

/* =====================================================================
 * c_creature.cpp
 * ===================================================================== */

int32_t dm2_v1_wound_creature(
    int creature_idx, DM2_V1_WoundCreatureState *creature, int16_t damage,
    const DM2_V1_WoundCreatureCallbacks *cb, void *ctx)
{
    if (!creature || !cb)
        return 0;
    if (creature->defense_type == 0xff)
        return 0;

    if (!(creature->ai_flags1 & 0x4) && cb->notify_cross_map_wake)
        cb->notify_cross_map_wake(ctx, creature_idx);

    int16_t hp = (int16_t)(creature->cur_hp - damage);
    if (hp > 0) {
        creature->cur_hp = hp;
        return 0;
    }

    if ((creature->ai_flags1 & 0x8) && cb->can_creature_die &&
        !cb->can_creature_die(ctx, creature_idx)) {
        /* not allowed to die right now: survive at 1 hp */
        creature->cur_hp = 1;
        return 0;
    }

    creature->cur_hp = 0;

    if ((creature->ai_flags1 & 0x8) && cb->set_ai_wander_timeout)
        cb->set_ai_wander_timeout(ctx, creature_idx, 0xa0);

    if ((creature->ai_flags1 & 0x80) && damage < creature->max_hp) {
        /* non-lethal in spirit but hp already <=0 here means lethal;
         * skproject's flee check only applies to non-lethal hits, which
         * cannot reach this branch, kept for parity documentation. */
    }

    if (creature->kill_flag & 1) {
        if (cb->creature_defeated)
            cb->creature_defeated(ctx, creature_idx);
        return 1;
    }

    if (cb->play_death_animation)
        cb->play_death_animation(ctx, creature_idx);
    return 0;
}

int32_t dm2_v1_creature_attacks_player(
    int creature_idx, int hero_idx,
    const DM2_V1_CreatureAttacksPlayerCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;

    DM2_V1_CreatureAttackProfile profile;
    if (!cb->get_attack_profile || !cb->get_attack_profile(ctx, creature_idx, &profile))
        return 0;

    int hit = 0;
    if (profile.is_surprise_attack || profile.attack_type == 8 || profile.attack_type == 9) {
        hit = 1;
    } else {
        int dex = cb->get_hero_dexterity ? cb->get_hero_dexterity(ctx, hero_idx) : 0;
        int roll = cb->random ? cb->random(ctx, 32) : 0;
        if (profile.to_hit_base + roll >= dex)
            hit = 1;
        else if (cb->hero_use_luck && cb->hero_use_luck(ctx, hero_idx, 100 - dex))
            hit = 1;
    }
    if (!hit)
        return 0;

    int32_t dmg = profile.max_damage;
    int r1 = cb->random ? cb->random(ctx, dmg / 2 + 1) : 0;
    dmg += r1;
    int r2 = cb->random ? cb->random(ctx, dmg / 2 + 1) : 0;
    dmg += r2;
    if (dmg < 1)
        dmg = 1;

    int16_t applied = cb->wound_player ? cb->wound_player(ctx, hero_idx, (int16_t)dmg) : 0;

    if (cb->queue_hit_noise)
        cb->queue_hit_noise(ctx, creature_idx, hero_idx, applied);

    if (profile.poison_chance != 0 && cb->hero_resist_poison &&
        !cb->hero_resist_poison(ctx, hero_idx, profile.poison_chance)) {
        if (cb->process_poison)
            cb->process_poison(ctx, hero_idx, profile.poison_chance);
    }

    if (profile.is_surprise_attack && cb->resume_from_wake)
        cb->resume_from_wake(ctx);

    return applied;
}

int32_t dm2_v1_creature_attacks_creature(
    int attacker_idx, int16_t target_x, int16_t target_y,
    const DM2_V1_CreatureAttacksCreatureCallbacks *cb, void *ctx)
{
    if (!cb)
        return -1;

    int defender_idx = cb->get_creature_at ? cb->get_creature_at(ctx, target_x, target_y) : -1;
    if (defender_idx < 0)
        return -1;

    int defense = cb->get_defense ? cb->get_defense(ctx, defender_idx) : 0;
    if (defense == 0xff)
        return 0;

    int to_hit = cb->get_to_hit ? cb->get_to_hit(ctx, attacker_idx) : 0;
    int r1 = cb->random ? cb->random(ctx, 32) : 0;
    int r2 = cb->random ? cb->random(ctx, 32) : 0;
    int hit = ((to_hit + r1) >= (defense + r2));
    if (!hit && cb->rand_dir && cb->rand_dir(ctx) == 0)
        hit = 1;
    if (!hit)
        return 0;

    int32_t dmg = cb->get_max_damage ? cb->get_max_damage(ctx, attacker_idx) : 0;
    int rd1 = cb->random ? cb->random(ctx, dmg / 2 + 1) : 0;
    dmg += rd1;
    int rd2 = cb->random ? cb->random(ctx, dmg / 2 + 1) : 0;
    dmg += rd2;
    if (dmg < 1)
        dmg = 1;

    if (cb->apply_creature_damage)
        cb->apply_creature_damage(ctx, attacker_idx, defender_idx, (int16_t)dmg);
    return dmg;
}

int32_t dm2_v1_creature_can_handle_it(
    const DM2_V1_CreatureHandleCaps *caps, uint16_t item, int flags,
    const DM2_V1_CreatureCanHandleCallbacks *cb, void *ctx)
{
    if (!caps)
        return 0;
    int item_class = item & 0x3f;
    if (item_class == 0x3e || item_class == 0x3f)
        return 0;

    if (item_class == 0x29 && cb && cb->is_container_moneybox &&
        cb->is_container_moneybox(ctx, item)) {
        if (cb->moneybox_already_opened && cb->moneybox_already_opened(ctx, item))
            return (caps->handle_mask & (1u << item_class)) != 0;
        return (flags & 0x80) != 0;
    }

    return (caps->handle_mask & ((uint32_t)1 << item_class)) != 0;
}

int32_t dm2_v1_creature_cast_spell(
    int creature_idx, const DM2_V1_CreatureCastSpellCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;

    DM2_V1_CreatureSpellProfile profile;
    if (!cb->get_spell_profile || !cb->get_spell_profile(ctx, creature_idx, &profile))
        return 0;

    int32_t d = profile.base_power / 4 + 1;
    if (cb->random) {
        d += cb->random(ctx, d > 0 ? d : 1);
        d += cb->random(ctx, d > 0 ? d : 1);
    }
    if (profile.spell_item == 0 || (profile.spell_item & 1))
        d *= 4;
    else
        d <<= 3;
    if (d < 4)
        d = 4;
    if (d > 255)
        d = 255;

    int8_t power;
    if (d >= 32)
        power = 7;
    else if (d >= 16)
        power = 3;
    else if (d >= 8)
        power = 2;
    else
        power = 1;

    if (cb->shoot_item)
        cb->shoot_item(ctx, creature_idx, profile.spell_item, profile.direction,
                        (int8_t)d, profile.speed, power);

    int32_t applied = 0;
    if (!profile.no_recoil && cb->wound_self) {
        applied = cb->wound_self(ctx, creature_idx, (int16_t)d);
    }
    if (cb->mark_creature_dirty)
        cb->mark_creature_dirty(ctx, creature_idx);
    return applied != 0 ? 1 : 0;
}

int32_t dm2_v1_creature_steal_from_champion(
    int creature_idx, const DM2_V1_CreatureStealCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;

    if (cb->same_tile_as_creature && !cb->same_tile_as_creature(ctx, creature_idx))
        return 1;

    int hero_idx = cb->get_player_at_creature_position
        ? cb->get_player_at_creature_position(ctx, creature_idx) : -1;
    if (hero_idx < 0)
        return 1;

    int dex = cb->get_hero_dexterity ? cb->get_hero_dexterity(ctx, hero_idx) : 0;
    if (cb->hero_use_luck && cb->hero_use_luck(ctx, hero_idx, 100 - dex))
        return 1;

    int best_hand = -1;
    int best_item = -1;
    int best_weight = -1;
    for (int hand = 0; hand < 2; hand++) {
        int item = cb->get_hero_hand_item ? cb->get_hero_hand_item(ctx, hero_idx, hand) : -1;
        if (item < 0)
            continue;
        if (cb->creature_can_handle_it && !cb->creature_can_handle_it(ctx, creature_idx, (uint16_t)item))
            continue;
        int weight = cb->get_item_weight ? cb->get_item_weight(ctx, (uint16_t)item) : 0;
        if (weight > best_weight) {
            best_weight = weight;
            best_hand = hand;
            best_item = item;
        }
    }
    if (best_hand < 0)
        return 1;

    int32_t taken = cb->remove_possession ? cb->remove_possession(ctx, hero_idx, best_hand) : -1;
    if (taken < 0)
        return 1;
    if (cb->give_creature_item)
        cb->give_creature_item(ctx, creature_idx, taken);

    if (cb->is_party_surprised && cb->is_party_surprised(ctx)) {
        if (cb->clear_party_surprised)
            cb->clear_party_surprised(ctx);
        if (cb->resume_from_wake)
            cb->resume_from_wake(ctx);
    }
    (void)best_item;
    return 0;
}

int32_t dm2_v1_creature_ccm0b(
    int creature_idx, const DM2_V1_CreatureCcm0bCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;
    if (!cb->go_there || cb->go_there(ctx, creature_idx))
        return 1;
    if (cb->get_target_tile_kind && cb->get_target_tile_kind(ctx, creature_idx) != 0xb)
        return 1;
    if (cb->invoke_message)
        cb->invoke_message(ctx, creature_idx, 0);
    return 0;
}

int32_t dm2_v1_creature_ccm0c(
    int creature_idx, const DM2_V1_CreatureCcm0cCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;

    uint8_t phase = cb->get_phase ? cb->get_phase(ctx, creature_idx) : 0;
    int32_t result = 0;

    if (cb->ccm06)
        cb->ccm06(ctx, creature_idx);

    if (phase == 0) {
        if (cb->arm_item_pickup_target)
            cb->arm_item_pickup_target(ctx, creature_idx);
    } else {
        result = cb->takes_item ? cb->takes_item(ctx, creature_idx) : 0;
    }

    if (cb->advance_phase)
        cb->advance_phase(ctx, creature_idx);
    return result;
}

int32_t dm2_v1_creature_uses_ladder_hole(
    int creature_idx, const DM2_V1_CreatureUsesLadderHoleCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;
    if (!cb->go_there || cb->go_there(ctx, creature_idx))
        return 1;
    if (cb->get_tile_traversable && !cb->get_tile_traversable(ctx, creature_idx))
        return 1;

    int kind = cb->get_tile_kind ? cb->get_tile_kind(ctx, creature_idx) : 0;
    int level = 0;
    int16_t lx = 0, ly = 0;
    int dir = 0;
    int have_landing = 0;

    if (kind == 0x39 || kind == 0x3a) {
        int going_up = (kind == 0x39);
        if (cb->find_ladder_landing)
            have_landing = cb->find_ladder_landing(ctx, creature_idx, going_up,
                                                    &level, &lx, &ly, &dir);
        if (!have_landing && cb->rand_dir)
            dir = cb->rand_dir(ctx);
    } else if (kind == 0x35 || kind == 0x36) {
        have_landing = 1;
    }

    if (cb->is_leaving_pit_tele_tile && cb->is_leaving_pit_tele_tile(ctx, creature_idx)) {
        if (cb->operate_pit_tele_tile)
            cb->operate_pit_tele_tile(ctx, lx, ly, 0);
    }

    if (cb->move_creature_to_level &&
        cb->move_creature_to_level(ctx, creature_idx, level, lx, ly, dir) != 0)
        return 1;

    if (cb->reposition_from_landing)
        cb->reposition_from_landing(ctx, creature_idx);
    if (cb->operate_pit_tele_tile)
        cb->operate_pit_tele_tile(ctx, lx, ly, 1);
    if (cb->decrement_pending_move_counter)
        cb->decrement_pending_move_counter(ctx, creature_idx);
    if (cb->mark_creature_dirty)
        cb->mark_creature_dirty(ctx, creature_idx);
    (void)have_landing;
    return 0;
}

int32_t dm2_v1_creature_walk_now(
    int creature_idx, const DM2_V1_CreatureWalkNowCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;

    if (cb->is_party_on_tile && cb->is_party_on_tile(ctx, creature_idx)) {
        if (cb->force_attack_pose)
            cb->force_attack_pose(ctx, creature_idx);
        if (cb->attacks_party)
            cb->attacks_party(ctx, creature_idx);
    }

    if (!cb->go_there || cb->go_there(ctx, creature_idx))
        return 1;
    if (cb->get_tile_walkable && !cb->get_tile_walkable(ctx, creature_idx))
        return 1;

    if (cb->is_leaving_pit_tele_tile && cb->is_leaving_pit_tele_tile(ctx, creature_idx)) {
        if (cb->operate_pit_tele_tile)
            cb->operate_pit_tele_tile(ctx, 0, 0, 0);
    }

    if (cb->move_creature_to_target && cb->move_creature_to_target(ctx, creature_idx) != 0)
        return 1;

    if (cb->reposition_from_landing)
        cb->reposition_from_landing(ctx, creature_idx);
    if (cb->operate_pit_tele_tile)
        cb->operate_pit_tele_tile(ctx, 0, 0, 1);
    if (cb->decrement_pending_move_counter)
        cb->decrement_pending_move_counter(ctx, creature_idx);
    if (cb->mark_creature_dirty)
        cb->mark_creature_dirty(ctx, creature_idx);
    return 0;
}

int32_t dm2_v1_creature_activates_wall(
    int creature_idx, const DM2_V1_CreatureActivatesWallCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;
    if (!cb->go_there_wall || cb->go_there_wall(ctx, creature_idx))
        return 1;

    int kind = cb->get_activation_kind ? cb->get_activation_kind(ctx, creature_idx) : 0;
    int held = cb->get_held_item ? cb->get_held_item(ctx, creature_idx) : -1;
    int32_t item = held;

    if (!(kind == 2 && held < 0)) {
        if (held < 0) {
            item = cb->pick_up_item_for_activation
                ? cb->pick_up_item_for_activation(ctx, creature_idx) : -1;
            if (item < 0)
                return 1;
        }
    }

    if (cb->activate_wall_event)
        cb->activate_wall_event(ctx, creature_idx, item);
    return 0;
}

int32_t dm2_v1_place_merchandise(
    int creature_idx, const DM2_V1_PlaceMerchandiseCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;
    if (!cb->is_merchant || !cb->is_merchant(ctx, creature_idx))
        return 1;

    int placed_any = 0;
    for (;;) {
        int32_t item = cb->next_inventory_item ? cb->next_inventory_item(ctx, creature_idx) : -1;
        if (item < 0)
            break;
        if (cb->is_container_moneybox && cb->is_container_moneybox(ctx, item)) {
            if (cb->mark_moneybox_opened)
                cb->mark_moneybox_opened(ctx, item);
        }
        if (cb->place_item_on_shop_tile)
            cb->place_item_on_shop_tile(ctx, creature_idx, item);
        if (!placed_any && cb->queue_shop_noise)
            cb->queue_shop_noise(ctx, creature_idx, item);
        placed_any = 1;
        if (!cb->bulk_transfer_active || !cb->bulk_transfer_active(ctx, creature_idx))
            break;
    }
    return placed_any ? 0 : 1;
}

int32_t dm2_v1_take_merchandise(
    int creature_idx, const DM2_V1_TakeMerchandiseCallbacks *cb, void *ctx)
{
    if (!cb)
        return 1;
    if (!cb->is_merchant || !cb->is_merchant(ctx, creature_idx))
        return 1;

    int taken_any = 0;
    for (;;) {
        int32_t item = cb->next_shop_item ? cb->next_shop_item(ctx, creature_idx) : -1;
        if (item < 0)
            break;
        if (cb->take_item_into_inventory)
            cb->take_item_into_inventory(ctx, creature_idx, item);
        taken_any = 1;
        if (!cb->bulk_transfer_active || !cb->bulk_transfer_active(ctx, creature_idx))
            break;
    }
    return taken_any ? 0 : 1;
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

int32_t dm2_v1_activate_item_teleport_narrow(
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
    static const int16_t dir_dx[4] = { 0, 1, 0, -1 }; /* N, E, S, W */
    static const int16_t dir_dy[4] = { -1, 0, 1, 0 };

    if (!cb)
        return 0;

    for (int dir = 0; dir < 4; dir++) {
        int16_t cand_x = (int16_t)(dst_x + dir_dx[dir]);
        int16_t cand_y = (int16_t)(dst_y + dir_dy[dir]);
        if (cb->is_tile_free(ctx, cand_x, cand_y)) {
            cb->move_record_to(ctx, record, cand_x, cand_y);
            if (out_x)
                *out_x = cand_x;
            if (out_y)
                *out_y = cand_y;
            return 1;
        }
    }
    return 0;
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
    if (amount < 2)
        amount = 2;
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
