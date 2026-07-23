#include "dm1_v1_f0074_f0079_mouse_cpsc_boundary_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;
#define CHECK(expression) do { ++assertions; if (!(expression)) { ++failures; \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); } } while (0)

int main(void)
{
    DM1_V1_F0074F0079BoundaryRequestPc34 request;
    DM1_V1_F0077F0078ScreenUpdateStatePc34 state;
    DM1_V1_F0074F0079BoundaryReceiptPc34 receipt;
    memset(&request, 0, sizeof(request));
    request.raw_pc34_fingerprint = 0x50433334u;
    request.original_pc34_data_verified = 1;
    request.no_host_cursor = 1;
    request.no_synthetic_pointer = 1;
    request.no_copy_protection_emulation = 1;
    CHECK(!dm1_v1_f0074_draw_pointer_screen_area_boundary_pc34(&request, &receipt));
    CHECK(receipt.fail_closed && receipt.platform_operation_suppressed &&
          !receipt.execution_permitted);
    CHECK(!dm1_v1_f0075_ikbd_mouse_status_boundary_pc34(&request, &receipt));
    CHECK(receipt.fail_closed && receipt.platform_operation_suppressed);
    CHECK(!dm1_v1_f0076_mouse_buttons_status_boundary_pc34(&request, &receipt));
    CHECK(receipt.fail_closed && receipt.platform_operation_suppressed);
    memset(&state, 0, sizeof(state));
    CHECK(dm1_v1_f0077_enable_screen_update_pc34(&request, &state, &receipt));
    CHECK(receipt.request_count_before == 0 && receipt.request_count_after == 1 &&
          receipt.outermost_transition && receipt.platform_operation_suppressed);
    CHECK(dm1_v1_f0077_enable_screen_update_pc34(&request, &state, &receipt));
    CHECK(receipt.request_count_before == 1 && receipt.request_count_after == 2 &&
          !receipt.outermost_transition);
    CHECK(dm1_v1_f0078_disable_screen_update_pc34(&request, &state, &receipt));
    CHECK(receipt.request_count_before == 2 && receipt.request_count_after == 1 &&
          !receipt.outermost_transition);
    CHECK(dm1_v1_f0078_disable_screen_update_pc34(&request, &state, &receipt));
    CHECK(receipt.request_count_before == 1 && receipt.request_count_after == 0 &&
          receipt.outermost_transition);
    CHECK(!dm1_v1_f0078_disable_screen_update_pc34(&request, &state, &receipt));
    CHECK(state.hide_mouse_pointer_request_count == 0 && !receipt.source_evidence);
    CHECK(!dm1_v1_f0079_cpsc_get_checksum_add_boundary_pc34(&request, &receipt));
    CHECK(receipt.fail_closed && receipt.copy_protection_suppressed &&
          !receipt.execution_permitted);
    request.no_synthetic_pointer = 0;
    CHECK(!dm1_v1_f0077_enable_screen_update_pc34(&request, &state, &receipt));
    CHECK(!receipt.source_evidence && state.hide_mouse_pointer_request_count == 0);
    CHECK(strstr(dm1_v1_f0074_f0079_mouse_cpsc_source_evidence_pc34(),
                 "COPYPRO1.C F0079") != NULL);
    printf("test_dm1_v1_f0074_f0079_mouse_cpsc_boundary_pc34_compat: %d assertions, %d failures\n",
           assertions, failures);
    return failures == 0 ? 0 : 1;
}
