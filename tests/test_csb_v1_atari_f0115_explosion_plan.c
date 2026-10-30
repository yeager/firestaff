#include "csb_v1_atari_f0115_explosion_plan.h"

#include <stdio.h>
#include <string.h>

static int expect(int ok, const char *name)
{
    if (!ok) fprintf(stderr, "FAIL: %s\n", name);
    return ok ? 0 : 1;
}

int main(void)
{
    uint8_t c558[4720] = {0};
    CSB_V1_AtariF0115ExplosionInput input;
    CSB_V1_AtariF0115ExplosionPlan plan;
    int failures = 0;

    memset(&input, 0, sizeof(input));
    c558[1558 + 3 * 6] = 9;
    c558[1558 + 3 * 6 + 1] = 54;
    c558[1558 + 3 * 6 + 2] = 32;
    c558[1558 + 3 * 6 + 3] = 15;
    c558[1558 + 3 * 6 + 4] = 0x01;
    c558[1558 + 3 * 6 + 5] = 0x12;
    input.c558 = c558;
    input.c558_size = sizeof(c558);
    input.explosion_type = 0; /* C000 fireball */
    input.attack = 255;
    input.centered = 1;
    input.view_square = 0; /* M600 D3C */
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_SPRITE,
                       "D3C fireball route");
    failures += expect(plan.explosion_coordinate_index == 3 &&
                       plan.aspect_index == 0 && plan.graphic_index == 348 &&
                       plan.source_byte_width == 80 && plan.source_height == 111 &&
                       plan.scale_32 == 16 && plan.draw_byte_width == 40 &&
                       plan.draw_height == 55 && plan.center_x == 111 &&
                       plan.center_y == 50 && plan.transparent_color == 10,
                       "D3C fireball matches MEDIA020 G0211/G0216/G0225");

    input.explosion_type = 40; /* C040 smoke reuses poison pixels. */
    input.attack = 127;
    input.centered = 0;
    input.cell = 1;
    input.party_direction = 0;
    input.view_square = 3; /* M603 D2C */
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_SPRITE,
                       "D2C smoke route");
    failures += expect(plan.aspect_index == 3 && plan.graphic_index == 350 &&
                       plan.smoke_palette && plan.center_x == 131 &&
                       plan.center_y == 53,
                       "D2C smoke uses poison native bitmap, G0212, G0226 right cell");

    input.explosion_type = 101; /* C101 rebirth step 2 uses G0227. */
    {
        static const int expected_g0227[7][3] = {
            {113,57,12},{24,57,12},{195,57,12},{111,63,16},
            {12,63,16},{213,63,16},{112,76,24}
        };
        int square;
        input.party_direction = 2;
        for (square = 0; square < 7; ++square) {
            input.view_square = square;
            input.last_view_cell = square & 3;
            input.cell = (input.last_view_cell + input.party_direction) & 3;
            failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                               CSB_V1_ATARI_F0115_EXPLOSION_SPRITE &&
                               plan.graphic_index == 348 && plan.aspect_index == 0 &&
                               plan.scale_32 == expected_g0227[square][2] &&
                               plan.center_x == expected_g0227[square][0] &&
                               plan.center_y == expected_g0227[square][1],
                               "C101 rebirth matches each G0227 source row");
        }
    }
    input.view_square = 6; /* M606 D1C */
    input.last_view_cell = 2;
    input.party_direction = 0;
    input.cell = 1;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN,
                       "C101 rebirth only draws in the last visible absolute cell");
    input.cell = 0;
    input.view_square = 7; /* M607 D1L is outside the G0227 range. */
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN,
                       "C101 rebirth is clipped after D1C");

    input.explosion_type = 100; /* C100 uses lightning G0210/G0228. */
    input.view_square = 0;
    input.party_direction = 2;
    input.last_view_cell = 1;
    input.cell = 3;
    {
        static const int expected_g0228[7][3] = {
            {112,53,15},{24,53,15},{194,53,15},{112,59,20},
            {15,59,20},{208,59,20},{112,70,32}
        };
        int square;
        for (square = 0; square < 7; ++square) {
            input.view_square = square;
            failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                               CSB_V1_ATARI_F0115_EXPLOSION_SPRITE &&
                               plan.uses_projectile_bitmap &&
                               plan.graphic_index == 326 &&
                               plan.aspect_index == 3 &&
                               plan.source_byte_width == 32 &&
                               plan.source_height == 15 &&
                               plan.scale_32 == expected_g0228[square][2] &&
                               plan.center_x == expected_g0228[square][0] &&
                               plan.center_y == expected_g0228[square][1],
                               "C100 rebirth matches G0210 lightning and G0228");
        }
    }
    input.view_square = 6;
    input.cell = 2;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN,
                       "C100 rebirth only draws in the final visible cell");
    input.cell = 3;
    input.view_square = 7;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN,
                       "C100 rebirth is clipped after D1C");
    input.view_square = 0;
    c558[1558 + 3 * 6] = 8;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_INVALID,
                       "C100 rejects a mismatched G0210 aspect record");

    input.explosion_type = 7; /* C007 poison cloud */
    input.attack = 0;
    input.centered = 1;
    input.view_square = 9; /* M609 D0C uses M636 pattern graphics. */
    input.d0c_last_unit = 89;
    input.d0c_first_unit = 12;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_D0C_PATTERN &&
                       plan.graphic_index == 357 && plan.d0c_pattern_size == 0 &&
                       plan.d0c_last_unit == 89 && plan.d0c_first_unit == 12,
                       "D0C poison pattern small graphic");
    input.attack = 128;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_D0C_PATTERN &&
                       plan.graphic_index == 359 && plan.d0c_pattern_size == 3,
                       "D0C poison pattern large graphic");

    input.explosion_type = 50; /* C050 fluxcage is a separate F0113 pass. */
    input.view_square = 1;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_FLUXCAGE,
                       "Fluxcage selects F0113 route");
    input.view_square = 0; /* F0115 only collects Fluxcage from D3L onward. */
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_NOT_DRAWN,
                       "D3C does not admit Fluxcage");

    input.view_square = 12;
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_INVALID,
                       "reject out-of-range MEDIA020 view square");
    if (failures) return 1;
    puts("PASS: CSB Atari F0115 explosion source plan");
    return 0;
}
