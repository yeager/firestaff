/* DM2 V1 CREATURE_ATTACKS_PARTY — skproject c_creature.cpp:1000-1306.
 * Creature melee attack resolution against the party. */

#ifndef DM2_V1_CREATURE_ATTACKS_PARTY_PC34_COMPAT_H
#define DM2_V1_CREATURE_ATTACKS_PARTY_PC34_COMPAT_H

#include <stdint.h>

typedef struct {
    int16_t (*abs_fn)(int16_t v);
    int16_t (*calc_vector_dir)(void *ctx,
                               int16_t x1, int16_t y1,
                               int16_t x2, int16_t y2);
    int16_t (*rand16)(void *ctx, int16_t max);
    int16_t (*randdir)(void *ctx);
    int     (*randbit)(void *ctx);
    int16_t (*rand_full)(void *ctx);
    int16_t (*min_fn)(int16_t a, int16_t b);
    int16_t (*get_player_at_position)(void *ctx, uint8_t pos);
    int16_t (*find_hero_at)(void *ctx, uint16_t x, uint16_t y, int16_t filter);
    int16_t (*creature_attacks_player)(void *ctx, uint16_t spx_record,
                                       int16_t hero_idx);
    uint8_t (*get_tile_type)(void *ctx, int16_t x, int16_t y);
    int     (*attack_door)(void *ctx, int16_t x, int16_t y,
                           int16_t damage, int p1, int p2);
    int16_t (*creature_attacks_creature)(void *ctx, int16_t x, int16_t y);
    int16_t (*get_creature_weight)(void *ctx, uint16_t record);
    void    (*push_party)(void *ctx, int16_t x, int16_t y,
                          int16_t dir, int flags);
} DM2_V1_CreatureAttacksPartyCallbacks;

typedef struct {
    /* Creature state */
    uint16_t spx_word_0e;
    int16_t creature_x;
    int16_t creature_y;
    uint8_t creature_b_20;
    uint8_t creature_b_1a;
    uint8_t creature_b_1c;
    uint16_t creature_record;

    /* AI spec pointer (v1e0552) */
    const uint8_t *ai_spec;

    /* Party state */
    int16_t party_x;
    int16_t party_y;
    int16_t party_map;
    int16_t current_map;
    int16_t heros_in_party;
    int16_t hero_hp[4];
    uint8_t hero_partypos[4];

    /* Gametick */
    int32_t gametick;
} DM2_V1_CreatureAttacksPartyState;

typedef struct {
    int valid;
    int fail_closed;
    int16_t result;
    int32_t last_attack_tick;
    uint8_t creature_b_1c;
    uint8_t hero_b_28[4];
    uint8_t hero_b_29[4];
    int attacked;
    int pushed;
    int16_t push_dir;
} DM2_V1_CreatureAttacksPartyReceipt;

int dm2_v1_creature_attacks_party_full(
    const DM2_V1_CreatureAttacksPartyState *state,
    const DM2_V1_CreatureAttacksPartyCallbacks *cb,
    void *ctx,
    DM2_V1_CreatureAttacksPartyReceipt *receipt);

#endif
