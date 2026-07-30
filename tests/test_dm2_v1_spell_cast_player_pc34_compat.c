/* test_dm2_v1_spell_cast_player_pc34_compat.c — DM2-007 runtime spell cast
 * slice: live hero rune lookup, resource spending, execution branches, and
 * timer-effect requests.
 *
 * Source: skproject/SKULLWIN/c_events.cpp:2211-2786
 *         skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)
 */

#include "dm2_v1_spell_cast_player.h"
#include "dm2_v1_spell_timer_handlers_pc34_compat.h"
#include "dm2_v1_timeline.h"

#include "dm2_v1_caii_alloc_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"
#include "dm2_v1_record_pool_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
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

static void init_champion_for_apply(DM2_ChampionRecord *champ)
{
    memset(champ, 0, sizeof(*champ));
    champ->mana = 100u;
    champ->runes_count = 2u;
    champ->spelled_runes[0] = DM2_RUNE_FUL;
    champ->spelled_runes[1] = DM2_RUNE_IR;
    champ->hand_cooldown[0] = 0u;
    champ->hand_cooldown[1] = 0u;
}

static void test_apply_success_light(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    DM2_V1_SpellCastApplyReceipt a;
    DM2_ChampionRecord champ;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_SourceTimer t;
    const uint8_t light[] = {DM2_RUNE_FUL, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    init_champion_for_apply(&champ);
    dm2_v1_source_timer_queue_init(&queue);

    r = dm2_v1_spell_cast_player(&table, light, 30, 100, 0);
    a = dm2_v1_spell_cast_player_apply(&r, &champ, 0, NULL, &queue,
                                       100u, 0, 15, 15, 2);

    expect_true(a.valid, "apply receipt valid for Light");
    expect_true(a.applied, "Light apply mutated state");
    expect_true(a.mana_after == 100, "Light mana unchanged (cost 0)");
    expect_true(champ.hand_cooldown[0] == (uint16_t)r.cooldown_ticks,
                "Light sets hand cooldown");
    expect_true(champ.runes_count == 0 && champ.spelled_runes[0] == 0,
                "Light clears rune tail");
    expect_true(a.timer_enqueued && a.timer_ticket != 0u,
                "Light enqueues a source timer");
    expect_true(a.timer_kind == DM2_V1_SPELL_TIMER_LIGHT,
                "Light timer kind recorded");
    expect_true(dm2_v1_source_timer_peek_ticket(&queue, a.timer_ticket, &t),
                "Light timer ticket is live");
    expect_true(t.type == 0x46, "Light timer type is 0x46");
    expect_true(t.actor == 2, "Light timer actor is champion index");
    expect_true((t.ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK) ==
                    (uint32_t)(100 + r.timer_duration),
                "Light timer due tick matches duration");
}

static void test_apply_success_fireball(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    DM2_V1_SpellCastApplyReceipt a;
    DM2_ChampionRecord champ;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_SourceTimer t;
    const uint8_t fireball[] = {DM2_RUNE_FUL, DM2_RUNE_IR, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    init_champion_for_apply(&champ);
    dm2_v1_source_timer_queue_init(&queue);

    r = dm2_v1_spell_cast_player(&table, fireball, 30, 100, 0);
    a = dm2_v1_spell_cast_player_apply(&r, &champ, 1, NULL, &queue,
                                       200u, 1, 20, 21, 3);

    expect_true(a.valid && a.applied, "Fireball apply valid");
    expect_true(a.mana_consumed > 0 && a.mana_after < 100,
                "Fireball consumes mana");
    expect_true(champ.hand_cooldown[1] == (uint16_t)r.cooldown_ticks,
                "Fireball sets hand 1 cooldown");
    expect_true(a.timer_enqueued && a.timer_kind == DM2_V1_SPELL_TIMER_PROJECTILE,
                "Fireball enqueues projectile timer");
    expect_true(dm2_v1_source_timer_peek_ticket(&queue, a.timer_ticket, &t),
                "Fireball timer ticket live");
    expect_true(t.type == 0x1e, "Fireball timer type is 0x1e");
    expect_true(t.reserved == DM2_OBJECT_EFFECT_FIREBALL,
                "Fireball timer carries FIREBALL object effect");
    expect_true((t.ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK) == 201u,
                "Fireball timer due on next tick (duration 0)");
}

static void test_apply_success_potion(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    DM2_V1_SpellCastApplyReceipt a;
    DM2_ChampionRecord champ;
    DM2_LeaderPossession flask;
    DM2_V1_SourceTimerQueue queue;
    const uint8_t str_potion[] = {DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_KU, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    init_champion_for_apply(&champ);
    dm2_v1_source_timer_queue_init(&queue);
    flask.object = 0x1234u;

    r = dm2_v1_spell_cast_player(&table, str_potion, 30, 100, 1);
    a = dm2_v1_spell_cast_player_apply(&r, &champ, 0, &flask, &queue,
                                       100u, 0, 15, 15, 0);

    expect_true(a.valid && a.applied, "Potion apply valid");
    expect_true(a.flask_consumed && flask.object == 0u,
                "Potion consumes the empty flask object");
    expect_true(!a.timer_enqueued, "Potion does not enqueue a timer");
    expect_true(champ.hand_cooldown[0] > 0, "Potion sets cooldown");
    expect_true(champ.runes_count == 0, "Potion clears runes");
}

static void test_apply_failure_skill(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    DM2_V1_SpellCastApplyReceipt a;
    DM2_ChampionRecord champ;
    DM2_V1_SourceTimerQueue queue;
    const uint8_t fireball[] = {DM2_RUNE_FUL, DM2_RUNE_IR, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    init_champion_for_apply(&champ);
    dm2_v1_source_timer_queue_init(&queue);

    r = dm2_v1_spell_cast_player(&table, fireball, 0, 100, 0);
    a = dm2_v1_spell_cast_player_apply(&r, &champ, 0, NULL, &queue,
                                       100u, 0, 15, 15, 0);

    expect_true(!r.cast_success && r.failure_class == 0x10,
                "skill failure produces class 0x10");
    expect_true(a.valid && a.failure_feedback, "apply reports failure feedback");
    expect_true(a.mana_after == 100, "failure does not consume mana");
    expect_true(champ.hand_cooldown[0] == 0, "failure does not set cooldown");
    expect_true(a.runes_cleared && champ.runes_count == 0,
                "class 0x10 failure clears runes");
    expect_true(!a.timer_enqueued && queue.count == 0,
                "failure does not enqueue timer");
}

static void test_apply_failure_flask(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    DM2_V1_SpellCastApplyReceipt a;
    DM2_ChampionRecord champ;
    const uint8_t str_potion[] = {DM2_RUNE_FUL, DM2_RUNE_BRO, DM2_RUNE_KU, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    init_champion_for_apply(&champ);

    r = dm2_v1_spell_cast_player(&table, str_potion, 30, 100, 0);
    a = dm2_v1_spell_cast_player_apply(&r, &champ, 0, NULL, NULL,
                                       100u, 0, 15, 15, 0);

    expect_true(!r.cast_success && r.failure_class == 0x30,
                "no-flask failure produces class 0x30");
    expect_true(a.valid && a.failure_feedback, "apply reports flask feedback");
    expect_true(!a.runes_cleared && champ.runes_count == 2u,
                "class 0x30 failure keeps runes");
    expect_true(a.mana_after == 100 && champ.hand_cooldown[0] == 0,
                "flask failure consumes nothing");
}

static void test_apply_no_queue(void)
{
    DM2_V1_RuntimeSpellTable table;
    DM2_V1_ExtendedSpellsReceipt ext;
    DM2_V1_SpellCastPlayerReceipt r;
    DM2_V1_SpellCastApplyReceipt a;
    DM2_ChampionRecord champ;
    const uint8_t light[] = {DM2_RUNE_FUL, 0};

    memset(&ext, 0, sizeof(ext));
    dm2_v1_spell_cast_player_build_table(&ext, &table);
    init_champion_for_apply(&champ);

    r = dm2_v1_spell_cast_player(&table, light, 30, 100, 0);
    a = dm2_v1_spell_cast_player_apply(&r, &champ, 0, NULL, NULL,
                                       100u, 0, 15, 15, 0);

    expect_true(a.valid && a.applied, "Light apply without queue is valid");
    expect_true(champ.runes_count == 0, "Light clears runes without queue");
    expect_true(!a.timer_enqueued && a.timer_ticket == 0u,
                "NULL queue means no timer enqueued");
}

/* Helper to build a source timer for the spell handler tests. */
static DM2_V1_SourceTimer make_spell_timer(uint32_t tick, int map,
                                           uint8_t type, uint8_t actor,
                                           int16_t value_a)
{
    DM2_V1_SourceTimer t;
    memset(&t, 0, sizeof(t));
    t.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                      (tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
    t.type = type;
    t.actor = actor;
    t.value_a = value_a;
    return t;
}

static DM2_V1_SourceTimer make_spell_timer_ex(uint32_t tick, int map,
                                              uint8_t type, uint8_t actor,
                                              int16_t value_a,
                                              int16_t value_b,
                                              int16_t reserved)
{
    DM2_V1_SourceTimer t;
    memset(&t, 0, sizeof(t));
    t.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                      (tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
    t.type = type;
    t.actor = actor;
    t.value_a = value_a;
    t.value_b = value_b;
    t.reserved = reserved;
    return t;
}

static void test_spell_timer_light_requeue(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;
    DM2_V1_SourceTimer peek;
    uint32_t requeue_ticket;

    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, NULL, 0, &queue, 100u, 0);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer(100u, 0, DM2_V1_TIMER_LIGHT, 0, 3);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    expect_true(dm2_v1_proceed_timers(&queue, 100u, &dispatcher, &receipt),
                "light handler dispatch ran");
    expect_true(receipt.dispatched_count == 1, "light timer consumed");
    expect_true(ctx.receipt.light_dispatched == 1, "light receipt flagged");
    expect_true(ctx.light_remaining == 2, "light duration decremented to 2");
    expect_true(queue.count == 1, "light requeued one step");
    requeue_ticket = queue.tickets[0];
    expect_true(requeue_ticket != 0u, "requeued ticket is non-zero");
    expect_true(dm2_v1_source_timer_peek_ticket(&queue, requeue_ticket, &peek),
                "requeued light ticket is live");
    expect_true(peek.type == DM2_V1_TIMER_LIGHT, "requeued timer is light");
    expect_true(peek.value_a == 2, "requeued light carries remaining duration");
    expect_true((peek.ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK) ==
                    100u + DM2_V1_SPELL_TIMER_LIGHT_REQUEUE_DELAY,
                "requeued light due in 8 ticks");
}

static void test_spell_timer_hero_ench_flag(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_ChampionRecord champs[4];
    DM2_V1_SourceTimer t;

    memset(champs, 0, sizeof(champs));
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, champs, 4, &queue, 0u, 0);
    /* One active aura refcount; the single 0x47 pop decrements it to zero
     * and sets the aura bit, matching c_events.cpp case 3 + c_tim_proc.cpp
     * 0x47 handler shape. */
    ctx.hero_ench_countdown = 1;
    ctx.hero_ench_target_index = 2;
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer(1u, 0, DM2_V1_TIMER_HERO_ENCH_FLAG, 2, 0);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.hero_ench_countdown_expired == 1,
                "hero enchantment countdown expired");
    expect_true(ctx.receipt.hero_ench_flag_set == 1,
                "hero enchantment flag was set");
    expect_true((champs[2].hero_flag & DM2_V1_SPELL_TIMER_HEROFLAG_AURA_BIT) != 0,
                "target champion hero_flag has aura bit");
    expect_true(ctx.hero_ench_countdown == 0,
                "hero enchantment refcount reached zero");
}

static void test_spell_timer_ench_power_decay(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_ChampionRecord champs[4];
    DM2_V1_SourceTimer t;

    memset(champs, 0, sizeof(champs));
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, champs, 4, &queue, 0u, 0);
    ctx.ench_power[1] = 50;
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer(1u, 0, DM2_V1_TIMER_ENCH_POWER, 1 << 1, 7);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.ench_power_dispatched == 1,
                "ench power handler dispatched");
    expect_true(ctx.ench_power[1] == 43, "ench power decayed by 7");
    expect_true(ctx.receipt.ench_power_decays[1] == 7,
                "ench power decay recorded");
    expect_true(champs[1].body_flag == 43, "ench power written to body_flag");
}

static void test_spell_timer_poison_decay(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_ChampionRecord champs[4];
    DM2_V1_SourceTimer t;

    memset(champs, 0, sizeof(champs));
    champs[0].poison_value = 5;
    ctx.poison_strength[0] = 10;
    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, champs, 4, &queue, 0u, 0);
    ctx.poison_strength[0] = 10;
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer(1u, 0, DM2_V1_TIMER_POISON, 0, 2);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.poison_dispatched == 1, "poison handler dispatched");
    expect_true(champs[0].poison_value == 4, "poison_value decremented");
    expect_true(ctx.poison_strength[0] == 8, "poison strength decayed by 2");
    expect_true(ctx.receipt.poison_value_decays[0] == 1,
                "poison value decay recorded");
}

static void test_spell_timer_cloud_fail_closed(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;

    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, NULL, 0, &queue, 1u, 0);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer_ex(1u, 0, DM2_V1_TIMER_PROCESS_CLOUD, 0,
                            7, 9, DM2_OBJECT_EFFECT_POISON_CLOUD);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.cloud_dispatched == 1,
                "cloud handler dispatched");
    expect_true(ctx.receipt.cloud_origin_x == 7,
                "cloud origin x recorded");
    expect_true(ctx.receipt.cloud_origin_y == 9,
                "cloud origin y recorded");
    expect_true(ctx.receipt.cloud_object_effect == DM2_OBJECT_EFFECT_POISON_CLOUD,
                "cloud object effect recorded");
    expect_true(ctx.receipt.cloud_record_creation_failed == 1,
                "cloud fails closed without DB14 owner");
    expect_true(receipt.dispatched_count == 1,
                "cloud timer consumed");
}

static void test_spell_timer_projectile_fireball(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;

    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, NULL, 0, &queue, 1u, 0);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer_ex(1u, 0, DM2_V1_TIMER_STEP_MISSILE, 0,
                            12, 14, DM2_OBJECT_EFFECT_FIREBALL);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.missile_dispatched == 1,
                "missile handler dispatched");
    expect_true(ctx.receipt.missile_origin_x == 12,
                "missile origin x recorded");
    expect_true(ctx.receipt.missile_origin_y == 14,
                "missile origin y recorded");
    expect_true(ctx.receipt.missile_object_effect == DM2_OBJECT_EFFECT_FIREBALL,
                "missile object effect recorded");
    expect_true(ctx.receipt.missile_projectile_accepted == 0 &&
                ctx.receipt.missile_projectile_slot < 0,
                "fireball rejects cache projectile without source owner state");
    expect_true(receipt.dispatched_count == 1,
                "missile timer consumed");
}

static void test_spell_timer_projectile_reject_unknown(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;

    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, NULL, 0, &queue, 1u, 0);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer_ex(1u, 0, DM2_V1_TIMER_STEP_MISSILE, 0,
                            5, 6, DM2_OBJECT_EFFECT_PUSH_SPELL);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.missile_dispatched == 1,
                "missile handler dispatched for unknown effect");
    expect_true(ctx.receipt.missile_projectile_accepted == 0,
                "unknown object effect rejects projectile creation");
    expect_true(receipt.dispatched_count == 1,
                "unknown-effect missile timer still consumed");
}

static void test_spell_timer_summon_fail_closed(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;

    dm2_v1_source_timer_queue_init(&queue);
    dm2_v1_spell_timer_handler_context_init(&ctx, NULL, 0, &queue, 1u, 0);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer_ex(1u, 0, DM2_V1_TIMER_ALLOC_NEW_CREATURE, 0,
                            3, 4, DM2_OBJECT_EFFECT_SUMMON_ATTACK_MINION);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.summon_dispatched == 1,
                "summon handler dispatched");
    expect_true(ctx.receipt.summon_origin_x == 3,
                "summon origin x recorded");
    expect_true(ctx.receipt.summon_origin_y == 4,
                "summon origin y recorded");
    expect_true(ctx.receipt.summon_failed_no_data == 1,
                "summon fails closed without real DB4/CAII data");
    expect_true(receipt.dispatched_count == 1,
                "summon timer consumed");
}

static void test_spell_timer_source_evidence(void)
{
    const char *ev = dm2_v1_spell_timer_handlers_source_evidence();
    expect_true(strstr(ev, "DM2_PROCESS_TIMER_LIGHT") != NULL,
                "source evidence cites light handler");
    expect_true(strstr(ev, "c_tim_proc.cpp") != NULL,
                "source evidence cites c_tim_proc.cpp");
    expect_true(strstr(ev, "DM2_PROCESS_TIMER_19") != NULL,
                "source evidence cites cloud handler");
    expect_true(strstr(ev, "DM2_STEP_MISSILE") != NULL,
                "source evidence cites missile handler");
    expect_true(strstr(ev, "DM2_ALLOC_NEW_CREATURE") != NULL,
                "source evidence cites summon handler");
}

/* ── Synthetic real-data fixtures for the cycle-13 handler tests ───────── */

static void init_test_record_pool(DM2_V1_RecordPoolSet *set)
{
    static const int pools_to_fill[] = { 4, 10, 14 };
    size_t i;

    memset(set, 0, sizeof(*set));
    for (i = 0; i < sizeof(pools_to_fill) / sizeof(pools_to_fill[0]); ++i) {
        int db = pools_to_fill[i];
        DM2_V1_RecordPool *p = &set->pools[db];
        int size = dm2_v1_record_pool_record_size(db);
        int count = 4;
        size_t bytes;

        p->record_size = size;
        p->record_count = count;
        bytes = (size_t)count * (size_t)size;
        p->bytes = (uint8_t *)malloc(bytes);
        memset(p->bytes, 0xff, bytes);
    }
    set->valid = 1;
}

static void free_test_record_pool(DM2_V1_RecordPoolSet *set)
{
    int i;

    if (set == NULL) return;
    for (i = 0; i < DM2_V1_RECORD_POOL_COUNT; ++i) {
        free(set->pools[i].bytes);
        set->pools[i].bytes = NULL;
        set->pools[i].record_count = 0;
        set->pools[i].record_size = 0;
    }
    set->valid = 0;
}

static void init_test_dungeon(DM2_V1_DungeonData *d)
{
    /* 1x1 byte-square map at (0,0) with the object flag set.
     * Layout: byte 0 = tile, bytes 1-2 = column index, bytes 3-4 = first thing. */
    memset(d, 0, sizeof(*d));
    d->raw_data = (uint8_t *)malloc(8);
    d->raw_size = 8;
    d->raw_data[0] = 0x11u;                 /* floor + object flag */
    d->raw_data[1] = 0x00u;                 /* column index (low) */
    d->raw_data[2] = 0x00u;                 /* column index (high) */
    d->raw_data[3] = 0xfeu;                 /* first thing = END marker */
    d->raw_data[4] = 0xffu;
    d->level_count = 1;
    d->level_widths[0] = 1;
    d->level_heights[0] = 1;
    d->level_offsets[0] = 0;
    d->square_bytes = 1;
    d->raw_map_data_base = 0;
    d->column_index_base = 1;
    d->square_first_thing_base = 3;
    d->square_first_thing_count = 1;
}

static void free_test_dungeon(DM2_V1_DungeonData *d)
{
    if (d == NULL) return;
    free(d->raw_data);
    d->raw_data = NULL;
}

static void test_spell_timer_cloud_real_data(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;
    DM2_V1_RecordPoolSet pool_set;
    DM2_V1_DungeonData dungeon;

    dm2_v1_source_timer_queue_init(&queue);
    init_test_record_pool(&pool_set);
    init_test_dungeon(&dungeon);
    dm2_v1_spell_timer_handler_context_init_ex(
        &ctx, NULL, 0, &queue, 10u, 0, &pool_set, &dungeon, NULL);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer_ex(10u, 0, DM2_V1_TIMER_PROCESS_CLOUD, 0,
                            0, 0, DM2_OBJECT_EFFECT_POISON_CLOUD);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 10u, &dispatcher, &receipt);

    expect_true(ctx.receipt.cloud_dispatched == 1,
                "cloud real-data handler dispatched");
    expect_true(ctx.receipt.cloud_record_created == 1,
                "cloud allocated a DB14 record");
    expect_true(ctx.receipt.cloud_record_handle != DM2_V1_RECORD_HANDLE_NULL,
                "cloud DB14 handle is valid");
    expect_true(ctx.receipt.cloud_duration_remaining ==
                    DM2_V1_SPELL_TIMER_CLOUD_INITIAL_DURATION - 1,
                "cloud duration decremented after first pop");
    expect_true(ctx.receipt.cloud_requeued == 1,
                "cloud requeued while alive");
    expect_true(queue.count == 1,
                "cloud requeued one timer");

    /* Step the requeued timer until the cloud expires. */
    while (queue.count > 0) {
        DM2_V1_SourceTimer peek;
        uint32_t tick;

        expect_true(dm2_v1_source_timer_peek_ticket(
                        &queue, queue.tickets[0], &peek),
                    "cloud requeued ticket live");
        tick = peek.ticks_and_map & DM2_V1_SOURCE_TIMER_TICK_MASK;
        dm2_v1_proceed_timers(&queue, tick, &dispatcher, &receipt);
    }
    expect_true(ctx.receipt.cloud_duration_remaining == 0,
                "cloud duration reached zero");

    free_test_record_pool(&pool_set);
    free_test_dungeon(&dungeon);
}

static void test_spell_timer_projectile_real_data(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;
    DM2_V1_RecordPoolSet pool_set;
    DM2_V1_DungeonData dungeon;

    dm2_v1_source_timer_queue_init(&queue);
    init_test_record_pool(&pool_set);
    init_test_dungeon(&dungeon);
    dm2_v1_spell_timer_handler_context_init_ex(
        &ctx, NULL, 0, &queue, 1u, 0, &pool_set, &dungeon, NULL);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer_ex(1u, 0, DM2_V1_TIMER_STEP_MISSILE, 0,
                            0, 0, DM2_OBJECT_EFFECT_FIREBALL);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.missile_dispatched == 1,
                "missile real-data handler dispatched");
    expect_true(ctx.receipt.missile_record_created == 1,
                "missile allocated a DB14 flying-item record");
    expect_true(ctx.receipt.missile_object_handle != DM2_V1_RECORD_HANDLE_NULL,
                "missile DB10 object handle is valid");
    expect_true(ctx.receipt.missile_projectile_accepted == 0 &&
                ctx.receipt.missile_projectile_slot < 0,
                "DB14 record does not fabricate a cache projectile");

    free_test_record_pool(&pool_set);
    free_test_dungeon(&dungeon);
}

static void test_spell_timer_summon_real_data(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_TimerDispatcher dispatcher;
    DM2_V1_SpellTimerHandlerContext ctx;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_SourceTimer t;
    DM2_V1_RecordPoolSet pool_set;
    DM2_V1_DungeonData dungeon;
    DM2_V1_CaiiArray caii;

    dm2_v1_source_timer_queue_init(&queue);
    init_test_record_pool(&pool_set);
    init_test_dungeon(&dungeon);
    dm2_v1_caii_array_init(&caii, 4);

    dm2_v1_spell_timer_handler_context_init_ex(
        &ctx, NULL, 0, &queue, 1u, 0, &pool_set, &dungeon, &caii);
    memset(&dispatcher, 0, sizeof(dispatcher));
    dispatcher.context = &ctx;
    dm2_v1_spell_timer_handlers_install(&dispatcher, &ctx);

    t = make_spell_timer_ex(1u, 0, DM2_V1_TIMER_ALLOC_NEW_CREATURE, 0,
                            0, 0, DM2_OBJECT_EFFECT_SUMMON_ATTACK_MINION);
    dm2_v1_source_timer_enqueue(&queue, &t, 0);

    dm2_v1_proceed_timers(&queue, 1u, &dispatcher, &receipt);

    expect_true(ctx.receipt.summon_dispatched == 1,
                "summon real-data handler dispatched");
    expect_true(ctx.receipt.summon_record_created == 1,
                "summon allocated a DB4 creature record");
    expect_true(ctx.receipt.summon_creature_type == 14,
                "attack minion maps to creature type 14");
    expect_true(ctx.receipt.summon_caii_allocated == 1,
                "summon activated a CAII slot");
    expect_true(ctx.receipt.summon_timer_scheduled == 1,
                "summon scheduled a think timer");
    expect_true(queue.count == 1,
                "summon left the creature think timer in the queue");

    dm2_v1_caii_array_free(&caii);
    free_test_record_pool(&pool_set);
    free_test_dungeon(&dungeon);
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
    test_apply_success_light();
    test_apply_success_fireball();
    test_apply_success_potion();
    test_apply_failure_skill();
    test_apply_failure_flask();
    test_apply_no_queue();

    test_spell_timer_light_requeue();
    test_spell_timer_hero_ench_flag();
    test_spell_timer_ench_power_decay();
    test_spell_timer_poison_decay();
    test_spell_timer_cloud_fail_closed();
    test_spell_timer_cloud_real_data();
    test_spell_timer_projectile_fireball();
    test_spell_timer_projectile_reject_unknown();
    test_spell_timer_projectile_real_data();
    test_spell_timer_summon_fail_closed();
    test_spell_timer_summon_real_data();
    test_spell_timer_source_evidence();

    printf("DM2 V1 Spell Cast Player: %d/%d checks passed\n",
           g_checks - g_failures, g_checks);
    return g_failures ? 1 : 0;
}
