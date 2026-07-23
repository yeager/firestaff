#include "csb_v1_f0277_f0278_source_audit_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;
static int assertions;

static void check(int condition, const char *expression, int line)
{
    ++assertions;
    if (!condition) {
        ++failures;
        fprintf(stderr, "FAIL:%d: %s\n", line, expression);
    }
}

#define CHECK(condition) check((condition), #condition, __LINE__)

static void test_f0277_requires_authenticated_raw_platform_material(void)
{
    uint8_t sector[CSB_V1_F0277_PC34_SECTOR_BYTES];
    uint16_t words[CSB_V1_F0277_FUZZY_WORD_COUNT];
    uint8_t sector_before[sizeof(sector)];
    uint16_t words_before[CSB_V1_F0277_FUZZY_WORD_COUNT];
    CSB_V1_F0277RawSectorPc34 raw;
    CSB_V1_F0277FuzzyReceiptPc34 receipt;

    memset(sector, 0x68, sizeof(sector));
    memset(words, 0, sizeof(words));
    memcpy(sector_before, sector, sizeof(sector));
    memcpy(words_before, words, sizeof(words));
    memset(&raw, 0, sizeof(raw));
    raw.sector_bytes = sector;
    raw.sector_size = sizeof(sector);
    raw.prior_fuzzy_words = words;
    raw.prior_fuzzy_word_count = CSB_V1_F0277_FUZZY_WORD_COUNT;
    raw.raw_capture_identity = 0x27700001u;
    raw.csb_pc34_platform = 1;
    raw.authenticated = 1;

    CHECK(csb_v1_f0277_fuzzy_bits_raw_receipt_pc34(&raw, &receipt) == 1);
    CHECK(receipt.admitted && receipt.platform_authenticated);
    CHECK(receipt.raw_shape_valid && receipt.analysis_intentionally_unexecuted);
    CHECK(receipt.raw_capture_identity == raw.raw_capture_identity);
    CHECK(memcmp(sector, sector_before, sizeof(sector)) == 0);
    CHECK(memcmp(words, words_before, sizeof(words)) == 0);

    raw.authenticated = 0;
    CHECK(csb_v1_f0277_fuzzy_bits_raw_receipt_pc34(&raw, &receipt) == 0);
    raw.authenticated = 1;
    raw.sector_size--;
    CHECK(csb_v1_f0277_fuzzy_bits_raw_receipt_pc34(&raw, &receipt) == 0);
}

static void test_f0278_is_an_isolated_plan(void)
{
    uint16_t attributes[] = {0xffff, 0x1234};
    uint16_t before[2];
    CSB_V1_F0278ChampionRawStatePc34 raw;
    CSB_V1_F0278ResetPlanPc34 plan;

    memcpy(before, attributes, sizeof(attributes));
    memset(&raw, 0, sizeof(raw));
    raw.new_game = 0;
    raw.party_count = 2;
    raw.champion_attributes = attributes;
    raw.champion_attribute_count = 2;
    raw.leader_hand_thing = 42;
    raw.leader_hand_icon = 9;
    raw.leader_index = 0;
    raw.magic_caster_index = 1;
    raw.raw_state_identity = 0x27800001u;
    raw.authenticated = 1;

    CHECK(csb_v1_f0278_champion_reset_plan_pc34(&raw, &plan) == 1);
    CHECK(plan.admitted && plan.restores_leader_hand);
    CHECK(plan.clears_champion_dirty_attributes);
    CHECK(plan.champion_dirty_mask == CSB_V1_F0278_CHAMPION_DIRTY_MASK);
    CHECK(plan.redraw_is_not_invoked && plan.leader_restore_is_not_invoked);
    CHECK(plan.magic_caster_restore_is_not_invoked);
    CHECK(memcmp(attributes, before, sizeof(attributes)) == 0);

    raw.new_game = 1;
    CHECK(csb_v1_f0278_champion_reset_plan_pc34(&raw, &plan) == 1);
    CHECK(plan.clears_leader_hand && plan.marks_empty_hand);
    raw.magic_caster_index = 2;
    CHECK(csb_v1_f0278_champion_reset_plan_pc34(&raw, &plan) == 0);
}

int main(void)
{
    test_f0277_requires_authenticated_raw_platform_material();
    test_f0278_is_an_isolated_plan();
    printf("csb_v1_f0277_f0278_source_audit: %d/%d assertions passed\n",
           assertions - failures, assertions);
    return failures == 0 ? 0 : 1;
}
