#include "dm2_v1_gdat_wall_b073_m11_consumer.h"

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
    return a && b && a->framebuffer == b->framebuffer &&
        a->width == b->width && a->height == b->height &&
        a->stride == b->stride && a->resolution == b->resolution &&
        a->generation == b->generation;
}

static int receipt_matches(const DM2_V1_GdatWallB073M11Receipt *r,
                           const DM2_V1_GdatB073InputReceipt *input,
                           const DM2_V1_GdatWallM11CommandPlan *wall,
                           const DM2_V1_GdatWallB073InterpreterReceipt *interpreter,
                           const DM2_V1_GdatWallB073OutputReceipt *output,
                           const DM2_V1_GdatWallTrimReceipt *trim,
                           const DM2_V1_GdatWallTrimM11Receipt *trim_m11,
                           const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
                           const DM2_V1_ViewportState *owner,
                           DM2_V1_ViewportSurfaceSnapshot *out_surface)
{
    const DM2_V1_GdatWallM11Command *command;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint32_t cache_hash;
    int16_t source_alpha;
    uint32_t clip_right, clip_bottom;

    if (!r || !input || !wall || !interpreter || !output || !trim || !trim_m11 ||
        !composition || !owner || !r->valid || !r->identity_hash ||
        !input->valid || !input->no_draw || !input->identity_hash ||
        !wall->valid || !wall->command_hash || r->command_index >= wall->command_count ||
        !interpreter->valid || !interpreter->no_draw || !interpreter->identity_hash ||
        !output->valid || !output->no_draw || !output->identity_hash ||
        !trim->valid || !trim->no_draw || !trim->identity_hash ||
        !trim_m11->valid || !trim_m11->no_draw || !trim_m11->identity_hash ||
        !composition->valid || !composition->no_draw || !composition->identity_hash ||
        !composition->session_identity || !composition->data_epoch ||
        !dm2_v1_viewport_surface_snapshot(owner, &surface) ||
        !same_surface(&surface, &trim->surface) ||
        !same_surface(&surface, &composition->surface_before) ||
        !same_surface(&surface, &composition->surface_after) ||
        r->surface_generation != surface.generation || r->wall_hash != wall->command_hash ||
        r->interpreter_identity_hash != interpreter->identity_hash ||
        r->output_identity_hash != output->identity_hash ||
        r->trim_identity_hash != trim_m11->identity_hash ||
        r->composition_identity_hash != composition->identity_hash ||
        trim_m11->wall_material_hash != wall->command_hash ||
        trim_m11->composition_identity_hash != composition->identity_hash ||
        !trim_m11->normal_scale || trim_m11->source_flip > 1u ||
        trim_m11->movement_offset_y != 0 ||
        interpreter->command_index != r->command_index ||
        interpreter->wall_hash != wall->command_hash ||
        interpreter->cache_palette_bytes_count != 256u ||
        output->command_index != r->command_index ||
        output->wall_hash != wall->command_hash ||
        output->cache_palette_bytes != interpreter->cache_palette_bytes ||
        output->cache_palette_bytes_count != 256u ||
        output->cache_identity != interpreter->cache_identity ||
        output->cache_allocation != interpreter->cache_allocation) return 0;

    command = &wall->commands[r->command_index];
    if (!command->pixels || !command->width || !command->height ||
        !command->raw_hash || !command->decoded_hash || !command->palette_hash ||
        !command->material_receipt_hash || !command->geometry_hash ||
        command->rect_number != trim_m11->rect_number ||
        command->view_square != trim_m11->view_square ||
        command->mirror_flip != r->source_flip ||
        trim_m11->source_flip != r->source_flip ||
        command->source_x != r->source_x || command->source_y != r->source_y ||
        command->destination_x != r->destination_x ||
        command->destination_y != r->destination_y ||
        command->source_width != r->width || command->source_height != r->height ||
        command->destination_width != r->width || command->destination_height != r->height ||
        (uint32_t)r->source_x + r->width > command->width ||
        (uint32_t)r->source_y + r->height > command->height ||
        !r->width || !r->height || !interpreter->cache_palette_bytes) return 0;
    source_alpha = (int16_t)input->input.alpha_mask;
    if (r->alpha_enabled != (source_alpha >= 0) ||
        r->alpha_index != (uint8_t)(input->input.alpha_mask & 0x0fu)) return 0;
    cache_hash = hash_bytes(interpreter->cache_palette_bytes, 256u);
    if (!cache_hash || cache_hash != interpreter->output_cache_hash ||
        cache_hash != r->cache_hash) return 0;
    clip_right = (uint32_t)surface.width - trim->right;
    clip_bottom = (uint32_t)surface.height - trim->bottom;
    if (r->destination_x < trim->left || r->destination_y < trim->top ||
        (uint32_t)r->destination_x + r->width > clip_right ||
        (uint32_t)r->destination_y + r->height > clip_bottom ||
        (uint32_t)r->destination_x + r->width > surface.width ||
        (uint32_t)r->destination_y + r->height > surface.height) return 0;
    if (out_surface) *out_surface = surface;
    return 1;
}

int dm2_v1_gdat_wall_b073_m11_receipt_build(
    const DM2_V1_GdatB073InputReceipt *input,
    const DM2_V1_GdatWallM11CommandPlan *wall_plan,
    uint8_t command_index,
    const DM2_V1_GdatWallB073InterpreterReceipt *interpreter,
    const DM2_V1_GdatWallB073OutputReceipt *output,
    const DM2_V1_GdatWallTrimReceipt *trim,
    const DM2_V1_GdatWallTrimM11Receipt *trim_m11,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    const DM2_V1_ViewportState *owner,
    DM2_V1_GdatWallB073M11Receipt *out_receipt)
{
    const DM2_V1_GdatWallM11Command *command;
    DM2_V1_GdatWallB073M11Receipt candidate;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint32_t hash = 2166136261u;
    int16_t source_alpha;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!input || !wall_plan || !interpreter || !output || !trim || !trim_m11 ||
        !composition || !owner || !input->valid || !input->no_draw ||
        !wall_plan->valid || command_index >= wall_plan->command_count ||
        !interpreter->valid || !output->valid || !trim->valid || !trim_m11->valid ||
        !composition->valid || !dm2_v1_viewport_surface_snapshot(owner, &surface)) return 0;
    command = &wall_plan->commands[command_index];
    if (!command->source_width || !command->source_height ||
        command->source_width != command->destination_width ||
        command->source_height != command->destination_height ||
        command->mirror_flip > 1u || command->movement_active ||
        command->movement_query_offset_y != 0) return 0;
    memset(&candidate, 0, sizeof(candidate));
    source_alpha = (int16_t)input->input.alpha_mask;
    candidate.valid = 1;
    candidate.command_index = command_index;
    candidate.source_flip = command->mirror_flip;
    candidate.alpha_enabled = source_alpha >= 0;
    candidate.alpha_index = (uint8_t)(input->input.alpha_mask & 0x0fu);
    candidate.source_x = command->source_x;
    candidate.source_y = command->source_y;
    candidate.destination_x = command->destination_x;
    candidate.destination_y = command->destination_y;
    candidate.width = command->source_width;
    candidate.height = command->source_height;
    candidate.wall_hash = wall_plan->command_hash;
    candidate.interpreter_identity_hash = interpreter->identity_hash;
    candidate.output_identity_hash = output->identity_hash;
    candidate.trim_identity_hash = trim_m11->identity_hash;
    candidate.composition_identity_hash = composition->identity_hash;
    candidate.surface_generation = surface.generation;
    candidate.cache_hash = hash_bytes(interpreter->cache_palette_bytes, 256u);
    hash ^= candidate.wall_hash; hash *= 16777619u;
    hash ^= candidate.source_flip; hash *= 16777619u;
    hash ^= candidate.interpreter_identity_hash; hash *= 16777619u;
    hash ^= candidate.output_identity_hash; hash *= 16777619u;
    hash ^= candidate.trim_identity_hash; hash *= 16777619u;
    hash ^= candidate.composition_identity_hash; hash *= 16777619u;
    hash ^= candidate.cache_hash; hash *= 16777619u;
    candidate.identity_hash = hash ? hash : 1u;
    if (!receipt_matches(&candidate, input, wall_plan, interpreter, output, trim,
                         trim_m11, composition, owner, NULL)) return 0;
    *out_receipt = candidate;
    return 1;
}

int dm2_v1_gdat_wall_b073_m11_consume(
    const DM2_V1_GdatWallB073M11Receipt *receipt,
    const DM2_V1_GdatB073InputReceipt *input,
    const DM2_V1_GdatWallM11CommandPlan *wall_plan,
    const DM2_V1_GdatWallB073InterpreterReceipt *interpreter,
    const DM2_V1_GdatWallB073OutputReceipt *output,
    const DM2_V1_GdatWallTrimReceipt *trim,
    const DM2_V1_GdatWallTrimM11Receipt *trim_m11,
    const DM2_V1_Dm2ViewportM11CompositionReceipt *composition,
    DM2_V1_ViewportState *owner)
{
    const DM2_V1_GdatWallM11Command *command;
    DM2_V1_ViewportSurfaceSnapshot surface;
    uint16_t row, column;

    if (!receipt_matches(receipt, input, wall_plan, interpreter, output, trim,
                         trim_m11, composition, owner, &surface)) return 0;
    command = &wall_plan->commands[receipt->command_index];
    /* SKWIN c_gfx_blit.cpp:495-548.  BLITMODE0 writes forward; BLITMODE1's
     * blitline_48_mi/mima reads forward but decrements destination X. */
    for (row = 0u; row < receipt->height; ++row) {
        const uint8_t *source = command->pixels +
            (size_t)(receipt->source_y + row) * command->width + receipt->source_x;
        uint8_t *destination = surface.framebuffer +
            (size_t)(receipt->destination_y + row) * surface.stride +
            receipt->destination_x;
        for (column = 0u; column < receipt->width; ++column) {
            uint8_t index = source[column];
            uint16_t destination_column = receipt->source_flip ?
                (uint16_t)(receipt->width - 1u - column) : column;
            if (!receipt->alpha_enabled || index != receipt->alpha_index)
                destination[destination_column] = interpreter->cache_palette_bytes[index];
        }
    }
    return 1;
}
