#include "dm1_v1_armour_defense_f0143_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    DM1_V1_F0143_ArmourInfoPc34 cloth;
    DM1_V1_F0143_ArmourInfoPc34 mail;
    DM1_V1_F0143_ArmourInfoPc34 plate;

    cloth.defense = 3;
    cloth.sharpDefense = 0;
    cloth.attributes = 0;
    mail.defense = 16;
    mail.sharpDefense = 5;
    mail.attributes = 0x0012u;
    plate.defense = 65;
    plate.sharpDefense = 7;
    plate.attributes = 0x00a0u;

    assert(strstr(DM1_V1_F0143_SourceEvidencePc34(),
                  "F0143_DUNGEON_GetArmourDefense") != 0);

    assert(F0143_DUNGEON_GetArmourDefense(&cloth, 0) == 3);
    assert(F0143_DUNGEON_GetArmourDefense(&cloth, 1) == 3);
    assert(F0143_DUNGEON_GetArmourDefense(&mail, 0) == 16);
    assert(F0143_DUNGEON_GetArmourDefense(&mail, 1) == 21);
    assert(F0143_DUNGEON_GetArmourDefense(&plate, 0) == 65);
    assert(F0143_DUNGEON_GetArmourDefense(&plate, 2) == 72);

    assert(DM1_V1_Dungeon_GetArmourDefenseF0143Pc34Compat(&plate, 1) == 72);
    assert(F0143_DUNGEON_GetArmourDefense(0, 1) == 0);
    assert(DM1_V1_Dungeon_GetArmourDefenseF0143Pc34Compat(0, 0) == 0);

    return 0;
}
