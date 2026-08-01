/*
 * dm2_v1_engage_command_pc34_compat.c — DM2 hand action dispatcher.
 *
 * Source: skproject/SKWINSPX/src/v5/skengage.cpp:23-869
 *
 * The 53-case switch maps action_id (from CMDSTR entry 2) to:
 *
 *   1       ATTACK:  queue timer type 0x47 with weapon delay
 *   2       CAST:    DM2_CAST_CHAMPION_MISSILE_SPELL
 *   3,7     WIELD:   DM2_WIELD_WEAPON -> attack creature or door
 *   4       CONFUSE: DM2_CONFUSE_CREATURE at target tile
 *   5,37,38 LIGHT:   DM2_PROCEED_LIGHT
 *   6       CLOUD:   DM2_CREATE_CLOUD at party position
 *   8       HEAL:    savegames1.b_04 += delay (party heal power)
 *   9       STEP:    DM2_MOVE_RECORD_TO into creature tile
 *   10      MANA:    savegames1.b_03 += delay (mana charge)
 *   11-14   ABILITY: DM2_hero_2c1d_0186 (type 3/4/5/6)
 *   15      CONSUME: DM2_PLAYER_CONSUME_OBJECT
 *   16      EQUIP:   FIND_POUCH -> REMOVE_POSSESSION -> EQUIP_ITEM
 *   31      SHOOT:   DM2_SHOOT_CHAMPION_MISSILE from launcher
 *   32-34   ACT:     DM2_hero_2c1d_0186 (type 0/1/2)
 *   35      HEAL_HP: convert MP to HP (loop)
 *   41      TURN:    DM2_hero_2c1d_1de2 (position swap)
 *   43      MINION:  DM2_SET_DESTINATION_OF_MINION_MAP
 *   44,45   MINION:  set minion missile ref action
 *   46      MINION:  DM2_CREATE_MINION (type 0x30)
 *   47      RELEASE: DM2_RELEASE_MINION
 *   48-50   MINION:  DM2_CREATE_MINION (types 0x31/0x34/0x35)
 *   53      IFACE:   DM2_LOAD_GDAT_INTERFACE_00_0A
 *
 * Epilogue (L_fin):
 *   - Queue noise gen for sound effect
 *   - Adjust hand cooldown by power
 *   - Drain stamina by vw_40
 *   - Award skill experience by vo_68
 *   - Record cooldown/item state in hand action table
 */

#include "dm2_v1_engage_command_pc34_compat.h"

#include <string.h>

int dm2_v1_engage_command(
    const DM2_V1_EngageCommandRequest *request,
    DM2_V1_EngageCommandReceipt *receipt)
{
    int action_case;

    if (!receipt) return 0;
    memset(receipt, 0, sizeof(*receipt));

    if (!request) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->valid = 1;

    if (!request->hero_alive || request->hero_hp <= 0) {
        receipt->hero_dead = 1;
        return 0;
    }

    action_case = request->cmd.action_id - 1;
    if (action_case < 0 || action_case > 53) {
        receipt->fail_closed = 1;
        return 0;
    }

    receipt->action_type = (DM2_V1_EngageCommandType)action_case;
    receipt->action_dispatched = 1;
    receipt->success = 1;

    switch (action_case) {
    /* No-op cases (0, 17-30, 36, 39-40, 42, 51-52) */
    case 0:
    case 17: case 18: case 19: case 20: case 21:
    case 22: case 23: case 24: case 25: case 26:
    case 27: case 28: case 29: case 30:
    case 36: case 39: case 40: case 42:
    case 51: case 52:
        receipt->success = 0;
        break;

    case 1: /* ATTACK: queue timer 0x47 */
        receipt->attack_queued = 1;
        receipt->cooldown_applied = request->cmd.delay;
        if (receipt->cooldown_applied < 32)
            receipt->cooldown_applied = 32;
        receipt->fail_closed = 1;
        break;

    case 2: /* CAST_CHAMPION_MISSILE_SPELL */
        receipt->spell_cast = 1;
        receipt->fail_closed = 1;
        break;

    case 3: /* WIELD_WEAPON */
    case 7: /* WIELD_WEAPON (variant) */
        receipt->weapon_wielded = 1;
        receipt->fail_closed = 1;
        break;

    case 4: /* CONFUSE_CREATURE */
        receipt->creature_confused = 1;
        receipt->fail_closed = 1;
        break;

    case 5:  /* PROCEED_LIGHT */
    case 37: /* PROCEED_LIGHT (variant) */
    case 38: /* PROCEED_LIGHT (variant) */
        receipt->light_toggled = 1;
        receipt->fail_closed = 1;
        break;

    case 6: /* CREATE_CLOUD */
        receipt->cloud_created = 1;
        receipt->fail_closed = 1;
        break;

    case 8: /* HEAL_PARTY: savegames1.b_04 += delay, capped at 255 */
        receipt->healed_party = 1;
        receipt->cooldown_applied = request->cmd.delay;
        if (receipt->cooldown_applied < 32)
            receipt->cooldown_applied = 32;
        break;

    case 9: /* STEP_FORWARD into creature tile */
        receipt->stepped_forward = 1;
        receipt->fail_closed = 1;
        break;

    case 10: /* MANA_GAIN: savegames1.b_03 += delay, capped at 200 */
        receipt->mana_gained = 1;
        break;

    case 11: /* ABILITY type 5 */
    case 12: /* ABILITY type 4 */
    case 13: /* ABILITY type 6 */
    case 14: /* ABILITY type 3 */
        receipt->fail_closed = 1;
        break;

    case 15: /* CONSUME object */
        receipt->consumed = 1;
        receipt->fail_closed = 1;
        break;

    case 16: /* EQUIP item to hand */
        receipt->equipped = 1;
        receipt->fail_closed = 1;
        break;

    case 31: /* SHOOT_CHAMPION_MISSILE */
        receipt->missile_shot = 1;
        receipt->fail_closed = 1;
        break;

    case 32: /* HERO_ACT type 1 */
    case 33: /* HERO_ACT type 0 */
    case 34: /* HERO_ACT type 2 */
        receipt->fail_closed = 1;
        break;

    case 35: /* HEAL_SELF: convert MP to HP */
    {
        int16_t hp_deficit = request->hero_max_hp - request->hero_hp;
        if (hp_deficit <= 0 || request->hero_mp <= 0) {
            receipt->success = 0;
            break;
        }
        receipt->healed_self = 1;
        /* The heal loop converts 2 MP per heal_amount HP.
         * heal_amount = min(skill_level, 10).
         * Actual HP/MP changes require live hero data. */
        receipt->fail_closed = 1;
        break;
    }

    case 41: /* TURN/POSITION swap */
        receipt->position_swapped = 1;
        receipt->fail_closed = 1;
        break;

    case 43: /* SET_MINION_DESTINATION */
        receipt->minion_dest_set = 1;
        receipt->fail_closed = 1;
        break;

    case 44: /* MINION missile */
    case 45: /* MINION attack */
        receipt->fail_closed = 1;
        break;

    case 46: /* CREATE_MINION type 0x30 */
    case 48: /* CREATE_MINION type 0x31 */
    case 49: /* CREATE_MINION type 0x34 */
    case 50: /* CREATE_MINION type 0x35 */
        receipt->minion_created = 1;
        receipt->fail_closed = 1;
        break;

    case 47: /* RELEASE_MINION */
        receipt->minion_released = 1;
        receipt->fail_closed = 1;
        break;

    case 53: /* LOAD_GDAT_INTERFACE */
        receipt->interface_loaded = 1;
        receipt->fail_closed = 1;
        break;

    default:
        receipt->success = 0;
        break;
    }

    /* Epilogue: stamina, skill experience, cooldown.
     * skengage.cpp:824-868 */
    if (receipt->success && !request->cmd_flag_8000) {
        receipt->stamina_cost = request->cmd.power_random;
        receipt->skill_exp_gained = request->cmd.skill_exp;
    }

    return receipt->success ? 1 : 0;
}
