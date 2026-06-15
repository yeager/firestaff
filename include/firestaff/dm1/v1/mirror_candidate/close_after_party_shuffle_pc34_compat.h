#ifndef FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_AFTER_PARTY_SHUFFLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_MIRROR_CANDIDATE_CLOSE_AFTER_PARTY_SHUFFLE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Lane: DM1 V1 mirror-candidate close after party shuffle
 * (pass783_dm1_v1_mirror_candidate_close_after_party_shuffle).
 *
 * Pins the source-locked contract for a C160/C161/C162 close click that
 * lands AFTER two F0284 party-direction rotations while a mirror
 * candidate (C040) is live. The gate is intentionally
 * non-duplicative with:
 *   - pass780 resurrect + chest-close order (no F0284 rotation)
 *   - dm1_v1_mirror_candidate_party_direction (5 turns + status click,
 *     no C160 close)
 *   - dm1_v1_mirror_candidate_reselect_after_deposit_with_party_rotate
 *     (close first, rotate after, reopen)
 *   - dm1_v1_mirror_candidate_reshuffle_panel_live (slot reorder,
 *     not direction rotation, via close_candidate_panel)
 *   - dm1_v1_mirror_candidate_close_button (close click without F0284)
 *   - dm1_v1_mirror_candidate_c040_chrome_inventory_owner_swap
 *     (single F0284 step in the same _run call, not two F0284 calls
 *     followed by C160 close)
 *
 * ReDMCSB anchors (WIP20210206, PC 3.4 path):
 *   - CHAMPION.C:93-130 F0284_CHAMPION_SetPartyDirection rotates the
 *     per-champion Cell/Direction by a delta, updates G0308, and calls
 *     F0296_CHAMPION_DrawChangedObjectIcons.
 *   - CHAMPION.C F0296 draws the per-champion portrait boxes that the
 *     C038 panel priority byte and C037 status hand box redraw around
 *     while a mirror candidate is live.
 *   - REVIVE.C F0282:744-806 reads the appended candidate as
 *     M516_CHAMPIONS[G0305_ui_PartyChampionCount - 1] and clears
 *     G0299 / decrements G0305 on the C160/C161/C162 click path.
 *   - COMMAND.C F0361:1709-1813 queues the keyboard turn input.
 *   - COMMAND.C F0359:1452-1662 queues the C040 panel Yes click.
 *   - COMMAND.C F0380:2045-2156 drains one command at a time.
 *   - DEFS.H anchors: C040, C037/C038, C159, C160..C162, M070, M516.
 */

#define DM1_V1_MC_CAPS_PARTY_COUNT_PC34 3
#define DM1_V1_MC_CAPS_DIRECTION_COUNT_PC34 4
#define DM1_V1_MC_CAPS_COMMAND_COUNT_PC34 6
#define DM1_V1_MC_CAPS_TRACE_COUNT_PC34 14
#define DM1_V1_MC_CAPS_NONE_PC34 0xffffu

typedef enum {
    DM1_V1_MC_CAPS_COMMAND_NONE_PC34 = 0,
    DM1_V1_MC_CAPS_COMMAND_TURN_RIGHT_PC34 = 1,
    DM1_V1_MC_CAPS_COMMAND_TURN_LEFT_PC34 = 2,
    DM1_V1_MC_CAPS_COMMAND_C160_YES_PC34 = 160
} Dm1V1MirrorCandidateCloseAfterPartyShuffleCommandPc34;

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
    int championCell[DM1_V1_MC_CAPS_PARTY_COUNT_PC34];
    int championDirection[DM1_V1_MC_CAPS_PARTY_COUNT_PC34];
    int c040PanelOpen;
    int c040PanelClosed;
    int c038PanelPriorityByte;
    int c037StatusHandBoxByte;
    int c159ChampionIconByte;
    int m568PanelResurrectReincarnate;
    int m070PanelOwnerOrdinal;
    Dm1V1MirrorCandidateCloseAfterPartyShuffleCommandPc34
        queuedCommands[DM1_V1_MC_CAPS_COMMAND_COUNT_PC34];
    Dm1V1MirrorCandidateCloseAfterPartyShuffleCommandPc34
        dispatchOrder[DM1_V1_MC_CAPS_COMMAND_COUNT_PC34];
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
    int f0282ReadsPostShuffleCandidate;
    int c159ChampionIconStable;
    int c038PanelPriorityStable;
    int c037StatusHandBoxStable;
    int m070PanelOwnerStable;
    int m568PanelContentStable;
    int directionPreservedAfterClose;
    int cellPreservedAfterClose;
    int trace[DM1_V1_MC_CAPS_TRACE_COUNT_PC34];
} Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34;

typedef struct {
    int shuffledFirst;
    int shuffledSecond;
    int f0284FiredTwice;
    int f0284FirstDeltaCorrect;
    int f0284SecondDeltaCorrect;
    int f0296CalledTwice;
    int g0308EastAfterFirst;
    int g0308SouthAfterSecond;
    int panelStayedOpenThroughShuffle;
    int yesAcceptedAfterShuffle;
    int f0282FiredOnPostShuffleParty;
    int g0299ClearedAfterShuffle;
    int g0305DecrementedAfterShuffle;
    int candidateIndexByteStable;
    int c040PanelClosedAfterShuffle;
    int c038PanelPriorityPreserved;
    int c037StatusHandBoxPreserved;
    int c159ChampionIconPreserved;
    int m070PanelOwnerPreserved;
    int m568PanelContentPreserved;
    int g0308SouthAfterClose;
    int championCellsAfterClose;
    int championDirectionsAfterClose;
    int cellPreservedAfterClose;
    int directionPreservedAfterClose;
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
    uint32_t afterFirstShuffleHash;
    uint32_t afterSecondShuffleHash;
    uint32_t afterYesHash;
    uint32_t afterCloseHash;
    uint32_t hash;
} Dm1V1MirrorCandidateCloseAfterPartyShuffleResultPc34;

Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34
dm1_v1_mirror_candidate_close_after_party_shuffle_default_state_pc34(void);

int dm1_v1_mirror_candidate_close_after_party_shuffle_run_pc34(
    Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state,
    Dm1V1MirrorCandidateCloseAfterPartyShuffleResultPc34 *result);

uint32_t dm1_v1_mirror_candidate_close_after_party_shuffle_hash_pc34(
    const Dm1V1MirrorCandidateCloseAfterPartyShuffleStatePc34 *state);

const char *
dm1_v1_mirror_candidate_close_after_party_shuffle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
