#include "dm1_v1_f0074_f0079_mouse_cpsc_boundary_pc34_compat.h"

#include <string.h>

static int request_valid(const DM1_V1_F0074F0079BoundaryRequestPc34 *request)
{
    return request && request->raw_pc34_fingerprint != 0u &&
           request->original_pc34_data_verified && request->no_host_cursor &&
           request->no_synthetic_pointer && request->no_copy_protection_emulation;
}

static void clear_receipt(DM1_V1_F0074F0079BoundaryReceiptPc34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

static int platform_boundary(const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
                             DM1_V1_F0074F0079BoundaryReceiptPc34 *receipt)
{
    clear_receipt(receipt);
    if (!request_valid(request) || !receipt) return 0;
    receipt->fail_closed = 1;
    receipt->platform_operation_suppressed = 1;
    receipt->source_evidence = dm1_v1_f0074_f0079_mouse_cpsc_source_evidence_pc34();
    return 0;
}

int dm1_v1_f0074_draw_pointer_screen_area_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt)
{
    return platform_boundary(request, out_receipt);
}

int dm1_v1_f0075_ikbd_mouse_status_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt)
{
    return platform_boundary(request, out_receipt);
}

int dm1_v1_f0076_mouse_buttons_status_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt)
{
    return platform_boundary(request, out_receipt);
}

int dm1_v1_f0077_enable_screen_update_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0077F0078ScreenUpdateStatePc34 *state,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt)
{
    int before;
    clear_receipt(out_receipt);
    if (!request_valid(request) || !state || !out_receipt ||
        state->hide_mouse_pointer_request_count < 0) return 0;
    before = state->hide_mouse_pointer_request_count;
    state->hide_mouse_pointer_request_count++;
    out_receipt->source_body_applicable = 1;
    out_receipt->execution_permitted = 1;
    out_receipt->request_count_before = before;
    out_receipt->request_count_after = state->hide_mouse_pointer_request_count;
    out_receipt->outermost_transition = before == 0;
    out_receipt->platform_operation_suppressed = 1;
    out_receipt->source_evidence = dm1_v1_f0074_f0079_mouse_cpsc_source_evidence_pc34();
    return 1;
}

int dm1_v1_f0078_disable_screen_update_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0077F0078ScreenUpdateStatePc34 *state,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt)
{
    int before;
    clear_receipt(out_receipt);
    if (!request_valid(request) || !state || !out_receipt ||
        state->hide_mouse_pointer_request_count <= 0) return 0;
    before = state->hide_mouse_pointer_request_count;
    state->hide_mouse_pointer_request_count--;
    out_receipt->source_body_applicable = 1;
    out_receipt->execution_permitted = 1;
    out_receipt->request_count_before = before;
    out_receipt->request_count_after = state->hide_mouse_pointer_request_count;
    out_receipt->outermost_transition = before == 1;
    out_receipt->platform_operation_suppressed = 1;
    out_receipt->source_evidence = dm1_v1_f0074_f0079_mouse_cpsc_source_evidence_pc34();
    return 1;
}

int dm1_v1_f0079_cpsc_get_checksum_add_boundary_pc34(
    const DM1_V1_F0074F0079BoundaryRequestPc34 *request,
    DM1_V1_F0074F0079BoundaryReceiptPc34 *out_receipt)
{
    clear_receipt(out_receipt);
    if (!request_valid(request) || !out_receipt) return 0;
    out_receipt->fail_closed = 1;
    out_receipt->copy_protection_suppressed = 1;
    out_receipt->source_evidence = dm1_v1_f0074_f0079_mouse_cpsc_source_evidence_pc34();
    return 0;
}

const char *dm1_v1_f0074_f0079_mouse_cpsc_source_evidence_pc34(void)
{
    return "ReDMCSB IO.C F0074:2908, F0075:3141, and F0076:3194 are Atari "
           "68000 assembly only; PC34 does not admit pointer, IKBD, or interrupt "
           "emulation. IO.C F0077:3407-3444 and F0078:3448-3469 define the "
           "G0587 nested screen-update bracket; Firestaff retains only that counter "
           "receipt and suppresses host cursor work. COPYPRO1.C F0079 is non-PC34 "
           "copy-protection checksum code and is not emulated.";
}
