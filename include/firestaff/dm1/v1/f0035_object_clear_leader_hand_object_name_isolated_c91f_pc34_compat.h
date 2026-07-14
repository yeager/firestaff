#ifndef FIRESTAFF_DM1_V1_F0035_OBJECT_CLEAR_LEADER_HAND_OBJECT_NAME_ISOLATED_C91F_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0035_OBJECT_CLEAR_LEADER_HAND_OBJECT_NAME_ISOLATED_C91F_PC34_COMPAT_H

/*
 * ReDMCSB SOURCE/ENGINE/OBJECT.C F0035 (274-286) invokes
 * F0733_FillZoneByIndex(C017_ZONE_LEADER_HAND_OBJECT_NAME, C00_COLOR_BLACK).
 * ATARIST.H:29 binds this callable to F035_aaaw_.
 */
enum {
    DM1_V1_F0035_LEADER_HAND_NAME_ZONE_ISOLATED_C91F_PC34 = 17,
    DM1_V1_F0035_BLACK_ISOLATED_C91F_PC34 = 0
};

typedef void (*DM1_V1_F0035ZoneFillIsolatedC91fPc34)(
    void *context,
    int zone_index,
    int color);

typedef struct DM1_V1_F0035ZoneBackendIsolatedC91fPc34 {
    void *context;
    DM1_V1_F0035ZoneFillIsolatedC91fPc34 fill_zone_by_index;
} DM1_V1_F0035ZoneBackendIsolatedC91fPc34;

void DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameIsolatedC91fPc34Compat(
    const DM1_V1_F0035ZoneBackendIsolatedC91fPc34 *backend);

#endif
