#ifndef FIRESTAFF_DM1_V1_DUNGEON_WEAPON_INFO_PC34_COMPAT_H
#define FIRESTAFF_DM1_V1_DUNGEON_WEAPON_INFO_PC34_COMPAT_H

#include "dm1_v1_combat_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ReDMCSB DUNGEON.C F0158: raw WEAPON.Type to G0238 WeaponInfo. */
int dm1_v1_dungeon_get_weapon_info_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing,
    DM1_WeaponInfo *outInfo);

const char *dm1_v1_dungeon_weapon_info_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
