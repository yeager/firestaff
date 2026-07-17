#include <stdio.h>
#include <string.h>

#include "asset_status_m12.h"
#include "theron_v1_track02_capture_artifact_importer.h"

static void plan_fixture(Theron_V1Track02CaptureTargetPlan *plan)
{
    size_t i;
    memset(plan, 0, sizeof(*plan)); plan->valid = 1; plan->cue_track_consumed = 1;
    plan->cd_read_chain_consumed = 1; plan->loader_output_consumed = 1; plan->palette_output_consumed = 1;
    plan->bitmap_transfer_consumed = 1; plan->destination_record_consumed = 1;
    for (i = 0u; i < THERON_V1_TRACK02_CAPTURE_TARGET_COUNT; ++i) {
        Theron_V1Track02CaptureTarget *target = &plan->targets[i];
        target->route = (Theron_V1Track02CaptureTargetRoute)i; target->track02_variant = THERON_TRACK02_VARIANT_US_BIN;
        strcpy(target->track02_md5, THERON_TRACK02_MD5_US_BIN); target->cd_read_record = 0x4e0u;
        target->loader_output_raw_offset = 0x930u; target->loader_output_bytes = 32u; target->loader_output_checksum = 0x11110000u;
        target->palette_output_identity = 0x22220000u; target->bitmap_raw_offset = 0x3000u + i * 0x100u;
        target->bitmap_bytes = 64u; target->bitmap_identity = 0x33330000u + (uint32_t)i;
        target->destination_record = 0xb52u; target->destination_offset = 0x2000u; target->destination_bytes = 128u;
        target->destination_identity = 0x44440000u + (uint32_t)i;
    }
}

static int write_trace(const char *path)
{
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (fputs("source=mednafen-pce-instrumented\n"
              "huc6280_loader_cd_read pc=4090 record=4e0 destination=3800 byte_count=32 payload_checksum=11110000\n"
              "huc6280_dungeon_consumer pc=4120 payload_offset=2000 byte_count=128 window_checksum=44440000\n"
              "huc6280_object_consumer pc=4180 payload_offset=2080 byte_count=64 window_checksum=55550000\n"
              "consumer_trace_checksum=99990000\n", file) < 0) { fclose(file); return 0; }
    return fclose(file) == 0;
}

static int write_bundle(const char *path, const Theron_V1Track02CaptureTargetPlan *plan, const char *trace_md5, uint32_t identity, int complete)
{
    FILE *file = fopen(path, "wb"); size_t i;
    if (!file || fprintf(file, "THERON_TRACK02_CAPTURE_ARTIFACT_BUNDLE_V1\ntrack02_md5=%s\nmednafen_trace_md5=%s\ncapture_target_plan_fnv1a=%x\ncampaign_route=0\ndescriptor_selector=30\ndescriptor_ordinal=1\ndescriptor_source_hash=44556677\n", plan->targets[0].track02_md5, trace_md5, identity) < 0) return 0;
    for (i = 0u; i < (complete ? 3u : 2u); ++i) {
        const Theron_V1Track02CaptureTarget *t = &plan->targets[i];
        if (fprintf(file, "route=%zu cd_record=%x loader_offset=%zx loader_bytes=%zu loader_checksum=%x palette_identity=%x bitmap_offset=%zx bitmap_bytes=%zu bitmap_identity=%x destination_record=%x destination_offset=%zx destination_bytes=%zu destination_identity=%x\n", i, t->cd_read_record, t->loader_output_raw_offset, t->loader_output_bytes, t->loader_output_checksum, t->palette_output_identity, t->bitmap_raw_offset, t->bitmap_bytes, t->bitmap_identity, t->destination_record, t->destination_offset, t->destination_bytes, t->destination_identity) < 0) { fclose(file); return 0; }
    }
    return fclose(file) == 0;
}

int main(void)
{
    const char *trace_path = "/tmp/firestaff-theron-artifact-trace.txt";
    const char *bundle_path = "/tmp/firestaff-theron-artifact.bundle";
    Theron_V1Track02CaptureTargetPlan plan; Theron_V1Track02CaptureArtifactImportRequest request;
    Theron_V1Track02CaptureArtifactRuntimeAdmissionReceipt receipt; char trace_md5[33], bundle_md5[33]; uint32_t identity;
    plan_fixture(&plan);
    identity = theron_v1_track02_capture_target_plan_identity(&plan);
    if (!identity || !write_trace(trace_path) || !m12_file_md5_hex(trace_path, trace_md5) || !write_bundle(bundle_path, &plan, trace_md5, identity, 1) || !m12_file_md5_hex(bundle_path, bundle_md5)) return 1;
    request = (Theron_V1Track02CaptureArtifactImportRequest){bundle_path, bundle_md5, trace_path, trace_md5};
    if (!theron_v1_track02_capture_artifact_import(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_READY || !receipt.opaque_envelope_verified || !receipt.opaque_runtime_ready || receipt.capture_target_plan_identity != theron_v1_track02_capture_target_plan_identity(&plan) || receipt.descriptor_selector != 0x30u || receipt.descriptor_ordinal != 1u || receipt.descriptor_source_hash != 0x44556677u || receipt.loader_output_raw_offset[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] != plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].loader_output_raw_offset || receipt.loader_output_bytes[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] != plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].loader_output_bytes || receipt.destination_offset[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] != plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].destination_offset || receipt.destination_bytes[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF] != plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].destination_bytes || receipt.render_allowed || receipt.pixel_decode_allowed || receipt.level_object_semantics_allowed) return 2;
    if (!write_bundle(bundle_path, &plan, trace_md5, identity + 1u, 1) || !m12_file_md5_hex(bundle_path, bundle_md5)) return 8;
    request.expected_bundle_md5 = bundle_md5;
    if (!theron_v1_track02_capture_artifact_import(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_REJECTED) return 9;
    if (!write_bundle(bundle_path, &plan, trace_md5, identity, 1) || !m12_file_md5_hex(bundle_path, bundle_md5)) return 10;
    request.expected_bundle_md5 = bundle_md5;
    request.expected_bundle_md5 = THERON_TRACK02_MD5_US_BIN;
    if (!theron_v1_track02_capture_artifact_import(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_REJECTED) return 3;
    request.expected_bundle_md5 = bundle_md5;
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].bitmap_identity = 0u;
    if (!theron_v1_track02_capture_artifact_import(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_REJECTED) return 5;
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_DUNGEON_HANDOFF].bitmap_identity = 0x33330002u;
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM].track02_variant = THERON_TRACK02_VARIANT_JP_BIN;
    if (!theron_v1_track02_capture_artifact_import(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_REJECTED) return 6;
    plan.targets[THERON_V1_TRACK02_CAPTURE_TARGET_SOUL_ROOM].track02_variant = THERON_TRACK02_VARIANT_US_BIN;
    if (!write_bundle(bundle_path, &plan, trace_md5, identity, 0) || !m12_file_md5_hex(bundle_path, bundle_md5)) return 4;
    request.expected_bundle_md5 = bundle_md5;
    if (!theron_v1_track02_capture_artifact_import(&plan, &request, &receipt) || receipt.status != THERON_V1_TRACK02_CAPTURE_ARTIFACT_REJECTED) return 7;
    remove(trace_path); remove(bundle_path); puts("test_theron_v1_track02_capture_artifact_importer: PASS"); return 0;
}
