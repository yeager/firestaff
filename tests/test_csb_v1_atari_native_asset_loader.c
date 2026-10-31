#include "asset_loader_m11.h"
#include "csb_v1_atari_f0113_fluxcage_blit.h"
#include "csb_v1_atari_f0113_fluxcage_plan.h"
#include "dm1_v1_creature_render_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static unsigned long long hash_pixels(const unsigned char *pixels, size_t count)
{
    unsigned long long hash = 14695981039346656037ull;
    size_t index;
    for (index = 0u; index < count; ++index) {
        hash ^= pixels[index];
        hash *= 1099511628211ull;
    }
    return hash;
}

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : getenv("FIRESTAFF_CSB_ATARI_GRAPHICS");
    M11_AssetLoader loader;
    CSB_AtariStLoader native;
    uint8_t *objects;
    unsigned int index;
    unsigned int decoded = 0u;
    unsigned int decoded_creatures = 0u;
    unsigned int absent_creature_slots = 0u;
    if (!path || !path[0]) return 77;
    if (!M11_AssetLoader_InitCsbAtariStFromFile(&loader, path)) {
        fprintf(stderr, "cannot open authentic CSB Atari ST GRAPHICS.DAT: %s\n", path);
        return 1;
    }
    if (!loader.csbAtariSt || loader.graphicCount != 563u) {
        M11_AssetLoader_Shutdown(&loader);
        return 2;
    }
    {
        /* ReDMCSB MEDIA020 G0188 D3L/D2L/D1L/D0L selects mask records
         * 69..72; the byte widths in G0188 become target pixel widths by
         * doubling. Decode the actual Atari media and compare those mask
         * assets with the source dimensions. */
        static const unsigned int mask_graphics[] = {69u, 70u, 71u, 72u};
        static const unsigned short mask_dimensions[][2] = {
            {96u, 51u}, {80u, 71u}, {64u, 111u}, {32u, 136u}
        };
        unsigned int field_index;
        for (field_index = 0u;
             field_index < sizeof(mask_graphics) / sizeof(mask_graphics[0]);
             ++field_index) {
            const unsigned int graphic = mask_graphics[field_index];
            const unsigned short expected_width = mask_dimensions[field_index][0];
            const unsigned short expected_height = mask_dimensions[field_index][1];
            unsigned short width = 0u, height = 0u;
            const M11_AssetSlot *asset;
            if (!M11_AssetLoader_QuerySize(&loader, graphic, &width, &height) ||
                width != expected_width || height != expected_height ||
                !(asset = M11_AssetLoader_Load(&loader, graphic)) ||
                !asset->loaded || !asset->pixels || asset->width != expected_width ||
                asset->height != expected_height) {
                fprintf(stderr,
                        "CSB Atari MEDIA020 F0113 mask %u failed authentic %ux%u decode (%ux%u)\n",
                        graphic, expected_width, expected_height, width, height);
                M11_AssetLoader_Shutdown(&loader);
                return 21;
            }
        }
    }
    {
        /* C074 is the 32x32 field source in the authenticated Atari bank.
         * ReDMCSB DEFS.H M076 counts 64 eight-byte/16-pixel source units.
         * BLITMASK.C F0133 treats lastUnit as an exclusive restart sentinel:
         * its Atari loop writes the current unit, increments firstUnit, then
         * compares with lastUnit (lines 222-231). Thus last=64 resets before
         * reading unit 64; the authentic source needs exactly 64 units. */
        unsigned short width = 0u, height = 0u;
        const unsigned int source_units_from_media = 64u;
        const unsigned int last_unit_base = 63u;
        const unsigned int last_unit_random_range = 2u;
        const unsigned int max_last_unit = last_unit_base +
                                           last_unit_random_range - 1u;
        const M11_AssetSlot *asset;
        if (!M11_AssetLoader_QuerySize(&loader, 74u, &width, &height) ||
            width != 32u || height != 32u ||
            ((unsigned int)width * height) / 16u != source_units_from_media ||
            max_last_unit > source_units_from_media ||
            !(asset = M11_AssetLoader_Load(&loader, 74u)) ||
            !asset->loaded || !asset->pixels || asset->width != 32u ||
            asset->height != 32u) {
            fprintf(stderr,
                    "CSB Atari C074 authentic F0133 source units invalid (%ux%u)\n",
                    width, height);
            M11_AssetLoader_Shutdown(&loader);
            return 22;
        }
    }
    {
        /* Compose original C074 with original field masks through the exact
         * MEDIA020 F0113/F0133/F0132 planner and pixel path. This tests the
         * source pixel boundary without fabricating dungeon/game state. */
        static const unsigned short target_squares[] = {1u, 3u};
        static const unsigned int expected_masks[] = {69u, 0u};
        static const unsigned int expected_pixel_counts[] = {757u, 1279u};
        static const unsigned long long expected_hashes[] = {
            0x85a19ec6443e5fc0ull, 0x8a956c37188746ceull
        };
        unsigned int composition_index;
        for (composition_index = 0u; composition_index < 2u;
             ++composition_index) {
            CSB_V1_AtariF0113FluxcageInput input;
            CSB_V1_AtariF0113FluxcagePlan plan;
            const M11_AssetSlot *field;
            const M11_AssetSlot *mask = NULL;
            unsigned char frame[320u * 200u];
            unsigned int written_in_clip = 0u;
            int written;
            unsigned int y;

            memset(&input, 0, sizeof(input));
            input.explosion_type = CSB_V1_ATARI_F0113_C050;
            input.view_square = (int)target_squares[composition_index];
            input.door_front_view_pass = 2;
            if (csb_v1_atari_f0113_plan_fluxcage(&input, &plan) !=
                    CSB_V1_ATARI_F0113_FLUXCAGE_DRAW_FIELD ||
                plan.graphic_index != 74 ||
                (composition_index == 0u &&
                 plan.mask_graphic_index != (int)expected_masks[0]) ||
                (composition_index == 1u &&
                 (plan.mask_graphic_index != -1 ||
                  plan.mask_flip_horizontal))) {
                fprintf(stderr,
                        "CSB Atari F0113 real-media composition plan failed at square %u\n",
                        target_squares[composition_index]);
                M11_AssetLoader_Shutdown(&loader);
                return 23;
            }
            field = M11_AssetLoader_Load(&loader, 74u);
            if (expected_masks[composition_index] != 0u) {
                mask = M11_AssetLoader_Load(&loader,
                    expected_masks[composition_index]);
            }
            if (!field || !field->loaded || !field->pixels ||
                (expected_masks[composition_index] != 0u &&
                 (!mask || !mask->loaded || !mask->pixels))) {
                fprintf(stderr,
                        "CSB Atari F0113 original media missing for composition\n");
                M11_AssetLoader_Shutdown(&loader);
                return 24;
            }
            memset(frame, 0xee, sizeof(frame));
            written = csb_v1_atari_f0113_blit_fluxcage(
                &plan, field->pixels, field->width, field->height,
                mask ? mask->pixels : NULL, mask ? mask->width : 0,
                mask ? mask->height : 0,
                composition_index == 0u ? 64 : 60, 0, frame, 320, 200,
                48, 33);
            if (written <= 0) {
                fprintf(stderr,
                        "CSB Atari F0113 original-media composition drew no pixels\n");
                M11_AssetLoader_Shutdown(&loader);
                return 25;
            }
            for (y = 0u; y < 200u; ++y) {
                unsigned int x;
                for (x = 0u; x < 320u; ++x) {
                    const unsigned char color = frame[y * 320u + x];
                    const unsigned int in_clip =
                        x >= 48u + plan.destination_frame[0] &&
                        x <= 48u + plan.destination_frame[1] &&
                        y >= 33u + plan.destination_frame[2] &&
                        y <= 33u + plan.destination_frame[3];
                    if (!in_clip && color != 0xeeu) {
                        fprintf(stderr,
                                "CSB Atari F0113 composition escaped G0163 clip\n");
                        M11_AssetLoader_Shutdown(&loader);
                        return 26;
                    }
                    if (in_clip && color != 0xeeu) {
                        if (color != 7u && color != 11u) {
                            fprintf(stderr,
                                    "CSB Atari F0113 composition emitted unexpected C074 color %u\n",
                                    (unsigned)color);
                            M11_AssetLoader_Shutdown(&loader);
                            return 27;
                        }
                        ++written_in_clip;
                    }
                }
            }
            if (written_in_clip != (unsigned int)written) {
                fprintf(stderr,
                        "CSB Atari F0113 composition pixel count mismatch %u != %d\n",
                        written_in_clip, written);
                M11_AssetLoader_Shutdown(&loader);
                return 28;
            }
            {
                const unsigned long long hash =
                    hash_pixels(frame, sizeof(frame));
                if ((unsigned int)written !=
                        expected_pixel_counts[composition_index] ||
                    hash != expected_hashes[composition_index]) {
                    fprintf(stderr,
                            "CSB Atari F0113 composition changed at square %u: pixels=%d hash=%016llx\n",
                            target_squares[composition_index], written, hash);
                    M11_AssetLoader_Shutdown(&loader);
                    return 29;
                }
                fprintf(stderr,
                        "F0113 composition square=%u mask=%d pixels=%d hash=%016llx\n",
                        target_squares[composition_index],
                        plan.mask_graphic_index, written, hash);
            }
        }
    }
    {
        /* ReDMCSB DUNVIEW.C MEDIA020 G0211 dimensions and M614 mapping:
         * fire/spell/poison native explosion bitmaps at indices 348..350.
         * Smoke (G0211 aspect 3) reuses poison pixels with G0212 colors. */
        static const unsigned short dimensions[3][2] = {
            {160u,111u}, {128u,97u}, {160u,91u}
        };
        for (index = 0u; index < 3u; ++index) {
            const unsigned int graphic = 348u + index;
            unsigned short width = 0u, height = 0u;
            const M11_AssetSlot *asset;
            if (!M11_AssetLoader_QuerySize(&loader, graphic, &width, &height) ||
                width != dimensions[index][0] ||
                height != dimensions[index][1]) {
                fprintf(stderr,
                        "CSB Atari MEDIA020 G0211 explosion %u has unexpected dimensions\n",
                        graphic);
                M11_AssetLoader_Shutdown(&loader);
                return 17;
            }
            asset = M11_AssetLoader_Load(&loader, graphic);
            if (!asset || !asset->loaded || !asset->pixels ||
                asset->width != width || asset->height != height) {
                fprintf(stderr,
                        "CSB Atari MEDIA020 M614 explosion %u did not decode from original media\n",
                        graphic);
                M11_AssetLoader_Shutdown(&loader);
                return 18;
            }
        }
    }
    {
        /* ReDMCSB G0210 lightning aspect 3 maps to native bitmap 326. */
        unsigned short width = 0u, height = 0u;
        const M11_AssetSlot *asset;
        if (!M11_AssetLoader_QuerySize(&loader, 326u, &width, &height) ||
            width != 64u || height != 15u ||
            !(asset = M11_AssetLoader_Load(&loader, 326u)) ||
            !asset->loaded || !asset->pixels || asset->width != 64u ||
            asset->height != 15u) {
            fprintf(stderr,
                    "CSB Atari C100 lightning bitmap 326 failed original-media decode\n");
            M11_AssetLoader_Shutdown(&loader);
            return 19;
        }
    }
    {
        /* ReDMCSB DUNVIEW.C D0C uses M636's nine indexed patterns through
         * F0133. Its randomized exclusive restart unit reaches 90, so every
         * source graphic must expose at least 90 complete 16-pixel units. */
        for (index = 351u; index <= 359u; ++index) {
            unsigned short width = 0u, height = 0u;
            const M11_AssetSlot *asset;
            if (!M11_AssetLoader_QuerySize(&loader, index, &width, &height) ||
                width == 0u || width % 16u != 0u || height == 0u ||
                (unsigned int)(width / 16u) * height < 90u ||
                !(asset = M11_AssetLoader_Load(&loader, index)) ||
                !asset->loaded || !asset->pixels || asset->width != width ||
                asset->height != height) {
                fprintf(stderr,
                        "CSB Atari M636 D0C pattern %u failed F0133 source validation (%ux%u)\n",
                        index, width, height);
                M11_AssetLoader_Shutdown(&loader);
                return 20;
            }
        }
    }
    csb_atari_st_graphics_loader_init(&native);
    if (!csb_atari_st_graphics_loader_open(&native, path) ||
        native.items[558u].decompressed_size != 4720u) {
        csb_atari_st_graphics_loader_close(&native);
        M11_AssetLoader_Shutdown(&loader);
        fprintf(stderr, "CSB Atari C558 object-aspect record has an unexpected layout\n");
        return 5;
    }
    objects = (uint8_t *)malloc(4720u);
    if (!objects || csb_atari_st_graphics_loader_read_item(
            &native, 558u, objects, 4720u) != 4720) {
        free(objects);
        csb_atari_st_graphics_loader_close(&native);
        M11_AssetLoader_Shutdown(&loader);
        return 6;
    }
    {
        /* ReDMCSB DUNGEON.C MEDIA389 (CSB Atari ST) G0243 GraphicInfo.
         * These values select the optional G0219 side/back/attack images;
         * do not assume the DM1 table is interchangeable without checking. */
        static const uint16_t csb_atari_graphic_info[27] = {
            0x623du, 0xa625u, 0x6198u, 0xb225u, 0xa3b8u, 0x539du,
            0x0020u, 0x0220u, 0x5225u, 0x71b8u, 0x11b8u, 0x0225u,
            0x6038u, 0xb23du, 0x1638u, 0x523du, 0xa038u, 0xf23du,
            0xa3bdu, 0xe23du, 0x0225u, 0xa3bdu, 0x53bdu, 0x0038u,
            0x97bdu, 0x0000u, 0x0000u
        };
        static const uint8_t first_creature_aspects[] = {
            0,0,0,0,56,84,56,84,56,84,0x1d,0x01,
            0,4,0,0,32,66,0,0,32,69,0x0b,0x20
        };
        static const uint8_t first_creature_coordinates[] = {
            95,70,127,70,129,75,93,75,111,72
        };
        if (memcmp(objects + 832u, first_creature_aspects,
                   sizeof(first_creature_aspects)) != 0 ||
            memcmp(objects + 264u, first_creature_coordinates,
                   sizeof(first_creature_coordinates)) != 0) {
            fprintf(stderr, "CSB Atari C558 G0219/G0224 tables do not match the source layouts\n");
            free(objects);
            M11_AssetLoader_Shutdown(&loader);
            return 8;
        }
        for (index = 0u; index < 27u; ++index) {
            const uint8_t *aspect = objects + 832u + index * 12u;
            const unsigned int first_native =
                ((unsigned int)aspect[0] << 8) | aspect[1];
            const unsigned int graphic_info = dm1_creature_graphic_info((int)index);
            const unsigned int side = (graphic_info & 0x0008u) != 0u;
            const unsigned int back = (graphic_info & 0x0010u) != 0u;
            const unsigned int attack = (graphic_info & 0x0020u) != 0u;
            unsigned short width = 0u, height = 0u;
            unsigned short variant_width = 0u, variant_height = 0u;
            unsigned int bitmap_index;
            const M11_AssetSlot *variant_asset;
            if (graphic_info != csb_atari_graphic_info[index]) {
                fprintf(stderr,
                        "CSB Atari G0243 creature %u GraphicInfo differs from ReDMCSB: Firestaff=0x%04x source=0x%04x\n",
                        index, graphic_info, csb_atari_graphic_info[index]);
                free(objects);
                M11_AssetLoader_Shutdown(&loader);
                return 16;
            }
            if (first_native >= 117u || aspect[4] == 0u || aspect[5] == 0u ||
                (aspect[10] >> 4) >= 3u ||
                !M11_AssetLoader_QuerySize(&loader, 446u + first_native,
                                           &width, &height) ||
                width != (unsigned short)(aspect[4] * 2u) ||
                height != aspect[5]) {
                fprintf(stderr,
                        "CSB Atari G0219 creature %u disagrees with its native front graphic\n",
                        index);
                free(objects);
                M11_AssetLoader_Shutdown(&loader);
                return 9;
            }
            if (side) {
                bitmap_index = 446u + first_native + 1u;
                if (aspect[6] == 0u || aspect[7] == 0u ||
                    !M11_AssetLoader_QuerySize(&loader, bitmap_index,
                                               &variant_width, &variant_height) ||
                    variant_width != (unsigned short)(aspect[6] * 2u) ||
                    variant_height != aspect[7]) {
                    fprintf(stderr,
                            "CSB Atari G0219 creature %u has an invalid G0243 side bitmap %u\n",
                            index, bitmap_index);
                    M11_AssetLoader_Shutdown(&loader);
                    return 13;
                }
                variant_asset = M11_AssetLoader_Load(&loader, bitmap_index);
                if (!variant_asset || !variant_asset->loaded ||
                    !variant_asset->pixels || variant_asset->width != variant_width ||
                    variant_asset->height != variant_height) {
                    fprintf(stderr,
                            "CSB Atari G0219 creature %u failed to decode G0243 side bitmap %u\n",
                            index, bitmap_index);
                    M11_AssetLoader_Shutdown(&loader);
                    return 13;
                }
            }
            if (back) {
                bitmap_index = 446u + first_native + (side ? 2u : 1u);
                if (!M11_AssetLoader_QuerySize(&loader, bitmap_index,
                                               &variant_width, &variant_height) ||
                    variant_width != width || variant_height != height) {
                    fprintf(stderr,
                            "CSB Atari G0219 creature %u has an invalid G0243 back bitmap %u\n",
                            index, bitmap_index);
                    M11_AssetLoader_Shutdown(&loader);
                    return 14;
                }
                variant_asset = M11_AssetLoader_Load(&loader, bitmap_index);
                if (!variant_asset || !variant_asset->loaded ||
                    !variant_asset->pixels || variant_asset->width != variant_width ||
                    variant_asset->height != variant_height) {
                    fprintf(stderr,
                            "CSB Atari G0219 creature %u failed to decode G0243 back bitmap %u\n",
                            index, bitmap_index);
                    M11_AssetLoader_Shutdown(&loader);
                    return 14;
                }
            }
            if (attack) {
                bitmap_index = 446u + first_native + 1u + side + back;
                if (aspect[8] == 0u || aspect[9] == 0u ||
                    !M11_AssetLoader_QuerySize(&loader, bitmap_index,
                                               &variant_width, &variant_height) ||
                    variant_width != (unsigned short)(aspect[8] * 2u) ||
                    variant_height != aspect[9]) {
                    fprintf(stderr,
                            "CSB Atari G0219 creature %u has an invalid G0243 attack bitmap %u\n",
                            index, bitmap_index);
                    M11_AssetLoader_Shutdown(&loader);
                    return 15;
                }
                variant_asset = M11_AssetLoader_Load(&loader, bitmap_index);
                if (!variant_asset || !variant_asset->loaded ||
                    !variant_asset->pixels || variant_asset->width != variant_width ||
                    variant_asset->height != variant_height) {
                    fprintf(stderr,
                            "CSB Atari G0219 creature %u failed to decode G0243 attack bitmap %u\n",
                            index, bitmap_index);
                    M11_AssetLoader_Shutdown(&loader);
                    return 15;
                }
            }
        }
        for (index = 0u; index < 330u; index += 2u) {
            if (objects[264u + index] >= 224u ||
                objects[264u + index + 1u] > 135u) {
                fprintf(stderr, "CSB Atari G0224 contains an invalid viewport coordinate\n");
                free(objects);
                M11_AssetLoader_Shutdown(&loader);
                return 10;
            }
        }
    }
    csb_atari_st_graphics_loader_close(&native);
    for (index = 0u; index < 85u; ++index) {
        const uint8_t *aspect = objects + 1642u + index * 6u;
        unsigned short width = 0u, height = 0u;
        if (aspect[0] >= 86u || aspect[2] == 0u || aspect[3] == 0u ||
            !M11_AssetLoader_QuerySize(&loader, 360u + aspect[0],
                                       &width, &height) ||
            width != (unsigned short)(aspect[2] * 2u) ||
            height != aspect[3]) {
            fprintf(stderr, "CSB Atari G0209 aspect %u disagrees with its source graphic\n",
                    index);
            free(objects);
            M11_AssetLoader_Shutdown(&loader);
            return 7;
        }
    }
    free(objects);
    for (index = 360u; index < 446u; ++index) {
        unsigned short width = 0u, height = 0u;
        const M11_AssetSlot *asset;
        /* C558 uses offsets into the M618 address range; the native archive
         * is sparse and an unreferenced hole (for example index 533) is not
         * a missing asset. G0219's referenced front/side/attack entries are
         * checked above and must decode. Validate every present slot here. */
        if (!M11_AssetLoader_QuerySize(&loader, index, &width, &height)) {
            ++absent_creature_slots;
            continue;
        }
        if (width == 0u || height == 0u) {
            fprintf(stderr, "CSB Atari object graphic %u has no valid dimensions\n", index);
            M11_AssetLoader_Shutdown(&loader);
            return 3;
        }
        asset = M11_AssetLoader_Load(&loader, index);
        if (!asset || !asset->loaded || !asset->pixels ||
            asset->width != width || asset->height != height) {
            fprintf(stderr, "CSB Atari object graphic %u failed to decode\n", index);
            M11_AssetLoader_Shutdown(&loader);
            return 4;
        }
        ++decoded;
    }
    /* MEDIA020 assigns M618_GRAPHIC_FIRST_CREATURE=446 and the Atari
     * GRAPHICS.DAT address range ends at C562. Validate every present native
     * creature surface, including alternate poses reached through G0219
     * offsets. The address range is sparse; holes are not broken assets. */
    for (index = 446u; index < 563u; ++index) {
        unsigned short width = 0u, height = 0u;
        const M11_AssetSlot *asset;
        if (!M11_AssetLoader_QuerySize(&loader, index, &width, &height)) {
            ++absent_creature_slots;
            continue;
        }
        if (width == 0u || height == 0u) {
            fprintf(stderr,
                    "CSB Atari creature graphic %u has no valid dimensions\n",
                    index);
            M11_AssetLoader_Shutdown(&loader);
            return 11;
        }
        asset = M11_AssetLoader_Load(&loader, index);
        if (!asset || !asset->loaded || !asset->pixels ||
            asset->width != width || asset->height != height) {
            fprintf(stderr,
                    "CSB Atari creature graphic %u failed to decode\n",
                    index);
            M11_AssetLoader_Shutdown(&loader);
            return 12;
        }
        ++decoded_creatures;
    }
    M11_AssetLoader_Shutdown(&loader);
    printf("decoded %u original CSB Atari ST object graphics and %u present creature graphics (%u unreferenced sparse slots); all 27 G0243 GraphicInfo values match MEDIA389, all G0219 front/side/back/attack references decode, and 85 G0209 objects plus G0224 coordinates match C558: %s\n",
           decoded, decoded_creatures, absent_creature_slots, path);
    return 0;
}
