#include "dm1_v1_ornament_cache_owner_pc34_compat.h"

int dm1_v1_ornament_cache_global_index_pc34(
    int cache_loaded,
    const int *global_indices,
    size_t global_index_count,
    int ornament_ordinal,
    int *out_global_index)
{
    size_t local_index;
    int global_index;

    if (!out_global_index || !cache_loaded || !global_indices ||
        ornament_ordinal <= 0) {
        return 0;
    }
    local_index = (size_t)(ornament_ordinal - 1);
    if (local_index >= global_index_count) {
        return 0;
    }
    global_index = global_indices[local_index];
    if (global_index < 0) {
        return 0;
    }
    *out_global_index = global_index;
    return 1;
}
