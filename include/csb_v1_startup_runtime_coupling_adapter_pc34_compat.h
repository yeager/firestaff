#ifndef FIRESTAFF_CSB_V1_STARTUP_RUNTIME_COUPLING_ADAPTER_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_RUNTIME_COUPLING_ADAPTER_PC34_COMPAT_H

#include "csb_v1_f0437_f0438_f0580_f0581_startup_runtime_coupling_pc34_compat.h"
#include "csb_v1_startup_session_contract_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Builds the ReDMCSB F0437/F0438/F0580/F0581 input only from the verified
 * CSB startup session and its complete-support receipt.  This keeps M11 from
 * supplying synthetic title, door, or HUD readiness flags. */
int csb_v1_startup_runtime_coupling_facts_from_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupRuntimeCouplingFacts_PC34 *out_facts);

/* Consumes the same receipt-owned data through the source-named title and
 * entrance gates.  A rejected source gate leaves the aggregate invalid. */
typedef struct CSB_V1_StartupRuntimeCouplingSessionReceipt_PC34 {
    int valid;
    int facts_from_verified_session;
    int title_consumed;
    int entrance_consumed;
    int door_step_consumed;
    int door_blit_consumed;
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 title;
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 entrance;
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 door_step;
    CSB_V1_StartupRuntimeCouplingReceipt_PC34 door_blit;
    const char *source_evidence;
} CSB_V1_StartupRuntimeCouplingSessionReceipt_PC34;

typedef enum CSB_V1_StartupTitleRuntimePhase_PC34 {
    CSB_V1_STARTUP_TITLE_RUNTIME_PRESENTS_PC34 = 0,
    CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_ZOOM_PC34,
    CSB_V1_STARTUP_TITLE_RUNTIME_CHAOS_HOLD_PC34,
    CSB_V1_STARTUP_TITLE_RUNTIME_STRIKES_PC34
} CSB_V1_StartupTitleRuntimePhase_PC34;

/* A source-owned C001 lifecycle sample.  `palette_identity` is supplied by
 * the same real title plan that supplied the capture, never by a host draw. */
typedef struct CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 {
    int valid;
    int real_asset_matched;
    int no_fallback_routes;
    CSB_V1_StartupTitleRuntimePhase_PC34 phase;
    unsigned int source_tick;
    unsigned int session_generation;
    uint32_t palette_identity;
    uint32_t capture_identity;
} CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34;

int csb_v1_startup_title_runtime_lifecycle_advance_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupTitleRuntimePhase_PC34 phase,
    unsigned int source_tick,
    uint32_t palette_identity,
    const CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *previous,
    CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *out_receipt);

int csb_v1_startup_title_runtime_lifecycle_from_host_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    const CSB_V1_StartupRuntimeHostSurfaceReceipt_PC34 *host_receipt,
    const CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *previous,
    CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *out_receipt);

typedef struct CSB_V1_StartupTitleTerminalLifecycleReceipt_PC34 {
    int valid;
    int title_lifecycle_bound;
    int terminal_session_bound;
    int no_fallback_routes;
    unsigned int title_source_tick;
    unsigned int terminal_source_tick;
    unsigned int session_generation;
    uint32_t strikes_palette_identity;
    uint32_t strikes_capture_identity;
} CSB_V1_StartupTitleTerminalLifecycleReceipt_PC34;

int csb_v1_startup_title_terminal_lifecycle_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    const CSB_V1_StartupTitleRuntimeLifecycleReceipt_PC34 *title_receipt,
    unsigned int terminal_source_tick,
    CSB_V1_StartupTitleTerminalLifecycleReceipt_PC34 *out_receipt);

typedef struct CSB_V1_StartupReleaseLifecycleReceipt_PC34 {
    int valid;
    int complete_support_bound;
    int title_phases_bound;
    int no_fallback_routes;
    uint32_t session_generation;
    uint32_t capture_tick;
    uint32_t complete_support_hash;
    uint32_t title_phase_hash;
    uint32_t title_phase_set_hash;
    uint32_t runtime_route_hash;
    uint32_t release_capture_hash;
} CSB_V1_StartupReleaseLifecycleReceipt_PC34;

int csb_v1_startup_release_lifecycle_advance_pc34(
    const CSB_V1_StartupReleaseAppCaptureReceipt_PC34 *release_capture,
    uint32_t capture_tick,
    const CSB_V1_StartupReleaseLifecycleReceipt_PC34 *previous,
    CSB_V1_StartupReleaseLifecycleReceipt_PC34 *out_receipt);

int csb_v1_startup_runtime_coupling_consume_session_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    const CSB_V1_StartupCompleteSupportReceipt_PC34 *complete_support,
    CSB_V1_StartupRuntimeCouplingSessionReceipt_PC34 *out_receipt);

const char *csb_v1_startup_runtime_coupling_adapter_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_STARTUP_RUNTIME_COUPLING_ADAPTER_PC34_COMPAT_H */
