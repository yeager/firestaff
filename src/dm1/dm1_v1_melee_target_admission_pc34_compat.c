#include "dm1_v1_melee_target_admission_pc34_compat.h"

#include <string.h>

static uint32_t dm1_v1_melee_target_fnv1a_pc34(const unsigned char *bytes,
                                                size_t byteCount)
{
    uint32_t hash = 2166136261u;
    size_t index;
    if (!bytes || byteCount == 0u) return 0u;
    for (index = 0u; index < byteCount; ++index) {
        hash ^= bytes[index];
        hash *= 16777619u;
    }
    return hash;
}

int dm1_v1_melee_target_admit_f0229_f0230_pc34(
    const DM1_MeleeTargetAdmissionInputPc34 *input,
    DM1_MeleeTargetAdmissionReceiptPc34 *outReceipt)
{
    DM1_MeleeTargetAdmissionReceiptPc34 receipt;
    const unsigned char *raw;
    struct RngState_Compat previewRng;
    unsigned int sourceCell;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB PROJEXPL.C F0229:1284-1305/F0230:1325-1402; "
        "GROUP.C F0207 C38-C41; DEFS.H C04/C29-C41";
    *outReceipt = receipt;
    if (!input || !input->things || !input->things->loaded ||
        !input->group || !input->activeGroup || !input->event || !input->rng ||
        input->groupIndex < 0 || input->groupIndex >= input->things->groupCount ||
        input->things->thingCounts[THING_TYPE_GROUP] != input->things->groupCount ||
        !input->things->groups || !input->things->rawThingData[THING_TYPE_GROUP] ||
        &input->things->groups[input->groupIndex] != input->group ||
        input->creatureIndex < 0 || input->creatureIndex > (int)input->group->count ||
        input->creatureIndex > 3 ||
        input->event->kind != TIMELINE_EVENT_CREATURE_REACTION ||
        input->event->aux0 != input->groupIndex ||
        input->event->aux2 < DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 ||
        input->event->aux2 > DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_3 ||
        input->event->aux2 - DM1_EVENT_UPDATE_BEHAVIOR_CREATURE_0 !=
            input->creatureIndex ||
        input->event->mapIndex != input->partyMapIndex ||
        input->activeGroup->groupThingIndex != input->groupIndex ||
        input->activeGroup->cells != input->group->cells ||
        (input->activeGroup->directions & 3) != (input->group->direction & 3)) {
        return 1;
    }

    raw = input->things->rawThingData[THING_TYPE_GROUP] +
        (size_t)input->groupIndex * 16u;
    if ((unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) !=
            input->group->next ||
        (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8)) !=
            input->group->slot ||
        raw[4] != input->group->creatureType || raw[5] != input->group->cells ||
        (raw[15] & 3u) != (input->group->direction & 3u)) {
        return 1;
    }

    sourceCell = (unsigned int)((input->activeGroup->cells >>
        (input->creatureIndex * 2)) & 3);
    previewRng = *input->rng;
    if (!F0229_DM1_GROUP_SetOrderedCellsToAttack_Compat(
            receipt.orderedCells, input->partyMapX, input->partyMapY,
            input->event->mapX, input->event->mapY, sourceCell,
            &previewRng)) {
        return 1;
    }

    receipt.rawC04FNV1a = dm1_v1_melee_target_fnv1a_pc34(raw, 16u);
    receipt.c29C41FNV1a = dm1_v1_melee_target_fnv1a_pc34(
        (const unsigned char *)input->event, sizeof(*input->event));
    receipt.rngBeforeFNV1a = dm1_v1_melee_target_fnv1a_pc34(
        (const unsigned char *)input->rng, sizeof(*input->rng));
    receipt.rngAfterFNV1a = dm1_v1_melee_target_fnv1a_pc34(
        (const unsigned char *)&previewRng, sizeof(previewRng));
    if (!receipt.rawC04FNV1a || !receipt.c29C41FNV1a ||
        !receipt.rngBeforeFNV1a || !receipt.rngAfterFNV1a) {
        return 1;
    }
    receipt.valid = 1;
    receipt.targetMapIndex = input->partyMapIndex;
    receipt.targetMapX = input->partyMapX;
    receipt.targetMapY = input->partyMapY;
    *outReceipt = receipt;
    return 1;
}
