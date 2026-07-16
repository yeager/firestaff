#include "nexus_v1_dgn_face_material_provenance.h"

#include <string.h>

static void set_status(Nexus_V1_DgnFaceMaterialReceipt *receipt,
                       Nexus_V1_DgnFaceMaterialStatus status)
{
    memset(receipt, 0, sizeof(*receipt));
    receipt->status = status;
}

int nexus_v1_dgn_face_material_validate(
    const Nexus_V1_DgnFaceMaterialInput *input,
    Nexus_V1_DgnFaceMaterialReceipt *out_receipt)
{
    uint8_t seen[NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES];
    int i;

    if (!out_receipt) return 0;
    set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_INPUT);
    if (!input || !input->dgn_bytes || input->dgn_size <= 0 ||
        !input->canonical_dgn_bytes || input->canonical_dgn_size <= 0 ||
        !input->bindings || input->face_count <= 0 ||
        input->face_count > NEXUS_V1_DGN_FACE_MATERIAL_MAX_FACES ||
        input->material_selector_count <= 0) {
        return 0;
    }
    if (input->source != NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN ||
        !input->canonical_source_verified) {
        set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
        return 0;
    }
    if (input->dgn_size != input->canonical_dgn_size ||
        memcmp(input->dgn_bytes, input->canonical_dgn_bytes,
               (size_t)input->dgn_size) != 0) {
        set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE);
        return 0;
    }

    memset(seen, 0, sizeof(seen));
    out_receipt->canonical_source_verified = 1;
    out_receipt->reopened_bytes_match_canonical = 1;
    out_receipt->canonical_dgn_size = input->canonical_dgn_size;
    for (i = 0; i < input->face_count; ++i) {
        const Nexus_V1_DgnFaceMaterialBinding *binding = &input->bindings[i];
        if (binding->face_ordinal >= (uint16_t)input->face_count ||
            seen[binding->face_ordinal] ||
            (binding->selector_kind != NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_COLOR &&
             binding->selector_kind != NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC &&
             binding->selector_kind != NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED) ||
            (binding->selector_kind != NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_COLOR &&
             binding->material_selector >= input->material_selector_count) ||
            (binding->selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_COLOR &&
             binding->material_selector != 0U)) {
            set_status(out_receipt, NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING);
            return 0;
        }
        seen[binding->face_ordinal] = 1;
        if (binding->selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_COLOR)
            ++out_receipt->color_selector_count;
        else if (binding->selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC)
            ++out_receipt->static_selector_count;
        else
            ++out_receipt->animated_selector_count;
    }

    out_receipt->status = NEXUS_V1_DGN_FACE_MATERIAL_READY;
    out_receipt->face_count = input->face_count;
    out_receipt->selector_bindings_complete = 1;
    out_receipt->can_submit_raster_input = 1;
    out_receipt->static_texture_draw_blocked =
        out_receipt->static_selector_count > 0 ? 1 : 0;
    out_receipt->animated_texture_draw_blocked =
        out_receipt->animated_selector_count > 0 ? 1 : 0;
    out_receipt->structure2_material_required =
        out_receipt->static_texture_draw_blocked;
    out_receipt->structure1g_material_required =
        out_receipt->animated_texture_draw_blocked;
    out_receipt->structure2_pixel_semantics_required =
        out_receipt->static_texture_draw_blocked;
    out_receipt->structure1g_animation_semantics_required =
        out_receipt->animated_texture_draw_blocked;
    out_receipt->material_host_route_bound =
        input->material_host_route_bound ? 1 : 0;
    out_receipt->material_host_route_prs3_blocked =
        input->material_host_route_prs3_blocked ? 1 : 0;
    out_receipt->material_host_route_pixel_promotion_blocked =
        input->material_host_route_pixel_promotion_blocked ? 1 : 0;
    out_receipt->material_host_route_decoder_promoted =
        input->material_host_route_decoder_promoted ? 1 : 0;
    out_receipt->material_admission_blocked =
        (out_receipt->structure2_material_required ||
         out_receipt->structure1g_material_required)
            ? 1
            : 0;
    if (out_receipt->material_host_route_bound &&
        !out_receipt->material_host_route_prs3_blocked &&
        !out_receipt->material_host_route_pixel_promotion_blocked &&
        !out_receipt->material_host_route_decoder_promoted) {
        out_receipt->material_admission_blocked = 0;
    }
    out_receipt->structure3_texture_admission_blocked =
        (out_receipt->material_admission_blocked ||
         out_receipt->material_host_route_decoder_promoted)
            ? 1
            : 0;
    out_receipt->material_bank_mutation_blocked =
        (out_receipt->structure3_texture_admission_blocked ||
         out_receipt->material_host_route_pixel_promotion_blocked)
            ? 1
            : 0;
    out_receipt->vdp1_command_required =
        (out_receipt->static_selector_count > 0 ||
         out_receipt->animated_selector_count > 0)
            ? 1
            : 0;
    out_receipt->vdp1_command_proven = 0;
    out_receipt->vdp1_draw_list_blocked =
        out_receipt->vdp1_command_required;
    out_receipt->can_submit_textured_draw = 0;
    out_receipt->textured_draw_blocked =
        out_receipt->vdp1_command_required;
    return 1;
}

const char *nexus_v1_dgn_face_material_status_name(
    Nexus_V1_DgnFaceMaterialStatus status)
{
    switch (status) {
    case NEXUS_V1_DGN_FACE_MATERIAL_READY: return "ready";
    case NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE: return "blocked-source";
    case NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_INPUT: return "blocked-input";
    case NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_BINDING: return "blocked-binding";
    }
    return "blocked-input";
}
