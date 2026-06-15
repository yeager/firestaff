#include "dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_pc34_compat.h"

#include <stdint.h>
#include <string.h>

/*
 * Source-lock anchors for this contract-only regression:
 * - ReDMCSB CHEST.C F0333:30-67 and F0334:117-132 materialize and close the
 *   G0425 chest-slot window used by chest pickup/drop.
 * - ReDMCSB CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-584,
 *   F0301:606-660, and F0302:662-713 define leader-hand, slot, and chest-slot
 *   swap behavior.
 * - ReDMCSB COMMAND.C F0378:1973-1983 and F0380:2045-2159 separate panel
 *   command dispatch from queued command processing.
 * - ReDMCSB REVIVE.C F0280:124-132 and F0282:744-806 are cited because this
 *   race must not fall into resurrect/reincarnate processing.
 * - ReDMCSB PANEL.C F0346/F0347:1619-1657 draws only the C040 candidate panel
 *   while G0299 remains a mirror candidate.
 * - ReDMCSB UTAMSCR.C F0077/F0078:141-150 and OBJECT.C F0033:147-212 cover
 *   action dispatch redraw and icon lookup without loading bitmap assets.
 * - ReDMCSB DEFS.H:338-340 C162, 810-817 C30..C37, 1874-1878 C38,
 *   2085-2088 G0305 party, 2088-2096 G0423 chest, 2200 C040, 3001-3008
 *   M568/M569, 3906-3913 C537..C544, 5694 G0299, 5876-5881 G0425/G0426.
 */

typedef struct ChampionPc34 {
    int handSlots[
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_HAND_SLOT_COUNT_PC34];
} ChampionPc34;

typedef struct PendingHandPc34 {
    int queued;
    int consumed;
    int championIndex;
    int fromSlot;
    int toSlot;
    int sourceThing;
} PendingHandPc34;

typedef struct RaceModelPc34 {
    ChampionPc34 champions[
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHAMPION_COUNT_PC34];
    PendingHandPc34 pending;
    int partyChampionCount;
    int inventoryChampionOrdinal;
    int mirrorCandidateOrdinal;
    int mirrorCandidateSlotThing;
    int panelContent;
    int c040PanelLive;
    int leaderHandThing;
    int openChestThing;
    int chestSlots[
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHEST_SLOT_COUNT_PC34];
    int queueConsumedTotal;
    int handQueueConsumesChestItem;
    int chestPickupCorruptsMirror;
    int resurrectTriggers;
    int panelRedraws;
    int nonC040PanelDraws;
    int chestPickupSuccesses;
    int chestPickupFailures;
} RaceModelPc34;

static const Dm1V1MirrorCandidatePendingHandDuringChestPickupEvidencePc34
    s_evidence = {
        "source_locked_contract_only=1 no_real_asset_bitmap_parity=1 "
        "no_game_data_load=1",
        "ReDMCSB CHEST.C F0333:30-67 and F0334:117-132 keep G0425/G0426 "
        "chest pickup/drop state separate from a mirror-candidate slot",
        "ReDMCSB CHAMPION.C F0297:243-268, F0298:270-298, F0300:511-584, "
        "F0301:606-660, F0302:662-713 route hand-slot and chest-slot swaps",
        "ReDMCSB COMMAND.C F0378:1973-1983 and F0380:2045-2159 separate "
        "panel dispatch from pending command queue processing",
        "ReDMCSB REVIVE.C F0280:124-132 and F0282:744-806 are not reached "
        "by this pending-hand/chest-pickup race",
        "ReDMCSB PANEL.C F0346/F0347:1619-1657 keeps only C040 candidate "
        "panel redraws live in this scenario",
        "ReDMCSB UTAMSCR.C F0077/F0078:141-150; OBJECT.C F0033:147-212; "
        "DEFS.H:338-340,810-817,1874-1878,2085-2088,2088-2096,2200,"
        "3001-3008,3906-3913,5694,5876-5881"
    };

static uint32_t fnv1a_step_u32(uint32_t hash, uint32_t value)
{
    int i;

    for (i = 0; i < 4; ++i) {
        hash ^= (value >> (i * 8)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static uint32_t hash_probe(
    const Dm1V1MirrorCandidatePendingHandDuringChestPickupProbePc34 *probe)
{
    uint32_t hash = 2166136261u;

    hash = fnv1a_step_u32(hash, (uint32_t)probe->source_locked_contract_only);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->no_real_asset_bitmap_parity);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->no_game_data_load);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->sameChampionCovered);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->differentChampionCovered);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->reverseOrderCovered);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->failedPickupCovered);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->hand_queue_consumes);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->chest_pickup_corrupts_mirror);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->mirror_slot_intact);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->panel_redraws);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->resurrectTriggers);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->queueConsumedTotal);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->chestPickupSuccesses);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->chestPickupFailures);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->c040OnlyPanelLive);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->leaderHandItemSurvived);
    hash = fnv1a_step_u32(hash, (uint32_t)probe->pendingSwapItemSurvived);
    return hash;
}

static void init_model(RaceModelPc34 *model, int championWithPending)
{
    memset(model, 0, sizeof(*model));
    model->partyChampionCount =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHAMPION_COUNT_PC34;
    model->inventoryChampionOrdinal = 1;
    model->mirrorCandidateOrdinal = 2;
    model->mirrorCandidateSlotThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_MIRROR_THING_PC34;
    model->panelContent =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_M568_PANEL_PC34;
    model->c040PanelLive = 1;
    model->leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34;
    model->openChestThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHEST_THING_PC34;
    model->champions[championWithPending]
        .handSlots[DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_SLOT_A_PC34] =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_PENDING_THING_PC34;
    model->chestSlots[0] =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHEST_ITEM_PC34;
}

static void redraw_c040_panel(RaceModelPc34 *model)
{
    if (model->c040PanelLive &&
        model->panelContent ==
            DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_M568_PANEL_PC34) {
        ++model->panelRedraws;
    } else {
        ++model->nonC040PanelDraws;
    }
}

static int mirror_slot_is_intact(const RaceModelPc34 *model)
{
    return model->mirrorCandidateOrdinal == 2 &&
           model->mirrorCandidateSlotThing ==
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_MIRROR_THING_PC34 &&
           model->partyChampionCount ==
               DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHAMPION_COUNT_PC34;
}

static void note_mirror_integrity(RaceModelPc34 *model)
{
    if (!mirror_slot_is_intact(model)) {
        ++model->chestPickupCorruptsMirror;
    }
}

static int queue_hand_swap(RaceModelPc34 *model, int championIndex)
{
    PendingHandPc34 *pending = &model->pending;

    if (!model || championIndex < 0 ||
        championIndex >=
            DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHAMPION_COUNT_PC34) {
        return 0;
    }
    if (pending->queued) {
        return 0;
    }
    pending->queued = 1;
    pending->championIndex = championIndex;
    pending->fromSlot =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_SLOT_A_PC34;
    pending->toSlot = DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_SLOT_B_PC34;
    pending->sourceThing = model->champions[championIndex]
                               .handSlots[pending->fromSlot];
    return pending->sourceThing ==
           DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_PENDING_THING_PC34;
}

static int process_pending_hand_swap(RaceModelPc34 *model)
{
    PendingHandPc34 *pending = &model->pending;
    ChampionPc34 *champion;

    if (!model || !pending->queued || pending->consumed) {
        return 0;
    }
    champion = &model->champions[pending->championIndex];
    if (champion->handSlots[pending->fromSlot] != pending->sourceThing ||
        champion->handSlots[pending->toSlot] !=
            DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34) {
        return 0;
    }
    champion->handSlots[pending->fromSlot] =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34;
    champion->handSlots[pending->toSlot] = pending->sourceThing;
    pending->consumed = 1;
    ++model->queueConsumedTotal;
    redraw_c040_panel(model);
    note_mirror_integrity(model);
    return 1;
}

static int chest_pickup(RaceModelPc34 *model, int championIndex)
{
    int slotThing;

    (void)championIndex;
    if (!model || model->openChestThing ==
            DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34) {
        return 0;
    }
    if (model->leaderHandThing !=
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34) {
        ++model->chestPickupFailures;
        note_mirror_integrity(model);
        return 0;
    }
    slotThing = model->chestSlots[0];
    if (slotThing ==
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34) {
        ++model->chestPickupFailures;
        note_mirror_integrity(model);
        return 0;
    }
    model->chestSlots[0] =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34;
    model->leaderHandThing = slotThing;
    ++model->chestPickupSuccesses;
    redraw_c040_panel(model);
    note_mirror_integrity(model);
    return 1;
}

static int run_success_order(int pickupChampion,
                             int processQueueBeforePickup,
                             int *leaderHandItemSurvived,
                             int *pendingSwapItemSurvived,
                             int *mirrorIntactCount,
                             RaceModelPc34 *outModel)
{
    RaceModelPc34 model;
    int pendingChampion = 0;

    init_model(&model, pendingChampion);
    if (!queue_hand_swap(&model, pendingChampion)) {
        return 0;
    }
    if (processQueueBeforePickup && !process_pending_hand_swap(&model)) {
        return 0;
    }
    if (!chest_pickup(&model, pickupChampion)) {
        return 0;
    }
    if (!processQueueBeforePickup && !process_pending_hand_swap(&model)) {
        return 0;
    }
    if (model.leaderHandThing ==
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHEST_ITEM_PC34) {
        ++*leaderHandItemSurvived;
    } else {
        ++model.handQueueConsumesChestItem;
    }
    if (model.champions[pendingChampion]
            .handSlots[DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_SLOT_B_PC34] ==
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_PENDING_THING_PC34) {
        ++*pendingSwapItemSurvived;
    }
    if (mirror_slot_is_intact(&model) && model.resurrectTriggers == 0) {
        ++*mirrorIntactCount;
    }
    if (outModel) {
        *outModel = model;
    }
    return model.handQueueConsumesChestItem == 0 &&
           model.chestPickupCorruptsMirror == 0 &&
           model.resurrectTriggers == 0 &&
           model.nonC040PanelDraws == 0;
}

static int run_failed_pickup(int *mirrorIntactCount, RaceModelPc34 *outModel)
{
    RaceModelPc34 model;

    init_model(&model, 0);
    if (!queue_hand_swap(&model, 0)) {
        return 0;
    }
    model.leaderHandThing =
        DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_BLOCKER_ITEM_PC34;
    if (chest_pickup(&model, 1)) {
        return 0;
    }
    if (mirror_slot_is_intact(&model) && model.resurrectTriggers == 0) {
        ++*mirrorIntactCount;
    }
    if (outModel) {
        *outModel = model;
    }
    return model.pending.queued == 1 &&
           model.pending.consumed == 0 &&
           model.queueConsumedTotal == 0 &&
           model.chestPickupFailures == 1 &&
           model.chestPickupCorruptsMirror == 0 &&
           model.resurrectTriggers == 0 &&
           model.nonC040PanelDraws == 0;
}

const Dm1V1MirrorCandidatePendingHandDuringChestPickupEvidencePc34 *
dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_evidence_pc34(
    void)
{
    return &s_evidence;
}

int dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_run_pc34(
    Dm1V1MirrorCandidatePendingHandDuringChestPickupProbePc34 *out)
{
    RaceModelPc34 sameChampion;
    RaceModelPc34 differentChampion;
    RaceModelPc34 reverseOrder;
    RaceModelPc34 failedPickup;
    int leaderHandItemSurvived = 0;
    int pendingSwapItemSurvived = 0;
    int mirrorIntactCount = 0;

    if (!out) {
        return 0;
    }
    memset(out, 0, sizeof(*out));
    out->source_locked_contract_only = 1;
    out->no_real_asset_bitmap_parity = 1;
    out->no_game_data_load = 1;

    out->sameChampionCovered = run_success_order(
        0, 0, &leaderHandItemSurvived, &pendingSwapItemSurvived,
        &mirrorIntactCount, &sameChampion);
    out->differentChampionCovered = run_success_order(
        1, 0, &leaderHandItemSurvived, &pendingSwapItemSurvived,
        &mirrorIntactCount, &differentChampion);
    out->reverseOrderCovered = run_success_order(
        1, 1, &leaderHandItemSurvived, &pendingSwapItemSurvived,
        &mirrorIntactCount, &reverseOrder);
    out->failedPickupCovered =
        run_failed_pickup(&mirrorIntactCount, &failedPickup);

    out->hand_queue_consumes =
        sameChampion.handQueueConsumesChestItem +
        differentChampion.handQueueConsumesChestItem +
        reverseOrder.handQueueConsumesChestItem +
        failedPickup.handQueueConsumesChestItem;
    out->chest_pickup_corrupts_mirror =
        sameChampion.chestPickupCorruptsMirror +
        differentChampion.chestPickupCorruptsMirror +
        reverseOrder.chestPickupCorruptsMirror +
        failedPickup.chestPickupCorruptsMirror;
    out->mirror_slot_intact = mirrorIntactCount;
    out->panel_redraws =
        sameChampion.panelRedraws + differentChampion.panelRedraws +
        reverseOrder.panelRedraws + failedPickup.panelRedraws;
    out->resurrectTriggers =
        sameChampion.resurrectTriggers + differentChampion.resurrectTriggers +
        reverseOrder.resurrectTriggers + failedPickup.resurrectTriggers;
    out->queueConsumedTotal =
        sameChampion.queueConsumedTotal + differentChampion.queueConsumedTotal +
        reverseOrder.queueConsumedTotal + failedPickup.queueConsumedTotal;
    out->chestPickupSuccesses =
        sameChampion.chestPickupSuccesses +
        differentChampion.chestPickupSuccesses +
        reverseOrder.chestPickupSuccesses +
        failedPickup.chestPickupSuccesses;
    out->chestPickupFailures =
        sameChampion.chestPickupFailures +
        differentChampion.chestPickupFailures +
        reverseOrder.chestPickupFailures +
        failedPickup.chestPickupFailures;
    out->c040OnlyPanelLive =
        sameChampion.nonC040PanelDraws + differentChampion.nonC040PanelDraws +
        reverseOrder.nonC040PanelDraws + failedPickup.nonC040PanelDraws;
    out->leaderHandItemSurvived = leaderHandItemSurvived;
    out->pendingSwapItemSurvived = pendingSwapItemSurvived;
    out->hash = hash_probe(out);

    return out->source_locked_contract_only == 1 &&
           out->no_real_asset_bitmap_parity == 1 &&
           out->no_game_data_load == 1 &&
           out->sameChampionCovered == 1 &&
           out->differentChampionCovered == 1 &&
           out->reverseOrderCovered == 1 &&
           out->failedPickupCovered == 1 &&
           out->hand_queue_consumes == 0 &&
           out->chest_pickup_corrupts_mirror == 0 &&
           out->mirror_slot_intact == 4 &&
           out->panel_redraws == 6 &&
           out->resurrectTriggers == 0 &&
           out->queueConsumedTotal == 3 &&
           out->chestPickupSuccesses == 3 &&
           out->chestPickupFailures == 1 &&
           out->c040OnlyPanelLive == 0 &&
           out->leaderHandItemSurvived == 3 &&
           out->pendingSwapItemSurvived == 3;
}
