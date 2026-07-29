#include "csb_v1_viewport_pc34_compat.h"
#include "csb_v1_viewport_d0l2_d0r2_f0111_door_front_pc34_compat.h"
#include "csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_pc34_compat.h"
#include "csb_v1_viewport_d1c_f0111_door_pc34_compat.h"
#include "firestaff/csb/v1/viewport/d1c_f0115_thing_pass_pc34_compat.h"
#include "csb_v1_viewport_d2c_f0111_door_front_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static int expect_int(const char *label, int got, int want)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s=%d\n", label, got);
    return 1;
}

static int expect_u32_nonzero(const char *label, uint32_t value)
{
    return expect_int(label, value != 0u, 1);
}

static unsigned read_be16(const unsigned char *p)
{
    return ((unsigned)p[0] << 8) | (unsigned)p[1];
}

static uint32_t fnv1a32(const unsigned char *data, size_t size)
{
    uint32_t hash = 2166136261u;
    size_t i;

    for (i = 0u; i < size; ++i) {
        hash ^= data[i];
        hash *= 16777619u;
    }
    return hash;
}

static void set_proof_route_hash(CSB_V1_ViewportFirstFrameMaterialProof *proof,
                                 unsigned int route_bit, uint32_t hash)
{
    switch (route_bit) {
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0111_DOOR:
        proof->d0_door_hash = hash;
        break;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0115_THING:
        proof->d0_thing_hash = hash;
        break;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0111_DOOR:
        proof->d1_door_hash = hash;
        break;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0115_THING:
        proof->d1_thing_hash = hash;
        break;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR:
        proof->d2_door_hash = hash;
        break;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR:
        proof->d3l2_door_hash = hash;
        break;
    case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR:
        proof->d3r2_door_hash = hash;
        break;
    }
}

static void set_d2_door_capture(
    CSB_V1_ViewportFirstFrameMaterialProof *proof,
    size_t byte_count, uint32_t payload_hash)
{
    proof->d2_door_capture_valid = 1;
    proof->d2_door_capture_real_graphics_dat = 1;
    proof->d2_door_capture_no_synthetic_pixels = 1;
    proof->d2_door_capture_no_fallback_visuals = 1;
    /* ReDMCSB DUNVIEW.C:2651-2658: G0694 is DoorSet base + 1. */
    proof->d2_door_capture_item_index = 247;
    proof->d2_door_capture_byte_count = byte_count;
    proof->d2_door_capture_payload_hash = payload_hash;
    proof->d2_door_capture_width = 64;
    proof->d2_door_capture_height = 61;
    proof->d2_door_capture_zone = 3760;
    proof->d2_door_capture_transparent_color = 10;
}

static void test_checked_material_byte_handoff_and_raster(void)
{
    static const unsigned char palette[] = { 1u, 2u, 3u, 4u, 5u, 6u };
    static unsigned char d2_pixels[64u * 61u];
    static const unsigned char pixels[5][4] = {
        { 21u, 22u, 23u, 24u }, { 31u, 32u, 33u, 34u },
        { 41u, 42u, 43u, 44u }, { 51u, 52u, 53u, 54u },
        { 61u, 62u, 63u, 64u }
    };
    const unsigned char *material_pixels[5];
    size_t material_sizes[5];
    int material_widths[5];
    int material_heights[5];
    CSB_V1_ViewportFirstFrameMaterialProof proof;
    CSB_V1_ViewportRuntimeDrawPlanPc34 plan;
    CSB_V1_ViewportFirstFrameMaterialBytesPc34 bytes;
    CSB_V1_ViewportFirstFrameMaterializationReceipt material_receipt;
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 raster_receipt;
    CSB_V1_ViewportConfig cfg;
    CSB_V1_ViewportRuntimeDrawerBinding binding;
    CSB_V1_ViewportRuntimeDrawCounts counts;
    uint8_t framebuffer[224 * 169];
    uint8_t live_framebuffer[320 * 200];
    int i;

    memset(d2_pixels, 21, sizeof(d2_pixels));
    material_pixels[0] = d2_pixels;
    material_sizes[0] = sizeof(d2_pixels);
    material_widths[0] = 64;
    material_heights[0] = 61;
    for (i = 1; i < 5; ++i) {
        material_pixels[i] = pixels[i];
        material_sizes[i] = sizeof(pixels[i]);
        material_widths[i] = 2;
        material_heights[i] = 2;
    }
    memset(&proof, 0, sizeof(proof));
    proof.valid = 1;
    proof.route_mask = CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES;
    proof.source_graphics_dat_bound = 1;
    proof.no_synthetic_pixels = 1;
    proof.no_fallback_visuals = 1;
    proof.shared_palette_material_proof = 1;
    proof.shared_palette_hash = fnv1a32(palette, sizeof(palette));
    proof.source_item_count = 5u;
    proof.source_evidence = "structural GRAPHICS.DAT decoded-span receipt";

    /* The fixture is admission-only: no production fallback pixels exist. */
    for (i = 0; i < 5; ++i) {
        set_proof_route_hash(&proof,
                             (unsigned int[]){
                                 CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR,
                                 CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0111_DOOR,
                                 CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0115_THING,
                                 CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0111_DOOR,
                                 CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0115_THING
                             }[i],
                             fnv1a32(material_pixels[i], material_sizes[i]));
    }
    set_d2_door_capture(&proof, sizeof(d2_pixels), proof.d2_door_hash);
    expect_int("bytes.plan", csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                   &proof, 1, 0, 5, 5, &plan), 1);

    memset(&bytes, 0, sizeof(bytes));
    bytes.valid = 1;
    bytes.source_path = "/verified/CSBGRAPHICS.DAT";
    bytes.source_md5 = "0123456789abcdef0123456789abcdef";
    bytes.palette.decoded_palette = palette;
    bytes.palette.decoded_size = sizeof(palette);
    bytes.palette.decoded_fnv1a = proof.shared_palette_hash;
    for (i = 0; i < plan.command_count; ++i) {
        bytes.materials[i].route_bit = plan.commands[i].route_bit;
        bytes.materials[i].decoded_pixels = material_pixels[i];
        bytes.materials[i].decoded_size = material_sizes[i];
        bytes.materials[i].decoded_fnv1a = fnv1a32(material_pixels[i],
                                                    material_sizes[i]);
        bytes.materials[i].width = material_widths[i];
        bytes.materials[i].height = material_heights[i];
    }
    bytes.d2_d3_capture.valid = 1;
    bytes.d2_d3_capture.original_graphics_dat_capture = 1;
    bytes.d2_d3_capture.no_synthetic_pixels = 1;
    bytes.d2_d3_capture.no_fallback_visuals = 1;
    bytes.d2_d3_capture.source_path = bytes.source_path;
    bytes.d2_d3_capture.source_md5 = bytes.source_md5;
    bytes.d2_d3_capture.palette_source_path = bytes.source_path;
    bytes.d2_d3_capture.palette_source_md5 = bytes.source_md5;
    bytes.d2_d3_capture.palette_capture_fnv1a = proof.shared_palette_hash;
    bytes.d2_d3_capture.capture_identity_hash = 0x62d3694u;
    bytes.d2_d3_capture.d2_item_index = proof.d2_door_capture_item_index;
    bytes.d2_d3_capture.d2_source_byte_count = proof.d2_door_capture_byte_count;
    bytes.d2_d3_capture.d2_source_payload_hash = proof.d2_door_hash;
    bytes.d2_d3_capture.d2_decoded_pixels = d2_pixels;
    bytes.d2_d3_capture.d2_decoded_size = sizeof(d2_pixels);
    bytes.d2_d3_capture.d2_decoded_fnv1a = fnv1a32(d2_pixels, sizeof(d2_pixels));
    bytes.d2_d3_capture.d2_width = 64;
    bytes.d2_d3_capture.d2_height = 61;

    expect_int("bytes.bind", csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 1);
    expect_int("bytes.bind.receipt", material_receipt.valid, 1);
    expect_int("bytes.command.material.attached",
               plan.commands[0].decoded_pixels == d2_pixels, 1);
    expect_int("bytes.command.doorset0.index",
               plan.commands[0].source_graphics_item_index, 247);
    expect_int("bytes.command.palette.attached",
               plan.commands[0].decoded_palette == palette, 1);

    memset(framebuffer, 0, sizeof(framebuffer));
    expect_int("bytes.raster", csb_v1_viewport_consume_first_frame_material_raster_pc34(
                   &material_receipt, &plan, &bytes, bytes.source_path,
                   bytes.source_md5, framebuffer, 224, 169, &raster_receipt), 1);
    expect_int("bytes.raster.receipt", raster_receipt.consumed_by_raster, 1);
    expect_u32_nonzero("bytes.raster.hash", raster_receipt.raster_hash);

    bytes.materials[0].decoded_pixels = NULL;
    expect_int("bytes.reject.missing.material",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 0);
    bytes.materials[0].decoded_pixels = d2_pixels;
    bytes.materials[0].decoded_size = 3u;
    expect_int("bytes.reject.truncated.material",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 0);
    bytes.materials[0].decoded_size = sizeof(d2_pixels);
    bytes.materials[0].decoded_fnv1a ^= 1u;
    expect_int("bytes.reject.stale.material.hash",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 0);
    bytes.materials[0].decoded_fnv1a = fnv1a32(d2_pixels, sizeof(d2_pixels));
    expect_int("bytes.rebind", csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 1);

    /* DoorSet 1 proves this handoff follows G0694's active-map slot instead
     * of silently accepting the obsolete fixed record 694. */
    proof.d2_door_capture_item_index = 250;
    bytes.d2_d3_capture.d2_item_index = 250;
    expect_int("bytes.plan.doorset1",
               csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                   &proof, 1, 0, 5, 5, &plan), 1);
    expect_int("bytes.bind.doorset1",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 1);
    expect_int("bytes.command.doorset1.index",
               plan.commands[0].source_graphics_item_index, 250);
    proof.d2_door_capture_item_index = 247;
    bytes.d2_d3_capture.d2_item_index = 247;
    expect_int("bytes.plan.doorset0",
               csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                   &proof, 1, 0, 5, 5, &plan), 1);
    expect_int("bytes.bind.doorset0",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 1);

    expect_int("bytes.raster.reject.path.mismatch",
               csb_v1_viewport_consume_first_frame_material_raster_pc34(
                   &material_receipt, &plan, &bytes, "/wrong/CSBGRAPHICS.DAT",
                   bytes.source_md5, framebuffer, 224, 169, &raster_receipt), 0);
    expect_int("bytes.raster.reject.md5.mismatch",
               csb_v1_viewport_consume_first_frame_material_raster_pc34(
                   &material_receipt, &plan, &bytes, bytes.source_path,
                   "fedcba9876543210fedcba9876543210", framebuffer, 224, 169,
                    &raster_receipt), 0);
    bytes.d2_d3_capture.palette_capture_fnv1a ^= 1u;
    expect_int("bytes.reject.mutated.palette.capture",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 0);
    bytes.d2_d3_capture.palette_capture_fnv1a = proof.shared_palette_hash;
    bytes.d2_d3_capture.d2_decoded_pixels = NULL;
    expect_int("bytes.reject.missing.d2.capture.span",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 0);
    bytes.d2_d3_capture.d2_decoded_pixels = d2_pixels;
    plan.commands[0].decoded_pixels = NULL;
    expect_int("bytes.raster.reject.stale.command",
               csb_v1_viewport_consume_first_frame_material_raster_pc34(
                   &material_receipt, &plan, &bytes, bytes.source_path,
                   bytes.source_md5, framebuffer, 224, 169, &raster_receipt), 0);

    memset(&binding, 0, sizeof(binding));
    memset(live_framebuffer, 0, sizeof(live_framebuffer));
    binding.real_graphics_session = 1;
    binding.first_frame_material_proof = &proof;
    binding.first_frame_material_bytes = &bytes;
    binding.first_frame_material_source_path = bytes.source_path;
    binding.first_frame_material_source_md5 = bytes.source_md5;
    csb_v1_viewport_init(&cfg);
    cfg.viewport_pixels = live_framebuffer;
    cfg.viewport_stride = 320;
    csb_v1_viewport_apply_runtime_drawer_binding(&cfg, &binding);
    csb_v1_viewport_render_frame(&cfg, 0, 5, 5);
    csb_v1_viewport_runtime_draw_counts_from_config(&cfg, &counts);
    expect_int("bytes.live.raster.consumed",
               counts.first_frame_material_raster_consumed_count, 1);
    expect_int("bytes.live.raster.not.blocked",
               counts.first_frame_material_raster_blocked_count, 0);
    expect_u32_nonzero("bytes.live.raster.hash",
                       counts.first_frame_material_raster_hash);

    binding.first_frame_material_source_md5 =
        "fedcba9876543210fedcba9876543210";
    csb_v1_viewport_apply_runtime_drawer_binding(&cfg, &binding);
    csb_v1_viewport_render_frame(&cfg, 0, 5, 5);
    csb_v1_viewport_runtime_draw_counts_from_config(&cfg, &counts);
    expect_int("bytes.live.raster.reject.stale.source",
               counts.first_frame_material_raster_consumed_count, 0);
    expect_int("bytes.live.raster.blocked.stale.source",
               counts.first_frame_material_raster_blocked_count, 1);
}

static void test_live_wall_floor_door_frame_progression(void)
{
    static const unsigned char palette[] = { 3u, 5u, 7u, 11u, 13u, 17u };
    static const unsigned char wall[] = { 71u, 72u, 73u, 74u };
    static const unsigned char floor[] = { 81u, 82u, 83u, 84u };
    static const unsigned char door[] = { 91u, 92u, 93u, 94u };
    CSB_V1_ViewportLiveFrameSourcePc34 source;
    CSB_V1_ViewportLiveFrameProgressionPc34 progression;
    CSB_V1_ViewportLiveFrameReceiptPc34 receipt;
    CSB_V1_ViewportFirstFrameMaterializationReceipt base_receipt;
    uint8_t framebuffer[32 * 24];
    uint32_t raster_hash = 0u;
    int i;

    memset(&source, 0, sizeof(source));
    memset(&progression, 0, sizeof(progression));
    memset(&base_receipt, 0, sizeof(base_receipt));
    base_receipt.valid = 1;
    base_receipt.consumed_by_m11_render = 1;
    base_receipt.real_graphics_session = 1;
    base_receipt.no_synthetic_pixels = 1;
    base_receipt.no_fallback_visuals = 1;
    source.valid = 1;
    source.source_path = "/verified/CSBGRAPHICS.DAT";
    source.source_md5 = "0123456789abcdef0123456789abcdef";
    source.palette.decoded_palette = palette;
    source.palette.decoded_size = sizeof(palette);
    source.palette.decoded_fnv1a = fnv1a32(palette, sizeof(palette));
    for (i = 0; i < CSB_V1_VIEWPORT_LIVE_FRAME_SURFACE_COUNT_PC34; ++i) {
        static const unsigned char *const pixels[] = { wall, floor, door };
        source.surfaces[i].kind = (CSB_V1_ViewportLiveSurfaceKindPc34)i;
        source.surfaces[i].decoded_pixels = pixels[i];
        source.surfaces[i].decoded_size = 4u;
        source.surfaces[i].decoded_fnv1a = fnv1a32(pixels[i], 4u);
        source.surfaces[i].width = 2;
        source.surfaces[i].height = 2;
        source.surfaces[i].clip_x = i * 8;
        source.surfaces[i].clip_y = 4;
        source.surfaces[i].clip_w = 8;
        source.surfaces[i].clip_h = 8;
        source.surfaces[i].transparent_color = 10;
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    source.frame_number = 0u;
    source.door_state = 4;
    expect_int("live.frame0.admit", csb_v1_viewport_admit_live_frame_progression_pc34(
                   &progression, &base_receipt, &source, source.source_path,
                   source.source_md5, &receipt), 1);
    expect_int("live.frame0.raster", csb_v1_viewport_consume_live_frame_raster_pc34(
                   &receipt, &source, framebuffer, 32, 24, &raster_hash), 1);
    expect_u32_nonzero("live.frame0.raster.hash", raster_hash);

    source.frame_number = 1u;
    source.door_state = 3;
    expect_int("live.frame1.admit", csb_v1_viewport_admit_live_frame_progression_pc34(
                   &progression, NULL, &source, source.source_path,
                   source.source_md5, &receipt), 1);
    source.frame_number = 2u;
    source.door_state = 2;
    expect_int("live.frame2.admit", csb_v1_viewport_admit_live_frame_progression_pc34(
                   &progression, NULL, &source, source.source_path,
                   source.source_md5, &receipt), 1);
    expect_int("live.frame2.raster", csb_v1_viewport_consume_live_frame_raster_pc34(
                   &receipt, &source, framebuffer, 32, 24, &raster_hash), 1);

    source.frame_number = 4u;
    source.door_state = 1;
    expect_int("live.reject.skipped.frame", csb_v1_viewport_admit_live_frame_progression_pc34(
                   &progression, NULL, &source, source.source_path,
                   source.source_md5, &receipt), 0);
    source.frame_number = 3u;
    source.door_state = 0;
    expect_int("live.reject.door.jump", csb_v1_viewport_admit_live_frame_progression_pc34(
                   &progression, NULL, &source, source.source_path,
                   source.source_md5, &receipt), 0);
    source.door_state = 1;
    source.source_md5 = "fedcba9876543210fedcba9876543210";
    expect_int("live.reject.source.mismatch", csb_v1_viewport_admit_live_frame_progression_pc34(
                   &progression, NULL, &source, "/verified/CSBGRAPHICS.DAT",
                   "0123456789abcdef0123456789abcdef", &receipt), 0);
    source.source_md5 = "0123456789abcdef0123456789abcdef";
    source.surfaces[1].decoded_fnv1a ^= 1u;
    expect_int("live.reject.stale.floor", csb_v1_viewport_admit_live_frame_progression_pc34(
                   &progression, NULL, &source, source.source_path,
                   source.source_md5, &receipt), 0);
}

static int read_real_graphics_item_hash(const char *path,
                                        unsigned item_index,
                                        size_t *out_size,
                                        uint32_t *out_hash)
{
    FILE *fp;
    unsigned char header[4];
    unsigned char *table = NULL;
    unsigned char *payload = NULL;
    unsigned count;
    size_t table_bytes;
    size_t payload_offset;
    size_t payload_size;
    unsigned i;
    int ok = 0;

    if (!path || !out_size || !out_hash) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(header, 1u, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (read_be16(header) != 0x8001u) {
        fclose(fp);
        return 0;
    }
    count = read_be16(header + 2u);
    if (count == 0u || item_index >= count || count > 2048u) {
        fclose(fp);
        return 0;
    }

    table_bytes = (size_t)count * 4u;
    table = (unsigned char *)malloc(table_bytes);
    if (!table || fread(table, 1u, table_bytes, fp) != table_bytes) {
        free(table);
        fclose(fp);
        return 0;
    }

    payload_offset = 4u + table_bytes;
    for (i = 0u; i < item_index; ++i) {
        payload_offset += read_be16(table + (size_t)i * 2u);
    }
    payload_size = read_be16(table + (size_t)item_index * 2u);
    if (payload_size == 0u ||
        read_be16(table + (size_t)count * 2u + (size_t)item_index * 2u) == 0u ||
        fseek(fp, (long)payload_offset, SEEK_SET) != 0) {
        free(table);
        fclose(fp);
        return 0;
    }

    payload = (unsigned char *)malloc(payload_size);
    if (payload && fread(payload, 1u, payload_size, fp) == payload_size) {
        *out_size = payload_size;
        *out_hash = fnv1a32(payload, payload_size);
        ok = *out_hash != 0u;
    }
    free(payload);
    free(table);
    fclose(fp);
    return ok;
}

static int read_real_graphics_catalog_hash(const char *path,
                                           unsigned *out_count,
                                           uint32_t *out_hash)
{
    FILE *fp;
    unsigned char header[4];
    unsigned char *catalog = NULL;
    unsigned count;
    size_t catalog_bytes;
    int ok = 0;

    if (!path || !out_count || !out_hash) return 0;
    fp = fopen(path, "rb");
    if (!fp) return 0;
    if (fread(header, 1u, sizeof(header), fp) != sizeof(header)) {
        fclose(fp);
        return 0;
    }
    if (read_be16(header) != 0x8001u) {
        fclose(fp);
        return 0;
    }
    count = read_be16(header + 2u);
    if (count == 0u || count > 2048u) {
        fclose(fp);
        return 0;
    }
    catalog_bytes = 4u + (size_t)count * 4u;
    catalog = (unsigned char *)malloc(catalog_bytes);
    if (!catalog) {
        fclose(fp);
        return 0;
    }
    memcpy(catalog, header, sizeof(header));
    if (fread(catalog + sizeof(header), 1u, catalog_bytes - sizeof(header), fp) ==
        catalog_bytes - sizeof(header)) {
        *out_count = count;
        *out_hash = fnv1a32(catalog, catalog_bytes);
        ok = *out_hash != 0u;
    }
    free(catalog);
    fclose(fp);
    return ok;
}

static const char *graphics_dat_path(void)
{
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    if (path && path[0]) {
        return path;
    }
    return "/Users/bosse/.firestaff/data/csb/GRAPHICS.DAT";
}

static int build_route_receipts_and_proof(
    CSB_V1_ViewportFirstFrameMaterialProof *proof)
{
    const char *path = graphics_dat_path();
    const CSB_V1_D0L2D0R2F0111DoorFrontSpecPc34 *d0_door_spec;
    const CSB_V1_D0L2D0R2F0115ThingPassPc34 *d0_thing_spec;
    const CSB_V1_ViewportD1CF0111DoorPc34Contract *d1_door_contract;
    const CSB_V1_D1CF0115ThingPassPc34 *d1_thing_pass;
    const CSB_V1_D2CF0111DoorFrontSpecPc34 *d2_door_spec;
    CSB_V1_D0L2D0R2F0111DoorFrontRealAssetReceiptPc34 d0_door;
    CSB_V1_D0L2D0R2F0115ThingPassRealAssetReceiptPc34 d0_thing;
    CSB_V1_ViewportD1CF0111DoorRealAssetReceiptPc34 d1_door;
    CSB_V1_D1CF0115ThingPassRealAssetReceiptPc34 d1_thing;
    CSB_V1_D2CF0111DoorFrontRealAssetReceiptPc34 d2_door;
    size_t d0_door_size = 0u;
    size_t d0_thing_size = 0u;
    size_t d1_door_size = 0u;
    size_t d1_thing_size = 0u;
    size_t d2_door_size = 0u;
    uint32_t d0_door_hash = 0u;
    uint32_t d0_thing_hash = 0u;
    uint32_t d1_door_hash = 0u;
    uint32_t d1_thing_hash = 0u;
    uint32_t d2_door_hash = 0u;
    uint32_t catalog_hash = 0u;
    unsigned item_count = 0u;

    memset(proof, 0, sizeof(*proof));
    if (!read_real_graphics_catalog_hash(path, &item_count, &catalog_hash)) {
        printf("SKIP real CSB GRAPHICS.DAT unavailable at %s\n", path);
        return 0;
    }

    d0_door_spec = csb_v1_viewport_d0l2_d0r2_f0111_door_front_spec_for_side_pc34(
        CSB_V1_D0L2_D0R2_F0111_SIDE_D0L2_PC34);
    d0_thing_spec = csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_for_square_pc34(
        CSB_V1_D0L2_D0R2_F0115_SIDE_D0L2_PC34);
    d1_door_contract = csb_v1_viewport_d1c_f0111_door_pc34_contract();
    d1_thing_pass = csb_v1_viewport_d1c_f0115_thing_pass_for_pass_pc34(
        CSB_V1_D1C_F0115_PASS_BACK_PC34);
    d2_door_spec = csb_v1_viewport_d2c_f0111_door_front_spec_pc34();

    if (!d0_door_spec || !d0_thing_spec || !d1_door_contract ||
        !d1_thing_pass || !d2_door_spec) {
        return 0;
    }
    if (!read_real_graphics_item_hash(path, 693u, &d0_door_size, &d0_door_hash) ||
        !read_real_graphics_item_hash(path, (unsigned)d0_thing_spec->wall_frame_row,
                                      &d0_thing_size, &d0_thing_hash) ||
        !read_real_graphics_item_hash(path, 558u, &d1_door_size, &d1_door_hash) ||
        !read_real_graphics_item_hash(path, 498u, &d1_thing_size, &d1_thing_hash) ||
        !read_real_graphics_item_hash(path, 247u, &d2_door_size, &d2_door_hash)) {
        return 0;
    }

    expect_int("route.d0.door.receipt",
               csb_v1_viewport_d0l2_d0r2_f0111_door_front_real_asset_receipt_pc34(
                   d0_door_spec, 1, 1, 1, d0_door_spec->f0111_front_bitmap_id,
                   d0_door_spec->f0111_door_ornament_view, d0_door_size,
                   d0_door_hash, &d0_door), 1);
    expect_int("route.d0.thing.receipt",
               csb_v1_viewport_d0l2_d0r2_f0115_thing_pass_real_asset_receipt_pc34(
                   d0_thing_spec, 1, 1, 1, d0_thing_spec->wall_frame_row,
                   d0_thing_size, d0_thing_hash, &d0_thing), 1);
    expect_int("route.d1.door.receipt",
               csb_v1_viewport_d1c_f0111_door_real_asset_receipt_pc34(
                   d1_door_contract, 1, 1, 1, 558, d1_door_size,
                   d1_door_hash, &d1_door), 1);
    expect_int("route.d1.thing.receipt",
               csb_v1_viewport_d1c_f0115_thing_pass_real_asset_receipt_pc34(
                   d1_thing_pass, 1, 1, 1, 1, 498, d1_thing_size,
                   d1_thing_hash, &d1_thing), 1);
    expect_int("route.d2.door.receipt",
               csb_v1_viewport_d2c_f0111_door_front_real_asset_receipt_pc34(
                   d2_door_spec, 1, 1, 1, 247, d2_door_size,
                   d2_door_hash, &d2_door), 1);

    proof->valid = d0_door.valid && d0_thing.valid && d1_door.valid &&
                   d1_thing.valid && d2_door.valid;
    proof->route_mask = CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES;
    proof->source_graphics_dat_bound = 1;
    proof->no_synthetic_pixels = 1;
    proof->no_fallback_visuals = 1;
    proof->shared_palette_material_proof = 1;
    proof->shared_palette_hash = catalog_hash;
    proof->d0_door_hash = d0_door.source_payload_hash;
    proof->d0_thing_hash = d0_thing.source_payload_hash;
    proof->d1_door_hash = d1_door.source_payload_hash;
    proof->d1_thing_hash = d1_thing.source_payload_hash;
    proof->d2_door_hash = d2_door.source_payload_hash;
    set_d2_door_capture(proof, d2_door.source_byte_count,
                        d2_door.source_payload_hash);
    proof->source_item_count = item_count;
    proof->source_evidence =
        "ReDMCSB DUNVIEW.C F0128 first-frame D0/D1/D2 F0111/F0115 routes; "
        "DMCSB1 GRAPHICS.DAT catalog and material payloads";

    return 1;
}

static void test_real_d3_pair_extends_material_plan(void)
{
    const char *path = graphics_dat_path();
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2;
    CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34 d3_receipt;
    CSB_V1_ViewportFirstFrameMaterialProof proof;
    CSB_V1_ViewportRuntimeDrawPlanPc34 plan;
    size_t payload_size = 0u;
    uint32_t payload_hash = 0u;

    if (!build_route_receipts_and_proof(&proof)) return;
    if (!read_real_graphics_item_hash(path, 246u, &payload_size, &payload_hash)) {
        printf("SKIP real D3 G0693 source unavailable at %s\n", path);
        return;
    }
    d3l2 = csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
        CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    d3r2 = csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
        CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);
    expect_int("d3.receipt",
               csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
                   d3l2, d3r2, 1, 1, 1, 246, payload_size, payload_hash,
                   &d3_receipt), 1);
    proof.route_mask |= CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR |
                        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;
    proof.d3l2_door_hash = d3_receipt.source_payload_hash;
    proof.d3r2_door_hash = d3_receipt.source_payload_hash;
    proof.d3_pair_real_asset_receipt = d3_receipt;
    expect_int("d3.proof.valid",
               csb_v1_viewport_first_frame_material_proof_valid_pc34(&proof), 1);
    expect_int("d3.plan",
               csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                   &proof, 1, 0, 5, 5, &plan), 1);
    expect_int("d3.plan.command.count", plan.command_count, 7);
    expect_int("d3.plan.first.route", plan.commands[0].route,
               CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3L2_F0111_DOOR_PC34);
    expect_int("d3.plan.second.route", plan.commands[1].route,
               CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D3R2_F0111_DOOR_PC34);
    expect_int("d3.plan.first.geometry",
               plan.commands[0].clip_x == 24 && plan.commands[0].clip_y == 28 &&
               plan.commands[0].clip_w == 48 && plan.commands[0].clip_h == 40, 1);

    proof.route_mask &= ~CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;
    expect_int("d3.reject.unpaired.route",
               csb_v1_viewport_first_frame_material_proof_valid_pc34(&proof), 0);
    proof.route_mask |= CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;
    memset(&proof.d3_pair_real_asset_receipt, 0,
           sizeof(proof.d3_pair_real_asset_receipt));
    expect_int("d3.reject.missing.real.receipt",
               csb_v1_viewport_first_frame_material_proof_valid_pc34(&proof), 0);
    proof.d3_pair_real_asset_receipt = d3_receipt;
    proof.d3r2_door_hash = 0u;
    expect_int("d3.reject.missing.right.hash",
               csb_v1_viewport_first_frame_material_proof_valid_pc34(&proof), 0);
}

static void test_d2_d3_capture_span_handoff_and_raster(void)
{
    static const unsigned char palette[] = { 2u, 3u, 5u, 7u, 11u, 13u };
    static const unsigned char graphics_table[] = {
        0x80u, 0x01u, 0x00u, 0x02u,
        0x00u, 0x10u, 0x00u, 0x00u,
        0x00u, 0x10u, 0x00u, 0x00u
    };
    static unsigned char d2_packed[32u * 61u];
    static unsigned char d3_packed[22u * 38u];
    static unsigned char d2_pixels[64u * 61u];
    static unsigned char d3_pixels[44u * 38u];
    static const unsigned char other_pixels[4][4] = {
        { 31u, 32u, 33u, 34u }, { 41u, 42u, 43u, 44u },
        { 51u, 52u, 53u, 54u }, { 61u, 62u, 63u, 64u }
    };
    CSB_V1_ViewportFirstFrameMaterialProof proof;
    CSB_V1_ViewportRuntimeDrawPlanPc34 plan;
    CSB_V1_ViewportFirstFrameMaterialBytesPc34 bytes;
    CSB_V1_ViewportFirstFrameMaterializationReceipt material_receipt;
    CSB_V1_ViewportFirstFrameRasterReceiptPc34 raster_receipt;
    CSB_V1_ViewportD2D3NativePackedCapturePc34 packed_capture;
    CSB_V1_ViewportD2D3MaterialCaptureReceiptPc34 decoded_capture;
    CSB_V1_ViewportGraphicsTableProvenancePc34 d2_table;
    CSB_V1_ViewportGraphicsTableProvenancePc34 d3_table;
    CSB_V1_ViewportGraphicsTableProvenancePc34 rejected_table;
    CSB_V1_ViewportD3L2D3R2F0111DoorRealAssetReceiptPc34 d3_receipt;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3l2;
    const CSB_V1_ViewportD3L2D3R2F0111DoorRouteSpecPc34 *d3r2;
    uint8_t framebuffer[224 * 169];
    int i;

    memset(d2_packed, 0xa5, sizeof(d2_packed));
    memset(d3_packed, 0x9a, sizeof(d3_packed));
    memset(&proof, 0, sizeof(proof));
    proof.valid = 1;
    proof.route_mask = CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES |
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR |
        CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR;
    proof.source_graphics_dat_bound = 1;
    proof.no_synthetic_pixels = 1;
    proof.no_fallback_visuals = 1;
    proof.shared_palette_material_proof = 1;
    proof.shared_palette_hash = fnv1a32(palette, sizeof(palette));
    proof.source_item_count = 7u;
    proof.source_evidence = "structural original GRAPHICS.DAT D2/D3 capture";
    proof.d2_door_hash = fnv1a32(d2_packed, sizeof(d2_packed));
    proof.d1_door_hash = fnv1a32(other_pixels[0], sizeof(other_pixels[0]));
    proof.d1_thing_hash = fnv1a32(other_pixels[1], sizeof(other_pixels[1]));
    proof.d0_door_hash = fnv1a32(other_pixels[2], sizeof(other_pixels[2]));
    proof.d0_thing_hash = fnv1a32(other_pixels[3], sizeof(other_pixels[3]));
    set_d2_door_capture(&proof, 2047u, proof.d2_door_hash);
    d3l2 = csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
        CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3L2_PC34);
    d3r2 = csb_v1_viewport_d3l2_d3r2_f0111_door_for_side_pc34(
        CSB_V1_VIEWPORT_D3L2_D3R2_F0111_DOOR_SIDE_D3R2_PC34);
    expect_int("capture.d3.route.receipt",
               csb_v1_viewport_d3l2_d3r2_f0111_door_real_asset_receipt_pc34(
                   d3l2, d3r2, 1, 1, 1, 246, 307u,
                   fnv1a32(d3_packed, sizeof(d3_packed)), &d3_receipt), 1);
    proof.d3l2_door_hash = d3_receipt.source_payload_hash;
    proof.d3r2_door_hash = d3_receipt.source_payload_hash;
    proof.d3_pair_real_asset_receipt = d3_receipt;
    memset(&packed_capture, 0, sizeof(packed_capture));
    packed_capture.valid = 1;
    packed_capture.original_graphics_dat_capture = 1;
    packed_capture.f0489_native_bitmap_selected = 1;
    packed_capture.f0488_expand_4bpp = 1;
    packed_capture.no_synthetic_pixels = 1;
    packed_capture.no_fallback_visuals = 1;
    packed_capture.source_path = "/verified/GRAPHICS.DAT";
    packed_capture.source_md5 = "0123456789abcdef0123456789abcdef";
    packed_capture.palette_source_path = packed_capture.source_path;
    packed_capture.palette_source_md5 = packed_capture.source_md5;
    packed_capture.palette_capture_fnv1a = proof.shared_palette_hash;
    packed_capture.capture_identity_hash = 0x693694u;
    packed_capture.d2_item_index = proof.d2_door_capture_item_index;
    packed_capture.d2_source_payload_hash = proof.d2_door_hash;
    packed_capture.d2_packed_pixels = d2_packed;
    packed_capture.d2_packed_size = sizeof(d2_packed);
    packed_capture.d2_packed_fnv1a = proof.d2_door_hash;
    packed_capture.d3_item_index = 246;
    packed_capture.d3_source_payload_hash = proof.d3l2_door_hash;
    packed_capture.d3_packed_pixels = d3_packed;
    packed_capture.d3_packed_size = sizeof(d3_packed);
    packed_capture.d3_packed_fnv1a = proof.d3l2_door_hash;
    expect_int("table.reject.truncated.layout",
               csb_v1_viewport_admit_graphics_table_provenance_pc34(
                   graphics_table, 4u, packed_capture.source_path,
                   packed_capture.source_md5,
                   (uint32_t)proof.d2_door_capture_item_index, &rejected_table), 0);
    expect_int("table.reject.invalid.source.identity",
               csb_v1_viewport_admit_graphics_table_provenance_pc34(
                   graphics_table, sizeof(graphics_table),
                   packed_capture.source_path, "not-an-md5",
                   (uint32_t)proof.d2_door_capture_item_index,
                   &rejected_table), 0);
    expect_int("table.admit.d2.original.layout",
               csb_v1_viewport_admit_graphics_table_provenance_pc34(
                   graphics_table, sizeof(graphics_table),
                   packed_capture.source_path, packed_capture.source_md5,
                   (uint32_t)proof.d2_door_capture_item_index,
                   &d2_table), 1);
    expect_int("table.admit.d3.original.layout",
               csb_v1_viewport_admit_graphics_table_provenance_pc34(
                   graphics_table, sizeof(graphics_table),
                   packed_capture.source_path, packed_capture.source_md5, 246u,
                   &d3_table), 1);
    expect_int("table.reject.no.native.mapping",
               d2_table.native_bitmap_mapping_proven == 0 &&
               d2_table.raster_blocked_without_mapping == 1 &&
               d3_table.native_bitmap_mapping_proven == 0 &&
               d3_table.raster_blocked_without_mapping == 1, 1);
    packed_capture.d2_table_provenance = &d2_table;
    packed_capture.d3_table_provenance = &d3_table;
    expect_int("capture.decode.reject.unmapped.native.indices",
               csb_v1_viewport_decode_d2_d3_native_packed_capture_pc34(
                   &proof,
                   &(CSB_V1_ViewportFirstFramePaletteSpanPc34){
                       palette, sizeof(palette), proof.shared_palette_hash },
                   &packed_capture, d2_pixels, sizeof(d2_pixels), d3_pixels,
                   sizeof(d3_pixels), &decoded_capture), 0);
    expect_int("capture.decode.reject.no.receipt.publish",
               decoded_capture.valid == 0, 1);
    memset(d2_pixels, 0x0au, sizeof(d2_pixels));
    memset(d3_pixels, 0x09u, sizeof(d3_pixels));
    memset(&decoded_capture, 0, sizeof(decoded_capture));
    decoded_capture.valid = 1;
    decoded_capture.original_graphics_dat_capture = 1;
    decoded_capture.no_synthetic_pixels = 1;
    decoded_capture.no_fallback_visuals = 1;
    decoded_capture.source_path = packed_capture.source_path;
    decoded_capture.source_md5 = packed_capture.source_md5;
    decoded_capture.palette_source_path = packed_capture.source_path;
    decoded_capture.palette_source_md5 = packed_capture.source_md5;
    decoded_capture.palette_capture_fnv1a = proof.shared_palette_hash;
    decoded_capture.capture_identity_hash = packed_capture.capture_identity_hash;
    decoded_capture.d2_item_index = proof.d2_door_capture_item_index;
    decoded_capture.d2_source_byte_count = proof.d2_door_capture_byte_count;
    decoded_capture.d2_source_payload_hash = proof.d2_door_hash;
    decoded_capture.d2_decoded_pixels = d2_pixels;
    decoded_capture.d2_decoded_size = sizeof(d2_pixels);
    decoded_capture.d2_decoded_fnv1a = fnv1a32(d2_pixels, sizeof(d2_pixels));
    decoded_capture.d2_width = 64;
    decoded_capture.d2_height = 61;
    decoded_capture.d3_item_index = 246;
    decoded_capture.d3_source_byte_count =
        proof.d3_pair_real_asset_receipt.source_byte_count;
    decoded_capture.d3_source_payload_hash = proof.d3l2_door_hash;
    decoded_capture.d3_decoded_pixels = d3_pixels;
    decoded_capture.d3_decoded_size = sizeof(d3_pixels);
    decoded_capture.d3_decoded_fnv1a = fnv1a32(d3_pixels, sizeof(d3_pixels));
    decoded_capture.d3_width = 44;
    decoded_capture.d3_height = 38;
    proof.d2_door_decoded_hash = decoded_capture.d2_decoded_fnv1a;
    proof.d3l2_door_decoded_hash = decoded_capture.d3_decoded_fnv1a;
    proof.d3r2_door_decoded_hash = decoded_capture.d3_decoded_fnv1a;
    expect_int("capture.plan", csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                   &proof, 1, 0, 4, 4, &plan), 1);

    memset(&bytes, 0, sizeof(bytes));
    bytes.valid = 1;
    bytes.source_path = "/verified/GRAPHICS.DAT";
    bytes.source_md5 = "0123456789abcdef0123456789abcdef";
    bytes.palette.decoded_palette = palette;
    bytes.palette.decoded_size = sizeof(palette);
    bytes.palette.decoded_fnv1a = proof.shared_palette_hash;
    bytes.d2_d3_capture = decoded_capture;
    for (i = 0; i < plan.command_count; ++i) {
        CSB_V1_ViewportFirstFrameMaterialSpanPc34 *span = &bytes.materials[i];
        span->route_bit = plan.commands[i].route_bit;
        switch (span->route_bit) {
        case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3L2_F0111_DOOR:
        case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D3R2_F0111_DOOR:
            span->decoded_pixels = d3_pixels;
            span->decoded_size = sizeof(d3_pixels);
            span->decoded_fnv1a = proof.d3l2_door_decoded_hash;
            span->width = 44;
            span->height = 38;
            break;
        case CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D2_F0111_DOOR:
            span->decoded_pixels = d2_pixels;
            span->decoded_size = sizeof(d2_pixels);
            span->decoded_fnv1a = proof.d2_door_decoded_hash;
            span->width = 64;
            span->height = 61;
            break;
        default: {
            int slot = span->route_bit ==
                CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0111_DOOR ? 0 :
                (span->route_bit ==
                 CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D1_F0115_THING ? 1 :
                 (span->route_bit ==
                  CSB_V1_VIEWPORT_FIRST_FRAME_ROUTE_D0_F0111_DOOR ? 2 : 3));
            span->decoded_pixels = other_pixels[slot];
            span->decoded_size = sizeof(other_pixels[slot]);
            span->decoded_fnv1a = fnv1a32(other_pixels[slot],
                                           sizeof(other_pixels[slot]));
            span->width = 2;
            span->height = 2;
            break;
        }
        }
    }
    expect_int("capture.bind", csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 1);
    memset(framebuffer, 0, sizeof(framebuffer));
    expect_int("capture.raster", csb_v1_viewport_consume_first_frame_material_raster_pc34(
                   &material_receipt, &plan, &bytes, bytes.source_path,
                   bytes.source_md5, framebuffer, 224, 169, &raster_receipt), 1);
    expect_int("capture.raster.commands", raster_receipt.command_count, 7);
    bytes.d2_d3_capture.d3_decoded_fnv1a ^= 1u;
    expect_int("capture.raster.reject.mutated.d3.span",
               csb_v1_viewport_consume_first_frame_material_raster_pc34(
                   &material_receipt, &plan, &bytes, bytes.source_path,
                   bytes.source_md5, framebuffer, 224, 169, &raster_receipt), 0);
    bytes.d2_d3_capture.d3_decoded_fnv1a = proof.d3l2_door_decoded_hash;
    bytes.d2_d3_capture.d3_source_payload_hash ^= 1u;
    expect_int("capture.reject.mutated.d3.source",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 0);
    bytes.d2_d3_capture.d3_source_payload_hash = proof.d3l2_door_hash;
    bytes.d2_d3_capture.d3_decoded_pixels = NULL;
    expect_int("capture.reject.missing.d3.span",
               csb_v1_viewport_bind_first_frame_material_bytes_pc34(
                   &proof, &plan, &bytes, &material_receipt), 0);
}

static void test_m11_consumer_materializes_real_first_frame(void)
{
    CSB_V1_ViewportFirstFrameMaterialProof proof;
    CSB_V1_ViewportFirstFrameMaterializationReceipt receipt;
    CSB_V1_ViewportRuntimeDrawPlanPc34 draw_plan;
    CSB_V1_ViewportConfig cfg;
    CSB_V1_ViewportRuntimeDrawerBinding binding;
    CSB_V1_ViewportRuntimeDrawCounts counts;
    uint8_t framebuffer[320 * 200];
    uint32_t expected_hash;

    if (!build_route_receipts_and_proof(&proof)) {
        return;
    }

    expect_int("proof.valid",
               csb_v1_viewport_first_frame_material_proof_valid_pc34(&proof), 1);
    expect_int("proof.routes",
               (int)(proof.route_mask & CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES),
               (int)CSB_V1_VIEWPORT_FIRST_FRAME_REQUIRED_ROUTES);
    expect_u32_nonzero("proof.shared.catalog.hash", proof.shared_palette_hash);

    expect_int("admit.without.real.session",
               csb_v1_viewport_admit_first_frame_materialization_pc34(
                   &proof, 0, &receipt), 0);
    proof.d2_door_hash = 0u;
    expect_int("admit.reject.missing.d2.material",
               csb_v1_viewport_admit_first_frame_materialization_pc34(
                   &proof, 1, &receipt), 0);

    build_route_receipts_and_proof(&proof);
    proof.d2_door_capture_valid = 0;
    expect_int("admit.reject.missing.d2.capture",
               csb_v1_viewport_admit_first_frame_materialization_pc34(
                   &proof, 1, &receipt), 0);
    proof.d2_door_capture_valid = 1;
    proof.d2_door_capture_payload_hash ^= 1u;
    expect_int("admit.reject.mixed.d2.capture",
               csb_v1_viewport_admit_first_frame_materialization_pc34(
                   &proof, 1, &receipt), 0);

    build_route_receipts_and_proof(&proof);
    expect_int("admit.with.real.session",
               csb_v1_viewport_admit_first_frame_materialization_pc34(
                   &proof, 1, &receipt), 1);
    expect_int("receipt.consumed", receipt.consumed_by_m11_render, 1);
    expect_u32_nonzero("receipt.combined.hash", receipt.combined_material_hash);
    expected_hash = receipt.combined_material_hash;

    expect_int("draw.plan.with.real.session",
               csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                   &proof, 1, 0, 5, 5, &draw_plan), 1);
    expect_int("draw.plan.valid", draw_plan.valid, 1);
    expect_int("draw.plan.command.count", draw_plan.command_count, 5);
    expect_int("draw.plan.palette.bound", draw_plan.palette_bound, 1);
    expect_int("draw.plan.clip.bound", draw_plan.clip_bound, 1);
    expect_int("draw.plan.state.bound", draw_plan.state_bound, 1);
    expect_int("draw.plan.input.bound", draw_plan.input_bound, 1);
    expect_int("draw.plan.no.synthetic", draw_plan.no_synthetic_pixels, 1);
    expect_int("draw.plan.no.fallback", draw_plan.no_fallback_visuals, 1);
    expect_int("draw.plan.palette.hash.matches",
               draw_plan.shared_palette_hash == proof.shared_palette_hash, 1);
    expect_u32_nonzero("draw.plan.hash", draw_plan.plan_hash);
    expect_int("draw.plan.first.route",
               draw_plan.commands[0].route,
               CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D2_F0111_DOOR_PC34);
    expect_int("draw.plan.last.route",
               draw_plan.commands[4].route,
               CSB_V1_VIEWPORT_RUNTIME_DRAW_ROUTE_D0_F0115_THING_PC34);
    expect_int("draw.plan.d1.thing.transparent",
               draw_plan.commands[2].transparent_color, 10);
    expect_int("draw.plan.input.party.x", draw_plan.commands[0].party_x, 5);
    expect_int("draw.plan.input.party.y", draw_plan.commands[0].party_y, 5);
    expect_int("draw.plan.reject.no.real.session",
               csb_v1_viewport_build_first_frame_runtime_draw_plan_pc34(
                   &proof, 0, 0, 5, 5, &draw_plan), 0);

    memset(&binding, 0, sizeof(binding));
    memset(framebuffer, 0, sizeof(framebuffer));
    csb_v1_viewport_init(&cfg);
    cfg.viewport_pixels = framebuffer;
    cfg.viewport_stride = 320;
    binding.real_graphics_session = 1;
    binding.first_frame_material_proof = &proof;
    csb_v1_viewport_apply_runtime_drawer_binding(&cfg, &binding);
    csb_v1_viewport_render_frame(&cfg, 0, 0, 0);
    csb_v1_viewport_runtime_draw_counts_from_config(&cfg, &counts);

    expect_int("render.consumer.count", counts.first_frame_material_consumed_count, 1);
    expect_int("render.blocked.count", counts.first_frame_material_blocked_count, 0);
    expect_int("render.hash.matches", counts.first_frame_material_hash == expected_hash, 1);
    expect_int("render.draw.plan.consumed",
               counts.first_frame_draw_plan_consumed_count, 1);
    expect_int("render.draw.plan.blocked",
               counts.first_frame_draw_plan_blocked_count, 0);
    expect_int("render.draw.plan.command.count",
               counts.first_frame_draw_plan_command_count, 5);
    expect_int("render.draw.plan.palette.hash",
               counts.first_frame_draw_plan_palette_hash ==
                   proof.shared_palette_hash, 1);
    expect_u32_nonzero("render.draw.plan.hash",
                       counts.first_frame_draw_plan_hash);

    binding.real_graphics_session = 0;
    csb_v1_viewport_apply_runtime_drawer_binding(&cfg, &binding);
    csb_v1_viewport_render_frame(&cfg, 0, 0, 0);
    csb_v1_viewport_runtime_draw_counts_from_config(&cfg, &counts);
    expect_int("render.fail.closed.no.real.session",
               counts.first_frame_material_consumed_count, 0);
    expect_int("render.fail.closed.blocked", counts.first_frame_material_blocked_count, 1);
    expect_int("render.draw.plan.fail.closed",
               counts.first_frame_draw_plan_consumed_count, 0);
    expect_int("render.draw.plan.blocked.no.real.session",
               counts.first_frame_draw_plan_blocked_count, 1);
}

int main(void)
{
    test_checked_material_byte_handoff_and_raster();
    test_live_wall_floor_door_frame_progression();
    test_real_d3_pair_extends_material_plan();
    test_d2_d3_capture_span_handoff_and_raster();
    test_m11_consumer_materializes_real_first_frame();
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
