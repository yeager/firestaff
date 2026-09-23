#ifndef THERON_V1_ORIGINAL_COMMAND_CAPTURE_H
#define THERON_V1_ORIGINAL_COMMAND_CAPTURE_H

#include <stdint.h>

typedef struct {
    const char *command_trace_path;
    const char *command_code_path;
    const char *command_before_ram_path;
    const char *command_after_ram_path;
    const char *main_ram_consumer_trace_path;
    const char *transition_receipt_path;
    const char *expected_mednafen_md5;
    const char *expected_track02_md5;
    const char *expected_system_card_md5;
    const char *expected_autoload_state_md5;
} Theron_V1OriginalCommandCaptureRequest;

typedef struct {
    int admitted;
    int source_identity_verified;
    int input_edge_verified;
    int command_window_verified;
    int consumer_window_verified;
    int snapshots_verified;
    int semantic_publication_allowed;
    uint8_t command_type;
    uint8_t click_x;
    uint8_t click_y;
    uint32_t command_write_count;
    uint32_t completion_sequence;
    uint32_t consumer_source_read_count;
    char trace_md5[33];
    char code_md5[33];
    char before_ram_md5[33];
    char after_ram_md5[33];
    char consumer_trace_md5[33];
} Theron_V1OriginalCommandCaptureReceipt;

/* Admit one original PCE command capture as raw execution evidence.  This
 * API deliberately never assigns movement, door, item, T700 or T900 meaning
 * to command_type or the changed RAM bytes. */
int theron_v1_original_command_capture_admit(
    const Theron_V1OriginalCommandCaptureRequest *request,
    Theron_V1OriginalCommandCaptureReceipt *out_receipt);

#endif
