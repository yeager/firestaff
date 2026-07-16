#include "dm2_v1_hud_survey_helpers.h"

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

static void test_money_box_survey(void)
{
    int16_t coin_order[DM2_V1_HUD_SURVEY_COIN_SLOTS] =
        {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int16_t coin_counts[DM2_V1_HUD_SURVEY_COIN_SLOTS] =
        {0, 2, 0, 40, 0, 1, 0, 0, 0, 0};
    uint16_t money_item_ids[DM2_V1_HUD_SURVEY_COIN_SLOTS] =
        {0x0301u, 0x0302u, 0x0303u, 0x0304u, 0x0305u,
         0x0306u, 0x0307u, 0x0308u, 0x0309u, 0x030au};
    uint16_t chain[] = {
        0x4100u, 0x4101u, 0x4102u, 0x4103u,
        0x4104u, 0x4105u, 0x4106u, 0x4107u,
        0x4108u
    };
    DM2_V1_MoneyBoxSurveyInput input = {
        0x4400u,
        6u,
        coin_order,
        coin_counts,
        money_item_ids,
        chain,
        9u
    };
    DM2_V1_MoneyBoxSurveyReceipt receipt;

    expect_true(dm2_v1_MONEY_BOX_SURVEY(&input, &receipt) == 1,
                "MONEY_BOX_SURVEY accepts real coin tables and record chain");
    expect_true(receipt.valid && receipt.source_locked &&
                    receipt.requested_real_gdat &&
                    receipt.requested_container_survey &&
                    !receipt.used_synthetic_gdat &&
                    strcmp(receipt.symbol, "MONEY_BOX_SURVEY") == 0,
                "MONEY_BOX_SURVEY receipt records source-named real-data route");
    expect_true(receipt.moneybox.valid &&
                    receipt.moneybox.box_icon.category == 20u &&
                    receipt.moneybox.box_icon.cls2 == 6u &&
                    receipt.moneybox.drawn_coin_slots == 3u &&
                    receipt.moneybox.first_coin_button_id == 0xdeu &&
                    receipt.moneybox.first_coin_stack_count == 2u &&
                    receipt.moneybox.last_coin_button_id == 0xe2u,
                "MONEY_BOX_SURVEY delegates coin drawing to GDAT moneybox receipt");
    expect_true(receipt.survey.valid &&
                    receipt.survey.drawn_items == 8u &&
                    receipt.survey.stopped_at_limit &&
                    receipt.survey.first_button_id == 0x2fu &&
                    receipt.survey.last_button_id == 0x36u,
                "MONEY_BOX_SURVEY delegates contents to container survey receipt");

    input.coin_order = 0;
    expect_true(dm2_v1_MONEY_BOX_SURVEY(&input, &receipt) == 0,
                "MONEY_BOX_SURVEY blocks missing coin-order table");
    expect_true(receipt.blocked &&
                    receipt.moneybox.blocked_missing_coin_tables &&
                    !receipt.used_synthetic_gdat,
                "MONEY_BOX_SURVEY fail-closed path does not synthesize coins");

    input.coin_order = coin_order;
    input.record_chain = 0;
    input.record_count = 0u;
    expect_true(dm2_v1_DM2_MONEY_BOX_SURVEY(&input, &receipt) == 0,
                "DM2_MONEY_BOX_SURVEY blocks missing record chain");
    expect_true(receipt.blocked &&
                    receipt.survey.blocked_missing_chain &&
                    strcmp(receipt.symbol, "DM2_MONEY_BOX_SURVEY") == 0,
                "DM2_MONEY_BOX_SURVEY records SKULLWIN alias and survey block");
}

static void test_show_attack_result(void)
{
    DM2_V1_ShowAttackResultReceipt receipt;

    expect_true(dm2_v1_SHOW_ATTACK_RESULT(2u, 47u, &receipt) == 1,
                "SHOW_ATTACK_RESULT accepts an in-party champion");
    expect_true(receipt.valid &&
                    receipt.source_locked &&
                    strcmp(receipt.symbol, "SHOW_ATTACK_RESULT") == 0 &&
                    receipt.damage.valid &&
                    receipt.damage.damage_icon.category == 1u &&
                    receipt.damage.damage_icon.cls2 == 2u &&
                    receipt.damage.damage_icon.entry == 3u &&
                    receipt.damage.damage_icon.button_id == 0xb3u &&
                    strcmp(receipt.damage.damage_text, "047") == 0,
                "SHOW_ATTACK_RESULT routes through player-damage HUD receipt");

    expect_true(dm2_v1_DM2_SHOW_ATTACK_RESULT(3u, 1000u, &receipt) == 1,
                "DM2_SHOW_ATTACK_RESULT exposes SKULLWIN alias");
    expect_true(receipt.damage.damage_value == 1000u &&
                    strcmp(receipt.damage.damage_text, "999") == 0 &&
                    strcmp(receipt.symbol, "DM2_SHOW_ATTACK_RESULT") == 0,
                "DM2_SHOW_ATTACK_RESULT preserves value and clamps visible text");

    expect_true(dm2_v1_SHOW_ATTACK_RESULT(4u, 12u, &receipt) == 0,
                "SHOW_ATTACK_RESULT blocks invalid champion index");
    expect_true(receipt.blocked && !receipt.valid &&
                    receipt.player == 4u &&
                    receipt.damage_value == 12u,
                "invalid champion attack result is fail-closed");
}

int main(void)
{
    test_money_box_survey();
    test_show_attack_result();
    expect_true(strstr(dm2_v1_hud_survey_helpers_source_evidence(),
                       "MONEY_BOX_SURVEY:13814") != 0,
                "source evidence includes SKWIN moneybox symbol");
    expect_true(strstr(dm2_v1_hud_survey_helpers_source_evidence(),
                       "DM2_SHOW_ATTACK_RESULT:864") != 0,
                "source evidence includes SKULLWIN attack-result alias");
    if (failures) {
        return 1;
    }
    puts("DM2 HUD survey helpers: ok");
    return 0;
}
