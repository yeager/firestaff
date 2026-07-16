#include "dm1_v1_group_destination_visibility_pc34_compat.h"

static const char s_source_evidence[] =
    "ReDMCSB PROJEXPL.C/GROUP.C F0227_GROUP_IsDestinationVisibleFromSource "
    "accepts a direction plus source/destination map coordinates before the "
    "caller's F0199/F0200 blocked-line checks. Firestaff keeps this boundary "
    "to the source-facing cardinal cone: north requires same X and lower Y, "
    "east same Y and higher X, south same X and higher Y, west same Y and "
    "lower X. Same-square, side, and behind destinations are not visible "
    "from the source direction; direction values are normalized through the "
    "source M021-style cardinal mask.";

int F0227_GROUP_IsDestinationVisibleFromSource(
    unsigned int direction,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY)
{
    switch (direction & 0x03u) {
    case DM1_V1_GROUP_DIRECTION_NORTH_PC34:
        return destinationMapX == sourceMapX && destinationMapY < sourceMapY;
    case DM1_V1_GROUP_DIRECTION_EAST_PC34:
        return destinationMapY == sourceMapY && destinationMapX > sourceMapX;
    case DM1_V1_GROUP_DIRECTION_SOUTH_PC34:
        return destinationMapX == sourceMapX && destinationMapY > sourceMapY;
    case DM1_V1_GROUP_DIRECTION_WEST_PC34:
        return destinationMapY == sourceMapY && destinationMapX < sourceMapX;
    default:
        return 0;
    }
}

int F0227_GROUP_IsDestinationVisibleFromSource_Compat(
    unsigned int direction,
    int sourceMapX,
    int sourceMapY,
    int destinationMapX,
    int destinationMapY)
{
    return F0227_GROUP_IsDestinationVisibleFromSource(
        direction, sourceMapX, sourceMapY, destinationMapX, destinationMapY);
}

const char *F0227_GROUP_IsDestinationVisibleFromSource_SourceEvidencePc34(void)
{
    return s_source_evidence;
}
