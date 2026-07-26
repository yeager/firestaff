#include "theron_v1_track02_dungeon_handoff_capture_plan_admission.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

static int read_value(FILE *file, const char *key, char *out, size_t cap)
{
    char line[1024]; size_t n = strlen(key);
    if (!fgets(line, sizeof(line), file) || strncmp(line, key, n) ||
        line[n] != '=') return 0;
    line[strcspn(line, "\r\n")] = '\0';
    if (!line[n + 1u] || strlen(line + n + 1u) >= cap) return 0;
    strcpy(out, line + n + 1u); return 1;
}

static int is_lower_md5(const char *value)
{
    size_t i;
    if (!value || strlen(value) != 32u) return 0;
    for (i = 0u; i < 32u; ++i) {
        if (!((value[i] >= '0' && value[i] <= '9') ||
              (value[i] >= 'a' && value[i] <= 'f'))) return 0;
    }
    return 1;
}

int theron_v1_track02_dungeon_capture_plan_admit(
    const char *plan_path,
    const Theron_V1Track02CampaignMediaDiscoveryReceipt *media,
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02LoaderTraceReplayConsistencyReceipt *replay,
    const Theron_V1Track02DescriptorBitmapPaletteCaptureIntakeReceipt *artifact,
    uint32_t campaign_layout_epoch, uint32_t campaign_media_scan_epoch,
    Theron_V1Track02DungeonCapturePlanAdmissionReceipt *out)
{
    Theron_V1Track02DungeonCapturePlanAdmissionReceipt receipt = {0};
    struct stat st; FILE *file; char value[1024], md5[33], trace_md5[33];
    unsigned long epoch, record, sector, identity;
    uint32_t plan_identity;
    if (!out) return 0;
    *out = receipt;
    if (!plan_path || !plan_path[0] || stat(plan_path, &st) != 0) {
        receipt.status = THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_UNAVAILABLE;
        *out = receipt; return 1;
    }
    if (!S_ISREG(st.st_mode) || !media || !plan || !replay ||
        !(file = fopen(plan_path, "rb"))) goto rejected;
    if (!fgets(value, sizeof(value), file) ||
        strcmp(value, "THERON_TRACK02_DUNGEON_HANDOFF_CAPTURE_PLAN_V1\n") ||
        !read_value(file, "route", value, sizeof(value)) || strcmp(value, "dungeon_handoff") ||
        !read_value(file, "cue_path", value, sizeof(value)) ||
        !read_value(file, "track02_payload_path", value, sizeof(value)) ||
        !read_value(file, "track02_md5", md5, sizeof(md5)) ||
        !read_value(file, "system_card_path", value, sizeof(value)) ||
        !read_value(file, "system_card_md5", value, sizeof(value)) ||
        strcmp(value, "ff1a674273fe3540ccef576376407d1d") ||
        !read_value(file, "layout_epoch", value, sizeof(value)) ||
        sscanf(value, "%lu", &epoch) != 1 || epoch != campaign_layout_epoch ||
        !read_value(file, "replay_final_record", value, sizeof(value)) ||
        sscanf(value, "%lx", &record) != 1 ||
        !read_value(file, "replay_final_raw_sector", value, sizeof(value)) ||
        sscanf(value, "%lx", &sector) != 1 || record != sector || !record ||
        !read_value(file, "capture_target_plan_fnv1a", value, sizeof(value)) ||
        sscanf(value, "%lx", &identity) != 1 || !identity ||
        !read_value(file, "mednafen_trace_path", value, sizeof(value)) ||
        !read_value(file, "source_trace_md5", trace_md5, sizeof(trace_md5)) || !is_lower_md5(trace_md5) ||
        !read_value(file, "descriptor_manifest_path", value, sizeof(value)) ||
        !read_value(file, "capture_artifact_path", value, sizeof(value)) ||
        !read_value(file, "payload_policy", value, sizeof(value)) || strcmp(value, "opaque_only") ||
        !read_value(file, "decoder_policy", value, sizeof(value)) || strcmp(value, "forbidden") ||
        !read_value(file, "render_policy", value, sizeof(value)) || strcmp(value, "no_draw") ||
        fgetc(file) != EOF) { fclose(file); goto rejected; }
    fclose(file);
    plan_identity = theron_v1_track02_capture_target_plan_identity(plan);
    if (!campaign_media_scan_epoch || !plan_identity || identity != plan_identity ||
        media->status != THERON_V1_TRACK02_CAMPAIGN_MEDIA_READY ||
        !theron_v1_track02_campaign_media_direct_layout_current(
            media, &media->direct_media, plan) ||
        strcmp(md5, media->track02_md5) ||
        replay->campaign_layout_epoch != campaign_layout_epoch ||
        replay->last_track02_record != (uint32_t)record ||
        replay->last_raw_sector != (size_t)sector) goto rejected;
    receipt.status = THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_CAPTURE_REQUIRED;
    receipt.direct_cue_bin_consumed = receipt.system_card_required = 1;
    receipt.replay_tail_consumed = receipt.capture_plan_consumed = 1;
    receipt.opaque_artifact_required = receipt.presentation_no_draw = 1;
    receipt.track02_variant = media->track02_variant;
    snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", media->track02_md5);
    snprintf(receipt.source_trace_md5, sizeof(receipt.source_trace_md5), "%s", trace_md5);
    receipt.campaign_layout_epoch = campaign_layout_epoch;
    receipt.campaign_media_scan_epoch = campaign_media_scan_epoch;
    receipt.replay_final_record = (uint32_t)record;
    receipt.capture_target_plan_identity = plan_identity;
    if (artifact) {
        if (artifact->status != THERON_V1_TRACK02_DESCRIPTOR_BITMAP_PALETTE_CAPTURE_READY ||
            !artifact->opaque_presentation_only || !artifact->palette_output_consumed ||
            !artifact->bitmap_transfer_consumed ||
            artifact->track02_variant != media->track02_variant ||
            strcmp(artifact->track02_md5, media->track02_md5) ||
            strcmp(artifact->coalesced_trace_md5, trace_md5) ||
            artifact->campaign_layout_epoch != campaign_layout_epoch ||
            artifact->campaign_media_scan_epoch != campaign_media_scan_epoch ||
            artifact->capture_target_plan_identity != plan_identity ||
            artifact->descriptor_record != (uint32_t)record) goto rejected;
        receipt.status = THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_RESUME_READY;
        receipt.resume_route_ready = 1;
    }
    *out = receipt; return 1;
rejected:
    receipt.status = THERON_V1_TRACK02_DUNGEON_CAPTURE_PLAN_REJECTED;
    *out = receipt; return 1;
}
