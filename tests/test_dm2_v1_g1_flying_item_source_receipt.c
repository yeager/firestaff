#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_runtime.h"
#include <stdio.h>
#include <string.h>
int main(void) {
    DM2_V1_G1FlyingItemSourceReceipt r;
    DM2_V1_DungeonData d;
    DM2_V1_G1DirectMissileReceipt missile;
    DM2_V1_G1MissileTimerReceipt timer;
    DM2_V1_G1FlyingItemSelectorReceipt selector;
    DM2_V1_G1FlyingItemGeometryReceipt geometry;
    DM2_V1_G1FlyingItemVb30Inputs vb30_in;
    DM2_V1_G1FlyingItemVb30Receipt vb30;
    DM2_V1_QueryGdatSummaryImageReceipt summary;
    DM2_V1_G1FlyingItemDecodedMaterialReceipt decoded;
    DM2_V1_RuntimeFlyingItemReceipt timer_receipt;
    DM2_V1_RuntimeFlyingItemDecodedMaterialPlan material_plan;
    DM2_V1_FlyingItemM11DeliveryPlan delivery;
    DM2_V1_Dm2FlyingItemM11MaterialConsumerReceipt consumer;
    DM2_V1_BootExpandedRectReceipt clip;
    DM2_V1_Dm2FlyingItemDestinationReceipt destination;
    DM2_V1_Dm2FlyingItemPicstTransformReceipt transform;
    DM2_V1_G1FlyingItemDecodedMaterialReceipt zero_offset_material;
    uint8_t framebuffer[64];
    DM2_V1_Dm2ViewportM11CompositionReceipt composition;
    DM2_V1_Dm2ViewportM11MaterialCompositionReceipt material_composition;
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entries[4];
    uint32_t raw_offsets[1] = { 0u };
    uint32_t raw_sizes[1];
    uint8_t raw[16] = {0,0,0x00,0x14,0x56,0x78,0x00,0x00,
                       0,0,0x02,0x00,0x00,0,0,0};
    uint8_t gfx_raw[34] = {0};
    uint8_t timers[20] = {0};
    uint16_t rect = 0;
    int ok = dm2_v1_g1_query_creature_blit_recti(3, 6, 0, &rect) &&
        rect == 5081 &&
        dm2_v1_g1_query_creature_blit_recti(3, 6, 1, &rect) && rect == 5091 &&
        dm2_v1_g1_query_creature_blit_recti(3, 6, 2, &rect) && rect == 5093 &&
        dm2_v1_g1_query_creature_blit_recti(3, 6, 3, &rect) && rect == 5083 &&
        !dm2_v1_g1_query_creature_blit_recti(23, 6, 0, &rect) &&
        !dm2_v1_g1_query_creature_blit_recti(3, 25, 0, &rect) &&
        !dm2_v1_g1_query_creature_blit_recti(3, 6, 4, &rect);
    ok &= dm2_v1_g1_flying_item_source_receipt(0xe401u,0x0d,2,8,1,3,6,0x40,&r) &&
        r.valid && r.clip_rect_id==5081 && r.identity_hash!=0u;
    ok &= !dm2_v1_g1_flying_item_source_receipt(0xe401u,0x0d,2,0xf9,1,3,6,0x40,&r);
    ok &= !dm2_v1_g1_flying_item_source_receipt(0xe401u,0x0d,2,8,1,23,6,0x40,&r);
    ok &= !dm2_v1_g1_flying_item_source_receipt(0xe401u,0x0d,2,8,1,3,25,0x40,&r);
    timers[14] = 0x1d; timers[15] = 0x44; timers[16] = 0x12; timers[17] = 0x34;
    timers[18] = 0x00; timers[19] = 0x0c;
    ok &= dm2_v1_g1_direct_missile_timer_receipt(timers,sizeof(timers),1,&timer) &&
        timer.valid && timer.timer_type == 0x1d && timer.actor == 0x44 &&
        timer.value == 0x3412u && timer.direction == 3 && timer.raw_timer_hash;
    ok &= !dm2_v1_g1_direct_missile_timer_receipt(timers,sizeof(timers),2,&timer);
    ok &= !dm2_v1_g1_direct_missile_timer_receipt(timers,19,1,&timer);
    /* An original-layout IMG3 U4 image and its local 16-byte palette. */
    gfx_raw[0] = 4u; gfx_raw[2] = 4u; gfx_raw[3] = 0x80u; gfx_raw[4] = 4u;
    for (int i = 0; i < 8; ++i) gfx_raw[10 + i] = (uint8_t)(0x10u + i);
    for (int i = 0; i < 16; ++i) gfx_raw[18 + i] = (uint8_t)(0x80u + i);
    raw_sizes[0] = (uint32_t)sizeof(gfx_raw);
    memset(&d,0,sizeof(d)); d.raw_data=raw; d.raw_size=sizeof(raw);
    d.thing_data_bases[14]=0; d.thing_type_counts[14]=1;
    d.thing_data_bases[5]=8; d.thing_type_counts[5]=1;
    memset(&loader, 0, sizeof(loader)); memset(entries, 0, sizeof(entries));
    entries[0].cls1=0x0d; entries[0].cls2=2; entries[0].cls3=0x0b;
    entries[0].cls4=1; entries[0].data_index=1;
    loader.loaded=1; loader.category_count=DM2_GDAT_CATEGORY_LIMIT + 1;
    loader.data=gfx_raw; loader.data_size=sizeof(gfx_raw); loader.raw_data_count=1;
    loader.raw_offsets=raw_offsets; loader.raw_sizes=raw_sizes;
    loader.entries=entries; loader.entry_count=4;
    ok &= dm2_v1_g1_direct_missile_receipt(&d,0x3800u,&missile) && missile.valid &&
        missile.missile_object==0x1400u && missile.energy_remaining==0x56u &&
        missile.energy_remaining2==0x78u && missile.timer_index==0u &&
        dm2_v1_g1_flying_item_selector_receipt(&d,&loader,&missile,&selector) &&
        selector.valid && selector.class1==0x10u && selector.class2==2u &&
        !selector.branch_temp_picst && selector.image_data_index==0u &&
        dm2_v1_g1_flying_item_geometry_receipt(&selector, 3, &geometry) &&
        geometry.no_draw && geometry.draw_item_opaque && !geometry.image_field_available &&
        geometry.depth_band==1 && geometry.placement_x==0 && geometry.placement_y==8;
    /* SKWIN c_gui_vp.cpp:3545-3770: category 0x0d is the only TEMP_PICST
     * branch and requires record byte+4 != 0xff plus dtWordValue(0x0b,1). */
    raw[10]=0; raw[4]=0;
    entries[0].cls1=0x0d; entries[0].cls2=0; entries[0].cls3=0x0b;
    entries[0].cls4=1; entries[0].data_index=1;
    entries[1].cls1=0x0d; entries[1].cls2=0;
    entries[1].cls3=DM2_GDAT_ENTRY_TYPE_IMAGE; entries[1].cls4=8;
    entries[1].data_index=0;
    /* c_querydb.cpp:1794-1812 applies dtImageOffset/FE then field 8. */
    entries[2].cls1=0x0d; entries[2].cls2=0;
    entries[2].cls3=DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET; entries[2].cls4=0xfe;
    entries[2].data_index=0x0507u;
    entries[3].cls1=0x0d; entries[3].cls2=0;
    entries[3].cls3=DM2_GDAT_ENTRY_TYPE_IMAGE_OFFSET; entries[3].cls4=8;
    entries[3].data_index=0xfffeu;
    /* A type-15 spell-missile reference resolves class1=0x0d. */
    d.thing_data_bases[15]=8; d.thing_type_counts[15]=1;
    d.thing_type_counts[5]=0;
    raw[2]=0x00; raw[3]=0x3c;
    ok &= dm2_v1_g1_direct_missile_receipt(&d,0x3800u,&missile) &&
        dm2_v1_g1_flying_item_selector_receipt(&d,&loader,&missile,&selector) &&
        selector.class1==0x0du && selector.branch_temp_picst &&
        selector.image_data_index==1u &&
        dm2_v1_g1_flying_item_geometry_receipt(&selector, 11, &geometry) &&
        geometry.temp_picst_eligible && !geometry.draw_item_opaque &&
        geometry.depth_band==3 && geometry.placement_x==0 && geometry.placement_y==0 &&
        !dm2_v1_g1_flying_item_geometry_receipt(&selector, 1, &geometry);
    raw[4]=0xff;
    ok &= !dm2_v1_g1_flying_item_selector_receipt(&d,&loader,&missile,&selector);
    raw[4]=0;
    ok &= dm2_v1_g1_flying_item_selector_receipt(&d,&loader,&missile,&selector);
    ok &= dm2_v1_g1_flying_item_geometry_receipt(&selector, 11, &geometry);
    /* SKWIN c_gui_vp.cpp:3610-3745: table1d6b43/table1d6b15 admit the
     * viewport row; the 9-byte row pair, timer bits, and directions select
     * only TEMP_PICST fields 8/9/10/12. */
    memset(&vb30_in, 0, sizeof(vb30_in));
    vb30_in.query_48ae_state=3; vb30_in.table_b43=0; vb30_in.table_b15=3;
    ok &= dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30) &&
        vb30.vb30==8 && dm2_v1_g1_flying_item_summary_image_receipt(
            &loader,&selector,&vb30,&summary) && summary.accepted &&
        summary.field==8 && summary.palette_hash != 0u &&
        summary.graphicsset_offset_word == 0x0507u &&
        summary.image_offset_word == 0xfffeu &&
        summary.metadata.query_offset_x == 4 && summary.metadata.query_offset_y == 5 &&
        summary.offset_receipt_hash != 0u;
    ok &= dm2_v1_g1_flying_item_decoded_material_receipt(
        &loader, &selector, &vb30, &geometry, &decoded) && decoded.valid &&
        decoded.no_draw && decoded.category == 0x0du && decoded.index == 0u &&
        decoded.field == 8u && decoded.raw_gfx256_hash != 0u &&
        decoded.raw_gfx256_receipt_hash != 0u && decoded.decoded_pixels_hash != 0u &&
        decoded.palette_hash == summary.palette_hash && decoded.source_offset_x == 4 &&
        decoded.source_offset_y == 5 && decoded.offset_receipt_hash ==
        summary.offset_receipt_hash;
    memset(&timer_receipt, 0, sizeof(timer_receipt));
    timer_receipt.valid = 1; timer_receipt.no_draw = 1;
    timer_receipt.source.valid = 1;
    timer_receipt.source.missile_object_id = selector.missile_object_id;
    timer_receipt.timer_receipt_hash = 0x4a8f02d1u;
    ok &= dm2_v1_runtime_flying_item_decoded_material_plan(
        &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u, &material_plan) && material_plan.valid &&
        material_plan.no_draw && material_plan.decoded_pixels_hash ==
        decoded.decoded_pixels_hash && dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u);
    ok &= !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223345u, 0x55667788u) && !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667789u);
    decoded.palette_hash ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u);
    decoded.palette_hash ^= 1u;
    timer_receipt.timer_receipt_hash ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u);
    timer_receipt.timer_receipt_hash ^= 1u;
    selector.identity_hash ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u);
    selector.identity_hash ^= 1u;
    vb30.identity_hash ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u);
    vb30.identity_hash ^= 1u;
    geometry.identity_hash ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u);
    geometry.identity_hash ^= 1u;
    decoded.raw_gfx256_hash ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_decoded_material_plan_matches(
        &material_plan, &timer_receipt, &selector, &vb30, &geometry, &decoded,
        0x11223344u, 0x55667788u);
    decoded.raw_gfx256_hash ^= 1u;
    ok &= dm2_v1_g1_flying_item_source_receipt(0x3800u, 0x0d, 0, 8, 1,
        3, 6, 0x40, &r);
    memset(&delivery, 0, sizeof(delivery));
    delivery.valid = 1; delivery.no_draw = 1; delivery.m11_delivery_ready = 1;
    delivery.missile_object_id = r.missile_object_id; delivery.category = r.category;
    delivery.item_type = r.item_type; delivery.image_field = r.image_field;
    delivery.flip_flags = r.flip_flags; delivery.cell_pos = r.cell_pos;
    delivery.position_5x5 = r.position_5x5; delivery.clip_rect_id = r.clip_rect_id;
    delivery.stretch_factor64 = r.stretch_factor64;
    delivery.raw_gfx256_hash = decoded.raw_gfx256_hash;
    delivery.raw_gfx256_receipt_hash = decoded.raw_gfx256_receipt_hash;
    delivery.palette_hash = decoded.palette_hash;
    delivery.timer_receipt_hash = material_plan.timer_receipt_hash;
    delivery.viewport_session_identity = material_plan.session_identity;
    delivery.viewport_map_load_token = material_plan.map_load_token;
    delivery.identity_hash = 0x91a2b3c4u;
    ok &= dm2_v1_runtime_consume_flying_item_decoded_material_for_m11(
        &loader, &delivery, &material_plan, &decoded, &r, &consumer) &&
        consumer.valid && consumer.no_draw && consumer.indexed_bytes_consumed &&
        consumer.orientation_unapplied && consumer.follows_static_and_creature &&
        consumer.source_order == 8u && consumer.clip_rect_id == r.clip_rect_id &&
        consumer.flip_flags == r.flip_flags && consumer.width == 4u &&
        consumer.height == 4u && consumer.indexed_pixels_hash ==
        decoded.decoded_pixels_hash && consumer.palette_hash == decoded.palette_hash &&
        consumer.source_offset_x == 4 && consumer.source_offset_y == 5 &&
        consumer.offset_receipt_hash == decoded.offset_receipt_hash;
    memset(&clip, 0, sizeof(clip));
    clip.valid = 1; clip.rect_id = r.clip_rect_id;
    clip.raw4_hash = 0x42u; clip.receipt_hash = 0x43u;
    clip.rect.x = 40; clip.rect.y = 50; clip.rect.w = 60; clip.rect.h = 70;
    ok &= dm2_v1_runtime_flying_item_destination_receipt(
        &consumer, &r, &clip, &destination) && destination.valid &&
        destination.no_draw && destination.clip_rect_id == r.clip_rect_id &&
        destination.clip_rect.x == 40 && destination.clip_rect.h == 70 &&
        destination.source_offset_x == 4 && destination.source_offset_y == 5 &&
        destination.flip_flags == 1u && destination.orientation_unapplied;
    clip.rect_id ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_destination_receipt(
        &consumer, &r, &clip, &destination);
    clip.rect_id ^= 1u; r.flip_flags ^= 1u;
    ok &= !dm2_v1_runtime_flying_item_destination_receipt(
        &consumer, &r, &clip, &destination);
    r.flip_flags ^= 1u; clip.rect.w = 0;
    ok &= !dm2_v1_runtime_flying_item_destination_receipt(
        &consumer, &r, &clip, &destination);
    clip.rect.w = 60;
    r.flip_flags = 1;
    ok &= dm2_v1_runtime_flying_item_destination_receipt(
        &consumer, &r, &clip, &destination);
    r.flip_flags = 0; destination.flip_flags = 0;
    ok &= dm2_v1_runtime_flying_item_picst_transform_receipt(
        &destination, &decoded, &r, &transform) && transform.valid &&
        transform.no_draw && transform.scale_x == 0x40u && transform.scale_y == 0x40u &&
        transform.blitmode == 0u && transform.clip_rect.w == 60 &&
        transform.source_offset_x == 4 && transform.source_offset_y == 5;
    r.flip_flags = 1;
    ok &= !dm2_v1_runtime_flying_item_picst_transform_receipt(
        &destination, &decoded, &r, &transform);
    r.flip_flags = 0; r.stretch_factor64 = 0x41u;
    ok &= !dm2_v1_runtime_flying_item_picst_transform_receipt(
        &destination, &decoded, &r, &transform);
    r.stretch_factor64 = 0x40u; destination.clip_rect.h = 0;
    ok &= !dm2_v1_runtime_flying_item_picst_transform_receipt(
        &destination, &decoded, &r, &transform);
    destination.clip_rect.h = 70;
    /* Exact normal-scale branch: no source offset/crop/flip, a RAW4 clip
     * with IMG3's native dimensions, and a direct indexed copy only. */
    zero_offset_material = decoded;
    zero_offset_material.source_offset_x = 0;
    zero_offset_material.source_offset_y = 0;
    consumer.source_offset_x = 0; consumer.source_offset_y = 0;
    destination.valid = 1; destination.no_draw = 1;
    destination.clip_rect.x = 1; destination.clip_rect.y = 1;
    destination.clip_rect.w = 4; destination.clip_rect.h = 4;
    destination.source_offset_x = 0; destination.source_offset_y = 0;
    destination.flip_flags = 0;
    memset(&transform, 0, sizeof(transform));
    transform.valid = 1; transform.no_draw = 1;
    transform.scale_x = 0x40u; transform.scale_y = 0x40u;
    transform.clip_rect = destination.clip_rect;
    transform.destination_identity_hash = destination.identity_hash;
    transform.material_identity_hash = zero_offset_material.identity_hash;
    memset(framebuffer, 0xee, sizeof(framebuffer));
    ok &= dm2_v1_runtime_blit_flying_item_normal_scale_indexed(
        &loader, &consumer, &destination, &transform, &zero_offset_material,
        framebuffer, 8, 8, 8) && framebuffer[9] == 1u && framebuffer[10] == 0u &&
        framebuffer[17] == 1u && framebuffer[0] == 0xeeu;
    transform.blitmode = 1u;
    ok &= !dm2_v1_runtime_blit_flying_item_normal_scale_indexed(
        &loader, &consumer, &destination, &transform, &zero_offset_material,
        framebuffer, 8, 8, 8);
    r.flip_flags = 4u;
    ok &= !dm2_v1_runtime_consume_flying_item_decoded_material_for_m11(
        &loader, &delivery, &material_plan, &decoded, &r, &consumer);
    r.flip_flags = 1u;
    delivery.clip_rect_id ^= 1u;
    ok &= !dm2_v1_runtime_consume_flying_item_decoded_material_for_m11(
        &loader, &delivery, &material_plan, &decoded, &r, &consumer);
    delivery.clip_rect_id ^= 1u;
    ok &= dm2_v1_runtime_consume_flying_item_decoded_material_for_m11(
        &loader, &delivery, &material_plan, &decoded, &r, &consumer);
    memset(&composition, 0, sizeof(composition));
    composition.valid = 1; composition.no_draw = 1;
    composition.m11_delivery_ready = 1;
    composition.flying_item_identity_hash = delivery.identity_hash;
    composition.identity_hash = 0x1234abcdu;
    ok &= dm2_v1_runtime_build_dm2_viewport_m11_material_composition(
        &composition, &consumer, &material_composition) && material_composition.valid &&
        material_composition.no_draw && dm2_v1_runtime_dm2_viewport_m11_material_composition_matches(
        &material_composition, &composition, &consumer);
    consumer.palette_hash ^= 1u;
    ok &= !dm2_v1_runtime_dm2_viewport_m11_material_composition_matches(
        &material_composition, &composition, &consumer);
    consumer.palette_hash ^= 1u;
    vb30_in.query_48ae_state=0; vb30_in.timer_direction=0;
    vb30_in.viewport_direction=0; vb30_in.direction_5x5=0;
    vb30_in.table_afe=0; vb30_in.table_b43=0;
    ok &= dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30) && vb30.vb30==9;
    vb30_in.table_afe=1;
    ok &= dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30) && vb30.vb30==8;
    vb30_in.table_afe=0;
    vb30_in.query_48ae_state=1;
    ok &= dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30) && vb30.vb30==10;
    vb30_in.timer_direction=1;
    ok &= dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30) && vb30.vb30==12;
    vb30_in.direction_5x5=5;
    ok &= !dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30);
    vb30_in.query_48ae_state=0; vb30_in.direction_5x5=1;
    vb30_in.timer_direction=0; vb30_in.viewport_direction=0;
    ok &= dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30) &&
        vb30.temp_picst_blocked && !dm2_v1_g1_flying_item_summary_image_receipt(
            &loader,&selector,&vb30,&summary);
    vb30_in.direction_5x5=0; vb30_in.table_b43=-1;
    ok &= !dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30);
    vb30_in.table_b43=0; vb30_in.table_b15=-1;
    ok &= !dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30);
    vb30_in.table_b15=3; vb30_in.query_48ae_state=4;
    ok &= !dm2_v1_g1_flying_item_vb30_receipt(&selector,&vb30_in,&vb30);
    entries[0].data_index=0;
    ok &= !dm2_v1_g1_flying_item_selector_receipt(&d,&loader,&missile,&selector);
    ok &= !dm2_v1_g1_direct_missile_receipt(&d,0x1400u,&missile);
    if (!ok) fputs("flying item source receipt failed\n", stderr);
    return ok ? 0 : 1;
}
