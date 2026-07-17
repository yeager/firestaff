#include "theron_v1_track02_dungeon_handoff_capture_plan_admission.h"

#include <stdio.h>
#include <string.h>

static void build_plan(Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;
    memset(plan, 0, sizeof(*plan));
    plan->valid = plan->cue_track_consumed = plan->cd_read_chain_consumed = 1;
    plan->loader_output_consumed = plan->palette_output_consumed = 1;
    plan->bitmap_transfer_consumed = plan->destination_record_consumed = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        plan->targets[i].route = (Theron_V1Track02CaptureTargetRoute)i;
        plan->targets[i].track02_variant = THERON_TRACK02_VARIANT_US_BIN;
        snprintf(plan->targets[i].track02_md5, sizeof(plan->targets[i].track02_md5),
                 "f23601102138f87c33025877767ebf76");
        plan->targets[i].cd_read_record = 0x510u;
        plan->targets[i].loader_output_bytes = 1u;
        plan->targets[i].loader_output_checksum = 1u;
        plan->targets[i].palette_output_identity = 1u;
        plan->targets[i].bitmap_transfer_capture_required = 1;
        plan->targets[i].bitmap_bytes = 1u;
        plan->targets[i].bitmap_identity = 1u;
        plan->targets[i].destination_record = 0x510u;
        plan->targets[i].destination_bytes = 1u;
        plan->targets[i].destination_identity = 1u;
    }
}

static int write_plan(const char *path, uint32_t identity, const char *trace_md5)
{
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    fprintf(file,
            "THERON_TRACK02_DUNGEON_HANDOFF_CAPTURE_PLAN_V1\n"
            "route=dungeon_handoff\ncue_path=/direct/theron.cue\n"
            "track02_payload_path=/direct/track02.bin\n"
            "track02_md5=f23601102138f87c33025877767ebf76\n"
            "system_card_path=/direct/syscard.pce\n"
            "system_card_md5=ff1a674273fe3540ccef576376407d1d\n"
            "layout_epoch=7\nreplay_final_record=510\n"
            "replay_final_raw_sector=510\n"
            "capture_target_plan_fnv1a=%x\nmednafen_trace_path=/capture/trace\n"
            "source_trace_md5=%s\ndescriptor_manifest_path=/capture/descriptor\n"
            "capture_artifact_path=/capture/artifact\npayload_policy=opaque_only\n"
            "decoder_policy=forbidden\nrender_policy=no_draw\n",
            identity, trace_md5);
    return fclose(file) == 0;
}

int main(void)
{
    Theron_V1Track02DungeonCapturePlanAdmissionReceipt receipt;
    Theron_V1Track02CampaignMediaDiscoveryReceipt media = {0};
    Theron_V1Track02CaptureTargetPlan plan;
    Theron_V1Track02LoaderTraceReplayConsistencyReceipt replay = {0};
    Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt artifact = {0};
    const char *path = "/tmp/firestaff-theron-dungeon-capture-plan";
    const char *trace_md5 = "11111111111111111111111111111111";
    uint32_t identity;
    if (!theron_v1_track02_dungeon_capture_plan_admit(
            NULL, NULL, NULL, NULL, NULL, 0u, 0u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_UNAVAILABLE)
        return 1;
    if (!theron_v1_track02_dungeon_capture_plan_admit(
            "/tmp/firestaff-missing-theron-dungeon-capture-plan", NULL, NULL,
            NULL, NULL, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_UNAVAILABLE)
        return 2;
    remove(path);
    build_plan(&plan);
    identity = theron_v1_track02_capture_target_plan_identity(&plan);
    if (!identity) return 3;
    media.status = THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY;
    media.exact_layout_bound = media.launchable_direct_media = 1;
    media.track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(media.track02_md5, sizeof(media.track02_md5), "%s",
             "f23601102138f87c33025877767ebf76");
    media.direct_media.status = THERON_V1_TRACK02_MEDIA_INTAKE_READY;
    media.direct_media.cue_consumed = media.direct_media.mode1_2352 = 1;
    media.direct_media.raw_trace_preparation_allowed = 1;
    media.direct_media.variant = media.track02_variant;
    snprintf(media.direct_media.track02_md5, sizeof(media.direct_media.track02_md5), "%s",
             media.track02_md5);
    snprintf(media.direct_media.media_path, sizeof(media.direct_media.media_path), "%s", "/direct/theron.cue");
    snprintf(media.direct_media.payload_path, sizeof(media.direct_media.payload_path), "%s", "/direct/track02.bin");
    replay.active = replay.direct_campaign_layout_consumed = 1;
    replay.track02_variant = media.track02_variant;
    replay.campaign_layout_epoch = 7u;
    replay.last_track02_record = 0x510u;
    replay.last_raw_sector = 0x510u;
    snprintf(replay.track02_md5, sizeof(replay.track02_md5), "%s", media.track02_md5);
    if (!write_plan(path, identity, trace_md5) ||
        !theron_v1_track02_dungeon_capture_plan_admit(path, &media, &plan, &replay,
                                                        NULL, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_CAPTURE_REQUIRED ||
        strcmp(receipt.source_trace_md5, trace_md5) || !receipt.presentation_no_draw)
        return 4;
    artifact.status = THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_READY;
    artifact.opaque_presentation_only = artifact.palette_output_consumed = 1;
    artifact.bitmap_transfer_consumed = 1;
    artifact.track02_variant = media.track02_variant;
    artifact.campaign_layout_epoch = 7u;
    artifact.campaign_media_scan_epoch = 3u;
    artifact.capture_target_plan_identity = identity;
    artifact.descriptor_record = 0x510u;
    snprintf(artifact.track02_md5, sizeof(artifact.track02_md5), "%s", media.track02_md5);
    snprintf(artifact.coalesced_trace_md5, sizeof(artifact.coalesced_trace_md5), "%s", trace_md5);
    if (!theron_v1_track02_dungeon_capture_plan_admit(path, &media, &plan, &replay,
                                                        &artifact, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_RESUME_READY)
        return 5;
    snprintf(artifact.coalesced_trace_md5, sizeof(artifact.coalesced_trace_md5), "%s",
             "22222222222222222222222222222222");
    if (!theron_v1_track02_dungeon_capture_plan_admit(path, &media, &plan, &replay,
                                                        &artifact, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_REJECTED)
        return 6;
    if (!write_plan(path, identity, "not-a-trace-md5") ||
        !theron_v1_track02_dungeon_capture_plan_admit(path, &media, &plan, &replay,
                                                        NULL, 7u, 3u, &receipt) ||
        receipt.status != THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_REJECTED)
        return 7;
    remove(path);
    puts("test_theron_v1_track02_dungeon_handoff_capture_plan_admission: PASS");
    return 0;
}
