#include "csb_hint_oracle_graphics_surface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    CSB_HintOracleGraphicsSurface surface;
    CSB_HintOracleHTC htc;
    uint8_t font[256u * 27u];
    uint8_t base[320u * 200u];
    uint8_t frame[320u * 200u];
    uint8_t record[26] = {0};
    uint8_t length[2] = {0, 3};
    uint8_t packed[3] = {65, 0, 0};
    size_t index;

    memset(&surface, 0, sizeof(surface));
    memset(&htc, 0, sizeof(htc));
    memset(font, 12, sizeof(font));
    memset(base, 2, sizeof(base));
    memset(frame, 1, sizeof(frame));
    surface.font_pixels = font;
    surface.font_width = 256;
    surface.font_height = 27;
    surface.pixels = base;
    surface.width = 320;
    surface.height = 200;
    for (index = 0u; index < 16u; ++index) {
        surface.font_palette_indices[index] = (uint8_t)index;
    }
    /* HCSB's original controls[5]=9 / controls[4]=1 C26 change. */
    surface.font_palette_indices[9u] = 1u;
    surface.font_palette_indices_valid = 1;
    font[(size_t)9u * 256u + 8u] = 7u;
    font[(size_t)9u * 256u + 9u] = 7u;
    font[(size_t)9u * 256u + 10u] = 9u;
    if (!csb_hint_oracle_graphics_surface_blit_st_text(
            &surface, frame, sizeof(frame), 10, 20, " A") ||
        frame[20u * 320u + 19u] != 7u ||
        frame[20u * 320u + 21u] != 1u ||
        frame[20u * 320u + 10u] != 1u) {
        fprintf(stderr, "FAIL: ST font mapping/C26 palette change\n");
        return 1;
    }
    if (!csb_hint_oracle_graphics_surface_blit_st_text(
            &surface, frame, sizeof(frame), -1, 0, "A") ||
        frame[0] != 7u) {
        fprintf(stderr, "FAIL: clipping/key\n");
        return 1;
    }
    memset(frame, 1, sizeof(frame));
    if (!csb_hint_oracle_graphics_surface_blit_st_centered_box(
            &surface, frame, sizeof(frame), 10, 28, 5, 30, "A/A") ||
        frame[16u * 320u + 15u] != 7u ||
        frame[27u * 320u + 15u] != 7u) {
        fprintf(stderr, "FAIL: box layout\n");
        return 1;
    }
    record[0] = 'A';
    record[25] = 1;
    htc.hints = record;
    htc.hint_record_size = 26;
    htc.hint_count = 1;
    htc.page_lengths = length;
    htc.page_count = 1;
    htc.contents = packed;
    htc.content_size = 3;
    if (!csb_hint_oracle_graphics_surface_render_st_hint_page(
            &surface, &htc, 0, 1, frame, sizeof(frame)) ||
        frame[0] != 2u || frame[16u * 320u + 155u] != 7u ||
        frame[42u * 320u + 155u] != 7u) {
        fprintf(stderr, "FAIL: page composition\n");
        return 1;
    }
    {
        const char *real_data_dir = getenv("FIRESTAFF_CSB_HINT_ORACLE_DATA_DIR");
        CSB_HintOracleGraphicsSurface real_surface;

        if (real_data_dir && real_data_dir[0] != '\0') {
            csb_hint_oracle_graphics_surface_init(&real_surface);
            if (csb_hint_oracle_graphics_surface_load(
                    &real_surface, real_data_dir, 6,
                    "708e113c869ab922633e885aa72a3c77") != 0 ||
                !real_surface.font_palette_indices_valid ||
                real_surface.controls[4u] != 1u ||
                real_surface.controls[5u] != 9u ||
                real_surface.font_palette_indices[9u] != 1u ||
                real_surface.font_palette_indices[12u] != 12u) {
                fprintf(stderr, "FAIL: real HCSB C26 font palette receipt\n");
                csb_hint_oracle_graphics_surface_free(&real_surface);
                return 1;
            }
            csb_hint_oracle_graphics_surface_free(&real_surface);
            puts("csb_hint_oracle_text_render: real HCSB C26 receipt PASS");
        }
    }
    puts("csb_hint_oracle_text_render: PASS");
    return 0;
}
