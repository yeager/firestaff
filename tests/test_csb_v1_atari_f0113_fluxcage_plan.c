#include "csb_v1_atari_f0113_fluxcage_plan.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *description)
{
    if (!condition) fprintf(stderr, "FAIL: %s\n", description);
    return condition ? 0 : 1;
}

int main(void)
{
    CSB_V1_AtariF0113FluxcageInput input;
    CSB_V1_AtariF0113FluxcagePlan plan;
    int failures = 0;

    memset(&input, 0, sizeof(input));
    input.explosion_type = CSB_V1_ATARI_F0113_C050;
    input.view_square = 1; /* M601/D3L */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD,
                       "C050 D3L uses F0113 field route");
    {
        static const uint8_t d3l_aspect[8] =
            {1,63,0x0a,0x80,48,51,11,64};
        static const uint8_t d3l_frame[8] =
            {0,83,25,75,64,51,32,0};
        failures += expect(plan.graphic_index == 74 &&
                           plan.mask_graphic_index == 69 &&
                           plan.mask_index == 0 &&
                           plan.mask_flip_horizontal == 1 &&
                           plan.transparent_color == 10 &&
                           plan.source_byte_width == 48 &&
                           plan.source_pixel_width == 96 &&
                           plan.source_height == 51 &&
                           plan.base_last_unit == 63 &&
                           plan.last_unit_random_range == 2 &&
                           plan.first_unit_random_range == 32 &&
                           plan.source_x == 11 &&
                           memcmp(plan.field_aspect, d3l_aspect,
                                  sizeof(d3l_aspect)) == 0 &&
                           memcmp(plan.destination_frame, d3l_frame,
                                  sizeof(d3l_frame)) == 0,
                           "D3L selects MEDIA020 G0188/G0163 and C074/mask69");
    }

    input.view_square = 4; /* M604/D2L */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD &&
                       plan.field_aspect[0] == 1 &&
                       plan.field_aspect[3] == 0x81 &&
                       plan.mask_graphic_index == 70 &&
                       plan.mask_flip_horizontal == 1 &&
                       plan.source_byte_width == 40 &&
                       plan.source_height == 71 &&
                       plan.source_x == 5 &&
                       plan.destination_frame[0] == 0 &&
                       plan.destination_frame[1] == 74,
                       "D2L preserves G0188 mask index and G0163 frame");

    input.view_square = 7; /* M607/D1L */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD &&
                       plan.mask_graphic_index == 71 &&
                       plan.source_pixel_width == 64 &&
                       plan.source_height == 111 &&
                       plan.destination_frame[0] == 0 &&
                       plan.destination_frame[1] == 63,
                       "D1L selects authentic mask 71 at 64x111");

    input.view_square = 10; /* M610/D0L */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD &&
                       plan.mask_graphic_index == 72 &&
                       plan.source_pixel_width == 32 &&
                       plan.source_height == 136 &&
                       plan.destination_frame[0] == 0 &&
                       plan.destination_frame[1] == 31,
                       "D0L selects authentic mask 72 at 32x136");

    input.view_square = 11; /* M611/D0R */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD &&
                       plan.graphic_index == 74 &&
                       plan.mask_graphic_index == 72 &&
                       plan.mask_index == 3 &&
                       plan.mask_flip_horizontal == 0 &&
                       plan.source_byte_width == 16 &&
                       plan.source_height == 136 &&
                       plan.destination_frame[0] == 192 &&
                       plan.destination_frame[1] == 223,
                       "D0R uses the far-end MEDIA020 field and frame rows");

    input.view_square = 3; /* M603/D2C: G0188 0xFF means no mask. */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD &&
                       plan.graphic_index == 74 &&
                       plan.mask_graphic_index == -1 &&
                       plan.mask_index == -1 &&
                       plan.mask_flip_horizontal == 0 &&
                       plan.source_pixel_width == 0 && plan.source_height == 0 &&
                       plan.base_last_unit == 60 &&
                       plan.destination_frame[0] == 60 &&
                       plan.destination_frame[1] == 163,
                       "D2C 0xFF aspect draws C074 unmasked without flip");

    input.view_square = 9; /* M609/D0C: no mask, full-width field. */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD &&
                       plan.mask_graphic_index == -1 &&
                       plan.mask_flip_horizontal == 0 &&
                       plan.base_last_unit == 59 &&
                       plan.destination_frame[0] == 0 &&
                       plan.destination_frame[1] == 223 &&
                       plan.destination_frame[2] == 0 &&
                       plan.destination_frame[3] == 135,
                       "D0C 0xFF aspect draws C074 across its unmasked frame");

    input.view_square = 0; /* M600/D3C is source-invisible for C050. */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_NOT_DRAWN,
                       "C050 is not drawn at D3C");
    input.view_square = 1;
    input.door_front_view_pass = 1;
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_NOT_DRAWN,
                       "first door-front pass records but does not draw C050");
    input.door_front_view_pass = 2;
    input.suppress_during_endgame = 1;
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_NOT_DRAWN,
                       "endgame-suppressed C050 has no field draw");
    input.suppress_during_endgame = 0;
    input.explosion_type = 0; /* C000 is an ordinary F0115 sprite. */
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_NOT_DRAWN,
                       "non-C050 explosion does not enter F0113");
    input.explosion_type = CSB_V1_ATARI_F0113_C050;
    input.view_square = 12;
    failures += expect(csb_v1_atari_f0113_plan_fluxcage(&input, &plan) ==
                       CSB_V1_ATARI_F0113_FLUXCAGE_INVALID,
                       "rejects out-of-range MEDIA020 view square");

    if (failures) return 1;
    puts("PASS: CSB Atari MEDIA020 F0113 Fluxcage plan");
    return 0;
}
