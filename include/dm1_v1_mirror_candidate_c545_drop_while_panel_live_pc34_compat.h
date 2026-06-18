/*
 * dm1_v1_mirror_candidate_c545_drop_while_panel_live_pc34_compat.h
 *
 * Public surface for the DM1 V1 mirror-candidate C545 drop-while-panel-live
 * (leader-hand drop-to-floor while a C040 candidate panel is live) contract
 * regression.
 *
 * Source-locked against (ReDMCSB):
 *   CHEST.C    F0334:117-132    clears G0426 and relinks non-empty G0425 slots
 *   CHAMPION.C F0297:243-268    seeds the post-candidate leader-hand object
 *   CHAMPION.C F0298:270-298    removes it for the drop-to-floor mutation
 *   COMMAND.C  F0378:1973-1983  dispatches panel input
 *   COMMAND.C  F0380:2045-2159  keeps queued command identity stable until
 *                               the C545 mutation runs
 *   REVIVE.C   F0280:124-132    publishes G0299 only while the hand is empty
 *   REVIVE.C   F0282:744-806    clears G0299 on the later C040 click
 *   PANEL.C    F0346/F0347:1619-1657 keeps C040 drawn while G0299 is non-zero
 *   UTAMSCR.C  F0077/F0078:141-150 brackets pointer redraws
 *   BLITMASK.C F0133:30-33      anchors the masked redraw path used by the
 *                               live panel
 */

#ifndef DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1MirrorC545DropEvidencePc34Compat {
    int contractOnly;
    const char *chestCloseAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *commandDispatchAnchor;
    const char *commandQueueAnchor;
    const char *candidateOpenAnchor;
    const char *candidateClearAnchor;
    const char *panelDrawAnchor;
    const char *mouseBracketAnchor;
    const char *blitmaskAnchor;
    const char *defsAnchor;
    const char *nonOverlap;
} Dm1V1MirrorC545DropEvidencePc34Compat;

typedef struct Dm1V1MirrorC545DropStatePc34Compat {
    int partyChampionCount;
    int activeChampionIndex;
    int chestSlots[8];
    int leaderHandThing;
    int previousCellThing;
    int openChestThing;
    int firstChestSlotThing;
    int candidateOrdinal;
    int c545Fired;
    int c040FiredAfterC545;
    int chestSlotsPreserved;
    int leaderHandReleasedAfterMutation;
} Dm1V1MirrorC545DropStatePc34Compat;

const Dm1V1MirrorC545DropEvidencePc34Compat *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_EvidencePc34Compat(void);

const char *
DM1_V1_MirrorCandidateC545DropWhilePanelLive_SourceEvidencePc34Compat(void);

void
DM1_V1_MirrorCandidateC545DropWhilePanelLive_InitPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state);

int
DM1_V1_MirrorCandidateC545DropWhilePanelLive_RunPc34Compat(
    Dm1V1MirrorC545DropStatePc34Compat *state,
    int sequenceIndex);

#ifdef __cplusplus
}
#endif

#endif /* DM1_V1_MIRROR_CANDIDATE_C545_DROP_WHILE_PANEL_LIVE_PC34_COMPAT_H */