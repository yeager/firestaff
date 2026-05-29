#ifndef FIRESTAFF_CSB_V1_UTILITY_FLOW_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_UTILITY_FLOW_PC34_COMPAT_H

/*
 * CSB V1 Phase 6 — Utility Setup Flow
 *
 * CSB utility disk setup menu flow.
 * Different from DM1's utility flow — CSB has different setup options:
 *   - Import champions from DM1 saves (vs DM1's champion creation)
 *   - Different party size rules (CSB allows 1-4, same as DM1 but different UI)
 *   - No champion creation hall (replaced by import UI)
 *   - Resurrect/reincarnate are always available from the champion panel
 *
 * Source references:
 *   ReDMCSB ENTRANCE.C — setup/selector flow adapted for CSB
 *   ReDMCSB CEDTINC7.C — utility disk prompts
 *   ReDMCSB CEDTDATA.C — G3921/G3755/G3764 utility disk strings
 *   CSBWin/CSBCode.cpp — StartChaos setup (11414 lines)
 */

/* ── Utility flow states ─────────────────────────────────────────────── */
typedef enum {
    CSB_V1_UTIL_FLOW_INIT               = 0,
    CSB_V1_UTIL_FLOW_INSERT_DISK        = 1,  /* prompt for utility disk */
    CSB_V1_UTIL_FLOW_VERIFY_DISK        = 2,  /* check disk type */
    CSB_V1_UTIL_FLOW_DISK_OK            = 3,  /* disk verified */
    CSB_V1_UTIL_FLOW_SELECT_ACTION      = 4,  /* main menu: import/load/etc */
    CSB_V1_UTIL_FLOW_IMPORT_CHAMPIONS   = 5,  /* DM1 save → CSB import */
    CSB_V1_UTIL_FLOW_CONFIRM_IMPORT     = 6,  /* confirm import before writing */
    CSB_V1_UTIL_FLOW_LOAD_GAME          = 7,  /* load existing CSB save */
    CSB_V1_UTIL_FLOW_NEW_GAME           = 8,  /* start new CSB game */
    CSB_V1_UTIL_FLOW_ERROR              = 9,
    CSB_V1_UTIL_FLOW_DONE               = 10
} CSB_V1_UtilFlowState;

/* ── Utility flow action types ─────────────────────────────────────── */
typedef enum {
    CSB_V1_UTIL_ACTION_IMPORT   = 0,  /* Import champions from DM1 save */
    CSB_V1_UTIL_ACTION_LOAD     = 1,  /* Load saved game */
    CSB_V1_UTIL_ACTION_NEW      = 2,  /* New game (DM1 import required first) */
    CSB_V1_UTIL_ACTION_VIEW    = 3,  /* View champion details */
    CSB_V1_UTIL_ACTION_EXIT     = 4   /* Exit utility flow */
} CSB_V1_UtilFlowAction;

/* ── Disk verification result ──────────────────────────────────────── */
typedef enum {
    CSB_V1_UTIL_DISK_OK          = 0,
    CSB_V1_UTIL_DISK_MISSING     = 1,  /* no disk in drive */
    CSB_V1_UTIL_DISK_WRONG       = 2,  /* not the CSB utility disk */
    CSB_V1_UTIL_DISK_UNREADABLE  = 3,  /* disk read error */
    CSB_V1_UTIL_DISK_CORRUPTED   = 4   /* disk data corrupted */
} CSB_V1_UtilDiskResult;

/* ── Utility flow context ───────────────────────────────────────────── */
typedef struct {
    CSB_V1_UtilFlowState state;        /* current flow state */
    CSB_V1_UtilFlowAction action;      /* selected action */
    CSB_V1_UtilDiskResult disk_result;  /* last disk check result */
    int                  attempts;     /* disk check attempts */
    int                  max_attempts; /* max disk check attempts (default 5) */
    char                 dm1_save_path[256]; /* path to DM1 save for import */
    char                 csb_save_path[256]; /* path to CSB save */
    int                  import_confirmed; /* import confirmed by user */
    int                  last_error;    /* last error code */
    int                  reserved[8];   /* future expansion */
} CSB_V1_UtilFlowContext;

/* ── Initialize utility flow ────────────────────────────────────────── */
void csb_v1_util_flow_init(CSB_V1_UtilFlowContext *ctx);

/* ── Run one step of the utility flow ──────────────────────────────── */
/* Returns: 0 = running (not done), 1 = done, negative = error.
 * On error, ctx->last_error is set and state = CSB_V1_UTIL_FLOW_ERROR. */
int csb_v1_util_flow_step(CSB_V1_UtilFlowContext *ctx);

/* ── Set the current action ─────────────────────────────────────────── */
void csb_v1_util_flow_set_action(CSB_V1_UtilFlowContext *ctx,
                                   CSB_V1_UtilFlowAction action);

/* ── Set DM1 save path for import ────────────────────────────────────── */
void csb_v1_util_flow_set_dm1_path(CSB_V1_UtilFlowContext *ctx,
                                     const char *path);

/* ── Set CSB save path for load ─────────────────────────────────────── */
void csb_v1_util_flow_set_csb_path(CSB_V1_UtilFlowContext *ctx,
                                     const char *path);

/* ── Confirm import (after showing preview) ────────────────────────── */
void csb_v1_util_flow_confirm_import(CSB_V1_UtilFlowContext *ctx,
                                       int confirmed);

/* ── Get current state name for UI ──────────────────────────────────── */
const char *csb_v1_util_flow_state_name(CSB_V1_UtilFlowState state);

/* ── Get last error string ──────────────────────────────────────────── */
const char *csb_v1_util_flow_last_error(CSB_V1_UtilFlowContext *ctx);

/* ── Utility flow ready check ────────────────────────────────────────── */
/* Returns 1 if the flow is ready to start a game (import done or save loaded). */
int csb_v1_util_flow_is_ready(const CSB_V1_UtilFlowContext *ctx);

/* ── Source evidence ──────────────────────────────────────────────────── */
const char *csb_v1_utility_flow_source_evidence(void);

#endif /* FIRESTAFF_CSB_V1_UTILITY_FLOW_PC34_COMPAT_H */