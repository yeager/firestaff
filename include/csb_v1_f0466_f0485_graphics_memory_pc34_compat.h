#ifndef FIRESTAFF_CSB_V1_F0466_F0485_GRAPHICS_MEMORY_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0466_F0485_GRAPHICS_MEMORY_PC34_COMPAT_H

#include "csb_v1_csbgraphics_dat_real_scan.h"
#include "csb_v1_csbgraphics_runtime_plan.h"

#define CSB_V1_F0466_F0485_SOURCE_MASK 0x000fffffu

typedef struct {
    int valid;
    uint32_t graphics_entry_count;
    uint64_t payload_offset;
    uint64_t payload_bytes;
    uint32_t source_bound_mask;
    int expand_owner_required;
    int memory_owner_required;
    int cache_owner_required;
    const char *source_evidence;
} CSB_V1_F0466F0485GraphicsMemoryReceiptPc34;

/* Read-only ReDMCSB EXPAND.C/MEMORY.C F0466-F0485 admission. */
int csb_v1_f0466_f0485_graphics_memory_receipt_pc34(
    const CSB_V1_CSBGraphicsDatRealCache *cache,
    const CSB_V1_CSBGraphicsStartupPackage *package,
    CSB_V1_F0466F0485GraphicsMemoryReceiptPc34 *out_receipt);

#endif
