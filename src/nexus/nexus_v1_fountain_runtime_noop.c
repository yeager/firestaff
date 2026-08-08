/* Capture-gated Nexus V1 fountain production adapter. */

#include "nexus_v1_fountain.h"
#include <string.h>

void nexus_v1_fountain_manager_init(Nexus_FountainManager *manager)
{
    if (manager) memset(manager, 0, sizeof(*manager));
}

int nexus_v1_fountain_register(Nexus_FountainManager *manager, int type,
                               int map_x, int map_y, int uses,
                               int restore_amount)
{
    (void)manager; (void)type; (void)map_x; (void)map_y;
    (void)uses; (void)restore_amount;
    return -1;
}

int nexus_v1_fountain_find_at(const Nexus_FountainManager *manager,
                              int map_x, int map_y)
{
    (void)manager; (void)map_x; (void)map_y;
    return -1;
}

int nexus_v1_fountain_drink(Nexus_FountainManager *manager,
                            int fountain_idx, Nexus_V1_Champion *champion)
{
    (void)manager; (void)fountain_idx; (void)champion;
    return 0;
}

int nexus_v1_fountain_uses_left(const Nexus_FountainManager *manager,
                                int fountain_idx)
{
    (void)manager; (void)fountain_idx;
    return 0;
}
