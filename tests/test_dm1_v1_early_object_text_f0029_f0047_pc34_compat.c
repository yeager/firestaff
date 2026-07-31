#include "dm1_v1_early_object_text_f0029_f0047_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int assertions;
static int failures;
#define CHECK(expression) do { ++assertions; if (!(expression)) { ++failures; \
    fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expression); } } while (0)

int main(void)
{
    const uint8_t atlas[8] = {1};
    const uint8_t font[8] = {2};
    const uint8_t message[] = "MESSAGE";
    const uint16_t first_icons[7] = {0, 32, 64, 96, 128, 160, 192};
    uint8_t names[DM1_V1_F0031_OBJECT_NAME_COUNT_PC34 * 2];
    DM1_V1_F0031ObjectNamesRequestPc34 names_request;
    DM1_V1_F0031ObjectNamesReceiptPc34 names_receipt;
    DM1_V1_F0036IconAtlasRequestPc34 icon_request;
    DM1_V1_F0036IconAtlasReceiptPc34 icon_receipt;
    DM1_V1_F0040F0047TextMaterialRequestPc34 text_request;
    DM1_V1_F0040F0047TextMaterialReceiptPc34 text_receipt;
    uint32_t seed = 1u;

    memset(names, 0, sizeof(names));
    names[0] = 'A';
    memset(&names_request, 0, sizeof(names_request));
    names_request.raw_names = names;
    names_request.raw_names_byte_count = sizeof(names);
    names_request.graphics_dat_fingerprint = 1u;
    /* PC3.4 binds ReDMCSB M564 to GRAPHICS.DAT record 694. Keep this
     * source-lock test coupled to the production media constant. */
    names_request.graphic_index = DM1_V1_F0031_OBJECT_NAMES_GRAPHIC_PC34;
    names_request.decoded_from_original_graphics_dat = 1;
    names_request.raw_record_verified = 1;
    names_request.no_synthetic_names = 1;
    CHECK(dm1_v1_f0031_object_names_admission_pc34(&names_request, &names_receipt));
    CHECK(names_receipt.accepted && names_receipt.raw_names_bound && names_receipt.name_count == 199);

    memset(&icon_request, 0, sizeof(icon_request));
    icon_request.raw_atlas_pixels = atlas;
    icon_request.raw_atlas_byte_count = sizeof(atlas);
    icon_request.graphics_dat_fingerprint = 2u;
    icon_request.first_icon_indices = first_icons;
    icon_request.icon_index = 65;
    icon_request.decoded_from_original_graphics_dat = 1;
    icon_request.raw_record_verified = 1;
    icon_request.no_synthetic_surface = 1;
    CHECK(dm1_v1_f0036_icon_atlas_admission_pc34(&icon_request, &icon_receipt));
    CHECK(icon_receipt.accepted && icon_receipt.source_sheet_ordinal == 2 &&
          icon_receipt.source_local_icon_index == 1 && icon_receipt.raw_atlas_bound);

    memset(&text_request, 0, sizeof(text_request));
    text_request.raw_font_bytes = font;
    text_request.raw_font_byte_count = sizeof(font);
    text_request.font_fingerprint = 3u;
    text_request.raw_message_bytes = message;
    text_request.raw_message_byte_count = sizeof(message);
    text_request.message_fingerprint = 4u;
    text_request.original_pc34_font_verified = 1;
    text_request.original_message_route_verified = 1;
    text_request.no_host_font = 1;
    text_request.no_synthetic_text = 1;
    CHECK(dm1_v1_f0040_f0047_text_material_admission_pc34(&text_request, &text_receipt));
    CHECK(text_receipt.accepted && text_receipt.font_bound && text_receipt.message_bound);
    text_request.no_host_font = 0;
    CHECK(!dm1_v1_f0040_f0047_text_material_admission_pc34(&text_request, &text_receipt));

    CHECK(dm1_v1_f0029_get_2bit_random_number_pc34(&seed) == 2u);
    CHECK(dm1_v1_f0029_get_2bit_random_number_pc34(0) == 0u);
    CHECK(strstr(dm1_v1_early_object_text_f0029_f0047_source_evidence_pc34(), "F0040-F0047") != NULL);
    printf("test_dm1_v1_early_object_text_f0029_f0047_pc34_compat: %d assertions, %d failures\n", assertions, failures);
    return failures == 0 ? 0 : 1;
}
