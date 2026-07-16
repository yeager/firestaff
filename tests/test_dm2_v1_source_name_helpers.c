#include "dm2_v1_source_name_helpers.h"

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

static void expect_name(int ok,
                        const DM2_V1_SourceNameReceipt *receipt,
                        uint16_t value,
                        const char *name,
                        const char *symbol,
                        const char *source_path,
                        const char *label)
{
    expect_true(ok == 1, label);
    expect_true(receipt->handled && receipt->source_locked &&
                    receipt->valid && !receipt->blocked &&
                    receipt->value == value &&
                    strcmp(receipt->name, name) == 0 &&
                    strcmp(receipt->symbol, symbol) == 0 &&
                    strcmp(receipt->source_path, source_path) == 0,
                "receipt records source name and provenance");
}

static void test_spell_type_names(void)
{
    DM2_V1_SourceNameReceipt receipt;

    expect_name(dm2_v1_getSpellTypeName(1u, &receipt), &receipt, 1u,
                "POTION", "getSpellTypeName", "SKWIN/SkWinCore.cpp:461",
                "getSpellTypeName names potion spell type");
    expect_name(dm2_v1_getSpellTypeName(2u, &receipt), &receipt, 2u,
                "MISSILE", "getSpellTypeName", "SKWIN/SkWinCore.cpp:461",
                "getSpellTypeName names missile spell type");
    expect_name(dm2_v1_getSpellTypeName(3u, &receipt), &receipt, 3u,
                "GENERAL", "getSpellTypeName", "SKWIN/SkWinCore.cpp:461",
                "getSpellTypeName names general spell type");
    expect_name(dm2_v1_getSpellTypeName(4u, &receipt), &receipt, 4u,
                "SUMMON", "getSpellTypeName", "SKWIN/SkWinCore.cpp:461",
                "getSpellTypeName names summon spell type");
    expect_true(dm2_v1_getSpellTypeName(0u, &receipt) == 0 &&
                    receipt.blocked && !receipt.valid && receipt.name == NULL,
                "getSpellTypeName rejects unknown spell type");
}

static void test_skill_names(void)
{
    DM2_V1_SourceNameReceipt receipt;

    expect_name(dm2_v1_getSkillName(0u, &receipt), &receipt, 0u,
                "FIGHTER", "getSkillName", "SKWIN/SkWinCore.cpp:474",
                "getSkillName names fighter skill");
    expect_name(dm2_v1_getSkillName(1u, &receipt), &receipt, 1u,
                "NINJA", "getSkillName", "SKWIN/SkWinCore.cpp:474",
                "getSkillName names ninja skill");
    expect_name(dm2_v1_getSkillName(2u, &receipt), &receipt, 2u,
                "PRIEST", "getSkillName", "SKWIN/SkWinCore.cpp:474",
                "getSkillName names priest skill");
    expect_name(dm2_v1_getSkillName(3u, &receipt), &receipt, 3u,
                "WIZARD", "getSkillName", "SKWIN/SkWinCore.cpp:474",
                "getSkillName names wizard skill");
    expect_true(dm2_v1_getSkillName(4u, &receipt) == 0 &&
                    receipt.blocked && !receipt.valid,
                "getSkillName rejects out-of-range skill");
}

static void test_stat_bonus_names(void)
{
    DM2_V1_SourceNameReceipt receipt;

    expect_name(dm2_v1_getStatBonusName(0u, &receipt), &receipt, 0u,
                "STRENGTH", "getStatBonusName", "SKWIN/SkWinCore.cpp:503",
                "getStatBonusName names strength stat");
    expect_name(dm2_v1_getStatBonusName(3u, &receipt), &receipt, 3u,
                "VIGOR", "getStatBonusName", "SKWIN/SkWinCore.cpp:503",
                "getStatBonusName names vigor stat");
    expect_name(dm2_v1_getStatBonusName(6u, &receipt), &receipt, 6u,
                "ANTIFIRE", "getStatBonusName", "SKWIN/SkWinCore.cpp:503",
                "getStatBonusName names antifire stat");
    expect_true(dm2_v1_getStatBonusName(7u, &receipt) == 0 &&
                    receipt.blocked && !receipt.valid,
                "getStatBonusName rejects out-of-range stat");
}

int main(void)
{
    DM2_V1_SourceNameReceipt receipt;

    test_spell_type_names();
    test_skill_names();
    test_stat_bonus_names();
    expect_true(dm2_v1_getSkillName(0u, NULL) == 0,
                "source-name helpers reject missing receipt");
    dm2_v1_source_name_receipt_clear(&receipt);
    expect_true(!receipt.valid && receipt.name == NULL,
                "source-name receipt clear resets output");
    expect_true(strstr(dm2_v1_source_name_helpers_source_evidence(),
                       "getSpellTypeName:461") != NULL,
                "source evidence names spell-type helper");
    expect_true(strstr(dm2_v1_source_name_helpers_source_evidence(),
                       "getSkillName:474") != NULL,
                "source evidence names skill helper");
    expect_true(strstr(dm2_v1_source_name_helpers_source_evidence(),
                       "getStatBonusName:503") != NULL,
                "source evidence names stat-bonus helper");

    if (failures) {
        return 1;
    }
    puts("DM2 source-name helpers: ok");
    return 0;
}
