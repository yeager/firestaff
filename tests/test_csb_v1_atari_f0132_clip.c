#include "csb_v1_atari_f0132_clip.h"
#include "csb_v1_atari_st_wall_ornament_data.h"

#include <stdio.h>

static int expect_span(const char *name, int16_t source_x,
                       uint16_t source_width, uint16_t box_width,
                       uint16_t destination_offset, uint16_t source_offset,
                       uint16_t pixel_count)
{
    CSB_V1_AtariF0132VisibleSpan span;
    if (!csb_v1_atari_f0132_visible_span(source_x, source_width, box_width,
                                         &span) ||
        span.destination_offset != destination_offset ||
        span.source_offset != source_offset || span.pixel_count != pixel_count) {
        fprintf(stderr,
                "%s: got dst=%u src=%u count=%u; expected dst=%u src=%u count=%u\n",
                name, span.destination_offset, span.source_offset,
                span.pixel_count, destination_offset, source_offset,
                pixel_count);
        return 0;
    }
    return 1;
}

int main(void)
{
    unsigned set;

    /* Authentic G0205 sets: D2L's AL0089_i_PixelWidth/AL0089_i_X alias
     * leaves source X = D2C span - D2L span before the F0132 blit. */
    for (set = 0u; set < 8u; ++set) {
        const unsigned char *d2l = csb_atari_st_g0205[set][7];
        const unsigned char *d2c = csb_atari_st_g0205[set][8];
        const unsigned char *d2r = csb_atari_st_g0205[set][9];
        const uint16_t bitmap_width =
            (uint16_t)(d2c[1] - d2c[0] + 1u);
        const uint16_t d2l_box_width =
            (uint16_t)(d2l[1] - d2l[0] + 1u);
        const uint16_t d2r_box_width =
            (uint16_t)(d2r[1] - d2r[0] + 1u);
        const uint16_t d2l_source_x =
            (uint16_t)((d2c[1] - d2c[0]) - (d2l[1] - d2l[0]));
        char label[32];

        (void)snprintf(label, sizeof(label), "D2L set %u", set);
        if (!expect_span(label, (int16_t)d2l_source_x, bitmap_width,
                         d2l_box_width, 0u, d2l_source_x,
                         d2l_box_width)) return 1;
        (void)snprintf(label, sizeof(label), "D2R set %u", set);
        if (!expect_span(label, 0, bitmap_width, d2r_box_width,
                         0u, 0u, d2r_box_width)) return 1;
    }

    if (!expect_span("negative source fully clipped", -8, 4, 5,
                     0u, 0u, 0u) ||
        !expect_span("positive source clipped at right", 3, 8, 4,
                     0u, 3u, 4u)) return 1;

    puts("CSB Atari F0107/F0132 clips all eight authentic G0205 sets");
    return 0;
}
