#include "dm2_v1_gdat_scene_m11_command.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_GdatSceneM11CommandPlan plan;
    DM2_V1_GdatSceneLightM11Receipt receipt;
    uint32_t original_hash;

    memset(&plan, 0, sizeof(plan));
    plan.valid = 1;
    plan.graphicsset = 3u;
    plan.command_hash = 0x13572468u;
    plan.commands[0].raw_hash = 1u;
    plan.commands[1].raw_hash = 2u;
    plan.ambient_light = 37u;
    plan.highest_light_level = 4u;
    plan.ambient_darkness = 3u;
    if (!dm2_v1_gdat_scene_light_m11_receipt(&plan, &receipt) ||
        !receipt.valid || receipt.ambient_light != 37u ||
        receipt.highest_light_level != 4u || receipt.ambient_darkness != 3u ||
        receipt.scene_control_hash != plan.command_hash || !receipt.receipt_hash) {
        fputs("ambient-light receipt mismatch\n", stderr);
        return 1;
    }
    original_hash = receipt.receipt_hash;
    ++plan.ambient_light;
    if (!dm2_v1_gdat_scene_light_m11_receipt(&plan, &receipt) ||
        receipt.ambient_light != 38u || receipt.receipt_hash == original_hash) {
        fputs("ambient-light identity was not bound\n", stderr);
        return 1;
    }
    puts("dm2 scene ambient-light receipt passed");
    return 0;
}
