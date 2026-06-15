#ifndef DM1_V1_MIRROR_CANDIDATE_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_PC34_COMPAT_H
#define DM1_V1_MIRROR_CANDIDATE_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Contract-only DM1 V1 regression:
 * ReDMCSB anchors used: CHEST.C F0333:30-67 and F0334:117-132 for
 * G0426/G0425 open/close ownership; CHAMPION.C F0284:93-130 for G0305 party,
 * F0287:243-268 for candidate find/insert, F0297:243-268 and F0298:270-298
 * for leader hand, and F0300:511-584/F0301:606-660/F0302:662-713 for C30+
 * slot operations; COMMAND.C F0378:1973-1983 and F0380:2045-2159 for panel
 * dispatch and queue identity; REVIVE.C F0280:124-132 and F0282:744-806 for
 * G0299/C040 candidate lifetime; PANEL.C F0344/F0345, F0346/F0347:1619-1657,
 * and F0354:2307-2344 for panel click, C040 redraw, and close/redraw;
 * UTAMSCR.C F0077:147-151/F0078:141-145 for mouth-route/pointer bracket;
 * OBJECT.C F0033:147-212 and BLITMASK.C F0133:30-33 for icon/mask redraw;
 * DEFS.H:338-340, 810-817, 1874-1878, 2085-2088, 2088-2096, 2200,
 * 3001-3008, 3906-3913, 5694, and 5876-5881 for C162, C30..C37, C38,
 * G0305, G0423/G0425/G0426, C040, M568/M569, C537..C544, and G0299.
 *
 * PASS test_dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat
 */

#define DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_SLOT_COUNT_PC34_COMPAT 8

enum {
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_NONE_PC34_COMPAT = 0,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C040_PC34_COMPAT = 40,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C162_PC34_COMPAT = 162,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C30_FIRST_PC34_COMPAT = 30,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C38_FIRST_PC34_COMPAT = 38,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C537_FIRST_PC34_COMPAT = 537,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_C544_LAST_PC34_COMPAT = 544,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M568_CANDIDATE_PC34_COMPAT = 5,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_M569_CHEST_PC34_COMPAT = 4,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_MOUTH_PC34_COMPAT = 1,
    DM1_V1_C040_CLOSE_NON_LEADER_SCROLL_PICKUP_ROUTE_PANEL_CLICK_PC34_COMPAT = 2
};

typedef struct Dm1V1C040CloseNonLeaderScrollPickupEvidencePc34Compat {
    int contractOnly;
    const char *chestOpenAnchor;
    const char *chestCloseAnchor;
    const char *partyAnchor;
    const char *candidateFindInsertAnchor;
    const char *leaderHandPutAnchor;
    const char *leaderHandRemoveAnchor;
    const char *slotRemoveAnchor;
    const char *slotAddAnchor;
    const char *slotDispatchAnchor;
    const char *panelDispatchAnchor;
    const char *queueAnchor;
    const char *candidateOpenAnchor;
    const char *candidateCleanupAnchor;
    const char *panelClickAnchor;
    const char *panelC040RedrawAnchor;
    const char *panelCloseRedrawAnchor;
    const char *mouseRouteAnchor;
    const char *objectAnchor;
    const char *blitMaskAnchor;
    const char *defsAnchor;
    const char *nonDuplicationScope;
} Dm1V1C040CloseNonLeaderScrollPickupEvidencePc34Compat;

const Dm1V1C040CloseNonLeaderScrollPickupEvidencePc34Compat *
dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_evidence_pc34_compat(
    void);

const char *
dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_source_evidence_pc34_compat(
    void);

int run_dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_pc34_compat_self_test(
    void);

int dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_assertions_pc34_compat(
    void);

int dm1_v1_mirror_candidate_c040_close_non_leader_scroll_pickup_failures_pc34_compat(
    void);

#ifdef __cplusplus
}
#endif

#endif
