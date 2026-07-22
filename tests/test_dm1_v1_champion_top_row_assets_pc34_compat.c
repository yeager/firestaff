#include "dm1_v1_champion_top_row_assets_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static unsigned char pixels[76 * 29];

static M11_AssetSlot slot(unsigned int graphic, unsigned short width, unsigned short height)
{
    M11_AssetSlot out;
    memset(&out, 0, sizeof(out));
    out.loaded = 1;
    out.graphicIndex = graphic;
    out.width = width;
    out.height = height;
    out.pixels = pixels;
    return out;
}

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

int main(void)
{
    M11_AssetSlot c008 = slot(DM1_GFX_DEAD_CHAMPION, 67, 29);
    M11_AssetSlot c028 = slot(DM1_GFX_CHAMPION_ICONS, 76, 14);
    M11_AssetSlot c033 = slot(DM1_GFX_SLOT_NORMAL, 18, 18);
    M11_AssetSlot c034 = slot(DM1_GFX_SLOT_WOUNDED, 18, 18);
    M11_AssetSlot c035 = slot(DM1_GFX_SLOT_ACTING, 18, 18);
    Dm1V1ChampionTopRowAssetsReceiptPc34 receipt;
    int ok = 1;

    ok &= check("complete receipt", dm1_v1_champion_top_row_assets_from_slots_pc34(
        &c008, &c028, &c033, &c034, &c035, &receipt));
    ok &= check("all source surfaces accepted", receipt.valid && receipt.c008Accepted &&
                receipt.c028Accepted && receipt.c033Accepted && receipt.c034Accepted &&
                receipt.c035Accepted);
    ok &= check("C028 retained source bytes", receipt.assets.championIcons.pixels == pixels &&
                receipt.assets.championIcons.width == 76);

    c034.height = 17;
    ok &= check("wrong wounded slot geometry rejects receipt",
                !dm1_v1_champion_top_row_assets_from_slots_pc34(
                    &c008, &c028, &c033, &c034, &c035, &receipt) && !receipt.c034Accepted);
    c034.height = 18;
    c028.pixels = NULL;
    ok &= check("missing C028 bytes rejects receipt",
                !dm1_v1_champion_top_row_assets_from_slots_pc34(
                    &c008, &c028, &c033, &c034, &c035, &receipt) && !receipt.c028Accepted);
    ok &= check("uninitialized loader rejects receipt",
                !dm1_v1_champion_top_row_assets_from_m11_loader_pc34(NULL, &receipt));
    return ok ? 0 : 1;
}
