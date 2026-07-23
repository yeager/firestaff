#include "csb_v1_f0247_launcher_materialization_pc34_compat.h"

#include <string.h>

int csb_v1_f0247_launcher_create_failure_materialization_pc34_compat(
    uint16_t associated_thing,
    int map_x,
    int map_y,
    int cell,
    CSB_V1_F0247LauncherMaterializationReceipt_PC34 *out_receipt)
{
    int thing_type;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (associated_thing == 0xFFFEu || associated_thing == 0xFFFFu ||
        map_x < 0 || map_y < 0 || cell < 0 || cell > 3) {
        return 0;
    }
    thing_type = (associated_thing >> 10) & 0x0f;
    /* F0212's CSB21 overflow recovery explicitly excludes C15 explosions. */
    if (thing_type <= 4 || thing_type >= 14) return 0;

    out_receipt->valid = 1;
    out_receipt->thing = (uint16_t)((associated_thing & 0x3fffu) |
                                    ((uint16_t)cell << 14));
    out_receipt->map_x = map_x;
    out_receipt->map_y = map_y;
    out_receipt->cell = cell;
    return 1;
}
