#ifndef DM1_V1_STARTUP_HANDOFF_M11_BRIDGE_PC34_COMPAT_H
#define DM1_V1_STARTUP_HANDOFF_M11_BRIDGE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* M11 bridge for the DM1 startup handoff callbacks.
 *
 * ReDMCSB APPA.C/STARTUP2.C fires three phases in order:
 *   1. SWSH (swoosh) — F0908/F0909/F0910 PSG sound over FTL logo
 *   2. Title (C001) — F0437 zoom animation (53 frames at 55ms cadence)
 *   3. Entrance — F0441 palette crossfade, wait for Enter
 *
 * M11 previously skipped the startup prelude and jumped straight to the
 * game view. This bridge provides the callback implementations that
 * dm1_v1_startup_execute_handoff_prelude_pc34 and
 * dm1_v1_startup_execute_handoff_post_launch_pc34 invoke.
 */

typedef struct {
    void *m11State;
    int swooshBound;
    int swooshPlayed;
    int titlePlayed;
    int entranceCompleted;
    int entranceCommand;
    int windowRaised;
    int preludePlanReceived;
    int postLaunchPlanReceived;
    uint32_t swooshSourceFnv1a;
} DM1_V1_StartupHandoffM11BridgeStatePc34;

typedef struct {
    int valid;
    int preludeExecuted;
    int postLaunchExecuted;
    int swooshPlayed;
    int titlePlayed;
    int entranceCompleted;
    int entranceCommand;
    const char *sourceEvidence;
} DM1_V1_StartupHandoffM11BridgeReceiptPc34;

/* Initialize the bridge state before first use. */
int dm1_v1_startup_handoff_m11_bridge_init_pc34(
    DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    void *m11State);

/* Execute the prelude (SWSH) phase through M11.
 * Returns 1 on success (prelude completed or not required), 0 on error. */
int dm1_v1_startup_handoff_m11_bridge_execute_prelude_pc34(
    DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    const char *gameId);

/* Execute the post-launch (title + entrance) phase through M11.
 * Returns 1 on success, 0 on error. */
int dm1_v1_startup_handoff_m11_bridge_execute_post_launch_pc34(
    DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    const char *sourceId);

/* Build a receipt summarizing what the bridge executed. */
int dm1_v1_startup_handoff_m11_bridge_receipt_pc34(
    const DM1_V1_StartupHandoffM11BridgeStatePc34 *state,
    DM1_V1_StartupHandoffM11BridgeReceiptPc34 *outReceipt);

const char *dm1_v1_startup_handoff_m11_bridge_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
