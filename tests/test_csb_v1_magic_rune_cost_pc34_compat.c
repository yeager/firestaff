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

    if (failures) return 1;
    puts("csb_v1_magic_rune_cost_pc34_compat: PASS");
    return 0;
}
