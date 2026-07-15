#include "dm1_v1_dungeon_weapon_info_pc34_compat.h"

#include "dm1_v1_dungeon_thing_data_pc34_compat.h"

#include <string.h>

int dm1_v1_dungeon_get_weapon_info_pc34(
    const struct DungeonThings_Compat *things,
    unsigned short thing,
    DM1_WeaponInfo *outInfo)
{
    const unsigned char *rawWeapon;
    int weaponType;

    if (outInfo) {
        memset(outInfo, 0, sizeof(*outInfo));
        outInfo->weaponClass = -1;
    }
    if (!outInfo || THING_GET_TYPE(thing) != THING_TYPE_WEAPON) return 0;

    rawWeapon = dm1_v1_dungeon_get_thing_data_pc34(things, thing);
    if (!rawWeapon) return 0;

    /* PC3.4 WEAPON.Type is bits 0..6 of bytes 2..3. */
    weaponType = rawWeapon[2] & 0x7f;
    return dm1_weapon_info_pc34(weaponType, outInfo) > 0;
}

const char *dm1_v1_dungeon_weapon_info_source_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C:F0158_DUNGEON_GetWeaponInfo:1651-1661; "
           "F0156(thing)->WEAPON.Type indexes G0238_as_Graphic559_WeaponInfo";
}
