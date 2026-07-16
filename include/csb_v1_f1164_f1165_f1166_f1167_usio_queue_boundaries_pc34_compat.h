#ifndef FIRESTAFF_CSB_V1_F1164_F1165_F1166_F1167_USIO_QUEUE_BOUNDARIES_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F1164_F1165_F1166_F1167_USIO_QUEUE_BOUNDARIES_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CSB_V1_USIO_QUEUE_CAPACITY_PC34 11

typedef struct csb_v1_usio_data_pc34_compat {
    int16_t usio_type;
    int16_t usio_value;
    int16_t mouse_buttons;
    int16_t mouse_x;
    int16_t mouse_y;
    int16_t keyboard_raw_key_code;
    int16_t keyboard_ascii_code;
} csb_v1_usio_data_pc34_compat;

typedef struct csb_v1_usio_queue_pc34_compat {
    int16_t first_index;
    int16_t last_index;
    csb_v1_usio_data_pc34_compat entries[CSB_V1_USIO_QUEUE_CAPACITY_PC34];
} csb_v1_usio_queue_pc34_compat;

typedef struct csb_v1_usio_mouse_status_pc34_compat {
    int16_t unused1;
    int16_t unused2;
    int16_t unused3;
    int16_t mouse_buttons;
    int16_t mouse_x;
    int16_t mouse_y;
} csb_v1_usio_mouse_status_pc34_compat;

int16_t F1164_USIO_15_GetFirstQueuedUsioDataType(void);
int16_t F1165_USIO_17_WaitUntilKeyboardOrMouseInput(
    csb_v1_usio_data_pc34_compat *out_usio_data);
int16_t F1166_USIO_16_ExtractFirstUsioDataFromQueue(
    csb_v1_usio_data_pc34_compat *out_usio_data);
void F1167_USIO_14_GetMouseStatus(
    csb_v1_usio_mouse_status_pc34_compat *out_mouse_status);

void csb_v1_usio_queue_init_empty_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue);
int16_t csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(
    const csb_v1_usio_queue_pc34_compat *queue);
int16_t csb_v1_f1164_usio_15_get_first_queued_usio_data_type_pc34_compat(
    const csb_v1_usio_queue_pc34_compat *queue);
int16_t csb_v1_f1176_extract_first_usio_data_from_queue_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue,
    csb_v1_usio_data_pc34_compat *out_usio_data);
int16_t csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue,
    csb_v1_usio_data_pc34_compat *out_usio_data);
int16_t csb_v1_f1165_usio_17_wait_until_keyboard_or_mouse_input_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue,
    csb_v1_usio_data_pc34_compat *out_usio_data);
int csb_v1_f1167_usio_14_get_mouse_status_pc34_compat(
    csb_v1_usio_mouse_status_pc34_compat *out_mouse_status,
    const csb_v1_usio_mouse_status_pc34_compat *caller_owned_host_status);

const char *csb_v1_f1164_usio_15_get_first_queued_usio_data_type_source_evidence_pc34(void);
const char *csb_v1_f1165_usio_17_wait_until_keyboard_or_mouse_input_source_evidence_pc34(void);
const char *csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_source_evidence_pc34(void);
const char *csb_v1_f1167_usio_14_get_mouse_status_source_evidence_pc34(void);
const char *csb_v1_f1175_f1176_usio_queue_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_CSB_V1_F1164_F1165_F1166_F1167_USIO_QUEUE_BOUNDARIES_PC34_COMPAT_H */
