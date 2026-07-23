/* test_dm2_v1_spell_cast_player_pc34_compat.c — DM2-007 runtime spell cast
 * slice: live hero rune lookup, resource spending, execution branches, and
 * timer-effect requests.
 *
 * Source: skproject/SKULLWIN/c_events.cpp:2211-2786
 *         skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)
 */

#include "dm2_v1_spell_cast_player.h"

#include <stdio.h>
#include <string.h>

static int g_checks;
static int g_failures;

static void expect_true(int condition, const char *label)
{
    ++g_checks;
    if (!condition) {
        ++g_failures;
        fprintf(stderr, "FAIL: %s\n", label);
    }
}

static void set_word(DM2_V1_GdatEntry *entry,
                     int spell_index,
                     int field,
                     uint16_t value)
{
    memset(entry, 0, sizeof(*entry));
    entry->cls1 = DM2_GDAT_CATEGORY_SPELL_DEF;
    entry->cls2 = (uint8_t)spell_index;
    entry->cls3 = DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    entry->cls4 = (uint8_t)field;
    entry->data_index = value;
}

static void build_extended_receipt(DM2_V1_ExtendedSpellsReceipt *receipt)
{
    DM2_V1_GdatEntry entries[14];
    uint32_t raw_offsets[1] = {0u};
    uint32_t raw_sizes[1] = {12u};
    const uint8_t raw_data[] = "TEST EXTENDED";
    uint16_t original_w6[DM2_V1_EXT_SPELLS_ORIGINAL_COUNT];
    DM2_V1_AssetLoader loader;
    int i;

    memset(entries, 0, sizeof(entries));
    memset(&loader, 0, sizeof(loader));
    for (i = 0; i < DM2_V1_EXT_SPELLS_ORIGINAL_COUNT; ++i) {
        original_w6[i] = (uint16_t)((i & 0x3f) << 4);
    }

    /* Custom spell index 7: FUL BRO NETA (unique extended GENERAL spell).
     * rune1=7 (FUL), rune2=8 (BRO), rune3=9 (NETA), difficulty=5,
     * skill=0x0F, type=GENERAL (3), result=0x0B. */
    set_word(&entries[0], 7, 0x01, 7u);
    set_word(&entries[1], 7, 0x02, 8u);
    set_word(&entries[2], 7, 0x03, 9u);
    set_word(&entries[3], 7, 0x04, 5u);
    set_word(&entries[4], 7, 0x05, 0x0Fu);
    set_word(&entries[5], 7, 0x06, 3u);
    set_word(&entries[6], 7, 0x07, 0x0Bu);

    /* Custom spell index 9: ZO KATH RA (ZoKathRa extended-only summon).
     * rune1=14 (ZO), rune2=10 (KATH), rune3=2 (RA), type=SUMMON (4). */
    set_word(&entries[7], 9, 0x01, 14u);
    set_word(&entries[8], 9, 0x02, 10u);
    set_word(&entries[9], 9, 0x03, 2u);
    set_word(&entries[10], 9, 0x04, 8u);
    set_word(&entries[11], 9, 0x05, 0x0Fu);
    set_word(&entries[12], 9, 0x06, 4u);
    set_word(&entries[13], 9, 0x07, 0x21u);

    loader.loaded = 1;
    loader.entries = entries;
    loader.entry_count = (uint16_t)(sizeof(entries) / sizeof(entries[0]));
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.raw_data_count = 1u;
    loader.data = raw_data;
    loader.data_size = sizeof(raw_data);

    dm2_v1_extended_load_spells_definition(&loader, 1, original_w6,
        DM2_V1_EXT_SPELLS_ORIGINAL_COUNT, receipt);
}

static void test_build_table(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    expect_true(table.count == DM2_MAX_SPELL_ORIGINAL,
                "fixed-only table has 34 entries");
    expect_true(table.extended_mode == 0,
                "disabled extended mode marks table non-extended");

    build_extended_receipt(&ext);
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    expect_true(table.extended_mode == 1,
                "extended receipt enables extended mode");
    expect_true(table.count == DM2_MAX_SPELL_ORIGINAL + 2,
                "fixed + two custom spells loaded");
    expect_true(table.records[DM2_MAX_SPELL_ORIGINAL].source == 1,
                "first extended entry marked source=1");
    expect_true(table.records[DM2_MAX_SPELL_ORIGINAL].index ==
                    DM2_MAX_SPELL_ORIGINAL + 7,
                "extended index maps to fixed base + custom index");
}

static void test_lookup_fixed_and_extended(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    const uint8_t light[] = {DM2_RUNE_FUL, 0};
    const uint8_t fireball[] = {DM2_RUNE_FUL, DM2_RUNE_IR, 0};
    const uint8_t extended_custom[] = {DM2_RUNE_FUL, DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_NETA, 0};
    const uint8_t extended_zok[] = {DM2_RUNE_FUL, DM2_RUNE_ZO, DM2_RUNE_KATH, DM2_RUNE_RA, 0};
    const uint8_t unknown[] = {DM2_RUNE_OH, DM2_RUNE_OH, DM2_RUNE_OH, 0};
    int idx;

    build_extended_receipt(&ext);
    dm2_v1_spell_cast_player_build_table(&ext, &table);

    idx = dm2_v1_spell_cast_player_find_by_runes(&table, light);
    expect_true(idx == 5, "Light (FUL) resolves to fixed index 5");

    idx = dm2_v1_spell_cast_player_find_by_runes(&table, fireball);
    expect_true(idx == 16, "Fireball (FUL IR) resolves to fixed index 16");

    idx = dm2_v1_spell_cast_player_find_by_runes(&table, extended_custom);
    expect_true(idx >= 0 && table.records[idx].source == 1 &&
                    table.records[idx].index == DM2_MAX_SPELL_ORIGINAL + 7,
                "FUL BRO NETA resolves to extended slot 7");

    idx = dm2_v1_spell_cast_player_find_by_runes(&table, extended_zok);
    expect_true(idx >= 0 && table.records[idx].source == 1 &&
                    table.records[idx].index == DM2_MAX_SPELL_ORIGINAL + 9,
                "ZO KATH RA resolves to extended slot 9");

    idx = dm2_v1_spell_cast_player_find_by_runes(&table, unknown);
    expect_true(idx == -1, "unknown rune combination returns -1");
    expect_true(dm2_v1_spell_cast_player_find_by_runes(NULL, light) == -1,
                "NULL table fails closed");
}

static void test_cast_success_branches(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    const uint8_t light[] = {DM2_RUNE_FUL, 0};
    const uint8_t fireball[] = {DM2_RUNE_FUL, DM2_RUNE_IR, 0};
    const uint8_t str_potion[] = {DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_KU, 0};
    const uint8_t attack_minion[] = {DM2_RUNE_ZO, DM2_RUNE_EW, DM2_RUNE_KU, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);

    /* Light (GENERAL) */
    r = dm2_v1_spell_cast_player(&table, light, 30, 100, 0);
    expect_true(r.valid && r.found, "Light lookup succeeded");
    expect_true(r.cast_success && r.bp0c > 0, "Light cast succeeds");
    expect_true(r.execution_class == DM2_V1_SPELL_EXEC_GENERAL,
                "Light is GENERAL branch");
    expect_true(r.timer_kind == DM2_V1_SPELL_TIMER_LIGHT,
                "Light requests a light timer");
    expect_true(r.timer_duration > 0, "Light has positive duration");
    expect_true(r.cooldown_ticks > 0, "Light applies cooldown");

    /* Fireball (MISSILE) */
    r = dm2_v1_spell_cast_player(&table, fireball, 30, 100, 0);
    expect_true(r.cast_success, "Fireball cast succeeds");
    expect_true(r.execution_class == DM2_V1_SPELL_EXEC_MISSILE,
                "Fireball is MISSILE branch");
    expect_true(r.timer_kind == DM2_V1_SPELL_TIMER_PROJECTILE,
                "Fireball requests projectile");
    expect_true(r.object_effect == DM2_OBJECT_EFFECT_FIREBALL,
                "Fireball carries FIREBALL object effect");

    /* STR Potion (POTION) with flask */
    r = dm2_v1_spell_cast_player(&table, str_potion, 30, 100, 1);
    expect_true(r.cast_success, "STR Potion cast succeeds with flask");
    expect_true(r.execution_class == DM2_V1_SPELL_EXEC_POTION,
                "STR Potion is POTION branch");
    expect_true(r.flask_required && r.flask_consumed,
                "STR Potion consumes flask");

    /* Attack Minion (SUMMON) */
    r = dm2_v1_spell_cast_player(&table, attack_minion, 60, 200, 0);
    expect_true(r.cast_success, "Attack Minion cast succeeds");
    expect_true(r.execution_class == DM2_V1_SPELL_EXEC_SUMMON,
                "Attack Minion is SUMMON branch");
    expect_true(r.timer_kind == DM2_V1_SPELL_TIMER_SUMMON,
                "Attack Minion requests summon timer");
    expect_true(r.timer_duration > 0, "Summon has positive duration");
}

static void test_cast_failure_paths(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    const uint8_t fireball[] = {DM2_RUNE_FUL, DM2_RUNE_IR, 0};
    const uint8_t str_potion[] = {DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_KU, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);

    /* Insufficient skill */
    r = dm2_v1_spell_cast_player(&table, fireball, 0, 100, 0);
    expect_true(!r.cast_success, "low skill fails cast");
    expect_true(r.failure_class == 0x10, "skill failure class 0x10");
    expect_true(r.failure.handled, "failure receipt handled");

    /* Insufficient mana */
    r = dm2_v1_spell_cast_player(&table, fireball, 30, 0, 0);
    expect_true(!r.cast_success, "zero mana fails cast");
    expect_true(r.failure_class == 0x10, "mana failure class 0x10");

    /* Potion without flask */
    r = dm2_v1_spell_cast_player(&table, str_potion, 30, 100, 0);
    expect_true(!r.cast_success, "potion cast fails without flask");
    expect_true(r.failure_class == 0x30, "flask failure class 0x30");
    expect_true(r.failure.flask_pic_drawn, "flask pic drawn receipt");
    expect_true(!r.failure.clears_runes, "class 0x30 keeps runes");

    /* Unknown runes */
    r = dm2_v1_spell_cast_player(&table, (const uint8_t *)"\xFF\xFF", 30, 100, 0);
    expect_true(!r.found, "unknown runes not found");
    expect_true(r.failure_class == 0x20, "unknown failure class 0x20");
}

static void test_resource_spending(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    const uint8_t fireball[] = {DM2_RUNE_FUL, DM2_RUNE_IR, 0};
    const uint8_t light[] = {DM2_RUNE_FUL, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);

    r = dm2_v1_spell_cast_player(&table, light, 30, 0, 0);
    expect_true(r.mana_cost == 0, "Light (1 rune) costs 0 mana");
    expect_true(r.mana_sufficient, "Light mana sufficient even at 0");

    r = dm2_v1_spell_cast_player(&table, fireball, 30, 100, 0);
    expect_true(r.mana_cost > 0, "Fireball has positive mana cost");
    expect_true(r.mana_sufficient, "Fireball mana sufficient at 100");
}

static void test_extended_cast(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    const uint8_t extended_custom[] = {DM2_RUNE_FUL, DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_NETA, 0};
    const uint8_t extended_zok[] = {DM2_RUNE_FUL, DM2_RUNE_ZO, DM2_RUNE_KATH, DM2_RUNE_RA, 0};

    build_extended_receipt(&ext);
    dm2_v1_spell_cast_player_build_table(&ext, &table);

    r = dm2_v1_spell_cast_player(&table, extended_custom, 30, 100, 0);
    expect_true(r.found && r.cast_success,
                "extended custom GENERAL cast succeeds");
    expect_true(r.execution_class == DM2_V1_SPELL_EXEC_GENERAL,
                "extended custom spell is GENERAL");
    expect_true(table.records[r.spell_index].source == 1 &&
                    table.records[r.spell_index].index >= DM2_MAX_SPELL_ORIGINAL,
                "extended spell index is beyond fixed table");

    r = dm2_v1_spell_cast_player(&table, extended_zok, 60, 200, 0);
    expect_true(r.found && r.cast_success, "extended ZoKathRa cast succeeds");
    expect_true(r.execution_class == DM2_V1_SPELL_EXEC_SUMMON,
                "extended ZoKathRa is SUMMON");
}

int main(void)
{
    printf("DM2 V1 Spell Cast Player — DM2-007 source-lock tests\n");
    printf("Source: skproject/SKULLWIN/c_events.cpp:2211-2786\n"
           "        skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)\n");

    test_build_table();
    test_lookup_fixed_and_extended();
    test_cast_success_branches();
    test_cast_failure_paths();
    test_resource_spending();
    test_extended_cast();

    printf("DM2 V1 Spell Cast Player: %d/%d checks passed\n",
           g_checks - g_failures, g_checks);
    return g_failures ? 1 : 0;
}
