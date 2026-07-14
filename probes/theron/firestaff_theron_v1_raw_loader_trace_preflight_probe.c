#include "asset_status_m12.h"
#include "theron_v1_capture_manifest.h"
#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_track02.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

int main(void)
{
    const char *raw = getenv("THERON_RAW_TRACK02");
    const char *system_card = getenv("THERON_RAW_SYSTEM_CARD");
    const char *trace = getenv("THERON_RAW_LOADER_TRACE");
    const char *manifest = getenv("THERON_CAPTURE_MANIFEST");
    struct stat st;
    char md5[33];
    char system_card_md5[33];
    char trace_md5[33];
    char text[1024];
    uint8_t *raw_bytes;
    FILE *file;
    Theron_V1CaptureManifest capture_manifest;
    Theron_V1RawLoaderTraceReceipt trace_receipt;
    Theron_V1RawLoaderTraceReceipt bound_receipt;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_V1RawLoaderTraceStage3SectorReceipt stage3_receipt;

    if (!raw || !system_card || !trace || stat(raw, &st) != 0 ||
        st.st_size <= 0) {
        printf("status=skip reason=explicit_raw_track02_system_card_and_trace_required\n");
        return 0;
    }
    if ((size_t)st.st_size % THERON_TRACK02_RAW_SECTOR_BYTES != 0u ||
        !m12_file_md5_hex(raw, md5) ||
        (strcmp(md5, THERON_TRACK02_MD5_US_BIN) != 0 &&
         strcmp(md5, THERON_TRACK02_MD5_JP_BIN) != 0)) {
        printf("status=blocked reason=raw_track02_unverified\n");
        return 1;
    }
    if (!m12_file_md5_hex(system_card, system_card_md5) ||
        !m12_file_md5_hex(trace, trace_md5)) {
        printf("status=blocked reason=system_card_or_loader_trace_unhashed\n");
        return 1;
    }
    if (manifest) {
        file = fopen(manifest, "r");
        if (!file || !fread(text, 1u, sizeof(text) - 1u, file)) {
            if (file) fclose(file);
            printf("status=blocked reason=capture_manifest_invalid\n");
            return 1;
        }
        fclose(file);
        text[sizeof(text) - 1u] = '\0';
        if (!theron_v1_capture_manifest_parse(text, &capture_manifest) ||
            !theron_v1_capture_manifest_matches_preflight_inputs(
                &capture_manifest, raw, md5, system_card, system_card_md5,
                trace, trace_md5)) {
            printf("status=blocked reason=capture_manifest_mismatch\n");
            return 1;
        }
    }
    raw_bytes = (uint8_t *)malloc((size_t)st.st_size);
    file = raw_bytes ? fopen(raw, "rb") : NULL;
    if (!file || fread(raw_bytes, 1u, (size_t)st.st_size, file) !=
                     (size_t)st.st_size) {
        if (file) fclose(file);
        free(raw_bytes);
        printf("status=blocked reason=raw_track02_read_failed\n");
        return 1;
    }
    fclose(file);
    if (!theron_v1_raw_loader_trace_import_mednafen_capture_file(
            trace, md5, &trace_receipt) ||
        !theron_v1_raw_loader_trace_bind_track02_destination_span(
            &trace_receipt, raw_bytes, (size_t)st.st_size, md5,
            &bound_receipt) ||
        theron_v1_track02_inspect_stage2_dynamic_payload(
            raw_bytes, (size_t)st.st_size, md5, &payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        !theron_v1_raw_loader_trace_stage3_sector_receipt_from_bound_span(
            &bound_receipt, &payload, &stage3_receipt)) {
        free(raw_bytes);
        printf("status=blocked reason=loader_trace_media_span_unproven\n");
        return 1;
    }
    free(raw_bytes);
    if (bound_receipt.bitmap_route_mask || bound_receipt.bitmap_atlas_checksum ||
        bound_receipt.palette_descriptor_relation_verified) {
        printf("status=blocked reason=unbound_trace_render_claim\n");
        return 1;
    }
    printf("status=ready trace=validated_dynamic_loader_stage3_sector_receipt="
           "blocked_pending_palette_source_provenance\n");
    return 0;
}
