#include "firestaff/dm1/v1/projectile/ra_door_projectile_reject_pc34_compat.h"

#include <string.h>

enum {
    DM1_V1_RA_DOOR_ATTR_PROJECTILES_CAN_PASS_PC34 = 0x0002,
    DM1_V1_RA_DOOR_ATTR_CREATURES_CAN_SEE_PC34 = 0x0001,
    DM1_V1_RA_DOOR_ATTR_ANIMATED_PC34 = 0x0004
};

/*
 * Source-lock anchors:
 * - ReDMCSB DUNGEON.C:560-565 and 796-801 define G0254 door defenses;
 *   the RA door is type 3 with MASK0x0004|MASK0x0001 and defense 255.
 * - ReDMCSB PROJEXPL.C:F0217:471-525 handles projectile impact on doors:
 *   non-Open-Door projectiles fall through to F0232 with C0_FALSE magic.
 * - ReDMCSB PROJEXPL.C:F0232:1569-1592 rejects when Attack<Defense and
 *   only destroys a closed door after the defense predicate succeeds.
 */

static const DM1_V1_RaDoorProjectileDoorInfoPc34 s_door_info[] = {
    {
        DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_PORTCULLIS_PC34,
        110,
        DM1_V1_RA_DOOR_ATTR_PROJECTILES_CAN_PASS_PC34 |
            DM1_V1_RA_DOOR_ATTR_CREATURES_CAN_SEE_PC34,
        1,
        1,
        0,
        "portcullis",
        "DUNGEON.C:560-562 / 796-798 G0254 type 0 defense 110"
    },
    {
        DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_WOODEN_PC34,
        DM1_V1_RA_DOOR_PROJECTILE_WOODEN_DEFENSE_PC34,
        0,
        0,
        0,
        0,
        "wooden",
        "DUNGEON.C:563 / 799 G0254 type 1 defense 42"
    },
    {
        DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_IRON_PC34,
        230,
        0,
        0,
        0,
        0,
        "iron",
        "DUNGEON.C:564 / 800 G0254 type 2 defense 230"
    },
    {
        DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_RA_PC34,
        DM1_V1_RA_DOOR_PROJECTILE_RA_DEFENSE_PC34,
        DM1_V1_RA_DOOR_ATTR_ANIMATED_PC34 |
            DM1_V1_RA_DOOR_ATTR_CREATURES_CAN_SEE_PC34,
        0,
        1,
        1,
        "ra",
        "DUNGEON.C:565 / 801 G0254 type 3 RA defense 255"
    }
};

static const char s_source_evidence[] =
    "contract_only=1; no GRAPHICS.DAT/DUNGEON.DAT load. ReDMCSB "
    "DUNGEON.C:560-565 and 796-801 define G0254_as_Graphic559_DoorInfo; "
    "the comments at 561/797 state melee attacks are limited to 100, so "
    "that attack envelope can destroy wooden defense 42 but not RA defense "
    "255. Iron is 230, and RA door type 3 has "
    "MASK0x0004_ANIMATED|MASK0x0001_CREATURES_CAN_SEE_THROUGH with "
    "Defense=255. ReDMCSB PROJEXPL.C:F0217:471-525 handles projectile "
    "door impacts: Open Door explosions skip F0232, while ordinary "
    "projectile impacts compute impact attack and call "
    "F0232_GROUP_IsDoorDestroyedByAttack(..., C0_FALSE, 0). ReDMCSB "
    "PROJEXPL.C:F0232:1569-1592 rejects non-magic attacks when the door "
    "is not destructible or Attack<Defense, and only destroys a C4 closed "
    "door after the defense predicate succeeds. Non-overlap: this gate is "
    "the RA-door projectile-rejection contract, not pass745/pass563 "
    "wall-impact sound, not side-wall cardinal routing, and not the "
    "door-bash feedback gate's UI/melee feedback surface.";

static const DM1_V1_RaDoorProjectileRejectContractPc34 s_contract = {
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_RA_PC34,
    DM1_V1_RA_DOOR_PROJECTILE_RA_DEFENSE_PC34,
    DM1_V1_RA_DOOR_PROJECTILE_MAX_NON_MAGIC_ATTACK_PC34,
    1,
    1,
    4,
    "DUNGEON.C:560-565 and 796-801 G0254 door defenses",
    "PROJEXPL.C:F0217:471-525 door projectile impact branch",
    "PROJEXPL.C:F0232:1569-1592 Attack>=Defense closed-door gate",
    "Distinct from F0217/F0219 wall-impact and door-bash feedback gates"
};

static const DM1_V1_RaDoorProjectileDoorInfoPc34 *
door_info_for_type(int door_type)
{
    size_t i;

    for (i = 0; i < dm1_v1_ra_door_projectile_reject_door_info_count_pc34();
         ++i) {
        if (s_door_info[i].door_type == door_type) return &s_door_info[i];
    }
    return 0;
}

static int bounded_non_magic_attack(int projectile_attack)
{
    if (projectile_attack < 0) return 0;
    return projectile_attack;
}

const DM1_V1_RaDoorProjectileRejectContractPc34 *
dm1_v1_ra_door_projectile_reject_contract_pc34(void)
{
    return &s_contract;
}

const DM1_V1_RaDoorProjectileDoorInfoPc34 *
dm1_v1_ra_door_projectile_reject_door_info_pc34(size_t index)
{
    if (index >= dm1_v1_ra_door_projectile_reject_door_info_count_pc34()) {
        return 0;
    }
    return &s_door_info[index];
}

size_t dm1_v1_ra_door_projectile_reject_door_info_count_pc34(void)
{
    return sizeof(s_door_info) / sizeof(s_door_info[0]);
}

int dm1_v1_ra_door_projectile_reject_simulate_pc34(
    int door_type,
    int projectile_attack,
    int is_open_door_spell,
    int door_state_closed,
    DM1_V1_RaDoorProjectileRejectResultPc34 *out_result)
{
    const DM1_V1_RaDoorProjectileDoorInfoPc34 *door_info =
        door_info_for_type(door_type);
    DM1_V1_RaDoorProjectileRejectResultPc34 result;

    if (!door_info || !out_result) return 0;

    memset(&result, 0, sizeof(result));
    result.door_type = door_type;
    result.door_defense = door_info->defense;
    result.input_attack = projectile_attack;
    result.bounded_attack = bounded_non_magic_attack(projectile_attack);
    result.door_state_closed = door_state_closed ? 1 : 0;
    result.is_open_door_spell = is_open_door_spell ? 1 : 0;
    result.magic_attack_flag = 0;
    result.projectile_consumed = 1;
    result.source_anchor =
        "DUNGEON.C:560-565/796-801; PROJEXPL.C:F0217:471-525; F0232:1569-1592";

    if (is_open_door_spell) {
        result.f0217_reaches_f0232 = 0;
        result.f0232_destroyed = 0;
        result.source_locked_reject =
            door_type == DM1_V1_RA_DOOR_PROJECTILE_DOOR_TYPE_RA_PC34 ? 1 : 0;
        *out_result = result;
        return 1;
    }

    result.f0217_reaches_f0232 = 1;
    result.f0232_destroyed =
        door_state_closed && result.bounded_attack >= door_info->defense;
    result.door_state_after_destroyed = result.f0232_destroyed;
    result.source_locked_reject =
        result.f0217_reaches_f0232 && !result.f0232_destroyed;

    *out_result = result;
    return 1;
}

const char *
dm1_v1_ra_door_projectile_reject_source_evidence_pc34(void)
{
    return s_source_evidence;
}
