#include "csb_v1_f0093_replacement_palette_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "ReDMCSB DUNVIEW.C F0093 lines 1996-2024: replacement set copies six "
    "dungeon-view palette rows plus D2/D3 remaps; DUNVIEW.C lines 2805-2815: "
    "restore slots 9/10 then walk CurrentMapAllowedCreatureTypes in map order; "
    "DEFS.H lines 1980-1984, 2013-2019: replacement-set shape and nibble selectors.";

/* ReDMCSB DUNVIEW.C G0219:1625-1651. F0093 must inspect every creature
 * permitted by the current map, not just the creature being drawn. */
static const CSB_V1_F0093CreatureAspectPc34 s_pc34_creature_aspects[] = {
    {0x01u}, {0x20u}, {0x00u}, {0x31u}, {0x34u}, {0x34u}, {0x00u},
    {0x00u}, {0x00u}, {0x00u}, {0x00u}, {0x00u}, {0x00u}, {0x20u},
    {0x30u}, {0x78u}, {0x65u}, {0x00u}, {0x00u}, {0xA9u}, {0x65u},
    {0xA9u}, {0xCBu}, {0x00u}, {0xCBu}, {0xCBu}, {0xCBu}
};

/* ReDMCSB DUNVIEW.C G0220:1705-1720 (MEDIA351 PC CSB table). */
static const CSB_V1_F0093ReplacementColorSetPc34 s_pc34_replacement_sets[] = {
    {{0x0CA0u,0x0A80u,0x0860u,0x0640u,0x0420u,0x0200u},9u,9u},
    {{0x0060u,0x0040u,0x0020u,0x0000u,0x0000u,0x0000u},0u,0u},
    {{0x0860u,0x0640u,0x0420u,0x0200u,0x0000u,0x0000u},10u,10u},
    {{0x0640u,0x0420u,0x0200u,0x0000u,0x0000u,0x0000u},9u,0u},
    {{0x000Au,0x0008u,0x0006u,0x0004u,0x0002u,0x0000u},9u,10u},
    {{0x0008u,0x0006u,0x0004u,0x0002u,0x0000u,0x0000u},10u,0u},
    {{0x0808u,0x0606u,0x0404u,0x0202u,0x0000u,0x0000u},9u,0u},
    {{0x0A0Au,0x0808u,0x0606u,0x0404u,0x0202u,0x0000u},10u,9u},
    {{0x0FA0u,0x0C80u,0x0A60u,0x0840u,0x0620u,0x0400u},10u,5u},
    {{0x0F80u,0x0C60u,0x0A40u,0x0820u,0x0600u,0x0200u},5u,7u},
    {{0x0800u,0x0600u,0x0400u,0x0200u,0x0000u,0x0000u},10u,12u},
    {{0x0600u,0x0400u,0x0200u,0x0000u,0x0000u,0x0000u},12u,0u},
    {{0x0C86u,0x0A64u,0x0842u,0x0620u,0x0400u,0x0200u},10u,5u}
};

int csb_v1_f0093_pc34_graphics558_receipt(
    CSB_V1_F0093Graphics558ReceiptPc34 *out_graphics)
{
    if (!out_graphics) return 0;
    out_graphics->creature_aspects = s_pc34_creature_aspects;
    out_graphics->creature_aspect_count = CSB_V1_F0093_CREATURE_COUNT_PC34;
    out_graphics->replacement_sets = s_pc34_replacement_sets;
    out_graphics->replacement_set_count = CSB_V1_F0093_REPLACEMENT_SET_COUNT_PC34;
    return 1;
}

static int assign_owner(CSB_V1_F0093PaletteOwnerPc34 *owner,
                        uint8_t creature_type,
                        uint8_t replacement_set_ordinal,
                        const CSB_V1_F0093Graphics558ReceiptPc34 *graphics)
{
    uint8_t set_index;

    if (replacement_set_ordinal == 0u) return 1;
    set_index = (uint8_t)(replacement_set_ordinal - 1u);
    if (set_index >= graphics->replacement_set_count) return 0;

    owner->assigned = 1;
    owner->source_creature_type = creature_type;
    owner->replacement_set_index = set_index;
    owner->values = graphics->replacement_sets[set_index];
    return 1;
}

int csb_v1_f0093_build_replacement_palette_receipt_pc34(
    const struct DungeonMapDesc_Compat *loaded_map,
    const CSB_V1_F0093Graphics558ReceiptPc34 *loaded_graphics,
    CSB_V1_F0093ReplacementPaletteReceiptPc34 *out_receipt)
{
    uint32_t i;

    if (!loaded_map || !loaded_graphics || !out_receipt ||
        !loaded_graphics->creature_aspects || !loaded_graphics->replacement_sets ||
        loaded_map->creatureTypeCount > sizeof(loaded_map->allowedCreatureTypes) ||
        loaded_graphics->creature_aspect_count != CSB_V1_F0093_CREATURE_COUNT_PC34 ||
        loaded_graphics->replacement_set_count != CSB_V1_F0093_REPLACEMENT_SET_COUNT_PC34) {
        return 0;
    }

    memset(out_receipt, 0, sizeof(*out_receipt));
    for (i = 0u; i < loaded_map->creatureTypeCount; ++i) {
        uint8_t creature_type = loaded_map->allowedCreatureTypes[i];
        uint8_t selectors;

        if (creature_type >= loaded_graphics->creature_aspect_count) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
        selectors = loaded_graphics->creature_aspects[creature_type]
                        .replacement_color_set_indices;
        if (!assign_owner(&out_receipt->palette_9, creature_type,
                          (uint8_t)(selectors & 0x0Fu), loaded_graphics) ||
            !assign_owner(&out_receipt->palette_10, creature_type,
                          (uint8_t)(selectors >> 4), loaded_graphics)) {
            memset(out_receipt, 0, sizeof(*out_receipt));
            return 0;
        }
    }
    return 1;
}

const char *csb_v1_f0093_replacement_palette_source_evidence_pc34(void)
{
    return s_source_evidence;
}
