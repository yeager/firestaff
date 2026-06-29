#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_ACCEPT_DEAD_OWNER_GUARD_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_C045_ACCEPT_DEAD_OWNER_GUARD_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * DM1 V1 mirror-candidate C045 food/water accept dead-owner guard gate.
 *
 * Source-locked contract-only gate that pins the C045 food/water accept
 * dispatch path while the C040 mirror-candidate's owner is dead. The lane
 * name is `dm1_v1_auto_mirror_c045_accept_dead_owner_guard_gate`; the
 * ReDMCSB anchors are cited in the source comment of the implementation.
 *
 * Non-overlap marker: pass772 covers C045 food/water ACCEPT plus same-drain
 * leader rotation with an ALIVE candidate owner; this gate covers the C045
 * accept flow while the C040 candidate's owner is DEAD and the accept must
 * route through the resurrection F0282 path without bypassing the
 * dead-owner contract, and must NOT mutate the alive leader's hand or
 * hand-off. Disjoint from c045_food_water_close_no_candidate,
 * c045_close_after_non_candidate_transition, c160_close_while_rotation_pending,
 * c061_drop_resurrect_pending, c545_accept_during_rotation,
 * close_while_c045_pending, resurrect_chest_close_order,
 * resurrect_reselect_with_inventory_pickup, resurrect_confirm_inventory_interrupt,
 * resurrect_cross_candidate_clear, resurrect_full_c30_chain,
 * resurrect_reincarnate_skills, resurrect_rearm,
 * panel_redraw_after_inventory_exit, c040_panel_browse_pickup_rotate_race,
 * c040_chrome_inventory_owner_swap, c040_close_non_leader_scroll_pickup,
 * c040_redraw_after_chest_close, c040_eye_live_candidate,
 * rotation_during_resurrect_confirmation, click_cancel_with_rotation,
 * click_cancel, inventory_toggle, teleporter_survival, resurrect_reincarnate_rearm,
 * chest_open_mirror_rotation_three_way, and the integrated F0107/F0108/
 * chest-scroll-wheel/viewport family.
 */

#define DM1_V1_MC_C045_DEAD_OWNER_PARTY_COUNT_PC34 4
#define DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34 4
#define DM1_V1_MC_C045_DEAD_OWNER_TRACE_COUNT_PC34 8
#define DM1_V1_MC_C045_DEAD_OWNER_NONE_PC34 0xffffu
#define DM1_V1_MC_C045_DEAD_OWNER_DEAD_PC34 0
#define DM1_V1_MC_C045_DEAD_OWNER_ALIVE_PC34 1

typedef struct {
    const char *reviveAddCandidateAnchor;
    const char *reviveAcceptClearAnchor;
    const char *reviveStatsResetAnchor;
    const char *championHandAnchor;
    const char *championSlotAnchor;
    const char *panelFoodWaterAnchor;
    const char *panelResurrectAnchor;
    const char *panelRedrawAnchor;
    const char *commandQueueAnchor;
    const char *commandPanelRouteAnchor;
    const char *commandDrainAnchor;
    const char *leaderSetAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
} Dm1V1MirrorCandidateC045AcceptDeadOwnerEvidencePc34;

typedef struct {
    int ordinal;
    int alive;
    int leader;
    int chainLinked;
    uint16_t handThing;
    int foodLevel;
    int waterLevel;
    int currentHealth;
    int maximumHealth;
} Dm1V1MirrorCandidateC045AcceptDeadOwnerChampionPc34;

typedef struct {
    int contractOnly;
    int assetFree;
    int partyChampionCount;
    int leaderIndex;
    int candidateOwnerIndex;
    int candidateChampionOrdinal;
    int candidateOwnerAlive;
    int inventoryChampionOrdinal;
    int g0299CandidateOrdinal;
    int c040PanelOpen;
    int c045PanelOpen;
    int c045AcceptPathActive;
    int panelContent;
    int panelGraphic;
    int c040Graphic;
    int c045Graphic;
    int acceptCommand;
    int queuedStatusCommand;
    int candidateChainOrdinals[DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34];
    int partyChainOrdinals[DM1_V1_MC_C045_DEAD_OWNER_PARTY_COUNT_PC34];
    uint16_t acceptedFoodThing;
    uint16_t leaderHandThing;
    int foodLevelBeforeAccept;
    int waterLevelBeforeAccept;
    int leaderHandEmpty;
    int candidateFoodLevelBefore;
    int candidateWaterLevelBefore;
    int f0280PublishCount;
    int f0282AcceptClearCount;
    int f0286StatsResetCount;
    int f0297PutLeaderHandCount;
    int f0298RemoveLeaderHandCount;
    int f0301FoodSlotAddCount;
    int f0302FoodSlotDispatchCount;
    int f0344FoodWaterReadCount;
    int f0345FoodWaterDrawCount;
    int f0346ResurrectDrawCount;
    int f0347PanelRedrawCount;
    int f0359QueueWriteCount;
    int f0361WheelLikeQueueWriteCount;
    int f0367LeaderClickRouteCount;
    int f0368SetLeaderCount;
    int f0378PanelRouteCount;
    int f0380DispatchCount;
    int commandQueueDepth;
    int f0077MouseScreenUpdateEnable;
    int f0078MouseScreenUpdateDisable;
    int f0457StartDrawEnabledMenus;
    int trace[DM1_V1_MC_C045_DEAD_OWNER_TRACE_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterAcceptHash;
    uint32_t afterLeaderSettleHash;
    Dm1V1MirrorCandidateC045AcceptDeadOwnerChampionPc34
        champions[DM1_V1_MC_C045_DEAD_OWNER_PARTY_COUNT_PC34];
} Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34;

typedef struct {
    int accepted;
    int deadOwnerAtStart;
    int aliveLeaderConfirmed;
    int c045PanelOpenForDeadOwner;
    int c040PanelNotLive;
    int acceptRoutedThroughF0282;
    int statsResetByF0286Ran;
    int candidateFoodLevelIncreased;
    int candidateWaterLevelIncreased;
    int leaderHandStable;
    int leaderHandNotConsumed;
    int leaderHandNotMismatched;
    int leaderHandEmptyBefore;
    int aliveLeaderChainLinked;
    int candidateRemovedFromChain;
    int g0299Cleared;
    int c040Cleared;
    int c045Cleared;
    int panelContentCleared;
    int panelRedrawStable;
    int mouseScreenUpdateBracketed;
    int drawEnabledMenusInvoked;
    int queueDrained;
    int noAcceptForAliveCandidate;
    int noAcceptForNullCandidate;
    int sourceAnchorsPresent;
    int guardRejectsAliveOwner;
    int guardRejectsNullCandidate;
    int guardRejectsNoC040Panel;
    int guardRejectsNoC045Path;
    int guardRejectsNoAcceptCommand;
    int guardRejectsGlobalLeaderHandThing;
    int guardRejectsLeaderHandEmptyFlagMismatch;
    int guardRejectsChampionHandThingOwnerMismatch;
    int leaderBefore;
    int leaderAfter;
    uint16_t leaderHandThingBefore;
    uint16_t leaderHandThingAfter;
    int candidateFoodLevelBefore;
    int candidateFoodLevelAfter;
    int candidateWaterLevelBefore;
    int candidateWaterLevelAfter;
    int g0299Before;
    int g0299After;
    int candidateChainBefore[DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34];
    int candidateChainAfter[DM1_V1_MC_C045_DEAD_OWNER_CHAIN_COUNT_PC34];
    int candidateIndexBefore;
    int candidateIndexAfter;
    int trace[DM1_V1_MC_C045_DEAD_OWNER_TRACE_COUNT_PC34];
    uint32_t beforeHash;
    uint32_t afterAcceptHash;
    uint32_t afterLeaderSettleHash;
    uint32_t hash;
} Dm1V1MirrorCandidateC045AcceptDeadOwnerResultPc34;

void dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_init_pc34(
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state);

int dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_run_pc34(
    Dm1V1MirrorCandidateC045AcceptDeadOwnerStatePc34 *state,
    Dm1V1MirrorCandidateC045AcceptDeadOwnerResultPc34 *result);

const Dm1V1MirrorCandidateC045AcceptDeadOwnerEvidencePc34 *
dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_evidence_pc34(void);

const char *
dm1_v1_mirror_candidate_c045_accept_dead_owner_guard_source_evidence_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif
