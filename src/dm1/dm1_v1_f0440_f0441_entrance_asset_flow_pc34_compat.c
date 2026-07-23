#include "dm1_v1_f0440_f0441_entrance_asset_flow_pc34_compat.h"

#include <string.h>

static int dm1_v1_f0440_index_allowed_pc34(int graphic_index)
{
    return graphic_index == DM1_V1_F0440_GRAPHIC_ENTRANCE_PC34 ||
        graphic_index == DM1_V1_F0440_GRAPHIC_CREDITS_PC34 ||
        graphic_index == DM1_V1_F0440_GRAPHIC_SOUND_DOOR_RATTLE_PC34 ||
        graphic_index == DM1_V1_F0440_GRAPHIC_SOUND_SWITCH_PC34;
}

static int dm1_v1_f0440_request_valid_pc34(
    const DM1_V1_F0440TemporaryGraphicRequestPc34 *request,
    int expected_index)
{
    return request && request->graphic_index == expected_index &&
        request->decompressed_bytes && request->decompressed_byte_count > 0u &&
        request->graphics_dat_record_fingerprint != 0u &&
        request->original_graphics_dat_member && request->raw_record_verified &&
        request->not_expanded_route && request->temporary_heap_target_bound &&
        request->no_synthetic_bytes && request->no_host_wrapper;
}

static int dm1_v1_f0440_receipt_matches_pc34(
    const DM1_V1_F0440TemporaryGraphicReceiptPc34 *receipt,
    int graphic_index)
{
    return receipt && receipt->accepted && receipt->graphic_index == graphic_index &&
        receipt->decompressed_byte_count > 0u &&
        receipt->graphics_dat_record_fingerprint != 0u &&
        receipt->temporary_heap_target_bound && receipt->not_expanded_route &&
        receipt->suppress_synthetic_fallback;
}

int dm1_v1_f0440_temporary_graphic_byte_count_pc34(
    const DM1_V1_F0440TemporaryGraphicRequestPc34 *request,
    DM1_V1_F0440TemporaryGraphicReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request || !dm1_v1_f0440_index_allowed_pc34(request->graphic_index) ||
        !dm1_v1_f0440_request_valid_pc34(request, request->graphic_index)) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->graphic_index = request->graphic_index;
        out_receipt->decompressed_byte_count = request->decompressed_byte_count;
        out_receipt->graphics_dat_record_fingerprint = request->graphics_dat_record_fingerprint;
        out_receipt->temporary_heap_target_bound = 1;
        out_receipt->not_expanded_route = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0440_f0441_entrance_asset_flow_source_evidence_pc34();
    }
    return 1;
}

int dm1_v1_f0441_entrance_asset_flow_admission_pc34(
    const DM1_V1_F0441EntranceFlowRequestPc34 *request,
    DM1_V1_F0441EntranceFlowReceiptPc34 *out_receipt)
{
    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!request ||
        !dm1_v1_f0440_receipt_matches_pc34(
            request->c004_entrance, DM1_V1_F0440_GRAPHIC_ENTRANCE_PC34) ||
        !dm1_v1_f0440_receipt_matches_pc34(
            request->c005_credits, DM1_V1_F0440_GRAPHIC_CREDITS_PC34) ||
        !dm1_v1_f0440_request_valid_pc34(
            request->c002_left_door, DM1_V1_F0441_GRAPHIC_LEFT_DOOR_PC34) ||
        !dm1_v1_f0440_request_valid_pc34(
            request->c003_right_door, DM1_V1_F0441_GRAPHIC_RIGHT_DOOR_PC34) ||
        request->door_frame_bank_count != DM1_V1_F0441_DOOR_FRAME_BANK_COUNT_PC34 ||
        request->composite_surface_count != DM1_V1_F0441_COMPOSITE_SURFACE_COUNT_PC34 ||
        !request->graphics_dat_closed_after_source_load ||
        !request->no_synthetic_pages || !request->no_host_lifecycle) {
        return 0;
    }
    if (out_receipt) {
        out_receipt->accepted = 1;
        out_receipt->c004_entrance_receipt_consumed = 1;
        out_receipt->c005_credits_receipt_consumed = 1;
        out_receipt->c002_left_door_bound = 1;
        out_receipt->c003_right_door_bound = 1;
        out_receipt->door_frame_bank_count = DM1_V1_F0441_DOOR_FRAME_BANK_COUNT_PC34;
        out_receipt->composite_surface_count = DM1_V1_F0441_COMPOSITE_SURFACE_COUNT_PC34;
        out_receipt->graphics_dat_closed_after_source_load = 1;
        out_receipt->suppress_synthetic_fallback = 1;
        out_receipt->source_evidence =
            dm1_v1_f0440_f0441_entrance_asset_flow_source_evidence_pc34();
    }
    return 1;
}

const char *dm1_v1_f0440_f0441_entrance_asset_flow_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:600-617 F0440 returns the original decompressed "
           "GRAPHICS.DAT member byte count after the NOT_EXPANDED temporary load; "
           "ENTRANCE.C:775-791 F0441 loads C002/C003, obtains C004/C005 through "
           "F0440, builds eight door frames plus two composites, then closes "
           "GRAPHICS.DAT before its separately-owned entrance wait loop.";
}
