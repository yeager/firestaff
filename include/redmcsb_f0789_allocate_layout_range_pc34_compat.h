/*
 * ReDMCSB COORD.C F0789_AllocateLayoutRange, PC 3.4 route.
 */
#ifndef FIRESTAFF_REDMCSB_F0789_ALLOCATE_LAYOUT_RANGE_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0789_ALLOCATE_LAYOUT_RANGE_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    REDMCSB_F0789_ALLOCATION_PERMANENT = 1,
    REDMCSB_F0789_MEMORY_REQUEST_LAYOUT_RANGE = UINT16_C(0x0400)
};

typedef unsigned char *(*redmcsb_f0789_memory_allocate_pc34_compat_fn)(
    unsigned long byte_count,
    int allocation_type,
    uint16_t memory_request);

/*
 * Mirrors COORD.C:2536-2539: allocate a permanent layout range through the
 * supplied F0468_MEMORY_Allocate-compatible allocator.
 */
unsigned char *redmcsb_f0789_allocate_layout_range_pc34_compat(
    unsigned long byte_count,
    redmcsb_f0789_memory_allocate_pc34_compat_fn memory_allocate);

const char *redmcsb_f0789_allocate_layout_range_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
