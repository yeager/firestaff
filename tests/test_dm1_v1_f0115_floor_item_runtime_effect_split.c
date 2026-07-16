#include "dm1_v1_projectile_explosion_render_pc34_compat.h"
#include "firestaff/dm1/v1/viewport/dm1_v1_viewport_d1l_d1r_f0115_thing_pass_pc34_compat.h"

#include <stdio.h>

static unsigned short thing(int type, int index, int cell)
{
    return (unsigned short)(((cell & 3) << 14) | ((type & 15) << 10) |
                            (index & 0x03ff));
}

int main(void)
{
    const DM1V1D1LD1RF0115LanePc34Data *lane;
    DM1_F0115ThingLayerReceiptPc34 floor;
    DM1V1D1LD1RF0115RuntimeThingReceiptPc34 projectile;
    unsigned short things[] = {
        thing(THING_TYPE_WEAPON, 2, 1),
        thing(THING_TYPE_PROJECTILE, 3, 1),
        thing(THING_TYPE_EXPLOSION, 4, 1),
        THING_ENDOFLIST
    };

    lane = dm1_v1_viewport_d1l_d1r_f0115_thing_pass_lane_at_pc34(0);
    if (!lane || !dm1_v1_f0115_thing_layer_receipt_pc34(
                     things, 4, 1, 0, &floor) ||
        !floor.valid || floor.visibleFloorItemCount != 1 ||
        floor.items != 1 || floor.projectiles != 0 || floor.explosions != 0 ||
        floor.ignoredStaticEffects != 2 ||
        !dm1_v1_viewport_d1l_d1r_f0115_runtime_thing_receipt_pc34(
            lane, 14, 1, 1, 1, &projectile) ||
        !projectile.valid || !projectile.input_valid ||
        !projectile.draw_projectile || projectile.suppress_projectile ||
        projectile.draw_item || !projectile.suppress_item ||
        !projectile.consumes_runtime_projectile_list ||
        !projectile.must_not_materialize_thing) {
        return 1;
    }
    puts("ok: DM1 F0115 separates floor material from runtime projectile effects");
    return 0;
}
