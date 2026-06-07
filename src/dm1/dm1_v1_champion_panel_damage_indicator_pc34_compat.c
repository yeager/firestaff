#include "dm1/dm1_v1_champion_panel_damage_indicator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static const DM1_V1_ChampionPanelDamageIndicatorEvidencePc34Compat s_evidence = {
    true,
    "CHAMDRAW.C F0623_DrawDamageToChampion_F0320_sub:680-699",
    "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:1720-1779",
    "DEFS.H:2176-2177 C015/C016 damage graphics",
    "DEFS.H:2086-2093 C08/C10/C15 colors",
    "DEFS.H:3792-3794 C167/C179 damage zones",
    "DEFS.H:7880-7886,8039-8043,9067-9070 prototypes",
    "BASE.C F0660_:1473-1507 bitmap-index-to-zone transparent blit",
    "PANEL.C F0355:2299-2316 G0423 inventory champion ordinal maintenance",
    "COMPILE.H:1038 M000_INDEX_TO_ORDINAL(value) ((value) + 1)",
    "contract-only PC34 F0623 damage-indicator route; no bitmap sampling",
    "without claiming real-asset parity"
};

static const char s_source_evidence[] =
    "contract_only=1; CHAMDRAW.C F0623:688-694 selects C016/C179 when "
    "M000_INDEX_TO_ORDINAL(championIndex) equals G0423 inventory ordinal, "
    "else C015/C167; F0623:695-699 brackets F0660, F0650, F0292 with "
    "F0077/F0078 mouse screen-update calls; F0623:696 adds championIndex "
    "to the selected first damage zone and uses C10 transparency; F0623:697 "
    "prints F0288(damage,C0_FALSE,3) centered with C15 on C08; "
    "CHAMPION.C F0320:1720-1779 calls F0623 after nonlethal pending damage; "
    "DEFS.H anchors graphics/colors/zones/prototypes; BASE.C F0660:1473-1507 "
    "anchors the bitmap-index-to-zone transparent blit; PANEL.C F0355 "
    "anchors G0423 inventory champion ordinal maintenance; without claiming "
    "real-asset parity.";

const DM1_V1_ChampionPanelDamageIndicatorEvidencePc34Compat *
DM1_V1_ChampionPanelDamageIndicator_EvidencePc34Compat(void)
{
    return &s_evidence;
}

const char *
DM1_V1_ChampionPanelDamageIndicator_SourceEvidencePc34Compat(void)
{
    return s_source_evidence;
}

void DM1_V1_ChampionPanelDamageIndicator_DefaultInputPc34Compat(
    DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat *input)
{
    if (!input) {
        return;
    }

    memset(input, 0, sizeof(*input));
    input->champion_index = 0;
    input->inventory_champion_ordinal = 0;
    input->damage = 1;
}

static void format_damage_text(int damage, char out_text[8])
{
    /*
     * ReDMCSB CHAMDRAW.C F0288:374-392 returns the integer digits when
     * padding is C0_FALSE; F0623:697 always passes C0_FALSE and width 3.
     */
    snprintf(out_text, 8, "%d", damage);
}

int DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(
    const DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat *input,
    DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat *out_result)
{
    DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat local_input;

    if (!out_result) {
        return 0;
    }

    memset(out_result, 0, sizeof(*out_result));
    out_result->contract_only = true;
    out_result->evidence = &s_evidence;

    if (!input) {
        DM1_V1_ChampionPanelDamageIndicator_DefaultInputPc34Compat(&local_input);
        input = &local_input;
    }

    if (input->champion_index < 0 ||
        input->champion_index >= DM1_V1_CPDI_CHAMPION_COUNT_PC34) {
        /*
         * CHAMPION.C F0320:1720-1721 iterates only active party champion
         * indices, which are bounded by the four champion panel cells.
         */
        out_result->rejected_champion_index = true;
        return 0;
    }

    if (input->damage < 0 ||
        input->damage > DM1_V1_CPDI_DAMAGE_MAX_PC34) {
        /*
         * CHAMDRAW.C F0623:682 receives int16 damage and F0288:354-361
         * formats an unsigned 16-bit integer; this synthetic gate accepts
         * only nonnegative int16 values to avoid modeling signed wraparound.
         */
        out_result->rejected_damage = true;
        return 0;
    }

    out_result->valid = true;
    out_result->champion_index = input->champion_index;
    out_result->champion_ordinal = input->champion_index + 1;
    out_result->inventory_champion_ordinal = input->inventory_champion_ordinal;
    out_result->damage = input->damage;
    out_result->is_inventory_champion =
        (out_result->champion_ordinal == input->inventory_champion_ordinal);

    /*
     * CHAMDRAW.C F0623:688-694: inventory champion uses the big C016
     * damage graphic and C179 first zone; other champions use C015/C167.
     */
    if (out_result->is_inventory_champion) {
        out_result->graphic_index = DM1_V1_CPDI_GFX_DAMAGE_BIG_PC34;
        out_result->base_zone_index = DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34;
    } else {
        out_result->graphic_index = DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34;
        out_result->base_zone_index = DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34;
    }

    /*
     * CHAMDRAW.C F0623:696 mutates L2257_i_ZoneIndex by adding
     * championIndex before passing it to F0660_ with C10 transparency.
     */
    out_result->zone_index =
        out_result->base_zone_index + out_result->champion_index;
    out_result->transparent_color = DM1_V1_CPDI_COLOR_TRANSPARENT_FLESH_PC34;

    /*
     * CHAMDRAW.C F0623:697 prints centered text in the same zone with C15
     * foreground, C08 background, and F0288(damage, C0_FALSE, 3).
     */
    out_result->text_color = DM1_V1_CPDI_COLOR_TEXT_WHITE_PC34;
    out_result->text_background_color = DM1_V1_CPDI_COLOR_TEXT_RED_PC34;
    out_result->integer_padding_enabled = DM1_V1_CPDI_FORMAT_PADDING_OFF_PC34;
    out_result->integer_padding_width = DM1_V1_CPDI_FORMAT_WIDTH_PC34;
    format_damage_text(out_result->damage, out_result->damage_text);

    /*
     * CHAMDRAW.C F0623:695-699 call order:
     * F0077, F0660, F0650, F0292, F0078.
     */
    out_result->operations[0] = DM1_V1_CPDI_OP_ENABLE_MOUSE_UPDATE_PC34;
    out_result->operations[1] = DM1_V1_CPDI_OP_BLIT_DAMAGE_GRAPHIC_PC34;
    out_result->operations[2] = DM1_V1_CPDI_OP_PRINT_CENTERED_TEXT_PC34;
    out_result->operations[3] = DM1_V1_CPDI_OP_REDRAW_CHAMPION_STATE_PC34;
    out_result->operations[4] = DM1_V1_CPDI_OP_DISABLE_MOUSE_UPDATE_PC34;
    out_result->operation_count = 5;
    out_result->redraw_champion_index = out_result->champion_index;

    return 1;
}
