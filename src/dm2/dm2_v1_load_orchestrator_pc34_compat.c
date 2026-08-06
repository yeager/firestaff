/* DM2 load orchestrator — source-load admission boundary.
 *
 * SKProject: SKWINSPX/src/v5/sksvgame.cpp::DM2_GAME_LOAD (1415-1530).
 *
 * The former callback transcript skipped the source-owned raw dungeon
 * blocks between the save prefix and DB pools.  It could consequently locate
 * a SUPPRESS stream at a fabricated offset and report a completed load.  The
 * real raw-SKSave receipt now owns the byte boundaries, but its complete
 * DM2_READ_SKSAVE_DUNGEON and live allocation consumers are not connected to
 * this legacy callback ABI yet.  Retain the public seam for that future
 * owner, but reject before invoking any callback so no partial state leaks.
 */

#include "dm2_v1_load_orchestrator_pc34_compat.h"
#include <string.h>

int dm2_v1_load_orchestrate(
    const DM2_LoadOrchestratorCallbacks *cb,
    const uint8_t *in_buf, size_t in_size,
    DM2_LoadOrchestratorResult *result)
{
    if (result) {
        memset(result, 0, sizeof(*result));
    }
    if (!cb || !in_buf || !result) {
        if (result) {
            result->error = DM2_LOAD_ORCHESTRATOR_ERROR_ARGUMENT;
        }
        return -1;
    }

    (void)in_size;
    result->error = DM2_LOAD_ORCHESTRATOR_ERROR_RAW_LAYOUT_UNBOUND;
    return -1;
}
