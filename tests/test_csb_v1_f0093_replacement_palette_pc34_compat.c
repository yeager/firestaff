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

    map.creatureTypeCount = 1u;
    map.allowedCreatureTypes[0] = 2u;
    ok &= expect(csb_v1_f0093_build_replacement_palette_receipt_pc34(
                     &map, &graphics, &receipt) &&
                     !receipt.palette_9.assigned && !receipt.palette_10.assigned,
                 "no selector leaves base palette outside this receipt");

    aspects[3].replacement_color_set_indices = 0xF1u;
    map.allowedCreatureTypes[0] = 3u;
    ok &= expect(!csb_v1_f0093_build_replacement_palette_receipt_pc34(
                      &map, &graphics, &receipt) &&
                     !receipt.palette_9.assigned && !receipt.palette_10.assigned,
                 "invalid loaded selector fails closed");

    return ok ? 0 : 1;
}
