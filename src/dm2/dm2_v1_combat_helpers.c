#include "dm2_v1_combat_helpers.h"
#include <string.h>

int dm2_v1_stun_champion_compute(
    const DM2_V1_HandCooldownPair *current,
    uint16_t stun_value,
    DM2_V1_StunReceipt *out)
{
    if (!out) return 0;
    memset(out, 0, sizeof(*out));

    if (!current) return 0;

    out->valid = 1;

    uint16_t left = current->left_cooldown + stun_value;
    uint16_t right = current->right_cooldown + stun_value;
    int clamped = 0;

    if (left > DM2_V1_HAND_COOLDOWN_MAX) {
        left = DM2_V1_HAND_COOLDOWN_MAX;
        clamped = 1;
    }
    if (right > DM2_V1_HAND_COOLDOWN_MAX) {
        right = DM2_V1_HAND_COOLDOWN_MAX;
        clamped = 1;
    }

    out->result.left_cooldown = left;
    out->result.right_cooldown = right;
    out->clamped = clamped;
    return 1;
}

uint8_t dm2_v1_get_champion_bones_item_id(int dm1_mode)
{
    return dm1_mode ? 5 : 0;
}

const char *dm2_v1_combat_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore2.cpp STUN_CHAMPION:435 "
           "GET_CHAMPION_BONES_ITEM_ID:583; "
           "bounded combat computation helpers.";
}
