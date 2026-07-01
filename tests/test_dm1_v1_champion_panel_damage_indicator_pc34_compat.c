#include "dm1/dm1_v1_champion_panel_damage_indicator_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions = 0;
static int g_failures = 0;

static void expect_int(const char *id, int got, int want, const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        printf("FAIL %s got=%d want=%d at %s\n", id, got, want, anchor);
        ++g_failures;
    } else {
        printf("PASS %s == %d (%s)\n", id, want, anchor);
    }
}

static void expect_bool(const char *id, bool got, bool want, const char *anchor)
{
    expect_int(id, got ? 1 : 0, want ? 1 : 0, anchor);
}

static void expect_str_eq(const char *id, const char *got, const char *want,
                          const char *anchor)
{
    ++g_assertions;
    if (!got || !want || strcmp(got, want) != 0) {
        printf("FAIL %s got=\"%s\" want=\"%s\" at %s\n",
               id, got ? got : "(null)", want ? want : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s == \"%s\" (%s)\n", id, want, anchor);
    }
}

static void expect_contains(const char *id, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == NULL) {
        printf("FAIL %s missing \"%s\" at %s\n",
               id, needle ? needle : "(null)", anchor);
        ++g_failures;
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", id, needle, anchor);
    }
}

static void test_evidence(void)
{
    const DM1_V1_ChampionPanelDamageIndicatorEvidencePc34Compat *evidence =
        DM1_V1_ChampionPanelDamageIndicator_EvidencePc34Compat();
    const char *source =
        DM1_V1_ChampionPanelDamageIndicator_SourceEvidencePc34Compat();

    expect_bool("evidence.contract_only", evidence->contract_only, true,
                "CHAMDRAW.C F0623:680-699 contract-only route");
    expect_str_eq("evidence.draw_function", evidence->draw_function_anchor,
                  "CHAMDRAW.C F0623_DrawDamageToChampion_F0320_sub:680-699",
                  "CHAMDRAW.C F0623:680-699");
    expect_str_eq("evidence.pipeline_caller", evidence->pipeline_caller_anchor,
                  "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:1744-1775 PC34 MEDIA009 box + 1/2/3-digit x-stride",
                  "CHAMPION.C F0320:1744-1775 PC34 MEDIA009 damage box");
    expect_contains("evidence.graphics", evidence->defs_graphics_anchor,
                    "C015/C016", "DEFS.H:2176-2177");
    expect_contains("evidence.colors", evidence->defs_colors_anchor,
                    "C08/C10/C15", "DEFS.H:2086-2093");
    expect_contains("evidence.zones", evidence->defs_zones_anchor,
                    "C167/C179", "DEFS.H:3792-3794");
    expect_contains("evidence.prototypes", evidence->defs_prototype_anchor,
                    "C69_CHAMPION_STATUS_BOX_SPACING", "DEFS.H:2157 champion status box spacing");
    expect_contains("evidence.base_blit", evidence->base_blit_anchor,
                    "F0660_", "BASE.C:1473-1507");
    expect_contains("evidence.panel_inventory", evidence->panel_inventory_anchor,
                    "G0423", "PANEL.C F0355:2299-2316");
    expect_contains("evidence.ordinal", evidence->ordinal_macro_anchor,
                    "M000_INDEX_TO_ORDINAL", "COMPILE.H:1038");
    expect_contains("evidence.scope", evidence->scope_note,
                    "contract-only", "CHAMDRAW.C F0623:680-699 bounded scope");
    expect_contains("evidence.no_real_asset_claim", evidence->no_real_asset_claim,
                    "without claiming real-asset parity",
                    "contract-only no original DOS parity claim");
    expect_contains("source.contract_only", source, "contract_only=1",
                    "CHAMDRAW.C F0623:680-699 source evidence");
    expect_contains("source.big_branch", source, "C016/C179",
                    "CHAMDRAW.C F0623:688-690");
    expect_contains("source.small_branch", source, "C015/C167",
                    "CHAMDRAW.C F0623:691-693");
    expect_contains("source.zone_add", source, "adds championIndex",
                    "CHAMDRAW.C F0623:696");
    expect_contains("source.text", source, "F0288(damage,C0_FALSE,3)",
                    "CHAMDRAW.C F0623:697");
    expect_contains("source.pipeline", source, "CHAMPION.C F0320:1720-1779",
                    "CHAMPION.C F0320 caller");
    expect_contains("source.f0320_pc34_media009",
                    source, "F0320:1744-1775 PC34 MEDIA009",
                    "F0320 PC34 MEDIA009 branch range");
    expect_contains("source.base", source, "BASE.C F0660:1473-1507",
                    "BASE.C F0660_ blit helper");
    expect_contains("source.panel", source, "PANEL.C F0355",
                    "PANEL.C inventory ordinal route");
    expect_contains("source.no_real_asset", source,
                    "without claiming real-asset parity",
                    "contract-only marker");
    /*
     * F0320 PC34 MEDIA009 box-geometry / text-stride citations.
     */
    expect_contains("source.f0320_media009_box",
                    source, "CHAMPION.C F0320:1744-1775 PC34 MEDIA009",
                    "F0320:1744-1775 PC34 MEDIA009 damage box");
    expect_contains("source.f0320_inventory_box",
                    source, "M771_BOX_BOTTOM=28",
                    "F0320:1748 inventory box bottom");
    expect_contains("source.f0320_noninventory_box",
                    source, "M771_BOX_BOTTOM=6",
                    "F0320:1762 non-inventory box bottom");
    expect_contains("source.f0320_inventory_byte_width",
                    source, "C016_BYTE_WIDTH",
                    "F0320:1749 inventory blit byte width");
    expect_contains("source.f0320_noninventory_byte_width",
                    source, "C024_BYTE_WIDTH",
                    "F0320:1763 non-inventory blit byte width");
    expect_contains("source.f0320_inventory_text_y",
                    source, "Y=16",
                    "F0320:1758 inventory text Y");
    expect_contains("source.f0320_noninventory_text_y",
                    source, "Y=5",
                    "F0320:1773 non-inventory text Y");
    expect_contains("source.f0320_text_print",
                    source, "F0053_TEXT_PrintToLogicalScreen",
                    "F0320:1775 F0053 centered text");
    expect_contains("source.f0320_defs_spacing",
                    source, "C69_CHAMPION_STATUS_BOX_SPACING",
                    "DEFS.H:2157 champion status box spacing");
}

static void assert_common_result(
    const char *prefix,
    const DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat *result,
    int champion_index,
    int inventory_ordinal,
    int damage,
    bool is_inventory,
    int graphic,
    int base_zone,
    int zone,
    const char *text,
    int expected_digit_count,
    int expected_text_x_offset,
    int expected_text_y,
    int expected_box_bottom,
    int expected_box_left_offset,
    int expected_box_right_offset,
    int expected_box_byte_width)
{
    char id[96];

    snprintf(id, sizeof(id), "%s.valid", prefix);
    expect_bool(id, result->valid, true, "CHAMDRAW.C F0623:680-699 valid route");
    snprintf(id, sizeof(id), "%s.contract_only", prefix);
    expect_bool(id, result->contract_only, true,
                "CHAMDRAW.C F0623 contract-only synthetic gate");
    snprintf(id, sizeof(id), "%s.champion_index", prefix);
    expect_int(id, result->champion_index, champion_index,
               "CHAMDRAW.C F0623:681 P2060 champion index");
    snprintf(id, sizeof(id), "%s.champion_ordinal", prefix);
    expect_int(id, result->champion_ordinal, champion_index + 1,
               "COMPILE.H:1038 M000_INDEX_TO_ORDINAL");
    snprintf(id, sizeof(id), "%s.inventory_ordinal", prefix);
    expect_int(id, result->inventory_champion_ordinal, inventory_ordinal,
               "PANEL.C F0355:2299-2316 G0423 inventory ordinal");
    snprintf(id, sizeof(id), "%s.is_inventory", prefix);
    expect_bool(id, result->is_inventory_champion, is_inventory,
                "CHAMDRAW.C F0623:688 inventory ordinal branch");
    snprintf(id, sizeof(id), "%s.graphic", prefix);
    expect_int(id, result->graphic_index, graphic,
               "CHAMDRAW.C F0623:689/692 damage graphic");
    snprintf(id, sizeof(id), "%s.base_zone", prefix);
    expect_int(id, result->base_zone_index, base_zone,
               "CHAMDRAW.C F0623:690/693 first damage zone");
    snprintf(id, sizeof(id), "%s.zone", prefix);
    expect_int(id, result->zone_index, zone,
               "CHAMDRAW.C F0623:696 zone += championIndex");
    snprintf(id, sizeof(id), "%s.transparent", prefix);
    expect_int(id, result->transparent_color,
               DM1_V1_CPDI_COLOR_TRANSPARENT_FLESH_PC34,
               "CHAMDRAW.C F0623:696 C10 transparent color");
    snprintf(id, sizeof(id), "%s.text_color", prefix);
    expect_int(id, result->text_color, DM1_V1_CPDI_COLOR_TEXT_WHITE_PC34,
               "CHAMDRAW.C F0623:697 C15 text color");
    snprintf(id, sizeof(id), "%s.text_background", prefix);
    expect_int(id, result->text_background_color,
               DM1_V1_CPDI_COLOR_TEXT_RED_PC34,
               "CHAMDRAW.C F0623:697 C08 text background");
    snprintf(id, sizeof(id), "%s.padding_enabled", prefix);
    expect_int(id, result->integer_padding_enabled,
               DM1_V1_CPDI_FORMAT_PADDING_OFF_PC34,
               "CHAMDRAW.C F0623:697 F0288 C0_FALSE padding");
    snprintf(id, sizeof(id), "%s.padding_width", prefix);
    expect_int(id, result->integer_padding_width,
               DM1_V1_CPDI_FORMAT_WIDTH_PC34,
               "CHAMDRAW.C F0623:697 F0288 width 3");
    snprintf(id, sizeof(id), "%s.damage", prefix);
    expect_int(id, result->damage, damage,
               "CHAMDRAW.C F0623:682 P2061 damage");
    snprintf(id, sizeof(id), "%s.damage_text", prefix);
    expect_str_eq(id, result->damage_text, text,
                  "CHAMDRAW.C F0288:374-392 unpadded integer text");
    snprintf(id, sizeof(id), "%s.redraw_index", prefix);
    expect_int(id, result->redraw_champion_index, champion_index,
               "CHAMDRAW.C F0623:698 F0292 same champion");
    snprintf(id, sizeof(id), "%s.op_count", prefix);
    expect_int(id, result->operation_count, 5,
               "CHAMDRAW.C F0623:695-699 five call operations");
    snprintf(id, sizeof(id), "%s.op0_enable", prefix);
    expect_int(id, result->operations[0],
               DM1_V1_CPDI_OP_ENABLE_MOUSE_UPDATE_PC34,
               "CHAMDRAW.C F0623:695 F0077");
    snprintf(id, sizeof(id), "%s.op1_blit", prefix);
    expect_int(id, result->operations[1],
               DM1_V1_CPDI_OP_BLIT_DAMAGE_GRAPHIC_PC34,
               "CHAMDRAW.C F0623:696 F0660_");
    snprintf(id, sizeof(id), "%s.op2_print", prefix);
    expect_int(id, result->operations[2],
               DM1_V1_CPDI_OP_PRINT_CENTERED_TEXT_PC34,
               "CHAMDRAW.C F0623:697 F0650");
    snprintf(id, sizeof(id), "%s.op3_redraw", prefix);
    expect_int(id, result->operations[3],
               DM1_V1_CPDI_OP_REDRAW_CHAMPION_STATE_PC34,
               "CHAMDRAW.C F0623:698 F0292");
    snprintf(id, sizeof(id), "%s.op4_disable", prefix);
    expect_int(id, result->operations[4],
               DM1_V1_CPDI_OP_DISABLE_MOUSE_UPDATE_PC34,
               "CHAMDRAW.C F0623:699 F0078");
    snprintf(id, sizeof(id), "%s.evidence_attached", prefix);
    expect_bool(id, result->evidence != NULL, true,
                "CHAMDRAW.C F0623:680-699 evidence pointer");
    /*
     * F0320 PC34 MEDIA009 box-geometry / text-stride assertions.
     * The damage == 0 short-circuit collapses all numeric offsets to 0
     * but still reports the F0320:1746 M770_BOX_TOP = 0 invariant.
     */
    snprintf(id, sizeof(id), "%s.box_top", prefix);
    expect_int(id, result->damage_box_top, DM1_V1_CPDI_BOX_TOP_PC34,
               "F0320:1746/1761 M770_BOX_TOP=0");
    snprintf(id, sizeof(id), "%s.box_bottom", prefix);
    expect_int(id, result->damage_box_bottom, expected_box_bottom,
               "F0320:1748/1762 M771_BOX_BOTTOM inventory vs non-inventory");
    snprintf(id, sizeof(id), "%s.box_left_offset", prefix);
    expect_int(id, result->damage_box_left_offset, expected_box_left_offset,
               "F0320:1749/1763 M768_BOX_LEFT inventory +7 vs non-inventory +0");
    snprintf(id, sizeof(id), "%s.box_right_offset", prefix);
    expect_int(id, result->damage_box_right_offset, expected_box_right_offset,
               "F0320:1749/1763 M769_BOX_RIGHT inventory +31 vs non-inventory +47");
    snprintf(id, sizeof(id), "%s.box_byte_width", prefix);
    expect_int(id, result->damage_box_byte_width, expected_box_byte_width,
               "F0320:1749/1763 C016_BYTE_WIDTH inventory vs C024_BYTE_WIDTH non-inventory");
    snprintf(id, sizeof(id), "%s.text_x_offset", prefix);
    expect_int(id, result->damage_text_x_offset, expected_text_x_offset,
               "F0320:1751-1758 / 1765-1772 1/2/3-digit x-stride");
    snprintf(id, sizeof(id), "%s.text_y", prefix);
    expect_int(id, result->damage_text_y, expected_text_y,
               "F0320:1758/1773 F0053 text Y inventory 16 vs non-inventory 5");
    snprintf(id, sizeof(id), "%s.digit_count", prefix);
    expect_int(id, result->damage_digit_count, expected_digit_count,
               "F0320:1751-1758 / 1765-1772 digit bucket / damage==0 short-circuit");
    snprintf(id, sizeof(id), "%s.champion_x_base", prefix);
    expect_int(id, result->champion_x_base,
               champion_index * DM1_V1_CPDI_CHAMPION_X_STRIDE_PC34,
               "F0320:1745 AL0969_i_X = championIndex * C69_CHAMPION_STATUS_BOX_SPACING");
}

static void test_small_and_big_routes(void)
{
    DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat input;
    DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat result;

    input.champion_index = 2;
    input.inventory_champion_ordinal = 1;
    input.damage = 42;
    expect_int("small.build_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
               1, "CHAMDRAW.C F0623:691-697 small route");
    /*
     * damage 42 = 2 digits; non-inventory: bottom=6, left=+0,
     * right=+47, byte_width=24, x-stride 16, Y=5.
     */
    assert_common_result("small", &result, 2, 1, 42, false,
                         DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34 + 2,
                         "42",
                         DM1_V1_CPDI_DIGIT_COUNT_2_PC34,
                         DM1_V1_CPDI_TEXT_X_STRIDE_2DIGIT_NONINVENTORY_PC34,
                         DM1_V1_CPDI_TEXT_Y_NONINVENTORY_PC34,
                         DM1_V1_CPDI_BOX_BOTTOM_NONINVENTORY_PC34,
                         DM1_V1_CPDI_BOX_LEFT_OFFSET_NONINVENTORY_PC34,
                         DM1_V1_CPDI_BOX_RIGHT_OFFSET_NONINVENTORY_PC34,
                         DM1_V1_CPDI_BOX_BYTE_WIDTH_NONINVENTORY_PC34);

    input.champion_index = 2;
    input.inventory_champion_ordinal = 3;
    input.damage = 105;
    expect_int("big.build_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
               1, "CHAMDRAW.C F0623:688-697 big route");
    /*
     * damage 105 = 3 digits; inventory: bottom=28, left=+7,
     * right=+31, byte_width=16, x-stride 15, Y=16.
     */
    assert_common_result("big", &result, 2, 3, 105, true,
                         DM1_V1_CPDI_GFX_DAMAGE_BIG_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34 + 2,
                         "105",
                         DM1_V1_CPDI_DIGIT_COUNT_3_PC34,
                         DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_INVENTORY_PC34,
                         DM1_V1_CPDI_TEXT_Y_INVENTORY_PC34,
                         DM1_V1_CPDI_BOX_BOTTOM_INVENTORY_PC34,
                         DM1_V1_CPDI_BOX_LEFT_OFFSET_INVENTORY_PC34,
                         DM1_V1_CPDI_BOX_RIGHT_OFFSET_INVENTORY_PC34,
                         DM1_V1_CPDI_BOX_BYTE_WIDTH_INVENTORY_PC34);
}

static void test_zone_stride_for_all_champions(void)
{
    int champion;

    for (champion = 0; champion < DM1_V1_CPDI_CHAMPION_COUNT_PC34; ++champion) {
        DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat input = {
            champion,
            champion + 1,
            champion + 7
        };
        DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat result;
        char id[96];

        expect_int("stride.build_return",
                   DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
                   1, "CHAMDRAW.C F0623:688-697 all champion slots");
        snprintf(id, sizeof(id), "stride.big_zone_champion_%d", champion);
        expect_int(id, result.zone_index,
                   DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34 + champion,
                   "CHAMDRAW.C F0623:696 C179 + championIndex");
        snprintf(id, sizeof(id), "stride.big_graphic_champion_%d", champion);
        expect_int(id, result.graphic_index,
                   DM1_V1_CPDI_GFX_DAMAGE_BIG_PC34,
                   "CHAMDRAW.C F0623:689 C016 inventory graphic");

        input.inventory_champion_ordinal = 0;
        expect_int("stride.small_build_return",
                   DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
                   1, "CHAMDRAW.C F0623:691-697 all small slots");
        snprintf(id, sizeof(id), "stride.small_zone_champion_%d", champion);
        expect_int(id, result.zone_index,
                   DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34 + champion,
                   "CHAMDRAW.C F0623:696 C167 + championIndex");
        snprintf(id, sizeof(id), "stride.small_graphic_champion_%d", champion);
        expect_int(id, result.graphic_index,
                   DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34,
                   "CHAMDRAW.C F0623:692 C015 non-inventory graphic");
    }
}

static void test_stale_inventory_ordinals_stay_small(void)
{
    static const int stale_ordinals[] = { -1, 5, 99 };
    unsigned i;

    for (i = 0; i < sizeof(stale_ordinals) / sizeof(stale_ordinals[0]); ++i) {
        DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat input = {
            1,
            stale_ordinals[i],
            17
        };
        DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat result;
        char id[96];

        snprintf(id, sizeof(id), "stale_inventory_%d.build",
                 stale_ordinals[i]);
        expect_int(id,
                   DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(
                       &input, &result),
                   1, "CHAMDRAW.C F0623:688 equality-only G0423 branch");
        snprintf(id, sizeof(id), "stale_inventory_%d.is_inventory",
                 stale_ordinals[i]);
        expect_bool(id, result.is_inventory_champion, false,
                    "CHAMDRAW.C F0623:688 only matching ordinal uses C016");
        snprintf(id, sizeof(id), "stale_inventory_%d.graphic",
                 stale_ordinals[i]);
        expect_int(id, result.graphic_index,
                   DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34,
                   "CHAMDRAW.C F0623:691-693 stale G0423 stays C015");
        snprintf(id, sizeof(id), "stale_inventory_%d.zone",
                 stale_ordinals[i]);
        expect_int(id, result.zone_index,
                   DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34 + 1,
                   "CHAMDRAW.C F0623:696 C167 + championIndex");
    }
}

static void test_damage_box_geometry_and_text_stride(void)
{
    /*
     * CHAMPION.C F0320:1744-1775 PC34 MEDIA009 branch:
     * inventory champion (G0423 match) -> bottom=28, left=+7,
     * right=+31, C016_BYTE_WIDTH blit, x-stride 21/18/15, Y=16.
     * non-inventory champion -> bottom=6, left=+0, right=+47,
     * C024_BYTE_WIDTH blit, x-stride 19/16/13, Y=5.
     * damage == 0 -> F0320:1736 short-circuit; all offsets/digit
     * counters collapse to 0 (damage_box_top stays 0 per F0320:1746
     * which the code only reaches when damage != 0, but 0 is the
     * default-initialized value).
     *
     * damage >= 1000 -> F0288 still prints 4 digits, but the
     * x-stride branching fires only for 1/2/3 digits, so damage
     * 1000..9999 lands in the 3-digit bucket (F0320:1756 / 1770
     * default arm).
     */
    static const struct {
        int damage;
        int digit_count;
        int inventory_x_stride;
        int noninventory_x_stride;
    } cases[] = {
        { 1, DM1_V1_CPDI_DIGIT_COUNT_1_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_1DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_1DIGIT_NONINVENTORY_PC34 },
        { 9, DM1_V1_CPDI_DIGIT_COUNT_1_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_1DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_1DIGIT_NONINVENTORY_PC34 },
        { 10, DM1_V1_CPDI_DIGIT_COUNT_2_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_2DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_2DIGIT_NONINVENTORY_PC34 },
        { 99, DM1_V1_CPDI_DIGIT_COUNT_2_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_2DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_2DIGIT_NONINVENTORY_PC34 },
        { 100, DM1_V1_CPDI_DIGIT_COUNT_3_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_NONINVENTORY_PC34 },
        { 999, DM1_V1_CPDI_DIGIT_COUNT_3_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_NONINVENTORY_PC34 },
        { 1000, DM1_V1_CPDI_DIGIT_COUNT_3_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_NONINVENTORY_PC34 },
        { 9999, DM1_V1_CPDI_DIGIT_COUNT_3_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_NONINVENTORY_PC34 },
        { DM1_V1_CPDI_DAMAGE_MAX_PC34, DM1_V1_CPDI_DIGIT_COUNT_3_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_INVENTORY_PC34,
          DM1_V1_CPDI_TEXT_X_STRIDE_3DIGIT_NONINVENTORY_PC34 }
    };
    int champion;
    unsigned i;

    for (champion = 0; champion < DM1_V1_CPDI_CHAMPION_COUNT_PC34; ++champion) {
        for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
            DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat input;
            DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat result;
            char id[96];

            /* inventory branch (G0423 matches champion ordinal) */
            input.champion_index = champion;
            input.inventory_champion_ordinal = champion + 1;
            input.damage = cases[i].damage;
            expect_int("geo.build_inv",
                       DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(
                           &input, &result),
                       1, "F0320:1744-1758 inventory branch");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.box_top", champion, cases[i].damage);
            expect_int(id, result.damage_box_top,
                       DM1_V1_CPDI_BOX_TOP_PC34,
                       "F0320:1746 M770_BOX_TOP=0");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.box_bottom", champion, cases[i].damage);
            expect_int(id, result.damage_box_bottom,
                       DM1_V1_CPDI_BOX_BOTTOM_INVENTORY_PC34,
                       "F0320:1748 M771_BOX_BOTTOM=28 inventory");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.box_left", champion, cases[i].damage);
            expect_int(id, result.damage_box_left_offset,
                       DM1_V1_CPDI_BOX_LEFT_OFFSET_INVENTORY_PC34,
                       "F0320:1749 inventory left=+7");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.box_right", champion, cases[i].damage);
            expect_int(id, result.damage_box_right_offset,
                       DM1_V1_CPDI_BOX_RIGHT_OFFSET_INVENTORY_PC34,
                       "F0320:1749 inventory right=+31");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.box_byte_width",
                     champion, cases[i].damage);
            expect_int(id, result.damage_box_byte_width,
                       DM1_V1_CPDI_BOX_BYTE_WIDTH_INVENTORY_PC34,
                       "F0320:1749 C016_BYTE_WIDTH=16 inventory");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.text_x", champion, cases[i].damage);
            expect_int(id, result.damage_text_x_offset,
                       cases[i].inventory_x_stride,
                       "F0320:1751-1758 inventory 1/2/3-digit x-stride");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.text_y", champion, cases[i].damage);
            expect_int(id, result.damage_text_y,
                       DM1_V1_CPDI_TEXT_Y_INVENTORY_PC34,
                       "F0320:1758 inventory text Y=16");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.digit_count", champion, cases[i].damage);
            expect_int(id, result.damage_digit_count, cases[i].digit_count,
                       "F0320:1751-1758 inventory digit bucket");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.x_base", champion, cases[i].damage);
            expect_int(id, result.champion_x_base,
                       champion * DM1_V1_CPDI_CHAMPION_X_STRIDE_PC34,
                       "F0320:1745 AL0969_i_X = championIndex * 69");
            snprintf(id, sizeof(id),
                     "geo.inv_%d_d%d.is_inventory",
                     champion, cases[i].damage);
            expect_bool(id, result.is_inventory_champion, true,
                        "F0320:1747 G0423 inventory ordinal match");

            /* non-inventory branch (G0423 = -1 -> no match) */
            input.inventory_champion_ordinal = -1;
            expect_int("geo.build_non",
                       DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(
                           &input, &result),
                       1, "F0320:1758-1773 non-inventory branch");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.box_bottom", champion, cases[i].damage);
            expect_int(id, result.damage_box_bottom,
                       DM1_V1_CPDI_BOX_BOTTOM_NONINVENTORY_PC34,
                       "F0320:1762 M771_BOX_BOTTOM=6 non-inventory");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.box_left", champion, cases[i].damage);
            expect_int(id, result.damage_box_left_offset,
                       DM1_V1_CPDI_BOX_LEFT_OFFSET_NONINVENTORY_PC34,
                       "F0320:1763 non-inventory left=+0");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.box_right", champion, cases[i].damage);
            expect_int(id, result.damage_box_right_offset,
                       DM1_V1_CPDI_BOX_RIGHT_OFFSET_NONINVENTORY_PC34,
                       "F0320:1763 non-inventory right=+47");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.box_byte_width",
                     champion, cases[i].damage);
            expect_int(id, result.damage_box_byte_width,
                       DM1_V1_CPDI_BOX_BYTE_WIDTH_NONINVENTORY_PC34,
                       "F0320:1763 C024_BYTE_WIDTH=24 non-inventory");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.text_x", champion, cases[i].damage);
            expect_int(id, result.damage_text_x_offset,
                       cases[i].noninventory_x_stride,
                       "F0320:1765-1772 non-inventory 1/2/3-digit x-stride");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.text_y", champion, cases[i].damage);
            expect_int(id, result.damage_text_y,
                       DM1_V1_CPDI_TEXT_Y_NONINVENTORY_PC34,
                       "F0320:1773 non-inventory text Y=5");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.digit_count",
                     champion, cases[i].damage);
            expect_int(id, result.damage_digit_count, cases[i].digit_count,
                       "F0320:1765-1772 non-inventory digit bucket");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.x_base", champion, cases[i].damage);
            expect_int(id, result.champion_x_base,
                       champion * DM1_V1_CPDI_CHAMPION_X_STRIDE_PC34,
                       "F0320:1760 AL0969_i_X carries the same championIndex*69");
            snprintf(id, sizeof(id),
                     "geo.non_%d_d%d.is_inventory",
                     champion, cases[i].damage);
            expect_bool(id, result.is_inventory_champion, false,
                        "F0320:1747 G0423 inventory ordinal miss");
        }
    }

    /*
     * damage == 0 short-circuit: F0320:1734-1737 `if (!pendingDamage)
     * continue;` skips the blit + centered text entirely. The gate
     * reports damage_box_top = 0 (F0320:1746 default) and collapses
     * all other geometry/stride fields to 0.
     */
    {
        DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat input;
        DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat result;
        char id[96];

        /* inventory + damage 0 */
        input.champion_index = 2;
        input.inventory_champion_ordinal = 3;
        input.damage = 0;
        expect_int("zero_inv.build",
                   DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(
                       &input, &result),
                   1, "F0320:1734-1737 damage==0 short-circuit");
        snprintf(id, sizeof(id), "zero_inv.box_top");
        expect_int(id, result.damage_box_top,
                   DM1_V1_CPDI_BOX_TOP_PC34,
                   "F0320:1746 M770_BOX_TOP=0 default");
        snprintf(id, sizeof(id), "zero_inv.box_bottom");
        expect_int(id, result.damage_box_bottom, 0,
                   "F0320:1736 short-circuit skips M771_BOX_BOTTOM");
        snprintf(id, sizeof(id), "zero_inv.box_left");
        expect_int(id, result.damage_box_left_offset, 0,
                   "F0320:1736 short-circuit skips M768_BOX_LEFT");
        snprintf(id, sizeof(id), "zero_inv.box_right");
        expect_int(id, result.damage_box_right_offset, 0,
                   "F0320:1736 short-circuit skips M769_BOX_RIGHT");
        snprintf(id, sizeof(id), "zero_inv.box_byte_width");
        expect_int(id, result.damage_box_byte_width, 0,
                   "F0320:1736 short-circuit skips blit byte width");
        snprintf(id, sizeof(id), "zero_inv.text_x");
        expect_int(id, result.damage_text_x_offset,
                   DM1_V1_CPDI_TEXT_X_STRIDE_NO_DAMAGE_PC34,
                   "F0320:1736 short-circuit skips AL0969_i_X stride");
        snprintf(id, sizeof(id), "zero_inv.text_y");
        expect_int(id, result.damage_text_y, 0,
                   "F0320:1736 short-circuit skips L0973_i_Y");
        snprintf(id, sizeof(id), "zero_inv.digit_count");
        expect_int(id, result.damage_digit_count,
                   DM1_V1_CPDI_DIGIT_COUNT_NO_DAMAGE_PC34,
                   "F0320:1736 short-circuit emits 0 digits");
        snprintf(id, sizeof(id), "zero_inv.x_base");
        expect_int(id, result.champion_x_base,
                   2 * DM1_V1_CPDI_CHAMPION_X_STRIDE_PC34,
                   "F0320:1745 AL0969_i_X still computed");

        /* non-inventory + damage 0 */
        input.inventory_champion_ordinal = -1;
        expect_int("zero_non.build",
                   DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(
                       &input, &result),
                   1, "F0320:1734-1737 damage==0 non-inventory short-circuit");
        snprintf(id, sizeof(id), "zero_non.box_bottom");
        expect_int(id, result.damage_box_bottom, 0,
                   "F0320:1736 short-circuit skips M771_BOX_BOTTOM");
        snprintf(id, sizeof(id), "zero_non.digit_count");
        expect_int(id, result.damage_digit_count,
                   DM1_V1_CPDI_DIGIT_COUNT_NO_DAMAGE_PC34,
                   "F0320:1736 short-circuit emits 0 digits");
        snprintf(id, sizeof(id), "zero_non.is_inventory");
        expect_bool(id, result.is_inventory_champion, false,
                    "F0320:1747 G0423 inventory ordinal miss");
    }
}

static void test_damage_text_edges(void)
{
    static const struct {
        int damage;
        const char *text;
    } cases[] = {
        { 0, "0" },
        { 1, "1" },
        { 9, "9" },
        { 10, "10" },
        { 99, "99" },
        { 100, "100" },
        { 999, "999" },
        { 1000, "1000" },
        { DM1_V1_CPDI_DAMAGE_MAX_PC34, "32767" }
    };
    unsigned i;

    for (i = 0; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat input = {
            0,
            0,
            cases[i].damage
        };
        DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat result;
        char id[96];

        expect_int("text_edge.build_return",
                   DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
                   1, "CHAMDRAW.C F0623:697 F0288 damage text");
        snprintf(id, sizeof(id), "text_edge.%d", cases[i].damage);
        expect_str_eq(id, result.damage_text, cases[i].text,
                      "CHAMDRAW.C F0288:374-392 C0_FALSE no padding");
        snprintf(id, sizeof(id), "text_edge.padding_off.%d", cases[i].damage);
        expect_int(id, result.integer_padding_enabled, 0,
                   "CHAMDRAW.C F0623:697 C0_FALSE");
    }
}

static void test_invalid_inputs_and_defaults(void)
{
    DM1_V1_ChampionPanelDamageIndicatorInputPc34Compat input;
    DM1_V1_ChampionPanelDamageIndicatorResultPc34Compat result;

    expect_int("invalid.null_output",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(NULL, NULL),
               0, "synthetic guard before CHAMDRAW.C F0623 model");

    expect_int("default.build_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(NULL, &result),
               1, "CHAMDRAW.C F0623 default synthetic champion 0");
    expect_int("default.champion", result.champion_index, 0,
               "CHAMPION.C F0320:1720 C00 first champion");
    expect_int("default.damage", result.damage, 1,
               "CHAMDRAW.C F0623:682 synthetic default damage");
    expect_int("default.graphic", result.graphic_index,
               DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34,
               "CHAMDRAW.C F0623:692 C015 when no inventory ordinal");
    expect_int("default.zone", result.zone_index,
               DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34,
               "CHAMDRAW.C F0623:696 C167 + 0");

    input.champion_index = -1;
    input.inventory_champion_ordinal = 0;
    input.damage = 1;
    expect_int("invalid.negative_champion_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
               0, "CHAMPION.C F0320:1720-1721 champion loop bound");
    expect_bool("invalid.negative_champion_flag", result.rejected_champion_index, true,
                "CHAMPION.C F0320:1720-1721 champion loop bound");

    input.champion_index = 4;
    expect_int("invalid.high_champion_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
               0, "CHAMPION.C F0320:1720-1721 four champion panel cells");
    expect_bool("invalid.high_champion_flag", result.rejected_champion_index, true,
                "CHAMPION.C F0320:1720-1721 four champion panel cells");

    input.champion_index = 0;
    input.damage = -1;
    expect_int("invalid.negative_damage_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
               0, "CHAMDRAW.C F0623:682 int16 damage guard");
    expect_bool("invalid.negative_damage_flag", result.rejected_damage, true,
                "CHAMDRAW.C F0623:682 int16 damage guard");

    input.damage = DM1_V1_CPDI_DAMAGE_MAX_PC34 + 1;
    expect_int("invalid.high_damage_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
               0, "CHAMDRAW.C F0623:682 int16 damage guard");
    expect_bool("invalid.high_damage_flag", result.rejected_damage, true,
                "CHAMDRAW.C F0623:682 int16 damage guard");
}

static void test_constants(void)
{
    expect_int("const.champion_count", DM1_V1_CPDI_CHAMPION_COUNT_PC34, 4,
               "CHAMPION.C F0320:1720-1721 party champion panel cells");
    expect_int("const.gfx_small", DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34, 15,
               "DEFS.H:2176 C015_GRAPHIC_DAMAGE_TO_CHAMPION_SMALL");
    expect_int("const.gfx_big", DM1_V1_CPDI_GFX_DAMAGE_BIG_PC34, 16,
               "DEFS.H:2177 C016_GRAPHIC_DAMAGE_TO_CHAMPION_BIG");
    expect_int("const.zone_small", DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34, 167,
               "DEFS.H:3792 C167_ZONE_FIRST_DAMAGE_TO_CHAMPION_SMALL");
    expect_int("const.zone_big", DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34, 179,
               "DEFS.H:3794 C179_ZONE_FIRST_DAMAGE_TO_CHAMPION_BIG");
    expect_int("const.transparent", DM1_V1_CPDI_COLOR_TRANSPARENT_FLESH_PC34, 10,
               "DEFS.H:2088 C10_COLOR_FLESH");
    expect_int("const.text_white", DM1_V1_CPDI_COLOR_TEXT_WHITE_PC34, 15,
               "DEFS.H:2093 C15_COLOR_WHITE");
    expect_int("const.text_red", DM1_V1_CPDI_COLOR_TEXT_RED_PC34, 8,
               "DEFS.H:2086 C08_COLOR_RED");
    expect_int("const.padding_off", DM1_V1_CPDI_FORMAT_PADDING_OFF_PC34, 0,
               "CHAMDRAW.C F0623:697 C0_FALSE");
    expect_int("const.padding_width", DM1_V1_CPDI_FORMAT_WIDTH_PC34, 3,
               "CHAMDRAW.C F0623:697 F0288 width");
    expect_int("const.damage_max", DM1_V1_CPDI_DAMAGE_MAX_PC34, 32767,
               "CHAMDRAW.C F0623:682 int16 P2061_i_Damage");
}

int main(void)
{
    test_evidence();
    test_constants();
    test_small_and_big_routes();
    test_zone_stride_for_all_champions();
    test_stale_inventory_ordinals_stay_small();
    test_damage_text_edges();
    test_damage_box_geometry_and_text_stride();
    test_invalid_inputs_and_defaults();

    printf("dm1_v1_champion_panel_damage_indicator_pc34_compat: assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
