#ifndef FIRESTAFF_DM1_V1_C14_LAYOUT_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_C14_LAYOUT_PC34_COMPAT_H

#include "memory_dungeon_dat_pc34_compat.h"

typedef struct {
    struct DungeonThings_Compat* things;
    struct DungeonDatState_Compat* dungeon;
    unsigned short thing;
    unsigned char raw[8];
    struct DungeonProjectile_Compat decoded;
    int mapIndex, mapX, mapY;
    int linked, active;
} DM1_C14PoolReservationPc34;

int dm1_v1_c14_pool_reserve_pc34(struct DungeonThings_Compat* things,
                                  DM1_C14PoolReservationPc34* out);
int dm1_v1_c14_pool_initialize_and_link_pc34(
    DM1_C14PoolReservationPc34* reservation,
    struct DungeonDatState_Compat* dungeon, unsigned short slot,
    int kinetic_energy, int attack, unsigned short event_index, int cell,
    int map_index, int map_x, int map_y);
int dm1_v1_c14_pool_rollback_pc34(DM1_C14PoolReservationPc34* reservation);

#endif
