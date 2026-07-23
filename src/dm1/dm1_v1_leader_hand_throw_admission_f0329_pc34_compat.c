#include "dm1_v1_leader_hand_throw_admission_f0329_pc34_compat.h"

#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <string.h>

static int dm1_v1_f0329_raw_thing_size_pc34(int type)
{
    switch (type) {
    case THING_TYPE_WEAPON:
    case THING_TYPE_ARMOUR:
    case THING_TYPE_SCROLL:
    case THING_TYPE_POTION:
    case THING_TYPE_JUNK:
        return 4;
    case THING_TYPE_CONTAINER:
        return 8;
    default:
        return 0;
    }
}

static uint32_t dm1_v1_f0329_fnv1a_pc34(const unsigned char *bytes,
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

int dm1_v1_leader_hand_throw_admit_f0329_pc34(
    const DM1_LeaderHandThrowAdmissionInputF0329Pc34 *input,
    DM1_LeaderHandThrowAdmissionReceiptF0329Pc34 *outReceipt)
{
    DM1_LeaderHandThrowAdmissionReceiptF0329Pc34 receipt;
    const struct ChampionState_Compat *leader;
    const unsigned char *rawLeaderHand;
    int type;
    int index;
    int rawSize;

    if (!outReceipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.sourceAnchor =
        "ReDMCSB CHAMPION.C F0329:2196-2208 -> F0328; "
        "DUNGEON.C F0156 raw leader-hand Thing";
    *outReceipt = receipt;
    if (!input || !input->things || !input->things->loaded || !input->party ||
        input->leaderIndex < 0 ||
        input->leaderIndex >= input->party->championCount ||
        input->leaderIndex >= CHAMPION_MAX_PARTY ||
        input->leaderIndex != input->party->activeChampionIndex ||
        input->throwSide < 0 || input->throwSide > 1 ||
        input->leaderHandThing == THING_NONE ||
        input->leaderHandThing == THING_ENDOFLIST) {
        return 1;
    }

    leader = &input->party->champions[input->leaderIndex];
    if (!leader->present) return 1;
    type = (int)THING_GET_TYPE(input->leaderHandThing);
    index = (int)THING_GET_INDEX(input->leaderHandThing);
    rawSize = dm1_v1_f0329_raw_thing_size_pc34(type);
    rawLeaderHand = dm1_v1_dungeon_get_thing_data_pc34(
        input->things, input->leaderHandThing);
    if (!rawLeaderHand || rawSize == 0 || type < 0 ||
        type >= DUNGEON_THING_TYPE_COUNT || index < 0 ||
        index >= input->things->thingCounts[type]) {
        return 1;
    }

    receipt.rawLeaderHandFNV1a = dm1_v1_f0329_fnv1a_pc34(
        rawLeaderHand, (size_t)rawSize);
    receipt.partyFNV1a = dm1_v1_f0329_fnv1a_pc34(
        (const unsigned char *)input->party, sizeof(*input->party));
    if (!receipt.rawLeaderHandFNV1a || !receipt.partyFNV1a) return 1;
    receipt.valid = 1;
    receipt.leaderIndex = input->leaderIndex;
    receipt.throwSide = input->throwSide;
    receipt.leaderHandThing = input->leaderHandThing;
    receipt.savedActionHandThing =
        leader->inventory[CHAMPION_SLOT_ACTION_HAND];
    *outReceipt = receipt;
    return 1;
}
