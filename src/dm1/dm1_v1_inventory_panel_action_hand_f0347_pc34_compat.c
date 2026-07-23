#include "dm1_v1_inventory_panel_action_hand_f0347_pc34_compat.h"

#include <string.h>

static unsigned short f0347_u16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static uint32_t f0347_hash(const unsigned char *bytes)
{
    uint32_t hash = 2166136261u;
    int i;
    for (i = 0; bytes && i < 4; ++i) {
        hash ^= bytes[i];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_inventory_panel_action_hand_admit_f0347_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short actionHandThing,
    DM1_V1_InventoryPanelActionHandReceiptF0347Pc34 *outReceipt)
{
    DM1_V1_InventoryPanelActionHandReceiptF0347Pc34 receipt;
    const unsigned char *raw;
    const struct DungeonWeapon_Compat *weapon;
    unsigned short bits;
    int index;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.actionHandThing = actionHandThing;
    receipt.sourceAnchor =
        "ReDMCSB PANEL.C F0347:1639-1693; F0342 C05 action-hand route; "
        "DUNGEON.C F0156 raw C05 weapon record";
    *outReceipt = receipt;
    if (!things || !things->loaded || !things->weapons ||
        !things->rawThingData[THING_TYPE_WEAPON] ||
        actionHandThing == THING_NONE || actionHandThing == THING_ENDOFLIST ||
        THING_GET_TYPE(actionHandThing) != THING_TYPE_WEAPON) return 1;

    index = (int)THING_GET_INDEX(actionHandThing);
    if (index < 0 || index >= things->weaponCount ||
        index >= things->thingCounts[THING_TYPE_WEAPON]) return 1;
    raw = things->rawThingData[THING_TYPE_WEAPON] + (size_t)index * 4u;
    weapon = &things->weapons[index];
    bits = f0347_u16(raw + 2);
    if (weapon->next != f0347_u16(raw) ||
        weapon->type != (unsigned char)(bits & 0x7fu) ||
        weapon->doNotDiscard != (unsigned char)((bits >> 7) & 1u) ||
        weapon->cursed != (unsigned char)((bits >> 8) & 1u) ||
        weapon->poisoned != (unsigned char)((bits >> 9) & 1u) ||
        weapon->chargeCount != (unsigned char)((bits >> 10) & 0x0fu) ||
        weapon->broken != (unsigned char)((bits >> 14) & 1u) ||
        weapon->lit != (unsigned char)((bits >> 15) & 1u)) return 1;

    receipt.weaponType = weapon->type;
    receipt.rawFingerprint = f0347_hash(raw);
    receipt.valid = receipt.rawFingerprint != 0u;
    *outReceipt = receipt;
    return 1;
}
