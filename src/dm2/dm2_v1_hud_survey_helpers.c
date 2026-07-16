#include "dm2_v1_hud_survey_helpers.h"

#include <string.h>

static void dm2_money_box_survey_begin(
    DM2_V1_MoneyBoxSurveyReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_money_box_survey_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

static void dm2_show_attack_result_begin(
    DM2_V1_ShowAttackResultReceipt *receipt,
    const char *symbol,
    const char *source_path)
{
    dm2_v1_show_attack_result_receipt_clear(receipt);
    if (!receipt) {
        return;
    }
    receipt->handled = 1;
    receipt->source_locked = 1;
    receipt->symbol = symbol;
    receipt->source_path = source_path;
}

void dm2_v1_money_box_survey_receipt_clear(
    DM2_V1_MoneyBoxSurveyReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

void dm2_v1_show_attack_result_receipt_clear(
    DM2_V1_ShowAttackResultReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

static int dm2_money_box_survey_impl(
    const DM2_V1_MoneyBoxSurveyInput *input,
    DM2_V1_MoneyBoxSurveyReceipt *out_receipt,
    const char *symbol,
    const char *source_path)
{
    DM2_V1_SkprojectDrawMoneyboxReceipt moneybox;
    DM2_V1_SkprojectDrawContainerSurveyReceipt survey;
    int moneybox_ok;
    int survey_ok;

    dm2_money_box_survey_begin(out_receipt, symbol, source_path);
    if (!input) {
        if (out_receipt) {
            out_receipt->blocked = 1;
        }
        return 0;
    }
    moneybox_ok = dm2_v1_skproject_draw_moneybox(
        input->moneybox_object_id,
        input->container_cls2,
        input->coin_order,
        input->coin_counts,
        input->money_item_ids,
        &moneybox);
    survey_ok = dm2_v1_skproject_draw_container_survey(
        input->record_chain,
        input->record_count,
        &survey);

    if (out_receipt) {
        out_receipt->requested_real_gdat = 1u;
        out_receipt->requested_container_survey = 1u;
        out_receipt->moneybox_object_id = input->moneybox_object_id;
        out_receipt->container_cls2 = input->container_cls2;
        out_receipt->moneybox = moneybox;
        out_receipt->survey = survey;
        out_receipt->valid = moneybox_ok && survey_ok ? 1 : 0;
        out_receipt->blocked = out_receipt->valid ? 0 : 1;
    }
    return moneybox_ok && survey_ok ? 1 : 0;
}

int dm2_v1_MONEY_BOX_SURVEY(
    const DM2_V1_MoneyBoxSurveyInput *input,
    DM2_V1_MoneyBoxSurveyReceipt *out_receipt)
{
    return dm2_money_box_survey_impl(input,
                                     out_receipt,
                                     "MONEY_BOX_SURVEY",
                                     "SKWIN/SkWinCore.cpp:13814");
}

int dm2_v1_DM2_MONEY_BOX_SURVEY(
    const DM2_V1_MoneyBoxSurveyInput *input,
    DM2_V1_MoneyBoxSurveyReceipt *out_receipt)
{
    return dm2_money_box_survey_impl(input,
                                     out_receipt,
                                     "DM2_MONEY_BOX_SURVEY",
                                     "SKULLWIN/c_gui_draw.cpp:458");
}

static int dm2_show_attack_result_impl(
    uint16_t player,
    uint16_t damage_value,
    DM2_V1_ShowAttackResultReceipt *out_receipt,
    const char *symbol,
    const char *source_path)
{
    DM2_V1_SkprojectDrawPlayerDamageReceipt damage;
    int damage_ok;

    dm2_show_attack_result_begin(out_receipt, symbol, source_path);
    if (player >= DM2_V1_HUD_SURVEY_MAX_PLAYERS) {
        if (out_receipt) {
            out_receipt->blocked = 1;
            out_receipt->player = player;
            out_receipt->damage_value = damage_value;
        }
        return 0;
    }
    damage_ok = dm2_v1_skproject_draw_player_damage(player,
                                                    damage_value,
                                                    &damage);
    if (out_receipt) {
        out_receipt->player = player;
        out_receipt->damage_value = damage_value;
        out_receipt->damage = damage;
        out_receipt->valid = damage_ok ? 1 : 0;
        out_receipt->blocked = damage_ok ? 0 : 1;
    }
    return damage_ok ? 1 : 0;
}

int dm2_v1_SHOW_ATTACK_RESULT(
    uint16_t player,
    uint16_t damage_value,
    DM2_V1_ShowAttackResultReceipt *out_receipt)
{
    return dm2_show_attack_result_impl(player,
                                       damage_value,
                                       out_receipt,
                                       "SHOW_ATTACK_RESULT",
                                       "SKWIN/SkWinCore.cpp:6929");
}

int dm2_v1_DM2_SHOW_ATTACK_RESULT(
    uint16_t player,
    uint16_t damage_value,
    DM2_V1_ShowAttackResultReceipt *out_receipt)
{
    return dm2_show_attack_result_impl(player,
                                       damage_value,
                                       out_receipt,
                                       "DM2_SHOW_ATTACK_RESULT",
                                       "SKULLWIN/c_gui_draw.cpp:864");
}

const char *dm2_v1_hud_survey_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp MONEY_BOX_SURVEY:13814 "
           "SHOW_ATTACK_RESULT:6929; SKULLWIN/c_gui_draw.cpp "
           "DM2_MONEY_BOX_SURVEY:458 DM2_SHOW_ATTACK_RESULT:864; "
           "bounded HUD receipts over real GDAT/container draw routes.";
}
