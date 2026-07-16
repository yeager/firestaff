#ifndef NEXUS_V1_PRS3_STRUCTURE2_ABI_H
#define NEXUS_V1_PRS3_STRUCTURE2_ABI_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_dungeon.h"
#include "nexus_v1_prs3_sh2_subset_trace.h"
#include "nexus_v1_prs3_structure2_intake.h"

typedef enum {
    NEXUS_V1_PRS3_STRUCTURE2_ABI_READY_BLOCKED = 0,
    NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_INPUT = 1,
    NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_VECTOR = 2,
    NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_PALT = 3,
    NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_STRUCTURE2 = 4,
    NEXUS_V1_PRS3_STRUCTURE2_ABI_BLOCKED_AUTH_TRACE = 5
} Nexus_V1_Prs3Structure2AbiStatus;

typedef struct {
    const uint8_t *dm_bin;
    size_t dm_bin_size;
    int dm_bin_source_verified;
    const uint8_t *menu_bpk;
    size_t menu_bpk_size;
    int menu_bpk_source_verified;
    const Nexus_V1_Level *level;
    int level_index;
    int level_source_verified;
    uint32_t prs3_entry_index;
    int independent_saturn_trace_bound;
    int vdp1_consumer_semantics_proven;
    int pixel_format_proven;
    int palette_application_proven;
    int structure2_placement_proven;
} Nexus_V1_Prs3Structure2AbiInput;

typedef struct {
    Nexus_V1_Prs3Structure2AbiStatus status;
    int dm_bin_source_verified;
    int menu_bpk_source_verified;
    int level_source_verified;
    int positive_prs3_vector_bound;
    uint32_t prs3_entry_index;
    uint32_t prs3_stream_offset;
    uint32_t prs3_stream_size;
    uint32_t prs3_expected_output_bytes;
    uint32_t prs3_width;
    uint32_t prs3_height;
    uint32_t prs3_bpp;
    uint64_t prs3_output_fnv1a64;
    uint32_t prs3_input_read_bytes;
    uint32_t prs3_output_store_count;
    uint32_t prs3_zero_merge_count;
    uint32_t prs3_zero_copy_count;
    int palt_trailer_bound;
    uint64_t palt_entries_fnv1a64;
    int palt_entries_are_be16;
    int structure2_intake_bound;
    int structure2_descriptor_count;
    int structure2_image_anchor_count;
    int structure2_palette_anchor_count;
    int structure2_palette_absent_count;
    int structure2_encoding_0x0008_count;
    int structure2_encoding_0x0028_count;
    int independent_saturn_trace_bound;
    int vdp1_consumer_semantics_proven;
    int pixel_format_proven;
    int palette_application_proven;
    int structure2_placement_proven;
    int decoder_output_to_structure2_bound;
    int can_submit_structure2_pixels;
    int can_submit_palette;
    int runtime_m11_handoff_permitted;
    int fallback_visuals_permitted;
    int no_draw_only;
} Nexus_V1_Prs3Structure2AbiReceipt;

int nexus_v1_prs3_structure2_abi_admit(
    const Nexus_V1_Prs3Structure2AbiInput *input,
    Nexus_V1_Prs3Structure2AbiReceipt *out_receipt);

const char *nexus_v1_prs3_structure2_abi_status_name(
    Nexus_V1_Prs3Structure2AbiStatus status);

#endif
