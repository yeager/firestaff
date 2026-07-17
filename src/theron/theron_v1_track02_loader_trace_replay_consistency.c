#include "theron_v1_track02_loader_trace_replay_consistency.h"

#include <stdio.h>
#include <string.h>

static uint32_t fnv1a_u32(uint32_t hash, uint32_t value)
{
    unsigned int i;
    for (i = 0u; i < 4u; ++i) {
        hash ^= (value >> (i * 8u)) & 0xffu;
        hash *= 16777619u;
    }
    return hash;
}

static void clear_receipt(Theron_V1Track02LoaderTraceReplayConsistencyReceipt *state)
{
    if (state) memset(state, 0, sizeof(*state));
}

static int layout_current(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan)
{
    return theron_v1_track02_campaign_media_direct_layout_current(
        media, refreshed, plan);
}

int theron_v1_track02_loader_trace_replay_consistency_begin(
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan,
    uint32_t campaign_layout_epoch,
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt *out)
{
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt receipt = {0};

    if (!out) return 0;
    *out = receipt;
    if (campaign_layout_epoch == 0u || !layout_current(media, refreshed, plan)) return 0;
    receipt.active = 1;
    receipt.direct_campaign_layout_consumed = 1;
    receipt.track02_variant = media->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s",
             media->track02_md5);
    receipt.campaign_layout_epoch = campaign_layout_epoch;
    receipt.ordered_record_checksum = 2166136261u;
    *out = receipt;
    return 1;
}

int theron_v1_track02_loader_trace_replay_consistency_accept(
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt *state,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02RawMediaIntakeReceipt *refreshed,
    const Theron_V1Track02CaptureTargetPlan *plan,
    uint32_t campaign_layout_epoch,
    const Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership)
{
    uint32_t checksum;

    if (!state || !state->active || !ownership || campaign_layout_epoch == 0u ||
        campaign_layout_epoch != state->campaign_layout_epoch ||
        !layout_current(media, refreshed, plan) ||
        state->track02_variant != media->track02_variant ||
        strcmp(state->track02_md5, media->track02_md5) ||
        !ownership->valid || !ownership->raw_cue_bin_identity_consumed ||
        !ownership->loader_trace_consumed || !ownership->register_record_normalized ||
        !ownership->raw_sector_window_owned ||
        ownership->track02_variant != state->track02_variant ||
        strcmp(ownership->track02_md5, state->track02_md5) ||
        ownership->track02_record == 0u || ownership->raw_sector == 0u ||
        ownership->raw_offset == 0u || ownership->user_data_bytes != 2048u ||
        ownership->destination_span_checksum == 0u || ownership->full_payload_checksum == 0u ||
        ownership->level_object_semantics_allowed ||
        ownership->bitmap_palette_admission_allowed || ownership->pixel_decode_allowed ||
        ownership->dungeon_draw_allowed || ownership->fallback_visuals_allowed ||
        (state->accepted_record_count != 0u &&
         (ownership->track02_record <= state->last_track02_record ||
          ownership->raw_sector <= state->last_raw_sector))) {
        clear_receipt(state);
        return 0;
    }

    checksum = fnv1a_u32(state->ordered_record_checksum, ownership->track02_record);
    checksum = fnv1a_u32(checksum, (uint32_t)ownership->raw_sector);
    checksum = fnv1a_u32(checksum, ownership->full_payload_checksum);
    if (checksum == 0u || state->accepted_record_count == UINT32_MAX) {
        clear_receipt(state);
        return 0;
    }
    if (state->accepted_record_count == 0u) {
        state->first_track02_record = ownership->track02_record;
    }
    ++state->accepted_record_count;
    state->dynamic_cd_read_records_consumed = 1;
    state->last_track02_record = ownership->track02_record;
    state->last_raw_sector = ownership->raw_sector;
    state->ordered_record_checksum = checksum;
    return 1;
}
