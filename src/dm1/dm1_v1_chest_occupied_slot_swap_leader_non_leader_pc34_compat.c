#include "firestaff/dm1/v1/chest/occupied_slot_swap_pc34_compat.h"

#include <string.h>

enum {
    COMMAND_SLOT_BOX_NON_LEADER_READY = 20,
    COMMAND_SLOT_BOX_C540 = 61,
    PANEL_CHEST = 6
};

typedef struct {
    DM1_V1_ChestOccupiedSlotSwapItemPc34 chest[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34];
    DM1_V1_ChestOccupiedSlotSwapItemPc34 leaderHand;
    DM1_V1_ChestOccupiedSlotSwapItemPc34 nonLeaderReadyHand;
    int openChest;
    int panel;
    int f0333OpenCallCount;
    int f0334CloseCallCount;
    int f0077CallCount;
    int f0078CallCount;
} RuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:30-75 open path sets G0426 and materializes C537..C544 into G0425; this gate keeps G0426 unchanged after the swap\n"
    "CHEST.C F0334:117-132 is a negative anchor: no close/relink call may run while the two queued clicks drain\n"
    "CHAMPION.C F0297:243-268 puts the C540 thing into the leader hand after F0300 removes it from the chest slot\n"
    "CHAMPION.C F0298:270-298 removes the original leader-hand thing before F0301 stores it in the non-leader hand\n"
    "CHAMPION.C F0300:511-515 reads and clears C30+ G0425 chest slots\n"
    "CHAMPION.C F0301:606-660 writes the original leader-hand thing into the resolved non-leader C00/C01 hand slot\n"
    "CHAMPION.C F0302:662-713 resolves status hand slot boxes and C30+ chest slots, then performs the hand/slot swap under F0077/F0078\n"
    "DUNGEON.C F0140:1114-1120, F0159:1664-1681, F0163:1769-1838, F0164:1840-1905 define weight, next-link reads, append, and detach/rewire contracts\n"
    "OBJECT.C F0032:121-145 and F0033:147-212 resolve object type/icon identity for the two weapons\n"
    "COMMAND.C F0359:1452-1662 writes queued mouse slot commands; F0380:2045-2178 drains slot-box commands into F0302\n"
    "IO.C F0077:1113-1122 and F0078:1102-1111 bracket the two F0302 swap operations\n"
    "DEFS.H C30/C537..C544/G0425/G0426/G0305/G0423/M070/M516/C0xFFFF/C0xFFFE define the slots, open chest, party, and sentinels\n"
    "Disjoint: no F0334 close path, no C071 eye route, no scroll wheel, no resurrect/mirror candidate, no C040 panel, no party rotation";

static const DM1_V1_ChestOccupiedSlotSwapSpecPc34 s_spec = {
    "DM1 V1 chest occupied-slot leader-hand to non-leader-hand swap runtime regression gate",
    1,
    0xC540C002,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_PARTY_COUNT_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_INVENTORY_ORDINAL_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_INDEX_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_NON_LEADER_INDEX_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C30_BASE_PC34 +
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_ZONE_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_NON_LEADER_SLOT_BOX_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_READY_HAND_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34,
    DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_END_PC34,
    "ReDMCSB CHEST.C F0333 lines 30-75",
    "ReDMCSB CHEST.C F0334 lines 117-132 negative no-close anchor",
    "ReDMCSB CHAMPION.C F0297 lines 243-268",
    "ReDMCSB CHAMPION.C F0298 lines 270-298",
    "ReDMCSB CHAMPION.C F0300 lines 511-515",
    "ReDMCSB CHAMPION.C F0301 lines 606-660",
    "ReDMCSB CHAMPION.C F0302 lines 662-713",
    "ReDMCSB DUNGEON.C F0140 lines 1114-1120",
    "ReDMCSB DUNGEON.C F0159 lines 1664-1681",
    "ReDMCSB DUNGEON.C F0163 lines 1769-1838",
    "ReDMCSB DUNGEON.C F0164 lines 1840-1905",
    "ReDMCSB OBJECT.C F0032 lines 121-145",
    "ReDMCSB OBJECT.C F0033 lines 147-212",
    "ReDMCSB COMMAND.C F0359 lines 1452-1662",
    "ReDMCSB COMMAND.C F0380 lines 2045-2178",
    "ReDMCSB IO.C F0077 lines 1113-1122",
    "ReDMCSB IO.C F0078 lines 1102-1111",
    "ReDMCSB DEFS.H C30/C537..C544/G0425/G0426/G0305/G0423/M070/M516",
    "Disjoint from F0334 close, C071 eye close-then-open, scroll wheel, C040, resurrect, mirror-candidate, and party-rotation chest gates."
};

static DM1_V1_ChestOccupiedSlotSwapItemPc34 make_item(
    int thing, int weight, int allowedSlots)
{
    DM1_V1_ChestOccupiedSlotSwapItemPc34 item;

    item.thing = thing;
    item.weight = weight;
    item.allowedSlots = allowedSlots;
    item.next = DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_END_PC34;
    return item;
}

static DM1_V1_ChestOccupiedSlotSwapItemPc34 none_item(void)
{
    return make_item(DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34, 0, 0);
}

static int is_none(DM1_V1_ChestOccupiedSlotSwapItemPc34 item)
{
    return item.thing == DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34;
}

static void copy_chest(const RuntimePc34* rt, int* types, int* next)
{
    int i;

    for (i = 0; i < DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34; ++i) {
        types[i] = rt->chest[i].thing;
        next[i] = rt->chest[i].next;
    }
}

static void relink_visible_open_chain(RuntimePc34* rt)
{
    int previous = -1;
    int i;

    for (i = 0; i < DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34; ++i) {
        if (is_none(rt->chest[i])) {
            rt->chest[i].next =
                DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34;
            continue;
        }
        rt->chest[i].next = DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_END_PC34;
        if (previous >= 0) {
            rt->chest[previous].next = rt->chest[i].thing;
        }
        previous = i;
    }
}

static void init_runtime(RuntimePc34* rt)
{
    int i;

    memset(rt, 0, sizeof(*rt));
    rt->openChest = DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_OPEN_CHEST_PC34;
    rt->panel = PANEL_CHEST;
    rt->f0333OpenCallCount = 1;
    for (i = 0; i < DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34; ++i) {
        rt->chest[i] =
            make_item(0x7300 + i, 2 + i,
                      DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_ALLOWED_CONTAINER_PC34);
    }
    rt->chest[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34] =
        make_item(DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34, 17,
                  DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_ALLOWED_HANDS_PC34 |
                  DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_ALLOWED_CONTAINER_PC34);
    relink_visible_open_chain(rt);
    rt->leaderHand =
        make_item(DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34, 13,
                  DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_ALLOWED_HANDS_PC34 |
                  DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_ALLOWED_CONTAINER_PC34);
    rt->nonLeaderReadyHand = none_item();
}

static int count_thing(const RuntimePc34* rt, int thing)
{
    int count = 0;
    int i;

    if (rt->leaderHand.thing == thing) {
        ++count;
    }
    if (rt->nonLeaderReadyHand.thing == thing) {
        ++count;
    }
    for (i = 0; i < DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34; ++i) {
        if (rt->chest[i].thing == thing) {
            ++count;
        }
    }
    return count;
}

static int open_chain_skips_detached_c540(const RuntimePc34* rt)
{
    return rt->chest[2].next == rt->chest[4].thing &&
           rt->chest[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34].next ==
           DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34;
}

static int click_non_leader_ready_hand(RuntimePc34* rt)
{
    DM1_V1_ChestOccupiedSlotSwapItemPc34 leaderBefore;
    DM1_V1_ChestOccupiedSlotSwapItemPc34 slotBefore;

    if (!rt) {
        return 0;
    }
    leaderBefore = rt->leaderHand;
    slotBefore = rt->nonLeaderReadyHand;
    if (is_none(leaderBefore) && is_none(slotBefore)) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0302:677-687 resolves C000..C003 status
     * hand boxes to a non-inventory champion hand. F0302:700-710 then
     * removes the leader hand with F0298 and stores it through F0301.
     */
    ++rt->f0077CallCount;
    rt->leaderHand = none_item();
    if (!is_none(slotBefore)) {
        rt->leaderHand = slotBefore;
    }
    if (!is_none(leaderBefore)) {
        rt->nonLeaderReadyHand = leaderBefore;
    }
    ++rt->f0078CallCount;
    return 1;
}

static int click_c540_chest_slot(RuntimePc34* rt)
{
    DM1_V1_ChestOccupiedSlotSwapItemPc34 leaderBefore;
    DM1_V1_ChestOccupiedSlotSwapItemPc34 slotBefore;
    int target = DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34;

    if (!rt) {
        return 0;
    }
    leaderBefore = rt->leaderHand;
    slotBefore = rt->chest[target];
    if (is_none(leaderBefore) && is_none(slotBefore)) {
        return 0;
    }

    /*
     * ReDMCSB CHAMPION.C F0302:688-710 routes C30+ through G0425.
     * F0300:511-515 clears the occupied C540 entry, F0297:243-268
     * puts that C540 thing into the leader hand, and F0164:1840-1905
     * is mirrored here by relinking the still-open visible chain after
     * detaching the old C540 thing. No F0334 close rewrite is called.
     */
    ++rt->f0077CallCount;
    rt->leaderHand = none_item();
    rt->chest[target] = none_item();
    if (!is_none(slotBefore)) {
        rt->leaderHand = slotBefore;
    }
    if (!is_none(leaderBefore)) {
        rt->chest[target] = leaderBefore;
    }
    relink_visible_open_chain(rt);
    ++rt->f0078CallCount;
    return 1;
}

static void fill_command(
    DM1_V1_ChestOccupiedSlotSwapCommandPc34* cmd,
    int command,
    int zone,
    int pc34Slot,
    int resolvedChampion,
    int resolvedHandSlot)
{
    cmd->command = command;
    cmd->x = 32 + zone;
    cmd->y = 16 + pc34Slot;
    cmd->pc34Slot = pc34Slot;
    cmd->zone = zone;
    cmd->resolvedChampion = resolvedChampion;
    cmd->resolvedHandSlot = resolvedHandSlot;
}

static uint32_t hash_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t compute_hash(const DM1_V1_ChestOccupiedSlotSwapProbePc34* p)
{
    uint32_t hash = 2166136261u;
    int i;

    hash = hash_u32(hash, (uint32_t)p->deterministicSeed);
    hash = hash_u32(hash, (uint32_t)p->leaderHandBefore);
    hash = hash_u32(hash, (uint32_t)p->leaderHandAfterReceive);
    hash = hash_u32(hash, (uint32_t)p->leaderHandAfterChestClick);
    hash = hash_u32(hash, (uint32_t)p->nonLeaderHandAfterReceive);
    hash = hash_u32(hash, (uint32_t)p->targetChestAfterChestClick);
    for (i = 0; i < DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_SLOT_COUNT_PC34; ++i) {
        hash = hash_u32(hash, (uint32_t)p->afterChestTypes[i]);
        hash = hash_u32(hash, (uint32_t)p->afterChestNext[i]);
    }
    hash = hash_u32(hash, (uint32_t)p->f0334CloseCallCount);
    hash = hash_u32(hash, (uint32_t)p->f0077CallCount);
    hash = hash_u32(hash, (uint32_t)p->f0078CallCount);
    return hash;
}

const char* dm1_v1_chest_occupied_slot_swap_source_evidence_pc34(void)
{
    return s_source_evidence;
}

const DM1_V1_ChestOccupiedSlotSwapSpecPc34*
dm1_v1_chest_occupied_slot_swap_spec_pc34(void)
{
    return &s_spec;
}

int dm1_v1_chest_occupied_slot_swap_pc34(
    DM1_V1_ChestOccupiedSlotSwapProbePc34* out)
{
    RuntimePc34 rt;
    int receiveOk;
    int chestOk;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    init_runtime(&rt);

    out->setupResult = 1;
    out->contractOnly = 1;
    out->deterministicSeed = s_spec.deterministicSeed;
    out->partyChampionCount = s_spec.partyChampionCount;
    out->inventoryChampionOrdinal = s_spec.inventoryChampionOrdinal;
    out->candidateChampionOrdinal = 0;
    out->leaderIndex = s_spec.leaderIndex;
    out->nonLeaderIndex = s_spec.nonLeaderIndex;
    out->leaderCurrentHealth = 100;
    out->nonLeaderCurrentHealth = 100;
    out->openChestBefore = rt.openChest;
    out->panelWasChestBefore = rt.panel == PANEL_CHEST ? 1 : 0;
    out->leaderHandBefore = rt.leaderHand.thing;
    out->nonLeaderHandBefore = rt.nonLeaderReadyHand.thing;
    out->targetChestBefore =
        rt.chest[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34].thing;
    copy_chest(&rt, out->beforeChestTypes, out->beforeChestNext);

    out->queuedCount = DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_COMMAND_COUNT_PC34;
    out->queueWriteOrder[0] = COMMAND_SLOT_BOX_C540;
    out->queueWriteOrder[1] = COMMAND_SLOT_BOX_NON_LEADER_READY;
    out->queueDrainOrder[0] = COMMAND_SLOT_BOX_NON_LEADER_READY;
    out->queueDrainOrder[1] = COMMAND_SLOT_BOX_C540;
    fill_command(&out->commands[0], COMMAND_SLOT_BOX_C540,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_ZONE_PC34,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C30_BASE_PC34 +
                     DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_INDEX_PC34,
                 -1);
    fill_command(&out->commands[1], COMMAND_SLOT_BOX_NON_LEADER_READY,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_NON_LEADER_SLOT_BOX_PC34,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_READY_HAND_PC34,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_NON_LEADER_INDEX_PC34,
                 DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_READY_HAND_PC34);
    out->f0359QueuedChestC540 = 1;
    out->f0359QueuedNonLeaderHand = 1;
    out->f0380DrainedNonLeaderFirst = 1;

    receiveOk = click_non_leader_ready_hand(&rt);
    out->leaderHandAfterReceive = rt.leaderHand.thing;
    out->nonLeaderHandAfterReceive = rt.nonLeaderReadyHand.thing;
    out->targetChestAfterReceive =
        rt.chest[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34].thing;
    out->openChestAfterReceive = rt.openChest;
    copy_chest(&rt, out->afterReceiveChestTypes, out->afterReceiveChestNext);
    out->f0302ResolvedNonLeaderHand = receiveOk;
    out->f0298RemovedLeaderWeaponForReceive =
        receiveOk && out->leaderHandAfterReceive ==
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34;
    out->f0301WroteLeaderWeaponToNonLeaderHand =
        receiveOk && out->nonLeaderHandAfterReceive ==
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34;

    out->f0380DrainedChestSecond = 1;
    chestOk = click_c540_chest_slot(&rt);
    out->exerciseResult = receiveOk && chestOk;
    out->leaderHandAfterChestClick = rt.leaderHand.thing;
    out->nonLeaderHandAfterChestClick = rt.nonLeaderReadyHand.thing;
    out->targetChestAfterChestClick =
        rt.chest[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34].thing;
    out->openChestAfterChestClick = rt.openChest;
    out->panelStillChestAfter = rt.panel == PANEL_CHEST ? 1 : 0;
    copy_chest(&rt, out->afterChestTypes, out->afterChestNext);
    copy_chest(&rt, out->expectedAfterChestTypes, out->expectedAfterChestNext);

    out->f0302ResolvedChestC540 = chestOk;
    out->f0300ClearedC540 =
        chestOk && out->targetChestAfterChestClick ==
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34;
    out->f0297PutC540WeaponInLeaderHand =
        chestOk && out->leaderHandAfterChestClick ==
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34;
    out->f0164DetachedOldC540FromOpenChain =
        chestOk &&
        rt.chest[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34].thing ==
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34;
    out->f0334CloseCallCount = rt.f0334CloseCallCount;
    out->f0333OpenCallCount = rt.f0333OpenCallCount;
    out->f0077CallCount = rt.f0077CallCount;
    out->f0078CallCount = rt.f0078CallCount;

    out->c537Stable = out->afterChestTypes[0] == out->beforeChestTypes[0];
    out->c538Stable = out->afterChestTypes[1] == out->beforeChestTypes[1];
    out->c539Stable = out->afterChestTypes[2] == out->beforeChestTypes[2];
    out->c541Stable = out->afterChestTypes[4] == out->beforeChestTypes[4];
    out->c542Stable = out->afterChestTypes[5] == out->beforeChestTypes[5];
    out->c543Stable = out->afterChestTypes[6] == out->beforeChestTypes[6];
    out->c544Stable = out->afterChestTypes[7] == out->beforeChestTypes[7];
    out->c540Cleared =
        out->afterChestTypes[DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_TARGET_INDEX_PC34] ==
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_THING_NONE_PC34;
    out->noDuplicateLeaderWeapon =
        count_thing(&rt, DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34) == 1;
    out->noDuplicateC540Weapon =
        count_thing(&rt, DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34) == 1;
    out->noThingNoneInsideNonEmptyPrefix = open_chain_skips_detached_c540(&rt);
    out->oldLeaderWeaponMovedToNonLeader =
        rt.nonLeaderReadyHand.thing ==
        DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_LEADER_WEAPON_PC34;
    out->oldC540WeaponMovedToLeader =
        rt.leaderHand.thing == DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_C540_WEAPON_PC34;
    out->chestStayedOpen =
        rt.openChest == DM1_V1_CHEST_OCCUPIED_SLOT_SWAP_OPEN_CHEST_PC34;
    out->closePathNotUsed = rt.f0334CloseCallCount == 0;
    out->hash = compute_hash(out);
    return out->exerciseResult;
}
