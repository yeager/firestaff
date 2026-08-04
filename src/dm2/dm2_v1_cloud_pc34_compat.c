/*
 * dm2_v1_cloud_pc34_compat.c — DM2 cloud/weather system.
 *
 * Ports DM2_075f_0182, DM2_CREATE_CLOUD, DM2_CALC_CLOUD_DAMAGE,
 * and DM2_PROCESS_CLOUD from skproject c_cloud.cpp.
 *
 * Source: skproject/SKULLWIN/c_cloud.cpp
 */

#include "dm2_v1_cloud_pc34_compat.h"

#include <string.h>

/* ── Helper: encode x,y into timer value_a ────────────────────────── */

static int16_t encode_xy(int16_t x, int16_t y)
{
    return (int16_t)((uint8_t)x | ((uint8_t)y << 8));
}

/* ── Helper: extract record type from record word ─────────────────── */

static int16_t record_type(uint16_t rec)
{
    return (int16_t)((rec & 0x3C00) >> 10);
}

static uint16_t record_to_word(uint16_t rec)
{
    return rec;
}

/* ── DM2_075f_0182 — actuator scan on tile for cloud effects ──────── */
/* Source: c_cloud.cpp:21-124 */

int32_t dm2_v1_cloud_actuator_scan(const DM2_V1_CloudCallbacks *cb,
                                   int32_t cloud_spell,
                                   int16_t tile_x, int16_t tile_y,
                                   int16_t intensity)
{
    uint16_t rec = record_to_word(
        cb->get_tile_record_link(cb->ctx, tile_x, tile_y));

    while (rec != 0xFFFE) {
        uint16_t cur = rec;
        /* Extract record type: bits 13..10 */
        int16_t rtype = record_type(cur);

        if (rtype == 0x3) {
            /* Actuator record (type 3) */
            uint8_t *rp = cb->get_address_of_record(cb->ctx, cur);
            uint16_t w2 = (uint16_t)(rp[2] | (rp[3] << 8));
            uint16_t actu_type = w2 & 0x7F;

            if (actu_type == 0x26) {
                int32_t match;
                uint16_t w2_masked = w2 & 0xFF80;

                if (w2_masked == 0xFF80) {
                    /* Universal match */
                    match = 1;
                } else {
                    int32_t spell_offset = cloud_spell - 0xFF80;
                    uint16_t actu_spell = w2 >> 7;
                    if ((int16_t)actu_spell == (int16_t)spell_offset)
                        match = 1;
                    else
                        match = 0;
                }

                /* Check w4 toggle bit (bit 5 of word 4) */
                uint16_t w4 = (uint16_t)(rp[4] | (rp[5] << 8));
                uint16_t toggle = (w4 << 10) >> 15;
                match ^= (int32_t)toggle;

                /* Check w4 bit 2 — intensity gate */
                if (rp[4] & 0x04) {
                    int32_t limit;
                    if ((uint16_t)cloud_spell == 0xFF82)
                        limit = 0x7F;
                    else
                        limit = 0xFF;
                    if ((int32_t)intensity < limit)
                        match = 0;
                }

                /* Check w4 bits 3..4 (AND gate) */
                uint16_t and_bits = w4 & 0x18;
                if (and_bits == 0x18) {
                    /* XOR toggle: invert match */
                    match = (match == 0) ? 1 : 0;
                } else {
                    if (match != 0) {
                        /* Extract action from w4 bits 14..12 */
                        int32_t action = (int32_t)((w4 << 11) >> 14);
                        action = (int32_t)(uint16_t)action;
                        cb->invoke_actuator(cb->ctx, rp, action, 0);
                    }
                    goto next_record;
                }

                cb->invoke_actuator(cb->ctx, rp, match, 0);
            }
        }

    next_record:
        rec = (uint16_t)cb->get_next_record_link(cb->ctx, cur);
    }

    return (int32_t)rec;
}

/* ── DM2_CREATE_CLOUD — create cloud record with timer and damage ─── */
/* Source: c_cloud.cpp:127-444 */

DM2_V1_CreateCloudReceipt dm2_v1_create_cloud(const DM2_V1_CloudCallbacks *cb,
                                               int32_t cloud_spell,
                                               int16_t strength,
                                               int16_t x_encoded,
                                               int16_t y_encoded,
                                               int16_t direction)
{
    DM2_V1_CreateCloudReceipt receipt;
    memset(&receipt, 0, sizeof(receipt));

    int16_t rec_idx = cb->alloc_new_record(cb->ctx, 15);
    if (rec_idx == -1) {
        receipt.created = false;
        receipt.record_index = -1;
        return receipt;
    }

    uint8_t *rp = cb->get_address_of_record(cb->ctx, (uint16_t)rec_idx);

    int16_t tile_x = x_encoded;
    int16_t tile_y = y_encoded;
    int16_t start_x, start_y;

    /* Decode coordinates if encoded (x > 0xFF means x|y packed) */
    if ((uint16_t)x_encoded > 0xFF) {
        start_x = (int16_t)(x_encoded & 0xFF);
        start_y = (int16_t)(y_encoded & 0xFF);
        tile_x = (int16_t)((uint16_t)x_encoded >> 8);
        tile_y = (int16_t)((uint16_t)y_encoded >> 8);
        tile_x -= 1;
    } else {
        start_x = x_encoded;
        start_y = y_encoded;
    }

    /* Store record_index with direction in high bits */
    uint32_t rec_packed = (uint32_t)(uint16_t)rec_idx;
    if ((uint16_t)direction != 0xFF) {
        /* Clear bit 7 of w2 */
        rp[2] &= 0x7F;
        /* Set direction in bits 15..14 of rec_packed */
        rec_packed &= 0x3FFF;
        rec_packed |= ((uint32_t)direction << 14);
    } else {
        /* Set bit 7 of w2 — omnidirectional */
        rp[2] |= 0x80;
    }

    /* Store cloud spell type in w2 low 7 bits */
    int16_t spell_offset = (int16_t)((cloud_spell - 0xFF80) & 0x7F);
    rp[2] &= 0x80;
    rp[2] |= (uint8_t)spell_offset;

    /* Store strength in w2 high byte + w3 */
    uint8_t str_lo = (uint8_t)(strength & 0xFF);
    rp[3] = (uint8_t)((strength >> 8) & 0xFF);
    rp[2] |= (uint8_t)((uint16_t)str_lo << 8 >> 8);
    /* Actually: w2 high byte = str_lo, encoded via shift */
    uint16_t w2 = (uint16_t)(rp[2] | (rp[3] << 8));
    w2 = (w2 & 0x00FF) | ((uint16_t)str_lo << 8);
    rp[2] = (uint8_t)(w2 & 0xFF);
    rp[3] = (uint8_t)(w2 >> 8);

    /* Calculate initial intensity for noise */
    int16_t noise_intensity;
    if (cb->current_map != cb->party_map ||
        tile_x != cb->party_x ||
        tile_y != cb->party_y) {
        /* Not on party tile: intensity = strength/2 + 0x80, capped at 0xFF */
        int16_t val = (int16_t)((uint16_t)strength / 2 + 0x80);
        if (val > 0xFF) val = 0xFF;
        noise_intensity = val;
    } else {
        noise_intensity = 0xFF;
    }

    /* Queue noise */
    cb->queue_noise_gen2(cb->ctx, 0x0D,
                         (uint8_t)(rp[2] & 0x7F),
                         0x81, 0xFE,
                         tile_x, tile_y,
                         1, 0x6C, noise_intensity);

    /* Append record to tile */
    cb->append_record_to(cb->ctx, (uint16_t)(rec_packed & 0xFFFF),
                         tile_x, tile_y);

    /* Calculate timer delay based on cloud spell type */
    int16_t delay;
    if ((uint16_t)cloud_spell == 0xFFE4)
        delay = 5;
    else if ((uint16_t)cloud_spell == 0xFF8E)
        delay = (int16_t)((uint16_t)strength / 2);
    else
        delay = 1;

    /* Queue cloud timer (type 0x19) */
    DM2_V1_CloudTimer t;
    memset(&t, 0, sizeof(t));
    t.ticks_and_map = ((uint32_t)(cb->current_map & 0xFF) << 24)
                    | ((cb->game_tick + (uint32_t)delay) & 0x00FFFFFF);
    t.type = 0x19;
    t.actor = 0;
    t.value_b = (int16_t)(rec_packed & 0xFFFF);
    t.value_a = encode_xy(tile_x, tile_y);
    cb->queue_timer(cb->ctx, &t);

    /* Handle spreading cloud types (0xFF80, 0xFF82, 0xFFB0, 0xFFB1) */
    uint16_t spell16 = (uint16_t)cloud_spell;
    if (spell16 == 0xFF82 || spell16 == 0xFF80 ||
        spell16 == 0xFFB0 || spell16 == 0xFFB1) {

        int16_t spread_x = start_x;
        int16_t spread_y = start_y;
        int16_t spread_damage = (int16_t)((uint16_t)strength / 2 + 1);
        spread_damage += cb->rand16(cb->ctx, spread_damage) + 1;

        bool do_spread = true;
        if (spell16 != 0xFF80 && spell16 != 0xFFB0) {
            if ((uint16_t)spread_damage >> 1 == 0)
                do_spread = false;
        }

        if (do_spread) {
            /* Attack party if on same tile */
            if (cb->current_map == cb->party_map &&
                tile_x == cb->party_x &&
                tile_y == cb->party_y) {
                cb->attack_party(cb->ctx, (int16_t)(uint16_t)spread_damage,
                                 0x3F, 1);
            }

            /* Attack creatures on tile */
            int16_t creature_rec = cb->get_creature_at(cb->ctx, tile_x, tile_y);
            if (creature_rec != -1) {
                int16_t check = cb->query_1c9a_0958(cb->ctx,
                    (uint16_t)creature_rec);
                if (check == 0) {
                    uint8_t *crec = cb->get_address_of_record(cb->ctx,
                        (uint16_t)creature_rec);
                    uint8_t ctype = crec[4];
                    uint8_t *ai = cb->query_creature_ai_spec_from_record(
                        cb->ctx, ctype);

                    if (ai[0x19] & 0x10) {
                        if (spell16 != 0xFF80) {
                            spread_damage >>= 2;
                            if (spread_damage < 1)
                                spread_damage = 1;
                        }
                    }

                    /* Check cloud resistance */
                    int16_t resist = (int16_t)(((uint16_t)(ai[0x18] |
                        (ai[0x19] << 8)) >> 4) & 0xF);
                    if (resist != 0xF) {
                        if (ai[0] & 0x20)
                            spread_damage >>= 2;
                        int16_t rand_reduce = cb->rand16(cb->ctx,
                            2 * resist + 1);
                        spread_damage -= rand_reduce;
                        if (spread_damage > 0) {
                            cb->attack_creature(cb->ctx,
                                (uint16_t)creature_rec,
                                tile_x, tile_y,
                                0x200D, 0x64,
                                spread_damage);
                        }
                    }
                }
            }

            /* Scan for next creature via direction query */
            int16_t next_x = spread_x;
            int16_t next_y = spread_y;
            int16_t next_creature = cb->query_1c9a_03cf(cb->ctx,
                &next_x, &next_y, (uint16_t)direction);
            if (next_creature != -1) {
                if (next_x != start_x || next_y != start_y) {
                    /* Spread to adjacent tile with halved damage */
                    tile_x = next_x;
                    tile_y = next_y;
                    spread_damage >>= 1;

                    /* Attack party on new tile */
                    if (cb->current_map == cb->party_map &&
                        tile_x == cb->party_x &&
                        tile_y == cb->party_y) {
                        cb->attack_party(cb->ctx, spread_damage, 0x3F, 1);
                    }

                    /* Attack creatures on new tile */
                    creature_rec = cb->get_creature_at(cb->ctx, tile_x, tile_y);
                    if (creature_rec != -1) {
                        int16_t check = cb->query_1c9a_0958(cb->ctx,
                            (uint16_t)creature_rec);
                        if (check == 0) {
                            uint8_t *crec = cb->get_address_of_record(cb->ctx,
                                (uint16_t)creature_rec);
                            uint8_t ctype = crec[4];
                            uint8_t *ai = cb->query_creature_ai_spec_from_record(
                                cb->ctx, ctype);

                            if (ai[0x19] & 0x10) {
                                if (spell16 != 0xFF80) {
                                    spread_damage >>= 2;
                                    if (spread_damage < 1)
                                        spread_damage = 1;
                                }
                            }

                            int16_t resist = (int16_t)(((uint16_t)(ai[0x18] |
                                (ai[0x19] << 8)) >> 4) & 0xF);
                            if (resist != 0xF) {
                                if (ai[0] & 0x20)
                                    spread_damage >>= 2;
                                int16_t rand_reduce = cb->rand16(cb->ctx,
                                    2 * resist + 1);
                                spread_damage -= rand_reduce;
                                if (spread_damage > 0) {
                                    cb->attack_creature(cb->ctx,
                                        (uint16_t)creature_rec,
                                        tile_x, tile_y,
                                        0x200D, 0x64,
                                        spread_damage);
                                }
                            }
                        }
                    }
                }
            }
        }

        /* Set viewport dirty if on viewed map */
        if (cb->current_map == cb->view_map && cb->viewport_dirty)
            *cb->viewport_dirty |= 1;

    } else if (spell16 == 0xFF84 || spell16 == 0xFF8D) {
        /* Door-affecting clouds: check if tile is a door */
        uint8_t tv = cb->get_tile_value(cb->ctx, tile_x, tile_y);
        int16_t tile_type = (int16_t)((tv >> 5) & 0x7);
        if (tile_type == 4) {
            /* Door tile */
            int16_t sub_type = (int16_t)(tv & 0x7);
            if (sub_type != 5) {
                uint8_t *tile_rec = cb->get_address_of_tile_record(cb->ctx,
                    tile_x, tile_y);
                if ((tile_rec[2] & 0x40) || (tile_rec[3] & 0x20)) {
                    int32_t invoke_tick = (int32_t)(cb->game_tick + 1);
                    int16_t action = (spell16 == 0xFF84) ? 2 : 0;
                    cb->invoke_message(cb->ctx, tile_x, tile_y, 0,
                                       action, invoke_tick);
                }
            }
        }
    }

    /* Actuator scan on the cloud's origin tile */
    uint16_t w2_final = (uint16_t)(rp[2] | (rp[3] << 8));
    int16_t final_intensity = (int16_t)(w2_final >> 8);

    receipt.created = true;
    receipt.record_index = rec_idx;

    dm2_v1_cloud_actuator_scan(cb, cloud_spell,
                               start_x, start_y, final_intensity);

    return receipt;
}

/* ── DM2_CALC_CLOUD_DAMAGE — calculate cloud damage ──────────────── */
/* Source: c_cloud.cpp:446-584 */

DM2_V1_CalcCloudDamageReceipt dm2_v1_calc_cloud_damage(
    const DM2_V1_CloudCallbacks *cb,
    uint16_t cloud_record,
    int32_t target_record)
{
    DM2_V1_CalcCloudDamageReceipt receipt;
    receipt.damage = 0;

    uint8_t *rp = cb->get_address_of_record(cb->ctx, cloud_record);
    uint16_t w2 = (uint16_t)(rp[2] | (rp[3] << 8));
    uint16_t cloud_type = w2 & 0x7F;

    if (cloud_type >= 8)
        return receipt;

    uint8_t type_flags = dm2_cloud_type_table[cloud_type];
    if (type_flags == 0)
        return receipt;

    int16_t target_rtype;  /* record type of target */
    uint8_t *target_rp = NULL;

    if ((int16_t)target_record == -1) {
        /* Party target: check bit 2 */
        if ((type_flags & 0x04) == 0)
            return receipt;
        target_rtype = -1;
    } else {
        target_rtype = record_type((uint16_t)target_record);

        if (target_rtype == 0) {
            /* Door/floor record: check bit 1 */
            if ((type_flags & 0x02) == 0) {
                return receipt;
            }
            target_rp = cb->get_address_of_record(cb->ctx,
                (uint16_t)target_record);
        } else if (target_rtype == 4) {
            /* Creature record: check bit 3 */
            if ((type_flags & 0x08) == 0)
                return receipt;
        } else {
            return receipt;
        }
    }

    /* Base damage from w2 high byte */
    int16_t damage = (int16_t)(w2 >> 8);

    /* Creature-specific resistance */
    if (target_rtype == 4) {
        uint8_t *ai = cb->query_creature_ai_spec_from_type(cb->ctx,
            (uint16_t)target_record);
        if (ai[0x19] & 0x10) {
            if (cloud_type != 0)
                damage >>= 2;
        }
    }

    /* Random bonus damage (bit 0) */
    if (type_flags & 0x01) {
        int16_t base = (int16_t)(w2 >> 8);
        int16_t half = (int16_t)((uint16_t)base >> 1) + 1;
        int16_t bonus = cb->rand16(cb->ctx, half) + 1;
        damage += bonus;
    }

    /* Target-type-specific adjustments */
    if ((uint16_t)target_rtype < 2) {
        /* Type 0 or 1 */
        if (target_rtype == 1) {
            /* Halve damage */
            damage >>= 1;
        } else if (target_rtype == 0 && target_rp != NULL) {
            /* Check record bit 7 of w2 */
            if (!(target_rp[2] & 0x80))
                damage = 0;
        }
        /* Type 0 with non-null rp: return damage if bit set */
        return (DM2_V1_CalcCloudDamageReceipt){ .damage = damage };
    } else if (target_rtype == 2 || target_rtype == 3) {
        /* Check creature AI spec flags bit 5 */
        int16_t flags = cb->query_creature_ai_spec_flags(cb->ctx,
            (uint16_t)target_record);
        receipt.damage = (flags & 0x20) ? damage : 0;
        return receipt;
    } else if (target_rtype == 7) {
        /* Poison cloud: special damage calc */
        int16_t base = (int16_t)(w2 >> 8);
        int16_t val = (int16_t)((uint16_t)base >> 5);
        int16_t capped = cb->min16(val, 4);
        int16_t bonus = (cb->randbit(cb->ctx) ? 1 : 0) + capped;
        int16_t final_val = cb->max16(1, bonus);
        if (target_rtype == 4) {
            receipt.damage = cb->apply_creature_poison_resistance(cb->ctx,
                (uint16_t)target_record, (uint16_t)final_val);
        } else {
            receipt.damage = final_val;
        }
        return receipt;
    }

    receipt.damage = damage;
    return receipt;
}

/* ── DM2_PROCESS_CLOUD — timer-driven cloud processing ───────────── */
/* Source: c_cloud.cpp:587-770 */

DM2_V1_ProcessCloudReceipt dm2_v1_process_cloud(const DM2_V1_CloudCallbacks *cb,
                                                 DM2_V1_CloudTimer *timer)
{
    DM2_V1_ProcessCloudReceipt receipt;
    receipt.requeued = false;
    receipt.deallocated = false;

    int16_t tile_x = (int16_t)((uint8_t)(timer->value_a & 0xFF));
    int16_t tile_y = (int16_t)((uint8_t)((timer->value_a >> 8) & 0xFF));
    uint16_t cloud_rec = (uint16_t)timer->value_b;

    uint8_t *rp = cb->get_address_of_record(cb->ctx, cloud_rec);

    /* Check if tile is a door (type 4) and apply door damage */
    uint8_t tv = cb->get_tile_value(cb->ctx, tile_x, tile_y);
    int16_t tile_type = (int16_t)((tv >> 5) & 0x7);

    if (tile_type == 4) {
        uint16_t tile_rec = cb->get_tile_record_link(cb->ctx, tile_x, tile_y);
        int16_t door_damage = dm2_v1_calc_cloud_damage(cb, cloud_rec,
            (int32_t)tile_rec).damage;
        if (door_damage != 0) {
            cb->attack_door(cb->ctx, tile_x, tile_y, door_damage, 1, 0);
        }
    }

    /* Check cloud type for further processing */
    uint16_t w2 = (uint16_t)(rp[2] | (rp[3] << 8));
    uint16_t cloud_type = w2 & 0x7F;

    /* Cloud types 0x0E, 0, and 2 skip damage/decay — go straight to dealloc */
    if (cloud_type == 0x0E || cloud_type == 0 || cloud_type == 2)
        goto dealloc;

    /* Attack party if on same tile */
    if (cb->current_map == cb->party_map &&
        tile_x == cb->party_x &&
        tile_y == cb->party_y) {
        int16_t party_damage = dm2_v1_calc_cloud_damage(cb, cloud_rec,
            0xFFFF).damage;
        if (party_damage != 0)
            cb->attack_party(cb->ctx, party_damage, 0, 0);
    }

    /* Attack creatures on tile */
    int16_t creature_rec = cb->get_creature_at(cb->ctx, tile_x, tile_y);
    if (creature_rec != -1) {
        int16_t check = cb->query_1c9a_0958(cb->ctx, (uint16_t)creature_rec);
        if (check == 0) {
            int16_t creature_damage = dm2_v1_calc_cloud_damage(cb, cloud_rec,
                (int32_t)creature_rec).damage;
            if (creature_damage != 0) {
                cb->attack_creature(cb->ctx, (uint16_t)creature_rec,
                    tile_x, tile_y,
                    0x200D, 0x64,
                    creature_damage);
            }
        }
    }

    /* Decay logic based on cloud type */
    bool requeue = false;

    if (cloud_type < 0x28) {
        if (cloud_type == 0x07) {
            /* Poison cloud: decay by 3 if strength >= 6 */
            uint16_t str_byte = w2 >> 8;
            if (str_byte >= 6) {
                uint8_t new_str = (uint8_t)(str_byte - 3);
                w2 = (w2 & 0x00FF) | ((uint16_t)new_str << 8);
                rp[2] = (uint8_t)(w2 & 0xFF);
                rp[3] = (uint8_t)(w2 >> 8);
                requeue = true;
            }
        }
        /* else: no decay, cloud expires */
    } else if (cloud_type == 0x28) {
        /* Type 0x28: decay by 0x28 if strength > 0x37 */
        uint16_t str_byte = w2 >> 8;
        if (str_byte > 0x37) {
            uint8_t new_str = (uint8_t)(str_byte - 0x28);
            w2 = (w2 & 0x00FF) | ((uint16_t)new_str << 8);
            rp[2] = (uint8_t)(w2 & 0xFF);
            rp[3] = (uint8_t)(w2 >> 8);
            requeue = true;
        }
    } else if (cloud_type == 0x64) {
        /* Type 0x64: increment type, play noise, requeue */
        cb->queue_noise_gen2(cb->ctx, 0x0D,
                             (uint8_t)cloud_type,
                             0x81, 0xFE,
                             tile_x, tile_y,
                             1, 0x6C, 0xC8);
        uint16_t new_type = (cloud_type + 1) & 0x7F;
        w2 = (w2 & 0xFF80) | new_type;
        rp[2] = (uint8_t)(w2 & 0xFF);
        rp[3] = (uint8_t)(w2 >> 8);
        requeue = true;
    }

    if (requeue) {
        /* Re-queue timer with incremented tick */
        DM2_V1_CloudTimer new_timer = *timer;
        /* Increment game tick in ticks_and_map */
        uint32_t tick = new_timer.ticks_and_map & 0x00FFFFFF;
        uint32_t map = new_timer.ticks_and_map & 0xFF000000;
        new_timer.ticks_and_map = map | ((tick + 1) & 0x00FFFFFF);
        cb->queue_timer(cb->ctx, &new_timer);
        receipt.requeued = true;
        return receipt;
    }

dealloc:
    /* Check viewport dirty flag */
    if (cb->current_map == cb->view_map) {
        uint16_t ct = w2 & 0x7F;
        if (ct == 2 || (w2 & 0x7F) != 0 || ct == 0x30) {
            if (cb->viewport_dirty)
                *cb->viewport_dirty |= 1;
        }
    }

    /* Remove and deallocate cloud record */
    cb->cut_record_from(cb->ctx, cloud_rec, tile_x, tile_y);
    cb->dealloc_record(cb->ctx, cloud_rec);
    receipt.deallocated = true;

    return receipt;
}
