#include "asset_loader_m11.h"
#include "csb_v1_viewport_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int test_synthetic_contract(void)
{
    const uint8_t source[4] = {13, 1, 2, 3};
    const uint8_t slime_source[4] = {11, 1, 2, 3};
    const uint8_t giggler_source[4] = {11, 1, 2, 3};
    const uint8_t screamer_source[4] = {13, 1, 2, 3};
    const uint8_t rockpile_source[4] = {4, 5, 8, 9};
    const uint8_t ghost_source[4] = {4, 5, 8, 9};
    const uint8_t wizard_eye_source[4] = {4, 9, 10, 1};
    struct DungeonMapDesc_Compat map;
    uint8_t screen[320 * 200];

    memset(&map, 0, sizeof(map));
    map.creatureTypeCount = 3u;
    map.allowedCreatureTypes[0] = 3u;
    map.allowedCreatureTypes[1] = 4u;
    map.allowedCreatureTypes[2] = 5u;
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            0, 584, source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 0x8000 | 3280, 0) != 3 ||
        screen[33 * 320 + 1] != 0x5a || screen[33 * 320 + 2] != 1 ||
        screen[34 * 320 + 1] != 2 || screen[34 * 320 + 2] != 3) {
        fprintf(stderr, "FAIL: Giant Scorpion D2 material/palette/transparency route\n");
        return 0;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            6, 603, screamer_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 3, 0x8000 | 3280, 0) != 3 ||
        screen[33 * 320 + 1] != 0x5a || screen[33 * 320 + 2] != 12 ||
        screen[34 * 320 + 1] != 1 || screen[34 * 320 + 2] != 3 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(3) != 594 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(4) != 596 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(5) != 600 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            6, 590, screamer_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 3, 3280, 0) != 0) {
        fprintf(stderr, "FAIL: Screamer D3/aspect/palette/fail-closed contract\n");
        return 0;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            1, 588, slime_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 3, 3280, 0) != 3 ||
        screen[33 * 320 + 2] != 12 || screen[34 * 320 + 1] != 1 ||
        screen[33 * 320 + 1] != 0x5a ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            1, 584, source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 3280, 0) != 0 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            0, 584, source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 2500, 0) != 0 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(3) != 594) {
        fprintf(stderr, "FAIL: Swamp Slime D3/aspect/fail-closed occlusion contract\n");
        return 0;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            2, 590, giggler_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 0x8000 | 3280, 0) != 3 ||
        screen[33 * 320 + 1] != 0x5a || screen[33 * 320 + 2] != 1 ||
        screen[34 * 320 + 1] != 2 || screen[34 * 320 + 2] != 3 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            2, 588, giggler_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 3280, 0) != 0 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(3) != 594) {
        fprintf(stderr, "FAIL: Giggler D2/aspect/fail-closed occlusion contract\n");
        return 0;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            7, 605, rockpile_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 3, 0x8000 | 3280, 0) != 3 ||
        screen[33 * 320 + 1] != 0x5a || screen[33 * 320 + 2] != 3 ||
        screen[34 * 320 + 1] != 3 || screen[34 * 320 + 2] != 0 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            7, 605, rockpile_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 3280, 0) != 3 ||
        screen[33 * 320 + 2] != 3 || screen[34 * 320 + 1] != 5 ||
        screen[34 * 320 + 2] != 0 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            7, 603, rockpile_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 3280, 0) != 0 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            7, 605, rockpile_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 2500, 0) != 0 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(7) != 605) {
        fprintf(stderr, "FAIL: Rockpile D3/aspect/fail-closed occlusion contract\n");
        return 0;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            8, 607, ghost_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 3, 0x8000 | 3280, 0) != 3 ||
        screen[33 * 320 + 1] != 0x5a || screen[33 * 320 + 2] != 3 ||
        screen[34 * 320 + 1] != 3 || screen[34 * 320 + 2] != 0 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            8, 605, ghost_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 3280, 0) != 0 ||
        csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            8, 607, ghost_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 2500, 0) != 0 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(8) != 607 ||
        csb_v1_viewport_f0115_native_group_front_graphic_pc34(9) != -1) {
        fprintf(stderr, "FAIL: Ghost D3/aspect/fail-closed occlusion contract\n");
        return 0;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_f0093_group_front_family_pc34(
            3, &map, wizard_eye_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 3280, 0) != 3 ||
        screen[33 * 320 + 1] != 0x5a || screen[33 * 320 + 2] != 9 ||
        screen[34 * 320 + 1] != 10 || screen[34 * 320 + 2] != 1) {
        fprintf(stderr, "FAIL: Wizard Eye F0093 D2 map-order/remap/transparency route\n");
        return 0;
    }
    map.creatureTypeCount = 17u;
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_f0093_group_front_family_pc34(
            3, &map, wizard_eye_source, 2, 2, screen, 320, 200, 320,
            1, 33, 2, 2, 2, 3280, 0) != 0 ||
        screen[33 * 320 + 2] != 0x5a) {
        fprintf(stderr, "FAIL: incomplete F0093 map receipt must fail closed\n");
        return 0;
    }
    return 1;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_GRAPHICS_DAT");
    M11_AssetLoader loader;
    uint8_t screen[320 * 200];
    const M11_AssetSlot *slot;

    if (!test_synthetic_contract()) return 1;
    if (!path || !path[0]) {
        printf("SKIP: FIRESTAFF_CSB_GRAPHICS_DAT is not set (synthetic contract passed)\n");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (!M11_AssetLoader_Init(&loader, path)) {
        fprintf(stderr, "FAIL: could not load CSB GRAPHICS.DAT: %s\n", path);
        return 1;
    }
    slot = M11_AssetLoader_Load(&loader, 584u);
    if (!slot || !slot->pixels || slot->width == 0 || slot->height == 0) {
        fprintf(stderr, "FAIL: missing Giant Scorpion GRAPHICS.DAT 584\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            0, 584, slot->pixels, (int)slot->width, (int)slot->height,
            screen, 320, 200, 320, 96, 80, (int)slot->width,
            (int)slot->height, 2, 3280, 0) <= 0) {
        fprintf(stderr, "FAIL: Giant Scorpion GRAPHICS.DAT 584 did not composite\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    slot = M11_AssetLoader_Load(&loader, 590u);
    if (!slot || !slot->pixels || slot->width == 0 || slot->height == 0) {
        fprintf(stderr, "FAIL: missing Giggler GRAPHICS.DAT 590\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            2, 590, slot->pixels, (int)slot->width, (int)slot->height,
            screen, 320, 200, 320, 96, 80, (int)slot->width,
            (int)slot->height, 2, 3280, 0) <= 0) {
        fprintf(stderr, "FAIL: Giggler GRAPHICS.DAT 590 did not composite\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    slot = M11_AssetLoader_Load(&loader, 603u);
    if (!slot || !slot->pixels || slot->width == 0 || slot->height == 0) {
        fprintf(stderr, "FAIL: missing Screamer GRAPHICS.DAT 603\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            6, 603, slot->pixels, (int)slot->width, (int)slot->height,
            screen, 320, 200, 320, 96, 80, (int)slot->width,
            (int)slot->height, 2, 3280, 0) <= 0) {
        fprintf(stderr, "FAIL: Screamer GRAPHICS.DAT 603 did not composite\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    slot = M11_AssetLoader_Load(&loader, 605u);
    if (!slot || !slot->pixels || slot->width == 0 || slot->height == 0) {
        fprintf(stderr, "FAIL: missing Rockpile GRAPHICS.DAT 605\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            7, 605, slot->pixels, (int)slot->width, (int)slot->height,
            screen, 320, 200, 320, 96, 80, (int)slot->width,
            (int)slot->height, 2, 3280, 0) <= 0) {
        fprintf(stderr, "FAIL: Rockpile GRAPHICS.DAT 605 did not composite\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    slot = M11_AssetLoader_Load(&loader, 607u);
    if (!slot || !slot->pixels || slot->width == 0 || slot->height == 0) {
        fprintf(stderr, "FAIL: missing Ghost GRAPHICS.DAT 607\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    memset(screen, 0x5a, sizeof(screen));
    if (csb_v1_viewport_f0115_blit_native_group_front_family_pc34(
            8, 607, slot->pixels, (int)slot->width, (int)slot->height,
            screen, 320, 200, 320, 96, 80, (int)slot->width,
            (int)slot->height, 2, 3280, 0) <= 0) {
        fprintf(stderr, "FAIL: Ghost GRAPHICS.DAT 607 did not composite\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    M11_AssetLoader_Shutdown(&loader);
    printf("PASS: CSB F0115 Scorpion/Slime/Giggler/Screamer/Rockpile/Ghost group material\n");
    return 0;
}
