#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "asset_status_m12.h"
#include "theron_v1_track02_capture_artifact_importer.h"

#define THERON_V1_CAPTURE_ARTIFACT_MAX_BYTES 8192u

static int direct_regular_file(const char *path)
{
    struct stat stat_value;
#if !defined(_WIN32)
    struct stat link_value;
    if (lstat(path, &link_value) != 0 || S_ISLNK(link_value.st_mode)) return 0;
#endif
    return path && path[0] && stat(path, &stat_value) == 0 && S_ISREG(stat_value.st_mode);
}

static int read_line(FILE *file, char *line, size_t capacity)
{
    size_t length;
    if (!fgets(line, capacity, file)) return 0;
    length = strlen(line);
    while (length && (line[length - 1] == '\n' || line[length - 1] == '\r')) line[--length] = '\0';
    return 1;
}

static int md5_line_matches(const char *line, const char *key, char md5[33])
{
    int consumed = 0;
    return sscanf(line, key, md5, &consumed) == 1 && line[consumed] == '\0';
}

static int route_matches(const char *line, unsigned int route,
                         const Theron_V1Track02CaptureTarget *target)
{
    unsigned int parsed_route, cd_record, loader_checksum, palette_identity;
    unsigned int bitmap_identity, destination_record, destination_identity;
    size_t loader_offset, loader_bytes, bitmap_offset, bitmap_bytes;
    size_t destination_offset, destination_bytes;
    int consumed = 0;
    if (sscanf(line,
               "route=%u cd_record=%x loader_offset=%zx loader_bytes=%zu loader_checksum=%x palette_identity=%x bitmap_offset=%zx bitmap_bytes=%zu bitmap_identity=%x destination_record=%x destination_offset=%zx destination_bytes=%zu destination_identity=%x%n",
               &parsed_route, &cd_record, &loader_offset, &loader_bytes,
               &loader_checksum, &palette_identity, &bitmap_offset, &bitmap_bytes,
               &bitmap_identity, &destination_record, &destination_offset,
               &destination_bytes, &destination_identity, &consumed) != 13 || line[consumed] != '\0') return 0;
    return parsed_route == route && target->route == (Theron_V1Track02CaptureTargetRoute)route &&
        cd_record && loader_bytes && loader_checksum && palette_identity &&
        bitmap_bytes && bitmap_identity && destination_record && destination_bytes &&
        destination_identity &&
        cd_record == target->cd_read_record && loader_offset == target->loader_output_raw_offset &&
        loader_bytes == target->loader_output_bytes && loader_checksum == target->loader_output_checksum &&
        palette_identity == target->palette_output_identity && bitmap_offset == target->bitmap_raw_offset &&
        bitmap_bytes == target->bitmap_bytes && bitmap_identity == target->bitmap_identity &&
        destination_record == target->destination_record && destination_offset == target->destination_offset &&
        destination_bytes == target->destination_bytes && destination_identity == target->destination_identity;
}

static int envelope_plan_complete(const Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;
    if (!plan || !plan->valid || !plan->cue_track_consumed ||
        !plan->cd_read_chain_consumed || !plan->loader_output_consumed ||
        !plan->palette_output_consumed || !plan->bitmap_transfer_consumed ||
        !plan->destination_record_consumed || plan->level_object_semantics_allowed ||
        plan->pixel_decode_allowed || plan->render_allowed ||
        plan->fallback_visuals_allowed) return 0;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        const Theron_V1Track02CaptureTarget *target = &plan->targets[i];
        if (target->route != (Theron_V1Track02CaptureTargetRoute)i ||
            target->track02_variant != plan->targets[0].track02_variant ||
            strcmp(target->track02_md5, plan->targets[0].track02_md5) ||
            theron_v1_track02_variant_for_md5(target->track02_md5) != target->track02_variant ||
            !target->cd_read_record ||
            !target->loader_output_bytes || !target->loader_output_checksum ||
            !target->palette_output_identity || !target->bitmap_bytes ||
            !target->bitmap_identity || !target->destination_record ||
            !target->destination_bytes || !target->destination_identity ||
            target->level_object_semantics_allowed || target->pixel_decode_allowed ||
            target->render_allowed || target->fallback_visuals_allowed) return 0;
    }
    return 1;
}

int theron_v1_track02_capture_artifact_import(
    const Theron_V1Track02CaptureTargetPlan *plan,
    const Theron_V1Track02CaptureArtifactImportRequest *request,
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt *out)
{
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt receipt = {0};
    Theron_V1Track02MednafenTraceConvertReceipt trace = {0};
    char bundle_md5[33], final_bundle_md5[33], line[1024], manifest_trace_md5[33];
    FILE *bundle;
    long bytes;
    unsigned int campaign_route, artifact_plan_identity, descriptor_selector;
    uint32_t descriptor_source_hash;
    size_t descriptor_ordinal;
    int campaign_route_consumed;
    uint32_t plan_identity;
    size_t i;

    if (!out) return 0;
    *out = receipt;
    if (!plan || !request || !request->bundle_path || !request->expected_bundle_md5 ||
        !request->mednafen_trace_path || !request->expected_mednafen_trace_md5 ||
        !request->bundle_path[0] || !request->expected_bundle_md5[0] ||
        !request->mednafen_trace_path[0] || !request->expected_mednafen_trace_md5[0]) return 1;
    if (!direct_regular_file(request->bundle_path) || !direct_regular_file(request->mednafen_trace_path)) goto rejected;
    plan_identity = theron_v1_track02_capture_target_plan_identity(plan);
    if (!plan_identity || !envelope_plan_complete(plan) ||
        !m12_file_md5_hex(request->bundle_path, bundle_md5) || strcmp(bundle_md5, request->expected_bundle_md5)) goto rejected;
    if (!theron_v1_track02_mednafen_trace_inspect_file(request->mednafen_trace_path, &trace) ||
        trace.status == THERON_V1_TRACK02_MEDNAFEN_TRACE_UNAVAILABLE) return 1;
    if (trace.status != THERON_V1_TRACK02_MEDNAFEN_TRACE_INSPECTED ||
        strcmp(trace.source_trace_md5, request->expected_mednafen_trace_md5)) goto rejected;
    bundle = fopen(request->bundle_path, "rb");
    if (!bundle || fseek(bundle, 0L, SEEK_END) != 0 || (bytes = ftell(bundle)) <= 0 ||
        (unsigned long)bytes > THERON_V1_CAPTURE_ARTIFACT_MAX_BYTES || fseek(bundle, 0L, SEEK_SET) != 0 ||
        !read_line(bundle, line, sizeof(line)) || strcmp(line, "THERON_TRACK02_CAPTURE_ARTIFACT_BUNDLE_V1") ||
        !read_line(bundle, line, sizeof(line)) || !md5_line_matches(line, "track02_md5=%32[0-9a-f]%n", receipt.track02_md5) ||
        strcmp(receipt.track02_md5, plan->targets[0].track02_md5) ||
        !read_line(bundle, line, sizeof(line)) || !md5_line_matches(line, "mednafen_trace_md5=%32[0-9a-f]%n", manifest_trace_md5) ||
        strcmp(manifest_trace_md5, trace.source_trace_md5) ||
        !read_line(bundle, line, sizeof(line)) || sscanf(line, "capture_target_plan_fnv1a=%x%n", &artifact_plan_identity, &campaign_route_consumed) != 1 ||
        line[campaign_route_consumed] != '\0' || !artifact_plan_identity || artifact_plan_identity != plan_identity ||
        !read_line(bundle, line, sizeof(line)) || sscanf(line, "campaign_route=%u%n", &campaign_route, &campaign_route_consumed) != 1 ||
        line[campaign_route_consumed] != '\0' || campaign_route >= THERON_V1_TRACK02_CAPTURE_TARGET_COUNT ||
        !read_line(bundle, line, sizeof(line)) || sscanf(line, "descriptor_selector=%x%n", &descriptor_selector, &campaign_route_consumed) != 1 ||
        line[campaign_route_consumed] != '\0' || !descriptor_selector ||
        descriptor_selector > UINT16_MAX ||
        !read_line(bundle, line, sizeof(line)) || sscanf(line, "descriptor_ordinal=%zu%n", &descriptor_ordinal, &campaign_route_consumed) != 1 ||
        line[campaign_route_consumed] != '\0' ||
        !read_line(bundle, line, sizeof(line)) || sscanf(line, "descriptor_source_hash=%x%n", &descriptor_source_hash, &campaign_route_consumed) != 1 ||
        line[campaign_route_consumed] != '\0' || !descriptor_source_hash) {
        if (bundle) fclose(bundle); goto rejected;
    }
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        if (!read_line(bundle, line, sizeof(line)) || !route_matches(line, (unsigned int)i, &plan->targets[i])) {
            fclose(bundle); goto rejected;
        }
    }
    /* Trailing content in the bundle short-circuited past fclose() and
     * leaked the handle; close unconditionally first. */
    { int io_failed = (fgets(line, sizeof(line), bundle) != NULL);
      if (fclose(bundle) != 0) io_failed = 1;
      if (io_failed ||
          !m12_file_md5_hex(request->bundle_path, final_bundle_md5) ||
          strcmp(bundle_md5, final_bundle_md5)) goto rejected; }
    receipt.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY;
    receipt.bundle_md5_verified = 1; receipt.mednafen_trace_md5_verified = 1;
    receipt.complete_route_set_consumed = receipt.opaque_envelope_verified = 1;
    receipt.track02_variant = plan->targets[0].track02_variant;
    receipt.campaign_route = (Theron_V1Track02CaptureTargetRoute)campaign_route;
    receipt.descriptor_selector = (uint16_t)descriptor_selector;
    receipt.descriptor_ordinal = descriptor_ordinal;
    receipt.descriptor_source_hash = descriptor_source_hash;
    snprintf(receipt.bundle_md5, sizeof(receipt.bundle_md5), "%s", bundle_md5);
    snprintf(receipt.mednafen_trace_md5, sizeof(receipt.mednafen_trace_md5), "%s", trace.source_trace_md5);
    receipt.capture_target_plan_identity = plan_identity;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        receipt.cd_read_record[i] = plan->targets[i].cd_read_record;
        receipt.loader_output_raw_offset[i] = plan->targets[i].loader_output_raw_offset;
        receipt.loader_output_bytes[i] = plan->targets[i].loader_output_bytes;
        receipt.loader_output_identity[i] = plan->targets[i].loader_output_checksum;
        receipt.palette_output_identity[i] = plan->targets[i].palette_output_identity;
        receipt.bitmap_transfer_identity[i] = plan->targets[i].bitmap_identity;
        receipt.destination_record[i] = plan->targets[i].destination_record;
        receipt.destination_offset[i] = plan->targets[i].destination_offset;
        receipt.destination_bytes[i] = plan->targets[i].destination_bytes;
        receipt.destination_identity[i] = plan->targets[i].destination_identity;
    }
    receipt.opaque_runtime_ready = 1;
    *out = receipt;
    return 1;
rejected:
    receipt.status = THERON_V1_TRACK02_CAPTURE_ARTIFACT_REJECTED;
    *out = receipt;
    return 1;
}
