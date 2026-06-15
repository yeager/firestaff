#ifndef FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_RESURRECT_CONFIRMATION_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHEST_SCROLL_WHEEL_RESURRECT_CONFIRMATION_PC34_COMPAT_H

/*
 * DM1 V1 contract-only chest scroll-wheel pickup gate for C040 resurrect
 * confirmation plus a different non-leader open G0426 chest.
 *
 * ReDMCSB source-lock anchors:
 * - REVIVE.C F0282:744-806: C160/C161/C162 confirmation flow owns the
 *   leader hand and clears G0299 only when the candidate command is consumed.
 * - REVIVE.C F0280:124-132 and F0281: candidate publish/state setup.
 * - CHEST.C F0333:30-67 and F0334:117-132: G0426 open plus G0425 close and
 *   recompaction.
 * - CHAMPION.C F0297:243-298, CHAMPION.C F0298:270-298,
 *   CHAMPION.C F0300:511-584, CHAMPION.C F0301:606-660, and
 *   CHAMPION.C F0302:662-713: leader-hand and C30+ exchange.
 * - COMMAND.C F0359:1985-1990, COMMAND.C F0378:1973-1983, and
 *   COMMAND.C F0380:2045-2159: empty-hand gated C040 dispatch, scroll pickup
 *   routing, and queue drain.
 * - PANEL.C F0344/F0345 and F0346/F0347:1619-1657: panel click/redraw.
 * - UTAMSCR.C F0077/F0078:141-150: pointer update bracket for the wheel path.
 * - DEFS.H:338-340, 810-817, 873/876, 1878, 2088, 2200, 3001-3008,
 *   3906-3913, 4205-4207, 5694, and 5876-5881: commands, C30..C37,
 *   M516/M070, C10, C040, M568/M569, chest zones, floor zones, G0299,
 *   G0423/G0425/G0426.
 *
 * Non-overlap siblings: dm1_v1_mirror_candidate_c545_pickup/drop_while_panel
 * live, dm1_v1_mirror_candidate_scroll_pickup_non_leader_panel_live, and
 * dm1_v1_mirror_candidate_resurrect_reselect_with_inventory_pickup.
 */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_CSWRC_CHEST_SLOT_COUNT_PC34 = 8,
    DM1_V1_CSWRC_C30_CHAIN_COUNT_PC34 = 8,
    DM1_V1_CSWRC_CHAMPION_COUNT_PC34 = 4,
    DM1_V1_CSWRC_NONE_PC34 = 0xffff,
    DM1_V1_CSWRC_C160_RESURRECT_PC34 = 160,
    DM1_V1_CSWRC_C161_REINCARNATE_PC34 = 161,
    DM1_V1_CSWRC_C162_CANCEL_PC34 = 162,
    DM1_V1_CSWRC_C30_PC34 = 30,
    DM1_V1_CSWRC_C37_PC34 = 37,
    DM1_V1_CSWRC_C040_PANEL_PC34 = 40,
    DM1_V1_CSWRC_M568_PANEL_PC34 = 5,
    DM1_V1_CSWRC_M569_PANEL_PC34 = 4,
    DM1_V1_CSWRC_C538_ZONE_PC34 = 538
};

typedef struct {
    int assertions;
    int failures;
    unsigned int deterministic_hash;
    int positive_rejections;
    int negative_browse_allowed;
    int negative_cancelled_allowed;
    int leader_hand_unchanged_checks;
    int c30_chain_unchanged_checks;
    int g0426_chain_unchanged_checks;
    int candidate_command_unchanged_checks;
    int scroll_events_reached_f0077;
    int scroll_events_reached_f0078;
    int scroll_events_queued_to_f0378;
    int queue_dispatches_f0380;
    int f0282_gate_rejections;
    int f0301_reached_when_allowed;
    int f0302_reached_when_allowed;
} Dm1V1ChestScrollWheelResurrectConfirmationResultPc34;

int run_dm1_v1_chest_scroll_wheel_resurrect_confirmation_self_test(void);

const Dm1V1ChestScrollWheelResurrectConfirmationResultPc34 *
dm1_v1_chest_scroll_wheel_resurrect_confirmation_last_self_test_result_pc34(
    void);

#ifdef __cplusplus
}
#endif

#endif
