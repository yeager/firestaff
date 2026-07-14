#ifndef FIRESTAFF_CSB_V1_F0227_GROUP_IS_DESTINATION_VISIBLE_FROM_SOURCE_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_F0227_GROUP_IS_DESTINATION_VISIBLE_FROM_SOURCE_PC34_COMPAT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB direction ordinals used by GROUP2.C F0227. */
enum {
    CSB_V1_F0227_DIRECTION_NORTH_PC34 = 0,
    CSB_V1_F0227_DIRECTION_EAST_PC34 = 1,
    CSB_V1_F0227_DIRECTION_SOUTH_PC34 = 2,
    CSB_V1_F0227_DIRECTION_WEST_PC34 = 3
};

/* ReDMCSB GROUP2.C F0227_GROUP_IsDestinationVisibleFromSource.
 *
 * Tests whether the destination lies in the source-facing 90-degree view
 * wedge. The original switch has no default arm, so any direction other than
 * north, east, or south follows the untransformed west-facing comparison.
 */
bool csb_v1_f0227_group_is_destination_visible_from_source_pc34(
    int direction,
    int source_map_x,
    int source_map_y,
    int destination_map_x,
    int destination_map_y);

#ifdef __cplusplus
}
#endif

#endif
