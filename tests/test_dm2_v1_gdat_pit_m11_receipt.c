#include "dm2_v1_gdat_pit_m11_receipt.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_GdatPitTransformReceipt receipt;
    DM2_V1_GdatPitM11Receipt material;
    DM2_V1_GdatPitRaw4Receipt raw4;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff;
    DM2_V1_GdatPitM11CompositionReceipt composition_receipt;
    DM2_V1_GdatPitM11CompositionReceipt composition_reject;
    DM2_V1_GdatPitPlacementCompositionReceipt placement_receipt;
    DM2_V1_GdatPitPlacementCompositionReceipt placement_reject;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot_receipt;
    DM2_V1_GdatPitNormalRowReceipt normal_rows;
    DM2_V1_GdatPitB073Receipt b073;
    DM2_V1_GdatPitCropProvenanceReceipt crop;
    DM2_V1_GdatPitM11Receipt material3;
    DM2_V1_GdatPitRaw4Receipt raw43;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff3;
    DM2_V1_GdatPitM11CompositionReceipt composition3;
    DM2_V1_GdatPitPlacementCompositionReceipt placement3;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot3;
    DM2_V1_GdatPitNormalRowReceipt normal3;
    DM2_V1_GdatPitB073Receipt b073_3;
    DM2_V1_GdatPitM11Receipt material4;
    DM2_V1_GdatPitRaw4Receipt raw44;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff4;
    DM2_V1_GdatPitM11CompositionReceipt composition4;
    DM2_V1_GdatPitPlacementCompositionReceipt placement4;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot4;
    DM2_V1_GdatPitNormalRowReceipt normal4;
    DM2_V1_GdatPitNormalRowReceipt normal4_reject;
    DM2_V1_GdatPitB073Receipt b073_4;
    DM2_V1_GdatPitB073Receipt b073_4_reject;
    DM2_V1_GdatPitM11Receipt material6;
    DM2_V1_GdatPitRaw4Receipt raw46;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff6;
    DM2_V1_GdatPitM11CompositionReceipt composition6;
    DM2_V1_GdatPitPlacementCompositionReceipt placement6;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot6;
    DM2_V1_GdatPitNormalRowReceipt normal6;
    DM2_V1_GdatPitB073Receipt b073_6;
    DM2_V1_GdatPitM11Receipt material7;
    DM2_V1_GdatPitRaw4Receipt raw47;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff7;
    DM2_V1_GdatPitM11CompositionReceipt composition7;
    DM2_V1_GdatPitPlacementCompositionReceipt placement7;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot7;
    DM2_V1_GdatPitNormalRowReceipt normal7;
    DM2_V1_GdatPitB073Receipt b073_7;
    DM2_V1_GdatPitM11Receipt material11;
    DM2_V1_GdatPitRaw4Receipt raw411;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff11;
    DM2_V1_GdatPitM11CompositionReceipt composition11;
    DM2_V1_GdatPitPlacementCompositionReceipt placement11;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot11;
    DM2_V1_GdatPitNormalRowReceipt normal11;
    DM2_V1_GdatPitB073Receipt b073_11;
    DM2_V1_GdatPitM11Receipt material12;
    DM2_V1_GdatPitRaw4Receipt raw412;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff12;
    DM2_V1_GdatPitM11CompositionReceipt composition12;
    DM2_V1_GdatPitPlacementCompositionReceipt placement12;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot12;
    DM2_V1_GdatPitNormalRowReceipt normal12;
    DM2_V1_GdatPitB073Receipt b073_12;
    DM2_V1_GdatPitM11Receipt material14;
    DM2_V1_GdatPitRaw4Receipt raw414;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff14;
    DM2_V1_GdatPitM11CompositionReceipt composition14;
    DM2_V1_GdatPitPlacementCompositionReceipt placement14;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot14;
    DM2_V1_GdatPitNormalRowReceipt normal14;
    DM2_V1_GdatPitB073Receipt b073_14;
    DM2_V1_GdatPitM11Receipt material2;
    DM2_V1_GdatPitRaw4Receipt raw42;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff2;
    DM2_V1_GdatPitM11CompositionReceipt composition2;
    DM2_V1_GdatPitPlacementCompositionReceipt placement2;
    DM2_V1_GdatPitM11ConsumeSlotReceipt slot2;
    DM2_V1_GdatPitNormalRowReceipt hflip2;
    DM2_V1_GdatPitB073Receipt b073_2;
    DM2_V1_GdatPitM11Receipt material5; DM2_V1_GdatPitRaw4Receipt raw45;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff5; DM2_V1_GdatPitM11CompositionReceipt composition5;
    DM2_V1_GdatPitPlacementCompositionReceipt placement5; DM2_V1_GdatPitM11ConsumeSlotReceipt slot5;
    DM2_V1_GdatPitNormalRowReceipt hflip5; DM2_V1_GdatPitB073Receipt b073_5;
    DM2_V1_GdatPitM11Receipt material8; DM2_V1_GdatPitRaw4Receipt raw48;
    DM2_V1_GdatPitMaterialHandoffReceipt handoff8; DM2_V1_GdatPitM11CompositionReceipt composition8;
    DM2_V1_GdatPitPlacementCompositionReceipt placement8; DM2_V1_GdatPitM11ConsumeSlotReceipt slot8;
    DM2_V1_GdatPitNormalRowReceipt hflip8; DM2_V1_GdatPitB073Receipt b073_8;
    DM2_V1_GdatPitM11Receipt material13; DM2_V1_GdatPitRaw4Receipt raw413; DM2_V1_GdatPitMaterialHandoffReceipt handoff13; DM2_V1_GdatPitM11CompositionReceipt composition13; DM2_V1_GdatPitPlacementCompositionReceipt placement13; DM2_V1_GdatPitM11ConsumeSlotReceipt slot13; DM2_V1_GdatPitNormalRowReceipt hflip13; DM2_V1_GdatPitB073Receipt b073_13;
    DM2_V1_GdatPitM11Receipt material15; DM2_V1_GdatPitRaw4Receipt raw415; DM2_V1_GdatPitMaterialHandoffReceipt handoff15; DM2_V1_GdatPitM11CompositionReceipt composition15; DM2_V1_GdatPitPlacementCompositionReceipt placement15; DM2_V1_GdatPitM11ConsumeSlotReceipt slot15; DM2_V1_GdatPitNormalRowReceipt hflip15; DM2_V1_GdatPitB073Receipt b073_15;
    DM2_V1_Dm2ViewportM11CompositionReceipt composition;
    DM2_V1_ViewportState viewport;
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entries[10];
    uint32_t raw_offsets[3] = { 0u, 28u, 148u };
    uint32_t raw_sizes[3] = { 28u, 120u, 516u };
    uint8_t raw[664] = {
        2u, 0u, 2u, 0x80u, 4u, 0u, 0u, 0u, 0u, 0u,
        0x12u, 0x34u,
        0x10u, 0x11u, 0x12u, 0x13u, 0x14u, 0x15u, 0x16u, 0x17u,
        0x18u, 0x19u, 0x1au, 0x1bu, 0x1cu, 0x1du, 0x1eu, 0x1fu
    };
    uint8_t surface_bytes[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    uint8_t rebound_bytes[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    int ok = 1;
    int i;

    /* Source lock: dm2data.cpp:776-800 and c_gui_vp.cpp:4806-4856. */
    ok &= dm2_v1_gdat_pit_transform_receipt(1u, 0u, 640u, &receipt) &&
        receipt.valid && receipt.no_draw && receipt.field == 0x6cu &&
        receipt.rect_number == 0x35du && receipt.mirror_flip == 0u &&
        !receipt.state_word_nonzero && receipt.identity_hash != 0u;
    ok &= dm2_v1_gdat_pit_transform_receipt(2u, 1u, 10u, &receipt) &&
        receipt.field == 0x83u && receipt.rect_number == 0x35fu &&
        receipt.mirror_flip == 1u && receipt.state_word_nonzero;
    ok &= dm2_v1_gdat_pit_transform_receipt(15u, 0u, 0u, &receipt) &&
        receipt.field == 0x79u && receipt.rect_number == 0x353u &&
        receipt.mirror_flip == 1u;
    ok &= !dm2_v1_gdat_pit_transform_receipt(0u, 0u, 0u, &receipt) &&
        !dm2_v1_gdat_pit_transform_receipt(9u, 0u, 0u, &receipt) &&
        !dm2_v1_gdat_pit_transform_receipt(16u, 0u, 0u, &receipt) &&
        !dm2_v1_gdat_pit_transform_receipt(1u, 0u, 641u, &receipt);
    memset(&material, 0xa5, sizeof(material));
    ok &= !dm2_v1_gdat_pit_m11_receipt_build(NULL, 1u, 1u, 0u, 0u, &material) &&
        material.valid == 0 && material.no_draw == 0;
    /* Isolated authentic GDAT-shaped IMG3/U4 record: its bytes, palette and
     * RAW index are consumed by the receipt, not manufactured by the API. */
    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    memset(&material4, 0, sizeof(material4));
    memset(&raw44, 0, sizeof(raw44));
    memset(&handoff4, 0, sizeof(handoff4));
    memset(&composition4, 0, sizeof(composition4));
    memset(&placement4, 0, sizeof(placement4));
    memset(&slot4, 0, sizeof(slot4));
    memset(&normal4, 0, sizeof(normal4));
    memset(&normal4_reject, 0, sizeof(normal4_reject));
    memset(&b073_4, 0, sizeof(b073_4));
    memset(&b073_4_reject, 0, sizeof(b073_4_reject));
    memset(&material6, 0, sizeof(material6));
    memset(&raw46, 0, sizeof(raw46));
    memset(&handoff6, 0, sizeof(handoff6));
    memset(&composition6, 0, sizeof(composition6));
    memset(&placement6, 0, sizeof(placement6));
    memset(&slot6, 0, sizeof(slot6));
    memset(&normal6, 0, sizeof(normal6));
    memset(&b073_6, 0, sizeof(b073_6));
    memset(&material7, 0, sizeof(material7));
    memset(&raw47, 0, sizeof(raw47));
    memset(&handoff7, 0, sizeof(handoff7));
    memset(&composition7, 0, sizeof(composition7));
    memset(&placement7, 0, sizeof(placement7));
    memset(&slot7, 0, sizeof(slot7));
    memset(&normal7, 0, sizeof(normal7));
    memset(&b073_7, 0, sizeof(b073_7));
    memset(&material11, 0, sizeof(material11));
    memset(&raw411, 0, sizeof(raw411));
    memset(&handoff11, 0, sizeof(handoff11));
    memset(&composition11, 0, sizeof(composition11));
    memset(&placement11, 0, sizeof(placement11));
    memset(&slot11, 0, sizeof(slot11));
    memset(&normal11, 0, sizeof(normal11));
    memset(&b073_11, 0, sizeof(b073_11));
    memset(&material12, 0, sizeof(material12));
    memset(&raw412, 0, sizeof(raw412));
    memset(&handoff12, 0, sizeof(handoff12));
    memset(&composition12, 0, sizeof(composition12));
    memset(&placement12, 0, sizeof(placement12));
    memset(&slot12, 0, sizeof(slot12));
    memset(&normal12, 0, sizeof(normal12));
    memset(&b073_12, 0, sizeof(b073_12));
    memset(&material14, 0, sizeof(material14));
    memset(&raw414, 0, sizeof(raw414));
    memset(&handoff14, 0, sizeof(handoff14));
    memset(&composition14, 0, sizeof(composition14));
    memset(&placement14, 0, sizeof(placement14));
    memset(&slot14, 0, sizeof(slot14));
    memset(&normal14, 0, sizeof(normal14));
    memset(&b073_14, 0, sizeof(b073_14));
    memset(&material2, 0, sizeof(material2)); memset(&raw42, 0, sizeof(raw42));
    memset(&handoff2, 0, sizeof(handoff2)); memset(&composition2, 0, sizeof(composition2));
    memset(&placement2, 0, sizeof(placement2)); memset(&slot2, 0, sizeof(slot2));
    memset(&hflip2, 0, sizeof(hflip2)); memset(&b073_2, 0, sizeof(b073_2));
    memset(&material5, 0, sizeof(material5)); memset(&raw45, 0, sizeof(raw45));
    memset(&handoff5, 0, sizeof(handoff5)); memset(&composition5, 0, sizeof(composition5));
    memset(&placement5, 0, sizeof(placement5)); memset(&slot5, 0, sizeof(slot5));
    memset(&hflip5, 0, sizeof(hflip5)); memset(&b073_5, 0, sizeof(b073_5));
    memset(&material8, 0, sizeof(material8)); memset(&raw48, 0, sizeof(raw48));
    memset(&handoff8, 0, sizeof(handoff8)); memset(&composition8, 0, sizeof(composition8));
    memset(&placement8, 0, sizeof(placement8)); memset(&slot8, 0, sizeof(slot8));
    memset(&hflip8, 0, sizeof(hflip8)); memset(&b073_8, 0, sizeof(b073_8));
    memset(&material13, 0, sizeof(material13)); memset(&raw413, 0, sizeof(raw413)); memset(&handoff13, 0, sizeof(handoff13)); memset(&composition13, 0, sizeof(composition13)); memset(&placement13, 0, sizeof(placement13)); memset(&slot13, 0, sizeof(slot13)); memset(&hflip13, 0, sizeof(hflip13)); memset(&b073_13, 0, sizeof(b073_13));
    memset(&material15,0,sizeof(material15)); memset(&raw415,0,sizeof(raw415)); memset(&handoff15,0,sizeof(handoff15)); memset(&composition15,0,sizeof(composition15)); memset(&placement15,0,sizeof(placement15)); memset(&slot15,0,sizeof(slot15)); memset(&hflip15,0,sizeof(hflip15)); memset(&b073_15,0,sizeof(b073_15));
    entries[0].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[0].cls2 = 1u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[0].cls4 = 0x6cu;
    entries[1].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[1].cls2 = 1u; entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[1].cls4 = 0x6eu;
    entries[2].cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_RAW4; entries[2].data_index = 1u;
    entries[3].cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entries[3].cls3 = DM2_GDAT_ENTRY_TYPE_RAW7; entries[3].cls4 = 2u;
    entries[3].data_index = 2u;
    entries[4].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[4].cls2 = 1u; entries[4].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[4].cls4 = 0x6fu;
    entries[5].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[5].cls2 = 1u; entries[5].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[5].cls4 = 0x71u;
    entries[6].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[6].cls2 = 1u; entries[6].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[6].cls4 = 0x72u;
    entries[7].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[7].cls2 = 1u; entries[7].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[7].cls4 = 0x76u;
    entries[8].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[8].cls2 = 1u; entries[8].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[8].cls4 = 0x77u;
    entries[9].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[9].cls2 = 1u; entries[9].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[9].cls4 = 0x79u;
    loader.data = raw; loader.data_size = sizeof(raw); loader.loaded = 1;
    loader.raw_data_count = 3u; loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes; loader.entries = entries; loader.entry_count = 10u;
    raw[28u] = 0x0du; raw[29u] = 0xfcu; raw[30u] = 1u;
    raw[32u] = 0x52u; raw[33u] = 0x03u;
    raw[34u] = 0x5fu; raw[35u] = 0x03u;
    for (i = 0; i < 14; ++i) {
        size_t offset = 36u + (size_t)i * 8u;
        raw[offset] = 1u;
        raw[offset + 4u] = 5u;
        raw[offset + 6u] = 6u;
    }
    raw[148u] = 1u; raw[149u] = 1u; raw[150u] = 0u; raw[151u] = 21u;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 1u, 0u, 20u, &material) &&
        material.valid && material.no_draw && material.transform.field == 0x6cu &&
        material.summary.accepted && material.raw_material.receipt_hash != 0u &&
        material.width == 2u && material.height == 2u &&
        material.format == DM2_IMG_FMT_U4 && material.decoded_hash != 0u &&
        material.palette_hash == material.summary.palette_hash &&
        material.identity_hash != 0u;
    ok &= dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material, &raw4) &&
        raw4.valid && raw4.no_draw && raw4.rect_number == 0x35du &&
        raw4.destination_x == 5 && raw4.destination_y == 6 &&
        raw4.width == material.width && raw4.height == material.height &&
        raw4.table_hash != 0u && raw4.row_hash != 0u;
    ok &= dm2_v1_gdat_pit_crop_provenance_intake(&material, &raw4, &crop) &&
        crop.valid && crop.no_draw && crop.query1 == 0x35du && crop.identity_hash != 0u;
    memset(&composition, 0, sizeof(composition));
    composition.valid = 1;
    composition.no_draw = 1;
    composition.session_identity = 0x1234u;
    composition.data_epoch = 0x5678u;
    composition.ordered_member_hash = 0x13579bdfu;
    composition.identity_hash = 0x2468ace1u;
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    memset(rebound_bytes, 0x3cu, sizeof(rebound_bytes));
    dm2_v1_viewport_init(&viewport, surface_bytes, DM2_VP_WIDTH);
    ok &= dm2_v1_viewport_surface_snapshot(&viewport, &composition.surface_before);
    composition.surface_after = composition.surface_before;
    ok &= dm2_v1_gdat_pit_m11_composition_receipt_build(&material, &composition,
        &composition_receipt) && composition_receipt.valid &&
        composition_receipt.no_draw && composition_receipt.source_order_unresolved &&
        composition_receipt.pit_material_identity_hash == material.identity_hash &&
        composition_receipt.parent_composition_identity_hash == composition.identity_hash &&
        dm2_v1_gdat_pit_m11_composition_receipt_matches(&composition_receipt,
            &material, &composition);
    ok &= dm2_v1_gdat_pit_placement_composition_receipt_build(&material, &raw4,
        &composition_receipt, &placement_receipt) && placement_receipt.valid &&
        placement_receipt.no_draw && placement_receipt.source_order_unresolved &&
        placement_receipt.pit_raw4_identity_hash == raw4.identity_hash;
    ++composition.data_epoch;
    ok &= !dm2_v1_gdat_pit_m11_composition_receipt_matches(&composition_receipt,
        &material, &composition);
    --composition.data_epoch;
    ++composition.ordered_member_hash;
    ok &= !dm2_v1_gdat_pit_m11_composition_receipt_matches(&composition_receipt,
        &material, &composition);
    --composition.ordered_member_hash;
    ++material.raw_material.receipt_hash;
    ok &= !dm2_v1_gdat_pit_m11_composition_receipt_matches(&composition_receipt,
        &material, &composition);
    --material.raw_material.receipt_hash;
    ++material.transform.identity_hash;
    ok &= !dm2_v1_gdat_pit_m11_composition_receipt_matches(&composition_receipt,
        &material, &composition);
    --material.transform.identity_hash;
    composition.session_identity = 0u;
    ok &= !dm2_v1_gdat_pit_m11_composition_receipt_build(&material, &composition,
        &composition_reject);
    composition.session_identity = 0x1234u;
    ok &= dm2_v1_gdat_pit_material_handoff_receipt_build(&material, &handoff) &&
        handoff.valid && handoff.no_draw && handoff.indexed_pixels == material.indexed_pixels &&
        handoff.indexed_pixel_count == material.indexed_pixel_count;
    i = dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material, &handoff,
        &composition_receipt, &placement_receipt, &composition, &viewport,
        &slot_receipt);
    ok &= i && slot_receipt.valid && slot_receipt.no_draw &&
        slot_receipt.normal_blit_unproven &&
        surface_bytes[0] == 0x5au && surface_bytes[DM2_VP_WIDTH * DM2_VP_HEIGHT - 1] == 0x5au;
    i = dm2_v1_gdat_pit_normal_row_receipt_build(&material, &raw4,
        &placement_receipt, &slot_receipt, &normal_rows);
    ok &= i && normal_rows.valid && normal_rows.no_draw &&
        normal_rows.view_cell == 1u && normal_rows.blit_mode == 0u &&
        normal_rows.source_width == material.width &&
        surface_bytes[0] == 0x5au && surface_bytes[DM2_VP_WIDTH * DM2_VP_HEIGHT - 1] == 0x5au;
    ++raw4.identity_hash;
    ok &= !dm2_v1_gdat_pit_normal_row_receipt_build(&material, &raw4,
        &placement_receipt, &slot_receipt, &normal_rows) && surface_bytes[0] == 0x5au;
    --raw4.identity_hash;
    ok &= dm2_v1_gdat_pit_normal_row_receipt_build(&material, &raw4,
        &placement_receipt, &slot_receipt, &normal_rows);
    ok &= dm2_v1_gdat_pit_b073_receipt_build(&loader, &material, &raw4,
        &placement_receipt, &normal_rows, &b073) && b073.valid && b073.no_draw &&
        b073.colors == 16u && b073.transformed_palette16[0] == 21u &&
        b073.raw7_hash != 0u && b073.transformed_palette_hash != 0u;
    raw[151u] = 22u;
    ok &= dm2_v1_gdat_pit_b073_receipt_build(&loader, &material, &raw4,
        &placement_receipt, &normal_rows, &b073) && b073.transformed_palette16[0] == 22u;
    raw[151u] = 21u;
    ++raw4.identity_hash;
    ok &= !dm2_v1_gdat_pit_b073_receipt_build(&loader, &material, &raw4,
        &placement_receipt, &normal_rows, &b073);
    --raw4.identity_hash;
    ok &= dm2_v1_gdat_pit_b073_receipt_build(&loader, &material, &raw4,
        &placement_receipt, &normal_rows, &b073);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell1_normal(&material, &handoff, &raw4,
        &placement_receipt, &normal_rows, &b073, &slot_receipt, &composition,
        &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff.indexed_pixels[i];
        uint8_t expected = index == (normal_rows.alpha_mask & 0x0fu) ? 0xa5u :
            b073.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++composition.ordered_member_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell1_normal(&material, &handoff, &raw4,
        &placement_receipt, &normal_rows, &b073, &slot_receipt, &composition,
        &viewport) && surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --composition.ordered_member_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 3u, 0u, 20u, &material3) &&
        material3.transform.field == 0x6eu && material3.transform.rect_number == 0x35bu &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material3, &raw43) &&
        raw43.rect_number == 0x35bu &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material3, &handoff3) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material3, &composition, &composition3) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material3, &raw43,
            &composition3, &placement3) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material3, &handoff3,
            &composition3, &placement3, &composition, &viewport, &slot3) &&
        dm2_v1_gdat_pit_normal_row_receipt_build(&material3, &raw43, &placement3,
            &slot3, &normal3) && normal3.view_cell == 3u &&
        dm2_v1_gdat_pit_b073_receipt_build(&loader, &material3, &raw43,
            &placement3, &normal3, &b073_3);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell1_normal(&material3, &handoff3, &raw43,
        &placement3, &normal3, &b073_3, &slot3, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff3.indexed_pixels[i];
        uint8_t expected = index == (normal3.alpha_mask & 0x0fu) ? 0xa5u :
            b073_3.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++raw43.identity_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell1_normal(&material3, &handoff3, &raw43,
        &placement3, &normal3, &b073_3, &slot3, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --raw43.identity_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 4u, 0u, 20u, &material4) &&
        material4.transform.field == 0x6fu && material4.transform.rect_number == 0x35au &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material4, &raw44) &&
        raw44.rect_number == 0x35au && raw44.material_identity_hash == material4.identity_hash &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material4, &handoff4) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material4, &composition, &composition4) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material4, &raw44,
            &composition4, &placement4) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material4, &handoff4,
            &composition4, &placement4, &composition, &viewport, &slot4) &&
        dm2_v1_gdat_pit_cell4_normal_row_receipt_build(&material4, &raw44, &placement4,
            &slot4, &normal4) && normal4.view_cell == 4u &&
        dm2_v1_gdat_pit_cell4_b073_receipt_build(&loader, &material4, &raw44,
            &placement4, &normal4, &b073_4);
    ok &= !dm2_v1_gdat_pit_cell4_normal_row_receipt_build(&material3, &raw43,
        &placement3, &slot3, &normal4_reject) &&
        !dm2_v1_gdat_pit_cell4_b073_receipt_build(&loader, &material3, &raw43,
            &placement3, &normal3, &b073_4_reject);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell4_normal(&material4, &handoff4, &raw44,
        &placement4, &normal4, &b073_4, &slot4, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff4.indexed_pixels[i];
        uint8_t expected = index == (normal4.alpha_mask & 0x0fu) ? 0xa5u :
            b073_4.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++b073_4.raw7_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell4_normal(&material4, &handoff4, &raw44,
        &placement4, &normal4, &b073_4, &slot4, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --b073_4.raw7_hash;
    ++raw44.identity_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell4_normal(&material4, &handoff4, &raw44,
        &placement4, &normal4, &b073_4, &slot4, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --raw44.identity_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 6u, 0u, 20u, &material6) &&
        material6.transform.field == 0x71u && material6.transform.rect_number == 0x358u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material6, &raw46) &&
        raw46.rect_number == 0x358u && raw46.material_identity_hash == material6.identity_hash &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material6, &handoff6) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material6, &composition, &composition6) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material6, &raw46,
            &composition6, &placement6) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material6, &handoff6,
            &composition6, &placement6, &composition, &viewport, &slot6) &&
        dm2_v1_gdat_pit_cell6_normal_row_receipt_build(&material6, &raw46, &placement6,
            &slot6, &normal6) && normal6.view_cell == 6u &&
        dm2_v1_gdat_pit_cell6_b073_receipt_build(&loader, &material6, &raw46,
            &placement6, &normal6, &b073_6);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell6_normal(&material6, &handoff6, &raw46,
        &placement6, &normal6, &b073_6, &slot6, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff6.indexed_pixels[i];
        uint8_t expected = index == (normal6.alpha_mask & 0x0fu) ? 0xa5u :
            b073_6.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++material6.identity_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell6_normal(&material6, &handoff6, &raw46,
        &placement6, &normal6, &b073_6, &slot6, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --material6.identity_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 7u, 0u, 20u, &material7) &&
        material7.transform.field == 0x72u && material7.transform.rect_number == 0x357u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material7, &raw47) &&
        raw47.rect_number == 0x357u && raw47.material_identity_hash == material7.identity_hash &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material7, &handoff7) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material7, &composition, &composition7) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material7, &raw47,
            &composition7, &placement7) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material7, &handoff7,
            &composition7, &placement7, &composition, &viewport, &slot7) &&
        dm2_v1_gdat_pit_cell7_normal_row_receipt_build(&material7, &raw47, &placement7,
            &slot7, &normal7) && normal7.view_cell == 7u &&
        dm2_v1_gdat_pit_cell7_b073_receipt_build(&loader, &material7, &raw47,
            &placement7, &normal7, &b073_7);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell7_normal(&material7, &handoff7, &raw47,
        &placement7, &normal7, &b073_7, &slot7, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff7.indexed_pixels[i];
        uint8_t expected = index == (normal7.alpha_mask & 0x0fu) ? 0xa5u :
            b073_7.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++b073_7.transformed_palette_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell7_normal(&material7, &handoff7, &raw47,
        &placement7, &normal7, &b073_7, &slot7, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --b073_7.transformed_palette_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 11u, 0u, 20u, &material11) &&
        material11.transform.field == 0x76u && material11.transform.rect_number == 0x355u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material11, &raw411) &&
        raw411.rect_number == 0x355u && raw411.material_identity_hash == material11.identity_hash &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material11, &handoff11) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material11, &composition, &composition11) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material11, &raw411,
            &composition11, &placement11) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material11, &handoff11,
            &composition11, &placement11, &composition, &viewport, &slot11) &&
        dm2_v1_gdat_pit_cell11_normal_row_receipt_build(&material11, &raw411, &placement11,
            &slot11, &normal11) && normal11.view_cell == 11u &&
        dm2_v1_gdat_pit_cell11_b073_receipt_build(&loader, &material11, &raw411,
            &placement11, &normal11, &b073_11);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell11_normal(&material11, &handoff11, &raw411,
        &placement11, &normal11, &b073_11, &slot11, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff11.indexed_pixels[i];
        uint8_t expected = index == (normal11.alpha_mask & 0x0fu) ? 0xa5u :
            b073_11.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++raw411.identity_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell11_normal(&material11, &handoff11, &raw411,
        &placement11, &normal11, &b073_11, &slot11, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --raw411.identity_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 12u, 0u, 20u, &material12) &&
        material12.transform.field == 0x77u && material12.transform.rect_number == 0x354u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material12, &raw412) &&
        raw412.rect_number == 0x354u && raw412.material_identity_hash == material12.identity_hash &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material12, &handoff12) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material12, &composition, &composition12) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material12, &raw412,
            &composition12, &placement12) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material12, &handoff12,
            &composition12, &placement12, &composition, &viewport, &slot12) &&
        dm2_v1_gdat_pit_cell12_normal_row_receipt_build(&material12, &raw412, &placement12,
            &slot12, &normal12) && normal12.view_cell == 12u &&
        dm2_v1_gdat_pit_cell12_b073_receipt_build(&loader, &material12, &raw412,
            &placement12, &normal12, &b073_12);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell12_normal(&material12, &handoff12, &raw412,
        &placement12, &normal12, &b073_12, &slot12, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff12.indexed_pixels[i];
        uint8_t expected = index == (normal12.alpha_mask & 0x0fu) ? 0xa5u :
            b073_12.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++b073_12.raw7_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell12_normal(&material12, &handoff12, &raw412,
        &placement12, &normal12, &b073_12, &slot12, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --b073_12.raw7_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 14u, 0u, 20u, &material14) &&
        material14.transform.field == 0x79u && material14.transform.rect_number == 0x352u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material14, &raw414) &&
        raw414.rect_number == 0x352u && raw414.material_identity_hash == material14.identity_hash &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material14, &handoff14) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material14, &composition, &composition14) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material14, &raw414,
            &composition14, &placement14) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material14, &handoff14,
            &composition14, &placement14, &composition, &viewport, &slot14) &&
        dm2_v1_gdat_pit_cell14_normal_row_receipt_build(&material14, &raw414, &placement14,
            &slot14, &normal14) && normal14.view_cell == 14u &&
        dm2_v1_gdat_pit_cell14_b073_receipt_build(&loader, &material14, &raw414,
            &placement14, &normal14, &b073_14);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell14_normal(&material14, &handoff14, &raw414,
        &placement14, &normal14, &b073_14, &slot14, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff14.indexed_pixels[i];
        uint8_t expected = index == (normal14.alpha_mask & 0x0fu) ? 0xa5u :
            b073_14.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++material14.palette_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell14_normal(&material14, &handoff14, &raw414,
        &placement14, &normal14, &b073_14, &slot14, &composition, &viewport) &&
        surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --material14.palette_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 2u, 0u, 20u, &material2) &&
        material2.transform.field == 0x6cu && material2.transform.rect_number == 0x35fu &&
        material2.transform.mirror_flip == 1u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material2, &raw42) &&
        raw42.rect_number == 0x35fu && dm2_v1_gdat_pit_material_handoff_receipt_build(&material2, &handoff2) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material2, &composition, &composition2) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material2, &raw42, &composition2, &placement2) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material2, &handoff2, &composition2,
            &placement2, &composition, &viewport, &slot2) &&
        dm2_v1_gdat_pit_cell2_hflip_row_receipt_build(&material2, &raw42, &placement2, &slot2, &hflip2) &&
        hflip2.blit_mode == 1u && dm2_v1_gdat_pit_cell2_hflip_b073_receipt_build(&loader,
            &material2, &raw42, &placement2, &hflip2, &b073_2);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell2_hflip(&material2, &handoff2, &raw42, &placement2,
        &hflip2, &b073_2, &slot2, &composition, &viewport);
    for (i = 0; i < 4; ++i) {
        uint8_t index = handoff2.indexed_pixels[(size_t)(i / 2) * handoff2.pixel_stride + 1u - i % 2];
        uint8_t expected = index == (hflip2.alpha_mask & 0x0fu) ? 0xa5u : b073_2.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    ++b073_2.raw7_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell2_hflip(&material2, &handoff2, &raw42, &placement2,
        &hflip2, &b073_2, &slot2, &composition, &viewport) && surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --b073_2.raw7_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 5u, 0u, 20u, &material5) &&
        material5.transform.field == 0x6fu && material5.transform.rect_number == 0x35cu && material5.transform.mirror_flip == 1u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material5, &raw45) && raw45.rect_number == 0x35cu &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material5, &handoff5) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material5, &composition, &composition5) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material5, &raw45, &composition5, &placement5) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material5, &handoff5, &composition5, &placement5, &composition, &viewport, &slot5) &&
        dm2_v1_gdat_pit_cell5_hflip_row_receipt_build(&material5, &raw45, &placement5, &slot5, &hflip5) && hflip5.blit_mode == 1u &&
        dm2_v1_gdat_pit_cell5_hflip_b073_receipt_build(&loader, &material5, &raw45, &placement5, &hflip5, &b073_5);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell5_hflip(&material5, &handoff5, &raw45, &placement5, &hflip5, &b073_5, &slot5, &composition, &viewport);
    for (i = 0; i < 4; ++i) { uint8_t index = handoff5.indexed_pixels[(size_t)(i / 2) * handoff5.pixel_stride + 1u - i % 2]; uint8_t expected = index == (hflip5.alpha_mask & 0x0fu) ? 0xa5u : b073_5.transformed_palette16[index]; ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected; }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes)); ++raw45.identity_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell5_hflip(&material5, &handoff5, &raw45, &placement5, &hflip5, &b073_5, &slot5, &composition, &viewport) && surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --raw45.identity_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 8u, 0u, 20u, &material8) &&
        material8.transform.field == 0x72u && material8.transform.rect_number == 0x359u && material8.transform.mirror_flip == 1u &&
        dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material8, &raw48) && raw48.rect_number == 0x359u &&
        dm2_v1_gdat_pit_material_handoff_receipt_build(&material8, &handoff8) &&
        dm2_v1_gdat_pit_m11_composition_receipt_build(&material8, &composition, &composition8) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material8, &raw48, &composition8, &placement8) &&
        dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material8, &handoff8, &composition8, &placement8, &composition, &viewport, &slot8) &&
        dm2_v1_gdat_pit_cell8_hflip_row_receipt_build(&material8, &raw48, &placement8, &slot8, &hflip8) &&
        dm2_v1_gdat_pit_cell8_hflip_b073_receipt_build(&loader, &material8, &raw48, &placement8, &hflip8, &b073_8);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_consume_cell8_hflip(&material8, &handoff8, &raw48, &placement8, &hflip8, &b073_8, &slot8, &composition, &viewport);
    for (i = 0; i < 4; ++i) { uint8_t index = handoff8.indexed_pixels[(size_t)(i / 2) * handoff8.pixel_stride + 1u - i % 2]; uint8_t expected = index == (hflip8.alpha_mask & 0x0fu) ? 0xa5u : b073_8.transformed_palette16[index]; ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected; }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes)); ++b073_8.raw7_hash;
    ok &= !dm2_v1_gdat_pit_consume_cell8_hflip(&material8, &handoff8, &raw48, &placement8, &hflip8, &b073_8, &slot8, &composition, &viewport) && surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au;
    --b073_8.raw7_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 13u, 0u, 20u, &material13) && material13.transform.field == 0x77u && material13.transform.rect_number == 0x356u && material13.transform.mirror_flip == 1u && dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material13, &raw413) && raw413.rect_number == 0x356u && dm2_v1_gdat_pit_material_handoff_receipt_build(&material13, &handoff13) && dm2_v1_gdat_pit_m11_composition_receipt_build(&material13, &composition, &composition13) && dm2_v1_gdat_pit_placement_composition_receipt_build(&material13, &raw413, &composition13, &placement13) && dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material13, &handoff13, &composition13, &placement13, &composition, &viewport, &slot13) && dm2_v1_gdat_pit_cell13_hflip_row_receipt_build(&material13, &raw413, &placement13, &slot13, &hflip13) && dm2_v1_gdat_pit_cell13_hflip_b073_receipt_build(&loader, &material13, &raw413, &placement13, &hflip13, &b073_13);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes)); ok &= dm2_v1_gdat_pit_consume_cell13_hflip(&material13, &handoff13, &raw413, &placement13, &hflip13, &b073_13, &slot13, &composition, &viewport);
    for (i = 0; i < 4; ++i) { uint8_t index = handoff13.indexed_pixels[(size_t)(i / 2) * handoff13.pixel_stride + 1u - i % 2]; uint8_t expected = index == (hflip13.alpha_mask & 0x0fu) ? 0xa5u : b073_13.transformed_palette16[index]; ok &= surface_bytes[(size_t)(6u + i / 2) * DM2_VP_WIDTH + 5u + i % 2] == expected; }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes)); ++b073_13.transformed_palette_hash; ok &= !dm2_v1_gdat_pit_consume_cell13_hflip(&material13, &handoff13, &raw413, &placement13, &hflip13, &b073_13, &slot13, &composition, &viewport) && surface_bytes[6u * DM2_VP_WIDTH + 5u] == 0x5au; --b073_13.transformed_palette_hash;
    ok &= dm2_v1_gdat_pit_m11_receipt_build(&loader,1u,15u,0u,20u,&material15) && material15.transform.field==0x79u && material15.transform.rect_number==0x353u && material15.transform.mirror_flip==1u && dm2_v1_gdat_pit_raw4_receipt_build(&loader,&material15,&raw415) && dm2_v1_gdat_pit_material_handoff_receipt_build(&material15,&handoff15) && dm2_v1_gdat_pit_m11_composition_receipt_build(&material15,&composition,&composition15) && dm2_v1_gdat_pit_placement_composition_receipt_build(&material15,&raw415,&composition15,&placement15) && dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material15,&handoff15,&composition15,&placement15,&composition,&viewport,&slot15) && dm2_v1_gdat_pit_cell15_hflip_row_receipt_build(&material15,&raw415,&placement15,&slot15,&hflip15) && dm2_v1_gdat_pit_cell15_hflip_b073_receipt_build(&loader,&material15,&raw415,&placement15,&hflip15,&b073_15);
    memset(surface_bytes,0xa5,sizeof(surface_bytes)); ok &= dm2_v1_gdat_pit_consume_cell15_hflip(&material15,&handoff15,&raw415,&placement15,&hflip15,&b073_15,&slot15,&composition,&viewport);
    for(i=0;i<4;++i){uint8_t z=handoff15.indexed_pixels[(size_t)(i/2)*handoff15.pixel_stride+1u-i%2];uint8_t e=z==(hflip15.alpha_mask&15u)?0xa5u:b073_15.transformed_palette16[z];ok&=surface_bytes[(size_t)(6u+i/2)*DM2_VP_WIDTH+5u+i%2]==e;}
    memset(surface_bytes,0x5a,sizeof(surface_bytes)); ++raw415.identity_hash; ok &= !dm2_v1_gdat_pit_consume_cell15_hflip(&material15,&handoff15,&raw415,&placement15,&hflip15,&b073_15,&slot15,&composition,&viewport) && surface_bytes[6u*DM2_VP_WIDTH+5u]==0x5au; --raw415.identity_hash;
    material.indexed_pixels = surface_bytes;
    ok &= !dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material, &handoff,
        &composition_receipt, &placement_receipt, &composition, &viewport,
        &slot_receipt) && surface_bytes[0] == 0x5au;
    material.indexed_pixels = handoff.indexed_pixels;
    ok &= dm2_v1_viewport_bind_surface(&viewport, rebound_bytes, DM2_VP_WIDTH) &&
        !dm2_v1_gdat_pit_m11_consume_slot_receipt_build(&material, &handoff,
            &composition_receipt, &placement_receipt, &composition, &viewport,
            &slot_receipt) && rebound_bytes[0] == 0x3cu;
    raw[36u + 11u * 8u + 4u] = 7u;
    ok &= dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material, &raw4) &&
        dm2_v1_gdat_pit_placement_composition_receipt_build(&material, &raw4,
            &composition_receipt, &placement_reject) &&
        !dm2_v1_gdat_pit_placement_composition_receipt_matches(&placement_receipt,
            &material, &raw4, &composition_receipt);
    raw[36u + 11u * 8u + 4u] = 5u;
    raw_sizes[1] = 7u;
    ok &= !dm2_v1_gdat_pit_raw4_receipt_build(&loader, &material, &raw4);
    raw_sizes[1] = 120u;
    raw_sizes[0] = 12u;
    ok &= !dm2_v1_gdat_pit_m11_receipt_build(&loader, 1u, 1u, 0u, 20u, &material);
    if (!ok) fputs("DM2 pit M11 receipt failed\n", stderr);
    return ok ? 0 : 1;
}
