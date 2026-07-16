#ifndef FIRESTAFF_CSB_V1_F0440_STARTEND_TEMPORARY_GRAPHIC_BYTE_COUNT_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0440_STARTEND_TEMPORARY_GRAPHIC_BYTE_COUNT_PC34_COMPAT_H

#include <stdint.h>

#include "csb_v1_f0474_f0477_f0478_f0479_f0488_f0490_startup_graphics_boundaries_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0440_GRAPHIC_ENTRANCE_PC34 = 4,
    CSB_V1_F0440_GRAPHIC_CREDITS_PC34 = 5,
    CSB_V1_F0440_GRAPHIC_SOUND_DOOR_RATTLE_PC34 = 534,
    CSB_V1_F0440_GRAPHIC_SOUND_SWITCH_PC34 = 535
};

typedef struct CSB_V1_F0440_TemporaryGraphicFacts_PC34 {
    int valid;
    int graphic_index;
    long decompressed_byte_count;
    int target_pointer_bound;
    int allocated_on_temporary_heap_top;
    int not_expanded_graphic_route;
    int real_graphics_dat_member_bound;
    int real_decompressed_payload_bound;
    int load_decompress_expand_route_reviewed;
    int no_synthetic_graphic_bytes;
    int no_synthetic_file_handle;
    int no_legacy_graphics_wrapper;
    CSB_V1_StartupGraphicsBoundaryReceipt_PC34 graphics_boundary;
} CSB_V1_F0440_TemporaryGraphicFacts_PC34;

typedef struct CSB_V1_F0440_TemporaryGraphicReceipt_PC34 {
    int valid;
    int graphic_index;
    long decompressed_byte_count;
    int temporary_heap_allocation_bound;
    int not_expanded_graphic_route;
    int graphics_boundary_consumed;
    int no_synthetic_graphic_bytes;
    int no_synthetic_file_handle;
    int no_legacy_graphics_wrapper;
    const char *source_evidence;
} CSB_V1_F0440_TemporaryGraphicReceipt_PC34;

void csb_v1_f0440_temporary_graphic_receipt_init_pc34(
    CSB_V1_F0440_TemporaryGraphicReceipt_PC34 *receipt);

long F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
    const CSB_V1_F0440_TemporaryGraphicFacts_PC34 *facts,
    CSB_V1_F0440_TemporaryGraphicReceipt_PC34 *out_receipt);

const char *csb_v1_f0440_temporary_loaded_graphic_byte_count_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F0440_STARTEND_TEMPORARY_GRAPHIC_BYTE_COUNT_PC34_COMPAT_H */
