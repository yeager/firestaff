/* DM2 V1 creature operations — skproject c_creature.cpp. */

#include "dm2_v1_creature_ops_pc34_compat.h"
#include <stddef.h>

#define OBJECT_END_WORD 0xFFFEu

int16_t dm2_v1_apply_creature_poison_resistance(
    uint16_t creature_type, int16_t poison_amount,
    const DM2_V1_CreaturePoisonCallbacks *cb, void *ctx)
{
    if (!cb || poison_amount == 0)
        return 0;
    const uint8_t *ai_spec = cb->query_ai_spec(ctx, creature_type);
    if (!ai_spec)
        return 0;
    /* w24 (resistance field), bits 8..11 = poison resistance 0..15 */
    uint16_t resistance_word = (uint16_t)(ai_spec[24] | (ai_spec[25] << 8));
    uint16_t poison_resist = (resistance_word >> 8) & 0xF;
    if (poison_resist == 0xF)
        return 0;
    uint16_t rand_val = cb->rand_dir(ctx);
    uint32_t numerator = ((uint32_t)poison_amount + rand_val) << 3;
    uint32_t denominator = poison_resist + 2;
    return (int16_t)(numerator / denominator);
}

void dm2_v1_rotate_creature(
    uint16_t creature_record, int mode, int direction,
    const DM2_V1_CreatureRotateCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    uint8_t *rec = cb->get_record_address(ctx, creature_record);
    if (!rec)
        return;
    /* Current facing from offset+0xE bits 8-9 */
    uint16_t word_0e = (uint16_t)(rec[0x0E] | (rec[0x0F] << 8));
    uint8_t cur_dir = (uint8_t)((word_0e >> 8) & 0x3);
    uint8_t new_dir;
    int delta;
    if (mode == 0) {
        new_dir = (uint8_t)((cur_dir + direction) & 0x3);
        delta = (new_dir - cur_dir) & 0x3;
    } else {
        new_dir = (uint8_t)(direction & 0x3);
        delta = (new_dir - cur_dir) & 0x3;
    }
    /* Write new direction: clear bits 8-9, set new */
    rec[0x0F] = (uint8_t)((rec[0x0F] & 0xFC) | (new_dir & 0x3));

    /* If AI flag bit 0 set, rotate all items in possession chain */
    uint16_t flags = cb->query_ai_spec_flags(ctx, creature_record);
    if ((flags & 0x01) == 0)
        return;
    /* Walk possession chain at offset+2, rotating each item's direction
     * bits by delta (skproject c_creature.cpp:86-100) */
    uint8_t *chain_ptr = rec + 2;
    for (;;) {
        uint16_t item_word = (uint16_t)(chain_ptr[0] | (chain_ptr[1] << 8));
        if (item_word == OBJECT_END_WORD || item_word == 0xFFFF)
            break;
        uint8_t item_dir = (uint8_t)((item_word >> 14) & 0x3);
        uint8_t rotated = (uint8_t)((item_dir + delta) & 0x3);
        item_word = (item_word & 0x3FFF) | ((uint16_t)rotated << 14);
        chain_ptr[0] = (uint8_t)(item_word & 0xFF);
        chain_ptr[1] = (uint8_t)((item_word >> 8) & 0xFF);
        uint8_t *item_rec = cb->get_record_address(ctx, item_word);
        if (!item_rec)
            break;
        chain_ptr = item_rec;
    }
}

int16_t dm2_v1_creature_can_handle_item_in(
    int16_t creature_type, int16_t first_record, uint8_t direction_filter,
    const DM2_V1_CreatureHandleCallbacks *cb, void *ctx)
{
    if (!cb)
        return (int16_t)OBJECT_END_WORD;
    int16_t current = first_record;
    for (;;) {
        uint16_t w = (uint16_t)current;
        if (w == OBJECT_END_WORD || w == 0xFFFF)
            return (int16_t)OBJECT_END_WORD;
        uint16_t db_type = (w >> 10) & 0xF;
        int is_item = (db_type > 4 && db_type < 14) || (db_type == 9);
        if (is_item) {
            int dir_match = 0;
            if (direction_filter == 0xFF)
                dir_match = 1;
            else if (((w >> 14) & 0x3) == direction_filter)
                dir_match = 1;
            if (dir_match) {
                if (cb->creature_can_handle_it(ctx, w, creature_type))
                    return current;
            }
        }
        current = cb->get_next_record_link(ctx, w);
    }
}

int dm2_v1_confuse_creature(
    int16_t power, int16_t x, int16_t y, int32_t damage,
    const DM2_V1_ConfuseCreatureCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint16_t target = cb->confuser_record;
    if (target == 0xFFFF)
        return 0;
    uint8_t *rec = cb->get_record_address(ctx, target);
    if (!rec)
        return 0;
    uint8_t ctype = cb->get_creature_type(ctx, target);
    uint16_t resist = cb->query_ai_spec_resistance(ctx, ctype);
    uint16_t confuse_resist = (resist >> 4) & 0xF;
    int16_t roll = cb->rand16(ctx, power);
    if (confuse_resist > (uint16_t)roll)
        return 0;
    if (confuse_resist == 0xF)
        return 0;
    cb->attack_creature(ctx, target, x, y, 0x2005, 0x64, damage);
    return 1;
}

void dm2_v1_creature_kill_on_timer_position(
    uint16_t x, uint16_t y,
    const DM2_V1_CreatureKillCallbacks *cb, void *ctx)
{
    if (!cb)
        return;
    cb->delete_creature_record(ctx, x, y, 0, 1);
    if (cb->v1e0570_flag)
        *cb->v1e0570_flag = 1;
}

uint8_t dm2_v1_creature_ccm06(
    const DM2_V1_CreatureCCM06Callbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    uint8_t facing = cb->creature_facing;
    uint8_t delta = (cb->target_dir - facing) & 0x3;
    if (delta == 2) {
        int r = cb->rand_fn(ctx);
        facing = (uint8_t)((facing + (r & 0x2) + 1) & 0x3);
    }
    return (uint8_t)(facing & 0x3);
}

int dm2_v1_creature_create_cloud(
    uint16_t x, uint16_t y,
    const DM2_V1_CreateCloudCallbacks *cb, void *ctx)
{
    if (!cb)
        return 0;
    int16_t strength = (int16_t)(cb->rand16(ctx, 40) + 20);
    return cb->create_cloud(ctx, (int)0xFF8E, (uint16_t)strength, x, y, 0xFF);
}

int dm2_v1_creature_shoot_item(
    const DM2_V1_CreatureShootState *state,
    const DM2_V1_CreatureShootCallbacks *cb, void *ctx)
{
    if (!cb || !state)
        return 1;
    int16_t item = cb->find_shootable_item(ctx, state->capability,
                                            state->inv_link, 0xFF);
    if (item == (int16_t)0xFFFE)
        return 1;
    uint16_t item_w = (uint16_t)item;
    uint16_t inv = state->inv_link;
    cb->cut_record(ctx, item_w, &inv, -1, 0);
    int16_t base_dmg = (int16_t)(state->attack_stat / 4 + 1);
    base_dmg = (int16_t)(base_dmg + cb->rand16(ctx, base_dmg));
    base_dmg = (int16_t)(base_dmg + cb->rand16(ctx, base_dmg));
    uint8_t damage = (uint8_t)cb->between_value(20, 255, base_dmg);
    cb->shoot_item(ctx, item_w, state->x, state->y,
                   state->creature_dir, state->creature_facing,
                   damage, state->speed_stat, 0x08);
    return 0;
}

uint8_t dm2_v1_attack_party(
    int16_t total_damage, int body_parts, int wound_type,
    const DM2_V1_AttackPartyCallbacks *cb, void *ctx)
{
    if (!cb || total_damage == 0)
        return 0;
    uint8_t wounded_mask = 0;
    int16_t range = (int16_t)(total_damage / 8 + 1);
    int16_t base = (int16_t)(total_damage - range);
    int16_t rand_range = (int16_t)(2 * range);
    for (int i = 0; i < cb->hero_count; i++) {
        int16_t dmg = (int16_t)(cb->rand16(ctx, rand_range) + base);
        int16_t clamped = dmg > 1 ? dmg : 1;
        cb->wound_player(ctx, i, clamped, body_parts, wound_type);
        if (clamped > 0)
            wounded_mask |= (uint8_t)(1 << i);
    }
    return wounded_mask;
}
