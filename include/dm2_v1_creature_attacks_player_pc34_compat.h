/* DM2 V1 CREATURE_ATTACKS_PLAYER — skproject c_creature.cpp:651-903.
 * Single creature-vs-hero attack resolution. */

#ifndef DM2_V1_CREATURE_ATTACKS_PLAYER_PC34_COMPAT_H
#define DM2_V1_CREATURE_ATTACKS_PLAYER_PC34_COMPAT_H

#include <stdint.h>

typedef struct {
    int16_t (*rand_fn)(void *ctx);
    int16_t (*rand16)(void *ctx, int16_t max);
    int16_t (*randdir)(void *ctx);
    int     (*randbit)(void *ctx);
    int16_t (*min_fn)(int16_t a, int16_t b);
    const uint8_t *(*query_ai_spec)(void *ctx, uint8_t creature_type);
    int16_t (*use_dexterity_attribute)(void *ctx, int16_t hero_idx);
    int     (*use_luck)(void *ctx, int16_t hero_idx, int16_t threshold);
    int16_t (*query_player_skill_lv)(void *ctx, int16_t hero_idx,
                                     int16_t skill, int16_t mode);
    int16_t (*wound_player)(void *ctx, int16_t hero_idx, int16_t damage,
                            int16_t wound_type, int16_t attack_type);
    void    (*queue_noise_gen2)(void *ctx, int16_t hero_idx, uint8_t herotype,
                                int16_t party_x, int16_t party_y);
    void    (*process_poison)(void *ctx, int16_t hero_idx, int16_t amount);
    void    (*resume_from_wake)(void *ctx);
    int16_t (*get_adj_ability2)(void *ctx, int16_t hero_idx,
                                int16_t ability, int16_t amount);
} DM2_V1_CreatureAttacksPlayerCallbacks;

typedef struct {
    int16_t hero_idx;
    int16_t heros_in_party;
    int16_t hero_cur_hp;
    uint8_t hero_type;
    const uint8_t *creature_record;
    const uint8_t *creature_data;
    int16_t party_x;
    int16_t party_y;
    int16_t v1e0238;
    int16_t v1e0286;
    uint8_t savegames1_b02;
} DM2_V1_CreatureAttacksPlayerState;

typedef struct {
    int valid;
    int fail_closed;
    int16_t damage_dealt;
    int hit;
    int dodged;
    int poisoned;
    int woke_up;
} DM2_V1_CreatureAttacksPlayerReceipt;

/* table1d26f8: wound type lookup (3 entries) */
extern const uint8_t dm2_v1_table1d26f8[3];

int32_t dm2_v1_creature_attacks_player(
    const DM2_V1_CreatureAttacksPlayerState *state,
    const DM2_V1_CreatureAttacksPlayerCallbacks *cb,
    void *ctx,
    DM2_V1_CreatureAttacksPlayerReceipt *receipt);

#endif
