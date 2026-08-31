#include "csb_v1_f0093_replacement_palette_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int expect(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        return 0;
    }
    return 1;
}

int main(void)
{
    CSB_V1_F0093CreatureAspectPc34 aspects[CSB_V1_F0093_CREATURE_COUNT_PC34];
    CSB_V1_F0093ReplacementColorSetPc34 sets[CSB_V1_F0093_REPLACEMENT_SET_COUNT_PC34];
    CSB_V1_F0093Graphics558ReceiptPc34 graphics;
    CSB_V1_F0093ReplacementPaletteReceiptPc34 receipt;
    struct DungeonMapDesc_Compat map;
    int ok = 1;

    memset(aspects, 0, sizeof(aspects));
    memset(sets, 0, sizeof(sets));
    memset(&map, 0, sizeof(map));
    memset(&graphics, 0, sizeof(graphics));

    /* ReDMCSB DUNVIEW.C G0219:1630-1632: Wizard Eye=0x31,
     * Pain Rat=0x34, Ruster=0x34. G0220:1707-1719 supplies the data.
     * The fixture has only the source-backed fields F0093 receives. */
    aspects[3].replacement_color_set_indices = 0x31u;
    aspects[4].replacement_color_set_indices = 0x34u;
    aspects[5].replacement_color_set_indices = 0x34u;
    sets[0].dungeon_view_rgb[0] = 0x0CA0u;
    sets[0].d2_replacement_color = 9u;
    sets[0].d3_replacement_color = 9u;
    sets[2].dungeon_view_rgb[0] = 0x0640u;
    sets[2].d2_replacement_color = 9u;
    sets[2].d3_replacement_color = 0u;
    sets[3].dungeon_view_rgb[0] = 0x000Au;
    sets[3].d2_replacement_color = 9u;
    sets[3].d3_replacement_color = 10u;
    graphics.creature_aspects = aspects;
    graphics.creature_aspect_count = CSB_V1_F0093_CREATURE_COUNT_PC34;
    graphics.replacement_sets = sets;
    graphics.replacement_set_count = CSB_V1_F0093_REPLACEMENT_SET_COUNT_PC34;

    map.creatureTypeCount = 3u;
    map.allowedCreatureTypes[0] = 3u;
    map.allowedCreatureTypes[1] = 4u;
    map.allowedCreatureTypes[2] = 5u;
    ok &= expect(csb_v1_f0093_build_replacement_palette_receipt_pc34(
                     &map, &graphics, &receipt),
                 "map-ordered receipt accepted");
    ok &= expect(receipt.palette_9.assigned &&
                     receipt.palette_9.source_creature_type == 5u &&
                     receipt.palette_9.replacement_set_index == 3u &&
                     receipt.palette_9.values.d3_replacement_color == 10u,
                 "last allowed creature owns palette 9");
    ok &= expect(receipt.palette_10.assigned &&
                     receipt.palette_10.source_creature_type == 5u &&
                     receipt.palette_10.replacement_set_index == 2u &&
                     receipt.palette_10.values.dungeon_view_rgb[0] == 0x0640u,
                 "last allowed creature owns palette 10");

    {
        uint8_t palette[16];
        int index;
        for (index = 0; index < 16; ++index) palette[index] = (uint8_t)index;
        /* D3 consumes G0221: the same final F0093 owners must reach the
         * live indexed creature-palette adapter, including a valid zero. */
        ok &= expect(csb_v1_f0093_apply_replacement_palette_pc34(
                         &map, 2, palette) &&
                         palette[9] == 0u && palette[10] == 10u,
                     "D3 adapter consumes F0093 final owners without rounding or fallback");
    }

    map.creatureTypeCount = 1u;
    map.allowedCreatureTypes[0] = 2u;
    ok &= expect(csb_v1_f0093_build_replacement_palette_receipt_pc34(
                     &map, &graphics, &receipt) &&
                     !receipt.palette_9.assigned && !receipt.palette_10.assigned,
                 "no selector leaves base palette outside this receipt");

    {
        uint8_t palette[16];
        int index;
        for (index = 0; index < 16; ++index) palette[index] = (uint8_t)(15 - index);
        /* F0093 restores 9 through set 8 and 10 through set 12 before it
         * sees an unowned map entry.  The real PC table maps both to 10 in
         * D2 and 5 in D3; neither may retain the preceding sprite remap. */
        ok &= expect(csb_v1_f0093_apply_replacement_palette_pc34(
                         &map, 1, palette) &&
                         palette[9] == 10u && palette[10] == 10u,
                     "D2 unowned slots restore F0093 defaults");
        ok &= expect(csb_v1_f0093_apply_replacement_palette_pc34(
                         &map, 2, palette) &&
                         palette[9] == 5u && palette[10] == 5u,
                     "D3 unowned slots restore F0093 defaults");
    }

    {
        uint8_t palette[16];
        int index;
        for (index = 0; index < 16; ++index) palette[index] = (uint8_t)index;
        /* ReDMCSB DUNVIEW.C:1733 documents Atari's distinct D3 target for
         * set 9: 3, not the PC table's 7.  Creature 19 (0xA9) owns slot 10
         * through that set after F0093's ordered walk. */
        map.creatureTypeCount = 1u;
        map.allowedCreatureTypes[0] = 19u;
        ok &= expect(csb_v1_f0093_apply_replacement_palette_for_profile_pc34(
                         &map, 2, CSB_V1_F0093_PALETTE_PROFILE_ATARI_ST,
                         palette) && palette[9] == 5u && palette[10] == 3u,
                     "Atari D3 uses its source table for final map owner");
        for (index = 0; index < 16; ++index) palette[index] = (uint8_t)index;
        ok &= expect(csb_v1_f0093_apply_replacement_palette_for_profile_pc34(
                         &map, 2,
                         CSB_V1_F0093_PALETTE_PROFILE_VERSION3_F0695,
                         palette) && palette[9] == 5u && palette[10] == 3u,
                     "Amiga/FM Towns F0695 D3 uses its source table for final map owner");
    }

    aspects[3].replacement_color_set_indices = 0xF1u;
    map.allowedCreatureTypes[0] = 3u;
    ok &= expect(!csb_v1_f0093_build_replacement_palette_receipt_pc34(
                      &map, &graphics, &receipt) &&
                     !receipt.palette_9.assigned && !receipt.palette_10.assigned,
                 "invalid loaded selector fails closed");

    return ok ? 0 : 1;
}
