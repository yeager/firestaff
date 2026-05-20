#include "dm1_v1_light_pc34_compat.h"
#include "dm1_v1_spell_casting_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>

static void fail_int(const char *id, int actual, int expected) {
    if (actual != expected) {
        printf("FAIL %s actual=%d expected=%d\n", id, actual, expected);
        exit(1);
    }
}

static void fail_true(const char *id, int condition) {
    if (!condition) {
        printf("FAIL %s\n", id);
        exit(1);
    }
}

static DM1_ChampionSpellStats make_stats(void) {
    DM1_ChampionSpellStats stats = {0};
    stats.currentMana = 200;
    stats.maximumMana = 200;
    stats.currentHealth = 50;
    stats.wisdom = 60;
    for (int i = 0; i < 20; ++i) {
        stats.skillLevels[i] = 12;
    }
    return stats;
}

static void cast_light_spell_applies_menu_c_f0412_effect(void) {
    DM1_SpellCastingState spells;
    DM1_LightState light;
    DM1_ChampionSpellStats stats = make_stats();
    const DM1_Spell *spell = NULL;
    int power_ordinal = 0;
    int failure = -1;

    dm1_spell_init(&spells);
    dm1_light_init(&light);

    /* Lo Oh Ir Ra: ReDMCSB spell table entry "Light". */
    fail_true("light.add.lo", dm1_spell_addSymbol(&spells, 0, &stats, DM1_POWER_LO));
    fail_true("light.add.oh", dm1_spell_addSymbol(&spells, 0, &stats, DM1_ELEM_OH));
    fail_true("light.add.ir", dm1_spell_addSymbol(&spells, 0, &stats, DM1_CLASS_IR));
    fail_true("light.add.ra", dm1_spell_addSymbol(&spells, 0, &stats, DM1_ALIGN_RA));

    fail_int("light.cast.success",
             dm1_spell_cast(&spells, 0, &stats, 0, &spell, &power_ordinal, &failure),
             DM1_SPELL_CAST_SUCCESS);
    fail_true("light.cast.spell", spell != NULL);
    fail_int("light.kind", DM1_SPELL_KIND(spell), DM1_SPELL_KIND_OTHER);
    fail_int("light.type", DM1_SPELL_TYPE(spell), DM1_LIGHT_SPELL_TYPE_OTHER_LIGHT);
    fail_int("light.power_ordinal", power_ordinal, 1);

    fail_int("light.apply",
             dm1_light_apply_other_spell_effect(&light, DM1_SPELL_TYPE(spell), power_ordinal),
             1);

    /* MENU.C F0412: spellPower=(1+1)<<2=8; lightPower=(8>>1)-1=3. */
    fail_int("light.magical_amount", light.magical_light_amount,
             dm1_light_power_to_amount[3]);
    fail_int("light.event_count", light.light_event_count, 1);
    fail_int("light.event_power", light.light_events[0].light_power, -3);
    fail_int("light.event_tick", light.light_events[0].expire_tick, 10000);
    fail_int("light.palette", light.dungeon_view_palette_idx, 4);
}

static void cast_darkness_spell_applies_menu_c_f0412_effect(void) {
    DM1_SpellCastingState spells;
    DM1_LightState light;
    DM1_ChampionSpellStats stats = make_stats();
    const DM1_Spell *spell = NULL;
    int power_ordinal = 0;
    int failure = -1;

    dm1_spell_init(&spells);
    dm1_light_init(&light);
    dm1_light_set_champion_count(&light, 1);
    dm1_light_set_torch(&light, 0, 1, 15, true);

    /* Lo Des Ir Sar: ReDMCSB spell table entry "Darkness". */
    fail_true("darkness.add.lo", dm1_spell_addSymbol(&spells, 0, &stats, DM1_POWER_LO));
    fail_true("darkness.add.des", dm1_spell_addSymbol(&spells, 0, &stats, DM1_ELEM_DES));
    fail_true("darkness.add.ir", dm1_spell_addSymbol(&spells, 0, &stats, DM1_CLASS_IR));
    fail_true("darkness.add.sar", dm1_spell_addSymbol(&spells, 0, &stats, DM1_ALIGN_SAR));

    fail_int("darkness.cast.success",
             dm1_spell_cast(&spells, 0, &stats, 0, &spell, &power_ordinal, &failure),
             DM1_SPELL_CAST_SUCCESS);
    fail_true("darkness.cast.spell", spell != NULL);
    fail_int("darkness.kind", DM1_SPELL_KIND(spell), DM1_SPELL_KIND_OTHER);
    fail_int("darkness.type", DM1_SPELL_TYPE(spell), DM1_LIGHT_SPELL_TYPE_OTHER_DARKNESS);
    fail_int("darkness.power_ordinal", power_ordinal, 1);

    fail_int("darkness.apply",
             dm1_light_apply_other_spell_effect(&light, DM1_SPELL_TYPE(spell), power_ordinal),
             1);

    /* MENU.C F0412: spellPower=(1+1)<<2=8; lightPower=8>>2=2. */
    fail_int("darkness.magical_amount", light.magical_light_amount,
             -dm1_light_power_to_amount[2]);
    fail_int("darkness.total_amount", light.total_light_amount,
             100 - dm1_light_power_to_amount[2]);
    fail_int("darkness.event_count", light.light_event_count, 1);
    fail_int("darkness.event_power", light.light_events[0].light_power, 2);
    fail_int("darkness.event_tick", light.light_events[0].expire_tick, 98);
    fail_int("darkness.palette", light.dungeon_view_palette_idx, 1);
}

static void unsupported_or_invalid_spell_effect_does_not_mutate(void) {
    DM1_LightState light;
    dm1_light_init(&light);

    fail_int("unsupported.type", dm1_light_apply_other_spell_effect(&light, 7, 1), 0);
    fail_int("unsupported.ordinal.low", dm1_light_apply_other_spell_effect(&light, 0, 0), 0);
    fail_int("unsupported.ordinal.high", dm1_light_apply_other_spell_effect(&light, 0, 7), 0);
    fail_int("unsupported.amount", light.magical_light_amount, 0);
    fail_int("unsupported.events", light.light_event_count, 0);
}

int main(void) {
    cast_light_spell_applies_menu_c_f0412_effect();
    cast_darkness_spell_applies_menu_c_f0412_effect();
    unsupported_or_invalid_spell_effect_does_not_mutate();
    puts("PASS dm1_v1_light_darkness_spell_bridge_source_lock");
    return 0;
}
