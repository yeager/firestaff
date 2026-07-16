#include "csb_v1_f1164_f1165_f1166_f1167_usio_queue_boundaries_pc34_compat.h"

#include <stddef.h>
#include <string.h>

static csb_v1_usio_queue_pc34_compat csb_v1_empty_source_named_queue = {
    0,
    CSB_V1_USIO_QUEUE_CAPACITY_PC34 - 1,
    {{0, 0, 0, 0, 0, 0, 0}}
};

static int csb_v1_usio_queue_indices_valid(
    const csb_v1_usio_queue_pc34_compat *queue)
{
    return queue != NULL && queue->first_index >= 0 &&
           queue->first_index < CSB_V1_USIO_QUEUE_CAPACITY_PC34 &&
           queue->last_index >= 0 &&
           queue->last_index < CSB_V1_USIO_QUEUE_CAPACITY_PC34;
}

void csb_v1_usio_queue_init_empty_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue)
{
    if (queue == NULL) {
        return;
    }
    memset(queue, 0, sizeof(*queue));
    queue->first_index = 0;
    queue->last_index = CSB_V1_USIO_QUEUE_CAPACITY_PC34 - 1;
}

int16_t csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(
    const csb_v1_usio_queue_pc34_compat *queue)
{
    int16_t candidate;

    if (!csb_v1_usio_queue_indices_valid(queue)) {
        return -1;
    }

    candidate = (int16_t)(queue->last_index + 1);
    if (candidate >= CSB_V1_USIO_QUEUE_CAPACITY_PC34) {
        candidate = 0;
    }

    if (candidate == queue->first_index) {
        return -1;
    }
    return queue->first_index;
}

int16_t csb_v1_f1164_usio_15_get_first_queued_usio_data_type_pc34_compat(
    const csb_v1_usio_queue_pc34_compat *queue)
{
    const int16_t first_index =
        csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(queue);

    if (first_index == -1) {
        return 0;
    }
    return queue->entries[first_index].usio_type;
}

int16_t csb_v1_f1176_extract_first_usio_data_from_queue_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue,
    csb_v1_usio_data_pc34_compat *out_usio_data)
{
    const int16_t first_index =
        csb_v1_f1175_get_first_queued_usio_data_index_pc34_compat(queue);

    if (out_usio_data == NULL) {
        return 0;
    }

    if (first_index == -1) {
        out_usio_data->usio_type = 0;
        return 0;
    }

    *out_usio_data = queue->entries[first_index];
    queue->first_index++;
    if (queue->first_index >= CSB_V1_USIO_QUEUE_CAPACITY_PC34) {
        queue->first_index = 0;
    }
    return out_usio_data->usio_type;
}

int16_t csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue,
    csb_v1_usio_data_pc34_compat *out_usio_data)
{
    return csb_v1_f1176_extract_first_usio_data_from_queue_pc34_compat(
        queue, out_usio_data);
}

int16_t csb_v1_f1165_usio_17_wait_until_keyboard_or_mouse_input_pc34_compat(
    csb_v1_usio_queue_pc34_compat *queue,
    csb_v1_usio_data_pc34_compat *out_usio_data)
{
    return csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_pc34_compat(
        queue, out_usio_data);
}

int csb_v1_f1167_usio_14_get_mouse_status_pc34_compat(
    csb_v1_usio_mouse_status_pc34_compat *out_mouse_status,
    const csb_v1_usio_mouse_status_pc34_compat *caller_owned_host_status)
{
    if (out_mouse_status == NULL || caller_owned_host_status == NULL) {
        return 0;
    }
    *out_mouse_status = *caller_owned_host_status;
    return 1;
}

int16_t F1164_USIO_15_GetFirstQueuedUsioDataType(void)
{
    return csb_v1_f1164_usio_15_get_first_queued_usio_data_type_pc34_compat(
        &csb_v1_empty_source_named_queue);
}

int16_t F1165_USIO_17_WaitUntilKeyboardOrMouseInput(
    csb_v1_usio_data_pc34_compat *out_usio_data)
{
    return csb_v1_f1165_usio_17_wait_until_keyboard_or_mouse_input_pc34_compat(
        &csb_v1_empty_source_named_queue, out_usio_data);
}

int16_t F1166_USIO_16_ExtractFirstUsioDataFromQueue(
    csb_v1_usio_data_pc34_compat *out_usio_data)
{
    return csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_pc34_compat(
        &csb_v1_empty_source_named_queue, out_usio_data);
}

void F1167_USIO_14_GetMouseStatus(
    csb_v1_usio_mouse_status_pc34_compat *out_mouse_status)
{
    (void)out_mouse_status;
}

const char *csb_v1_f1164_usio_15_get_first_queued_usio_data_type_source_evidence_pc34(void)
{
    return "ReDMCSB USIO2.C:41-56 F1164_USIO_15_GetFirstQueuedUsioDataType; "
           "calls F1172, reads F1175 index, returns 0 for empty queue or "
           "G3276_as_InputQueue[index].UsioType";
}

const char *csb_v1_f1165_usio_17_wait_until_keyboard_or_mouse_input_source_evidence_pc34(void)
{
    return "ReDMCSB USIO2.C:60-72 F1165_USIO_17_WaitUntilKeyboardOrMouseInput; "
           "source loops F1172/F1176 until nonzero input, but PC34 has no "
           "host-input pump and returns the bounded F1166 extraction verdict";
}

const char *csb_v1_f1166_usio_16_extract_first_usio_data_from_queue_source_evidence_pc34(void)
{
    return "ReDMCSB USIO2.C:75-86 F1166_USIO_16_ExtractFirstUsioDataFromQueue; "
           "calls F1172 before F1176, with no synthesized PC34 input data";
}

const char *csb_v1_f1167_usio_14_get_mouse_status_source_evidence_pc34(void)
{
    return "ReDMCSB USIO2.C:89-113 and USIO2.C:122-128 "
           "F1167_USIO_14_GetMouseStatus; Amiga/X68000 host mouse status "
           "boundary, PC34 copies only explicit caller-owned host facts";
}

const char *csb_v1_f1175_f1176_usio_queue_source_evidence_pc34(void)
{
    return "ReDMCSB USIO2.C:359-389 F1175/F1176; 11-slot ring queue, "
           "last+1 empty test, -1 empty index, UsioType zero on empty "
           "extract, and first-index wrap after successful extraction";
}
