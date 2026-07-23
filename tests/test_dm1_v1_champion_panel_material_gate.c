#include "asset_loader_m11.h"
#include "dm1_v1_champion_panel_material_pc34_compat.h"
#include "font_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char path[1024];
    const char *root = getenv("FIRESTAFF_DM1_DATA_DIR");
    const char *home = getenv("HOME");
    M11_AssetLoader loader;
    M11_FontState font;
    Dm1V1ChampionPanelMaterialReceiptPc34 receipt;
    if (root && root[0]) snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", root);
    else if (home && home[0]) snprintf(path, sizeof(path), "%s/.firestaff/data/dm1/GRAPHICS.DAT", home);
    else return 0;
    if (!M11_AssetLoader_Init(&loader, path)) {
        if (root && root[0]) return 1;
        puts("SKIP: PC34 GRAPHICS.DAT not installed");
        return 0;
    }
    M11_Font_Init(&font);
    if (!M11_Font_LoadFromGraphicsDat(&font, loader.fileState, loader.runtimeState) ||
        !dm1_v1_champion_panel_material_from_m11_loader_pc34(
            &loader, &font, &G9010_auc_VgaPaletteBrightest_Compat[0][0], 16,
            &receipt) || !receipt.valid || !receipt.suppressSyntheticFallback ||
        receipt.materialFingerprint == 0u) {
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    if (dm1_v1_champion_panel_material_from_m11_loader_pc34(
            &loader, &font, &G9010_auc_VgaPaletteBrightest_Compat[0][0], 15,
            &receipt)) {
        M11_AssetLoader_Shutdown(&loader);
        return 1;
    }
    M11_AssetLoader_Shutdown(&loader);
    puts("ok: real PC34 F0292/F0293/F0296/F0302 panel material gate");
    return 0;
}
