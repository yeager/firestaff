/*
 * ReDMCSB STARTUP2.C F0748_Get_EMS_Memory, PC 3.4 (I34E/I34M) route.
 */
#ifndef FIRESTAFF_REDMCSB_F0748_GET_EMS_MEMORY_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0748_GET_EMS_MEMORY_PC34_COMPAT_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum { REDMCSB_F0748_MAX_MAPPABLE_PAGES_PC34 = 64 };

typedef struct {
    uint16_t page_frame_segment;
    uint16_t physical_page;
} redmcsb_f0748_physical_page_pc34_compat;

typedef struct {
    bool (*is_ems_present)(void *context);
    uint16_t (*get_unallocated_page_count)(void *context);
    int16_t (*get_ems_version)(void *context);
    uint16_t (*get_mappable_physical_address_array)(
        void *context,
        redmcsb_f0748_physical_page_pc34_compat *entries,
        uint16_t capacity);
    uint16_t (*allocate_pages)(void *context, uint16_t page_count);
    void (*map_multiple_handle_pages)(
        void *context,
        uint16_t handle,
        const redmcsb_f0748_physical_page_pc34_compat *entries,
        uint16_t entry_count);
    uint16_t (*get_page_frame_segment)(void *context);
    void (*map_page)(
        void *context,
        uint16_t handle,
        uint16_t physical_page,
        uint16_t logical_page);
    void *context;
} redmcsb_f0748_ems_pc34_compat;

/*
 * Executes F0748's PC 3.4 EMS allocation route. The address-array callback
 * must return no more than REDMCSB_F0748_MAX_MAPPABLE_PAGES_PC34 entries,
 * matching the source routine's fixed 64-entry automatic arrays.
 */
int32_t redmcsb_f0748_get_ems_memory_pc34_compat(
    const redmcsb_f0748_ems_pc34_compat *ems,
    uint16_t *ems_handle,
    uint16_t *ems_page_frame_segment,
    uint16_t initial_previous_page_frame_segment);

const char *redmcsb_f0748_get_ems_memory_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
