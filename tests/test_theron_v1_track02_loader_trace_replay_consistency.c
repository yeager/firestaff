#include "theron_v1_track02_loader_trace_replay_consistency.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "failed: %s\n", #c); ++failures; } } while (0)

static void media_fixture(Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
                          Theron_V1Track02RawMediaIntakeReceipt *refreshed,
                          Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;
    memset(media, 0, sizeof(*media));
    memset(plan, 0, sizeof(*plan));
    media->status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    media->source = THERON_V1_TRACK02_CAMPAIGN_MEDIA_SOURCE_CUE;
    media->candidate_count = 1;
    media->exact_layout_bound = media->launchable_direct_media = 1;
    media->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media->track02_md5, sizeof(media->track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    media->direct_media.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    media->direct_media.cue_consumed = media->direct_media.mode1_2352 = 1;
    media->direct_media.raw_trace_preparation_allowed = 1;
    media->direct_media.variant = media->track02_variant;
    snprintf(media->direct_media.track02_md5, sizeof(media->direct_media.track02_md5),
             "%s", media->track02_md5);
    snprintf(media->direct_media.media_path, sizeof(media->direct_media.media_path),
             "/original/TQUS-Raw.cue");
    snprintf(media->direct_media.payload_path, sizeof(media->direct_media.payload_path),
             "/original/TQUS02.bin");
    media->direct_media.cue_index01_sector = 225u;
    media->direct_media.payload_bytes = 2352u * 0x600u;
    media->direct_media.sector_count = 0x600u;
    media->direct_media.first_user_data_offset = 225u * 2352u + 16u;
    media->direct_media.logical_user_data_window_bytes = 2048u;
    *refreshed = media->direct_media;
    plan->valid = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        plan->targets[i].route = (Theron_V1Track02CaptureTargetRoute)i;
        plan->targets[i].track02_variant = media->track02_variant;
        snprintf(plan->targets[i].track02_md5, sizeof(plan->targets[i].track02_md5),
                 "%s", media->track02_md5);
    }
}

static void ownership_fixture(Theron_V1Track02DynamicCdReadOwnershipReceipt *ownership,
                              uint32_t record)
{
    memset(ownership, 0, sizeof(*ownership));
    ownership->valid = ownership->raw_cue_bin_identity_consumed = 1;
    ownership->loader_trace_consumed = ownership->register_record_normalized = 1;
    ownership->raw_sector_window_owned = 1;
    ownership->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(ownership->track02_md5, sizeof(ownership->track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    ownership->track02_record = ownership->raw_sector = record;
    ownership->raw_offset = (size_t)record * 2352u;
    ownership->user_data_offset = ownership->raw_offset + 16u;
    ownership->user_data_bytes = 2048u;
    ownership->destination = 0x3800u;
    ownership->destination_span_checksum = record ^ 0x10203040u;
    ownership->full_payload_checksum = record ^ 0x50607080u;
}

int main(void)
{
    Theron_V1Track02CampaignMediaDiscoveryReceipt media;
    Theron_V1Track02RawMediaIntakeReceipt refreshed;
    Theron_V1Track02CaptureTargetPlan plan;
    Theron_V1Track02DynamicCdReadOwnershipReceipt first, second;
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt replay;

    media_fixture(&media, &refreshed, &plan);
    ownership_fixture(&first, 0x4e0u);
    ownership_fixture(&second, 0x4e1u);
    CHECK(theron_v1_track02_loader_trace_replay_consistency_begin(
        &media, &refreshed, &plan, 7u, &replay));
    CHECK(theron_v1_track02_loader_trace_replay_consistency_accept(
        &replay, &media, &refreshed, &plan, 7u, &first));
    CHECK(theron_v1_track02_loader_trace_replay_consistency_accept(
        &replay, &media, &refreshed, &plan, 7u, &second));
    CHECK(replay.active && replay.accepted_record_count == 2u &&
          replay.first_track02_record == first.track02_record &&
          replay.last_track02_record == second.track02_record &&
          !replay.level_object_semantics_allowed && !replay.dungeon_draw_allowed);
    CHECK(!theron_v1_track02_loader_trace_replay_consistency_accept(
        &replay, &media, &refreshed, &plan, 7u, &second));
    CHECK(!replay.active);

    CHECK(theron_v1_track02_loader_trace_replay_consistency_begin(
        &media, &refreshed, &plan, 7u, &replay));
    CHECK(theron_v1_track02_loader_trace_replay_consistency_accept(
        &replay, &media, &refreshed, &plan, 7u, &second));
    CHECK(!theron_v1_track02_loader_trace_replay_consistency_accept(
        &replay, &media, &refreshed, &plan, 7u, &first));

    CHECK(theron_v1_track02_loader_trace_replay_consistency_begin(
        &media, &refreshed, &plan, 8u, &replay));
    CHECK(!theron_v1_track02_loader_trace_replay_consistency_accept(
        &replay, &media, &refreshed, &plan, 7u, &first));
    CHECK(theron_v1_track02_loader_trace_replay_consistency_begin(
        &media, &refreshed, &plan, 9u, &replay));
    refreshed.sector_count++;
    CHECK(!theron_v1_track02_loader_trace_replay_consistency_accept(
        &replay, &media, &refreshed, &plan, 9u, &first));
    return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
