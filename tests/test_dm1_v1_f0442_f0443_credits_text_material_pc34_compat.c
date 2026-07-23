#include "dm1_v1_f0442_f0443_credits_text_material_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
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

int main(void)
{
    const size_t pixel_count = (size_t)320u * 200u;
    uint8_t *pixels = (uint8_t *)calloc(pixel_count, 1u);
    uint8_t palette[DM1_V1_F0442_PALETTE_BYTE_COUNT_PC34] = {0};
    const uint8_t scroll_font[] = {1, 2, 3};
    DM1_V1_F0442CreditsMaterialRequestPc34 credits;
    DM1_V1_F0442CreditsMaterialReceiptPc34 credits_receipt;
    DM1_V1_F0443EndgameTextRequestPc34 text;
    DM1_V1_F0443EndgameTextReceiptPc34 text_receipt;

    CHECK(pixels != NULL);
    if (!pixels) return 1;
    pixels[0] = 1u;
    palette[2] = 63u;
    memset(&credits, 0, sizeof(credits));
    credits.c005_indexed_pixels = pixels;
    credits.c005_pixel_byte_count = pixel_count;
    credits.credits_palette_rgb6 = palette;
    credits.credits_palette_byte_count = sizeof(palette);
    credits.c005_record_fingerprint = 1u;
    credits.palette_record_fingerprint = 2u;
    credits.original_graphics_dat_c005 = 1;
    credits.original_palette_verified = 1;
    credits.decoded_indexed_pixels_verified = 1;
    credits.no_synthetic_credits_page = 1;
    credits.no_host_wrapper = 1;
    CHECK(dm1_v1_f0442_entrance_credits_material_admission_pc34(&credits, &credits_receipt));
    CHECK(credits_receipt.accepted && credits_receipt.c005_credits_bound &&
          credits_receipt.credits_palette_bound && credits_receipt.wait_vblank_count == 1800u);

    memset(&text, 0, sizeof(text));
    text.x = 87;
    text.y = 42;
    text.text_color = 9;
    text.source_text = "ARCH MASTER-9";
    text.scroll_font_bytes = scroll_font;
    text.scroll_font_byte_count = sizeof(scroll_font);
    text.source_text_fingerprint = 3u;
    text.scroll_font_fingerprint = 4u;
    text.source_text_verified = 1;
    text.original_scroll_font_verified = 1;
    text.no_host_font = 1;
    text.no_synthetic_text = 1;
    CHECK(dm1_v1_f0443_endgame_print_string_material_admission_pc34(&text, &text_receipt));
    CHECK(text_receipt.accepted && text_receipt.background_color == 12 &&
          (unsigned char)text_receipt.scroll_font_text[0] == 1u &&
          (unsigned char)text_receipt.scroll_font_text[5] == 13u &&
          text_receipt.scroll_font_text[11] == '-' && text_receipt.scroll_font_bound);

    text.no_host_font = 0;
    CHECK(!dm1_v1_f0443_endgame_print_string_material_admission_pc34(&text, &text_receipt));
    text.no_host_font = 1;
    palette[0] = 64u;
    CHECK(!dm1_v1_f0442_entrance_credits_material_admission_pc34(&credits, &credits_receipt));

    free(pixels);
    printf("test_dm1_v1_f0442_f0443_credits_text_material_pc34_compat: %d assertions, %d failures\\n", assertions, failures);
    return failures == 0 ? 0 : 1;
}
