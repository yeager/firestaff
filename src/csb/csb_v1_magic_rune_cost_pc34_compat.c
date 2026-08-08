#include "csb_v1_magic_rune_cost_pc34_compat.h"

#include <string.h>

static uint32_t fnv1a32(const uint8_t *bytes, size_t size)
{
    uint32_t value = 2166136261u;
    size_t i;
    for (i = 0u; i < size; ++i) {
        value ^= bytes[i];
        value *= 16777619u;
    }
    return value;
}

static uint32_t read_u32be(const uint8_t *bytes)
{
    return ((uint32_t)bytes[0] << 24) | ((uint32_t)bytes[1] << 16) |
           ((uint32_t)bytes[2] << 8) | (uint32_t)bytes[3];
}

static uint16_t read_u16be(const uint8_t *bytes)
{
    return (uint16_t)(((uint16_t)bytes[0] << 8) | bytes[1]);
}

int csb_v1_magic_rune_cost_table_from_decoded_graphic_pc34(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_MagicRuneCostTablePc34 *out_table)
{
    CSB_V1_MagicRuneCostTablePc34 table;
    size_t i;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!decoded_graphic || !out_table ||
        decoded_size != CSB_V1_MAGIC_RUNE_TABLE_DECODED_SIZE_PC34) return 0;

    memset(&table, 0, sizeof(table));
    memcpy(table.power_multiplier,
           decoded_graphic + CSB_V1_MAGIC_RUNE_POWER_MULTIPLIER_OFFSET_PC34,
           sizeof(table.power_multiplier));
    memcpy(table.base_cost,
           decoded_graphic + CSB_V1_MAGIC_RUNE_BASE_COST_OFFSET_PC34,
           sizeof(table.base_cost));
    for (i = 0u; i < sizeof(table.power_multiplier); ++i) {
        if (table.power_multiplier[i] == 0u) return 0;
    }
    table.decoded_payload_fnv1a = fnv1a32(decoded_graphic, decoded_size);
    if (table.decoded_payload_fnv1a == 0u) return 0;
    table.valid = 1;
    *out_table = table;
    return 1;
}

int csb_v1_magic_spell_table_from_decoded_graphic_pc34(
    const uint8_t *decoded_graphic, size_t decoded_size,
    CSB_V1_MagicSpellTablePc34 *out_table)
{
    CSB_V1_MagicSpellTablePc34 table;
    size_t i;

    if (out_table) memset(out_table, 0, sizeof(*out_table));
    if (!decoded_graphic || !out_table ||
        decoded_size != CSB_V1_MAGIC_RUNE_TABLE_DECODED_SIZE_PC34 ||
        CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 +
                CSB_V1_MAGIC_SPELL_COUNT_PC34 *
                    CSB_V1_MAGIC_SPELL_RECORD_SIZE_PC34 > decoded_size) {
        return 0;
    }
    memset(&table, 0, sizeof(table));
    for (i = 0u; i < CSB_V1_MAGIC_SPELL_COUNT_PC34; ++i) {
        const uint8_t *record = decoded_graphic +
            CSB_V1_MAGIC_SPELL_TABLE_OFFSET_PC34 +
            i * CSB_V1_MAGIC_SPELL_RECORD_SIZE_PC34;
        table.spells[i].spell_id = read_u32be(record);
        table.spells[i].skill_required = record[4];
        table.spells[i].skill_kind = record[5];
        table.spells[i].descriptor = read_u16be(record + 6);
        /* Every CSBWin source entry has an incantation and a class. */
        if (table.spells[i].spell_id == 0u ||
            (table.spells[i].descriptor & 0x0fu) == 0u) return 0;
    }
    table.decoded_payload_fnv1a = fnv1a32(decoded_graphic, decoded_size);
    if (table.decoded_payload_fnv1a == 0u) return 0;
    table.valid = 1;
    *out_table = table;
    return 1;
}

const CSB_V1_MagicSpellPc34 *csb_v1_magic_spell_lookup_pc34(
    const CSB_V1_MagicSpellTablePc34 *table, const uint8_t runes[4])
{
    uint32_t packed = 0u;
    size_t i;
    size_t rune;

    if (!table || !table->valid || !runes || runes[1] == 0u) return NULL;
    for (rune = 0u; rune < 4u && runes[rune] != 0u; ++rune) {
        /* ReDMCSB MENU.C F0409 lines 1683-1703 starts at bit 24 and
         * shifts down once per entered symbol.  Its table records use a
         * zero high byte for normal spells, so the power symbol occupies
         * the byte that F0409 subsequently masks out.  For example,
         * `FUL IR` is stored as 0x00696f00, while an OH-power cast is
         * assembled as 0x68696f00 before the low-24-bit comparison. */
        packed |= (uint32_t)runes[rune] << (24u - (uint32_t)rune * 8u);
    }
    for (i = 0u; i < CSB_V1_MAGIC_SPELL_COUNT_PC34; ++i) {
        const CSB_V1_MagicSpellPc34 *spell = &table->spells[i];
        if ((spell->spell_id & 0xff000000u) != 0u
                ? spell->spell_id == packed
                : spell->spell_id == (packed & 0x00ffffffu)) {
            return spell;
        }
    }
    return NULL;
}

int csb_v1_magic_rune_cost_compute_pc34(
    const CSB_V1_MagicRuneCostTablePc34 *table,
    int symbol_step, int symbol_index, int power_rune, int *out_cost)
{
    int cost;
    int power_index;

    if (out_cost) *out_cost = 0;
    if (!table || !table->valid || !out_cost || symbol_step < 0 ||
        symbol_step >= CSB_V1_MAGIC_RUNE_ROW_COUNT_PC34 || symbol_index < 0 ||
        symbol_index >= CSB_V1_MAGIC_RUNE_SYMBOLS_PER_ROW_PC34) return 0;
    cost = table->base_cost[symbol_step][symbol_index];
    if (symbol_step != 0) {
        power_index = power_rune - 96;
        if (power_index < 0 ||
            power_index >= CSB_V1_MAGIC_RUNE_SYMBOLS_PER_ROW_PC34) return 0;
        cost = (cost * table->power_multiplier[power_index]) >> 3;
    }
    *out_cost = cost;
    return 1;
}
