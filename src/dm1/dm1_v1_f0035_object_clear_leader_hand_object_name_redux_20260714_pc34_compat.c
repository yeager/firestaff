#include "firestaff/dm1/v1/f0035_object_clear_leader_hand_object_name_redux_20260714_pc34_compat.h"

void
DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameRedux20260714Pc34Compat(
    const DM1_V1_F0035ObjectClearBackendRedux20260714Pc34 *backend)
{
    if (!backend || !backend->fill_zone_by_index) {
        return;
    }

    backend->fill_zone_by_index(
        backend->context,
        DM1_V1_F0035_LEADER_HAND_OBJECT_NAME_ZONE_REDUX_20260714_PC34,
        DM1_V1_F0035_BLACK_REDUX_20260714_PC34);
}
