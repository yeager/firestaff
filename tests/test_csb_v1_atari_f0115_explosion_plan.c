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
    CSB_V1_AtariF0115ExplosionInput input;
    CSB_V1_AtariF0115ExplosionPlan plan;
    int failures = 0;

    memset(&input, 0, sizeof(input));
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

    input.explosion_type = 7; /* C007 poison cloud */
    input.attack = 0;
    input.centered = 1;
    input.view_square = 9; /* M609 D0C uses M636 pattern graphics. */
    failures += expect(csb_v1_atari_f0115_plan_explosion(&input, &plan) ==
                       CSB_V1_ATARI_F0115_EXPLOSION_D0C_PATTERN &&
                       plan.graphic_index == 357 && plan.d0c_pattern_size == 0,
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
