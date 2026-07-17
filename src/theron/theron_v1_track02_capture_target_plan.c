#include <stdio.h>
#include <string.h>

#include "theron_v1_track02_capture_target_plan.h"

static uint32_t palette_identity(const Theron_V1Track02PaletteRouteReceipt *palette)
{
    uint32_t result = 2166136261u;
    const uint16_t address[] = {palette->vce_index_address, palette->vce_low_address, palette->vce_high_address};
    const uint8_t value[] = {palette->vce_index, palette->vce_low, palette->vce_high};
    size_t i;
    for (i = 0u; i < 3u; ++i) {
        result = (result ^ (uint8_t)address[i]) * 16777619u;
        result = (result ^ (uint8_t)(address[i] >> 8)) * 16777619u;
        result = (result ^ value[i]) * 16777619u;
    }
    return result;
}

static uint32_t plan_hash_byte(uint32_t hash, unsigned char value)
{
    return (hash ^ value) * 16777619u;
}

static uint32_t plan_hash_u32(uint32_t hash, uint32_t value)
{
    unsigned shift;
    for (shift = 0u; shift < 32u; shift += 8u) {
        hash = plan_hash_byte(hash, (unsigned char)(value >> shift));
    }
    return hash;
}

static uint32_t plan_hash_size(uint32_t hash, size_t value)
{
    unsigned shift;
    for (shift = 0u; shift < sizeof(value) * 8u; shift += 8u) {
        hash = plan_hash_byte(hash, (unsigned char)(value >> shift));
    }
    return hash;
}

static uint32_t plan_hash_text(uint32_t hash, const char *text)
{
    if (!text) return 0u;
    do {
        hash = plan_hash_byte(hash, (unsigned char)*text);
    } while (*text++);
    return hash;
}

uint32_t theron_v1_track02_capture_target_plan_identity(
    const Theron_V1Track02CaptureTargetPlan *plan)
{
    uint32_t hash = 2166136261u;
    size_t i;

    if (!plan || !plan->valid || plan->level_object_semantics_allowed ||
        plan->pixel_decode_allowed || plan->render_allowed ||
        plan->fallback_visuals_allowed) return 0u;
    hash = plan_hash_u32(hash, (uint32_t)plan->cue_track_consumed);
    hash = plan_hash_u32(hash, (uint32_t)plan->cd_read_chain_consumed);
    hash = plan_hash_u32(hash, (uint32_t)plan->loader_output_consumed);
    hash = plan_hash_u32(hash, (uint32_t)plan->palette_output_consumed);
    hash = plan_hash_u32(hash, (uint32_t)plan->bitmap_transfer_consumed);
    hash = plan_hash_u32(hash, (uint32_t)plan->destination_record_consumed);
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        const Theron_V1Track02CaptureTarget *target = &plan->targets[i];
        if (target->route != (Theron_V1Track02CaptureTargetRoute)i ||
            !target->track02_md5[0] || target->level_object_semantics_allowed ||
            target->pixel_decode_allowed || target->render_allowed ||
            target->fallback_visuals_allowed) return 0u;
        hash = plan_hash_u32(hash, (uint32_t)target->route);
        hash = plan_hash_u32(hash, (uint32_t)target->track02_variant);
        hash = plan_hash_text(hash, target->track02_md5);
        hash = plan_hash_u32(hash, target->cd_read_record);
        hash = plan_hash_size(hash, target->loader_output_raw_offset);
        hash = plan_hash_size(hash, target->loader_output_bytes);
        hash = plan_hash_u32(hash, target->loader_output_checksum);
        hash = plan_hash_u32(hash, target->palette_index_address);
        hash = plan_hash_u32(hash, target->palette_low_address);
        hash = plan_hash_u32(hash, target->palette_high_address);
        hash = plan_hash_u32(hash, target->palette_output_identity);
        hash = plan_hash_u32(hash, (uint32_t)target->bitmap_transfer_capture_required);
        hash = plan_hash_size(hash, target->bitmap_raw_offset);
        hash = plan_hash_size(hash, target->bitmap_bytes);
        hash = plan_hash_u32(hash, target->bitmap_identity);
        hash = plan_hash_u32(hash, target->destination_record);
        hash = plan_hash_size(hash, target->destination_offset);
        hash = plan_hash_size(hash, target->destination_bytes);
        hash = plan_hash_u32(hash, target->destination_identity);
    }
    return hash;
}

static void fill_common(Theron_V1Track02CaptureTarget *target,
                        Theron_V1Track02CaptureTargetRoute route,
                        const Theron_V1Track02RawMediaIntakeReceipt *media,
                        const Theron_V1RawLoaderTraceReceipt *loader,
                        const Theron_V1Track02PaletteRouteReceipt *palette,
                        const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *destination)
{
    target->route = route;
    target->track02_variant = media->variant;
    snprintf(target->track02_md5, sizeof(target->track02_md5), "%s", media->track02_md5);
    target->cd_read_record = loader->dynamic_cd_read_record;
    target->loader_output_raw_offset = loader->dynamic_cd_read_raw_offset;
    target->loader_output_bytes = loader->dynamic_cd_read_destination_span_bytes;
    target->loader_output_checksum = loader->dynamic_cd_read_destination_span_checksum;
    target->palette_index_address = palette->vce_index_address;
    target->palette_low_address = palette->vce_low_address;
    target->palette_high_address = palette->vce_high_address;
    target->palette_output_identity = palette_identity(palette);
    target->bitmap_transfer_capture_required = 1;
    target->destination_record = destination->loader_record;
    target->destination_offset = destination->dungeon_record_payload_offset;
    target->destination_bytes = destination->dungeon_record_byte_count;
    target->destination_identity = destination->dungeon_record_window_checksum;
}

int theron_v1_track02_capture_target_plan_build(
    const Theron_V1Track02RawMediaIntakeReceipt *media,
    const Theron_V1RawLoaderTraceReceipt *loader,
    const Theron_V1Track02PaletteRouteReceipt *palette,
    const Theron_V1BitmapCaptureRuntimeAdmissionReceipt *bitmap,
    const Theron_V1Track02CaptureTraceRuntimeAdmissionReceipt *destination,
    Theron_V1Track02CaptureTargetPlan *out)
{
    Theron_V1Track02CaptureTargetPlan plan = {0};
    Theron_V1Track02CaptureTarget *start;
    Theron_V1Track02CaptureTarget *soul;
    Theron_V1Track02CaptureTarget *dungeon;

    if (!out) return 0;
    *out = plan;
    if (!media || !loader || !palette || !bitmap || !destination ||
        media->status != THERON_V1_TRACK02_MEDIA_INTAKE_READY || !media->cue_consumed ||
        !media->mode1_2352 || !media->raw_trace_preparation_allowed ||
        !loader->valid || !loader->dynamic_cd_read_verified ||
        !loader->dynamic_cd_read_registers_verified ||
        !loader->dynamic_cd_read_destination_span_verified ||
        !loader->dynamic_cd_read_media_span_verified || !loader->stage2_dynamic_payload_verified ||
        !loader->dynamic_cd_read_destination_span_bytes || !loader->dynamic_cd_read_destination_span_checksum ||
        !palette->accepted || !palette->real_cd_verified || palette->render_allowed ||
        !bitmap->valid || !bitmap->source_to_runtime_verified ||
        bitmap->first_raw_offset > bitmap->last_raw_offset || bitmap->pixel_decode_verified ||
        bitmap->render_allowed || bitmap->dungeon_draw_allowed || bitmap->fallback_visuals_allowed ||
        !destination->valid || !destination->opaque_route_ready ||
        !destination->dungeon_record_byte_count || !destination->dungeon_record_window_checksum ||
        destination->level_field_decoder_allowed || destination->object_field_decoder_allowed ||
        destination->bitmap_palette_admission_allowed || destination->pixel_decode_allowed ||
        destination->dungeon_draw_allowed || destination->fallback_visuals_allowed ||
        media->variant != loader->variant || media->variant != palette->track02_variant ||
        media->variant != bitmap->track02_variant || media->variant != destination->track02_variant ||
        strcmp(media->track02_md5, loader->track02_md5) || strcmp(media->track02_md5, palette->track02_md5) ||
        strcmp(media->track02_md5, bitmap->track02_md5) || strcmp(media->track02_md5, destination->track02_md5) ||
        loader->dynamic_cd_read_record != bitmap->dynamic_cd_read_record) return 0;

    start = &plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_START];
    soul = &plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM];
    dungeon = &plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF];
    fill_common(start, THERON_V1_TRACK02_CAPTURE_TARGET_START, media, loader, palette, destination);
    fill_common(soul, THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM, media, loader, palette, destination);
    fill_common(dungeon, THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF, media, loader, palette, destination);
    start->bitmap_raw_offset = loader->dynamic_cd_read_raw_offset;
    start->bitmap_bytes = loader->dynamic_cd_read_destination_span_bytes;
    start->bitmap_identity = loader->dynamic_cd_read_destination_span_checksum;
    soul->bitmap_raw_offset = bitmap->first_raw_offset;
    soul->bitmap_bytes = bitmap->last_raw_offset - bitmap->first_raw_offset + 1u;
    soul->bitmap_identity = bitmap->bitmap_checksum;
    dungeon->bitmap_raw_offset = destination->dungeon_record_payload_offset;
    dungeon->bitmap_bytes = destination->dungeon_record_byte_count;
    dungeon->bitmap_identity = destination->dungeon_record_window_checksum;
    plan.valid = 1;
    plan.cue_track_consumed = 1;
    plan.cd_read_chain_consumed = 1;
    plan.loader_output_consumed = 1;
    plan.palette_output_consumed = 1;
    plan.bitmap_transfer_consumed = 1;
    plan.destination_record_consumed = 1;
    *out = plan;
    return 1;
}
