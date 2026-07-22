#include "dm1_v1_champion_top_row_frame_pc34_compat.h"

#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const unsigned char pixels[76 * 18];

static Dm1V1ChampionTopRowSurfacePc34 surface(int graphic, int width, int height)
{
    Dm1V1ChampionTopRowSurfacePc34 out;
    out.graphicIndex = graphic;
    out.loaded = 1;
    out.pixels = pixels;
    out.width = width;
    out.height = height;
    return out;
}

static int check(const char *label, int value)
{
    if (!value) fprintf(stderr, "FAIL %s\n", label);
    return value;
}

int main(void)
{
    struct PartyState_Compat party;
    Dm1V1ChampionTopRowAssetsPc34 assets;
    Dm1V1ChampionTopRowFramePc34 frame;
    int ok = 1;

    memset(&party, 0, sizeof(party));
    party.championCount = 4;
    party.activeChampionIndex = 1;
    party.direction = 3;
    for (int i = 0; i < 4; ++i) {
        party.champions[i].present = 1;
        party.champions[i].direction = (unsigned char)i;
        party.champions[i].hp.current = (unsigned short)(100 - i * 20);
        party.champions[i].hp.maximum = 100;
        party.champions[i].stamina.current = (unsigned short)(80 - i * 10);
        party.champions[i].stamina.maximum = 80;
        party.champions[i].mana.current = (unsigned short)(60 - i * 10);
        party.champions[i].mana.maximum = 60;
    }
    party.champions[2].wounds = 1u << DM1_SLOT_READY_HAND;
    party.champions[3].hp.current = 0;

    assets.deadStatusBox = surface(DM1_GFX_DEAD_CHAMPION, 67, 29);
    assets.championIcons = surface(DM1_GFX_CHAMPION_ICONS, 76, 14);
    assets.slotNormal = surface(DM1_GFX_SLOT_NORMAL, 18, 18);
    assets.slotWounded = surface(DM1_GFX_SLOT_WOUNDED, 18, 18);
    assets.slotActing = surface(DM1_GFX_SLOT_ACTING, 18, 18);

    ok &= check("source-bound live party frame", dm1_v1_champion_top_row_frame_from_party_pc34(
        &party, 2, 0, &assets, &frame));
    ok &= check("frame valid", frame.valid && frame.partyChampionCount == 4);
    ok &= check("slot one leader name", frame.slots[1].nameColor == DM1_COLOR_YELLOW);
    ok &= check("slot two wounded ready hand", frame.slots[2].hands[0].graphicIndex == DM1_GFX_SLOT_WOUNDED);
    ok &= check("acting action hand", frame.slots[1].hands[1].graphicIndex == DM1_GFX_SLOT_ACTING);
    ok &= check("dead slot source surface", !frame.slots[3].alive &&
                frame.slots[3].stats[0].zoneId == 0);
    ok &= check("bar geometry", frame.slots[0].stats[0].zoneId == 195 &&
                frame.slots[0].stats[0].x == 46 && frame.slots[0].stats[0].height == 25);

    assets.slotActing.loaded = 0;
    ok &= check("missing selected source surface rejects frame",
                !dm1_v1_champion_top_row_frame_from_party_pc34(
                    &party, 2, 0, &assets, &frame));
    assets.slotActing.loaded = 1;
    assets.deadStatusBox.width = 66;
    ok &= check("wrong C008 geometry rejects dead frame",
                !dm1_v1_champion_top_row_frame_from_party_pc34(
                    &party, 2, 0, &assets, &frame));

    return ok ? 0 : 1;
}
