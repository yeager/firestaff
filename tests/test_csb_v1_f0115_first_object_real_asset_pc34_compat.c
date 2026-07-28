#include "asset_loader_m11.h"
#include "csb_v1_viewport_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    M11_AssetLoader loader;
    uint8_t screen[320 * 200];
    int graphics[86];
    const int weapon_graphics[] = {
        537, 537, 534, 536, 510, 511, 511, 538, 516, 511, 511, 511,
        511, 511, 511, 511, 541, 511, 512, 512, 520, 520, 532, 542,
        543, 513, 544, 515, 545, 510, 546, 547, 548, 549, 510, 530,
        530, 510, 510, 510, 550, 531, 529, 564, 544, 581
    };
    const int armour_graphics[] = {
        522, 522, 522, 554, 507, 523, 523, 523, 523, 568, 523, 523,
        568, 506, 506, 556, 522, 522, 528, 568, 568, 523, 523, 552,
        552, 508, 508, 508, 553, 553, 509, 553, 518, 518, 518, 518,
        508, 518, 551, 519, 521, 555, 509, 551, 519, 521, 555, 509,
        551, 519, 521, 555, 509, 551, 518, 521, 580, 583
    };
    const int junk_graphics[] = {
        533, 505, 514, 514, 539, 540, 503, 582, 503, 517, 517, 517,
        517, 517, 517, 517, 517, 561, 561, 561, 561, 561, 561, 561,
        561, 502, 559, 560, 526, 527, 524, 525, 570, 569, 504, 565,
        514, 514, 557, 558, 558, 578, 562, 563, 571, 572, 573, 574,
        576, 577, 573, 540, 539
    };
    size_t i;
    {
        const uint8_t source[4] = {10, 1, 2, 3};
        uint8_t pixels[320 * 200];

        memset(pixels, 0x5a, sizeof(pixels));
        if (csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
                498, source, 2, 2, pixels, 320, 200, 320,
                1, 33, 2, 2, 2, 1) != 3 ||
            pixels[33 * 320 + 1] != 1 ||
            pixels[33 * 320 + 2] != 0x5a ||
            pixels[34 * 320 + 1] != 3 ||
            pixels[34 * 320 + 2] != 2 ||
            csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
                498, source, 2, 2, pixels, 320, 200, 320,
                1, 32, 2, 1, 2, 0) != 0) {
            fprintf(stderr, "FAIL: first-object C10/flip/palette/clip contract\n");
            return 1;
        }
        memset(pixels, 0x5a, sizeof(pixels));
        if (csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
                511, source, 2, 2, pixels, 320, 200, 320,
                1, 33, 2, 2, 2, 1) != 3 ||
            pixels[33 * 320 + 1] != 0x5a ||
            pixels[33 * 320 + 2] != 1 ||
            pixels[34 * 320 + 1] != 2 ||
            pixels[34 * 320 + 2] != 3) {
            fprintf(stderr, "FAIL: source-nonflippable weapon lane contract\n");
            return 1;
        }
        memset(pixels, 0x5a, sizeof(pixels));
        if (csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
                499, source, 2, 2, pixels, 320, 200, 320,
                1, 33, 2, 2, 2, 1) != 3 ||
            pixels[33 * 320 + 1] != 0x5a ||
            pixels[33 * 320 + 2] != 1 ||
            pixels[34 * 320 + 1] != 2 ||
            pixels[34 * 320 + 2] != 3) {
            fprintf(stderr, "FAIL: alcove Chest must suppress right-lane flip\n");
            return 1;
        }
        memset(pixels, 0x5a, sizeof(pixels));
        if (csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
                563, source, 2, 2, pixels, 320, 200, 320,
                1, 33, 2, 2, 2, 1) != 3 ||
            pixels[33 * 320 + 1] != 1 ||
            pixels[33 * 320 + 2] != 0x5a ||
            pixels[34 * 320 + 1] != 3 ||
            pixels[34 * 320 + 2] != 2 ||
            csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
                579, source, 2, 2, pixels, 320, 200, 320,
                1, 36, 2, 2, 2, 1) != 3) {
            fprintf(stderr, "FAIL: source-marked armour/junk flip contract\n");
            return 1;
        }
    }

    if (csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_CONTAINER, 0, 0) != 498 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_CONTAINER, 0, 1) != 499 ||
        csb_v1_viewport_f0115_first_object_native_graphic_pc34(
            THING_TYPE_CONTAINER, 0) != 498 ||
        csb_v1_viewport_f0115_first_object_native_graphic_pc34(
            THING_TYPE_SCROLL, 0) != 500 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_POTION, 0, 0) != 565 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_POTION, 6, 0) != 500 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_POTION, 16, 0) != 566 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_POTION, 20, 0) != 578 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_WEAPON, 0, 0) != 537 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_WEAPON, 8, 0) != 516 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_WEAPON, 41, 0) != 531 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_WEAPON, 45, 0) != 581 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_POTION, 21, 0) >= 0 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_WEAPON, 46, 0) >= 0 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_ARMOUR, 58, 0) >= 0 ||
        csb_v1_viewport_f0115_object_native_graphic_pc34(
            THING_TYPE_JUNK, 53, 0) >= 0 ||
        csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
            584, (const uint8_t[]){1}, 1, 1, screen, 320, 200, 320,
            0, 33, 1, 1, 1, 0) != 0 ||
        csb_v1_viewport_f0115_first_object_native_graphic_pc34(
            THING_TYPE_CONTAINER, 1) >= 0) {
        fprintf(stderr, "FAIL: CSB first-object G0209 mapping is not fail-closed\n");
        return 1;
    }
    for (i = 0; i < sizeof(weapon_graphics) / sizeof(weapon_graphics[0]); ++i) {
        if (csb_v1_viewport_f0115_object_native_graphic_pc34(
                THING_TYPE_WEAPON, (int)i, 0) != weapon_graphics[i]) {
            fprintf(stderr, "FAIL: weapon subtype %zu native mapping\n", i);
            return 1;
        }
    }
    for (i = 0; i < sizeof(armour_graphics) / sizeof(armour_graphics[0]); ++i) {
        if (csb_v1_viewport_f0115_object_native_graphic_pc34(
                THING_TYPE_ARMOUR, (int)i, 0) != armour_graphics[i]) {
            fprintf(stderr, "FAIL: armour subtype %zu native mapping\n", i);
            return 1;
        }
    }
    for (i = 0; i < sizeof(junk_graphics) / sizeof(junk_graphics[0]); ++i) {
        if (csb_v1_viewport_f0115_object_native_graphic_pc34(
                THING_TYPE_JUNK, (int)i, 0) != junk_graphics[i]) {
            fprintf(stderr, "FAIL: junk subtype %zu native mapping\n", i);
            return 1;
        }
    }
    for (i = 0; i < sizeof(graphics) / sizeof(graphics[0]); ++i) {
        graphics[i] = (int)i + 498;
    }
    if (!path || !path[0]) {
        printf("SKIP: FIRESTAFF_CSB_GRAPHICS_DAT is not set\n");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (!M11_AssetLoader_Init(&loader, path)) {
        fprintf(stderr, "FAIL: could not load CSB GRAPHICS.DAT: %s\n", path);
        return 1;
    }
    for (i = 0; i < sizeof(graphics) / sizeof(graphics[0]); ++i) {
        const M11_AssetSlot *slot = M11_AssetLoader_Load(
            &loader, (unsigned int)graphics[i]);
        int drawn;
        if (!slot || !slot->pixels || !slot->width || !slot->height) {
            fprintf(stderr, "FAIL: missing first-object graphic %d\n", graphics[i]);
            M11_AssetLoader_Shutdown(&loader);
            return 1;
        }
        memset(screen, 0x5a, sizeof(screen));
        drawn = csb_v1_viewport_f0115_blit_first_object_native_family_pc34(
            graphics[i], slot->pixels, (int)slot->width, (int)slot->height,
            screen, 320, 200, 320, 96, 80, (int)slot->width,
            (int)slot->height, 2, 1);
        if (drawn <= 0) {
            fprintf(stderr, "FAIL: first-object graphic %d did not composite\n",
                    graphics[i]);
            M11_AssetLoader_Shutdown(&loader);
            return 1;
        }
    }
    M11_AssetLoader_Shutdown(&loader);
    printf("PASS: CSB F0115 native object graphics through armour and junk\n");
    return 0;
}
