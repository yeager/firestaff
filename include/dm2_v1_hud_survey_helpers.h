#ifndef FIRESTAFF_DM2_V1_HUD_SURVEY_HELPERS_H
#define FIRESTAFF_DM2_V1_HUD_SURVEY_HELPERS_H

#include "dm2_v1_skproject_core.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DM2_V1_HUD_SURVEY_COIN_SLOTS 10u
#define DM2_V1_HUD_SURVEY_MAX_PLAYERS 4u

typedef struct {
    uint16_t moneybox_object_id;
    uint8_t container_cls2;
    const int16_t *coin_order;
    const int16_t *coin_counts;
    const uint16_t *money_item_ids;
    const uint16_t *record_chain;
    uint16_t record_count;
} DM2_V1_MoneyBoxSurveyInput;

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint8_t requested_real_gdat;
    uint8_t requested_container_survey;
    uint8_t used_synthetic_gdat;
    uint16_t moneybox_object_id;
    uint8_t container_cls2;
    DM2_V1_SkprojectDrawMoneyboxReceipt moneybox;
    DM2_V1_SkprojectDrawContainerSurveyReceipt survey;
    const char *symbol;
    const char *source_path;
} DM2_V1_MoneyBoxSurveyReceipt;

typedef struct {
    int handled;
    int source_locked;
    int valid;
    int blocked;
    uint16_t player;
    uint16_t damage_value;
    DM2_V1_SkprojectDrawPlayerDamageReceipt damage;
    const char *symbol;
    const char *source_path;
} DM2_V1_ShowAttackResultReceipt;

void dm2_v1_money_box_survey_receipt_clear(
    DM2_V1_MoneyBoxSurveyReceipt *receipt);
void dm2_v1_show_attack_result_receipt_clear(
    DM2_V1_ShowAttackResultReceipt *receipt);

int dm2_v1_MONEY_BOX_SURVEY(
    const DM2_V1_MoneyBoxSurveyInput *input,
    DM2_V1_MoneyBoxSurveyReceipt *out_receipt);
int dm2_v1_DM2_MONEY_BOX_SURVEY(
    const DM2_V1_MoneyBoxSurveyInput *input,
    DM2_V1_MoneyBoxSurveyReceipt *out_receipt);

int dm2_v1_SHOW_ATTACK_RESULT(
    uint16_t player,
    uint16_t damage_value,
    DM2_V1_ShowAttackResultReceipt *out_receipt);
int dm2_v1_DM2_SHOW_ATTACK_RESULT(
    uint16_t player,
    uint16_t damage_value,
    DM2_V1_ShowAttackResultReceipt *out_receipt);

const char *dm2_v1_hud_survey_helpers_source_evidence(void);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_HUD_SURVEY_HELPERS_H */
