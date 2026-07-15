#ifndef FIRESTAFF_DM1_V1_ORNAMENT_CACHE_OWNER_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_ORNAMENT_CACHE_OWNER_PC34_COMPAT_H

#include <stddef.h>

/* ReDMCSB DUNGEON.C F0173 materializes map-local ornament ordinals into
 * G0261/G0262 tables. Rendering must not infer a global graphic when the
 * verified DUNGEON.DAT metadata table is unavailable. */
int dm1_v1_ornament_cache_global_index_pc34(
    int cache_loaded,
    const int *global_indices,
    size_t global_index_count,
    int ornament_ordinal,
    int *out_global_index);

#endif
