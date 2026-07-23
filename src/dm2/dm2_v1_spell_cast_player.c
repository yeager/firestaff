/* dm2_v1_spell_cast_player.c — DM2-007 DM2_CAST_SPELL_PLAYER bounded slice.
 *
 * Source: skproject/SKULLWIN/c_events.cpp:2211-2786
 *         DM2_FIND_SPELL_BY_RUNES, DM2_TRY_CAST_SPELL, DM2_CAST_SPELL_PLAYER
 *         skproject/SKWIN/SkWinCore.cpp:17521-17670 (CAST_SPELL_PLAYER)
 *
 * The module binds live hero rune strings to validated original spell records,
 * classifies the execution branch, and emits a timer-effect request without
 * mutating champion state or creating objects.
 */

#include "dm2_v1_spell_cast_player.h"

#include <string.h>

/* Source cast-chance constants (SkWinCore.cpp:17535-17555):
 *   bp08 = difficulty + power
 *   bp0c = (wizard_skill + 15) - bp08
 * The source also adds a random term and a difficulty loop; this bounded
 * slice uses the deterministic threshold that the source reaches when the
 * random loop succeeds. */
#define DM2_CAST_SKILL_BONUS 15

/* Default hand cooldown after successful cast.
 * Source: skproject/SKWIN/SkWinCore.cpp:17623 (bp0e = 0x08). */
#define DM2_SPELL_COOLDOWN_BASE 0x08

/* Skill decay on failure.
 * Source: skproject/SKWIN/SkWinCore.cpp:17544. */
#define DM2_SPELL_SKILL_DECAY_ON_FAIL 1

/* Source duration model for bounded timer-effect requests.
 * Light duration: Long Light / Light scale with power in the source;
 * this bounded slice uses the source-visible power factor directly.
 * Aura/Enchantment duration: derived from spell value and power.
 * Summon duration: source uses (RAND02 + skill*2) * power / 6; we emit a
 * deterministic lower bound scaled by power.
 * Cloud duration: source uses BETWEEN_VALUE(21, ((skill<<1)+4)*(power+2), 255). */
#define DM2_SPELL_LIGHT_DURATION(power)      ((power + 1) * 120)
#define DM2_SPELL_AURA_DURATION(power)       ((power + 1) * 180)
#define DM2_SPELL_ENCHANTMENT_DURATION(power) ((power + 1) * 200)
#define DM2_SPELL_SUMMON_DURATION(skill, power) \
    (((skill + 1) * (power + 1) * 30) / 6)
#define DM2_SPELL_CLOUD_DURATION(skill, power) \
    (((((skill) << 1) + 4) * ((power) + 2)) + 21)

/* Fixed-table entries already carry a rune_count derived from the first 0xFF
 * terminator; for runtime lookup we pack the first rune as the POWER rune and
 * the next three symbols into the query key tail. */
/* Pack a runtime record key from the full rune sequence (power rune first,
 * followed by tail runes).  The source DM2_FIND_SPELL_BY_RUNES stores the
 * power rune in the top byte and the tail runes in the low 24 bits; a zero
 * top byte means any power rune matches.  Single-rune spells (e.g. Light)
 * have an empty tail and a power-locked top byte. */
static uint32_t dm2_cast_player_pack_key(const uint8_t *runes, int count)
{
    uint32_t key = 0u;
    int i;

    if (!runes || count < 1) return 0u;
    key |= (uint32_t)runes[0] << 0x18;
    for (i = 1; i < count && i < 4; i++) {
        key |= (uint32_t)runes[i] << (0x18 - (i * 8));
    }
    return key;
}

static void dm2_cast_player_pack_fixed_record(DM2_V1_RuntimeSpellRecord *out,
                                              int index)
{
    const DM2_SpellDefinition *def = dm2_v1_spell_get(index);

    memset(out, 0, sizeof(*out));
    if (!def) return;

    out->index = index;
    out->source = 0;
    out->difficulty = (uint8_t)def->difficulty;
    out->skill = (uint8_t)def->required_skill;
    /* Phase 4 table stores mana_per_rune; map it to the source w6 layout:
     *   spell_type = type low 4 bits
     *   spell_value = object-effect-ish result (upper bits, kept verbatim)
     * The existing fixed table does not encode the full w6 result field, so
     * we keep type from spell_type and spell_value from the OBJECT_EFFECT
     * mapping. */
    out->spell_type = (uint8_t)def->spell_type;
    out->spell_value = 0;
    out->w6 = (uint16_t)(out->spell_type & 0x0fu);

    out->rune_count = def->rune_count;
    if (def->rune_count > 0 && def->rune_count <= 4) {
        memcpy(out->runes, def->rune_symbols, def->rune_count);
    }
    out->key = dm2_cast_player_pack_key(out->runes, out->rune_count);
}

static void dm2_cast_player_pack_extended_record(DM2_V1_RuntimeSpellRecord *out,
                                                 int custom_index,
                                                 const DM2_V1_ExtendedSpellDefinition *src)
{
    memset(out, 0, sizeof(*out));
    out->index = DM2_MAX_SPELL_ORIGINAL + custom_index;
    out->source = 1;
    /* GDAT SPELL_DEF stores rune1/rune2/rune3 as the tail after an implicit
     * power rune.  The source query key tail is rune1<<16|rune2<<8|rune3 with
     * a zero top byte (power-stripped).  We keep the power byte zero because
     * custom spells are not power-locked in the bounded GDAT receipt. */
    out->runes[0] = 0;
    out->runes[1] = src->rune1;
    out->runes[2] = src->rune2;
    out->runes[3] = src->rune3;
    out->rune_count = 4; /* implicit POWER + 3 tail runes */
    out->key = ((uint32_t)src->rune1 << 16) |
               ((uint32_t)src->rune2 << 8) |
               (uint32_t)src->rune3;
    out->difficulty = src->difficulty;
    out->skill = src->requiredSkill;
    out->w6 = src->w6;
    out->spell_value = src->spellValue;
    out->spell_type = (uint8_t)(src->w6 & 0x0fu);
}

void dm2_v1_spell_cast_player_build_table(
    const DM2_V1_ExtendedSpellsReceipt *extended,
    DM2_V1_RuntimeSpellTable *out_table)
{
    int i;
    int fixed_count;

    if (!out_table) return;
    memset(out_table, 0, sizeof(*out_table));
    out_table->extended_mode = (extended && extended->loaded) ? 1 : 0;

    fixed_count = dm2_v1_spell_count();
    if (fixed_count > DM2_MAX_SPELL_ORIGINAL) {
        fixed_count = DM2_MAX_SPELL_ORIGINAL;
    }
    for (i = 0; i < fixed_count; i++) {
        dm2_cast_player_pack_fixed_record(&out_table->records[i], i);
    }
    out_table->count = fixed_count;

    if (!out_table->extended_mode) return;

    for (i = 0; i < DM2_V1_EXT_SPELLS_CUSTOM_COUNT; i++) {
        if (extended->custom[i].loaded) {
            dm2_cast_player_pack_extended_record(
                &out_table->records[out_table->count], i,
                &extended->custom[i]);
            out_table->count++;
        }
    }
}

int dm2_v1_spell_cast_player_find_by_runes(
    const DM2_V1_RuntimeSpellTable *table,
    const uint8_t *runes)
{
    uint32_t query;
    int rune_count = 0;
    int index;

    if (!table || !runes) return -1;
    while (runes[rune_count] != 0 && rune_count < 4) {
        rune_count++;
    }
    if (rune_count < 1) return -1;

    /* Pack the live hero string as power + tail.  Single-rune spells (e.g.
     * Light) are power-locked to that one symbol. */
    query = dm2_cast_player_pack_key(runes, rune_count);
    if (query == 0u) return -1;

    /* c_events.cpp:2236-2262 — reverse scan, masked 24-bit compare when the
     * record key top byte is zero, full 32-bit compare otherwise. */
    for (index = table->count - 1; index >= 0; --index) {
        uint32_t record_key = table->records[index].key;
        if ((record_key & 0xff000000u) == 0u) {
            if ((query & 0x00ffffffu) == record_key) return index;
        } else {
            if (query == record_key) return index;
        }
    }
    return -1;
}

/* Classify the timer-effect request for a resolved record.  This is a bounded
 * slice of the CAST_SPELL_PLAYER switch body that only emits the request; it
 * does not create objects, clouds, or timers. */
static void dm2_cast_player_classify_timer(
    const DM2_V1_RuntimeSpellRecord *rec,
    int wizard_skill,
    int cast_power,
    DM2_V1_SpellCastPlayerReceipt *r)
{
    int spell_index;

    r->timer_kind = DM2_V1_SPELL_TIMER_NONE;
    r->timer_duration = 0;
    r->object_effect = DM2_OBJECT_EFFECT_NONE;

    if (!rec) return;
    spell_index = rec->source ? -1 : rec->index;

    switch (rec->spell_type) {
    case DM2_V1_SPELL_EXEC_POTION:
        r->timer_kind = DM2_V1_SPELL_TIMER_NONE;
        r->flask_required = 1;
        break;
    case DM2_V1_SPELL_EXEC_MISSILE:
        r->timer_kind = DM2_V1_SPELL_TIMER_PROJECTILE;
        if (spell_index >= 0) {
            r->object_effect = dm2_v1_spell_resolves_object_effect(spell_index, 0);
        }
        break;
    case DM2_V1_SPELL_EXEC_SUMMON:
        r->timer_kind = DM2_V1_SPELL_TIMER_SUMMON;
        r->timer_duration = DM2_SPELL_SUMMON_DURATION(wizard_skill, cast_power);
        break;
    case DM2_V1_SPELL_EXEC_GENERAL:
    default:
        /* GENERAL spells: light, auras, enchantments, clouds, item creation. */
        if (spell_index == 0 || spell_index == 5) {
            r->timer_kind = DM2_V1_SPELL_TIMER_LIGHT;
            r->timer_duration = DM2_SPELL_LIGHT_DURATION(cast_power);
        } else if (spell_index >= 6 && spell_index <= 11) {
            r->timer_kind = DM2_V1_SPELL_TIMER_AURA;
            r->timer_duration = DM2_SPELL_AURA_DURATION(cast_power);
        } else if (spell_index == 2 || spell_index == 3 || spell_index == 8) {
            r->timer_kind = DM2_V1_SPELL_TIMER_ENCHANTMENT;
            r->timer_duration = DM2_SPELL_ENCHANTMENT_DURATION(cast_power);
        } else if (spell_index == 12 || spell_index == 14) {
            r->timer_kind = DM2_V1_SPELL_TIMER_CLOUD;
            r->timer_duration = DM2_SPELL_CLOUD_DURATION(wizard_skill, cast_power);
            r->object_effect = dm2_v1_spell_resolves_object_effect(spell_index, 0);
        } else if (spell_index == 13) {
            /* Magical Marker — item creation, no bounded timer. */
            r->timer_kind = DM2_V1_SPELL_TIMER_NONE;
        } else if (spell_index == 20) {
            /* Open/Close Door — no bounded timer effect in this slice. */
            r->timer_kind = DM2_V1_SPELL_TIMER_NONE;
        }
        break;
    }
}

DM2_V1_SpellCastPlayerReceipt dm2_v1_spell_cast_player(
    const DM2_V1_RuntimeSpellTable *table,
    const uint8_t *runes,
    int wizard_skill,
    int current_mana,
    int flask_in_hand)
{
    DM2_V1_SpellCastPlayerReceipt r;
    const DM2_V1_RuntimeSpellRecord *rec;
    int table_index;
    int cast_power;

    memset(&r, 0, sizeof(r));
    r.valid = 1;
    r.wizard_skill = wizard_skill;
    r.current_mana = current_mana;
    r.failure.glob_var = -1;

    if (!table || !runes) {
        r.failure_class = 0x20;
        dm2_v1_spell_proceed_failure(0x20, &r.failure);
        return r;
    }

    table_index = dm2_v1_spell_cast_player_find_by_runes(table, runes);
    if (table_index < 0) {
        r.failure_class = 0x20;
        dm2_v1_spell_proceed_failure(0x20, &r.failure);
        return r;
    }

    r.found = 1;
    r.spell_index = table_index;
    rec = &table->records[table_index];

    /* Cast power is the POWER rune (first symbol).  The source uses the power
     * rune index directly; this bounded slice clamps it to a positive scalar. */
    cast_power = rec->runes[0];
    if (cast_power < 1) cast_power = 1;
    r.cast_power = cast_power;

    /* Source mana cost (c_events.cpp:2282-2289).
     * For fixed-table records the w6 field was reconstructed from spell_type
     * only, so the factor ((w6 >> 10) & 0x3f) is zero.  Fall back to the Phase 4
     * per-rune mana cost from the fixed table so potion/missile casts still
     * carry a realistic resource receipt. */
    {
        DM2_V1_SpellRecord tmp;
        tmp.key = rec->key;
        tmp.difficulty = rec->difficulty;
        tmp.skill = rec->skill;
        tmp.w6 = rec->w6;
        r.mana_cost = dm2_v1_spell_record_mana_cost(&tmp, cast_power);
    }
    if (r.mana_cost < 0 || (rec->source == 0 && r.mana_cost == 0)) {
        int fallback = dm2_v1_spell_mana_cost(rec->index);
        if (fallback > 0) r.mana_cost = fallback;
    }
    r.mana_sufficient = (current_mana >= r.mana_cost) ? 1 : 0;

    /* Source cast-chance math (SkWinCore.cpp:17535-17555):
     *   bp08 = difficulty + power
     *   bp0c = (wizard_skill + 15) - bp08
     * The full source adds random terms and a difficulty loop; this bounded
     * slice treats bp0c > 0 as success, matching the deterministic threshold
     * after the loop. */
    r.bp08 = (int)rec->difficulty + cast_power;
    r.bp0c = (wizard_skill + DM2_CAST_SKILL_BONUS) - r.bp08;

    /* Skill-gate: source queries QUERY_PLAYER_SKILL_LV; if the champion does
     * not meet the required skill, the cast fails with class 0x10. */
    if (wizard_skill < (int)rec->skill) {
        r.cast_success = 0;
        r.skill_decay = DM2_SPELL_SKILL_DECAY_ON_FAIL;
        r.failure_class = 0x10;
        dm2_v1_spell_proceed_failure(0x10, &r.failure);
        return r;
    }

    if (!r.mana_sufficient) {
        r.cast_success = 0;
        r.skill_decay = DM2_SPELL_SKILL_DECAY_ON_FAIL;
        /* Mana failure in the source is folded into the cast loop; this slice
         * reports it as class 0x10 (insufficient resources) so the UI can
         * display the standard failure message. */
        r.failure_class = 0x10;
        dm2_v1_spell_proceed_failure(0x10, &r.failure);
        return r;
    }

    if (r.bp0c > 0) {
        r.cast_success = 1;
        /* Source cooldown: bp0e = (w6_a_f * (power + 18)) / 24.
         * The Phase 4 fixed table uses a flat 0x08 cooldown.  This slice
         * preserves the source formula when w6 carries a factor, otherwise
         * falls back to the Phase 4 default. */
        {
            int factor = (int)((rec->w6 >> 10) & 0x3fu);
            if (factor > 0) {
                r.cooldown_ticks = factor * (cast_power + 18) / 24;
            } else {
                r.cooldown_ticks = DM2_SPELL_COOLDOWN_BASE;
            }
        }
        r.skill_decay = 0;
    } else {
        r.cast_success = 0;
        r.cooldown_ticks = 0;
        r.skill_decay = DM2_SPELL_SKILL_DECAY_ON_FAIL;
        r.failure_class = 0x10;
        dm2_v1_spell_proceed_failure(0x10, &r.failure);
        return r;
    }

    /* Execution branch classification (SkWinCore.cpp:17563). */
    r.execution_class = rec->spell_type;
    dm2_cast_player_classify_timer(rec, wizard_skill, cast_power, &r);

    /* POTION branch resource gate: requires an empty flask in hand.
     * Source: c_events.cpp / CAST_SPELL_PLAYER POTION case. */
    if (r.execution_class == DM2_V1_SPELL_EXEC_POTION && !flask_in_hand) {
        r.cast_success = 0;
        r.cooldown_ticks = 0;
        r.skill_decay = 0;
        r.failure_class = 0x30;
        dm2_v1_spell_proceed_failure(0x30, &r.failure);
        return r;
    }

    if (r.execution_class == DM2_V1_SPELL_EXEC_POTION && flask_in_hand) {
        r.flask_required = 1;
        r.flask_consumed = 1;
    }

    return r;
}

const char *dm2_v1_spell_cast_player_source_evidence(void)
{
    return
        "DM2 V1 Spell Cast Player — DM2-007 bounded slice\n"
        "Source: skproject/SKULLWIN/c_events.cpp:2211-2264 DM2_FIND_SPELL_BY_RUNES\n"
        "Source: skproject/SKULLWIN/c_events.cpp:2282-2289 mana cost formula\n"
        "Source: skproject/SKULLWIN/c_events.cpp:2687-2786 DM2_PROCEED_SPELL_FAILURE / DM2_TRY_CAST_SPELL\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:17521-17670 CAST_SPELL_PLAYER\n"
        "Source: skproject/SKWIN/SkGlobal.cpp:966-1011 dSpellsTable (34 fixed spells)\n"
        "Source: skproject/SKWIN/SkWinCore.cpp:189 EXTENDED_LOAD_SPELLS_DEFINITION\n";
}
