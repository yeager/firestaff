/*
 * Opt-in, read-only Track 02 CD-read chain probe.
 *
 * Inputs are explicit only:
 *   THERON_RAW_TRACK02=/absolute/path/to/authentic-track02.bin
 *   THERON_SYSTEM_CARD=/absolute/path/to/syscard3.pce
 *
 * It validates the documented IPL -> $4090 local-RAM CD_READ boundary, then
 * hands the resulting hash-bound receipt to the real Theron boot graphics
 * consumer.  The consumer may acknowledge raw bitmap provenance, but cannot
 * draw until a separate original palette-byte relation is captured.  No
 * filesystem scan outside an explicit root, emulator, framebuffer, or
 * fallback executor is involved.
 */
#define _XOPEN_SOURCE 700

#include "asset_status_m12.h"
#include "theron_v1_boot.h"
#include "theron_v1_raw_loader_trace.h"
#include "theron_v1_startup_media.h"
#include "theron_v1_system_card_irq2_entry_gate.h"
#include "theron_v1_track02.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ftw.h>
#include <sys/stat.h>

#define THERON_CHAIN_SYSCARD3_MD5 "ff1a674273fe3540ccef576376407d1d"
#define THERON_CHAIN_SYSCARD3_BYTES 0x40200u
#define THERON_CHAIN_PATH_CAPACITY 1024u
#define THERON_CHAIN_TRACE_MAX_BYTES (1024u * 1024u)

typedef struct {
    char raw_track02_path[THERON_CHAIN_PATH_CAPACITY];
    char raw_track02_md5[33];
    char system_card_path[THERON_CHAIN_PATH_CAPACITY];
    char loader_trace_path[THERON_CHAIN_PATH_CAPACITY];
    unsigned int known_track02_iso_count;
    int trace_scan_only;
} Theron_ChainLocalArtifacts;

static Theron_ChainLocalArtifacts *g_local_artifacts;

static int read_file(const char *path, unsigned char **out_data,
                     size_t *out_size)
{
    FILE *file;
    long length;
    unsigned char *data;

    if (!path || !path[0] || !out_data || !out_size) {
        return 0;
    }
    *out_data = NULL;
    *out_size = 0u;
    file = fopen(path, "rb");
    if (!file || fseek(file, 0L, SEEK_END) != 0 ||
        (length = ftell(file)) <= 0 || fseek(file, 0L, SEEK_SET) != 0) {
        if (file) {
            fclose(file);
        }
        return 0;
    }
    data = (unsigned char *)malloc((size_t)length);
    if (!data || fread(data, 1u, (size_t)length, file) != (size_t)length) {
        free(data);
        fclose(file);
        return 0;
    }
    fclose(file);
    *out_data = data;
    *out_size = (size_t)length;
    return 1;
}

static int raw_track02_md5(const char *path, size_t bytes, char md5[33])
{
    return path && bytes > 0u &&
           bytes % THERON_TRACK02_RAW_SECTOR_BYTES == 0u &&
           m12_file_md5_hex(path, md5) &&
           (strcmp(md5, THERON_TRACK02_MD5_JP_BIN) == 0 ||
            strcmp(md5, THERON_TRACK02_MD5_US_BIN) == 0);
}

static int system_card_md5(const char *path, size_t bytes, char md5[33])
{
    return path && bytes == THERON_CHAIN_SYSCARD3_BYTES &&
           m12_file_md5_hex(path, md5) &&
           strcmp(md5, THERON_CHAIN_SYSCARD3_MD5) == 0;
}

static int known_track02_iso(const char *path, size_t bytes)
{
    char md5[33];

    return path && bytes > 0u && bytes % 2048u == 0u &&
           m12_file_md5_hex(path, md5) &&
           (strcmp(md5, THERON_TRACK02_MD5_JP_REV1_ISO) == 0 ||
            strcmp(md5, THERON_TRACK02_MD5_US_ISO) == 0);
}

static void copy_artifact_path(char destination[THERON_CHAIN_PATH_CAPACITY],
                               const char *source)
{
    if (!destination || !source || destination[0]) {
        return;
    }
    snprintf(destination, THERON_CHAIN_PATH_CAPACITY, "%s", source);
}

static int scan_local_artifact(const char *path, const struct stat *status,
                               int typeflag, struct FTW *entry)
{
    char md5[33];
    Theron_V1RawLoaderTraceReceipt trace;

    (void)entry;
    if (!g_local_artifacts || typeflag != FTW_F || !status ||
        status->st_size <= 0 || (uintmax_t)status->st_size > SIZE_MAX) {
        return 0;
    }
    if (!g_local_artifacts->trace_scan_only) {
        if (!g_local_artifacts->raw_track02_path[0] &&
            raw_track02_md5(path, (size_t)status->st_size, md5)) {
            copy_artifact_path(g_local_artifacts->raw_track02_path, path);
            snprintf(g_local_artifacts->raw_track02_md5,
                     sizeof(g_local_artifacts->raw_track02_md5), "%s", md5);
            return 0;
        }
        if (!g_local_artifacts->system_card_path[0] &&
            system_card_md5(path, (size_t)status->st_size, md5)) {
            copy_artifact_path(g_local_artifacts->system_card_path, path);
            return 0;
        }
        if (known_track02_iso(path, (size_t)status->st_size)) {
            ++g_local_artifacts->known_track02_iso_count;
            return 0;
        }
    }
    if (g_local_artifacts->raw_track02_path[0] &&
        !g_local_artifacts->loader_trace_path[0] &&
        (size_t)status->st_size <= THERON_CHAIN_TRACE_MAX_BYTES &&
        theron_v1_raw_loader_trace_import_mednafen_capture_file(
            path, g_local_artifacts->raw_track02_md5, &trace)) {
        copy_artifact_path(g_local_artifacts->loader_trace_path, path);
    }
    return 0;
}

static int find_local_artifacts(const char *root,
                                Theron_ChainLocalArtifacts *out_artifacts)
{
    if (!out_artifacts) {
        return 0;
    }
    memset(out_artifacts, 0, sizeof(*out_artifacts));
    if (!root || !root[0]) {
        return 0;
    }
    g_local_artifacts = out_artifacts;
    if (nftw(root, scan_local_artifact, 16, FTW_PHYS) != 0) {
        g_local_artifacts = NULL;
        return 0;
    }
    /* The trace can sort before the raw BIN.  Repeat the bounded walk after
     * Track 02 identity is known so discovery never depends on directory
     * order.  This still imports a trace only through its variant-matched
     * Mednafen schema and never treats arbitrary text as capture evidence. */
    if (out_artifacts->raw_track02_path[0] &&
        !out_artifacts->loader_trace_path[0]) {
        out_artifacts->trace_scan_only = 1;
        if (nftw(root, scan_local_artifact, 16, FTW_PHYS) != 0) {
            g_local_artifacts = NULL;
            return 0;
        }
    }
    g_local_artifacts = NULL;
    return 1;
}

static int requested_route(const char *name, unsigned int *out_route_bit,
                           size_t *out_anchor_index, const char **out_name)
{
    if (!name || !out_route_bit || !out_anchor_index || !out_name) {
        return 0;
    }
    if (strcmp(name, "title") == 0) {
        *out_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_TITLE;
        *out_anchor_index = 1u;
        *out_name = "title";
        return 1;
    }
    if (strcmp(name, "stage") == 0) {
        *out_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_STAGE;
        *out_anchor_index = 2u;
        *out_name = "stage";
        return 1;
    }
    if (strcmp(name, "soul_room") == 0) {
        *out_route_bit = THERON_TRACK02_STARTUP_BITMAP_ROUTE_SOUL_ROOM;
        *out_anchor_index = 0u;
        *out_name = "soul_room";
        return 1;
    }
    return 0;
}

static const Theron_Track02StartupBitmapAtlasRoute *find_route(
    const Theron_Track02StartupBitmapAtlas *atlas, unsigned int route_bit)
{
    size_t i;

    if (!atlas) {
        return NULL;
    }
    for (i = 0u; i < atlas->route_count; ++i) {
        if (atlas->routes[i].route_bit == route_bit) {
            return &atlas->routes[i];
        }
    }
    return NULL;
}

int main(void)
{
    const char *track02_path = getenv("THERON_RAW_TRACK02");
    const char *system_card_path = getenv("THERON_SYSTEM_CARD");
    const char *loader_trace_path = getenv("THERON_RAW_LOADER_TRACE");
    const char *capture_root = getenv("THERON_CAPTURE_ROOT");
    Theron_ChainLocalArtifacts local_artifacts;
    unsigned char *track02 = NULL;
    unsigned char *system_card = NULL;
    size_t track02_bytes = 0u;
    size_t system_card_bytes = 0u;
    char track02_md5[33];
    char system_card_md5_hex[33];
    Theron_Track02IplLoaderReceipt ipl;
    Theron_Track02Stage2DynamicPayloadReceipt payload;
    Theron_Track02BankSignal bank;
    Theron_V1SystemCardIrq2EntryGate system_card_gate;
    Theron_StartupMediaStateReceipt bitmap;
    Theron_StartupRawBitmapRouteReceipt raw_bitmap;
    Theron_V1RawLoaderTraceReceipt loader_trace;
    Theron_V1RawLoaderTraceReceipt media_bound_trace;
    Theron_V1RawLoaderTraceReceipt bitmap_bound_trace;
    Theron_V1_BootStartupRawMediaGraphicsReceipt handoff;
    const Theron_Track02StartupBitmapAtlasRoute *route;
    const char *route_name = NULL;
    unsigned int route_bit = 0u;
    size_t route_anchor_index = 0u;
    int result = 1;

    if (!track02_path || !track02_path[0] || !system_card_path ||
        !system_card_path[0] || !loader_trace_path || !loader_trace_path[0]) {
        if (!find_local_artifacts(capture_root, &local_artifacts)) {
            printf("status=skip reason=explicit_artifacts_or_capture_root_required "
                   "emulator=not_started fallback=not_run\n");
            return 0;
        }
        if (!track02_path || !track02_path[0]) {
            track02_path = local_artifacts.raw_track02_path;
        }
        if (!system_card_path || !system_card_path[0]) {
            system_card_path = local_artifacts.system_card_path;
        }
        if (!loader_trace_path || !loader_trace_path[0]) {
            loader_trace_path = local_artifacts.loader_trace_path;
        }
    }
    if (!track02_path || !track02_path[0] || !system_card_path ||
        !system_card_path[0] || !loader_trace_path || !loader_trace_path[0]) {
        printf("status=skip reason=hash_verified_raw_track02_system_card_and_loader_trace_not_found "
               "known_track02_iso_candidates=%u emulator=not_started fallback=not_run\n",
               local_artifacts.known_track02_iso_count);
        return 0;
    }
    if (!requested_route(getenv("THERON_BITMAP_ROUTE"), &route_bit,
                         &route_anchor_index, &route_name)) {
        printf("status=skip reason=explicit_bitmap_route_title_stage_or_soul_room_required "
               "emulator=not_started fallback=not_run\n");
        return 0;
    }
    if (!read_file(track02_path, &track02, &track02_bytes) ||
        !raw_track02_md5(track02_path, track02_bytes, track02_md5)) {
        printf("status=blocked reason=raw_track02_missing_or_unverified "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (!read_file(system_card_path, &system_card, &system_card_bytes) ||
        !system_card_md5(system_card_path, system_card_bytes,
                         system_card_md5_hex)) {
        printf("status=blocked reason=system_card_missing_or_unverified "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (!theron_v1_raw_loader_trace_import_mednafen_capture_file(
            loader_trace_path, track02_md5, &loader_trace) ||
        !theron_v1_raw_loader_trace_bind_track02_destination_span(
            &loader_trace, track02, track02_bytes, track02_md5,
            &media_bound_trace)) {
        printf("status=blocked reason=loader_trace_track02_span_unproven "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (theron_v1_track02_find_ipl_loader(track02, track02_bytes, track02_md5,
                                           &ipl) != THERON_TRACK02_SIGNAL_OK ||
        !ipl.valid || !ipl.stage2_cd_read_record_proven ||
        !ipl.stage2_cd_read_dynamic_boundary_valid ||
        ipl.stage2_cd_read_cpu_address !=
            THERON_TRACK02_IPL_STAGE2_CD_READ_CPU_ADDRESS ||
        ipl.stage2_cd_read_destination != THERON_TRACK02_IPL_DESTINATION_LOCAL_RAM ||
        ipl.stage2_cd_read_local_destination !=
            THERON_TRACK02_IPL_STAGE2_CD_READ_LOCAL_DESTINATION ||
        ipl.vram_transfer_proven) {
        printf("status=blocked reason=ipl_cd_read_receipt_invalid "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (theron_v1_track02_inspect_stage2_dynamic_payload(
            track02, track02_bytes, track02_md5, &payload) !=
            THERON_TRACK02_SIGNAL_OK ||
        !payload.valid || payload.track02_record != ipl.stage2_cd_read_record ||
        payload.raw_sector != ipl.stage2_cd_read_raw_sector ||
        payload.user_data_bytes != THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        !payload.user_data_hash) {
        printf("status=blocked reason=cd_read_sector_receipt_invalid "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (!theron_v1_system_card_irq2_entry_gate_from_original_media(
            &payload, system_card, system_card_bytes, system_card_md5_hex,
            &system_card_gate) || !system_card_gate.valid ||
        system_card_gate.stage3_track02_record != payload.track02_record ||
        !system_card_gate.selected_branch_unobserved) {
        printf("status=blocked reason=system_card_cd_read_gate_invalid "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }

    theron_v1_startup_media_capture_track02_state_receipt(
        track02, track02_bytes, track02_md5, &bitmap);
    if (!theron_v1_startup_media_state_receipt_has_complete_bitmap_routes(&bitmap) ||
        bitmap.startup_bitmap_raw_route_mask == 0u ||
        bitmap.startup_bitmap_atlas_checksum == 0u) {
        printf("status=blocked reason=raw_bitmap_gate_incomplete "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (!theron_v1_raw_loader_trace_final_bind(
            &media_bound_trace, &bitmap, &bitmap_bound_trace) ||
        !bitmap_bound_trace.valid ||
        !bitmap_bound_trace.stage2_dynamic_payload_verified ||
        bitmap_bound_trace.stage2_dynamic_payload_bytes !=
            THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES ||
        !bitmap_bound_trace.soul_room_raw_route_verified ||
        !bitmap_bound_trace.soul_room_route_disjoint_from_dynamic_span ||
        bitmap_bound_trace.bitmap_route_mask !=
            bitmap.startup_bitmap_raw_route_mask ||
        bitmap_bound_trace.bitmap_atlas_checksum !=
            bitmap.startup_bitmap_atlas_checksum) {
        printf("status=blocked reason=loader_trace_bitmap_route_unproven "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }
    if (theron_v1_track02_find_bank_signal(track02, track02_bytes, track02_md5,
                                            &bank) != THERON_TRACK02_SIGNAL_OK ||
        route_anchor_index >= bank.anchor_count ||
        (bitmap.startup_bitmap_raw_route_mask & route_bit) == 0u ||
        !(route = find_route(&bitmap.startup_bitmap_atlas, route_bit)) ||
        route->tile_count == 0u || route->checksum == 0u ||
        route->first_raw_offset !=
            bank.post_boundary_span_offsets[route_anchor_index]) {
        printf("status=blocked reason=bitmap_descriptor_relation_unproven "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }

    if (!theron_v1_startup_media_consume_raw_bitmap_route(
            &bitmap, route_bit, &raw_bitmap) || !raw_bitmap.valid ||
        !raw_bitmap.raw_source_verified ||
        raw_bitmap.variant != (Theron_Track02Variant)bitmap.track02_variant ||
        raw_bitmap.route_bit != route_bit ||
        raw_bitmap.checksum != route->checksum ||
        raw_bitmap.first_raw_offset != route->first_raw_offset ||
        raw_bitmap.first_user_data_offset != route->first_user_data_offset ||
        raw_bitmap.palette_binding_verified || raw_bitmap.rgba_output_allowed) {
        printf("status=blocked reason=raw_bitmap_consumer_rejected "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }

    /* This is the production boot boundary.  It admits only the receipt that
     * has already bound the original Mednafen transaction to the selected raw
     * Track 02 bytes.  The System Card is checked above through the original
     * IRQ2 entry gate, so an independently supplied hash-valid ROM cannot
     * bypass the combined capture chain. */
    if (!theron_v1_boot_startup_raw_media_graphics_receipt_from_loader_trace(
            &bitmap, &bitmap_bound_trace, &handoff) || !handoff.valid ||
        !handoff.raw_track02_verified || !handoff.cd_read_receipt_verified ||
        !handoff.bitmap_route_receipt_verified ||
        !handoff.no_fallback_visuals ||
        handoff.track02_variant != bitmap.track02_variant ||
        strcmp(handoff.track02_md5, bitmap.track02_md5) != 0 ||
        handoff.bitmap_route_mask != bitmap.startup_bitmap_raw_route_mask ||
        handoff.bitmap_atlas_checksum != bitmap.startup_bitmap_atlas_checksum) {
        printf("status=blocked reason=raw_capture_handoff_rejected "
               "emulator=not_started fallback=not_run\n");
        goto done;
    }

    /* The bounded route-to-bank relation and the production handoff are now
     * positive.  Palette byte provenance is intentionally separate: do not
     * inspect arbitrary spans, publish pixels, or substitute a surface. */
    printf("status=ready route=%s raw_bitmap_consumed=1 "
           "system_card_gate_bound=1 loader_trace_track02_bound=1 "
           "bitmap_route_receipt_bound=1 boot_handoff_bound=1 "
           "bitmap_descriptor_hash=%08x palette_descriptor=%s "
           "rgba_output=%s emulator=not_started fallback=not_run\n",
           route_name, raw_bitmap.checksum,
           handoff.palette_descriptor_relation_verified ? "verified" : "unproven",
           handoff.palette_descriptor_relation_verified ? "not_exercised" : "blocked");

done:
    free(track02);
    free(system_card);
    return result;
}
