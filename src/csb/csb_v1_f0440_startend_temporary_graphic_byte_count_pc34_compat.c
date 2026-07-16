#include "csb_v1_f0440_startend_temporary_graphic_byte_count_pc34_compat.h"

#include <string.h>

static int csb_v1_f0440_graphic_index_is_startup_temporary_pc34(int graphic_index)
{
    return graphic_index == CSB_V1_F0440_GRAPHIC_ENTRANCE_PC34 ||
        graphic_index == CSB_V1_F0440_GRAPHIC_CREDITS_PC34 ||
        graphic_index == CSB_V1_F0440_GRAPHIC_SOUND_DOOR_RATTLE_PC34 ||
        graphic_index == CSB_V1_F0440_GRAPHIC_SOUND_SWITCH_PC34;
}

static int csb_v1_f0440_graphics_boundary_matches_pc34(
    const CSB_V1_StartupGraphicsBoundaryReceipt_PC34 *receipt)
{
    return receipt &&
        (receipt->attempted_stage_mask &
         CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34) &&
        (receipt->no_synthetic_fallback_mask &
         CSB_V1_STARTUP_GRAPHICS_F0490_LOAD_DECOMPRESS_EXPAND_PC34);
}

void csb_v1_f0440_temporary_graphic_receipt_init_pc34(
    CSB_V1_F0440_TemporaryGraphicReceipt_PC34 *receipt)
{
    if (receipt) memset(receipt, 0, sizeof(*receipt));
}

long F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount(
    const CSB_V1_F0440_TemporaryGraphicFacts_PC34 *facts,
    CSB_V1_F0440_TemporaryGraphicReceipt_PC34 *out_receipt)
{
    const char *evidence =
        csb_v1_f0440_temporary_loaded_graphic_byte_count_source_evidence_pc34();

    csb_v1_f0440_temporary_graphic_receipt_init_pc34(out_receipt);
    if (!facts || !facts->valid ||
        !csb_v1_f0440_graphic_index_is_startup_temporary_pc34(
            facts->graphic_index) ||
        facts->decompressed_byte_count <= 0 ||
        !facts->target_pointer_bound ||
        !facts->allocated_on_temporary_heap_top ||
        !facts->not_expanded_graphic_route ||
        !facts->real_graphics_dat_member_bound ||
        !facts->real_decompressed_payload_bound ||
        !facts->load_decompress_expand_route_reviewed ||
        !facts->no_synthetic_graphic_bytes ||
        !facts->no_synthetic_file_handle ||
        !facts->no_legacy_graphics_wrapper ||
        !csb_v1_f0440_graphics_boundary_matches_pc34(
            &facts->graphics_boundary)) {
        if (out_receipt) {
            out_receipt->no_synthetic_graphic_bytes = 1;
            out_receipt->no_synthetic_file_handle = 1;
            out_receipt->source_evidence = evidence;
        }
        return 0;
    }

    out_receipt->valid = 1;
    out_receipt->graphic_index = facts->graphic_index;
    out_receipt->decompressed_byte_count = facts->decompressed_byte_count;
    out_receipt->temporary_heap_allocation_bound = 1;
    out_receipt->not_expanded_graphic_route = 1;
    out_receipt->graphics_boundary_consumed = 1;
    out_receipt->no_synthetic_graphic_bytes = 1;
    out_receipt->no_synthetic_file_handle = 1;
    out_receipt->no_legacy_graphics_wrapper = 1;
    out_receipt->source_evidence = evidence;
    return facts->decompressed_byte_count;
}

const char *csb_v1_f0440_temporary_loaded_graphic_byte_count_source_evidence_pc34(void)
{
    return "ReDMCSB ENTRANCE.C:600-617 F0440_STARTEND_GetTemporarilyLoadedGraphicByteCount "
           "allocates a temporary heap buffer for the real decompressed GRAPHICS.DAT "
           "member byte count and calls F0490_MEMORY_LoadDecompressAndExpandGraphic "
           "with MASK0x8000_NOT_EXPANDED; ENTRANCE.C:692-710/777-788 uses it "
           "for C004 entrance, C005 credits, and entrance sound graphic members";
}
