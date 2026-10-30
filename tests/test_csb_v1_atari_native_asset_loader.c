#include "asset_loader_m11.h"
#include "dm1_v1_creature_render_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
