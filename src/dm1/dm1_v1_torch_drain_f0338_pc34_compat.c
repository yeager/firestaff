#include "dm1_v1_torch_drain_f0338_pc34_compat.h"

#include <string.h>

static uint32_t f0338_hash(const unsigned char *bytes, int count)
{
    uint32_t hash = 2166136261u;
    int i;
    for (i = 0; bytes && i < count; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

static unsigned short f0338_read_u16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static int f0338_admit_weapon(const struct DungeonThings_Compat *things,
                              unsigned short thing, uint32_t *ioHash)
{
    const unsigned char *raw;
    const struct DungeonWeapon_Compat *weapon;
    unsigned short bits;
    int index;
    if (!things || !ioHash || THING_GET_TYPE(thing) != THING_TYPE_WEAPON) return 0;
    index = (int)THING_GET_INDEX(thing);
    if (!things->loaded || !things->weapons || !things->rawThingData[THING_TYPE_WEAPON] ||
        index < 0 || index >= things->weaponCount ||
        index >= things->thingCounts[THING_TYPE_WEAPON]) return 0;
    raw = things->rawThingData[THING_TYPE_WEAPON] + index * 4;
    weapon = &things->weapons[index];
    bits = f0338_read_u16(raw + 2);
    if (weapon->next != f0338_read_u16(raw) ||
        weapon->type != (unsigned char)(bits & 0x7fu) ||
        weapon->doNotDiscard != (unsigned char)((bits >> 7) & 1u) ||
        weapon->chargeCount != (unsigned char)((bits >> 10) & 0x0fu) ||
        weapon->lit != (unsigned char)((bits >> 15) & 1u)) return 0;
    *ioHash ^= f0338_hash(raw, 4);
    *ioHash *= 16777619u;
    return *ioHash != 0u;
}

int dm1_v1_torch_drain_f0338_pc34(
    struct DungeonThings_Compat *things,
    const unsigned short handThings[DM1_V1_F0338_HAND_COUNT_PC34],
    int championCount,
    DM1_V1_TorchDrainReceiptF0338Pc34 *outReceipt)
{
    DM1_V1_TorchDrainReceiptF0338Pc34 receipt;
    int handCount;
    int i;
    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    for (i = 0; i < DM1_V1_F0338_HAND_COUNT_PC34; ++i)
        receipt.drainedThings[i] = THING_NONE;
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0338:434-499; GAMELOOP.C:124-126 raw C05 weapon charge";
    *outReceipt = receipt;
    if (!things || !handThings || championCount < 0 || championCount > 4 ||
        !things->loaded) return 1;
    handCount = championCount * 2;
    receipt.rawFingerprint = 2166136261u;
    for (i = 0; i < handCount; ++i) {
        const unsigned short thing = handThings[i];
        const struct DungeonWeapon_Compat *weapon;
        int index;
        int prior;
        if (thing == THING_NONE || thing == THING_ENDOFLIST) continue;
        if (THING_GET_TYPE(thing) != THING_TYPE_WEAPON) continue;
        if (!f0338_admit_weapon(things, thing, &receipt.rawFingerprint)) return 1;
        index = (int)THING_GET_INDEX(thing);
        weapon = &things->weapons[index];
        if (weapon->type != DM1_V1_F0338_TORCH_WEAPON_TYPE_PC34 ||
            !weapon->lit || weapon->chargeCount == 0) continue;
        for (prior = 0; prior < receipt.changedCount; ++prior) {
            if (receipt.drainedThings[prior] == thing) return 1;
        }
        receipt.drainedThings[receipt.changedCount++] = thing;
    }
    for (i = 0; i < receipt.changedCount; ++i) {
        int index = (int)THING_GET_INDEX(receipt.drainedThings[i]);
        struct DungeonWeapon_Compat *weapon = &things->weapons[index];
        unsigned char *raw = things->rawThingData[THING_TYPE_WEAPON] + index * 4;
        unsigned short bits = f0338_read_u16(raw + 2);
        --weapon->chargeCount;
        if (weapon->chargeCount == 0) weapon->doNotDiscard = 0;
        bits = (unsigned short)((bits & ~(unsigned short)(0x0fu << 10)) |
                                ((unsigned short)weapon->chargeCount << 10));
        if (!weapon->doNotDiscard) bits &= (unsigned short)~(1u << 7);
        raw[2] = (unsigned char)(bits & 0xffu);
        raw[3] = (unsigned char)(bits >> 8);
    }
    receipt.valid = receipt.rawFingerprint != 0u;
    *outReceipt = receipt;
    return 1;
}
