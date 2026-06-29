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
                  "CHAMPION.C F0320_CHAMPION_ApplyAndDrawPendingDamageAndWounds:1720-1779",
                  "CHAMPION.C F0320:1720-1779");
    expect_contains("evidence.graphics", evidence->defs_graphics_anchor,
                    "C015/C016", "DEFS.H:2176-2177");
    expect_contains("evidence.colors", evidence->defs_colors_anchor,
                    "C08/C10/C15", "DEFS.H:2086-2093");
    expect_contains("evidence.zones", evidence->defs_zones_anchor,
                    "C167/C179", "DEFS.H:3792-3794");
    expect_contains("evidence.prototypes", evidence->defs_prototype_anchor,
                    "9067-9070", "DEFS.H:9067-9070");
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
    expect_contains("source.base", source, "BASE.C F0660:1473-1507",
                    "BASE.C F0660_ blit helper");
    expect_contains("source.panel", source, "PANEL.C F0355",
                    "PANEL.C inventory ordinal route");
    expect_contains("source.no_real_asset", source,
                    "without claiming real-asset parity",
                    "contract-only marker");
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
    const char *text)
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
    assert_common_result("small", &result, 2, 1, 42, false,
                         DM1_V1_CPDI_GFX_DAMAGE_SMALL_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_SMALL_FIRST_PC34 + 2,
                         "42");

    input.champion_index = 2;
    input.inventory_champion_ordinal = 3;
    input.damage = 105;
    expect_int("big.build_return",
               DM1_V1_ChampionPanelDamageIndicator_BuildPc34Compat(&input, &result),
               1, "CHAMDRAW.C F0623:688-697 big route");
    assert_common_result("big", &result, 2, 3, 105, true,
                         DM1_V1_CPDI_GFX_DAMAGE_BIG_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34,
                         DM1_V1_CPDI_ZONE_DAMAGE_BIG_FIRST_PC34 + 2,
                         "105");
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
    test_invalid_inputs_and_defaults();

    printf("dm1_v1_champion_panel_damage_indicator_pc34_compat: assertions=%d failures=%d\n",
           g_assertions, g_failures);
    return g_failures ? 1 : 0;
}
