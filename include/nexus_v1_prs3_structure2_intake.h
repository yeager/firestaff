#ifndef NEXUS_V1_PRS3_STRUCTURE2_INTAKE_H
#define NEXUS_V1_PRS3_STRUCTURE2_INTAKE_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_dungeon.h"

typedef enum {
    NEXUS_V1_PRS3_STRUCTURE2_INTAKE_READY_NO_DRAW = 0,
    NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_INPUT = 1,
    NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_BPK_SOURCE = 2,
    NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PRS3 = 3,
    NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_PALT = 4,
    NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_STRUCTURE2 = 5,
    NEXUS_V1_PRS3_STRUCTURE2_INTAKE_BLOCKED_DECODER_PROVENANCE = 6
} Nexus_V1_Prs3Structure2IntakeStatus;

typedef struct {
    const uint8_t *menu_bpk;
    size_t menu_bpk_size;
    int menu_bpk_source_verified;
    const Nexus_V1_Level *level;
    int level_index;
    int level_source_verified;
} Nexus_V1_Prs3Structure2IntakeInput;

typedef struct {
    Nexus_V1_Prs3Structure2IntakeStatus status;
    int menu_bpk_source_verified;
    int menu_bpk_archive_bound;
    uint32_t menu_bpk_entry_count;
    uint32_t menu_bpk_prs3_entry_count;
    uint32_t menu_bpk_trailer_entry_count;
    uint32_t prs3_stream_plan_count;
    uint32_t prs3_stream_plan_failure_count;
    uint32_t prs3_first_entry_index;
    uint32_t prs3_first_stream_offset;
    uint32_t prs3_first_stream_size;
    uint32_t prs3_first_expected_output_bytes;
    uint32_t prs3_first_width;
    uint32_t prs3_first_height;
    uint32_t prs3_first_bpp;
    int prs3_framing_bound;
    int prs3_decoder_required;
    int prs3_decoder_promoted;
    uint32_t prs3_decoded_pixels_emitted;
    int palt_trailer_bound;
    Nexus_V1_BpkPaletteTrailerReceipt palt_trailer;
    int level_source_verified;
    int structure2_descriptor_bound;
    int structure2_payload_envelope_bound;
    int structure2_payload_anchor_intake_bound;
    int structure2_descriptor_count;
    int structure2_image_anchor_count;
    int structure2_palette_anchor_count;
    int structure2_palette_absent_count;
    int structure2_encoding_0x0008_count;
    int structure2_encoding_0x0028_count;
    int structure2_pixel_span_proven;
    int structure2_palette_addressing_proven;
    int structure2_decoder_permitted;
    int can_decode_prs3;
    int can_submit_structure2_pixels;
    int can_submit_palette;
    int runtime_render_permitted;
    int no_draw_only;
    int blocks_real_menu_surface_render;
    int blocks_real_dgn_mesh_render;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3Structure2IntakeReceipt;

int nexus_v1_prs3_structure2_intake_admit(
    const Nexus_V1_Prs3Structure2IntakeInput *input,
    Nexus_V1_Prs3Structure2IntakeReceipt *out_receipt);

const char *nexus_v1_prs3_structure2_intake_status_name(
    Nexus_V1_Prs3Structure2IntakeStatus status);

#endif
