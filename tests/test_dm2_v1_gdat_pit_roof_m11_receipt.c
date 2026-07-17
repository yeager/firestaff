#include "dm2_v1_gdat_pit_roof_m11_receipt.h"

#include <stdio.h>
#include <string.h>

int main(void)
{
    DM2_V1_GdatPitRoofSourceState state = { 1u, 1u, 2u, 1u, 0x11223344u };
    DM2_V1_GdatPitRoofTransformReceipt transform;
    DM2_V1_GdatPitRoofM11Receipt material;
    DM2_V1_GdatPitRoofB073Receipt b073;
    DM2_V1_GdatPitRoofRaw4Receipt raw4;
    DM2_V1_GdatPitRoofAlphaBlendReceipt blend;
    DM2_V1_GdatPitRoofB073TablesReceipt b073_tables;
    DM2_V1_GdatPitRoofB073TablesReceipt b073_tables_drift;
    DM2_V1_GdatPitRoofB073TraversalReceipt traversal;
    DM2_V1_GdatPitRoofB073TraversalReceipt traversal_reject;
    DM2_V1_GdatPitRoofDestinationReceipt destination;
    DM2_V1_GdatPitRoofDestinationReceipt destination_reject;
    DM2_V1_GdatPitRoofSurfaceClipReceipt surface_clip;
    DM2_V1_GdatPitRoofSurfaceBindingReceipt surface_binding;
    DM2_V1_GdatPitRoofMaterialHandoffReceipt handoff;
    DM2_V1_ViewportSurfaceSnapshot snapshot;
    DM2_V1_Dm2ViewportM11CompositionReceipt composition;
    uint8_t surface_bytes[320u * 200u];
    DM2_V1_GdatPitRoofM11Receipt mirror_material;
    DM2_V1_GdatPitRoofMaterialHandoffReceipt mirror_handoff;
    DM2_V1_GdatPitRoofB073Receipt mirror_b073;
    DM2_V1_GdatPitRoofRaw4Receipt mirror_raw4;
    DM2_V1_GdatPitRoofB073TablesReceipt mirror_tables;
    DM2_V1_GdatPitRoofB073TraversalReceipt mirror_traversal;
    DM2_V1_GdatPitRoofDestinationReceipt mirror_destination;
    DM2_V1_GdatPitRoofSurfaceClipReceipt mirror_surface_clip;
    DM2_V1_GdatPitRoofSurfaceBindingReceipt mirror_surface_binding;
    DM2_V1_CLightM11Receipt light;
    DM2_V1_AssetLoader loader;
    DM2_V1_GdatEntry entries[3];
    uint32_t raw_offsets[3] = { 0u, 28u, 108u };
    uint32_t raw_sizes[3] = { 28u, 80u, 520u };
    uint8_t raw[628] = {
        2u, 0u, 2u, 0x80u, 4u, 0u, 0u, 0u, 0u, 0u,
        0x12u, 0x34u,
        0x10u, 0x11u, 0x12u, 0x13u, 0x14u, 0x15u, 0x16u, 0x17u,
        0x18u, 0x19u, 0x1au, 0x1bu, 0x1cu, 0x1du, 0x1eu, 0x1fu,
        0x0du, 0xfcu, 1u, 0u, 0x60u, 0x03u, 0x68u, 0x03u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 0u, 0u, 0u, 5u, 0u, 6u, 0u,
        1u, 2u, 0x10u, 0x11u, 0x12u, 0x13u, 0x14u, 0x15u
    };
    int ok;
    int i;
    const uint8_t *original_pixels;
    uint16_t original_width;
    uint16_t original_height;
    uint16_t original_stride;
    uint32_t original_pixel_count;
    uint32_t original_palette_hash;
    uint32_t original_material_identity_hash;
    uint8_t original_first_index;

    /* Source lock: c_gui_vp.cpp:118-206, dm2data.cpp:814-816. */
    ok = dm2_v1_gdat_pit_roof_transform_receipt(1u, 1u, 20u, &state,
        &transform) && transform.valid && transform.no_draw &&
        transform.field == 0x9au && transform.rect_number == 0x366u &&
        transform.mirror_flip == 0u && transform.identity_hash != 0u;
    ok &= dm2_v1_gdat_pit_roof_transform_receipt(1u, 2u, 20u, &state,
        &transform) && transform.field == 0x9au && transform.rect_number == 0x368u &&
        transform.mirror_flip == 1u;
    state.remote_tile_bit_08 = 0u;
    ok &= !dm2_v1_gdat_pit_roof_transform_receipt(1u, 1u, 20u, &state, &transform);
    state.remote_tile_bit_08 = 1u;
    ok &= !dm2_v1_gdat_pit_roof_transform_receipt(1u, 0u, 20u, &state, &transform) &&
        !dm2_v1_gdat_pit_roof_transform_receipt(1u, 9u, 20u, &state, &transform) &&
        !dm2_v1_gdat_pit_roof_transform_receipt(1u, 1u, 641u, &state, &transform);

    memset(&loader, 0, sizeof(loader));
    memset(entries, 0, sizeof(entries));
    entries[0].cls1 = DM2_GDAT_CATEGORY_GRAPHICSSET; entries[0].cls2 = 1u;
    entries[0].cls3 = DM2_GDAT_ENTRY_TYPE_IMAGE; entries[0].cls4 = 0x9au;
    entries[1].cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entries[1].cls3 = DM2_GDAT_ENTRY_TYPE_RAW4;
    entries[1].data_index = 1u;
    entries[2].cls1 = DM2_GDAT_CATEGORY_INTERFACE_GENERAL;
    entries[2].cls3 = DM2_GDAT_ENTRY_TYPE_RAW7; entries[2].cls4 = 2u;
    entries[2].data_index = 2u;
    loader.data = raw; loader.data_size = sizeof(raw); loader.loaded = 1;
    loader.raw_data_count = 3u; loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes; loader.entries = entries; loader.entry_count = 3u;
    raw[112u] = 20u; raw[113u] = 21u;
    for (i = 0; i < 16; ++i) {
        raw[114u + (size_t)(0x10u + (uint8_t)i) * 2u] = 0u;
        raw[114u + (size_t)(0x10u + (uint8_t)i) * 2u + 1u] = 0u;
    }
    ok &= dm2_v1_gdat_pit_roof_m11_receipt_build(&loader, 1u, 1u, 20u,
        &state, &material) && material.valid && material.no_draw &&
        material.transform.field == 0x9au && material.summary.accepted &&
        material.raw_material.receipt_hash != 0u && material.width == 2u &&
        material.height == 2u && material.format == DM2_IMG_FMT_U4 &&
        material.indexed_pixels != NULL && material.indexed_pixel_count == 4u &&
        material.pixel_stride == 2u &&
        material.palette_hash == material.summary.palette_hash &&
        material.identity_hash != 0u;
    ok &= dm2_v1_gdat_pit_roof_material_handoff_receipt_build(&material, &handoff) &&
        handoff.valid && handoff.no_draw && handoff.indexed_pixels == material.indexed_pixels &&
        handoff.indexed_pixel_count == material.indexed_pixel_count &&
        handoff.palette_hash == material.palette_hash &&
        handoff.material_identity_hash == material.identity_hash;
    original_pixels = material.indexed_pixels;
    original_width = material.width;
    original_height = material.height;
    original_stride = material.pixel_stride;
    original_pixel_count = material.indexed_pixel_count;
    original_palette_hash = material.palette_hash;
    original_material_identity_hash = material.identity_hash;
    memset(&light, 0, sizeof(light));
    light.valid = 1; light.light_level = 20u; light.receipt_hash = 0x55667788u;
    ok &= dm2_v1_gdat_pit_roof_b073_receipt_build(&material, &light, &b073) &&
        b073.valid && b073.no_draw && b073.light_level == 20u &&
        b073.alphamask == 20u && b073.local_palette_hash == material.palette_hash;
    /* c_gdatfile.cpp:1919-2003 builds B073's tables from dt07/2; this is
     * source material admission, not a palette conversion or blit. */
    ok &= dm2_v1_gdat_pit_roof_b073_tables_receipt_build(&loader, &material,
        &b073, &b073_tables) && b073_tables.valid && b073_tables.no_draw &&
        b073_tables.table_count == 1u && b073_tables.table_data_bytes == 2u &&
        b073_tables.color_lookup_bytes == 514u && b073_tables.raw7_hash != 0u;
    loader.entry_count = 2u;
    ok &= !dm2_v1_gdat_pit_roof_b073_tables_receipt_build(&loader, &material,
        &b073, &b073_tables_drift);
    loader.entry_count = 3u;
    raw_sizes[2] = 7u;
    ok &= !dm2_v1_gdat_pit_roof_b073_tables_receipt_build(&loader, &material,
        &b073, &b073_tables_drift);
    raw_sizes[2] = 520u;
    raw[108u + 2u] ^= 0x01u;
    ok &= dm2_v1_gdat_pit_roof_b073_tables_receipt_build(&loader, &material,
        &b073, &b073_tables_drift) &&
        b073_tables_drift.identity_hash != b073_tables.identity_hash;
    raw[108u + 2u] ^= 0x01u;
    i = dm2_v1_gdat_pit_roof_b073_traversal_receipt_build(&loader, &material,
        &b073, &b073_tables, &traversal);
    ok &= i && traversal.valid && traversal.no_draw &&
        traversal.colors == 16u && traversal.transformed_palette16[0] == 21u &&
        traversal.identity_hash != 0u;
    raw[114u + 0x10u * 2u + 1u] = 2u;
    i = dm2_v1_gdat_pit_roof_b073_traversal_receipt_build(&loader, &material,
        &b073, &b073_tables, &traversal_reject);
    ok &= !i;
    raw[114u + 0x10u * 2u + 1u] = 0u;
    ++b073.alphamask;
    i = dm2_v1_gdat_pit_roof_b073_traversal_receipt_build(&loader, &material,
        &b073, &b073_tables, &traversal_reject);
    ok &= !i;
    --b073.alphamask;
    ++light.light_level;
    ok &= dm2_v1_gdat_pit_roof_b073_receipt_build(&material, &light, &b073) &&
        b073.light_level == 21u;
    light.light_level = 65u;
    ok &= !dm2_v1_gdat_pit_roof_b073_receipt_build(&material, &light, &b073);
    light.light_level = 20u;
    ok &= dm2_v1_gdat_pit_roof_b073_receipt_build(&material, &light, &b073);
    ok &= dm2_v1_gdat_pit_roof_raw4_receipt_build(&loader, &material, &raw4) &&
        raw4.valid && raw4.no_draw && raw4.rect_number == 0x366u &&
        raw4.destination_x == 5 && raw4.destination_y == 6 && raw4.width == 2u &&
        raw4.height == 2u && raw4.table_hash != 0u && raw4.row_hash != 0u;
    ok &= dm2_v1_gdat_pit_roof_no_draw_admission(&material, &light, &b073, &raw4);
    /* SKWIN c_image.cpp:450-475; c_gfx_blit.cpp:370-549: the 4bpp alpha
     * sentinel is the low nibble of the source mask.  Pit roofs prove only
     * normal and horizontal-mirror modes; vertical variants stay rejected. */
    ok &= dm2_v1_gdat_pit_roof_alpha_blend_receipt_build(&material, &b073, &raw4,
        20u, 0u, &blend) && blend.valid && blend.no_draw &&
        blend.alphamask == 20u && blend.alpha_index == 4u &&
        blend.blit_mode == 0u && blend.identity_hash != 0u;
    i = dm2_v1_gdat_pit_roof_destination_receipt_build(&material, &raw4,
        &blend, &traversal, 0x40u, 0x40u, 0, 0, 0u, &destination);
    ok &= i &&
        destination.valid && destination.no_draw && destination.destination_x == 5 &&
        destination.destination_y == 6 && destination.source_width == 2u &&
        destination.alpha_index == 4u;
    ok &= !dm2_v1_gdat_pit_roof_destination_receipt_build(&material, &raw4,
        &blend, &traversal, 0x41u, 0x40u, 0, 0, 0u, &destination_reject);
    ++raw4.identity_hash;
    ok &= !dm2_v1_gdat_pit_roof_destination_receipt_build(&material, &raw4,
        &blend, &traversal, 0x40u, 0x40u, 0, 0, 0u, &destination_reject);
    --raw4.identity_hash;
    ok &= dm2_v1_gdat_pit_roof_surface_clip_receipt_build(&destination,
        320u, 200u, 8u, 1u, 5, 6, 2u, 2u, &surface_clip) &&
        surface_clip.valid && surface_clip.no_draw && surface_clip.bitmap_width == 320u;
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.framebuffer = surface_bytes; snapshot.width = 320u; snapshot.height = 200u;
    snapshot.stride = 320u; snapshot.resolution = 8u; snapshot.generation = 1u;
    ok &= dm2_v1_gdat_pit_roof_surface_binding_receipt_build(&surface_clip,
        &snapshot, &surface_binding) && surface_binding.valid && surface_binding.no_draw;
    memset(&composition, 0, sizeof(composition));
    composition.valid = 1; composition.no_draw = 1; composition.identity_hash = 1u;
    composition.ordered_member_hash = 1u;
    composition.session_identity = 1u; composition.data_epoch = 1u;
    composition.surface_before.framebuffer = surface_bytes;
    composition.surface_after.framebuffer = surface_bytes;
    composition.surface_before.width = 320u; composition.surface_after.width = 320u;
    composition.surface_before.height = 200u; composition.surface_after.height = 200u;
    composition.surface_before.stride = 320u; composition.surface_after.stride = 320u;
    composition.surface_before.resolution = 8u; composition.surface_after.resolution = 8u;
    composition.session_identity = 0u;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    composition.session_identity = 1u;
    composition.data_epoch = 0u;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    composition.data_epoch = 1u;
    composition.ordered_member_hash = 0u;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    composition.ordered_member_hash = 1u;
    composition.surface_before.generation = 1u; composition.surface_after.generation = 1u;
    composition.surface_generation_hash = 1u;
    ok &= dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.indexed_pixels = surface_bytes;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.indexed_pixels = original_pixels;
    ++material.width;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.width = original_width;
    ++material.height;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.height = original_height;
    material.indexed_pixels = NULL;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.indexed_pixels = original_pixels;
    material.indexed_pixel_count = 3u;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.indexed_pixel_count = original_pixel_count;
    ++material.pixel_stride;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.pixel_stride = original_stride;
    material.palette_hash = 0u;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.palette_hash = original_palette_hash;
    material.identity_hash = 0u;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    material.identity_hash = original_material_identity_hash;
    ++composition.surface_after.generation;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    --composition.surface_after.generation;
    composition.surface_after.framebuffer = surface_bytes + 1u;
    ok &= !dm2_v1_gdat_pit_roof_composition_surface_matches(&surface_binding, &composition, &material, &handoff);
    composition.surface_after.framebuffer = surface_bytes;
    composition.m11_delivery_ready = 1;
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_roof_consume_ordered_m11(&surface_binding, &composition,
        &material, &handoff, &raw4, &blend, &traversal, &destination, &surface_clip);
    for (i = 0; i < 4; ++i) {
        uint16_t x = (uint16_t)(i % 2);
        uint16_t y = (uint16_t)(i / 2);
        uint8_t index = handoff.indexed_pixels[i];
        uint8_t expected = index == destination.alpha_index ? 0xa5u :
            traversal.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + y) * 320u + 5u + x] == expected;
    }
    memset(surface_bytes, 0x5au, sizeof(surface_bytes));
    composition.ordered_member_hash = 0u;
    ok &= !dm2_v1_gdat_pit_roof_consume_ordered_m11(&surface_binding, &composition,
        &material, &handoff, &raw4, &blend, &traversal, &destination, &surface_clip) &&
        surface_bytes[6u * 320u + 5u] == 0x5au &&
        surface_bytes[6u * 320u + 6u] == 0x5au &&
        surface_bytes[7u * 320u + 5u] == 0x5au &&
        surface_bytes[7u * 320u + 6u] == 0x5au;
    composition.ordered_member_hash = 1u;
    original_first_index = material.indexed_pixels[0];
    ((uint8_t *)material.indexed_pixels)[0] = 0x10u;
    ok &= !dm2_v1_gdat_pit_roof_consume_ordered_m11(&surface_binding, &composition,
        &material, &handoff, &raw4, &blend, &traversal, &destination, &surface_clip) &&
        surface_bytes[6u * 320u + 5u] == 0x5au &&
        surface_bytes[6u * 320u + 6u] == 0x5au &&
        surface_bytes[7u * 320u + 5u] == 0x5au &&
        surface_bytes[7u * 320u + 6u] == 0x5au;
    ((uint8_t *)material.indexed_pixels)[0] = original_first_index;
    composition.surface_after.framebuffer = surface_bytes + 1u;
    ok &= !dm2_v1_gdat_pit_roof_consume_ordered_m11(&surface_binding, &composition,
        &material, &handoff, &raw4, &blend, &traversal, &destination, &surface_clip) &&
        surface_bytes[6u * 320u + 5u] == 0x5au &&
        surface_bytes[6u * 320u + 6u] == 0x5au &&
        surface_bytes[7u * 320u + 5u] == 0x5au &&
        surface_bytes[7u * 320u + 6u] == 0x5au;
    composition.surface_after.framebuffer = surface_bytes;
    ++snapshot.generation;
    ok &= !dm2_v1_gdat_pit_roof_surface_binding_receipt_build(&surface_clip,
        &snapshot, &surface_binding);
    --snapshot.generation;
    ok &= !dm2_v1_gdat_pit_roof_surface_clip_receipt_build(&destination,
        320u, 200u, 8u, 0u, 5, 6, 2u, 2u, &surface_clip);
    ok &= !dm2_v1_gdat_pit_roof_surface_clip_receipt_build(&destination,
        320u, 200u, 8u, 0x12345678u, 6, 6, 2u, 2u, &surface_clip);
    ok &= !dm2_v1_gdat_pit_roof_alpha_blend_receipt_build(&material, &b073, &raw4,
        21u, 0u, &blend);
    ok &= !dm2_v1_gdat_pit_roof_alpha_blend_receipt_build(&material, &b073, &raw4,
        20u, 2u, &blend);
    ok &= dm2_v1_gdat_pit_roof_m11_receipt_build(&loader, 1u, 2u, 20u,
        &state, &mirror_material) &&
        dm2_v1_gdat_pit_roof_material_handoff_receipt_build(&mirror_material,
            &mirror_handoff) &&
        dm2_v1_gdat_pit_roof_b073_receipt_build(&mirror_material, &light,
            &mirror_b073) &&
        dm2_v1_gdat_pit_roof_raw4_receipt_build(&loader, &mirror_material,
            &mirror_raw4) &&
        dm2_v1_gdat_pit_roof_b073_tables_receipt_build(&loader, &mirror_material,
            &mirror_b073, &mirror_tables) &&
        dm2_v1_gdat_pit_roof_b073_traversal_receipt_build(&loader, &mirror_material,
            &mirror_b073, &mirror_tables, &mirror_traversal) &&
        dm2_v1_gdat_pit_roof_alpha_blend_receipt_build(&mirror_material,
            &mirror_b073, &mirror_raw4, 20u, 1u, &blend) &&
        blend.mirror_flip == 1u && blend.blit_mode == 1u &&
        dm2_v1_gdat_pit_roof_destination_receipt_build(&mirror_material,
            &mirror_raw4, &blend, &mirror_traversal, 0x40u, 0x40u, 0, 0,
            1u, &mirror_destination) &&
        dm2_v1_gdat_pit_roof_surface_clip_receipt_build(&mirror_destination,
            320u, 200u, 8u, 1u, 5, 6, 2u, 2u, &mirror_surface_clip) &&
        dm2_v1_gdat_pit_roof_surface_binding_receipt_build(&mirror_surface_clip,
            &snapshot, &mirror_surface_binding);
    memset(surface_bytes, 0xa5, sizeof(surface_bytes));
    ok &= dm2_v1_gdat_pit_roof_consume_ordered_m11(&mirror_surface_binding,
        &composition, &mirror_material, &mirror_handoff, &mirror_raw4, &blend,
        &mirror_traversal, &mirror_destination, &mirror_surface_clip);
    for (i = 0; i < 4; ++i) {
        uint16_t x = (uint16_t)(i % 2);
        uint16_t y = (uint16_t)(i / 2);
        uint8_t index = mirror_handoff.indexed_pixels[(size_t)y * 2u + (1u - x)];
        uint8_t expected = index == mirror_destination.alpha_index ? 0xa5u :
            mirror_traversal.transformed_palette16[index];
        ok &= surface_bytes[(size_t)(6u + y) * 320u + 5u + x] == expected;
    }
    ++light.receipt_hash;
    ok &= !dm2_v1_gdat_pit_roof_no_draw_admission(&material, &light, &b073, &raw4);
    --light.receipt_hash;
    raw[28u + 8u + 6u * 8u + 2u] = 1u;
    ok &= !dm2_v1_gdat_pit_roof_raw4_receipt_build(&loader, &material, &raw4);
    raw[28u + 8u + 6u * 8u + 2u] = 0u;
    raw_sizes[0] = 12u;
    ok &= !dm2_v1_gdat_pit_roof_m11_receipt_build(&loader, 1u, 1u, 20u,
        &state, &material);
    if (!ok) fputs("DM2 pit roof M11 receipt failed\n", stderr);
    return ok ? 0 : 1;
}
