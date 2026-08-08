#include "nexus_v1_prs3_structure2_intake.h"

#include <string.h>

static void reset_receipt(Nexus_V1_Prs3Structure2IntakeReceipt *receipt,
                          Nexus_V1_Prs3Structure2IntakeStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
    receipt->prs3_decoder_required = 1;
    receipt->no_draw_only = 1;
    receipt->blocks_real_menu_surface_render = 1;
    receipt->blocks_real_dgn_mesh_render = 1;
}

static int bpk_intake_ready(
    const Nexus_V1_Prs3Structure2IntakeInput *input,
    Nexus_V1_Prs3Structure2IntakeReceipt *receipt)
{
    Nexus_V1_BpkArchiveInfo archive;
    Nexus_V1_BpkModeDistribution modes;
    Nexus_V1_BpkRuntimeDecodeReceipt decode;
    Nexus_V1_BpkPrs3StreamPlan plan;
    uint32_t index;

    if (!input->menu_bpk || input->menu_bpk_size == 0U ||
        !input->menu_bpk_source_verified) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_BPK_SOURCE;
        return 0;
    }
    receipt->menu_bpk_source_verified = 1;

    if (nexus_v1_bpk_archive_parse(input->menu_bpk, input->menu_bpk_size,
                                   &archive) != 0 ||
        nexus_v1_bpk_archive_mode_distribution(input->menu_bpk,
                                               input->menu_bpk_size,
                                               &modes) != 0 ||
        archive.candidate_offset_count == 0U ||
        archive.prs3_payload_count == 0U ||
        !modes.trailer_found ||
        modes.mode_count[NEXUS_V1_BPK_MODE_TRAILER] != 1U) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_BPK_SOURCE;
        return 0;
    }

    receipt->menu_bpk_archive_bound = 1;
    receipt->menu_bpk_entry_count = archive.candidate_offset_count;
    receipt->menu_bpk_prs3_entry_count = archive.prs3_payload_count;
    receipt->menu_bpk_trailer_entry_count =
        modes.mode_count[NEXUS_V1_BPK_MODE_TRAILER];

    if (nexus_v1_bpk_archive_inspect_palette_trailer(
            input->menu_bpk, input->menu_bpk_size, &receipt->palt_trailer) != 0 ||
        !receipt->palt_trailer.valid ||
        receipt->palt_trailer.entry_count != 256U ||
        receipt->palt_trailer.entry_bytes != 512U ||
        receipt->palt_trailer.entry_bytes_fnv1a64 == 0U ||
        !receipt->palt_trailer.raw_entries_are_be16 ||
        receipt->palt_trailer.palette_format_proven ||
        receipt->palt_trailer.decoder_promoted ||
        receipt->palt_trailer.fallback_visuals_permitted) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PALT;
        return 0;
    }
    receipt->palt_trailer_bound = 1;

    if (nexus_v1_bpk_archive_runtime_decode_receipt(
            input->menu_bpk, input->menu_bpk_size, &decode) != 0 ||
        decode.route != NEXUS_V1_BPK_DECODE_ROUTE_READY_DECODED ||
        !decode.requires_prs3_decoder ||
        decode.prs3_evidence_only ||
        !decode.prs3_decoder_promoted ||
        decode.prs3_decoded_pixels_emitted != 0U ||
        !decode.renderer_handoff_blocked ||
        decode.fallback_visuals_permitted ||
        decode.decode_blocked) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PRS3;
        return 0;
    }
    receipt->prs3_stream_plan_count = decode.prs3_stream_plans;
    receipt->prs3_stream_plan_failure_count = decode.prs3_stream_plan_failures;
    receipt->prs3_decoder_promoted = decode.prs3_decoder_promoted;
    receipt->prs3_decoded_pixels_emitted = decode.prs3_decoded_pixels_emitted;
    receipt->can_decode_prs3 = decode.prs3_decoder_promoted ? 1 : 0;

    for (index = 0U; index < archive.candidate_offset_count; ++index) {
        if (nexus_v1_bpk_archive_prs3_stream_plan(
                input->menu_bpk, input->menu_bpk_size, index, &plan) ==
            NEXUS_V1_BPK_PRS3_STREAM_OK) {
            receipt->prs3_first_entry_index = index;
            receipt->prs3_first_stream_offset = plan.stream_offset;
            receipt->prs3_first_stream_size = plan.stream_size;
            receipt->prs3_first_expected_output_bytes =
                plan.expected_output_bytes;
            receipt->prs3_first_width = plan.width;
            receipt->prs3_first_height = plan.height;
            receipt->prs3_first_bpp = plan.bpp;
            receipt->prs3_first_header_span_fnv1a64 = plan.header_span_fnv1a64;
            receipt->prs3_first_bitmap_candidate_fnv1a64 = plan.body_span_fnv1a64;
            receipt->prs3_first_bitmap_candidate_byte_count = plan.body_size;
            receipt->prs3_bitmap_candidate_bound = plan.header_span_fnv1a64 != 0U &&
                plan.body_span_fnv1a64 != 0U && plan.body_size > 0U &&
                plan.body_offset <= input->menu_bpk_size &&
                plan.body_size <= input->menu_bpk_size - plan.body_offset;
            receipt->prs3_framing_bound =
                plan.stream_size > 0U &&
                plan.expected_output_bytes > 0U &&
                plan.pixel_count == (uint32_t)plan.width *
                    (uint32_t)plan.height;
            receipt->prs3_framing_bound = receipt->prs3_framing_bound &&
                receipt->prs3_bitmap_candidate_bound;
            break;
        }
    }
    if (!receipt->prs3_framing_bound) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PRS3;
        return 0;
    }
    receipt->palt_candidate_fnv1a64 = receipt->palt_trailer.entry_bytes_fnv1a64;
    receipt->palt_candidate_byte_count = receipt->palt_trailer.entry_bytes;
    receipt->palt_candidate_bound = receipt->palt_candidate_fnv1a64 != 0U &&
        receipt->palt_candidate_byte_count == 512U;
    if (!receipt->palt_candidate_bound) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PALT;
        return 0;
    }
    return 1;
}

static int structure2_intake_ready(
    const Nexus_V1_Prs3Structure2IntakeInput *input,
    Nexus_V1_Prs3Structure2IntakeReceipt *receipt)
{
    const Nexus_V1_Level *level = input->level;
    int index;

    if (!level || input->level_index < 0 || !input->level_source_verified ||
        !level->structure2_texture_table_valid ||
        level->structure2_texture_count <= 0 ||
        !level->structure2_payload.valid ||
        !level->structure2_payload.descriptor_offset_envelope_valid ||
        level->structure2_payload.material_or_image_data_proven) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_STRUCTURE2;
        return 0;
    }
    receipt->level_source_verified = 1;
    receipt->structure2_descriptor_bound = 1;
    receipt->structure2_payload_envelope_bound = 1;
    receipt->structure2_descriptor_count = level->structure2_texture_count;

    for (index = 0; index < level->structure2_texture_count; ++index) {
        const Nexus_V1_DgnStructure2Texture *texture =
            &level->structure2_textures[index];

        if (texture->width == 0U || texture->height == 0U ||
            texture->image_relative_offset == 0U) {
            receipt->status =
                NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_STRUCTURE2;
            return 0;
        }
        ++receipt->structure2_image_anchor_count;
        if (texture->palette_relative_offset != 0U) {
            ++receipt->structure2_palette_anchor_count;
        } else {
            ++receipt->structure2_palette_absent_count;
        }
        if (texture->encoding == 0x0008U) {
            ++receipt->structure2_encoding_0x0008_count;
        } else if (texture->encoding == 0x0028U) {
            ++receipt->structure2_encoding_0x0028_count;
        }
    }

    if (receipt->structure2_image_anchor_count !=
            receipt->structure2_descriptor_count ||
        level->structure2_payload.nonzero_descriptor_offsets_outside_opaque_payload_count != 0 ||
        level->structure2_payload.nonzero_descriptor_offsets_unaligned_count != 0 ||
        level->structure2_payload.nonzero_descriptor_offsets_word_bounded_count !=
            level->structure2_payload.nonzero_descriptor_offsets_in_opaque_payload_count) {
        receipt->status = NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_STRUCTURE2;
        return 0;
    }

    receipt->structure2_payload_anchor_intake_bound = 1;
    receipt->structure2_pixel_span_proven = 0;
    receipt->structure2_palette_addressing_proven = 0;
    receipt->structure2_decoder_permitted = 0;
    return 1;
}

int nexus_v1_prs3_structure2_intake_admit(
    const Nexus_V1_Prs3Structure2IntakeInput *input,
    Nexus_V1_Prs3Structure2IntakeReceipt *out_receipt)
{
    if (!out_receipt) return 0;
    reset_receipt(out_receipt,
                  NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_INPUT);
    if (!input) return 0;
    if (!bpk_intake_ready(input, out_receipt) ||
        !structure2_intake_ready(input, out_receipt)) {
        return 0;
    }

    if (!out_receipt->prs3_decoder_promoted ||
        out_receipt->prs3_decoded_pixels_emitted != 0U ||
        out_receipt->structure2_pixel_span_proven ||
        out_receipt->structure2_palette_addressing_proven ||
        out_receipt->structure2_decoder_permitted) {
        out_receipt->status =
            NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_DECODER_PROVENANCE;
        return 0;
    }

    out_receipt->status =
        NEXUS_V1_PRS3_STRUCTURE2_INTAKE_READY_NO_DRAW;
    /* The DMWeb byte decoder is source-bound now. This does not authorize
     * Structure2 pixels: Saturn CLUT addressing and VDP1/VDP2 placement are
     * still absent, so the intake remains explicitly no-draw. */
    out_receipt->can_decode_prs3 = 1;
    out_receipt->can_submit_structure2_pixels = 0;
    out_receipt->can_submit_palette = 0;
    out_receipt->runtime_render_permitted = 0;
    out_receipt->fallback_visuals_permitted = 0;
    return 1;
}

const char *nexus_v1_prs3_structure2_intake_status_name(
    Nexus_V1_Prs3Structure2IntakeStatus status)
{
    switch (status) {
    case NEXUS_V1_PRS3_STRUCTURE2_INTAKE_READY_NO_DRAW:
        return "ready-no-draw";
    case NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_INPUT:
        return "blocked-input";
    case NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_BPK_SOURCE:
        return "blocked-bpk-source";
    case NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PRS3:
        return "blocked-prs3";
    case NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PALT:
        return "blocked-palt";
    case NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_STRUCTURE2:
        return "blocked-structure2";
    case NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_DECODER_PROVENANCE:
        return "blocked-decoder-provenance";
    default:
        return "unknown";
    }
}
