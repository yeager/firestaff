#ifndef DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_DURING_CHEST_PICKUP_RACE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_DURING_CHEST_PICKUP_RACE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHAMPION_COUNT_PC34 = 2,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_HAND_SLOT_COUNT_PC34 = 2,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHEST_SLOT_COUNT_PC34 = 8,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_NONE_PC34 = 0,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_SLOT_A_PC34 = 0,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_SLOT_B_PC34 = 1,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C30_SLOT_PC34 = 30,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C37_SLOT_PC34 = 37,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C38_SLOT_BOX_PC34 = 38,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C537_ZONE_PC34 = 537,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C544_ZONE_PC34 = 544,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C040_GRAPHIC_PC34 = 40,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_C162_CANCEL_PC34 = 162,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_M568_PANEL_PC34 = 568,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_M569_PANEL_PC34 = 569,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_MIRROR_THING_PC34 =
        0x7280,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_PENDING_THING_PC34 =
        0x7281,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHEST_THING_PC34 =
        0x7282,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_CHEST_ITEM_PC34 =
        0x7283,
    DM1_V1_MIRROR_CANDIDATE_PENDING_HAND_CHEST_RACE_BLOCKER_ITEM_PC34 =
        0x7284
};

typedef struct Dm1V1MirrorCandidatePendingHandDuringChestPickupEvidencePc34 {
    const char *contract;
    const char *chestPickup;
    const char *championHands;
    const char *commandQueue;
    const char *resurrectNotReached;
    const char *panelOnlyC040;
    const char *utilityObjectDefs;
} Dm1V1MirrorCandidatePendingHandDuringChestPickupEvidencePc34;

typedef struct Dm1V1MirrorCandidatePendingHandDuringChestPickupProbePc34 {
    int source_locked_contract_only;
    int no_real_asset_bitmap_parity;
    int no_game_data_load;
    int sameChampionCovered;
    int differentChampionCovered;
    int reverseOrderCovered;
    int failedPickupCovered;
    int hand_queue_consumes;
    int chest_pickup_corrupts_mirror;
    int mirror_slot_intact;
    int panel_redraws;
    int resurrectTriggers;
    int queueConsumedTotal;
    int chestPickupSuccesses;
    int chestPickupFailures;
    int c040OnlyPanelLive;
    int leaderHandItemSurvived;
    int pendingSwapItemSurvived;
    unsigned int hash;
} Dm1V1MirrorCandidatePendingHandDuringChestPickupProbePc34;

const Dm1V1MirrorCandidatePendingHandDuringChestPickupEvidencePc34 *
dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_evidence_pc34(
    void);

int dm1_v1_mirror_candidate_pending_hand_during_chest_pickup_race_run_pc34(
    Dm1V1MirrorCandidatePendingHandDuringChestPickupProbePc34 *out);

#ifdef __cplusplus
}
#endif

#endif
