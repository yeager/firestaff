#include "csb_v1_magic_rune_cost_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void check_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL %s: got %d expected %d\n", name, actual, expected);
        ++failures;
    }
}

int main(void)
{
    uint8_t graphic[CSB_V1_MAGIC_RUNE_TABLE_DECODED_SIZE_PC34];
    CSB_V1_MagicRuneCostTablePc34 table;
    CSB_V1_MagicSpellTablePc34 spell_table;
    const CSB_V1_MagicSpellPc34 *spell;
    const uint8_t fireball[4] = { 0x69u, 0x6fu, 0u, 0u };
    int cost = -1;

    memset(graphic, 0, sizeof(graphic));
    graphic[CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 + 0] = 8;
    graphic[CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 + 1] = 12;
    graphic[CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 + 2] = 16;
    graphic[CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 + 3] = 20;
    graphic[CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 + 4] = 24;
    graphic[CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 + 5] = 28;
    graphic[CSB_V1_MAGIC_RUNE_BASE_COST_OFFSET_PC34 + 0] = 1;
    graphic[CSB_V1_MAGIC_RUNE_BASE_COST_OFFSET_PC34 + 6 + 2] = 7;
    /* CSBWin Data.h spell [08]: 4-4, Fireball, class 2. */
    graphic[CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 + 8u * 8u + 1u] = 0x69u;
    graphic[CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 + 8u * 8u + 2u] = 0x6fu;
    graphic[CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 + 8u * 8u + 4u] = 3u;
    graphic[CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 + 8u * 8u + 5u] = 4u;
    graphic[CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 + 8u * 8u + 7u] = 2u;
    /* Minimal valid source records for the remaining table slots. */
    for (unsigned int i = 0u; i < CSB_V1_MAGIC_SPELL_COUNT_PC34; ++i) {
        unsigned int offset = CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 + i * 8u;
        if (i == 8u) continue;
        graphic[offset + 1u] = 0x60u + (uint8_t)i;
        graphic[offset + 7u] = 3u;
    }

    check_int("table.accept", csb_v1_magic_rune_cost_table_from_decoded_graphic_pc34(
        graphic, sizeof(graphic), &table), 1);
    check_int("table.row0", csb_v1_magic_rune_cost_compute_pc34(
        &table, 0, 0, -1, &cost), 1);
    check_int("table.row0.cost", cost, 1);
    check_int("table.multiplied", csb_v1_magic_rune_cost_compute_pc34(
        &table, 1, 2, 97, &cost), 1);
    check_int("table.multiplied.cost", cost, 10);
    check_int("table.reject.power", csb_v1_magic_rune_cost_compute_pc34(
        &table, 1, 2, 0, &cost), 0);
    graphic[CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34 + 4] = 0;
    check_int("table.reject.zero_multiplier",
              csb_v1_magic_rune_cost_table_from_decoded_graphic_pc34(
                  graphic, sizeof(graphic), &table), 0);
    check_int("table.reject.size", csb_v1_magic_rune_cost_table_from_decoded_graphic_pc34(
        graphic, sizeof(graphic) - 1u, &table), 0);

    check_int("spells.accept", csb_v1_magic_spell_table_from_decoded_graphic_pc34(
        graphic, sizeof(graphic), &spell_table), 1);
    spell = csb_v1_magic_spell_lookup_pc34(&spell_table, fireball);
    check_int("spells.lookup", spell != NULL, 1);
    check_int("spells.skill", spell ? spell->skill_required : -1, 3);
    check_int("spells.kind", spell ? spell->skill_kind : -1, 4);
    check_int("spells.class", spell ? (spell->descriptor & 0x0f) : -1, 2);

    if (failures) return 1;
    puts("csb_v1_magic_rune_cost_pc34_compat: PASS");
    return 0;
}
