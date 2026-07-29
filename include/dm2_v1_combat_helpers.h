#ifndef FIRESTAFF_DM2_V1_COMBAT_HELPERS_H
#define FIRESTAFF_DM2_V1_COMBAT_HELPERS_H

/*
 * DM2 Combat Helpers — bounded pure-computation functions for combat.
 *
 * Source: skproject SKWIN/SkWinCore2.cpp:435 STUN_CHAMPION
 *         skproject SKWIN/SkWinCore2.cpp:583 GET_CHAMPION_BONES_ITEM_ID
 *         skproject SKULLWIN/c_hero.cpp:232  DM2_CALC_PLAYER_ATTACK_DAMAGE (partial)
 */

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_HAND_COOLDOWN_MAX 255

typedef struct {
    uint16_t left_cooldown;
    uint16_t right_cooldown;
} DM2_V1_HandCooldownPair;

typedef struct {
    int valid;
    DM2_V1_HandCooldownPair result;
    int clamped;
} DM2_V1_StunReceipt;

int dm2_v1_stun_champion_compute(
    const DM2_V1_HandCooldownPair *current,
    uint16_t stun_value,
    DM2_V1_StunReceipt *out);

uint8_t dm2_v1_get_champion_bones_item_id(int dm1_mode);

const char *dm2_v1_combat_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif
