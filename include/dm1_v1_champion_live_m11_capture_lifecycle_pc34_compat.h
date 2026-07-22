#ifndef FIRESTAFF_DM1_V1_CHAMPION_LIVE_M11_CAPTURE_LIFECYCLE_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_CHAMPION_LIVE_M11_CAPTURE_LIFECYCLE_PC34_COMPAT_H

#include "dm1_v1_champion_live_m11_bridge_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 {
    unsigned int lastTick;
    unsigned int lastGeneration;
} Dm1V1ChampionLiveM11CaptureLifecycleStatePc34;

typedef struct Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 {
    int valid;
    int clearOnly;
    unsigned int tick;
    unsigned int generation;
    int commandCount;
    Dm1V1ChampionRuntimeSourceM11CommandPc34
        commands[DM1_V1_CHAMPION_TOP_ROW_ATOMIC_FRAME_MAX_OPS_PC34];
} Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34;

void dm1_v1_champion_live_m11_capture_lifecycle_init_pc34(
    Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 *state);

/* Cross-tick runtime capture gate for live champion frames. A full M11 bridge
 * receipt must match its original capture evidence; stale or mismatched frames
 * are converted to material-free clears for only their own command zones. */
int dm1_v1_champion_live_m11_capture_lifecycle_step_pc34(
    Dm1V1ChampionLiveM11CaptureLifecycleStatePc34 *state,
    const Dm1V1ChampionLiveM11BridgeReceiptPc34 *liveBridge,
    const Dm1V1ChampionLiveRuntimeCompositionReceiptPc34 *captureEvidence,
    Dm1V1ChampionLiveM11CaptureLifecycleReceiptPc34 *outReceipt);

const char *dm1_v1_champion_live_m11_capture_lifecycle_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
