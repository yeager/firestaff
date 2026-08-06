#include "dm1_v1_fmtowns_title.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static DM1_V1_FmtownsStartupReceipt receipt(void)
{
    DM1_V1_FmtownsStartupReceipt value;
    memset(&value, 0, sizeof(value));
    value.valid = 1;
    value.game_title_animation_plan_verified = 1;
    value.game_title_graphic_index = 1u;
    value.game_title_presents_source_y = 137u;
    value.game_title_master_source_y = 80u;
    value.game_title_zoom_start_width = 320u;
    value.game_title_zoom_start_height = 80u;
    value.game_title_zoom_width_step = 16u;
    value.game_title_zoom_height_step = 4u;
    value.game_title_zoom_step_count = 18u;
    return value;
}

int main(void)
{
    static unsigned char source[320u * 200u];
    static unsigned char frame[320u * 200u];
    DM1_V1_FmtownsStartupReceipt startup = receipt();
    size_t i;

    for (i = 0u; i < sizeof(source); ++i) source[i] = (unsigned char)(i & 0x0fu);
    assert(!dm1_v1_fmtowns_title_compose_frame(NULL, source, 320u, 200u,
                                                0u, frame, sizeof(frame)));
    assert(!dm1_v1_fmtowns_title_compose_frame(&startup, source, 319u, 200u,
                                                0u, frame, sizeof(frame)));
    assert(!dm1_v1_fmtowns_title_compose_frame(&startup, source, 320u, 200u,
                                                19u, frame, sizeof(frame)));

    assert(dm1_v1_fmtowns_title_compose_frame(&startup, source, 320u, 200u,
                                               0u, frame, sizeof(frame)));
    /* First reverse-order frame is 48x12, centred at (136,74). */
    assert(frame[74u * 320u + 136u] == source[0]);
    assert(frame[90u * 320u] == source[137u * 320u]);
    assert(frame[40u * 320u] == 0u);

    assert(dm1_v1_fmtowns_title_compose_frame(&startup, source, 320u, 200u,
                                               17u, frame, sizeof(frame)));
    assert(frame[40u * 320u] == source[0]);
    assert(frame[119u * 320u] == 0u);

    assert(dm1_v1_fmtowns_title_compose_frame(&startup, source, 320u, 200u,
                                               18u, frame, sizeof(frame)));
    assert(frame[118u * 320u] == source[80u * 320u]);
    assert(frame[90u * 320u] == source[137u * 320u]);
    puts("PASS: dm1_v1_fmtowns_title");
    return 0;
}
