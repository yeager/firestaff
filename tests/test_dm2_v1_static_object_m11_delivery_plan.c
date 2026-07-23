#include "dm2_v1_runtime.h"
#include "dm2_v1_viewport_renderer.h"
#include <stdio.h>
#include <string.h>

#define CHECK(cond) do { if (!(cond)) { fprintf(stderr, "check failed: " #cond " (line %d)\n", __LINE__); ok = 0; } } while (0)

static int make_plan(int category, int opened,
                     DM2_V1_G1StaticObjectMaterialReceipt *material,
                     DM2_V1_StaticObjectSourcePlan *source)
{
    DM2_V1_G1DirectWeaponRoot weapon = { 1, 2, 0x1401u, 1u, 2u, 0u, 0u, 0u };
    DM2_V1_G1DirectContainerRoot container = { 1, 2, 0xe401u, 1u, 3u, 0u, 1u };
    uint8_t raw[4] = { 1, 2, 3, 4 };
    int pass = dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(3);
    uint32_t visibility_mask;
    if (!material || !source || pass < 0) return 0;
    memset(material, 0, sizeof(*material));
    if (category == 0x10) {
        if (!dm2_v1_g1_static_object_material_selector(&weapon, 0x1234u,
                                                        &material->selector)) return 0;
    } else {
        container.opened = opened ? 1u : 0u;
        if (!dm2_v1_g1_static_container_material_selector(&container, 0x1234u,
                                                           &material->selector)) return 0;
    }
    visibility_mask = (uint32_t)(1u << (unsigned)
        ((uint8_t[]){6, 8, 18, 16}[material->selector.direction & 3]));
    if (!dm2_v1_viewport_static_object_source_plan(
            3, pass, category, material->selector.direction,
            material->selector.container_open, 0, 0, 1u, visibility_mask, source)) return 0;
    material->raw_gfx256_bytes = raw;
    material->raw_gfx256_byte_count = sizeof(raw);
    material->raw_gfx256_hash = 11;
    material->raw_gfx256_receipt_hash = 12;
    material->local_palette_hash = 13;
    material->clip_rect_id = (uint16_t)(source->clip_rect_id & 0x7fffu);
    material->raw4_hash = 14;
    material->raw4_receipt_hash = 15;
    return 1;
}

int main(void)
{
    DM2_V1_G1StaticObjectMaterialReceipt material;
    DM2_V1_StaticObjectSourcePlan source;
    DM2_V1_StaticObjectM11DeliveryPlan plan;
    int ok = make_plan(0x10, 0, &material, &source) &&
        dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan) && plan.valid && plan.no_draw &&
        !plan.pixel_decoder_ready && plan.m11_delivery_ready &&
        plan.category == 0x10u && plan.image_field == 0u &&
        plan.clip_rect_id == material.clip_rect_id &&
        dm2_v1_viewport_static_object_m11_delivery_plan_matches(
            &plan, &material, &source, 101);
    ok &= make_plan(0x14, 1, &material, &source) &&
        dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 202, &plan) && plan.category == 0x14u &&
        plan.image_field == 4u && plan.container_open &&
        dm2_v1_viewport_static_object_m11_delivery_plan_matches(
            &plan, &material, &source, 202);
    ok &= !dm2_v1_viewport_static_object_m11_delivery_plan_matches(
        &plan, &material, &source, 203);
    material.raw_gfx256_hash = 99;
    ok &= !dm2_v1_viewport_static_object_m11_delivery_plan_matches(
        &plan, &material, &source, 202);
    material.raw_gfx256_hash = 11;
    material.raw4_receipt_hash = 99;
    ok &= !dm2_v1_viewport_static_object_m11_delivery_plan_matches(
        &plan, &material, &source, 202);
    material.raw4_receipt_hash = 15;
    source.clip_rect_id ^= 1;
    ok &= !dm2_v1_viewport_static_object_m11_delivery_plan_matches(
        &plan, &material, &source, 202);
    make_plan(0x10, 0, &material, &source);
    material.selector.image_field = 0xf9u;
    ok &= !dm2_v1_viewport_build_static_object_m11_delivery_plan(
        &material, &source, 101, &plan);

    /* Source 5x5 visibility mask and record-list ordinal are mandatory and
     * fail-closed. A zero ordinal, zero mask, or a mask that does not contain
     * the object's position blocks M11 delivery. */
    {
        uint32_t good_mask;
        make_plan(0x10, 0, &material, &source);
        good_mask = source.visibility_mask_5x5;
        CHECK(dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan));
        CHECK(plan.record_list_ordinal == 1u);
        CHECK(plan.visibility_mask_5x5 == good_mask);
        source.record_list_ordinal = 0u;
        CHECK(!dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan));
        source.record_list_ordinal = 1u;
        source.visibility_mask_5x5 = 0u;
        CHECK(!dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan));
        source.visibility_mask_5x5 = good_mask & ~(1u << source.position_5x5);
        CHECK(!dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan));
        source.visibility_mask_5x5 = good_mask;
        CHECK(dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan));
    }

    /* Real INTERFACE_GENERAL dt07/0x0A Rect14 wiring: a synthetic row that
     * matches the static object's clip rect enriches the source plan and
     * changes the M11 delivery identity without overriding the GDAT image
     * field selected by the record. */
    {
        uint8_t rect14_weapon[14] = {
            18, 0, 0, 0, 0, 0, 64, 64, 64, 64, 0, 0, 0, 0
        };
        uint8_t rect14_container[14] = {
            16, 0, 4, 4, 4, 4, 64, 64, 64, 64, 0, 0, 0, 0
        };
        uint8_t rect14_none[14] = {
            0, 0, 0, 0, 0, 0, 64, 64, 64, 64, 0, 0, 0, 0
        };
        const uint8_t *rows[2] = { rect14_weapon, rect14_container };
        uint32_t table_hash = 0x12345678u;
        uint32_t no_rect_id, rect_id;

#define CHECKRect14(cond) do { if (!(cond)) { fprintf(stderr, "rect14 check failed: " #cond " (line %d)\n", __LINE__); ok = 0; } } while (0)
        CHECKRect14(make_plan(0x10, 0, &material, &source));
        CHECKRect14(source.rect14_applied == 0);
        CHECKRect14(source.rect14_row_hash == 0u);
        CHECKRect14(dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan));
        no_rect_id = plan.identity_hash;
        CHECKRect14(plan.rect14_row_hash == 0u);
        CHECKRect14(plan.rect14_placement_hash == 0u);

        CHECKRect14(dm2_v1_viewport_enrich_static_object_source_plan_with_rect14(
            rows[0], 1, table_hash, material.selector.direction, 0, &source));
        CHECKRect14(source.rect14_applied == 1);
        CHECKRect14(source.rect14_row_hash != 0u);
        CHECKRect14(source.rect14_placement_hash != 0u);
        CHECKRect14(source.rect14_image_field == 0u);
        CHECKRect14(source.rect14_scale64 == 64);
        CHECKRect14(source.rect14_flip_mirror == 0);
        CHECKRect14(dm2_v1_viewport_build_static_object_m11_delivery_plan(
            &material, &source, 101, &plan));
        rect_id = plan.identity_hash;
        CHECKRect14(rect_id != no_rect_id);
        CHECKRect14(plan.rect14_row_hash == source.rect14_row_hash);
        CHECKRect14(plan.rect14_placement_hash == source.rect14_placement_hash);
        CHECKRect14(dm2_v1_viewport_static_object_m11_delivery_plan_matches(
            &plan, &material, &source, 101));

        /* A table with no matching row must not synthesize placement data. */
        CHECKRect14(make_plan(0x14, 1, &material, &source));
        CHECKRect14(!dm2_v1_viewport_enrich_static_object_source_plan_with_rect14(
            rect14_none, 1, table_hash, material.selector.direction, 0, &source));
        CHECKRect14(source.rect14_applied == 0);

        /* The matching container row must bind the open-container image field
         * from the Rect14 record while keeping the source category intact. */
        CHECKRect14(make_plan(0x14, 1, &material, &source));
        CHECKRect14(dm2_v1_viewport_enrich_static_object_source_plan_with_rect14(
            rows[1], 1, table_hash, material.selector.direction, 0, &source));
        CHECKRect14(source.rect14_applied == 1);
        CHECKRect14(source.rect14_image_field == 4u);
#undef CHECKRect14
    }

    if (!ok) fputs("static object M11 delivery plan failed\n", stderr);
    return ok ? 0 : 1;
}
