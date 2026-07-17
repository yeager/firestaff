#include "dm2_v1_gdat_door_side_frame_m11_consumer.h"
#include "dm2_v1_viewport_renderer.h"

#include <string.h>

static uint32_t hash_bytes(const uint8_t *bytes, size_t count)
{
    uint32_t hash = 2166136261u;
    while (count-- != 0u) { hash ^= *bytes++; hash *= 16777619u; }
    return hash;
}

static int same_surface(const DM2_V1_ViewportSurfaceSnapshot *a,
                        const DM2_V1_ViewportSurfaceSnapshot *b)
{
    return a && b && a->framebuffer == b->framebuffer && a->width == b->width &&
        a->height == b->height && a->stride == b->stride &&
        a->resolution == b->resolution && a->generation == b->generation;
}

static int matches(const DM2_V1_GdatDoorSideFrameM11Receipt *receipt,
                   const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
                   const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
                   const DM2_V1_ViewportState *owner,
                   DM2_V1_ViewportSurfaceSnapshot *out_surface)
{
    const DM2_V1_GdatDoorOverlayM11Command *command;
    DM2_V1_ViewportSurfaceSnapshot surface;
    int expected_field, expected_rect, expected_mirror;
    int expected_offset_x, expected_offset_y;

    if (!receipt || !plan || !composition || !owner || !receipt->valid ||
        !receipt->identity_hash || !plan->valid || !plan->command_hash ||
        receipt->command_index >= plan->command_count || !composition->valid ||
        !composition->no_draw || !composition->identity_hash ||
        !composition->session_identity || !composition->data_epoch ||
        composition->door_command_hash != plan->command_hash ||
        receipt->door_hash != plan->command_hash ||
        !owner->gdat_scene_control_ready || !owner->gdat_scene_control_hash ||
        owner->gdat_scene_control_hash != receipt->scene_hash ||
        composition->scene_command_hash != receipt->scene_hash ||
        owner->gdat_scene_colorkey > 15u ||
        owner->gdat_scene_colorkey != receipt->scene_color_key ||
        (!!owner->gdat_scene_movement_active) != (!!receipt->movement_active) ||
        receipt->composition_identity_hash != composition->identity_hash ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        !same_surface(&surface, &composition->surface_before) ||
        !same_surface(&surface, &composition->surface_after) ||
        surface.generation != receipt->surface_generation) return 0;

    command = &plan->commands[receipt->command_index];
    if (!dm2_v1_viewport_door_side_frame_source_for_movement(
            command->view_square,
            command->kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT ? 0 : 1,
            receipt->movement_active, &expected_field, &expected_rect,
            &expected_mirror, &expected_offset_x, &expected_offset_y) ||
        expected_offset_y != 4) return 0;
    /* c_gui_vp.cpp DRAW_DOOR_FRAMES: left is QUERY_TEMP_PICST(0,...,10,4)
     * and right is QUERY_TEMP_PICST(1,...,14,3), both at 0x40 scale. */
    if ((command->kind != DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT &&
         command->kind != DM2_V1_GDAT_DOOR_SIDE_FRAME_RIGHT) ||
        command->kind != receipt->kind ||
        command->category != DM2_GDAT_CATEGORY_GRAPHICSSET ||
        command->door_state != 0u || command->door_opening_dir != 0u ||
        command->door_open_pct != 0u ||
        ((command->kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_LEFT &&
          command->mirror_flip != 0u) ||
         (command->kind == DM2_V1_GDAT_DOOR_SIDE_FRAME_RIGHT &&
          command->mirror_flip != 1u)) ||
        (!!command->movement_active) != (!!receipt->movement_active) ||
        command->field != expected_field || command->rect_number != expected_rect ||
        command->mirror_flip != expected_mirror || !command->pixels || !command->width ||
        !command->height || command->source_x || command->source_y ||
        command->source_width != command->width ||
        command->source_height != command->height ||
        command->rect_width != command->width ||
        command->rect_height != command->height || !command->material_source_bytes ||
        !command->material_source_byte_count || !command->material_receipt_hash ||
        !command->raw_hash || !command->decoded_hash || !command->palette_hash ||
        !command->geometry_hash || !command->rect_number ||
        !command->rect_table_hash || !command->rect_row_hash ||
        command->rect_number != receipt->rect_number ||
        command->rect_x != receipt->destination_x ||
        command->rect_y != receipt->destination_y ||
        command->rect_width != receipt->width || command->rect_height != receipt->height ||
        command->raw_hash != receipt->raw_hash ||
        command->decoded_hash != receipt->decoded_hash ||
        command->palette_hash != receipt->palette_hash ||
        command->geometry_hash != receipt->geometry_hash ||
        command->rect_table_hash != receipt->rect_table_hash ||
        command->rect_row_hash != receipt->rect_row_hash ||
        receipt->destination_x < 0 || receipt->destination_y < 0 ||
        (uint32_t)receipt->destination_x + receipt->width > surface.width ||
        (uint32_t)receipt->destination_y + receipt->height > surface.height ||
        hash_bytes(command->material_source_bytes, command->material_source_byte_count) !=
            command->raw_hash ||
        hash_bytes(command->pixels, (size_t)command->width * command->height) !=
            command->decoded_hash ||
        hash_bytes(command->palette16, 16u) != command->palette_hash) return 0;

    if (out_surface) *out_surface = surface;
    return 1;
}

int dm2_v1_gdat_door_side_frame_m11_receipt_build(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan, uint8_t command_index,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatDoorSideFrameM11Receipt *out_receipt)
{
    const DM2_V1_GdatDoorOverlayM11Command *command;
    DM2_V1_GdatDoorSideFrameM11Receipt receipt;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint32_t hash = 2166136261u;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!plan || !composition || !owner || !plan->valid ||
        command_index >= plan->command_count ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface)) return 0;

    command = &plan->commands[command_index];
    memset(&receipt, 0, sizeof(receipt));
    receipt.valid = 1; receipt.command_index = command_index;
    receipt.kind = command->kind; receipt.movement_active = command->movement_active;
    receipt.scene_color_key = (uint8_t)owner->gdat_scene_colorkey;
    receipt.rect_number = command->rect_number;
    receipt.destination_x = command->rect_x; receipt.destination_y = command->rect_y;
    receipt.width = command->width; receipt.height = command->height;
    receipt.door_hash = plan->command_hash;
    receipt.scene_hash = owner->gdat_scene_control_hash;
    receipt.raw_hash = command->raw_hash; receipt.decoded_hash = command->decoded_hash;
    receipt.palette_hash = command->palette_hash; receipt.geometry_hash = command->geometry_hash;
    receipt.rect_table_hash = command->rect_table_hash;
    receipt.rect_row_hash = command->rect_row_hash;
    receipt.composition_identity_hash = composition->identity_hash;
    receipt.surface_generation = surface.generation;
    hash ^= receipt.door_hash; hash *= 16777619u;
    hash ^= receipt.scene_hash; hash *= 16777619u;
    hash ^= receipt.raw_hash; hash *= 16777619u;
    hash ^= receipt.decoded_hash; hash *= 16777619u;
    hash ^= receipt.palette_hash; hash *= 16777619u;
    hash ^= receipt.geometry_hash; hash *= 16777619u;
    hash ^= receipt.rect_number; hash *= 16777619u;
    hash ^= receipt.composition_identity_hash; hash *= 16777619u;
    receipt.identity_hash = hash ? hash : 1u;
    if (!matches(&receipt, plan, composition, owner, NULL)) return 0;
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_gdat_door_side_frame_m11_consume(
    const DM2_V1_GdatDoorSideFrameM11Receipt *receipt,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_ViewportState *owner)
{
    const DM2_V1_GdatDoorOverlayM11Command *command;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint16_t y;

    if (!matches(receipt, plan, composition, owner, &surface)) return 0;
    command = &plan->commands[receipt->command_index];
    for (y = 0u; y < receipt->height; ++y) {
        const uint8_t *source = command->pixels + (size_t)y * command->width;
        uint8_t *destination = surface.framebuffer +
            (size_t)(receipt->destination_y + y) * surface.stride +
            receipt->destination_x;
        uint16_t x;
        for (x = 0u; x < receipt->width; ++x) {
            const uint8_t index = source[x];
            if (index != receipt->scene_color_key) {
                const uint16_t destination_x = command->mirror_flip
                    ? (uint16_t)(receipt->width - 1u - x) : x;
                destination[destination_x] = command->palette16[index & 0x0fu];
            }
        }
    }
    return 1;
}
