#include "dm1_v1_dungeon_light_admission_f0337_pc34_compat.h"

#include <string.h>

static uint32_t f0337_hash(const unsigned char *bytes)
{
    uint32_t hash = 2166136261u;
    int i;
    for (i = 0; bytes && i < 4; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static unsigned short f0337_u16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

int dm1_v1_dungeon_light_admit_f0337_pc34(
    const struct DungeonThings_Compat *things,
    const unsigned short handThings[DM1_V1_F0337_HAND_COUNT_PC34],
    DM1_V1_DungeonLightReceiptF0337Pc34 *outReceipt)
{
    DM1_V1_DungeonLightReceiptF0337Pc34 receipt;
    int i;
    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0337:329-432; DUNGEON.C F0156 raw C05 weapon records";
    *outReceipt = receipt;
    if (!handThings) return 1;
    receipt.rawFingerprint = 2166136261u;
    for (i = 0; i < DM1_V1_F0337_HAND_COUNT_PC34; ++i) {
        unsigned short thing = handThings[i];
        const unsigned char *raw;
        const struct DungeonWeapon_Compat *weapon;
        unsigned short bits;
        int index;
        int prior;
        if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON || !things ||
            !things->loaded || !things->weapons ||
            !things->rawThingData[THING_TYPE_WEAPON]) return 1;
        index = (int)THING_GET_INDEX(thing);
        if (index < 0 || index >= things->weaponCount ||
            index >= things->thingCounts[THING_TYPE_WEAPON]) return 1;
        raw = things->rawThingData[THING_TYPE_WEAPON] + index * 4;
        weapon = &things->weapons[index];
        bits = f0337_u16(raw + 2);
        if (weapon->next != f0337_u16(raw) ||
            weapon->type != (unsigned char)(bits & 0x7fu) ||
            weapon->chargeCount != (unsigned char)((bits >> 10) & 0x0fu) ||
            weapon->lit != (unsigned char)((bits >> 15) & 1u)) return 1;
        for (prior = 0; prior < i; ++prior) {
            if (handThings[prior] == thing) return 1;
        }
        receipt.rawFingerprint ^= f0337_hash(raw);
        receipt.rawFingerprint *= 16777619u;
        if (weapon->type == 2 && weapon->lit && weapon->chargeCount > 0) {
            receipt.torchLightPower[i] = weapon->chargeCount;
        }
    }
    receipt.valid = receipt.rawFingerprint != 0u;
    *outReceipt = receipt;
    return 1;
}
