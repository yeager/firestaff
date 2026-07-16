#ifndef FIRESTAFF_DM1_V1_F0035_OBJECT_CLEAR_LEADER_HAND_OBJECT_NAME_REDUX_20260714_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_F0035_OBJECT_CLEAR_LEADER_HAND_OBJECT_NAME_REDUX_20260714_PC34_COMPAT_H

/*
 * ReDMCSB SOURCE/ENGINE/OBJECT.C F0035 (lines 274-286) delegates to
 * F0733_FillZoneByIndex(C017_ZONE_LEADER_HAND_OBJECT_NAME, C00_COLOR_BLACK).
 * ATARIST.H line 29 exposes the original F0035 callable name.
 */
enum {
    DM1_V1_F0035_LEADER_HAND_OBJECT_NAME_ZONE_REDUX_20260714_PC34 = 17,
    DM1_V1_F0035_BLACK_REDUX_20260714_PC34 = 0
};

typedef void (*DM1_V1_F0035FillZoneByIndexRedux20260714Pc34)(
    void *context,
    int zone_index,
    int color);

typedef struct DM1_V1_F0035ObjectClearBackendRedux20260714Pc34 {
    void *context;
    DM1_V1_F0035FillZoneByIndexRedux20260714Pc34 fill_zone_by_index;
} DM1_V1_F0035ObjectClearBackendRedux20260714Pc34;

void DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameRedux20260714Pc34Compat(
    const DM1_V1_F0035ObjectClearBackendRedux20260714Pc34 *backend);

#endif
