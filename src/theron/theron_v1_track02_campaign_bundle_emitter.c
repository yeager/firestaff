#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <fcntl.h>
#include <io.h>
#define CLOSE _close
#define FDOPEN _fdopen
#else
#include <fcntl.h>
#include <unistd.h>
#define CLOSE close
#define FDOPEN fdopen
#endif

#include "asset_status_m12.h"
#include "theron_v1_track02_campaign_bundle_emitter.h"
#include "theron_v1_track02_mednafen_trace_converter.h"
#include "theron_v1_track02_raw_media_intake.h"

static FILE *open_new(const char *path)
{
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0600);
    FILE *file;
    if (fd < 0) return NULL;
    file = FDOPEN(fd, "wb");
    if (!file) { CLOSE(fd); remove(path); }
    return file;
}

static int valid_plan(const Theron_V1Track02CaptureTargetPlan *plan)
{
    return plan && plan->valid && plan->cue_track_consumed && plan->cd_read_chain_consumed &&
        plan->loader_output_consumed && plan->palette_output_consumed && plan->bitmap_transfer_consumed &&
        plan->destination_record_consumed && !plan->level_object_semantics_allowed &&
        !plan->pixel_decode_allowed && !plan->render_allowed && !plan->fallback_visuals_allowed;
}

static int write_bundle(FILE *file, const Theron_V1Track02CaptureTargetPlan *plan,
                        const char *trace_md5, unsigned int campaign_route)
{
    size_t i;
    uint32_t plan_identity = theron_v1_track02_capture_target_plan_identity(plan);
    if (!plan_identity || fprintf(file, "THERON_TRACK02_CAPTURE_ARTIFACT_BUNDLE_V1\ntrack02_md5=%s\nmednafen_trace_md5=%s\ncapture_target_plan_fnv1a=%x\ncampaign_route=%u\n", plan->targets[0].track02_md5, trace_md5, plan_identity, campaign_route) < 0) return 0;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        const Theron_V1Track02CaptureTarget *t = &plan->targets[i];
        if (fprintf(file, "route=%zu cd_record=%x loader_offset=%zx loader_bytes=%zu loader_checksum=%x palette_identity=%x bitmap_offset=%zx bitmap_bytes=%zu bitmap_identity=%x destination_record=%x destination_offset=%zx destination_bytes=%zu destination_identity=%x\n", i, t->cd_read_record, t->loader_output_raw_offset, t->loader_output_bytes, t->loader_output_checksum, t->palette_output_identity, t->bitmap_raw_offset, t->bitmap_bytes, t->bitmap_identity, t->destination_record, t->destination_offset, t->destination_bytes, t->destination_identity) < 0) return 0;
    }
    return 1;
}

int theron_v1_track02_campaign_bundle_emit(
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02CampaignBundleEmitRequest *request,
    Theron_V1Track02CampaignBundleEmitReceipt *out)
{
    Theron_V1Track02CampaignBundleEmitReceipt receipt = {0};
    Theron_V1Track02RawMediaIntakeReceipt media;
    Theron_V1Track02MednafenTraceConvertReceipt trace;
    size_t i, j;
    if (!out) return 0;
    *out = receipt;
    if (!valid_plan(plan) || !request || !request->media_path || !request->expected_track02_md5 ||
        !request->mednafen_trace_path || !request->expected_mednafen_trace_md5 || !request->media_path[0] ||
        !request->expected_track02_md5[0] || !request->mednafen_trace_path[0] || !request->expected_mednafen_trace_md5[0]) goto rejected;
    if (!theron_v1_track02_raw_media_intake_discover(request->media_path, &media) ||
        media.status == THERON_V1_TRACK02_MEDIA_INTAKE_UNAVAILABLE) { receipt.status = THERON_V1_TRACK02_CAMPAIGN_EMIT_UNAVAILABLE; *out = receipt; return 1; }
    if (media.status != THERON_V1_TRACK02_MEDIA_INTAKE_READY || !media.raw_trace_preparation_allowed ||
        strcmp(media.track02_md5, request->expected_track02_md5) || strcmp(media.track02_md5, plan->targets[0].track02_md5) ||
        !theron_v1_track02_mednafen_trace_inspect_file(request->mednafen_trace_path, &trace) ||
        trace.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE) { receipt.status = THERON_V1_TRACK02_CAMPAIGN_EMIT_UNAVAILABLE; *out = receipt; return 1; }
    if (trace.status != THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED || strcmp(trace.source_trace_md5, request->expected_mednafen_trace_md5)) goto rejected;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        if (!request->bundle_path[i] || !request->bundle_path[i][0]) goto rejected;
        for (j = 0u; j < i; ++j) if (!strcmp(request->bundle_path[i], request->bundle_path[j])) goto rejected;
    }
    receipt.raw_media_verified = 1; receipt.mednafen_trace_verified = 1; receipt.route_selection_verified = 1;
    receipt.emitted_without_launch = 1; snprintf(receipt.track02_md5, sizeof(receipt.track02_md5), "%s", media.track02_md5);
    snprintf(receipt.mednafen_trace_md5, sizeof(receipt.mednafen_trace_md5), "%s", trace.source_trace_md5);
    if (request->dry_run) { receipt.status = THERON_V1_TRACK02_CAMPAIGN_EMIT_DRY_RUN_READY; *out = receipt; return 1; }
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        FILE *file = open_new(request->bundle_path[i]);
        int written = 0;
        if (file) {
            written = write_bundle(file, plan, trace.source_trace_md5, (unsigned int)i);
            if (fclose(file) != 0) written = 0;
        }
        if (!written || !m12_file_md5_hex(request->bundle_path[i], receipt.bundle_md5[i])) {
            remove(request->bundle_path[i]); goto rejected;
        }
    }
    receipt.status = THERON_V1_TRACK02_CAMPAIGN_EMIT_WRITTEN; *out = receipt; return 1;
rejected:
    receipt.status = THERON_V1_TRACK02_CAMPAIGN_EMIT_REJECTED; *out = receipt; return 1;
}
