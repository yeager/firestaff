#include "dm2_v1_champion_hud_helpers.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

/* ---- Mock callbacks for PROCESS_ITEM_BONUS ---- */

typedef struct {
    uint16_t dbspec_word0;
    uint16_t dbspec_word19;
    int32_t fit_result;
    int16_t mp_bonus;
    int16_t ability_bonus;
    int16_t skill_bonus;
    int16_t walkspeed_bonus;
    int16_t light_bonus;
    uint8_t cls2;
} MockContext;

static uint16_t mock_query_dbspec(void *ctx, uint16_t item_ref, int16_t idx)
{
    MockContext *m = (MockContext *)ctx;
    (void)item_ref;
    if (idx == 0) return m->dbspec_word0;
    if (idx == 0x13) return m->dbspec_word19;
    return 0;
}

static int32_t mock_is_item_fit(void *ctx, uint16_t item_ref, int16_t slot)
{
    MockContext *m = (MockContext *)ctx;
    (void)item_ref;
    (void)slot;
    return m->fit_result;
}

static int16_t mock_retrieve_bonus(void *ctx, uint16_t item_ref,
                                    uint8_t idx, int32_t fit, int16_t mode)
{
    MockContext *m = (MockContext *)ctx;
    (void)item_ref;
    (void)fit;
    (void)mode;
    if (idx == 0x14) return m->mp_bonus;
    if (idx >= 0x15 && idx <= 0x1B) return m->ability_bonus;
    if (idx >= 0x1E && idx <= 0x31) return m->skill_bonus;
    if (idx == 0x33) return m->walkspeed_bonus;
    if (idx == 0x32) return m->light_bonus;
    return 0;
}

static uint8_t mock_query_cls2(void *ctx, uint16_t item_ref)
{
    MockContext *m = (MockContext *)ctx;
    (void)item_ref;
    return m->cls2;
}

static DM2_V1_ProcessItemBonusCallbacks make_cbs(MockContext *m)
{
    DM2_V1_ProcessItemBonusCallbacks cbs;
    memset(&cbs, 0, sizeof(cbs));
    cbs.query_dbspec_word = mock_query_dbspec;
    cbs.is_item_fit_for_equip = mock_is_item_fit;
    cbs.retrieve_item_bonus = mock_retrieve_bonus;
    cbs.query_cls2_from_record = mock_query_cls2;
    cbs.ctx = m;
    return cbs;
}

static void test_process_item_bonus_blocked(void)
{
    DM2_V1_ProcessItemBonusReceipt receipt;

    /* NULL input blocked */
    dm2_v1_PROCESS_ITEM_BONUS(NULL, NULL, &receipt);
    expect_true(receipt.blocked && !receipt.valid,
                "PROCESS_ITEM_BONUS blocks NULL input");

    /* hero_index < 0 blocked */
    DM2_V1_ProcessItemBonusInput input;
    memset(&input, 0, sizeof(input));
    input.hero_index = -1;
    input.item_ref = 0x1234;
    input.slot = 0;
    input.mode = 1;
    DM2_V1_ProcessItemBonusCallbacks cbs;
    memset(&cbs, 0, sizeof(cbs));
    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.blocked, "PROCESS_ITEM_BONUS blocks negative hero");

    /* item_ref == 0xFFFF blocked */
    input.hero_index = 0;
    input.item_ref = 0xFFFF;
    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.blocked, "PROCESS_ITEM_BONUS blocks null item");
}

static void test_process_item_bonus_light_recalc(void)
{
    MockContext mctx;
    memset(&mctx, 0, sizeof(mctx));
    mctx.dbspec_word0 = 0x10; /* bit 0x10 set, bit 0x2000 clear */
    DM2_V1_ProcessItemBonusCallbacks cbs = make_cbs(&mctx);
    DM2_V1_ProcessItemBonusInput input;
    memset(&input, 0, sizeof(input));
    input.hero_index = 0;
    input.item_ref = 0x100;
    input.slot = 5;
    input.mode = 1;
    DM2_V1_ProcessItemBonusReceipt receipt;

    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.light_recalc_count == 1,
                "PROCESS_ITEM_BONUS requests light recalc on bit 0x10");
    expect_true(receipt.weight_recalc == 1,
                "PROCESS_ITEM_BONUS requests weight recalc for mode != 0");

    /* mode == 0: no light recalc */
    input.mode = 0;
    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.light_recalc_count == 0,
                "PROCESS_ITEM_BONUS skips light recalc for mode 0");
    expect_true(receipt.weight_recalc == 0,
                "PROCESS_ITEM_BONUS skips weight recalc for mode 0");
}

static void test_process_item_bonus_equip_mode1(void)
{
    MockContext mctx;
    memset(&mctx, 0, sizeof(mctx));
    mctx.dbspec_word0 = 0x2000;
    mctx.fit_result = 1;
    mctx.mp_bonus = 5;
    mctx.ability_bonus = 2;
    mctx.skill_bonus = 0;
    mctx.walkspeed_bonus = 0;
    mctx.light_bonus = 3;
    DM2_V1_ProcessItemBonusCallbacks cbs = make_cbs(&mctx);
    DM2_V1_ProcessItemBonusInput input;
    memset(&input, 0, sizeof(input));
    input.hero_index = 0;
    input.item_ref = 0x100;
    input.slot = 2;
    input.mode = 1;
    DM2_V1_ProcessItemBonusReceipt receipt;

    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.valid, "mode 1 equip: valid");
    expect_true(receipt.mp_dirty && receipt.max_mp_delta == 5,
                "mode 1 equip: maxMP delta = 5");
    expect_true((receipt.heroflag_or & 0x800) != 0,
                "mode 1 equip: heroflag 0x800 set for MP bonus");
    expect_true(receipt.ability_dirty,
                "mode 1 equip: ability changes present");
    /* All 7 abilities get eability_delta = 2 */
    {
        int all_eab = 1;
        int i;
        for (i = 0; i < DM2_V1_NUM_ABILITIES; i++) {
            if (receipt.eability_delta[i] != 2) all_eab = 0;
        }
        expect_true(all_eab, "mode 1 equip: eability deltas = 2");
    }
    expect_true((receipt.heroflag_or & 0x3000) != 0,
                "mode 1 equip: heroflag 0x3000 for abilities");
    expect_true(receipt.light_bonus_dirty && receipt.light_w00_delta == 3,
                "mode 1 equip: light bonus delta = 3");
    expect_true(receipt.light_bonus_recalc,
                "mode 1 equip: light bonus triggers recalc");
}

static void test_process_item_bonus_activate_mode2(void)
{
    MockContext mctx;
    memset(&mctx, 0, sizeof(mctx));
    mctx.dbspec_word0 = 0x2000;
    mctx.fit_result = 1;
    mctx.mp_bonus = 10;
    mctx.ability_bonus = 3;
    mctx.walkspeed_bonus = 1;
    mctx.skill_bonus = 0;
    mctx.light_bonus = 0;
    mctx.dbspec_word19 = 42;
    mctx.cls2 = 7;
    DM2_V1_ProcessItemBonusCallbacks cbs = make_cbs(&mctx);
    DM2_V1_ProcessItemBonusInput input;
    memset(&input, 0, sizeof(input));
    input.hero_index = 0;
    input.item_ref = 0x100;
    input.slot = 2;
    input.mode = 2;
    DM2_V1_ProcessItemBonusReceipt receipt;

    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.valid, "mode 2 activate: valid");
    expect_true(receipt.cur_mp_set && receipt.cur_mp_value == 10,
                "mode 2 activate: curMP bonus = 10");
    expect_true(receipt.max_mp_delta == 0,
                "mode 2 activate: maxMP unchanged");
    {
        int i;
        for (i = 0; i < DM2_V1_NUM_ABILITIES; i++) {
            expect_true(receipt.ability_use_adjust[i] == 1,
                        "mode 2: abilities use adjust path");
        }
    }
    expect_true(receipt.walkspeed_dirty && receipt.walkspeed_delta == 1,
                "mode 2 activate: walkspeed delta = 1");
    expect_true(receipt.queue_timer,
                "mode 2 activate: timer queued for walkspeed");
    expect_true(receipt.timer_dbspec_0x13 == 42,
                "mode 2 activate: timer dbspec 0x13 correct");
    expect_true(receipt.timer_xB == 7,
                "mode 2 activate: timer cls2 correct");
    expect_true(!receipt.light_bonus_dirty,
                "mode 2 activate: light bonus skipped");
}

static void test_process_item_bonus_mode3_skips(void)
{
    MockContext mctx;
    memset(&mctx, 0, sizeof(mctx));
    mctx.dbspec_word0 = 0x2000;
    mctx.fit_result = 1;
    mctx.mp_bonus = 5;
    mctx.ability_bonus = 2;
    mctx.skill_bonus = 1;
    mctx.walkspeed_bonus = 1;
    mctx.light_bonus = 1;
    DM2_V1_ProcessItemBonusCallbacks cbs = make_cbs(&mctx);
    DM2_V1_ProcessItemBonusInput input;
    memset(&input, 0, sizeof(input));
    input.hero_index = 0;
    input.item_ref = 0x100;
    input.slot = 2;
    input.mode = 3;
    DM2_V1_ProcessItemBonusReceipt receipt;

    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.valid, "mode 3: valid");
    expect_true(!receipt.mp_dirty, "mode 3: MP skipped");
    expect_true(!receipt.ability_dirty, "mode 3: abilities skipped");
    expect_true(receipt.skill_dirty, "mode 3: skill bonuses run");
    expect_true(receipt.walkspeed_dirty, "mode 3: walkspeed runs");
    expect_true(!receipt.light_bonus_dirty, "mode 3: light bonus skipped");
}

static void test_process_item_bonus_slot_ge_0x1e(void)
{
    DM2_V1_ProcessItemBonusCallbacks cbs;
    memset(&cbs, 0, sizeof(cbs));
    DM2_V1_ProcessItemBonusInput input;
    memset(&input, 0, sizeof(input));
    input.hero_index = 0;
    input.item_ref = 0x100;
    input.slot = 0x1E;
    input.mode = 1;
    DM2_V1_ProcessItemBonusReceipt receipt;

    dm2_v1_PROCESS_ITEM_BONUS(&input, &cbs, &receipt);
    expect_true(receipt.weight_recalc == 1,
                "slot >= 0x1E: weight recalc requested");
    expect_true(!receipt.mp_dirty && !receipt.ability_dirty,
                "slot >= 0x1E: no MP or ability changes");
}

static void test_query_player_skill_lv(void)
{
    DM2_V1_PlayerSkillInput input;
    DM2_V1_ChampionHudReceipt receipt;

    memset(&input, 0, sizeof(input));
    input.experience = 4096u;
    input.base_level = 1u;
    input.temporary_bonus = 0u;
    input.maximum_level = 8u;

    expect_true(dm2_v1_QUERY_PLAYER_SKILL_LV(&input, &receipt) == 2u,
                "QUERY_PLAYER_SKILL_LV advances through bounded thresholds");
    expect_true(receipt.valid && receipt.result == 2 &&
                    strcmp(receipt.symbol, "QUERY_PLAYER_SKILL_LV") == 0,
                "skill level receipt records source symbol");

    input.experience = 0u;
    input.base_level = 14u;
    input.temporary_bonus = 5u;
    input.maximum_level = 15u;
    expect_true(dm2_v1_QUERY_PLAYER_SKILL_LV(&input, &receipt) == 15u,
                "QUERY_PLAYER_SKILL_LV clamps temporary boost");
    expect_true(receipt.dirty,
                "temporary skill boost marks display-affecting result");

    expect_true(dm2_v1_QUERY_PLAYER_SKILL_LV(0, &receipt) == 0u,
                "QUERY_PLAYER_SKILL_LV blocks missing input");
    expect_true(receipt.blocked && !receipt.valid,
                "missing skill input is fail-closed");
}

static void test_refresh_player_stat_disp(void)
{
    DM2_V1_PlayerStatDisplayInput input;
    DM2_V1_PlayerStatDisplay display;
    DM2_V1_ChampionHudReceipt receipt;

    memset(&input, 0, sizeof(input));
    input.current_value = 35;
    input.maximum_value = 70;
    input.previous_current_value = 30;
    input.previous_maximum_value = 70;
    input.bar_color = 12;

    expect_true(dm2_v1_REFRESH_PLAYER_STAT_DISP(&input, &display,
                                                &receipt) == 1,
                "REFRESH_PLAYER_STAT_DISP accepts bounded stat input");
    expect_true(display.percent == 50u && display.redraw_value &&
                    !display.redraw_maximum && display.redraw_bar &&
                    receipt.valid && receipt.result == 50 &&
                    strcmp(receipt.symbol,
                           "REFRESH_PLAYER_STAT_DISP") == 0,
                "stat display receipt records percent and redraw flags");

    input.current_value = 90;
    input.maximum_value = 80;
    input.previous_current_value = 80;
    input.previous_maximum_value = 80;
    expect_true(dm2_v1_REFRESH_PLAYER_STAT_DISP(&input, &display,
                                                &receipt) == 1,
                "REFRESH_PLAYER_STAT_DISP clamps current to maximum");
    expect_true(display.current_value == 80 && display.percent == 100u,
                "stat display percent is capped at full bar");

    input.maximum_value = 0;
    expect_true(dm2_v1_REFRESH_PLAYER_STAT_DISP(&input, &display,
                                                &receipt) == 0,
                "REFRESH_PLAYER_STAT_DISP blocks zero maximum");
    expect_true(receipt.blocked && !receipt.valid,
                "invalid stat display input is fail-closed");
}

static void test_bar_color_queries(void)
{
    DM2_V1_BarColorReceipt receipt;

    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(-1, 5, &receipt) == 5,
                "food/water bar color returns default when no GDAT");
    expect_true(receipt.valid && !receipt.gdat_override && receipt.color == 5,
                "food/water receipt: no override, default color");

    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(42, 5, &receipt) == 298,
                "food/water bar color returns 256+gdat when found");
    expect_true(receipt.valid && receipt.gdat_override && receipt.color == 298,
                "food/water receipt: GDAT override active");

    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(-1, 7, &receipt) == 7,
                "3stat bar color returns default when no GDAT");
    expect_true(receipt.valid && !receipt.gdat_override,
                "3stat receipt: no override");

    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(11, 7, &receipt) == 11,
                "3stat bar color returns GDAT value directly");
    expect_true(receipt.valid && receipt.gdat_override && receipt.color == 11,
                "3stat receipt: GDAT override active");

    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(-1, 14, 0) == 14,
                "food/water bar color works with NULL receipt");
    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(-1, 8, 0) == 8,
                "3stat bar color works with NULL receipt");
}

int main(void)
{
    test_process_item_bonus_blocked();
    test_process_item_bonus_light_recalc();
    test_process_item_bonus_equip_mode1();
    test_process_item_bonus_activate_mode2();
    test_process_item_bonus_mode3_skips();
    test_process_item_bonus_slot_ge_0x1e();
    test_query_player_skill_lv();
    test_refresh_player_stat_disp();
    test_bar_color_queries();
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "PROCESS_ITEM_BONUS:59") != 0,
                "source evidence includes item bonus symbol");
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "REFRESH_PLAYER_STAT_DISP:14573") != 0,
                "source evidence includes stat display symbol");
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "QUERY_FOOD_WATER_BAR_COLOR:13194") != 0,
                "source evidence includes food/water bar color symbol");
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "QUERY_3STAT_BAR_COLOR:13203") != 0,
                "source evidence includes 3stat bar color symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 champion/HUD helpers: ok");
    return 0;
}
