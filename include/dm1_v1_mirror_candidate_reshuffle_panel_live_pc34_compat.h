#ifndef DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PANEL_LIVE_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PANEL_LIVE_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB anchors used by this contract-only runtime regression:
 * CHEST.C F0333:30-67 opens G0426 into visible C30..C37/G0425 chest
 * slots, and F0334:117-132 closes/relinks them; CHAMPION.C F0284:93-130
 * mutates G0305 party order on direction changes, F0287:243-268 resolves
 * candidate find/insert identity, F0297:243-268 and F0298:270-298 own the
 * leader hand, and F0300:511-584, F0301:606-660, F0302:662-713 own slot
 * operations; COMMAND.C F0378:1973-1983 and F0380:2045-2159 preserve
 * panel/queue identity; REVIVE.C F0280:124-132 opens G0299 and
 * F0282:744-806 clears it on C162; PANEL.C F0344/F0345 route panel input
 * and F0346/F0347:1619-1657 redraw C040 while G0299 is set; UTAMSCR.C
 * F0077/F0078:141-150 bracket redraw updates; DEFS.H:338-340, 810-817,
 * 1874-1878, 2085-2088, 2088-2096, 2200, 3001-3008, 5694, and 5876-5881
 * name C162, C30..C37, C38, G0305, G0423/G0425/G0426, C040, M568/M569,
 * and G0299.
 * PASS dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat
 */

#define DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_SLOT_COUNT_PC34_COMPAT 8
#define DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_PARTY_COUNT_PC34_COMPAT 4

enum {
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_NONE_PC34_COMPAT = 0xFFFF,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C162_CANCEL_PC34_COMPAT = 162,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C30_PC34_COMPAT = 30,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C37_PC34_COMPAT = 37,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C38_PC34_COMPAT = 38,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_C040_PANEL_PC34_COMPAT = 40,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M568_PANEL_PC34_COMPAT = 5,
    DM1_V1_MIRROR_CANDIDATE_RESHUFFLE_M569_PANEL_PC34_COMPAT = 4
};

typedef struct Dm1V1MirrorCandidateReshuffleContractPc34Compat {
    int contractOnly;
    int seed;
    int partyCount;
    int candidateOrdinal;
    int c040PanelGraphic;
    int candidatePanelId;
    int chestPanelId;
    int cancelCommand;
    int firstChestSlotId;
    int lastChestSlotId;
    int firstChestSlotBox;
    int openChestThing;
    int leaderHandThing;
    const char *sourceAnchors;
    const char *scope;
} Dm1V1MirrorCandidateReshuffleContractPc34Compat;

typedef struct Dm1V1MirrorCandidateReshuffleSelfTestStatsPc34Compat {
    int assertions;
    int failures;
} Dm1V1MirrorCandidateReshuffleSelfTestStatsPc34Compat;

const Dm1V1MirrorCandidateReshuffleContractPc34Compat *
DM1_V1_MirrorCandidateReshufflePanelLive_ContractPc34Compat(void);

int run_dm1_v1_mirror_candidate_reshuffle_panel_live_pc34_compat_self_test(
    void);

Dm1V1MirrorCandidateReshuffleSelfTestStatsPc34Compat
DM1_V1_MirrorCandidateReshufflePanelLive_LastSelfTestStatsPc34Compat(void);

#ifdef __cplusplus
}
#endif

#endif
