#ifndef FIRESTAFF_CSB_V1_F0806_F0825_STARTUP_SOURCE_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0806_F0825_STARTUP_SOURCE_ADMISSION_PC34_COMPAT_H

#include "csb_v1_f0806_entrance_loop_runtime_handoff_pc34_compat.h"
#include "csb_v1_startup_real_asset_receipt.h"

#include <stdint.h>

typedef struct {
    int valid;
    uint16_t function_id;
    int authentic_package_consumed;
    int existing_runtime_owner_required;
    int runtime_execution_blocked;
    uint32_t source_tick;
    uint32_t session_generation;
    uint64_t package_receipt_hash;
    const char *source_evidence;
} CSB_V1_F0806F0825StartupSourceReceiptPc34;

/* Read-only admission for F0806-F0825 CSB startup/material routes. */
int csb_v1_f0806_f0825_startup_source_admit_pc34(
    const CSB_V1_StartupRealPackageConsumptionReceipt_PC34 *package_receipt,
    const CSB_V1_F0806_EntranceLoopReceipt_PC34 *entrance_receipt,
    const CSB_V1_F0807_EntranceAnimationStepReceipt_PC34 *door_receipt,
    uint16_t function_id,
    CSB_V1_F0806F0825StartupSourceReceiptPc34 *out_receipt);

#endif
