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
    int graphic_index;

    if (!path || !path[0]) {
        printf("SKIP: FIRESTAFF_CSB_GRAPHICS_DAT is not set\n");
        return 0;
    }
    memset(&loader, 0, sizeof(loader));
    if (!M11_AssetLoader_Init(&loader, path)) {
        fprintf(stderr, "FAIL: could not load CSB GRAPHICS.DAT: %s\n", path);
        return 1;
    }
    for (graphic_index = 454; graphic_index <= 464; ++graphic_index) {
        if (graphic_index == 456 || graphic_index == 459) continue;
        const M11_AssetSlot *slot = M11_AssetLoader_Load(&loader,
                                                           (unsigned int)graphic_index);
        int drawn;
        if (!slot || !slot->pixels || slot->width == 0 || slot->height == 0) {
            fprintf(stderr, "FAIL: missing M715/M716/M717/M718 graphic %d\n", graphic_index);
            M11_AssetLoader_Shutdown(&loader);
            return 1;
        }
        memset(screen, 0x5a, sizeof(screen));
        drawn = csb_v1_viewport_f0115_blit_m715_m716_m717_m718_projectile_family_pc34(
            graphic_index, slot->pixels, (int)slot->width, (int)slot->height,
            screen, 320, 200, 320, 96, 80, (int)slot->width,
            (int)slot->height, 1, 0);
        if (drawn <= 0) {
            fprintf(stderr, "FAIL: M715/M716/M717/M718 graphic %d did not composite\n",
                    graphic_index);
            M11_AssetLoader_Shutdown(&loader);
            return 1;
        }
    }
    if (csb_v1_viewport_f0115_blit_m715_m716_m717_m718_projectile_family_pc34(
            465, (const uint8_t *)"\x01", 1, 1, screen, 320, 200, 320,
            96, 80, 1, 1, 1, 0) != 0) {
        fprintf(stderr, "FAIL: unproven GRAPHICS.DAT 465 must remain blocked\n");
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    M11_AssetLoader_Shutdown(&loader);
    printf("PASS: CSB F0115 M715/M716/M717/M718 real-asset graphics 454/455/457/458/460-464\n");
    return 0;
}
