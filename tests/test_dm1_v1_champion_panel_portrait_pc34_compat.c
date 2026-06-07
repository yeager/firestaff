#include "dm1/dm1_v1_champion_panel_portrait_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
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
    const DM1_V1_ChampionPanelPortraitEvidencePc34Compat *evidence =
        DM1_V1_ChampionPanelPortrait_EvidencePc34Compat();
    const char *source =
        DM1_V1_ChampionPanelPortrait_SourceEvidencePc34Compat();

    expect_bool("evidence.contract_only", evidence->contract_only, true,
                "PANEL.C F0354:2208-2240 contract-only route");
    expect_str_eq("evidence.drawstate", evidence->drawstate_anchor,
                  "CHAMDRAW.C F0292_CHAMPION_DrawState:810-812 inventory champion calls "
                  "F0354 and keeps only MASK0x0100_STATISTICS dirty",
                  "CHAMDRAW.C F0292:810-812");
    expect_contains("evidence.portrait", evidence->portrait_anchor,
                    "2208-2213", "PANEL.C F0354:2208-2213");
    expect_contains("evidence.portrait.pc34", evidence->portrait_anchor,
                    "2226-2232", "PANEL.C F0354:2226-2232");
    expect_contains("evidence.portrait.hatch", evidence->portrait_anchor,
                    "2237-2240", "PANEL.C F0354:2237-2240");
    expect_contains("evidence.defs.no_transparency", evidence->defs_anchor,
                    "CM1_COLOR_NO_TRANSPARENCY", "DEFS.H:2076");
    expect_contains("evidence.defs.byte_width", evidence->defs_anchor,
                    "C016_BYTE_WIDTH", "DEFS.H:2471");
    expect_contains("evidence.defs.zone", evidence->defs_anchor,
                    "C175_ZONE_FIRST_CHAMPION_STATUS_BOX", "DEFS.H:3793");
    expect_contains("evidence.coords.screen", evidence->coord_anchor,
                    "G2071_C320", "COORD.C:1713");
    expect_contains("evidence.coords.portrait", evidence->coord_anchor,
                    "G2078_C32/G2079_C29", "COORD.C:1748-1749");
    expect_contains("evidence.scope", evidence->scope_note,
                    "contract-only", "PANEL.C F0354:2208-2240 scope");
    expect_contains("evidence.no_real_asset", evidence->no_real_asset_claim,
                    "without claiming real-asset portrait parity",
                    "contract-only no original DOS parity claim");
    expect_contains("source.drawstate", source, "CHAMDRAW.C F0292:810-812",
                    "CHAMDRAW.C F0292:810-812");
    expect_contains("source.box", source, "championIndex*C69+7",
                    "PANEL.C F0354:2211");
    expect_contains("source.zone", source, "C175+championIndex",
                    "PANEL.C F0354:2226");
    expect_contains("source.transparent", source, "CM1_COLOR_NO_TRANSPARENCY",
                    "PANEL.C F0354:2229-2232");
    expect_contains("source.hatch", source, "hatches the same",
                    "PANEL.C F0354:2237-2240");
    expect_contains("source.no_real_asset", source,
                    "without claiming real-asset portrait parity",
                    "contract-only marker");
}

static void assert_draw_result(
    const char *prefix,
    const DM1_V1_ChampionPanelPortraitResultPc34Compat *result,
    int champion_index,
    int inventory_ordinal,
    int expected_zone,
    int expected_left,
    int expected_right,
    int expected_hatch,
    int expected_operation_count)
{
    char id[96];

    snprintf(id, sizeof(id), "%s.valid", prefix);
    expect_bool(id, result->valid, true, "CHAMDRAW.C F0292:810-812 live route");
    snprintf(id, sizeof(id), "%s.contract_only", prefix);
    expect_bool(id, result->contract_only, true,
                "PANEL.C F0354 contract-only synthetic gate");
    snprintf(id, sizeof(id), "%s.champion_index", prefix);
    expect_int(id, result->champion_index, champion_index,
               "CHAMDRAW.C F0292:755 M516_CHAMPIONS[index]");
    snprintf(id, sizeof(id), "%s.champion_ordinal", prefix);
    expect_int(id, result->champion_ordinal, champion_index + 1,
               "CHAMDRAW.C F0292:759 M000_INDEX_TO_ORDINAL");
    snprintf(id, sizeof(id), "%s.inventory_ordinal", prefix);
    expect_int(id, result->inventory_champion_ordinal, inventory_ordinal,
               "CHAMDRAW.C F0292:759 G0423 inventory ordinal");
    snprintf(id, sizeof(id), "%s.is_inventory", prefix);
    expect_bool(id, result->is_inventory_champion, true,
                "CHAMDRAW.C F0292:810 inventory champion branch");
    snprintf(id, sizeof(id), "%s.is_alive", prefix);
    expect_bool(id, result->is_alive, true,
                "CHAMDRAW.C F0292:784 live champion status box");
    snprintf(id, sizeof(id), "%s.should_draw", prefix);
    expect_bool(id, result->should_draw_portrait, true,
                "CHAMDRAW.C F0292:810-812 calls F0354");
    snprintf(id, sizeof(id), "%s.zone", prefix);
    expect_int(id, result->status_box_zone, expected_zone,
               "PANEL.C F0354:2226 C175 + championIndex");
    snprintf(id, sizeof(id), "%s.left", prefix);
    expect_int(id, result->target_left, expected_left,
               "PANEL.C F0354:2211 championIndex*C69 + 7");
    snprintf(id, sizeof(id), "%s.top", prefix);
    expect_int(id, result->target_top, 0, "PANEL.C F0354:2209 top=0");
    snprintf(id, sizeof(id), "%s.right", prefix);
    expect_int(id, result->target_right, expected_right,
               "PANEL.C F0354:2211 right=left+31");
    snprintf(id, sizeof(id), "%s.bottom", prefix);
    expect_int(id, result->target_bottom, 28,
               "PANEL.C F0354:2210 bottom=28");
    snprintf(id, sizeof(id), "%s.width", prefix);
    expect_int(id, result->target_width, 32,
               "COORD.C:1748 G2078_C32_PortraitWidth");
    snprintf(id, sizeof(id), "%s.height", prefix);
    expect_int(id, result->target_height, 29,
               "COORD.C:1749 G2079_C29_PortraitHeight");
    snprintf(id, sizeof(id), "%s.source_x", prefix);
    expect_int(id, result->portrait_source_x, 0,
               "PANEL.C F0354:2229-2232 portrait source x=0");
    snprintf(id, sizeof(id), "%s.source_y", prefix);
    expect_int(id, result->portrait_source_y, 0,
               "PANEL.C F0354:2229-2232 portrait source y=0");
    snprintf(id, sizeof(id), "%s.portrait_width", prefix);
    expect_int(id, result->portrait_width, 32,
               "PANEL.C F0354:2229-2232 G2078_C32 width");
    snprintf(id, sizeof(id), "%s.portrait_height", prefix);
    expect_int(id, result->portrait_height, 29,
               "PANEL.C F0354:2209-2211 0..28 portrait height");
    snprintf(id, sizeof(id), "%s.byte_width", prefix);
    expect_int(id, result->portrait_byte_width, 16,
               "DEFS.H:2471 C016_BYTE_WIDTH");
    snprintf(id, sizeof(id), "%s.screen_width", prefix);
    expect_int(id, result->screen_pixel_width, 320,
               "COORD.C:1713 G2071_C320_ScreenPixelWidth");
    snprintf(id, sizeof(id), "%s.transparent", prefix);
    expect_int(id, result->transparent_color, -1,
               "DEFS.H:2076 CM1_COLOR_NO_TRANSPARENCY");
    snprintf(id, sizeof(id), "%s.hatch", prefix);
    expect_bool(id, result->hatches_for_invisibility, expected_hatch != 0,
                "PANEL.C F0354:2237-2240 invisibility hatch branch");
    snprintf(id, sizeof(id), "%s.hatch_color", prefix);
    expect_int(id, result->hatch_color, expected_hatch ? 12 : -1,
               "PANEL.C F0354:2239 C12 hatch color");
    snprintf(id, sizeof(id), "%s.op_count", prefix);
    expect_int(id, result->operation_count, expected_operation_count,
               "PANEL.C F0354:2226-2240 zone/blit/hatch operation count");
    snprintf(id, sizeof(id), "%s.op0", prefix);
    expect_int(id, result->operations[0],
               DM1_V1_CPPOR_OP_GET_STATUS_BOX_ZONE_PC34,
               "PANEL.C F0354:2226 F0638_GetZone");
    snprintf(id, sizeof(id), "%s.op1", prefix);
    expect_int(id, result->operations[1],
               DM1_V1_CPPOR_OP_BLIT_CHAMPION_PORTRAIT_PC34,
               "PANEL.C F0354:2229-2232 F0132 portrait blit");
    snprintf(id, sizeof(id), "%s.op2", prefix);
    expect_int(id, result->operations[2],
               expected_hatch ? DM1_V1_CPPOR_OP_HATCH_INVISIBILITY_PC34
                              : DM1_V1_CPPOR_OP_NONE_PC34,
               "PANEL.C F0354:2237-2240 optional F0136 hatch");
    snprintf(id, sizeof(id), "%s.evidence", prefix);
    expect_bool(id, result->evidence != NULL, true,
                "PANEL.C F0354:2208-2240 evidence pointer");
}

static void test_draw_routes(void)
{
    DM1_V1_ChampionPanelPortraitInputPc34Compat input;
    DM1_V1_ChampionPanelPortraitResultPc34Compat result;

    DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(&input);
    input.champion_index = 0;
    input.inventory_champion_ordinal = 1;
    input.current_health = 77;
    input.party_invisibility_count = 0;
    expect_int("draw0.build",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               1, "CHAMDRAW.C F0292:810-812 live inventory portrait");
    assert_draw_result("draw0", &result, 0, 1, 175, 7, 38, 0, 2);

    input.champion_index = 3;
    input.inventory_champion_ordinal = 4;
    input.current_health = 12;
    input.party_invisibility_count = 5;
    expect_int("draw3.build",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               1, "PANEL.C F0354:2226-2240 champion 3 invisible route");
    assert_draw_result("draw3", &result, 3, 4, 178, 214, 245, 1, 3);
}

static void test_no_draw_routes(void)
{
    DM1_V1_ChampionPanelPortraitInputPc34Compat input;
    DM1_V1_ChampionPanelPortraitResultPc34Compat result;

    DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(&input);
    input.champion_index = 2;
    input.inventory_champion_ordinal = 1;
    input.current_health = 80;
    expect_int("nondraw.non_inventory.build",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               1, "CHAMDRAW.C F0292:813-814 non-inventory branch");
    expect_bool("nondraw.non_inventory.valid", result.valid, true,
                "CHAMDRAW.C F0292:755 valid champion state");
    expect_bool("nondraw.non_inventory.is_inventory",
                result.is_inventory_champion, false,
                "CHAMDRAW.C F0292:759 ordinal mismatch");
    expect_bool("nondraw.non_inventory.should_draw",
                result.should_draw_portrait, false,
                "CHAMDRAW.C F0292:813-814 skips F0354");
    expect_int("nondraw.non_inventory.ops", result.operation_count, 0,
               "CHAMDRAW.C F0292:813-814 no portrait operations");
    expect_int("nondraw.non_inventory.zone", result.status_box_zone, -1,
               "PANEL.C F0354:2226 not reached");

    input.champion_index = 1;
    input.inventory_champion_ordinal = 2;
    input.current_health = 0;
    expect_int("nondraw.dead.build",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               1, "CHAMDRAW.C F0292:816-838 dead status-box route");
    expect_bool("nondraw.dead.is_alive", result.is_alive, false,
                "CHAMDRAW.C F0292:784/816 current health gate");
    expect_bool("nondraw.dead.is_inventory", result.is_inventory_champion, true,
                "CHAMDRAW.C F0292:759 inventory ordinal still matches");
    expect_bool("nondraw.dead.should_draw", result.should_draw_portrait, false,
                "CHAMDRAW.C F0292:816-838 dead route skips F0354");
    expect_int("nondraw.dead.ops", result.operation_count, 0,
               "CHAMDRAW.C F0292:816-838 no portrait operations");
    expect_int("nondraw.dead.left", result.target_left, -1,
               "PANEL.C F0354:2208-2213 not reached");
}

static void test_validation_and_default(void)
{
    DM1_V1_ChampionPanelPortraitInputPc34Compat input;
    DM1_V1_ChampionPanelPortraitResultPc34Compat result;

    expect_int("default.null_input.build",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(NULL, &result),
               1, "CHAMDRAW.C F0292:810-812 default live inventory route");
    expect_bool("default.null_input.should_draw", result.should_draw_portrait,
                true, "CHAMDRAW.C F0292:810-812 default calls F0354");
    expect_int("default.null_input.zone", result.status_box_zone, 175,
               "PANEL.C F0354:2226 default C175 zone");

    DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(&input);
    input.champion_index = -1;
    expect_int("reject.negative_champion",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               0, "CHAMDRAW.C F0292:755 rejects non-panel index");
    expect_bool("reject.negative_champion.flag", result.rejected_champion_index,
                true, "CHAMDRAW.C F0292:755 four status-box scope");

    input.champion_index = 4;
    expect_int("reject.high_champion",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               0, "CHAMDRAW.C F0292:755 rejects champion index 4");
    expect_bool("reject.high_champion.flag", result.rejected_champion_index,
                true, "CHAMDRAW.C F0292:755 four status-box scope");

    DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(&input);
    input.inventory_champion_ordinal = 5;
    expect_int("reject.inventory_ordinal",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               0, "CHAMDRAW.C F0292:759 G0423 ordinal range");
    expect_bool("reject.inventory_ordinal.flag",
                result.rejected_inventory_ordinal, true,
                "CHAMDRAW.C F0292:759 ordinals are none or 1..4");

    DM1_V1_ChampionPanelPortrait_DefaultInputPc34Compat(&input);
    input.party_invisibility_count = -1;
    expect_int("reject.invisibility",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, &result),
               0, "PANEL.C F0354:2237 nonnegative event count");
    expect_bool("reject.invisibility.flag",
                result.rejected_invisibility_count, true,
                "PANEL.C F0354:2237 nonnegative event count");

    expect_int("reject.null_output",
               DM1_V1_ChampionPanelPortrait_BuildPc34Compat(&input, NULL),
               0, "PANEL.C F0354:2208-2240 requires output contract");
}

int main(void)
{
    printf("== DM1 V1 champion-panel portrait PC34 source-lock gate ==\n");

    test_evidence();
    test_draw_routes();
    test_no_draw_routes();
    test_validation_and_default();

    if (g_failures == 0) {
        printf("PASS: %d assertions passed.\n", g_assertions);
    } else {
        printf("FAIL: %d/%d assertions failed.\n", g_failures, g_assertions);
    }

    return g_failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
