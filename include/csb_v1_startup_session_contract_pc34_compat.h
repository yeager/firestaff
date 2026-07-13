#ifndef FIRESTAFF_CSB_V1_STARTUP_SESSION_CONTRACT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_STARTUP_SESSION_CONTRACT_PC34_COMPAT_H

#include "csb_v1_boot.h"

typedef struct CSB_V1_StartupSessionTerminalReceipt_PC34 {
    int valid;
    int c001_complete;
    int terminal_f0807_complete;
    int c017_ready;
    int c040_ready;
    unsigned int source_tick;
    unsigned int session_generation;
} CSB_V1_StartupSessionTerminalReceipt_PC34;

/* ReDMCSB TITLE.C F0437, ENTRANCE.C F0807, PANEL.C F0347. */
int csb_v1_startup_session_terminal_receipt_pc34(
    const CSB_V1_StartupRuntimeAssetSession_PC34 *session,
    CSB_V1_StartupSessionTerminalReceipt_PC34 *out_receipt);

#endif
