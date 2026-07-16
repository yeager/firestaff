#include "dm1_v1_champion_mirror_pc34_compat.h"
#include "dm1_v1_projectile_explosion_render_pc34_compat.h"

#include <stdio.h>

static unsigned short thing(int type, int index, int cell)
{
    return (unsigned short)(((cell & 3) << 14) | ((type & 15) << 10) |
                            (index & 0x03ff));
}

int main(void)
{
    DM1_V1_ChampionMirrorFrontWallReceiptPc34 wall;
    DM1_V1_ChampionMirrorRenderReceiptPc34 mirror;
    DM1_F0115ThingLayerReceiptPc34 f0115;
    unsigned short things[] = {
        thing(THING_TYPE_WEAPON, 2, 1),
        thing(THING_TYPE_PROJECTILE, 3, 1),
        thing(THING_TYPE_EXPLOSION, 4, 1),
        THING_ENDOFLIST
    };

    /* ReDMCSB DUNGEON.C F0172 publishes the visible C127 wall payload
     * before DUNVIEW.C F0115 draws the cell's floor-object phase. */
    if (!DM1_V1_ChampionMirror_F0172FrontWallSensorReceiptPc34(
            127, 13, 4, 1, 1, &wall) ||
        !DM1_V1_ChampionMirror_BuildRenderReceiptPc34(&wall, &mirror) ||
        !mirror.valid || !mirror.drawChampionPortrait ||
        !mirror.suppressMaterializedItemPayload ||
        !dm1_v1_f0115_thing_layer_receipt_pc34(
            things, 4, 1, 0, &f0115) ||
        !f0115.valid || f0115.visibleFloorItemCount != 1 ||
        f0115.projectiles != 0 || f0115.explosions != 0 ||
        f0115.ignoredStaticEffects != 2 ||
        !dm1_v1_verify_f0115_draw_order(
            (int[]){DM1_F0115_LAYER_FLOOR_ITEMS,
                    DM1_F0115_LAYER_PROJECTILES}, 2)) {
        return 1;
    }
    puts("ok: DM1 HoC C127 precedes F0115 floor material without fallback effects");
    return 0;
}
