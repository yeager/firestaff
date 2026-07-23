#ifndef FIRESTAFF_DM1_V1_F0074_F0079_MOUSE_CPSC_BOUNDARY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0074_F0079_MOUSE_CPSC_BOUNDARY_PC34_COMPAT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DM1_V1_F0074F0079BoundaryRequestPc34 {
    uint32_t raw_pc34_fingerprint;
    int original_pc34_data_verified;
    int no_host_cursor;
    int no_synthetic_pointer;
    int no_copy_protection_emulation;
} DM1_V1_F0074F0079BoundaryRequestPc34;

typedef struct DM1_V1_F0077F0078ScreenUpdateStatePc34 {
    int hide_mouse_pointer_request_count;
} DM1_V1_F0077F0078ScreenUpdateStatePc34;

typedef struct DM1_V1_F0074F0079BoundaryReceiptPc34 {
    int source_body_applicable;
    int execution_permitted;
    int fail_closed;
    int platform_operation_suppressed;
    int copy_protection_suppressed;
    int request_count_before;
    int request_count_after;
    int outermost_transition;
    const char *source_evidence;
} DM1_V1_F0074F0079BoundaryReceiptPc34;

int dm1_v1_f0074_draw_pointer_screen_area_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt);
int dm1_v1_f0075_ikbd_mouse_status_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt);
int dm1_v1_f0076_mouse_buttons_status_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt);
int dm1_v1_f0077_enable_screen_update_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0077F0078ScreenUpdateStatePc34 *state,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt);
int dm1_v1_f0078_disable_screen_update_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0077F0078ScreenUpdateStatePc34 *state,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt);
int dm1_v1_f0079_cpsc_get_checksum_add_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt);
const char *dm1_v1_f0074_f0079_mouse_cpsc_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM1_V1_F0074_F0079_MOUSE_CPSC_BOUNDARY_PC34_COMPAT_H */
