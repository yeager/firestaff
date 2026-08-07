#include "dm1_v1_fmtowns_tmenu_input.h"

#define TMENU_EVENT_MAX 4096U

uint32_t dm1_v1_fmtowns_tmenu_event_record_vaddr_pc34(uint32_t event_slot) {
    if (event_slot >= TMENU_EVENT_MAX) return 0U;
    return DM1_V1_FMTOWNS_TMENU_EVENT_RECORD_BASE_VADDR +
           event_slot * DM1_V1_FMTOWNS_TMENU_EVENT_RECORD_STRIDE_BYTES;
}

uint32_t dm1_v1_fmtowns_tmenu_event_text_vaddr_pc34(uint32_t event_slot) {
    if (event_slot >= TMENU_EVENT_MAX) return 0U;
    return DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_POOL_VADDR +
           DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_HEADER_BYTES +
           event_slot * DM1_V1_FMTOWNS_TMENU_EVENT_TEXT_STRIDE_BYTES;
}

uint32_t dm1_v1_fmtowns_tmenu_handler_vaddr_pc34(uint32_t event_id) {
    if (event_id >= TMENU_EVENT_MAX) return 0U;
    return DM1_V1_FMTOWNS_TMENU_HANDLER_TABLE_VADDR + event_id * 4U;
}
