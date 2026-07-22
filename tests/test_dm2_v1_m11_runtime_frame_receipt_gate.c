/* Data-free M11 acceptance gate for the source-required DM2 viewport frame.
 * skproject binds GRAPHICSSET and dtPalette16 for one active map generation;
 * M11 must not present a receipt from another map, scene, or palette. */
#include "m11_dm2_runtime_frame_receipt_gate.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check(int condition, const char *label)
{
    if (condition) {
        fprintf(stderr, "PASS: %s\n", label);
    } else {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static DM2_V1_BootRuntimeRenderReceipt make_boot_receipt(void)
{
    DM2_V1_BootRuntimeRenderReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.runtime_m11_frame_receipt_consumed = 1;
    receipt.runtime_m11_frame_map_load_token = 42u;
    receipt.runtime_m11_frame_scene_control_hash = 0x53434e45u;
    receipt.runtime_m11_frame_scene_light_hash = 0x4c495447u;
    receipt.runtime_m11_frame_floor_material_hash = 0x464c4f52u;
    receipt.runtime_m11_frame_ceiling_material_hash = 0x4345494cu;
    receipt.runtime_m11_frame_wall_material_plan_hash = 0x57414c4cu;
    receipt.runtime_m11_frame_wall_material_plan_command_count = 10;
    receipt.runtime_m11_frame_door_material_plan_required = 1;
    receipt.runtime_m11_frame_door_material_plan_hash = 0x444f4f52u;
    receipt.runtime_m11_frame_door_material_plan_command_count = 4;
    receipt.runtime_m11_frame_door_material_plan_consumed = 1;
    receipt.runtime_m11_frame_hud_material_plan_required = 1;
    receipt.runtime_m11_frame_hud_material_plan_hash = 0x48554431u;
    receipt.runtime_m11_frame_hud_scene_control_hash = 0x53434e45u;
    receipt.runtime_m11_frame_hud_material_plan_command_count = 13;
    receipt.runtime_m11_frame_hud_material_plan_consumed = 1;
    receipt.runtime_m11_frame_creature_material_plan_required = 1;
    receipt.runtime_m11_frame_creature_material_plan_hash = 0x43524541u;
    receipt.runtime_m11_frame_creature_material_plan_command_count = 2;
    receipt.runtime_m11_frame_creature_material_plan_consumed = 1;
    receipt.runtime_m11_frame_projectile_material_plan_required = 1;
    receipt.runtime_m11_frame_projectile_material_plan_hash = 0x50524f4au;
    receipt.runtime_m11_frame_projectile_material_plan_command_count = 2;
    receipt.runtime_m11_frame_projectile_material_plan_consumed = 1;
    receipt.runtime_m11_frame_item_material_plan_required = 1;
    receipt.runtime_m11_frame_item_material_plan_hash = 0x4954454du;
    receipt.runtime_m11_frame_item_scene_control_hash = 0x53434e45u;
    receipt.runtime_m11_frame_item_material_plan_command_count = 3;
    receipt.runtime_m11_frame_item_material_plan_consumed = 1;
    receipt.runtime_m11_frame_weather_material_plan_required = 1;
    receipt.runtime_m11_frame_weather_material_plan_hash = 0x57454154u;
    receipt.runtime_m11_frame_weather_material_plan_command_count = 2;
    receipt.runtime_m11_frame_weather_material_plan_consumed = 1;
    receipt.runtime_m11_frame_teleporter_material_plan_required = 1;
    receipt.runtime_m11_frame_teleporter_material_plan_hash = 0x54454c45u;
    receipt.runtime_m11_frame_teleporter_material_plan_consumed = 1;
    receipt.runtime_m11_frame_floor_gfx_map_chip_material_plan_required = 1;
    receipt.runtime_m11_frame_floor_gfx_map_chip_material_plan_hash = 0x46474d43u;
    receipt.runtime_m11_frame_floor_gfx_map_chip_material_plan_consumed = 1;
    receipt.runtime_m11_frame_wall_gfx_map_chip_material_plan_required = 1;
    receipt.runtime_m11_frame_wall_gfx_map_chip_material_plan_hash = 0x57474d43u;
    receipt.runtime_m11_frame_wall_gfx_map_chip_material_plan_consumed = 1;
    receipt.runtime_m11_frame_door_map_chip_material_plan_required = 1;
    receipt.runtime_m11_frame_door_map_chip_material_plan_hash = 0x44474d43u;
    receipt.runtime_m11_frame_door_map_chip_material_plan_consumed = 1;
    receipt.runtime_m11_frame_palette_hash = 0x50414c31u;
    receipt.runtime_m11_frame_interface_action_palette_hash = 0x4143544eu;
    receipt.runtime_m11_frame_interface_action_palette_consumed = 1;
    receipt.runtime_m11_frame_interface_rect14_required = 1;
    receipt.runtime_m11_frame_interface_rect14_consumed = 1;
    receipt.runtime_m11_frame_interface_rect14_table_hash = 0x52313454u;
    receipt.runtime_m11_frame_interface_rect14_placement_hash = 0x52313450u;
    receipt.runtime_m11_frame_interface_rect14_row_count = 97u;
    receipt.runtime_render_asset_floor_ceiling_count = 2;
    receipt.runtime_render_fallback_floor_ceiling_count = 0;
    receipt.runtime_render_blocked_material_draw_count = 0;
    receipt.runtime_render_no_core_fallbacks = 1;
    return receipt;
}

static DM2_V1_ViewportM11FrameReceipt make_runtime_receipt(void)
{
    DM2_V1_ViewportM11FrameReceipt receipt;

    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1;
    receipt.m11_consume_frame = 1;
    receipt.source_materials_required = 1;
    receipt.map_load_token = 42u;
    receipt.scene_control_hash = 0x53434e45u;
    receipt.scene_light_hash = 0x4c495447u;
    receipt.floor_material_hash = 0x464c4f52u;
    receipt.ceiling_material_hash = 0x4345494cu;
    receipt.wall_material_plan_hash = 0x57414c4cu;
    receipt.wall_material_plan_command_count = 10;
    receipt.door_material_plan_required = 1;
    receipt.door_material_plan_hash = 0x444f4f52u;
    receipt.door_material_plan_command_count = 4;
    receipt.door_material_plan_consumed = 1;
    receipt.hud_material_plan_required = 1;
    receipt.hud_material_plan_hash = 0x48554431u;
    receipt.hud_scene_control_hash = 0x53434e45u;
    receipt.hud_material_plan_command_count = 13;
    receipt.hud_material_plan_consumed = 1;
    receipt.creature_material_plan_required = 1;
    receipt.creature_material_plan_hash = 0x43524541u;
    receipt.creature_material_plan_command_count = 2;
    receipt.creature_material_plan_consumed = 1;
    receipt.projectile_material_plan_required = 1;
    receipt.projectile_material_plan_hash = 0x50524f4au;
    receipt.projectile_material_plan_command_count = 2;
    receipt.projectile_material_plan_consumed = 1;
    receipt.item_material_plan_required = 1;
    receipt.item_material_plan_hash = 0x4954454du;
    receipt.item_scene_control_hash = 0x53434e45u;
    receipt.item_material_plan_command_count = 3;
    receipt.item_material_plan_consumed = 1;
    receipt.weather_material_plan_required = 1;
    receipt.weather_material_plan_hash = 0x57454154u;
    receipt.weather_material_plan_command_count = 2;
    receipt.weather_material_plan_consumed = 1;
    receipt.teleporter_material_plan_required = 1;
    receipt.teleporter_material_plan_hash = 0x54454c45u;
    receipt.teleporter_material_plan_consumed = 1;
    receipt.floor_gfx_map_chip_material_plan_required = 1;
    receipt.floor_gfx_map_chip_material_plan_hash = 0x46474d43u;
    receipt.floor_gfx_map_chip_material_plan_consumed = 1;
    receipt.wall_gfx_map_chip_material_plan_required = 1;
    receipt.wall_gfx_map_chip_material_plan_hash = 0x57474d43u;
    receipt.wall_gfx_map_chip_material_plan_consumed = 1;
    receipt.door_map_chip_material_plan_required = 1;
    receipt.door_map_chip_material_plan_hash = 0x44474d43u;
    receipt.door_map_chip_material_plan_consumed = 1;
    receipt.palette_hash = 0x50414c31u;
    receipt.interface_action_palette_hash = 0x4143544eu;
    receipt.interface_action_palette_consumed = 1;
    receipt.interface_rect14_required = 1;
    receipt.interface_rect14_consumed = 1;
    receipt.interface_rect14_table_hash = 0x52313454u;
    receipt.interface_rect14_placement_hash = 0x52313450u;
    receipt.interface_rect14_row_count = 97u;
    return receipt;
}

int main(void)
{
    DM2_V1_BootRuntimeRenderReceipt boot = make_boot_receipt();
    DM2_V1_ViewportM11FrameReceipt runtime = make_runtime_receipt();

    check(runtime.source_materials_required == 1 &&
              boot.runtime_m11_frame_map_load_token ==
                  runtime.map_load_token &&
              boot.runtime_m11_frame_scene_control_hash ==
                  runtime.scene_control_hash &&
              boot.runtime_m11_frame_palette_hash ==
                  runtime.palette_hash,
          "boot and viewport receipts retain one source-required GDAT identity");
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 1,
          "M11 presents the current verified atomic DM2 frame");

    runtime.map_load_token++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale DM2 map token");
    runtime = make_runtime_receipt();

    runtime.floor_gfx_map_chip_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale local FLOOR_GFX map-chip material receipt");
    runtime = make_runtime_receipt();

    runtime.floor_gfx_map_chip_material_plan_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed local FLOOR_GFX map-chip material plan");
    runtime = make_runtime_receipt();

    runtime.wall_gfx_map_chip_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale local WALL_GFX map-chip material receipt");
    runtime = make_runtime_receipt();

    runtime.wall_gfx_map_chip_material_plan_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed local WALL_GFX map-chip material plan");
    runtime = make_runtime_receipt();

    runtime.door_map_chip_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale local DOORS map-chip material receipt");
    runtime = make_runtime_receipt();

    runtime.door_map_chip_material_plan_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed local DOORS map-chip material plan");
    runtime = make_runtime_receipt();

    runtime.floor_material_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT floor material receipt");
    runtime = make_runtime_receipt();

    runtime.wall_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT wall material receipt");
    runtime = make_runtime_receipt();
    runtime.wall_material_plan_command_count++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a GDAT wall command-count mismatch");
    runtime = make_runtime_receipt();

    runtime.door_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT door material receipt");
    runtime = make_runtime_receipt();

    runtime.door_material_plan_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed GDAT door material plan");
    runtime = make_runtime_receipt();
    runtime.door_material_plan_command_count++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a GDAT door command-count mismatch");
    runtime = make_runtime_receipt();

    runtime.hud_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT HUD material receipt");
    runtime = make_runtime_receipt();

    runtime.hud_scene_control_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects HUD from another G1 scene");
    runtime = make_runtime_receipt();

    runtime.hud_material_plan_command_count--;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a partial GDAT HUD command plan");
    runtime = make_runtime_receipt();

    runtime.hud_material_plan_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed GDAT HUD material plan");
    runtime = make_runtime_receipt();

    runtime.creature_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT creature material receipt");
    runtime = make_runtime_receipt();

    runtime.creature_material_plan_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed GDAT creature material plan");
    runtime = make_runtime_receipt();
    runtime.creature_material_plan_command_count++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a GDAT creature command-count mismatch");
    runtime = make_runtime_receipt();
    runtime.projectile_material_plan_command_count++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a GDAT projectile command-count mismatch");
    runtime = make_runtime_receipt();
    runtime.item_material_plan_command_count++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a GDAT item command-count mismatch");
    runtime = make_runtime_receipt();

    runtime.item_scene_control_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects item icons from another G1 scene");
    runtime = make_runtime_receipt();
    runtime.weather_material_plan_command_count++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a GDAT weather command-count mismatch");
    runtime = make_runtime_receipt();

    runtime.teleporter_material_plan_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT teleporter material receipt");
    runtime = make_runtime_receipt();

    runtime.teleporter_material_plan_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed GDAT teleporter material plan");
    runtime = make_runtime_receipt();

    runtime.interface_action_palette_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT HUD action-palette receipt");
    runtime = make_runtime_receipt();

    runtime.interface_action_palette_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed GDAT HUD action palette");
    runtime = make_runtime_receipt();

    runtime.scene_control_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale DM2 scene hash");
    runtime = make_runtime_receipt();

    runtime.scene_light_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale GDAT scene/light receipt");
    runtime = make_runtime_receipt();

    runtime.palette_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale DM2 palette hash");
    runtime = make_runtime_receipt();

    runtime.valid = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an invalid complete-wall material receipt");
    runtime = make_runtime_receipt();

    runtime.source_materials_required = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a frame without source-owned wall materials");
    runtime = make_runtime_receipt();

    boot.runtime_render_asset_floor_ceiling_count = 1;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a receipt without both real GDAT planes");
    boot = make_boot_receipt();

    boot.runtime_render_blocked_material_draw_count = 1;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a blocked floor or ceiling material pass");
    boot = make_boot_receipt();

    runtime.interface_rect14_table_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale INTERFACE_GENERAL dt07/0x0A Rect14 table hash");
    runtime = make_runtime_receipt();

    runtime.interface_rect14_placement_hash++;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a stale INTERFACE_GENERAL dt07/0x0A Rect14 placement hash");
    runtime = make_runtime_receipt();

    runtime.interface_rect14_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects an unconsumed INTERFACE_GENERAL dt07/0x0A Rect14 table");
    runtime = make_runtime_receipt();

    runtime.interface_rect14_required = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a Rect14-required frame that claims no requirement");
    runtime = make_runtime_receipt();

    boot.runtime_m11_frame_receipt_consumed = 0;
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, &runtime) == 0,
          "M11 rejects a missing boot handoff receipt");
    check(M11_Dm2RuntimeFrameReceipt_ShouldPresent(NULL, &runtime) == 0 &&
              M11_Dm2RuntimeFrameReceipt_ShouldPresent(&boot, NULL) == 0,
          "M11 rejects absent receipt inputs without fallback");

    return failures ? 1 : 0;
}
