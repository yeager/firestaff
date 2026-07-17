#include "dm2_v1_runtime.h"
#include <stdio.h>
#include <string.h>

static int make_receipt(DM2_V1_RuntimeFlyingItemReceipt *receipt)
{
    DM2_V1_G1FlyingItemSourceReceipt source;
    if (!receipt || !dm2_v1_g1_flying_item_source_receipt(
            0x3800u, 0x0d, 2, 8, 1, 3, 6, 0x40, &source)) return 0;
    memset(receipt, 0, sizeof(*receipt));
    receipt->valid = 1;
    receipt->no_draw = 1;
    receipt->source = source;
    receipt->raw_gfx256_hash = 11;
    receipt->raw_gfx256_receipt_hash = 12;
    receipt->palette_hash = 13;
    receipt->raw4_hash = 14;
    receipt->raw4_receipt_hash = 15;
    receipt->timer_receipt_hash = 16;
    receipt->identity_hash = 17;
    return 1;
}

int main(void)
{
    DM2_V1_RuntimeFlyingItemReceipt receipt;
    DM2_V1_FlyingItemM11DeliveryPlan plan;
    DM2_V1_G1FlyingItemSelectorReceipt selector;
    DM2_V1_G1FlyingItemGeometryReceipt geometry;
    DM2_V1_RuntimeFlyingItemViewportEvidence evidence;
    int ok = make_receipt(&receipt) &&
        dm2_v1_viewport_build_flying_item_m11_delivery_plan(&receipt, &plan) &&
        plan.valid && plan.no_draw && !plan.pixel_decoder_ready &&
        plan.m11_delivery_ready && plan.missile_object_id == 0x3800u &&
        plan.clip_rect_id == 5081u && plan.raw_gfx256_hash == 11u &&
        plan.raw_gfx256_receipt_hash == 12u && plan.palette_hash == 13u &&
        plan.raw4_hash == 14u && plan.raw4_receipt_hash == 15u &&
        plan.timer_receipt_hash == 16u && plan.identity_hash != 0u;
    for (int drift = 0; drift < 6; ++drift) {
        DM2_V1_RuntimeFlyingItemReceipt changed;
        make_receipt(&changed);
        if (drift == 0) changed.raw_gfx256_hash = 0;
        if (drift == 1) changed.raw_gfx256_receipt_hash = 0;
        if (drift == 2) changed.palette_hash = 0;
        if (drift == 3) changed.raw4_hash = 0;
        if (drift == 4) changed.raw4_receipt_hash = 0;
        if (drift == 5) changed.timer_receipt_hash = 0;
        ok &= !dm2_v1_viewport_build_flying_item_m11_delivery_plan(&changed, &plan) &&
            !plan.valid && !plan.m11_delivery_ready;
    }
    make_receipt(&receipt);
    receipt.source.image_field = 0xf9u;
    ok &= !dm2_v1_viewport_build_flying_item_m11_delivery_plan(&receipt, &plan);
    make_receipt(&receipt);
    memset(&selector, 0, sizeof(selector)); memset(&geometry, 0, sizeof(geometry));
    selector.valid = 1; selector.missile_object_id = receipt.source.missile_object_id;
    selector.identity_hash = 21; selector.branch_temp_picst = 1;
    geometry.valid = 1; geometry.no_draw = 1; geometry.temp_picst_eligible = 1;
    geometry.identity_hash = 22;
    ok &= dm2_v1_runtime_flying_item_viewport_evidence(
        &receipt, &selector, &geometry, 23, 24, &evidence) && evidence.valid &&
        evidence.no_draw && evidence.identity_hash &&
        dm2_v1_viewport_build_flying_item_m11_delivery_plan_from_viewport_evidence(
            &receipt, &evidence, &plan) && plan.no_draw &&
        plan.viewport_evidence_hash == evidence.identity_hash;
    geometry.image_field_available = 1;
    ok &= !dm2_v1_runtime_flying_item_viewport_evidence(
        &receipt, &selector, &geometry, 23, 24, &evidence);
    make_receipt(&receipt);
    receipt.no_draw = 0;
    ok &= !dm2_v1_viewport_build_flying_item_m11_delivery_plan(&receipt, &plan);
    if (!ok) fputs("flying item M11 frame-plan gate failed\n", stderr);
    return ok ? 0 : 1;
}
