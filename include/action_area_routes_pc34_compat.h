#ifndef ACTION_AREA_ROUTES_PC34_COMPAT_H
#define ACTION_AREA_ROUTES_PC34_COMPAT_H

typedef struct ActionAreaRoutePc34Compat {
    unsigned int commandId;
    unsigned int zoneIndex;
    int actionListIndex;
    int championIndex;
    int x;
    int y;
    int w;
    int h;
    const char* name;
    const char* sourceEvidence;
} ActionAreaRoutePc34Compat;

const char* action_area_routes_GetEvidence(void);
unsigned int action_area_routes_GetInvariant(void);
unsigned int action_area_routes_GetTouchMatrixInvariant(void);
unsigned int action_area_routes_GetRouteCount(void);
int action_area_routes_GetRoute(unsigned int ordinal, ActionAreaRoutePc34Compat* outRoute);
int action_area_routes_ResolveNameMenuClick(int screenX,
                                            int screenY,
                                            unsigned int actionCount,
                                            ActionAreaRoutePc34Compat* outRoute);
int action_area_routes_ResolveIconClick(int screenX,
                                        int screenY,
                                        unsigned int partyChampionCount,
                                        ActionAreaRoutePc34Compat* outRoute);
#endif
