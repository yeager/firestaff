#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_ORDER_PARTY_SHUFFLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_ORDER_PARTY_SHUFFLE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Lane: DM1 V1 mirror-candidate close-order party-shuffle
 * (pass790plus dm1_v1_auto_mirror_candidate_party_shuffle_close_order_gate).
 *
 * Pins the source-locked contract for a queue order in which a C160/C161/
 * C162 close click is dispatched BEFORE two F0284 party-direction
 * rotations while a mirror candidate (C040) is live. This is the
 * inverse queue order from pass783
 * (dm1_v1_mirror_candidate_close_after_party_shuffle) which dispatches
 * the F0284 rotations first and the C160 close last. The close-order
 * discipline pins that the C160 close click must resolve the
 * pre-shuffle state (G0305 = 3 -> 2, G0299 = 0, candidate append
 * chain cleared) and that the subsequent F0284 rotations must NOT
 * re-open the C040 panel, must NOT recreate the candidate, must NOT
 * call F0282 again, and must NOT mutate the C038 panel priority
 * byte, the C037 status hand box, the C159 champion icon, the M070
 * panel owner ordinal, or the M568 panel content past their
 * post-close stable bytes.
 *
 * The gate is intentionally non-duplicative with:
 *   - dm1_v1_mirror_candidate_close_after_party_shuffle
 *     (pass783, F0284, F0284, C160 in order, all in same _run call)
 *   - dm1_v1_mirror_candidate_c160_close_while_rotation_pending
 *     (pass788, C160 closes while F0302 slot rotation is in-flight)
 *   - dm1_v1_mirror_candidate_party_direction
 *     (5 turns + status click, no C160 close click)
 *   - dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate
 *     (close first, rotate after, reopen with the post-rotate
 *     candidate)
 *   - dm1_v1_mirror_candidate_reshuffle_panel_live
 *     (party slot reorder, not direction rotation, via
 *     close_candidate_panel)
 *   - dm1_v1_mirror_candidate_close_button
 *     (close click without F0284 rotation)
 *   - dm1_v1_mirror_candidate_c040_chrome_inventory_owner_swap
 *     (single F0284 step in the same _run call, no C160 close)
 *   - dm1_v1_mirror_candidate_resurrect_chest_close_order
 *     (pass780, C028 resurrect + chest-close, no F0284 rotation)
 *
 * ReDMCSB anchors (WIP20210206, PC 3.4 path):
 *   - REVIVE.C F0282:744-806
 *     F0282_CHAMPION_ProcessCommands160To162_ClickInResurrectReincarnatePanel
 *     routes the C160 click through F0282, clears G0299, decrements
 *     G0305, calls F0355_INVENTORY_Toggle_CPSE(C04_CHAMPION_CLOSE_INVENTORY)
 *     at line 895, then calls F0457_START_DrawEnabledMenus_CPSF and
 *     F0067_MOUSE_SetPointerToNormal.
 *   - CHAMPION.C:93-130 F0284_CHAMPION_SetPartyDirection rotates the
 *     per-champion Cell/Direction by a delta derived from the
 *     requested direction vs. G0308, updates G0308, and calls
 *     F0296_CHAMPION_DrawChangedObjectIcons.
 *   - CHAMPION.C F0296 redraws the per-champion portrait boxes; the
 *     C038 panel priority byte, C037 status hand box, C159 champion
 *     icon, M070 panel owner ordinal, and M568 panel content stay
 *     byte-stable because the C040 panel is already closed before
 *     the F0284 dispatches.
 *   - COMMAND.C F0361:1709-1813 queues the keyboard turn input
 *     (TURN_RIGHT/TURN_LEFT) into the shared command queue.
 *   - COMMAND.C F0359:1452-1662 queues the C040 panel Yes click.
 *   - COMMAND.C F0380:2045-2156 drains one command at a time and
 *     routes C160 to F0282 and C001/C002 to F0365.
 *   - CLIKMENU.C:142-174 F0365_COMMAND_ProcessTypes1To2_TurnParty
 *     sets the highlight box, then calls
 *     F0276_SENSOR_ProcessThingAdditionOrRemoval twice and
 *     F0284_CHAMPION_SetPartyDirection with the new direction.
 *   - PANEL.C F0355:2244+ closes the inventory, calls F0334 chest
 *     close, redraws the movement arrows, and returns.
 *   - DEFS.H anchors: C040, C037/C038, C159, C160..C162, M070, M516.
 *
 * The close-order discipline also pins that the C160 close click
 * resolve happens once, that the F0282 accept-clear is called once
 * (no cancel-clear, no extra F0282 call), that the F0284 dispatches
 * are counted as 2 (no early-return no-op), and that the post-close
 * command queue depth is 0.
 */

#define DM1_V1_MC_COPS_PARTY_COUNT_PC34 3
#define DM1_V1_MC_COPS_DIRECTION_COUNT_PC34 4
#define DM1_V1_MC_COPS_COMMAND_COUNT_PC34 3
#define DM1_V1_MC_COPS_TRACE_COUNT_PC34 14

typedef enum {
    DM1_V1_MC_COPS_COMMAND_NONE_PC34 = 0,
    DM1_V1_MC_COPS_COMMAND_TURN_RIGHT_PC34 = 1,
    DM1_V1_MC_COPS_COMMAND_TURN_LEFT_PC34 = 2,
    DM1_V1_MC_COPS_COMMAND_C160_YES_PC34 = 160
} Dm1V1MirrorCandidateCloseOrderPartyShuffleCommandPc34;

typedef struct {
    int contractOnly;
    int noAssetReads;
    int noOriginalDosPixelParityClaim;
    int partyChampionCount;
    int candidatePartyOrdinal;
    int candidateIndexByte;
    int g0299CandidateOrdinal;
    int g0305PartyChampionCount;
    int g0308PartyDirection;
    int championCell[DM1_V1_MC_COPS_PARTY_COUNT_PC34];
    int championDirection[DM1_V1_MC_COPS_PARTY_COUNT_PC34];
    int c040PanelOpen;
    int c040PanelClosed;
    int c038PanelPriorityByte;
    int c037StatusHandBoxByte;
    int c159ChampionIconByte;
    int m568PanelResurrectReincarnate;
    int m070PanelOwnerOrdinal;
    Dm1V1MirrorCandidateCloseOrderPartyShuffleCommandPc34
        queuedCommands[DM1_V1_MC_COPS_COMMAND_COUNT_PC34];
    Dm1V1MirrorCandidateCloseOrderPartyShuffleCommandPc34
        dispatchOrder[DM1_V1_MC_COPS_COMMAND_COUNT_PC34];
    int commandQueueDepth;
    int queueWriteCountF0361Turn;
    int queueWriteCountF0359PanelClick;
    int dispatchDrainCountF0380;
    int f0284SetPartyDirectionCount;
    int f0284FirstDelta;
    int f0284SecondDelta;
    int f0296DrawChangedObjectIconsCount;
    int f0282AcceptClearCount;
    int f0282CancelClearCount;
    int f0282ReadsPreShuffleCandidate;
    int c159ChampionIconStable;
    int c038PanelPriorityStable;
    int c037StatusHandBoxStable;
    int m070PanelOwnerStable;
    int m568PanelContentStable;
    int panelStayedClosedThroughShuffle;
    int cellMutatedThroughShuffle;
    int directionMutatedThroughShuffle;
    int trace[DM1_V1_MC_COPS_TRACE_COUNT_PC34];
} Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34;

typedef struct {
    int closeDispatchedFirst;
    int f0282FiredOnPreShuffleParty;
    int g0299ClearedOnClose;
    int g0305DecrementedOnClose;
    int candidateIndexByteStableThroughClose;
    int c040PanelClosedOnClose;
    int shuffledFirst;
    int shuffledSecond;
    int f0284FiredTwice;
    int f0284FirstDeltaCorrect;
    int f0284SecondDeltaCorrect;
    int f0296CalledTwice;
    int g0308EastAfterFirst;
    int g0308SouthAfterSecond;
    int panelStayedClosedThroughShuffle;
    int c038PanelPriorityPreserved;
    int c037StatusHandBoxPreserved;
    int c159ChampionIconPreserved;
    int m070PanelOwnerPreserved;
    int m568PanelContentPreserved;
    int g0308SouthAfterShuffle;
    int championCellsAfterShuffle;
    int championDirectionsAfterShuffle;
    int cellMutatedThroughShuffle;
    int directionMutatedThroughShuffle;
    int queueWriteOrderPreserved;
    int dispatchOrderPreserved;
    int f0380DrainProcessedAll;
    int sourceAnchorsPresent;
    int guardRejectsNullState;
    int guardRejectsNullResult;
    int guardRejectsNonContract;
    int guardRejectsPanelClosed;
    int guardRejectsNoCandidate;
    int assertionsRepresented;
    uint32_t beforeHash;
    uint32_t afterCloseHash;
    uint32_t afterFirstShuffleHash;
    uint32_t afterSecondShuffleHash;
    uint32_t hash;
} Dm1V1MirrorCandidateCloseOrderPartyShuffleResultPc34;

Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34
dm1_v1_mirror_candidate_close_order_party_shuffle_default_state_pc34(void);

int dm1_v1_mirror_candidate_close_order_party_shuffle_run_pc34(
    Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state,
    Dm1V1MirrorCandidateCloseOrderPartyShuffleResultPc34 *result);

uint32_t dm1_v1_mirror_candidate_close_order_party_shuffle_hash_pc34(
    const Dm1V1MirrorCandidateCloseOrderPartyShuffleStatePc34 *state);

const char *
dm1_v1_mirror_candidate_close_order_party_shuffle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
