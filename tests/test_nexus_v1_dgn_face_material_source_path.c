#include "nexus_v1_dgn_face_material_provenance.h"
#include "nexus_v1_dungeon.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static void put16(uint8_t *dst, unsigned value)
{
    dst[0] = (uint8_t)(value >> 8);
    dst[1] = (uint8_t)value;
}

static void put32(uint8_t *dst, unsigned value)
{
    dst[0] = (uint8_t)(value >> 24);
    dst[1] = (uint8_t)(value >> 16);
    dst[2] = (uint8_t)(value >> 8);
    dst[3] = (uint8_t)value;
}

int main(void)
{
    uint8_t dgn[128];
    uint8_t canonical[sizeof(dgn)];
    Nexus_V1_Level level;
    Nexus_V1_DgnFaceMaterialBinding bindings[3];
    Nexus_V1_DgnFaceMaterialInput input;
    Nexus_V1_DgnFaceMaterialReceipt receipt;
    int binding_count = 0;

    memset(dgn, 0, sizeof(dgn));
    put32(dgn, 1);              /* Structure3 directory entry count. */
    put32(dgn + 4, 8);          /* First entry. */
    put16(dgn + 14, 3);         /* Entry-local face count. */
    put32(dgn + 24, 32);        /* Structure3b face region. */
    dgn[52] = 0x40;             /* Static fill at face 1. */
    put16(dgn + 54, 7);
    dgn[64] = 0x40;             /* Structure1G animated fill at face 2. */
    put16(dgn + 66, 0x0803);
    memcpy(canonical, dgn, sizeof(dgn));

    memset(&level, 0, sizeof(level));
    level.structure3_payload.valid = 1;
    level.structure3_payload.byte_offset = 0;
    level.structure3_payload.byte_size = (int)sizeof(dgn);
    level.structure3_directory.valid = 1;
    level.structure3_directory.entry_count = 1;
    level.structure3_directory.directory_byte_count = 8;
    level.structure3_entry_headers.valid = 1;
    level.structure3_faces.valid = 1;
    level.structure3_faces.face_count = 3;
    level.structure3_face_materials.valid = 1;
    level.structure3_face_materials.selector_bindings_complete = 1;

    expect(nexus_v1_level_collect_structure3_face_material_bindings(
               &level, dgn, (int)sizeof(dgn), bindings, 3, &binding_count) == 0 &&
               binding_count == 3 &&
               bindings[0].selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_COLOR &&
               bindings[1].selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_STATIC &&
               bindings[1].material_selector == 7 &&
               bindings[2].selector_kind == NEXUS_V1_DGN_FACE_MATERIAL_SELECTOR_ANIMATED &&
               bindings[2].material_selector == 3,
           "the active Structure3 bytes produce only documented color/static/animated bindings");

    memset(&input, 0, sizeof(input));
    input.source = NEXUS_V1_DGN_FACE_MATERIAL_SOURCE_RETAIL_DGN;
    input.dgn_bytes = dgn;
    input.dgn_size = (int)sizeof(dgn);
    input.canonical_dgn_bytes = canonical;
    input.canonical_dgn_size = (int)sizeof(canonical);
    input.canonical_source_verified = 1;
    input.bindings = bindings;
    input.face_count = binding_count;
    input.material_selector_count = 256;
    input.material_host_route_bound = 1;
    input.material_host_route_prs3_blocked = 1;
    input.material_host_route_pixel_promotion_blocked = 1;
    input.material_host_route_decoder_promoted = 0;
    expect(nexus_v1_dgn_face_material_validate(&input, &receipt) == 1 &&
               receipt.color_selector_count == 1 &&
               receipt.static_selector_count == 1 &&
               receipt.animated_selector_count == 1 &&
               receipt.can_submit_raster_input &&
               !receipt.can_submit_textured_draw &&
               receipt.textured_draw_blocked &&
               receipt.static_texture_draw_blocked &&
               receipt.animated_texture_draw_blocked &&
               receipt.structure2_material_required &&
               receipt.structure1g_material_required &&
               receipt.structure2_pixel_semantics_required &&
               receipt.structure1g_animation_semantics_required &&
               receipt.material_host_route_bound &&
               receipt.material_host_route_prs3_blocked &&
               receipt.material_host_route_pixel_promotion_blocked &&
               !receipt.material_host_route_decoder_promoted &&
               receipt.material_admission_blocked &&
               receipt.structure3_texture_admission_blocked &&
               receipt.material_bank_mutation_blocked &&
               receipt.vdp1_command_required &&
               !receipt.vdp1_command_proven &&
               receipt.vdp1_draw_list_blocked &&
               !receipt.permits_fallback_visuals,
           "the real-buffer binding table reaches only the no-fallback source boundary");

    ++dgn[67];
    expect(!nexus_v1_dgn_face_material_validate(&input, &receipt) &&
               receipt.status == NEXUS_V1_DGN_FACE_MATERIAL_BLOCKED_SOURCE &&
               !receipt.can_submit_raster_input,
           "a changed retained LEV buffer cannot reuse canonical Structure3 admission");

    if (failures) return 1;
    puts("Nexus DGN face/material source path passed");
    return 0;
}
