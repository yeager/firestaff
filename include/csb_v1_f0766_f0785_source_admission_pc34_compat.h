#ifndef FIRESTAFF_CSB_V1_F0766_F0785_SOURCE_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0766_F0785_SOURCE_ADMISSION_PC34_COMPAT_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdint.h>

typedef struct {
    int valid;
    uint16_t function_id;
    uint32_t source_entry_index;
    uint32_t source_entry_count;
    int authentic_package_required;
    int existing_owner_required;
    int runtime_execution_blocked;
    const char *source_evidence;
} CSB_V1_F0766F0785SourceReceiptPc34;

/* Read-only original-material admission for CSB F0766-F0785. */
int csb_v1_f0766_f0785_source_admit_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    uint16_t function_id,
    CSB_V1_F0766F0785SourceReceiptPc34 *out_receipt);

#endif
