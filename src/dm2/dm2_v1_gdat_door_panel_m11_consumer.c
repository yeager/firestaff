#include "dm2_v1_gdat_door_panel_m11_consumer.h"

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

static int matches(const DM2_V1_GdatDoorPanelM11Receipt *r,
                   const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
                   const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
                   const DM2_V1_ViewportState *owner,
                   DM2_V1_ViewportSurfaceSnapshot *out_surface)
{
    const DM2_V1_GdatDoorOverlayM11Command *c;
    DM2_V1_ViewportSurfaceSnapshot surface;
    if (!r || !plan || !composition || !owner || !r->valid || !r->identity_hash ||
        !plan->valid || !plan->command_hash || r->command_index >= plan->command_count ||
        !composition->valid || !composition->no_draw || !composition->identity_hash ||
        !composition->session_identity || !composition->data_epoch ||
        composition->door_command_hash != plan->command_hash ||
        r->door_hash != plan->command_hash ||
        r->composition_identity_hash != composition->identity_hash ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        !same_surface(&surface, &composition->surface_before) ||
        !same_surface(&surface, &composition->surface_after) ||
        surface.generation != r->surface_generation) return 0;
    c = &plan->commands[r->command_index];
    if (c->kind != DM2_V1_GDAT_DOOR_PANEL || c->category != DM2_GDAT_CATEGORY_DOORS ||
        !c->pixels || !c->width || !c->height || !c->material_source_bytes ||
        !c->material_source_byte_count || !c->material_receipt_hash || !c->raw_hash ||
        !c->decoded_hash || !c->palette_hash || !c->geometry_hash ||
        c->door_state != 0u || c->door_opening_dir != 0u || c->door_open_pct != 0u ||
        c->movement_active || c->mirror_flip || c->palette_darkness ||
        c->palette_light_receipt_hash || c->palette_transform_hash ||
        c->source_x != r->source_x || c->source_y != r->source_y ||
        c->rect_x != r->destination_x || c->rect_y != r->destination_y ||
        c->source_width != r->width || c->source_height != r->height ||
        c->rect_width != r->width || c->rect_height != r->height ||
        c->color_key != r->color_key || c->raw_hash != r->raw_hash ||
        c->decoded_hash != r->decoded_hash || c->palette_hash != r->palette_hash ||
        c->geometry_hash != r->geometry_hash ||
        (uint32_t)r->source_x + r->width > c->width ||
        (uint32_t)r->source_y + r->height > c->height ||
        r->destination_x < 0 || r->destination_y < 0 ||
        (uint32_t)r->destination_x + r->width > surface.width ||
        (uint32_t)r->destination_y + r->height > surface.height ||
        hash_bytes(c->material_source_bytes, c->material_source_byte_count) != c->raw_hash ||
        hash_bytes(c->pixels, (size_t)c->width * c->height) != c->decoded_hash ||
        hash_bytes(c->palette16, 16u) != c->palette_hash) return 0;
    if (out_surface) *out_surface = surface;
    return 1;
}

int dm2_v1_gdat_door_panel_m11_receipt_build(
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan, uint8_t command_index,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner, DM2_V1_GdatDoorPanelM11Receipt *out)
{
    const DM2_V1_GdatDoorOverlayM11Command *c;
    DM2_V1_GdatDoorPanelM11Receipt candidate;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint32_t hash = 2166136261u;
    if (!out) return 0;
    memset(out, 0, sizeof(*out));
    if (!plan || !composition || !owner || !plan->valid ||
        command_index >= plan->command_count ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface)) return 0;
    c = &plan->commands[command_index];
    memset(&candidate, 0, sizeof(candidate));
    candidate.valid = 1; candidate.command_index = command_index;
    candidate.color_key = (uint8_t)c->color_key;
    candidate.source_x = c->source_x; candidate.source_y = c->source_y;
    candidate.destination_x = c->rect_x; candidate.destination_y = c->rect_y;
    candidate.width = c->source_width; candidate.height = c->source_height;
    candidate.door_hash = plan->command_hash; candidate.raw_hash = c->raw_hash;
    candidate.decoded_hash = c->decoded_hash; candidate.palette_hash = c->palette_hash;
    candidate.geometry_hash = c->geometry_hash;
    candidate.composition_identity_hash = composition->identity_hash;
    candidate.surface_generation = surface.generation;
    hash ^= candidate.door_hash; hash *= 16777619u;
    hash ^= candidate.raw_hash; hash *= 16777619u;
    hash ^= candidate.decoded_hash; hash *= 16777619u;
    hash ^= candidate.palette_hash; hash *= 16777619u;
    hash ^= candidate.geometry_hash; hash *= 16777619u;
    hash ^= candidate.composition_identity_hash; hash *= 16777619u;
    candidate.identity_hash = hash ? hash : 1u;
    if (!matches(&candidate, plan, composition, owner, NULL)) return 0;
    *out = candidate;
    return 1;
}

int dm2_v1_gdat_door_panel_m11_consume(
    const DM2_V1_GdatDoorPanelM11Receipt *receipt,
    const DM2_V1_GdatDoorOverlayM11CommandPlan *plan,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_ViewportState *owner)
{
    const DM2_V1_GdatDoorOverlayM11Command *c;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint16_t y, x;
    if (!matches(receipt, plan, composition, owner, &surface)) return 0;
    c = &plan->commands[receipt->command_index];
    /* SKWIN c_gfx_blit.cpp:495-548 BLITMODE0 4-to-8: forward source rows
     * and the source DOORS colour key leave destination bytes untouched. */
    for (y = 0u; y < receipt->height; ++y) {
        const uint8_t *source = c->pixels +
            (size_t)(receipt->source_y + y) * c->width + receipt->source_x;
        uint8_t *destination = surface.framebuffer +
            (size_t)(receipt->destination_y + y) * surface.stride + receipt->destination_x;
        for (x = 0u; x < receipt->width; ++x)
            if (source[x] != receipt->color_key)
                destination[x] = c->palette16[source[x] & 0x0fu];
    }
    return 1;
}
