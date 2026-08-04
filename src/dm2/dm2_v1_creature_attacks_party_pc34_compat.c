/* DM2 V1 CREATURE_ATTACKS_PARTY — skproject c_creature.cpp:1000-1306.
 * Ported from register-machine code to clean callback-based C. */

#include "dm2_v1_creature_attacks_party_pc34_compat.h"
#include <string.h>

int dm2_v1_creature_attacks_party_full(
    const DM2_V1_CreatureAttacksPartyState *state,
    const DM2_V1_CreatureAttacksPartyCallbacks *cb,
    void *ctx,
    DM2_V1_CreatureAttacksPartyReceipt *receipt)
{
    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!state || !cb) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    /* c_creature.cpp:1020-1028 — extract direction from spx_word_0e */
    int16_t direction = (int16_t)(((uint16_t)(state->spx_word_0e << 6)) >> 14);

    /* c_creature.cpp:1030-1038 — type 6 creatures rotate direction +2 */
    if (state->creature_b_20 == 6) {
        direction = (int16_t)((direction + 2) & 0x3);
    }

    /* c_creature.cpp:1039-1051 — check if creature is on same tile as party */
    int same_tile = 0;
    if (state->creature_x == state->party_x &&
        state->creature_y == state->party_y) {
        same_tile = 1;
    }

    /* c_creature.cpp:1058-1071 — if not same tile, check adjacency */
    if (!same_tile) {
        if (!cb->abs_fn || !cb->calc_vector_dir) {
            receipt->fail_closed = 1;
            return 0;
        }
        int16_t dx = cb->abs_fn((int16_t)(state->creature_x - state->party_x));
        int16_t dy = cb->abs_fn((int16_t)(state->creature_y - state->party_y));
        if (dx + dy > 1) {
            receipt->result = 1;
            return 1;
        }
        int16_t vec_dir = cb->calc_vector_dir(ctx,
            state->creature_x, state->creature_y,
            state->party_x, state->party_y);
        vec_dir &= 0xFFFF;
        if (vec_dir != direction && state->ai_spec &&
            (state->ai_spec[0] & 0x4) == 0) {
            receipt->result = 1;
            return 1;
        }
    }

    /* c_creature.cpp:1072-1089 — check if party is on current map */
    int party_on_map = (state->current_map == state->party_map &&
                        state->party_x >= 0 && state->party_y >= 0);

    if (party_on_map) {
        /* c_creature.cpp:1087-1088 — update last attack tick */
        receipt->last_attack_tick = state->gametick;

        /* c_creature.cpp:1090-1091 — check spec flag 0x10 */
        int16_t spec_flag = state->ai_spec ? (state->ai_spec[0] & 0x10) : 0;

        /* c_creature.cpp:1092-1135 — count alive heroes */
        int16_t alive_count = 0;
        int8_t alive_indices[4];
        memset(alive_indices, -1, sizeof(alive_indices));

        for (int16_t i = 0; i < state->heros_in_party && i < 4; i++) {
            if (state->hero_hp[i] != 0) {
                alive_indices[alive_count] = (int8_t)i;
                alive_count++;
            }
        }

        if (alive_count == 0) {
            receipt->result = 1;
            return 1;
        }

        /* c_creature.cpp:1105-1123 — determine attack count */
        int16_t attack_count;
        if (state->ai_spec && (state->ai_spec[0] & 0x8) != 0) {
            if (state->ai_spec && (state->ai_spec[9] & 0x20) != 0)
                attack_count = 1;
            else
                attack_count = 2;
        } else {
            if (spec_flag == 0)
                attack_count = state->heros_in_party;
            else {
                if (!cb->rand16) {
                    receipt->fail_closed = 1;
                    return 0;
                }
                attack_count = cb->rand16(ctx, alive_count) + 1;
            }
        }

        /* c_creature.cpp:1137-1152 — handle creature type 4 (b_1c update) */
        uint8_t target_pos = state->creature_b_1c;
        if (state->creature_b_1c == 4) {
            if (cb->find_hero_at) {
                int16_t found = cb->find_hero_at(ctx,
                    (uint16_t)state->creature_x,
                    (uint16_t)state->creature_y, 0xFF);
                if (found != -1 && found < 4) {
                    target_pos = state->hero_partypos[found];
                    receipt->creature_b_1c = target_pos;
                }
            }
        }

        /* c_creature.cpp:1153-1156 — clamp attack_count */
        if (cb->min_fn)
            attack_count = cb->min_fn(attack_count, state->heros_in_party);

        /* c_creature.cpp:1157-1249 — attack loop */
        int attacked_any = 0;
        for (int16_t atk = attack_count - 1; atk >= 0; atk--) {
            int16_t hero_idx;

            if (spec_flag == 0) {
                /* c_creature.cpp:1167-1193 — sequential targeting */
                if (cb->get_player_at_position)
                    hero_idx = cb->get_player_at_position(ctx, target_pos);
                else
                    hero_idx = -1;

                /* c_creature.cpp:1172-1174 — compute next position */
                if (cb->calc_vector_dir) {
                    int16_t attack_dir = cb->calc_vector_dir(ctx,
                        state->party_x, state->party_y,
                        state->creature_x, state->creature_y);
                    int16_t step = (int16_t)(target_pos + attack_dir);
                    if ((step & 1) == 0)
                        target_pos++;
                    else
                        target_pos--;
                    target_pos &= 0x3;
                }

                if (hero_idx == -1) {
                    if (cb->find_hero_at) {
                        hero_idx = cb->find_hero_at(ctx,
                            (uint16_t)state->creature_x,
                            (uint16_t)state->creature_y, 0xFF);
                    }
                }
                if (hero_idx == -1)
                    continue;
            } else {
                /* c_creature.cpp:1197-1228 — random targeting */
                if (!cb->rand16) {
                    receipt->fail_closed = 1;
                    return 0;
                }
                int16_t pick = cb->rand16(ctx, alive_count);
                if (alive_indices[pick] < 0) {
                    int16_t scan = 0;
                    for (;;) {
                        if (scan >= alive_count)
                            break;
                        pick++;
                        if (pick >= alive_count)
                            pick = 0;
                        if (alive_indices[pick] >= 0)
                            break;
                        scan++;
                    }
                }
                hero_idx = alive_indices[pick];
                alive_indices[pick] = -1;
            }

            /* c_creature.cpp:1231-1248 — CREATURE_ATTACKS_PLAYER */
            attacked_any = 1;
            if (cb->creature_attacks_player) {
                int16_t damage = cb->creature_attacks_player(ctx,
                    state->spx_word_0e, hero_idx);
                int16_t damage_plus_1 = (int16_t)(damage + 1);
                if (hero_idx >= 0 && hero_idx < 4) {
                    if (damage_plus_1 > (int16_t)receipt->hero_b_29[hero_idx]) {
                        receipt->hero_b_29[hero_idx] = (uint8_t)damage_plus_1;
                        if (cb->calc_vector_dir) {
                            int16_t hit_dir = cb->calc_vector_dir(ctx,
                                state->creature_x, state->creature_y,
                                state->party_x, state->party_y);
                            receipt->hero_b_28[hero_idx] =
                                (uint8_t)((hit_dir + 2) & 0x3);
                        }
                    }
                }
            }
        }

        /* c_creature.cpp:1251-1253 */
        if (!attacked_any) {
            receipt->result = 1;
            return 1;
        }
        receipt->attacked = 1;
    }

    /* c_creature.cpp:1257-1278 — door attack / creature-vs-creature */
    if (!party_on_map) {
        if (cb->get_tile_type) {
            uint8_t tile = cb->get_tile_type(ctx, state->party_x, state->party_y);
            if (tile == 4) {
                /* Door tile — attack the door */
                if (state->ai_spec && cb->rand16 && cb->attack_door) {
                    uint8_t str = state->ai_spec[6];
                    int16_t damage = (int16_t)(str + str / 2);
                    damage = cb->rand16(ctx, damage);
                    int r = cb->attack_door(ctx, state->party_x, state->party_y,
                                            damage, 0, 0);
                    if (r != 0) {
                        receipt->result = 1;
                        return 1;
                    }
                }
            }
            if (same_tile) {
                receipt->result = 1;
                return 1;
            }
            if (cb->creature_attacks_creature) {
                int16_t r = cb->creature_attacks_creature(ctx,
                    state->party_x, state->party_y);
                if (r < 0) {
                    receipt->result = 1;
                    return 1;
                }
            }
        }
    }

    /* c_creature.cpp:1282-1305 — push party if b_1a == 0x26 */
    if (state->creature_b_1a != 0x26) {
        receipt->result = 0;
        return 1;
    }

    if (cb->get_creature_weight) {
        int16_t weight = cb->get_creature_weight(ctx, state->creature_record);
        weight &= 0xFFFF;
        if (weight > 0x64) {
            if (cb->rand_full) {
                int16_t rval = cb->rand_full(ctx);
                if ((rval & 0xF) != 0) {
                    receipt->result = 0;
                    return 1;
                }
            }
        }
    }

    /* c_creature.cpp:1298-1305 — push party in random direction */
    if (cb->randdir) {
        int16_t push_dir = cb->randdir(ctx);
        if (push_dir == 0) {
            receipt->result = 0;
            return 1;
        }
        if (cb->push_party) {
            cb->push_party(ctx, state->party_x, state->party_y,
                           direction, 0xFE);
            receipt->pushed = 1;
            receipt->push_dir = direction;
        }
    }

    receipt->result = 0;
    return 1;
}
