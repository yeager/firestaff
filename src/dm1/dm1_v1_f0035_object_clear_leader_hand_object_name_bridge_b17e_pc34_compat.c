#include "firestaff/dm1/v1/f0035_object_clear_leader_hand_object_name_bridge_b17e_pc34_compat.h"

void
DM1_V1_F0035_OBJECT_ClearLeaderHandObjectNameBridgeB17ePc34Compat(
    const DM1_V1_F0035ZoneBackendBridgeB17ePc34 *backend)
{
    if (backend && backend->fill_zone_by_index) {
        backend->fill_zone_by_index(
            backend->context,
            DM1_V1_F0035_LEADER_HAND_NAME_ZONE_BRIDGE_B17E_PC34,
            DM1_V1_F0035_BLACK_BRIDGE_B17E_PC34);
    }
}
