#include "dm2_v1_source_name_helpers.h"

#include <string.h>

typedef struct {
    uint16_t value;
    const char *name;
} DM2_V1_SourceNameEntry;

static const DM2_V1_SourceNameEntry dm2_spell_type_names[] = {
    { 1u, "POTION" },
    { 2u, "MISSILE" },
    { 3u, "GENERAL" },
    { 4u, "SUMMON" }
};

static const DM2_V1_SourceNameEntry dm2_skill_names[] = {
    { 0u, "FIGHTER" },
    { 1u, "NINJA" },
    { 2u, "PRIEST" },
    { 3u, "WIZARD" }
};

static const DM2_V1_SourceNameEntry dm2_stat_bonus_names[] = {
    { 0u, "STRENGTH" },
    { 1u, "BRAVERY" },
    { 2u, "PIETY" },
    { 3u, "VIGOR" },
    { 4u, "DEXTERITY" },
    { 5u, "WISDOM" },
    { 6u, "ANTIFIRE" }
};

void dm2_v1_source_name_receipt_clear(DM2_V1_SourceNameReceipt *receipt)
{
    if (!receipt) {
        return;
    }
    memset(receipt, 0, sizeof(*receipt));
}

static int dm2_source_name_lookup(
    uint16_t value,
    const DM2_V1_SourceNameEntry *entries,
    size_t entry_count,
    const char *symbol,
    const char *source_path,
    DM2_V1_SourceNameReceipt *out_receipt)
{
    size_t i;

    dm2_v1_source_name_receipt_clear(out_receipt);
    if (!out_receipt) {
        return 0;
    }
    out_receipt->handled = 1;
    out_receipt->source_locked = 1;
    out_receipt->value = value;
    out_receipt->symbol = symbol;
    out_receipt->source_path = source_path;

    for (i = 0u; i < entry_count; ++i) {
        if (entries[i].value == value) {
            out_receipt->valid = 1;
            out_receipt->name = entries[i].name;
            return 1;
        }
    }

    out_receipt->blocked = 1;
    return 0;
}

int dm2_v1_getSpellTypeName(uint16_t spell_type,
                            DM2_V1_SourceNameReceipt *out_receipt)
{
    return dm2_source_name_lookup(
        spell_type,
        dm2_spell_type_names,
        sizeof(dm2_spell_type_names) / sizeof(dm2_spell_type_names[0]),
        "getSpellTypeName",
        "SKWIN/SkWinCore.cpp:461",
        out_receipt);
}

int dm2_v1_getSkillName(uint16_t skill,
                        DM2_V1_SourceNameReceipt *out_receipt)
{
    return dm2_source_name_lookup(
        skill,
        dm2_skill_names,
        sizeof(dm2_skill_names) / sizeof(dm2_skill_names[0]),
        "getSkillName",
        "SKWIN/SkWinCore.cpp:474",
        out_receipt);
}

int dm2_v1_getStatBonusName(uint16_t stat_bonus,
                            DM2_V1_SourceNameReceipt *out_receipt)
{
    return dm2_source_name_lookup(
        stat_bonus,
        dm2_stat_bonus_names,
        sizeof(dm2_stat_bonus_names) / sizeof(dm2_stat_bonus_names[0]),
        "getStatBonusName",
        "SKWIN/SkWinCore.cpp:503",
        out_receipt);
}

const char *dm2_v1_source_name_helpers_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp getSpellTypeName:461 "
           "getSkillName:474 getStatBonusName:503; bounded source-name "
           "receipts over existing DM2 spell-type, base-skill, and champion "
           "stat constants. Unknown values fail closed without fallback labels.";
}
