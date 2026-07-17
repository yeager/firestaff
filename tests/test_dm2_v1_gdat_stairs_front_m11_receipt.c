#include "dm2_v1_gdat_stairs_front_m11_receipt.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_GdatStairsFrontSourceReceipt source, reject_source;
    DM2_V1_GdatStairsFrontMaterialReceipt material;
    DM2_V1_GdatStairsFrontRaw4Receipt raw4;
    DM2_V1_GdatStairsFrontM11Receipt m11;
    DM2_V1_GdatStairsFrontFallbackReceipt fallback, fallback_reject;
    DM2_V1_GdatStairsFrontMaterialReceipt fallback_material;
    DM2_V1_GdatStairsFrontRaw4Receipt fallback_raw4;
    DM2_V1_GdatStairsFrontM11Receipt fallback_m11;
    DM2_V1_GdatStairsFrontFallbackTempPicstReceipt fallback_temp;
    DM2_V1_Dm2ViewportM11CompositionReceipt composition;
    DM2_V1_ViewportState viewport;
    DM2_V1_GdatEntry entries[2];
    uint32_t offsets[2] = { 0u, 28u }, sizes[2] = { 28u, 16u };
    uint8_t bytes[44] = {
        /* Authentic-shaped IMG3/U4: 2x2, local palette and four indices. */
        2u,0u,2u,0x80u,4u,0u,0u,0u,0u,0u,0x12u,0x34u,
        0x10u,0x11u,0x12u,0x13u,0x14u,0x15u,0x16u,0x17u,
        0x18u,0x19u,0x1au,0x1bu,0x1cu,0x1du,0x1eu,0x1fu,
        /* RAW4 root group, rect 0x336, placement (5,6). */
        0x0du,0xfcu,1u,0u,0x36u,0x03u,0x36u,0x03u,
        1u,0u,0u,0u,5u,0u,6u,0u
    };
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_AssetLoader loader;
    int ok = 1;

    memset(&loader, 0, sizeof(loader)); memset(entries, 0, sizeof(entries));
    entries[0].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET; entries[0].cls2 = 1u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE; entries[0].cls4 = 0x4fu;
    entries[1].cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_RAW4; entries[1].data_index = 1u;
    loader.data = bytes; loader.data_size = sizeof(bytes); loader.loaded = 1;
    loader.raw_data_count = 2u; loader.raw_offsets = offsets; loader.raw_sizes = sizes;
    loader.entries = entries; loader.entry_count = 2u;

    /* Source lock: c_gui_vp.cpp:480-511; dm2data.cpp:289-310. */
    ok &= dm2_v1_gdat_stairs_front_source_receipt(3u, 0u, 1u, 20u, 1, &source) &&
        source.valid && source.no_draw && source.field == 0x4fu &&
        source.rect_number == 0x336u && source.state_variant == 0u;
    ok &= dm2_v1_gdat_stairs_front_source_receipt(3u, 1u, 1u, 20u, 1, &reject_source) &&
        reject_source.field == 0x3bu && reject_source.rect_number == 0x329u;
    ok &= !dm2_v1_gdat_stairs_front_source_receipt(0u, 0u, 1u, 20u, 1, &reject_source) &&
        !dm2_v1_gdat_stairs_front_source_receipt(3u, 0u, 1u, 20u, 0, &reject_source) &&
        !dm2_v1_gdat_stairs_front_source_receipt(3u, 0u, 1u, 641u, 1, &reject_source);
    ok &= dm2_v1_gdat_stairs_front_material_receipt_build(&loader, &source, &material) &&
        material.valid && material.no_draw && material.summary.accepted &&
        material.raw_material.receipt_hash && material.format == DM2_IMG_FMT_U4 &&
        material.width == 2u && material.height == 2u && material.indexed_pixel_count == 4u &&
        material.indexed_pixels && material.decoded_hash && material.palette_hash;
    ok &= dm2_v1_gdat_stairs_front_raw4_receipt_build(&loader, &material, &raw4) &&
        raw4.valid && raw4.no_draw && raw4.rect_number == 0x336u &&
        raw4.destination_x == 5 && raw4.destination_y == 6 && raw4.raw4_table_hash && raw4.raw4_row_hash;
    memset(framebuffer, 0xa5, sizeof(framebuffer)); dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    memset(&composition, 0, sizeof(composition)); composition.valid = 1; composition.no_draw = 1;
    composition.session_identity = 0x1234u; composition.data_epoch = 0x5678u; composition.identity_hash = 0x2468ace1u;
    ok &= dm2_v1_viewport_surface_snapshot(&viewport, &composition.surface_before);
    composition.surface_after = composition.surface_before;
    ok &= dm2_v1_gdat_stairs_front_m11_receipt_build(&material, &raw4, &composition, &viewport, &m11) &&
        m11.valid && m11.no_draw && m11.indexed_pixels == material.indexed_pixels &&
        m11.palette_hash == material.palette_hash && m11.raw4_identity_hash == raw4.identity_hash &&
        dm2_v1_gdat_stairs_front_m11_receipt_matches(&m11, &material, &raw4, &composition, &viewport) &&
        framebuffer[0] == 0xa5 && framebuffer[sizeof(framebuffer) - 1u] == 0xa5;
    ++raw4.identity_hash;
    ok &= !dm2_v1_gdat_stairs_front_m11_receipt_matches(&m11, &material, &raw4, &composition, &viewport);
    --raw4.identity_hash; ++composition.data_epoch;
    ok &= !dm2_v1_gdat_stairs_front_m11_receipt_matches(&m11, &material, &raw4, &composition, &viewport);
    --composition.data_epoch; dm2_v1_viewport_bind_surface(&viewport, framebuffer, DM2_VP_WIDTH);
    ok &= !dm2_v1_gdat_stairs_front_m11_receipt_matches(&m11, &material, &raw4, &composition, &viewport) &&
        framebuffer[0] == 0xa5 && framebuffer[sizeof(framebuffer) - 1u] == 0xa5;

    /* Secondary source branch: c_gui_vp.cpp:514-527. It is selected only
     * by a failed primary IF_LOADABLE query and uses table1d6f7c, not f5c. */
    ok &= dm2_v1_gdat_stairs_front_fallback_receipt(5u, 0u, 1u, 20u, 0, &fallback) &&
        fallback.valid && fallback.no_draw && fallback.field == 0x50u &&
        fallback.rect_number == 0x337u && fallback.blit_mode == 1u &&
        fallback.normal_scale && fallback.palette_transaction_unproven;
    ok &= !dm2_v1_gdat_stairs_front_fallback_receipt(5u, 0u, 1u, 20u, 1, &fallback_reject) &&
        !dm2_v1_gdat_stairs_front_fallback_receipt(0u, 0u, 1u, 20u, 0, &fallback_reject) &&
        !dm2_v1_gdat_stairs_front_fallback_receipt(5u, 0u, 1u, 641u, 0, &fallback_reject);
    entries[0].cls4 = 0x50u;
    bytes[32] = 0x37u; bytes[34] = 0x37u;
    ok &= dm2_v1_gdat_stairs_front_fallback_material_receipt_build(&loader, &fallback,
        &fallback_material) && fallback_material.valid && fallback_material.no_draw &&
        fallback_material.source.field == 0x50u && fallback_material.format == DM2_IMG_FMT_U4 &&
        fallback_material.palette_hash && fallback_material.decoded_hash;
    ok &= dm2_v1_gdat_stairs_front_raw4_receipt_build(&loader, &fallback_material,
        &fallback_raw4) && fallback_raw4.valid && fallback_raw4.rect_number == 0x337u;
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    ok &= dm2_v1_viewport_surface_snapshot(&viewport, &composition.surface_before);
    composition.surface_after = composition.surface_before;
    ok &= dm2_v1_gdat_stairs_front_m11_receipt_build(&fallback_material, &fallback_raw4,
        &composition, &viewport, &fallback_m11) && fallback_m11.valid && fallback_m11.no_draw;
    ok &= dm2_v1_gdat_stairs_front_fallback_temp_picst_receipt_build(&fallback,
        &fallback_material, &fallback_raw4, &fallback_m11, &fallback_temp) &&
        fallback_temp.valid && fallback_temp.no_draw && fallback_temp.blit_mode == 1u &&
        fallback_temp.scale_x == 0x40u && fallback_temp.scale_y == 0x40u &&
        fallback_temp.offset_x == 0 && fallback_temp.offset_y == 0 &&
        fallback_temp.palette_mode == -1 && fallback_temp.palette_arg == -1 &&
        fallback_temp.alpha_mask == 20u && fallback_temp.palette_transaction_unproven &&
        framebuffer[0] == 0xa5 && framebuffer[sizeof(framebuffer) - 1u] == 0xa5;
    ++fallback_material.identity_hash;
    ok &= !dm2_v1_gdat_stairs_front_fallback_temp_picst_receipt_build(&fallback,
        &fallback_material, &fallback_raw4, &fallback_m11, &fallback_temp);
    --fallback_material.identity_hash; ++fallback_raw4.identity_hash;
    ok &= !dm2_v1_gdat_stairs_front_fallback_temp_picst_receipt_build(&fallback,
        &fallback_material, &fallback_raw4, &fallback_m11, &fallback_temp) && framebuffer[0] == 0xa5;
    --fallback_raw4.identity_hash;
    dm2_v1_asset_free_pixels((uint8_t *)fallback_material.indexed_pixels);
    dm2_v1_asset_free_pixels((uint8_t *)material.indexed_pixels);
    printf("%s dm2_v1_gdat_stairs_front_m11_receipt\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
