#include "dm2_v1_gdat_stairs_side_m11_receipt.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_GdatStairsSideSourceReceipt source;
    DM2_V1_GdatStairsSideMaterialReceipt material, shifted;
    DM2_V1_GdatStairsSideRaw4Receipt raw4, raw4_shifted;
    DM2_V1_GdatStairsSideM11Receipt m11, m11_shifted;
    DM2_V1_GdatStairsSideDungeonGraphicReceipt transform;
    DM2_V1_Dm2ViewportM11CompositionReceipt composition;
    DM2_V1_ViewportState viewport;
    DM2_V1_GdatEntry entries[2];
    uint32_t offsets[2] = {0, 28}, sizes[2] = {28, 16};
    uint8_t raw[44] = {2,0,2,0x80,4,0,0,0,0,0,0x12,0x34,0x10,0x11,
                       0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
                       0x1c,0x1d,0x1e,0x1f,0x0d,0xfc,1,0,0x40,3,0x40,3,
                       1,0,0,0,5,0,6,0};
    uint8_t framebuffer[DM2_VP_WIDTH * DM2_VP_HEIGHT];
    DM2_V1_AssetLoader loader;
    int ok = 1;

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    entries[0].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET;
    entries[0].cls2 = 1;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE;
    entries[0].cls4 = 0xcd;
    entries[1].cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_RAW4;
    entries[1].data_index = 1;
    loader.data = raw;
    loader.data_size = sizeof(raw);
    loader.loaded = 1;
    loader.raw_data_count = 2;
    loader.raw_offsets = offsets;
    loader.raw_sizes = sizes;
    loader.entries = entries;
    loader.entry_count = 2;

    /* c_gui_vp.cpp:540-565 -> c_image.cpp:450-475. */
    ok &= dm2_v1_gdat_stairs_side_source_receipt(1, 0, 1, 20, &source);
    ok &= dm2_v1_gdat_stairs_side_material_receipt_build(&loader, &source, &material);
    ok &= dm2_v1_gdat_stairs_side_raw4_receipt_build(&loader, &material, &raw4);
    memset(framebuffer, 0xa5, sizeof(framebuffer));
    dm2_v1_viewport_init(&viewport, framebuffer, DM2_VP_WIDTH);
    memset(&composition, 0, sizeof(composition));
    composition.valid = composition.no_draw = 1;
    composition.session_identity = 0x1234;
    composition.data_epoch = 0x5678;
    composition.identity_hash = 0x2468ace1;
    ok &= dm2_v1_viewport_surface_snapshot(&viewport, &composition.surface_before);
    composition.surface_after = composition.surface_before;
    ok &= dm2_v1_gdat_stairs_side_m11_receipt_build(&material, &raw4, &composition,
                                                     &viewport, &m11);
    ok &= dm2_v1_gdat_stairs_side_dungeon_graphic_receipt_build(&material, &raw4, &m11,
                                                                  &transform);
    ok &= transform.valid && transform.no_draw && transform.blit_mode == 0;
    ok &= transform.normal_scale && transform.source_offset_is_zero;
    ok &= transform.palette_transaction_unproven && transform.alpha_mask == 20;
    ok &= framebuffer[0] == 0xa5;

    ++material.identity_hash;
    ok &= !dm2_v1_gdat_stairs_side_dungeon_graphic_receipt_build(&material, &raw4, &m11,
                                                                   &transform);
    --material.identity_hash;
    ++raw4.identity_hash;
    ok &= !dm2_v1_gdat_stairs_side_dungeon_graphic_receipt_build(&material, &raw4, &m11,
                                                                   &transform);
    --raw4.identity_hash;
    shifted = material;
    raw4_shifted = raw4;
    m11_shifted = m11;
    shifted.source.rect_number = 0x2bc;
    raw4_shifted.rect_number = 0x2bc;
    ok &= !dm2_v1_gdat_stairs_side_dungeon_graphic_receipt_build(&shifted, &raw4_shifted,
                                                                   &m11_shifted, &transform);
    ok &= framebuffer[0] == 0xa5;
    dm2_v1_asset_free_pixels((uint8_t *)material.indexed_pixels);
    printf("%s dm2_v1_gdat_stairs_side_transform_receipt\n", ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
