#include "firestaff/dm1/v1/f0035_object_clear_leader_hand_object_name_isolated_c91f_pc34_compat.h"

void
DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameIsolatedC91fPc34Compat(
    const DM1_V1_F0035ZoneBackendIsolatedC91fPc34 *backend)
{
    if (!backend || !backend->fill_zone_by_index) {
        return;
    }

    backend->fill_zone_by_index(
        backend->context,
        DM1_V1_F0035_LEADER_HAND_NAME_ZONE_ISOLATED_C91F_PC34,
        DM1_V1_F0035_BLACK_ISOLATED_C91F_PC34);
}
