#include "redmcsb_f8160_creature_palette_c25_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_byte(const char *name, uint8_t actual, uint8_t expected)
{
    if (actual != expected) {
        fprintf(stderr, "FAIL: %s (got %u, expected %u)\n", name,
                (unsigned)actual, (unsigned)expected);
        ++failures;
    }
}

static void expect_true(const char *name, int actual)
{
    if (!actual) {
        fprintf(stderr, "FAIL: %s\n", name);
        ++failures;
    }
}

int main(void)
{
    RedmcsbF8160ColorPc34Compat tables[6][4];
    RedmcsbF8160ColorPc34Compat *table_bases[6];
    RedmcsbF8160ColorPc34Compat creature_palettes[14][6];
    size_t table_index;
    size_t color_index;

    memset(tables, 0, sizeof(tables));
    memset(creature_palettes, 0, sizeof(creature_palettes));
    for (table_index = 0U; table_index < 6U; ++table_index) {
        table_bases[table_index] = tables[table_index];
        for (color_index = 0U; color_index < 4U; ++color_index) {
            tables[table_index][color_index].index = (uint8_t)(40U + color_index);
        }
        creature_palettes[3][table_index].index = (uint8_t)(1U + table_index);
        creature_palettes[3][table_index].red = (uint8_t)(10U + table_index);
        creature_palettes[3][table_index].green = (uint8_t)(20U + table_index);
        creature_palettes[3][table_index].blue = (uint8_t)(30U + table_index);
    }

    expect_true("six RGB replacements",
                redmcsb_f8160_set_creature_replacement_colors_c25_pc34_compat(
                    table_bases, 4U, (const RedmcsbF8160ColorPc34Compat (*)[REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34])creature_palettes, 2, 3));
    expect_byte("first target index retained", tables[0][2].index, 42U);
    expect_byte("first target red", tables[0][2].red, 10U);
    expect_byte("first target green", tables[0][2].green, 20U);
    expect_byte("last target blue", tables[5][2].blue, 35U);
    expect_byte("unselected target retained", tables[4][1].red, 0U);

    tables[0][2].red = 99U;
    expect_true("invalid source set rejected",
                !redmcsb_f8160_set_creature_replacement_colors_c25_pc34_compat(
                    table_bases, 4U, (const RedmcsbF8160ColorPc34Compat (*)[REDMCSB_F8160_CREATURE_REPLACEMENT_COUNT_PC34])creature_palettes, 2, 14));
    expect_byte("invalid source leaves palette", tables[0][2].red, 99U);

    if (strstr(redmcsb_f8160_creature_palette_source_evidence_pc34(),
               "VIDEODRV.C:3543-3563") == NULL) {
        fprintf(stderr, "FAIL: evidence\n");
        ++failures;
    }
    if (failures != 0) return 1;
    puts("PASSED: ReDMCSB F8160 PC 3.4 C25 creature palette replacement");
    return 0;
}
