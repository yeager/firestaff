#include "dm2_v1_gdat_door_side_frame_m11_consumer.h"

#include <stdio.h>
#include <string.h>

static uint32_t hash_bytes(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    while (count-- != 0u) { hash ^= *bytes++; hash *= 16777619u; }
    return hash;
}

int main(void)
{
    DM2_V1_GdatDoorOverlayM11CommandPlan plan = {0};
    DM2_V1_Dm2ViewportM11CompositionReceipt composition = {0};
    DM2_V1_GdatDoorSideFrameM11Receipt receipt;
    DM2_V1_ViewportState viewport;
    DM2_V1_GdatDoorOverlayM11Command *command = &plan.commands[0];
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t before[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t raw[4] = { 1, 2, 3, 4 };
    uint8_t pixels[4] = { 1, 2, 3, 4 };
    int golden, scene_reject, direction_reject;

    memset(framebuffer, 0xa5, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    viewport.gdat_scene_control_ready = 1;
    viewport.gdat_scene_control_hash = 10;
    viewport.gdat_scene_colorkey = 2;
    plan.valid = 1; plan.command_count = 1; plan.command_hash = 7;
    command->kind = DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT;
    command->category = DM2_GDAT_CATEGORY_GRAPHICSSET;
    command->view_square = DM2_SQ_D0C; command->field = 0xd3;
    command->pixels = pixels; command->width = command->source_width = command->rect_width = 2;
    command->height = command->source_height = command->rect_height = 2;
    command->rect_x = 12; command->rect_y = 22;
    command->rect_number = 5010; command->material_source_bytes = raw;
    command->material_source_byte_count = sizeof(raw); command->material_receipt_hash = 1;
    command->geometry_hash = 2; command->rect_table_hash = 3; command->rect_row_hash = 4;
    command->raw_hash = hash_bytes(raw, sizeof(raw));
    command->decoded_hash = hash_bytes(pixels, sizeof(pixels));
    command->palette16[1] = 0x31; command->palette16[2] = 0x32;
    command->palette16[3] = 0x33; command->palette16[4] = 0x34;
    command->palette_hash = hash_bytes(command->palette16, 16u);
    composition.valid = composition.no_draw = 1;
    composition.identity_hash = 9; composition.session_identity = 1;
    composition.data_epoch = 1; composition.scene_command_hash = 10;
    composition.door_command_hash = plan.command_hash;
    composition.surface_before = composition.surface_after = viewport.surface_snapshot;

    golden = dm2_v1_gdat_door_side_frame_m11_receipt_build(
                 &plan, 0, &composition, &viewport, &receipt) &&
        receipt.kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT &&
        dm2_v1_gdat_door_side_frame_m11_consume(
            &receipt, &plan, &composition, &viewport) &&
        framebuffer[22 * DM2_VP_WIDTH + 12] == 0x31 &&
        framebuffer[22 * DM2_VP_WIDTH + 13] == 0xa5 &&
        framebuffer[23 * DM2_VP_WIDTH + 12] == 0x33 &&
        framebuffer[23 * DM2_VP_WIDTH + 13] == 0x34;
    memcpy(before, framebuffer, sizeof(before));
    ++viewport.gdat_scene_control_hash;
    scene_reject = !dm2_v1_gdat_door_side_frame_m11_consume(
        &receipt, &plan, &composition, &viewport) &&
        memcmp(before, framebuffer, sizeof(before)) == 0;
    --viewport.gdat_scene_control_hash;
    command->mirror_flip = 1;
    direction_reject = !dm2_v1_gdat_door_side_frame_m11_receipt_build(
        &plan, 0, &composition, &viewport, &receipt);
    printf("golden=%d scene=%d direction=%d\n", golden, scene_reject,
           direction_reject);
    return golden && scene_reject && direction_reject ? 0 : 1;
}
