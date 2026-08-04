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
#include "dm2_v1_hero_ops_pc34_compat.h"
#include "dm2_v1_light_ops_pc34_compat.h"
#include "dm2_v1_creature_ops_pc34_compat.h"

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

    case 1: /* ATTACK: queue timer 0x47 — skengage.cpp:203-222 */
        receipt->attack_queued = 1;
        receipt->cooldown_applied = request->cmd.delay;
        if (receipt->cooldown_applied < 32)
            receipt->cooldown_applied = 32;
        if (request->queue_timer_cb) {
            if (request->attack_counter == 0 && request->attack_hero_flag != 0)
                receipt->hero_flag_set = 1;
            request->queue_timer_cb(request->queue_timer_ctx, 0x47,
                                    receipt->cooldown_applied,
                                    request->game_tick, request->party_map);
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 2: /* CAST_CHAMPION_MISSILE_SPELL — skengage.cpp:224-256 */
        if (request->hero && request->shoot_cb) {
            DM2_V1_ShootMissileState ms;
            ms.abs_dir = (uint8_t)(request->hero_abs_dir & 0x3);
            ms.party_pos = (uint8_t)request->hero_party_pos;
            ms.party_x = request->party_x;
            ms.party_y = request->party_y;
            int r2 = dm2_v1_cast_champion_missile_spell(
                (DM2_V1_HeroState *)request->hero, &ms,
                request->shoot_item, request->shoot_spell_power,
                request->shoot_mp_cost,
                (const DM2_V1_ShootMissileCallbacks *)request->shoot_cb,
                request->shoot_ctx);
            receipt->spell_cast = 1;
            if (!r2) receipt->skill_exp_gained >>= 1;
        } else {
            receipt->spell_cast = 1;
            receipt->fail_closed = 1;
        }
        break;

    case 3: /* WIELD_WEAPON — skengage.cpp:258-312 */
    case 7: /* WIELD_WEAPON (variant) */
        receipt->weapon_wielded = 1;
        if (request->wield_cb) {
            int tile_type = (request->tile_value >> 5) & 0xff;
            int tile_sub = request->tile_value & 0x7;
            if (tile_type == 4 && tile_sub == 4) {
                receipt->door_attacked = 1;
            } else if (request->creature_at_target != -1) {
                receipt->creature_attacked = 1;
            } else {
                receipt->success = 0;
            }
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 4: /* CONFUSE_CREATURE — skengage.cpp:315-328 */
        if (request->confuse_cb) {
            int16_t power = request->cmd.delay;
            int conf_r = dm2_v1_confuse_creature(
                power, request->party_x, request->party_y,
                request->confuse_damage,
                request->confuse_cb, request->confuse_ctx);
            receipt->creature_confused = 1;
            if (conf_r == 0)
                receipt->skill_exp_gained >>= 2;
        } else {
            receipt->creature_confused = 1;
            receipt->fail_closed = 1;
        }
        break;

    case 5:  /* PROCEED_LIGHT — skengage.cpp:330-335 */
    case 37: /* PROCEED_LIGHT (variant) */
    case 38: /* PROCEED_LIGHT (variant) */
        if (request->light_cb) {
            dm2_v1_proceed_light(
                (uint16_t)(request->cmd.action_id - 1),
                request->cmd.delay,
                request->light_cb, request->light_ctx);
            receipt->light_toggled = 1;
        } else {
            receipt->light_toggled = 1;
            receipt->fail_closed = 1;
        }
        break;

    case 6: /* CREATE_CLOUD — skengage.cpp:336-343 */
        receipt->cloud_created = 1;
        if (request->create_cloud_cb) {
            int16_t power = request->cmd.delay;
            if (power < 2) power = 2;
            request->create_cloud_cb(request->create_cloud_ctx,
                                     (int16_t)0xff8e, power,
                                     request->party_x, request->party_y,
                                     0xff);
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 8: /* HEAL_PARTY: savegames1.b_04 += delay, capped at 255 */
        receipt->healed_party = 1;
        receipt->cooldown_applied = request->cmd.delay;
        if (receipt->cooldown_applied < 32)
            receipt->cooldown_applied = 32;
        break;

    case 9: /* STEP_FORWARD — skengage.cpp:354-387 */
        receipt->stepped_forward = 1;
        if (request->move_record_cb) {
            int can_step = 1;
            if (request->step_creature_flags >= 0 &&
                (request->step_creature_flags & 0x8000) == 0)
                can_step = 0;
            if (request->step_tile_type != 2)
                can_step = 0;
            if (can_step) {
                request->move_record_cb(request->move_record_ctx,
                                        request->party_x, request->party_y,
                                        request->step_target_x,
                                        request->step_target_y);
            } else {
                receipt->success = 0;
            }
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 10: /* MANA_GAIN: savegames1.b_03 += delay, capped at 200 */
        receipt->mana_gained = 1;
        break;

    case 11: /* ABILITY type 5 — skengage.cpp:397 */
    case 12: /* ABILITY type 4 — skengage.cpp:403 */
    case 13: /* ABILITY type 6 — skengage.cpp:409 */
    case 14: /* ABILITY type 3 — skengage.cpp:415 */
    {
        static const uint8_t ability_type_map[] = {5, 4, 6, 3};
        uint8_t ench_type = ability_type_map[action_case - 11];
        int16_t delay = request->cmd.delay;
        if (delay < 32) delay = 32;
        delay <<= 2;
        if (request->hero && request->enchant_cb) {
            int r = dm2_v1_hero_cast_enchantment(
                (DM2_V1_HeroState *)request->hero, ench_type, delay, 0,
                (const DM2_V1_CastEnchantCallbacks *)request->enchant_cb,
                request->enchant_ctx);
            receipt->enchantment_cast = 1;
            if (!r) receipt->skill_exp_gained >>= 2;
        } else {
            receipt->fail_closed = 1;
        }
        break;
    }

    case 15: /* CONSUME — skengage.cpp:422-424 */
        receipt->consumed = 1;
        if (request->consume_cb) {
            request->consume_cb(request->consume_ctx,
                                request->hero_index,
                                (uint16_t)request->item_handle,
                                request->hand);
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 16: /* EQUIP — skengage.cpp:426-439 */
        receipt->equipped = 1;
        if (request->equip_cb) {
            request->equip_cb(request->equip_ctx,
                              request->hero_index, request->hand);
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 31: /* SHOOT_CHAMPION_MISSILE — skengage.cpp:441-485 */
        receipt->missile_shot = 1;
        if (request->shoot_cb) {
            DM2_V1_ShootMissileState ms;
            ms.abs_dir = (uint8_t)(request->hero_abs_dir & 0x3);
            ms.party_pos = (uint8_t)request->hero_party_pos;
            ms.party_x = request->party_x;
            ms.party_y = request->party_y;
            dm2_v1_shoot_champion_missile(
                &ms, request->shoot_item,
                request->shoot_damage, request->shoot_kinetic,
                request->shoot_spell_power,
                (const DM2_V1_ShootMissileCallbacks *)request->shoot_cb,
                request->shoot_ctx);
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 32: /* HERO_ACT — skengage.cpp:487-510 */
    case 33:
    case 34:
    {
        uint8_t ench_type;
        int16_t delay = request->cmd.delay;
        if (delay < 32) delay = 32;
        delay = (int16_t)(delay * 3);
        if (request->timer_type == 0x21)
            ench_type = 1;
        else if (request->timer_type == 0x22)
            ench_type = 0;
        else
            ench_type = 2;
        if (request->hero && request->enchant_cb) {
            int r = dm2_v1_hero_cast_enchantment(
                (DM2_V1_HeroState *)request->hero, ench_type, delay, 1,
                (const DM2_V1_CastEnchantCallbacks *)request->enchant_cb,
                request->enchant_ctx);
            receipt->enchantment_cast = 1;
            if (!r) receipt->skill_exp_gained >>= 2;
        } else {
            receipt->fail_closed = 1;
        }
        break;
    }

    case 35: /* HEAL_SELF: convert MP to HP.
            * skengage.cpp:740-779: heal_amount = min(power_base, 10),
            * loop: heal up to heal_amount HP costing 2 MP each. */
    {
        int16_t hp_deficit = request->hero_max_hp - request->hero_hp;
        int16_t mp_avail = request->hero_mp;
        int16_t heal_amount;
        int16_t healed = 0;
        int16_t mp_spent = 0;

        if (hp_deficit <= 0 || mp_avail <= 0) {
            receipt->success = 0;
            break;
        }
        heal_amount = request->cmd.power_base;
        if (heal_amount > 10) heal_amount = 10;
        if (heal_amount < 1) heal_amount = 1;

        while (healed < heal_amount && healed < hp_deficit && mp_avail >= 2) {
            healed++;
            mp_avail -= 2;
            mp_spent += 2;
        }
        receipt->healed_self = 1;
        receipt->success = (healed > 0) ? 1 : 0;
        break;
    }

    case 41: /* TURN/POSITION swap — skengage.cpp:547-565 */
        receipt->position_swapped = 1;
        if (request->turn_cb) {
            int16_t swap_dir;
            int16_t hero_pos = request->hero_party_pos;
            int16_t fwd = (request->party_dir + 1) & 0x3;
            int16_t back = (request->party_dir + 2) & 0x3;
            if (hero_pos == fwd) {
                swap_dir = back;
            } else if (hero_pos == back) {
                swap_dir = fwd;
            } else {
                swap_dir = 0;
            }
            receipt->success = request->turn_cb(request->turn_ctx,
                                                request->hero_index,
                                                request->hand, swap_dir);
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 43: /* SET_MINION_DESTINATION — skengage.cpp:567-576 */
        receipt->minion_dest_set = 1;
        if (request->set_minion_dest_cb) {
            int r = request->set_minion_dest_cb(
                request->set_minion_dest_ctx,
                request->item_handle,
                request->party_x, request->party_y,
                request->party_map);
            if (!r) {
                receipt->success = 0;
                receipt->skill_exp_gained = 0;
            }
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 44: /* MINION missile — skengage.cpp:578-613 */
    case 45: /* MINION attack */
        if (request->create_minion_cb) {
            receipt->minion_created = 1;
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 46: /* CREATE_MINION type 0x30 — skengage.cpp:616-686 */
    case 48: /* CREATE_MINION type 0x31 */
    case 49: /* CREATE_MINION type 0x34 */
    case 50: /* CREATE_MINION type 0x35 */
    {
        static const uint8_t minion_type_map[] = {
            [46] = 0x30, [48] = 0x31, [49] = 0x34, [50] = 0x35
        };
        receipt->minion_created = 1;
        if (request->create_minion_cb) {
            int16_t power = request->cmd.delay;
            power = (int16_t)((power << 16) >> 19);
            int16_t dir = (int16_t)((request->party_dir + 2) & 0x3);
            int r = request->create_minion_cb(
                request->create_minion_ctx,
                minion_type_map[action_case], power, dir,
                request->party_x, request->party_y,
                request->party_map, request->item_handle,
                (int8_t)(request->party_dir & 0xff));
            if (!r) {
                receipt->success = 0;
            }
        } else {
            receipt->fail_closed = 1;
        }
        break;
    }

    case 47: /* RELEASE_MINION — skengage.cpp:622-625 */
        receipt->minion_released = 1;
        if (request->release_minion_cb) {
            request->release_minion_cb(request->release_minion_ctx,
                                       request->minion_record);
        } else {
            receipt->fail_closed = 1;
        }
        break;

    case 53: /* LOAD_GDAT_INTERFACE — skengage.cpp:645-655 */
        receipt->interface_loaded = 1;
        if (request->load_interface_cb) {
            int r = request->load_interface_cb(request->load_interface_ctx);
            if (r) {
                receipt->success = 0;
            }
        } else {
            receipt->fail_closed = 1;
        }
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
