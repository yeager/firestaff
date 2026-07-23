#ifndef FIRESTAFF_CSB_V1_F0706_F0725_PACKAGE_ADMISSION_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0706_F0725_PACKAGE_ADMISSION_PC34_COMPAT_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_runtime_plan.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdint.h>

typedef struct {
    int valid;
    uint16_t function_id;
    uint32_t graphics_entry_index;
    uint32_t graphics_entry_count;
    int graphics_package_admitted;
    int existing_owner_required;
    int runtime_execution_blocked;
    int source_has_no_callable_pc34_package_route;
    const char *source_evidence;
} CSB_V1_F0706F0725PackageReceiptPc34;

/* Read-only package admission for the F0706-F0725 PC 3.4 family. */
int csb_v1_f0706_f0725_package_admit_pc34(
    const CSB_V1_RuntimeProfile *profile,
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    uint16_t function_id,
    CSB_V1_F0706F0725PackageReceiptPc34 *out_receipt);

#endif
