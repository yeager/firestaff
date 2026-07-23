#include "dm1_v1_group_los_direction_admission_pc34_compat.h"

#include <string.h>

static uint32_t dm1_v1_group_los_fnv1a_pc34(const unsigned char *bytes,
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

int dm1_v1_group_los_direction_admit_f0227_f0228_pc34(
    const DM1_GroupLosDirectionAdmissionInputPc34 *input,
    DM1_GroupLosDirectionAdmissionReceiptPc34 *outReceipt)
{
    DM1_GroupLosDirectionAdmissionReceiptPc34 receipt;
    const unsigned char *raw;
    const struct DungeonGroup_Compat *group;
    struct RngState_Compat previewRng;
    int primary = -1;
    int secondary = -1;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.primaryDirection = -1;
    receipt.secondaryDirection = -1;
    receipt.sourceAnchor =
        "ReDMCSB GROUP.C F0227:13772-13808/F0228:13810-13859; "
        "DEFS.H C04/C29-C41 ACTIVE_GROUP";
    *outReceipt = receipt;
    if (!input || !input->things || !input->things->loaded ||
        !input->activeGroup || !input->event || !input->rng ||
        input->groupIndex < 0 || input->groupIndex >= input->things->groupCount ||
        input->things->thingCounts[THING_TYPE_GROUP] != input->things->groupCount ||
        !input->things->groups || !input->things->rawThingData[THING_TYPE_GROUP] ||
        input->event->kind != TIMELINE_EVENT_CREATURE_REACTION ||
        input->event->aux0 != input->groupIndex ||
        input->event->mapIndex != input->partyMapIndex ||
        input->activeGroup->reserved0 != input->groupIndex ||
        input->activeGroup->groupMapIndex != input->event->mapIndex ||
        input->activeGroup->groupMapX != input->event->mapX ||
        input->activeGroup->groupMapY != input->event->mapY ||
        input->activeGroup->groupCells < 0 || input->activeGroup->groupCells > 0xff ||
        (input->activeGroup->groupDirection & 3) != (input->activeDirections & 3)) {
        return 1;
    }
    group = &input->things->groups[input->groupIndex];
    raw = input->things->rawThingData[THING_TYPE_GROUP] +
        (size_t)input->groupIndex * 16u;
    if ((unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) != group->next ||
        (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8)) != group->slot ||
        raw[4] != group->creatureType || raw[5] != group->cells ||
        (raw[15] & 3u) != (group->direction & 3u) ||
        (group->direction & 3u) != (input->activeDirections & 3u)) {
        return 1;
    }
    previewRng = *input->rng;
    if (!F0228_DM1_GROUP_GetDirectionsWhereDestinationIsVisibleFromSource_Compat(
            input->event->mapX, input->event->mapY,
            input->partyMapX, input->partyMapY, &previewRng,
            &primary, &secondary) || primary < 0 || primary > 3 ||
        secondary < 0 || secondary > 3) {
        return 1;
    }
    receipt.rawC04FNV1a = dm1_v1_group_los_fnv1a_pc34(raw, 16u);
    receipt.c29C41FNV1a = dm1_v1_group_los_fnv1a_pc34(
        (const unsigned char *)input->event, sizeof(*input->event));
    receipt.rngBeforeFNV1a = dm1_v1_group_los_fnv1a_pc34(
        (const unsigned char *)input->rng, sizeof(*input->rng));
    receipt.rngAfterFNV1a = dm1_v1_group_los_fnv1a_pc34(
        (const unsigned char *)&previewRng, sizeof(previewRng));
    if (!receipt.rawC04FNV1a || !receipt.c29C41FNV1a ||
        !receipt.rngBeforeFNV1a || !receipt.rngAfterFNV1a) {
        return 1;
    }
    receipt.valid = 1;
    receipt.primaryDirection = primary;
    receipt.secondaryDirection = secondary;
    *outReceipt = receipt;
    return 1;
}
