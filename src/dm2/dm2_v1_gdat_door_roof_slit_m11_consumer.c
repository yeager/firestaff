#include "dm2_v1_gdat_door_roof_slit_m11_consumer.h"

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

static int roof_source_for_square(int view_square, int *out_field,
                                  int *out_rect_number)
{
    static const uint8_t fields[14] = {
        0xffu, 0xffu, 0xffu, 0x12u, 0x13u, 0x14u, 0x15u,
        0x16u, 0x17u, 0xffu, 0xffu, 0xffu, 0xffu, 0xffu
    };
    static const uint16_t rects[14] = {
        0u, 0u, 0u, 0x02f2u, 0x02f1u, 0x02f3u, 0x02efu,
        0x02eeu, 0x02f0u, 0u, 0u, 0u, 0u, 0u
    };
    const int cell = dm2_v1_viewport_skproject_cell_for_square(view_square);

    if (!out_field || !out_rect_number || cell < 0 || cell >= 14 ||
        fields[cell] == 0xffu || rects[cell] == 0u) return 0;
    *out_field = fields[cell];
    *out_rect_number = rects[cell];
    return 1;
}

static int matches(const DM2_V1_GdatDoorRoofSlitM11Receipt *receipt,
                   const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
                   const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
                   const DM2_V1_ViewportState *owner,
                   DM2_V1_ViewportSurfaceSnapshot *out_surface)
{
    const DM2_V1_GdatDoorOverlayM11Command *command;
    DM2_V1_ViewportSurfaceSnapshot surface;
    int expected_field, expected_rect;

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
        receipt->composition_identity_hash != composition->identity_hash ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        !same_surface(&surface, &composition->surface_before) ||
        !same_surface(&surface, &composition->surface_after) ||
        surface.generation != receipt->surface_generation) return 0;

    command = &plan->commands[receipt->command_index];
    if (!roof_source_for_square(command->view_square, &expected_field,
                                &expected_rect) ||
        command->kind != DM2_V1_GDAT_DOOR_ROOF_SLIT ||
        command->category != DM2_GDAT_CATEGORY_GRAPHICSSET ||
        command->field != expected_field || command->rect_number != expected_rect ||
        command->mirror_flip || !command->pixels || !command->width ||
        !command->height || command->source_x || command->source_y ||
        command->source_width != command->width ||
        command->source_height != command->height ||
        command->rect_width != command->width ||
        command->rect_height != command->height || !command->material_source_bytes ||
        !command->material_source_byte_count || !command->material_receipt_hash ||
        !command->raw_hash || !command->decoded_hash || !command->palette_hash ||
        !command->geometry_hash || !command->rect_table_hash ||
        !command->rect_row_hash || command->field != receipt->graphicsset_field ||
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

int dm2_v1_gdat_door_roof_slit_m11_receipt_build(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan, uint8_t command_index,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatDoorRoofSlitM11Receipt *out_receipt)
{
    const DM2_V1_GdatDoorOverlayM11Command *command;
    DM2_V1_GdatDoorRoofSlitM11Receipt receipt;
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
    receipt.scene_color_key = (uint8_t)owner->gdat_scene_colorkey;
    receipt.graphicsset_field = command->field; receipt.rect_number = command->rect_number;
    receipt.destination_x = command->rect_x; receipt.destination_y = command->rect_y;
    receipt.width = command->width; receipt.height = command->height;
    receipt.door_hash = plan->command_hash; receipt.scene_hash = owner->gdat_scene_control_hash;
    receipt.raw_hash = command->raw_hash; receipt.decoded_hash = command->decoded_hash;
    receipt.palette_hash = command->palette_hash; receipt.geometry_hash = command->geometry_hash;
    receipt.rect_table_hash = command->rect_table_hash; receipt.rect_row_hash = command->rect_row_hash;
    receipt.composition_identity_hash = composition->identity_hash;
    receipt.surface_generation = surface.generation;
    hash ^= receipt.door_hash; hash *= 16777619u; hash ^= receipt.scene_hash; hash *= 16777619u;
    hash ^= receipt.raw_hash; hash *= 16777619u; hash ^= receipt.decoded_hash; hash *= 16777619u;
    hash ^= receipt.palette_hash; hash *= 16777619u; hash ^= receipt.geometry_hash; hash *= 16777619u;
    hash ^= receipt.rect_number; hash *= 16777619u;
    hash ^= receipt.composition_identity_hash; hash *= 16777619u;
    receipt.identity_hash = hash ? hash : 1u;
    if (!matches(&receipt, plan, composition, owner, NULL)) return 0;
    *out_receipt = receipt;
    return 1;
}

int dm2_v1_gdat_door_roof_slit_m11_consume(
    const DM2_V1_GdatDoorRoofSlitM11Receipt *receipt,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_ViewportState *owner)
{
    const DM2_V1_GdatDoorOverlayM11Command *command;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint16_t y, x;

    if (!matches(receipt, plan, composition, owner, &surface)) return 0;
    command = &plan->commands[receipt->command_index];
    for (y = 0u; y < receipt->height; ++y) {
        const uint8_t *source = command->pixels + (size_t)y * command->width;
        uint8_t *destination = surface.framebuffer +
            (size_t)(receipt->destination_y + y) * surface.stride + receipt->destination_x;
        for (x = 0u; x < receipt->width; ++x)
            if (source[x] != receipt->scene_color_key)
                destination[x] = command->palette16[source[x] & 0x0fu];
    }
    return 1;
}
