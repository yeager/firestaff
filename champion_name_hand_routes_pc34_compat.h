#ifndef CHAMPION_NAME_HAND_ROUTES_PC34_COMPAT_H
#define CHAMPION_NAME_HAND_ROUTES_PC34_COMPAT_H

typedef enum ChampionStatusDispatchKindPc34Compat {
    CHAMPION_STATUS_DISPATCH_NONE_PC34_COMPAT = 0,
    CHAMPION_STATUS_DISPATCH_SET_LEADER_PC34_COMPAT = 1,
    CHAMPION_STATUS_DISPATCH_SLOT_BOX_PC34_COMPAT = 2
} ChampionStatusDispatchKindPc34Compat;

typedef struct ChampionNameHandRoutePc34Compat {
    unsigned int ordinal;
    unsigned int commandId;
    unsigned int zoneIndex;
    unsigned int championIndex;
    unsigned int slotIndex;
    int x;
    int y;
    int w;
    int h;
    const char* name;
    const char* dispatch;
    const char* evidence;
} ChampionNameHandRoutePc34Compat;

typedef struct ChampionStatusDispatchPc34Compat {
    unsigned int primaryCommandId;
    unsigned int primaryZoneIndex;
    unsigned int secondaryCommandId;
    unsigned int secondaryZoneIndex;
    unsigned int championIndex;
    unsigned int slotIndex;
    ChampionStatusDispatchKindPc34Compat kind;
} ChampionStatusDispatchPc34Compat;

unsigned int champion_name_hand_routes_GetRouteCount(void);
int champion_name_hand_routes_GetRoute(unsigned int ordinal, ChampionNameHandRoutePc34Compat* outRoute);
int champion_name_hand_routes_ResolveStatusBoxClick(unsigned int championIndex,
                                                    int screenX,
                                                    int screenY,
                                                    unsigned int inventoryChampionOrdinal,
                                                    ChampionStatusDispatchPc34Compat* outDispatch);
const char* champion_name_hand_routes_GetEvidence(void);
unsigned int champion_name_hand_routes_GetInvariant(void);
#endif
