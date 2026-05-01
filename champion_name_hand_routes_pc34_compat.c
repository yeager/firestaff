#include "champion_name_hand_routes_pc34_compat.h"
#include <string.h>

#define DM1_PRIMARY_STATUS_COMMAND_BASE_PC34_COMPAT 12u
#define DM1_SECONDARY_NAME_COMMAND_BASE_PC34_COMPAT 16u
#define DM1_SECONDARY_HAND_COMMAND_BASE_PC34_COMPAT 20u
#define DM1_STATUS_ZONE_BASE_PC34_COMPAT 151u
#define DM1_ORDINAL_NONE_PC34_COMPAT 0u

static const ChampionNameHandRoutePc34Compat kChampionNameHandRoutes[] = {
    { 1u, 16u, 159u, 0u, 0u,   0,  0, 43,  7, "champion0.name", "F0368_COMMAND_SetLeader(0)", "COMMAND.C:484-489 G0455 maps C016 to C159; CLIKCHAM.C:27-30 dispatches C016..C019 to F0368 set-leader; layout-696 C159 gives x=0 y=0 w=43 h=7." },
    { 2u, 17u, 160u, 1u, 0u,  69,  0, 43,  7, "champion1.name", "F0368_COMMAND_SetLeader(1)", "COMMAND.C:484-489 G0455 maps C017 to C160; CLIKCHAM.C:27-30 dispatches C016..C019 to F0368 set-leader; layout-696 C160 gives x=69 y=0 w=43 h=7." },
    { 3u, 18u, 161u, 2u, 0u, 138,  0, 43,  7, "champion2.name", "F0368_COMMAND_SetLeader(2)", "COMMAND.C:484-489 G0455 maps C018 to C161; CLIKCHAM.C:27-30 dispatches C016..C019 to F0368 set-leader; layout-696 C161 gives x=138 y=0 w=43 h=7." },
    { 4u, 19u, 162u, 3u, 0u, 207,  0, 43,  7, "champion3.name", "F0368_COMMAND_SetLeader(3)", "COMMAND.C:484-489 G0455 maps C019 to C162; CLIKCHAM.C:27-30 dispatches C016..C019 to F0368 set-leader; layout-696 C162 gives x=207 y=0 w=43 h=7." },
    { 5u, 20u, 211u, 0u, 0u,   4, 10, 16, 16, "champion0.ready_hand",  "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=0)", "COMMAND.C:489 maps C020 ready-hand to C211; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C211 gives x=4 y=10 w=16 h=16." },
    { 6u, 21u, 212u, 0u, 1u,  24, 10, 16, 16, "champion0.action_hand", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=1)", "COMMAND.C:490 maps C021 action-hand to C212; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C212 gives x=24 y=10 w=16 h=16." },
    { 7u, 22u, 213u, 1u, 2u,  73, 10, 16, 16, "champion1.ready_hand",  "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=2)", "COMMAND.C:491 maps C022 ready-hand to C213; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C213 gives x=73 y=10 w=16 h=16." },
    { 8u, 23u, 214u, 1u, 3u,  93, 10, 16, 16, "champion1.action_hand", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=3)", "COMMAND.C:492 maps C023 action-hand to C214; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C214 gives x=93 y=10 w=16 h=16." },
    { 9u, 24u, 215u, 2u, 4u, 142, 10, 16, 16, "champion2.ready_hand",  "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=4)", "COMMAND.C:493 maps C024 ready-hand to C215; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C215 gives x=142 y=10 w=16 h=16." },
    {10u, 25u, 216u, 2u, 5u, 162, 10, 16, 16, "champion2.action_hand", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=5)", "COMMAND.C:494 maps C025 action-hand to C216; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C216 gives x=162 y=10 w=16 h=16." },
    {11u, 26u, 217u, 3u, 6u, 211, 10, 16, 16, "champion3.ready_hand",  "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=6)", "COMMAND.C:495 maps C026 ready-hand to C217; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C217 gives x=211 y=10 w=16 h=16." },
    {12u, 27u, 218u, 3u, 7u, 231, 10, 16, 16, "champion3.action_hand", "F0302_CHAMPION_ProcessCommands28To65_ClickOnSlotBox(slot=7)", "COMMAND.C:496 maps C027 action-hand to C218; CLIKCHAM.C:31-33 dispatches C020..C027 to F0302 with slot index command-C020; layout-696 C218 gives x=231 y=10 w=16 h=16." }
};

unsigned int champion_name_hand_routes_GetRouteCount(void) {
    return (unsigned int)(sizeof(kChampionNameHandRoutes) / sizeof(kChampionNameHandRoutes[0]));
}

int champion_name_hand_routes_GetRoute(unsigned int ordinal, ChampionNameHandRoutePc34Compat* outRoute) {
    if (!outRoute || ordinal < 1u || ordinal > champion_name_hand_routes_GetRouteCount()) return 0;
    *outRoute = kChampionNameHandRoutes[ordinal - 1u];
    return 1;
}

static int point_in_route(int screenX, int screenY, const ChampionNameHandRoutePc34Compat* route) {
    return screenX >= route->x && screenY >= route->y && screenX < route->x + route->w && screenY < route->y + route->h;
}

int champion_name_hand_routes_ResolveStatusBoxClick(unsigned int championIndex,
                                                    int screenX,
                                                    int screenY,
                                                    unsigned int inventoryChampionOrdinal,
                                                    ChampionStatusDispatchPc34Compat* outDispatch) {
    unsigned int i;
    if (!outDispatch || championIndex > 3u) return 0;
    memset(outDispatch, 0, sizeof(*outDispatch));
    outDispatch->primaryCommandId = DM1_PRIMARY_STATUS_COMMAND_BASE_PC34_COMPAT + championIndex;
    outDispatch->primaryZoneIndex = DM1_STATUS_ZONE_BASE_PC34_COMPAT + championIndex;
    outDispatch->championIndex = championIndex;

    if (inventoryChampionOrdinal == championIndex + 1u) {
        outDispatch->secondaryCommandId = outDispatch->primaryCommandId;
        outDispatch->secondaryZoneIndex = outDispatch->primaryZoneIndex;
        outDispatch->kind = CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT;
        return 1;
    }

    for (i = 0; i < champion_name_hand_routes_GetRouteCount(); ++i) {
        const ChampionNameHandRoutePc34Compat* route = &kChampionNameHandRoutes[i];
        if (route->championIndex == championIndex && point_in_route(screenX, screenY, route)) {
            outDispatch->secondaryCommandId = route->commandId;
            outDispatch->secondaryZoneIndex = route->zoneIndex;
            outDispatch->slotIndex = route->slotIndex;
            outDispatch->kind = (route->commandId >= DM1_SECONDARY_HAND_COMMAND_BASE_PC34_COMPAT)
                ? CHAMPION_STATUS_DISPATCH_SLOT_BOX_PC34_COMPAT
                : CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT;
            return 1;
        }
    }
    return 0;
}

const char* champion_name_hand_routes_GetEvidence(void) {
    return "COMMAND.C:484-497 defines G0455 champion name/ready/action hand subroutes; COMMAND.C:375-395 primary input scans champion status boxes, including C012..C015 left-click status-box commands; COMMAND.C:1394-1449 F0358 scans mouse routes in table order with inclusive source bounds; COMMAND.C:2158-2162 dispatches C012..C015 to F0367; CLIKCHAM.C:24-35 F0367 either sets the leader for the inventory champion or scans G0455 and dispatches name hits to F0368 and hand hits C020..C027 to F0302; CHAMPION.C:662-711 F0302 swaps leader-hand/slot objects.";
}

unsigned int champion_name_hand_routes_GetInvariant(void) {
    ChampionNameHandRoutePc34Compat route;
    ChampionStatusDispatchPc34Compat dispatch;
    return champion_name_hand_routes_GetRouteCount() == 12u &&
           champion_name_hand_routes_GetRoute(6u, &route) && route.commandId == 21u && route.zoneIndex == 212u && route.slotIndex == 1u &&
           champion_name_hand_routes_GetRoute(12u, &route) && route.commandId == 27u && route.zoneIndex == 218u && route.slotIndex == 7u &&
           champion_name_hand_routes_ResolveStatusBoxClick(0u, 25, 11, DM1_ORDINAL_NONE_PC34_COMPAT, &dispatch) &&
           dispatch.primaryCommandId == 12u && dispatch.primaryZoneIndex == 151u &&
           dispatch.secondaryCommandId == 21u && dispatch.secondaryZoneIndex == 212u && dispatch.slotIndex == 1u &&
           dispatch.kind == CHAMPION_STATUS_DISPATCH_SLOT_BOX_PC34_COMPAT &&
           champion_name_hand_routes_ResolveStatusBoxClick(3u, 232, 11, DM1_ORDINAL_NONE_PC34_COMPAT, &dispatch) &&
           dispatch.primaryCommandId == 15u && dispatch.secondaryCommandId == 27u && dispatch.slotIndex == 7u &&
           champion_name_hand_routes_ResolveStatusBoxClick(1u, 70, 1, DM1_ORDINAL_NONE_PC34_COMPAT, &dispatch) &&
           dispatch.primaryCommandId == 13u && dispatch.secondaryCommandId == 17u && dispatch.secondaryZoneIndex == 160u &&
           dispatch.kind == CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT &&
           champion_name_hand_routes_ResolveStatusBoxClick(2u, 170, 20, 3u, &dispatch) &&
           dispatch.primaryCommandId == 14u && dispatch.secondaryCommandId == 14u &&
           dispatch.kind == CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT &&
           !champion_name_hand_routes_ResolveStatusBoxClick(0u, 50, 20, DM1_ORDINAL_NONE_PC34_COMPAT, &dispatch);
}
