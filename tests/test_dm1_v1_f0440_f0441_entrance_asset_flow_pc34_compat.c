#include "dm1_v1_f0440_f0441_entrance_asset_flow_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;

#define CHECK(expression) do { \
    ++assertions; \
    if (!(expression)) { \
        ++failures; \
        fprintf(stderr, "%s:%d: %s\\n", __FILE__, __LINE__, #expression); \
    } \
} while (0)

static DM1_V1_F0440TemporaryGraphicRequestPc34 make_request(int graphic_index, const uint8_t *bytes)
{
    DM1_V1_F0440TemporaryGraphicRequestPc34 request;
    memset(&request, 0, sizeof(request));
    request.graphic_index = graphic_index;
    request.decompressed_bytes = bytes;
    request.decompressed_byte_count = 64u;
    request.graphics_dat_record_fingerprint = (uint32_t)(graphic_index + 1);
    request.original_graphics_dat_member = 1;
    request.raw_record_verified = 1;
    request.not_expanded_route = 1;
    request.temporary_heap_target_bound = 1;
    request.no_synthetic_bytes = 1;
    request.no_host_wrapper = 1;
    return request;
}

int main(void)
{
    const uint8_t bytes[64] = {1};
    DM1_V1_F0440TemporaryGraphicRequestPc34 c004 = make_request(4, bytes);
    DM1_V1_F0440TemporaryGraphicRequestPc34 c005 = make_request(5, bytes);
    DM1_V1_F0440TemporaryGraphicRequestPc34 c002 = make_request(2, bytes);
    DM1_V1_F0440TemporaryGraphicRequestPc34 c003 = make_request(3, bytes);
    DM1_V1_F0440TemporaryGraphicReceiptPc34 c004_receipt;
    DM1_V1_F0440TemporaryGraphicReceiptPc34 c005_receipt;
    DM1_V1_F0440TemporaryGraphicReceiptPc34 rejected_receipt;
    DM1_V1_F0441EntranceFlowRequestPc34 flow;
    DM1_V1_F0441EntranceFlowReceiptPc34 receipt;

    CHECK(strstr(dm1_v1_f0440_f0441_entrance_asset_flow_source_evidence_pc34(), "F0441") != NULL);
    CHECK(dm1_v1_f0440_temporary_graphic_byte_count_pc34(&c004, &c004_receipt));
    CHECK(c004_receipt.accepted && c004_receipt.decompressed_byte_count == 64u &&
          c004_receipt.suppress_synthetic_fallback);
    CHECK(dm1_v1_f0440_temporary_graphic_byte_count_pc34(&c005, &c005_receipt));
    CHECK(!dm1_v1_f0440_temporary_graphic_byte_count_pc34(&c002, &rejected_receipt));

    memset(&flow, 0, sizeof(flow));
    flow.c004_entrance = &c004_receipt;
    flow.c005_credits = &c005_receipt;
    flow.c002_left_door = &c002;
    flow.c003_right_door = &c003;
    flow.door_frame_bank_count = 8u;
    flow.composite_surface_count = 2u;
    flow.graphics_dat_closed_after_source_load = 1;
    flow.no_synthetic_pages = 1;
    flow.no_host_lifecycle = 1;
    CHECK(dm1_v1_f0441_entrance_asset_flow_admission_pc34(&flow, &receipt));
    CHECK(receipt.accepted && receipt.c002_left_door_bound && receipt.c003_right_door_bound &&
          receipt.door_frame_bank_count == 8u && receipt.composite_surface_count == 2u);

    flow.door_frame_bank_count = 7u;
    CHECK(!dm1_v1_f0441_entrance_asset_flow_admission_pc34(&flow, &receipt));
    flow.door_frame_bank_count = 8u;
    c003.no_host_wrapper = 0;
    CHECK(!dm1_v1_f0441_entrance_asset_flow_admission_pc34(&flow, &receipt));

    printf("test_dm1_v1_f0440_f0441_entrance_asset_flow_pc34_compat: %d assertions, %d failures\\n", assertions, failures);
    return failures == 0 ? 0 : 1;
}
