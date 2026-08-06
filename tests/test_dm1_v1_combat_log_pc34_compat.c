/*
 * Test dm1_v1_combat_log_pc34_compat — header-level tests only.
 * The combat log source is excluded from firestaff_m10 (has QoL runtime
 * dependencies), so we test enum values, struct layout, and aliases.
 */
#include "dm1_v1_combat_log_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_enum_values(void) {
    assert(DM1_V1_COMBAT_LOG_TYPE_INFO == 0);
    assert(DM1_V1_COMBAT_LOG_TYPE_CHAMP_HIT == 1);
    assert(DM1_V1_COMBAT_LOG_TYPE_CREATURE_HIT == 2);
    assert(DM1_V1_COMBAT_LOG_TYPE_SPELL == 3);
    assert(DM1_V1_COMBAT_LOG_TYPE_MISS == 4);
}

static void test_enum_aliases(void) {
    assert(M11_COMBAT_LOG_TYPE_INFO == DM1_V1_COMBAT_LOG_TYPE_INFO);
    assert(M11_COMBAT_LOG_TYPE_CHAMP_HIT == DM1_V1_COMBAT_LOG_TYPE_CHAMP_HIT);
    assert(M11_COMBAT_LOG_TYPE_CREATURE_HIT == DM1_V1_COMBAT_LOG_TYPE_CREATURE_HIT);
    assert(M11_COMBAT_LOG_TYPE_SPELL == DM1_V1_COMBAT_LOG_TYPE_SPELL);
    assert(M11_COMBAT_LOG_TYPE_MISS == DM1_V1_COMBAT_LOG_TYPE_MISS);
}

static void test_entry_struct_layout(void) {
    DM1_V1_CombatLogEntryPc34 entry;
    memset(&entry, 0, sizeof(entry));
    assert(sizeof(entry.text) == 128);

    entry.gameTick = 0xDEADBEEF;
    assert(entry.gameTick == 0xDEADBEEF);

    entry.type = DM1_V1_COMBAT_LOG_TYPE_MISS;
    assert(entry.type == 4);
    (void)entry;
}

static void test_typedef_aliases(void) {
    /* M11_CombatLogType and M11_CombatLogEntry are typedefs */
    M11_CombatLogType t = DM1_V1_COMBAT_LOG_TYPE_SPELL;
    assert(t == 3);
    (void)t;

    M11_CombatLogEntry e;
    memset(&e, 0, sizeof(e));
    e.type = DM1_V1_COMBAT_LOG_TYPE_INFO;
    assert(e.type == 0);
    (void)e;
}

static void test_text_field_capacity(void) {
    DM1_V1_CombatLogEntryPc34 entry;
    /* Fill text to max, ensure no overflow */
    memset(entry.text, 'A', sizeof(entry.text) - 1);
    entry.text[sizeof(entry.text) - 1] = '\0';
    assert(strlen(entry.text) == 127);
    (void)entry;
}

static void test_source_font_gate(void) {
    assert(DM1_CombatLog_SourceAllowsFallbackFont(
               M11_GAME_SOURCE_BUILTIN_CATALOG) == 0);
    assert(DM1_CombatLog_SourceAllowsFallbackFont(
               M11_GAME_SOURCE_CUSTOM_DUNGEON) == 0);
    assert(DM1_CombatLog_SourceAllowsFallbackFont(
               M11_GAME_SOURCE_DIRECT_DUNGEON) == 0);
    assert(DM1_CombatLog_SourceAllowsFallbackFont(
               M11_GAME_SOURCE_CSB_BOOT) == 1);
    assert(DM1_CombatLog_SourceAllowsFallbackFont(
               M11_GAME_SOURCE_DM2_BOOT) == 1);
}

int main(void) {
    test_enum_values();
    test_enum_aliases();
    test_entry_struct_layout();
    test_typedef_aliases();
    test_text_field_capacity();
    test_source_font_gate();
    puts("ok: dm1_v1_combat_log_pc34_compat 6 tests passed");
    return 0;
}
