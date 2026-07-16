#ifndef FIRESTAFF_DM1_V1_GROUP_DESTINATION_VISIBILITY_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_GROUP_DESTINATION_VISIBILITY_PC34_COMPAT_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
    DM1_V1_GROUP_DIRECTION_NORTH_PC34 = 0,
    DM1_V1_GROUP_DIRECTION_EAST_PC34 = 1,
    DM1_V1_GROUP_DIRECTION_SOUTH_PC34 = 2,
    DM1_V1_GROUP_DIRECTION_WEST_PC34 = 3
};

/*
 * ReDMCSB PROJEXPL.C/GROUP.C F0227_GROUP_IsDestinationVisibleFromSource.
 *
 * This boundary owns the source direction cone only: destination squares must
 * be strictly in front of the source along the normalized cardinal direction.
 * Blocked-line walking remains owned by the F0199/F0200 visibility callers.
 */
int F0227_GROUP_IsDestinationVisibleFromSource(
    unsigned int direction,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY);

int F0227_GROUP_IsDestinationVisibleFromSource_Compat(
    unsigned int direction,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY);

const char *F0227_GROUP_IsDestinationVisibleFromSource_SourceEvidencePc34(void);

#ifdef __cplusplus
}
#endif

#endif
