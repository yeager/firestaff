#include "dm2_v1_gdat_door_vertical_m11_consumer.h"

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
    DM2_V1_GdatDoorVerticalM11Receipt receipt;
    DM2_V1_ViewportState viewport;
    DM2_V1_GdatDoorOverlayM11Command *command = &plan.commands[0];
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t before[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t raw[4] = { 1, 2, 3, 4 };
    uint8_t pixels[6] = { 1, 2, 3, 4, 0, 1 };
    int golden, geometry_reject, motion_reject;

    memset(framebuffer, 0xa5, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    plan.valid = 1; plan.command_count = 1; plan.command_hash = 7;
    command->kind = DM2_V1_GDAT_DOOR_PANEL;
    command->category = DM2_GDAT_CATEGORY_DOORS;
    command->pixels = pixels;
    command->width = command->source_width = command->rect_width = 2;
    command->height = command->source_height = command->rect_height = 3;
    command->rect_x = 12; command->rect_y = 22;
    command->door_state = 2; command->door_opening_dir = 1;
    command->door_open_pct = 50; command->color_key = 2;
    command->rect_number = 0x0ee4;
    command->material_source_bytes = raw; command->material_source_byte_count = 4;
    command->material_receipt_hash = 1; command->geometry_hash = 2;
    command->rect_table_hash = 3; command->rect_row_hash = 4;
    command->raw_hash = hash_bytes(raw, sizeof(raw));
    command->decoded_hash = hash_bytes(pixels, sizeof(pixels));
    command->palette16[1] = 0x31; command->palette16[2] = 0x32;
    command->palette16[3] = 0x33; command->palette16[4] = 0x34;
    command->palette_hash = hash_bytes(command->palette16, 16u);
    composition.valid = composition.no_draw = 1;
    composition.identity_hash = 9; composition.session_identity = 1;
    composition.data_epoch = 1; composition.door_command_hash = plan.command_hash;
    composition.surface_before = composition.surface_after = viewport.surface_snapshot;

    golden = dm2_v1_gdat_door_vertical_m11_receipt_build(
                 &plan, 0, &composition, &viewport, &receipt) &&
        dm2_v1_gdat_door_vertical_m11_consume(
            &receipt, &plan, &composition, &viewport) &&
        framebuffer[22 * DM2_VP_WIDTH + 12] == 0x31 &&
        framebuffer[22 * DM2_VP_WIDTH + 13] == 0xa5 &&
        framebuffer[23 * DM2_VP_WIDTH + 12] == 0x33 &&
        framebuffer[23 * DM2_VP_WIDTH + 13] == 0x34 &&
        framebuffer[24 * DM2_VP_WIDTH + 12] == 0x00 &&
        framebuffer[24 * DM2_VP_WIDTH + 13] == 0x31;
    memcpy(before, framebuffer, sizeof(before));
    ++command->rect_row_hash;
    geometry_reject = !dm2_v1_gdat_door_vertical_m11_consume(
        &receipt, &plan, &composition, &viewport) &&
        memcmp(before, framebuffer, sizeof(before)) == 0;
    --command->rect_row_hash;
    command->movement_active = 1;
    motion_reject = !dm2_v1_gdat_door_vertical_m11_receipt_build(
        &plan, 0, &composition, &viewport, &receipt);
    printf("golden=%d geometry=%d motion=%d\n", golden, geometry_reject,
           motion_reject);
    return golden && geometry_reject && motion_reject ? 0 : 1;
}
