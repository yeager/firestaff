#include "dm1_v1_chest_teleport_survival_pc34_compat.h"

#include <string.h>

typedef struct {
    int thing;
    int weight;
    int iconIndex;
} ChestTeleportItemPc34;

typedef struct {
    int currentMapIndex;
    int partyMapIndex;
    int partyMapX;
    int partyMapY;
    int openChestThing;
    int chestThing;
    int chestOwningMapIndex;
    ChestTeleportItemPc34 g0425[
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT];
    ChestTeleportItemPc34 chestLinks[
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT];
    ChestTeleportItemPc34 leaderHand;
    int leaderLoad;
    int leaderHandPutCount;
    int leaderHandRemoveCount;
    int leaderHandNameRefreshed;
    int leaderHandPointerStable;
    int championHealth[DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT];
    int mapSetCount;
    int teleportCount;
    int closeRewriteCount;
    int closeRecompactCount;
    int closeClearedOpenChest;
    int closeWithoutOpenEarlyReturnCount;
    int objectIconLookupCount;
    int blitRouteCount;
    int c30SlotClearWriteObserved;
} ChestTeleportRuntimePc34;

static const char s_source_evidence[] =
    "CHEST.C F0333:31-67 opens by selecting G0426 and copying linked chest contents into G0425\n"
    "CHEST.C F0334:113-132 closes by clearing G0426 and recompacting non-empty G0425 slots\n"
    "CHAMPION.C F0297:243-298 puts leader-hand object pointer/name/weight and redraw state\n"
    "CHAMPION.C F0298:270-298 removes leader-hand object and reverses weight state\n"
    "CHAMPION.C F0300:511-515 clears C30+ G0425 slots and F0301:606-614 writes C30+ slots\n"
    "CHAMPION.C F0302:662-710 swaps occupied slots through leader-hand remove/put routing\n"
    "DUNGEON.C F0163:1769-1838 appends rewritten links while preserving dungeon list identity\n"
    "DUNGEON.C F0173:2724-2740/F0174:2742-2756 change current/party map metadata only\n"
    "MOVESENS.C F0267:469-492 resolves teleporter targets and calls F0173 during level changes\n"
    "OBJECT.C F0033:147-212 maps thing identity to icon index; BLITMASK.C F0133:30-33 is presentation-only\n"
    "DEFS.H:2088 plus C30/G0425/G0426/M070/M516 define color, slots, globals, hand macro, and champions";

static ChestTeleportItemPc34 make_item(int thing, int weight)
{
    ChestTeleportItemPc34 item;

    item.thing = thing;
    item.weight = weight;
    item.iconIndex = thing == DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE ?
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE : thing & 0x00FF;
    return item;
}

static int item_is_empty(const ChestTeleportItemPc34* item)
{
    return !item ||
        item->thing == DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE ||
        item->thing == DM1_PC34_CHEST_TELEPORT_SURVIVAL_END_OF_LIST;
}

static void clear_slots(ChestTeleportItemPc34* items, int count)
{
    int i;

    for (i = 0; i < count; ++i) {
        items[i] = make_item(DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE, 0);
    }
}

static int count_visible(const ChestTeleportItemPc34* items)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        if (!item_is_empty(&items[i])) {
            ++count;
        }
    }
    return count;
}

static void copy_things(const ChestTeleportItemPc34* src, int* dst)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        dst[i] = item_is_empty(&src[i]) ?
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE : src[i].thing;
    }
}

static int link_head(const ChestTeleportItemPc34* items)
{
    int i;

    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        if (!item_is_empty(&items[i])) {
            return items[i].thing;
        }
    }
    return DM1_PC34_CHEST_TELEPORT_SURVIVAL_END_OF_LIST;
}

static int link_tail(const ChestTeleportItemPc34* items)
{
    int i;
    int tail = DM1_PC34_CHEST_TELEPORT_SURVIVAL_END_OF_LIST;

    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        if (!item_is_empty(&items[i])) {
            tail = items[i].thing;
        }
    }
    return tail;
}

static int alive_count(const ChestTeleportRuntimePc34* runtime)
{
    int count = 0;
    int i;

    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT; ++i) {
        if (runtime->championHealth[i] > 0) {
            ++count;
        }
    }
    return count;
}

static void init_runtime(ChestTeleportRuntimePc34* runtime)
{
    int i;

    memset(runtime, 0, sizeof(*runtime));
    runtime->currentMapIndex = DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A;
    runtime->partyMapIndex = DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A;
    runtime->partyMapX = 11;
    runtime->partyMapY = 17;
    runtime->openChestThing = DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE;
    runtime->chestThing = DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A;
    runtime->chestOwningMapIndex = DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A;
    clear_slots(runtime->g0425, DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT);
    clear_slots(runtime->chestLinks,
                DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT);
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT; ++i) {
        runtime->chestLinks[i] = make_item(
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST + i,
            2 + i);
    }
    runtime->leaderHand = make_item(
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_THING,
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_WEIGHT);
    runtime->leaderLoad = runtime->leaderHand.weight;
    runtime->leaderHandPutCount = 1;
    runtime->leaderHandNameRefreshed = 1;
    runtime->leaderHandPointerStable = 1;
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT; ++i) {
        runtime->championHealth[i] = 100 - (i * 7);
    }
}

static int chest_resolved_on_current_map(const ChestTeleportRuntimePc34* runtime)
{
    return runtime->openChestThing == runtime->chestThing &&
        runtime->currentMapIndex == runtime->chestOwningMapIndex;
}

static void snapshot_runtime(
    const ChestTeleportRuntimePc34* runtime,
    const char* phaseName,
    M11_GameView_ChestTeleportSurvivalSnapshotPc34* out)
{
    int i;

    memset(out, 0, sizeof(*out));
    out->phaseName = phaseName;
    out->currentMapIndex = runtime->currentMapIndex;
    out->partyMapIndex = runtime->partyMapIndex;
    out->partyMapX = runtime->partyMapX;
    out->partyMapY = runtime->partyMapY;
    out->openChestThing = runtime->openChestThing;
    out->chestThing = runtime->chestThing;
    out->chestOwningMapIndex = runtime->chestOwningMapIndex;
    out->chestResolvedOnCurrentMap = chest_resolved_on_current_map(runtime);
    copy_things(runtime->g0425, out->g0425Slots);
    copy_things(runtime->chestLinks, out->chestLinkThings);
    out->chestVisibleCount = count_visible(runtime->g0425);
    out->chestLinkHead = link_head(runtime->chestLinks);
    out->chestLinkTail = link_tail(runtime->chestLinks);
    out->leaderHandThing = runtime->leaderHand.thing;
    out->leaderHandWeight = runtime->leaderHand.weight;
    out->leaderHandIconIndex = runtime->leaderHand.iconIndex;
    out->leaderHandNameRefreshed = runtime->leaderHandNameRefreshed;
    out->leaderHandPointerStable = runtime->leaderHandPointerStable;
    out->leaderHandRemoveCount = runtime->leaderHandRemoveCount;
    out->leaderHandPutCount = runtime->leaderHandPutCount;
    out->leaderLoad = runtime->leaderLoad;
    out->aliveChampionCount = alive_count(runtime);
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT; ++i) {
        out->championHealth[i] = runtime->championHealth[i];
    }
    out->mapSetCount = runtime->mapSetCount;
    out->teleportCount = runtime->teleportCount;
    out->closeRewriteCount = runtime->closeRewriteCount;
    out->closeRecompactCount = runtime->closeRecompactCount;
    out->closeClearedOpenChest = runtime->closeClearedOpenChest;
    out->closeWithoutOpenEarlyReturnCount =
        runtime->closeWithoutOpenEarlyReturnCount;
    out->objectIconLookupCount = runtime->objectIconLookupCount;
    out->blitRouteCount = runtime->blitRouteCount;
    out->c30SlotClearWriteObserved = runtime->c30SlotClearWriteObserved;
}

static int open_chest(ChestTeleportRuntimePc34* runtime)
{
    int i;

    if (!runtime || runtime->openChestThing == runtime->chestThing) {
        return 0;
    }

    /*
     * ReDMCSB CHEST.C F0333 lines 31-67 selects G0426 and fills the first
     * eight G0425 slots from the container link chain. OBJECT.C F0033 lines
     * 147-212 supplies icon identity, and BLITMASK.C F0133 lines 30-33 stays
     * on the presentation side of that open materialization.
     */
    runtime->openChestThing = runtime->chestThing;
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        runtime->g0425[i] = runtime->chestLinks[i];
        if (!item_is_empty(&runtime->g0425[i])) {
            runtime->objectIconLookupCount++;
        }
    }
    runtime->blitRouteCount++;
    return 1;
}

static void teleport_party(ChestTeleportRuntimePc34* runtime,
                           int mapIndex,
                           int mapX,
                           int mapY)
{
    if (!runtime) {
        return;
    }

    /*
     * ReDMCSB MOVESENS.C F0267 lines 469-492 and DUNGEON.C F0173/F0174
     * lines 2724-2756 change map metadata and party coordinates; neither
     * path clears PANEL.C's G0425/G0426 globals or the leader-hand object.
     */
    runtime->currentMapIndex = mapIndex;
    runtime->partyMapIndex = mapIndex;
    runtime->partyMapX = mapX;
    runtime->partyMapY = mapY;
    runtime->mapSetCount++;
    runtime->teleportCount++;
}

static int close_chest(ChestTeleportRuntimePc34* runtime)
{
    ChestTeleportItemPc34 rewritten[
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT];
    int i;
    int outIndex = 0;

    if (!runtime ||
        runtime->openChestThing == DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE ||
        !chest_resolved_on_current_map(runtime)) {
        if (runtime) {
            runtime->closeWithoutOpenEarlyReturnCount++;
        }
        return 0;
    }

    /*
     * ReDMCSB CHEST.C F0334 lines 113-132 clears G0426, resets the container
     * head, skips C0xFFFF_NONE G0425 slots, and recompacts visible links via
     * DUNGEON.C F0163 lines 1769-1838. C30+ slot clears model CHAMPION.C
     * F0300/F0301 lines 511-515 and 606-614.
     */
    clear_slots(rewritten, DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT);
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        if (!item_is_empty(&runtime->g0425[i])) {
            rewritten[outIndex++] = runtime->g0425[i];
            runtime->g0425[i] = make_item(
                DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE, 0);
        }
    }
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT; ++i) {
        runtime->chestLinks[i] = rewritten[i];
    }
    runtime->openChestThing = DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE;
    runtime->closeRewriteCount++;
    runtime->closeRecompactCount = outIndex;
    runtime->closeClearedOpenChest = 1;
    runtime->c30SlotClearWriteObserved = outIndex > 0 ? 1 : 0;
    return 1;
}

static void fill_anchors(M11_GameView_ChestTeleportSurvivalAnchorsPc34* out)
{
    out->chestF0333OpenMaterialization =
        "ReDMCSB CHEST.C F0333 lines 31-67";
    out->chestF0334CloseRewrite =
        "ReDMCSB CHEST.C F0334 lines 113-132";
    out->championF0297LeaderHandPut =
        "ReDMCSB CHAMPION.C F0297 lines 243-298";
    out->championF0298LeaderHandRemove =
        "ReDMCSB CHAMPION.C F0298 lines 270-298";
    out->championF0300ChestSlotClear =
        "ReDMCSB CHAMPION.C F0300 lines 511-515";
    out->championF0301ChestSlotWrite =
        "ReDMCSB CHAMPION.C F0301 lines 606-614";
    out->championF0302OccupiedSlotSwap =
        "ReDMCSB CHAMPION.C F0302 lines 662-710";
    out->dungeonF0163LinkAppend =
        "ReDMCSB DUNGEON.C F0163 lines 1769-1838";
    out->dungeonF0173F0174MapSet =
        "ReDMCSB DUNGEON.C F0173/F0174 lines 2724-2756";
    out->movesensF0267TeleportLevelChange =
        "ReDMCSB MOVESENS.C F0267 lines 469-492";
    out->objectF0033IconIndex =
        "ReDMCSB OBJECT.C F0033 lines 147-212";
    out->blitmaskF0133PresentationRoute =
        "ReDMCSB BLITMASK.C F0133 lines 30-33";
    out->defsSentinelsAndSlots =
        "ReDMCSB DEFS.H line 2088, C30, G0425, G0426, M070, M516";
}

static void fill_probe_constants(
    const ChestTeleportRuntimePc34* runtime,
    M11_GameView_ChestTeleportSurvivalProbePc34* out)
{
    int i;

    out->sourceLockedContractOnly = 1;
    out->c0xFFFFThingNone = DM1_PC34_CHEST_TELEPORT_SURVIVAL_THING_NONE;
    out->c0xFFFEThingEndOfList =
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_END_OF_LIST;
    out->c30ChestSlotBase = DM1_PC34_SLOT_CHEST_1;
    out->c37ChestSlotLast = DM1_PC34_SLOT_CHEST_8;
    out->g0425SlotCount = DM1_PC34_CHEST_TELEPORT_SURVIVAL_SLOT_COUNT;
    out->partyChampionCount = DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT;
    out->allChampionsAlive = alive_count(runtime) ==
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_PARTY_COUNT ? 1 : 0;
    out->leaderOrdinal = 1;
    out->inventoryChampionOrdinal = 1;
    out->chestThing = runtime->chestThing;
    out->chestMapIndex = runtime->chestOwningMapIndex;
    out->teleportDestinationMapIndex =
        DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B;
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT; ++i) {
        out->initialChestItems[i] = runtime->chestLinks[i].thing;
        out->initialChestWeights[i] = runtime->chestLinks[i].weight;
    }
    out->initialLeaderHandThing = runtime->leaderHand.thing;
    out->initialLeaderHandWeight = runtime->leaderHand.weight;
    fill_anchors(&out->anchors);
}

static void run_negative_path(
    M11_GameView_ChestTeleportSurvivalNegativePc34* out)
{
    ChestTeleportRuntimePc34 runtime;
    int i;

    init_runtime(&runtime);
    open_chest(&runtime);
    out->beforeMapIndex = runtime.currentMapIndex;
    teleport_party(&runtime, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B, 3, 9);
    out->afterMapIndex = runtime.currentMapIndex;
    close_chest(&runtime);
    out->closeAttemptedOnMap = runtime.currentMapIndex;
    out->openChestThingAfterCloseAttempt = runtime.openChestThing;
    copy_things(runtime.g0425, out->g0425SlotsAfterCloseAttempt);
    copy_things(runtime.chestLinks, out->chestLinkThingsAfterCloseAttempt);
    out->closeRewriteCount = runtime.closeRewriteCount;
    out->closeWithoutOpenEarlyReturnCount =
        runtime.closeWithoutOpenEarlyReturnCount;
    out->chestResolvedOnCurrentMap = chest_resolved_on_current_map(&runtime);
    out->leaderHandThingAfterCloseAttempt = runtime.leaderHand.thing;
    out->leaderHandWeightAfterCloseAttempt = runtime.leaderHand.weight;
    out->preservedOpenChestOnForeignMap =
        runtime.openChestThing == DM1_PC34_CHEST_TELEPORT_SURVIVAL_CHEST_A;
    out->preservedLeaderHandOnForeignMap =
        runtime.leaderHand.thing ==
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_THING &&
        runtime.leaderHand.weight ==
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_LEADER_HAND_WEIGHT;
    out->preservedG0425OnForeignMap = 1;
    for (i = 0; i < DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_COUNT; ++i) {
        if (out->g0425SlotsAfterCloseAttempt[i] !=
            DM1_PC34_CHEST_TELEPORT_SURVIVAL_ITEM_FIRST + i) {
            out->preservedG0425OnForeignMap = 0;
        }
    }
}

const char* M11_GameView_ChestTeleportSurvivalSourceEvidencePc34(void)
{
    return s_source_evidence;
}

int M11_GameView_ChestTeleportSurvivalRunPc34(
    M11_GameView_ChestTeleportSurvivalProbePc34* out)
{
    ChestTeleportRuntimePc34 runtime;

    if (!out) {
        return 0;
    }

    memset(out, 0, sizeof(*out));
    init_runtime(&runtime);
    fill_probe_constants(&runtime, out);

    open_chest(&runtime);
    snapshot_runtime(
        &runtime, "open chest on level A",
        &out->snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_OPEN_A]);

    teleport_party(&runtime, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_B, 3, 9);
    snapshot_runtime(
        &runtime, "teleport to level B",
        &out->snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_TELEPORT_B]);

    teleport_party(&runtime, DM1_PC34_CHEST_TELEPORT_SURVIVAL_MAP_A, 11, 17);
    snapshot_runtime(
        &runtime, "teleport back to level A",
        &out->snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_TELEPORT_A]);

    close_chest(&runtime);
    snapshot_runtime(
        &runtime, "close chest on level A",
        &out->snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_CLOSE_A]);

    open_chest(&runtime);
    snapshot_runtime(
        &runtime, "reopen chest on level A",
        &out->snapshots[DM1_PC34_CHEST_TELEPORT_SURVIVAL_SNAPSHOT_REOPEN_A]);

    run_negative_path(&out->negative);
    return 1;
}
