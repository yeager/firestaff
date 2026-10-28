#include "csb_hint_oracle_graphics_surface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static size_t pack_codes_9bit(const uint16_t *codes,
                              size_t code_count,
                              uint8_t *out,
                              size_t out_capacity)
{
    size_t bit_position = 0u;
    size_t i;

    memset(out, 0, out_capacity);
    for (i = 0u; i < code_count; ++i) {
        unsigned int bit;
        for (bit = 0u; bit < 9u; ++bit) {
            size_t position = bit_position + bit;
            if ((codes[i] & (uint16_t)(1u << bit)) != 0u) {
                out[position / 8u] |= (uint8_t)(1u << (position % 8u));
            }
        }
        bit_position += 9u;
    }
    return (bit_position + 7u) / 8u;
}

int main(void)
{
    CSB_HintOracleGraphicsSurface surface;
    CSB_HintOracleHTC htc;
    uint8_t font[256u * CSB_HINT_ORACLE_FONT_SURFACE_MAX_HEIGHT];
    uint8_t base[320u * 200u];
    uint8_t frame[320u * 200u];
    uint8_t next_frame[320u * 200u];
    uint8_t record[26] = {0};
    uint8_t length[4] = {0};
    uint8_t packed[8] = {0};
    const uint16_t hint_pages[] = {'A', 0u, 'B', 0u};
    size_t packed_size;
    size_t index;

    memset(&surface, 0, sizeof(surface));
    memset(&htc, 0, sizeof(htc));
    memset(font, 12, sizeof(font));
    memset(base, 2, sizeof(base));
    memset(frame, 1, sizeof(frame));
    surface.font_pixels = font;
    surface.font_width = 256;
    surface.font_height = CSB_HINT_ORACLE_FONT_SURFACE_HEIGHT_ST;
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
    font[(size_t)9u * 256u + 16u] = 6u;
    font[(size_t)9u * 256u + 17u] = 6u;
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
    surface.font_height = CSB_HINT_ORACLE_FONT_SURFACE_HEIGHT_AU2;
    font[(size_t)(('A' >> 5u) * 9u) * 256u +
         (size_t)(('A' & 31u) * 8u)] = 5u;
    memset(frame, 1, sizeof(frame));
    if (!csb_hint_oracle_graphics_surface_blit_st_text(
            &surface, frame, sizeof(frame), 0, 0, "A") ||
        frame[0] != 5u) {
        fprintf(stderr, "FAIL: AU2 raw-ASCII font indexing\n");
        return 1;
    }
    surface.font_height = CSB_HINT_ORACLE_FONT_SURFACE_HEIGHT_ST;
    memset(frame, 1, sizeof(frame));
    if (!csb_hint_oracle_graphics_surface_blit_st_centered_box(
            &surface, frame, sizeof(frame), 10, 28, 5, 30, "A/A") ||
        frame[16u * 320u + 15u] != 7u ||
        frame[27u * 320u + 15u] != 7u) {
        fprintf(stderr, "FAIL: box layout\n");
        return 1;
    }
    record[0] = 'A';
    record[25] = 2;
    packed_size = pack_codes_9bit(hint_pages,
                                  sizeof(hint_pages) / sizeof(hint_pages[0]),
                                  packed, sizeof(packed));
    length[0] = (uint8_t)(packed_size >> 8u);
    length[1] = (uint8_t)packed_size;
    htc.hints = record;
    htc.hint_record_size = 26;
    htc.hint_count = 1;
    htc.page_lengths = length;
    htc.page_count = 2;
    htc.contents = packed;
    htc.content_size = packed_size;
    if (!csb_hint_oracle_graphics_surface_render_st_hint_page(
            &surface, &htc, 0, 1, frame, sizeof(frame)) ||
        frame[0] != 2u || frame[16u * 320u + 155u] != 7u ||
        frame[42u * 320u + 155u] != 7u ||
        !csb_hint_oracle_graphics_surface_render_st_hint_page(
            &surface, &htc, 0, 2, next_frame, sizeof(next_frame)) ||
        next_frame[42u * 320u + 155u] != 6u ||
        memcmp(frame, next_frame, sizeof(frame)) == 0) {
        fprintf(stderr, "FAIL: NUL-separated page composition\n");
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
