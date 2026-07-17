#include "dm2_v1_runtime.h"
#include <stdio.h>
#include <string.h>

static int make_plan(int category, int opened,
                     DM2_V1_G1StaticObjectMaterialReceipt *material,
                     DM2_V1_StaticObjectSourcePlan *source)
{
    DM2_V1_G1DirectWeaponRoot weapon = { 1, 2, 0x1401u, 1u, 2u, 0u, 0u, 0u };
    DM2_V1_G1DirectContainerRoot container = { 1, 2, 0xe401u, 1u, 3u, 0u, 1u };
    uint8_t raw[4] = { 1, 2, 3, 4 };
    int pass = dm2_v1_viewport_draw_dungeon_tiles_pass_for_cell(3);
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
    if (!dm2_v1_viewport_static_object_source_plan(
            3, pass, category, material->selector.direction,
            material->selector.container_open, 0, source)) return 0;
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
    if (!ok) fputs("static object M11 delivery plan failed\n", stderr);
    return ok ? 0 : 1;
}
