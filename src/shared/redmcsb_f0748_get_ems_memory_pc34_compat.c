#include "redmcsb_f0748_get_ems_memory_pc34_compat.h"

int32_t redmcsb_f0748_get_ems_memory_pc34_compat(
    const redmcsb_f0748_ems_pc34_compat *ems,
    uint16_t *ems_handle,
    uint16_t *ems_page_frame_segment,
    uint16_t initial_previous_page_frame_segment)
{
    redmcsb_f0748_physical_page_pc34_compat physical_pages[
        REDMCSB_F0748_MAX_MAPPABLE_PAGES_PC34];
    uint16_t contiguous_page_counts[REDMCSB_F0748_MAX_MAPPABLE_PAGES_PC34];
    uint16_t unallocated_page_count;
    uint16_t mappable_page_count;
    uint16_t selected_page_frame_segment = UINT16_C(0);
    uint16_t selected_page_count = UINT16_C(0);
    uint16_t selected_index = UINT16_C(0);
    uint16_t running_index = UINT16_C(0);
    uint16_t previous_page_frame_segment = initial_previous_page_frame_segment;
    int16_t ems_version;
    uint16_t index;

    /* ReDMCSB STARTUP2.C:178-274, PC 3.4 MEDIA707_I34E_I34M. */
    *ems_page_frame_segment = UINT16_C(0);
    if (!ems->is_ems_present(ems->context)) {
        return INT32_C(0);
    }

    unallocated_page_count = ems->get_unallocated_page_count(ems->context);
    ems_version = ems->get_ems_version(ems->context);
    if (ems_version >= INT16_C(4)) {
        mappable_page_count = ems->get_mappable_physical_address_array(
            ems->context, physical_pages,
            REDMCSB_F0748_MAX_MAPPABLE_PAGES_PC34);
        if (mappable_page_count > REDMCSB_F0748_MAX_MAPPABLE_PAGES_PC34) {
            return INT32_C(0);
        }

        for (index = UINT16_C(0); index < mappable_page_count; index++) {
            uint16_t page_frame_segment = physical_pages[index].page_frame_segment;

            contiguous_page_counts[index] = UINT16_C(0);
            if (page_frame_segment >= UINT16_C(0xc800)) {
                if ((uint16_t)(previous_page_frame_segment + UINT16_C(1024)) ==
                    page_frame_segment) {
                    contiguous_page_counts[running_index]++;
                } else {
                    running_index = index;
                    contiguous_page_counts[index] = UINT16_C(1);
                }
            }
            previous_page_frame_segment = page_frame_segment;
        }

        for (index = UINT16_C(0); index < mappable_page_count; index++) {
            if (contiguous_page_counts[index] > selected_page_count) {
                selected_page_frame_segment = physical_pages[index].page_frame_segment;
                selected_page_count = contiguous_page_counts[index];
                selected_index = index;
            }
        }
        if (unallocated_page_count < selected_page_count) {
            selected_page_count = unallocated_page_count;
        }

        *ems_handle = ems->allocate_pages(ems->context, selected_page_count);
        for (index = UINT16_C(0); index < selected_page_count; index++) {
            uint16_t page_frame_segment =
                physical_pages[(uint16_t)(selected_index + index)]
                    .page_frame_segment;

            physical_pages[(uint16_t)(selected_index + index)].page_frame_segment =
                index;
            physical_pages[(uint16_t)(selected_index + index)].physical_page =
                page_frame_segment;
        }
        ems->map_multiple_handle_pages(
            ems->context, *ems_handle, &physical_pages[selected_index],
            selected_page_count);
        if (selected_page_count == UINT16_C(0)) {
            return INT32_C(0);
        }
        *ems_page_frame_segment = selected_page_frame_segment;
        return (int32_t)selected_page_count << 14;
    }

    if (ems_version == INT16_C(3)) {
        uint16_t page_frame_segment;

        if (unallocated_page_count > UINT16_C(4)) {
            unallocated_page_count = UINT16_C(4);
        }
        page_frame_segment = ems->get_page_frame_segment(ems->context);
        *ems_page_frame_segment = page_frame_segment;
        *ems_handle = ems->allocate_pages(ems->context, unallocated_page_count);
        for (index = UINT16_C(0); index < unallocated_page_count; index++) {
            ems->map_page(ems->context, *ems_handle, index, index);
        }
        return (int32_t)unallocated_page_count << 14;
    }

    return INT32_C(0);
}

const char *redmcsb_f0748_get_ems_memory_source_evidence_pc34(void)
{
    return "ReDMCSB STARTUP2.C:160-274 (PC 3.4 MEDIA707_I34E_I34M): "
           "F0748 clears P2152, calls F0746, gets EMS pages (INT 67h/42h), "
           "then uses F0747. EMS 4+ selects the longest contiguous mappable "
           "page-frame run at segment >= C800h, allocates it (43h), converts "
           "the selected entries to logical-page/physical-segment pairs, and "
           "maps them with 5001h. EMS 3 caps pages at four, gets the frame "
           "segment (41h), allocates (43h), and maps matching logical and "
           "physical pages with 44h. The original fixed automatic arrays have "
           "64 entries; the portable callback has the same capacity contract.";
}
