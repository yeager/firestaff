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
        !read_real_graphics_item_hash(path, 694u, &d2_door_size, &d2_door_hash)) {
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
                   d2_door_spec, 1, 1, 1, 694, d2_door_size,
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
    proof->source_item_count = item_count;
    proof->source_evidence =
        "ReDMCSB DUNVIEW.C F0128 first-frame D0/D1/D2 F0111/F0115 routes; "
        "DMCSB1 GRAPHICS.DAT catalog and material payloads";

    return 1;
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
    test_m11_consumer_materializes_real_first_frame();
    printf("assertions=%d failures=%d\n", g_assertions, g_failures);
    return g_failures == 0 ? 0 : 1;
}
