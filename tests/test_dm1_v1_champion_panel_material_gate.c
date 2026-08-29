#include "asset_loader_m11.h"
#include "dm1_v1_champion_panel_material_pc34_compat.h"
#include "font_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    char graphics_path[2048];
    const char *archive = getenv("FIRESTAFF_DM1_DOS_PC34_ARCHIVE");
    M11_AssetLoader loader;
    M11_FontState font;
    Dm1V1ChampionPanelMaterialReceiptPc34 receipt;
    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_DM1_DOS_PC34_ARCHIVE is not selected");
        return 77;
    }
    snprintf(graphics_path, sizeof(graphics_path),
             "%s::dungeon-master/dmaster/DATA/GRAPHICS.DAT", archive);
    if (!M11_AssetLoader_Init(&loader, graphics_path)) {
        fputs("original PC34 ZIP GRAPHICS.DAT is unavailable\n", stderr);
        return 1;
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
    puts("ok: original PC34 ZIP F0292/F0293/F0296/F0302 panel material gate");
    return 0;
}
