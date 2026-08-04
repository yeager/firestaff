#ifndef FIRESTAFF_DM2_V1_ENGAGE_COMMAND_PC34_COMPAT_H
#define FIRESTAFF_DM2_V1_ENGAGE_COMMAND_PC34_COMPAT_H

/*
 * dm2_v1_engage_command_pc34_compat.h — DM2 hand action dispatcher.
 *
 * Ports DM2_ENGAGE_COMMAND from skproject/SKWINSPX/src/v5/skengage.cpp.
 * This is the 53-case switch that dispatches all hand actions: melee
 * attack, spell cast, throw, consume, wield, heal, confuse, light,
 * minion control, etc.
 *
 * Each case maps to a command string entry (CMDSTR) that encodes the
 * action type, skill, power, delay, and sound effect.  The dispatcher
 * resolves the hero, hand, item, target tile, and creature, then
 * delegates to the appropriate sub-function.
 *
 * Source: skproject/SKWINSPX/src/v5/skengage.cpp DM2_ENGAGE_COMMAND
 */

#include <stdint.h>
#include "dm2_v1_light_ops_pc34_compat.h"
#include "dm2_v1_creature_ops_pc34_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Engage command types — the action_id values (vw_54 - 1 in reference).
 * Each corresponds to a case in the ENGAGE_COMMAND switch.
 * Source: skengage.cpp:178-657 */
typedef enum {
    DM2_ENGAGE_ATTACK           = 1,
    DM2_ENGAGE_CAST_MISSILE     = 2,
    DM2_ENGAGE_WIELD_WEAPON     = 3,
    DM2_ENGAGE_CONFUSE          = 4,
    DM2_ENGAGE_LIGHT_1          = 5,
    DM2_ENGAGE_CREATE_CLOUD     = 6,
    DM2_ENGAGE_WIELD_WEAPON_2   = 7,
    DM2_ENGAGE_HEAL_PARTY       = 8,
    DM2_ENGAGE_STEP_FORWARD     = 9,
    DM2_ENGAGE_MANA_GAIN        = 10,
    DM2_ENGAGE_ABILITY_5        = 11,
    DM2_ENGAGE_ABILITY_4        = 12,
    DM2_ENGAGE_ABILITY_6        = 13,
    DM2_ENGAGE_ABILITY_3        = 14,
    DM2_ENGAGE_CONSUME          = 15,
    DM2_ENGAGE_EQUIP_HAND       = 16,
    DM2_ENGAGE_SHOOT_MISSILE    = 31,
    DM2_ENGAGE_HERO_ACT_1       = 32,
    DM2_ENGAGE_HERO_ACT_0       = 33,
    DM2_ENGAGE_HERO_ACT_2       = 34,
    DM2_ENGAGE_HEAL_SELF        = 35,
    DM2_ENGAGE_LIGHT_2          = 37,
    DM2_ENGAGE_LIGHT_3          = 38,
    DM2_ENGAGE_TURN_POSITION    = 41,
    DM2_ENGAGE_SET_MINION_DEST  = 43,
    DM2_ENGAGE_MINION_MISSILE   = 44,
    DM2_ENGAGE_MINION_ATTACK    = 45,
    DM2_ENGAGE_CREATE_MINION_0  = 46,
    DM2_ENGAGE_RELEASE_MINION   = 47,
    DM2_ENGAGE_CREATE_MINION_1  = 48,
    DM2_ENGAGE_CREATE_MINION_2  = 49,
    DM2_ENGAGE_CREATE_MINION_3  = 50,
    DM2_ENGAGE_LOAD_INTERFACE   = 53
} DM2_V1_EngageCommandType;

/* Command string entry — the pre-resolved action parameters.
 * In the reference these come from DM2_QUERY_CUR_CMDSTR_ENTRY(n)
 * where n indexes fields of the current command definition.
 *
 * Source: skengage.cpp various QUERY_CUR_CMDSTR_ENTRY calls */
typedef struct {
    int16_t skill_type;       /* entry 0: primary skill */
    int16_t action_id;        /* entry 2: action case id (vw_54) */
    int16_t power_base;       /* entry 3: base power (vol_20) */
    int16_t power_random;     /* entry 4: power + RANDBIT */
    int16_t delay;            /* entry 5: action delay (vw_60) */
    int16_t spell_param;      /* entry 6: spell parameter */
    int16_t defense_class;    /* entry 7: hand defense class */
    int16_t skill_exp;        /* entry 9: skill experience (vo_68) */
    int16_t sound_effect;     /* entry 0xd: sound effect id */
    int16_t cooldown_base;    /* entry 0xe: cooldown duration */
    int16_t damage_type;      /* entry 0x10: damage type (vw_64) */
} DM2_V1_EngageCmdStr;

/* Engage command request — all inputs the dispatcher needs. */
typedef struct {
    int hero_index;
    int hand;                 /* 0 = left, 1 = right (curactmode) */
    int hero_alive;
    int16_t hero_hp;
    int16_t hero_max_hp;
    int16_t hero_mp;
    int16_t hero_abs_dir;
    int16_t hero_party_pos;
    int16_t item_handle;      /* item in hand, -1 if empty */
    int16_t party_x;
    int16_t party_y;
    int16_t party_dir;
    int16_t party_map;
    int16_t creature_at_target; /* creature handle at target tile, -1 if none */
    int16_t tile_value;       /* raw tile at target */
    uint8_t cmd_flag_8000;    /* high bit of command word */
    DM2_V1_EngageCmdStr cmd;
    uint32_t game_tick;

    /* Optional callbacks for cases that delegate to subsystems.
     * When NULL, the case falls through to fail_closed. */
    const DM2_V1_ProceedLightCallbacks *light_cb;
    void *light_ctx;
    const DM2_V1_ConfuseCreatureSimpleCallbacks *confuse_cb;
    void *confuse_ctx;
    int32_t confuse_damage;   /* skengage.cpp:322: RG61l (accumulated) */

    void *hero;               /* DM2_V1_HeroState* (for MP deduction) */
    const void *enchant_cb;   /* DM2_V1_CastEnchantCallbacks* */
    void *enchant_ctx;
    int16_t timer_type;       /* vw_54: timer subtype for ACT mapping */

    const void *shoot_cb;     /* DM2_V1_ShootMissileCallbacks* */
    void *shoot_ctx;
    uint16_t shoot_item;      /* item handle for SHOOT/CAST_MISSILE */
    int16_t shoot_damage;
    int16_t shoot_kinetic;
    int16_t shoot_spell_power;
    int16_t shoot_mp_cost;

    /* case 1 ATTACK: queue timer 0x47 */
    int (*queue_timer_cb)(void *ctx, uint8_t type, int16_t delay,
                          uint32_t game_tick, int16_t map);
    void *queue_timer_ctx;
    uint8_t attack_counter;   /* savegames1.b_02 pre-increment value */
    int16_t attack_hero_flag; /* v1e0976: hero index+1 that gets 0x4000 flag */

    /* case 3/7 WIELD: wield weapon + attack door/creature */
    const void *wield_cb;     /* DM2_V1_WieldWeaponCallbacks* */
    void *wield_ctx;
    int16_t wield_strength;   /* pre-computed attack/throw strength */

    /* case 6 CLOUD: create cloud at party position */
    int (*create_cloud_cb)(void *ctx, int16_t type, int16_t power,
                           int16_t x, int16_t y, uint8_t duration);
    void *create_cloud_ctx;

    /* case 9 STEP: move party into creature tile */
    int (*move_record_cb)(void *ctx, int16_t src_x, int16_t src_y,
                          int16_t dst_x, int16_t dst_y);
    void *move_record_ctx;
    int16_t step_target_x;    /* pre-computed facing tile coords */
    int16_t step_target_y;
    int16_t step_creature_flags; /* creature AI spec flags at target */
    int16_t step_tile_type;   /* tile type at step target (bits 7:5) */

    /* case 15 CONSUME */
    int (*consume_cb)(void *ctx, int hero_idx, uint16_t item, int hand);
    void *consume_ctx;

    /* case 16 EQUIP */
    int (*equip_cb)(void *ctx, int hero_idx, int hand);
    void *equip_ctx;

    /* case 41 TURN: position swap */
    int (*turn_cb)(void *ctx, int hero_idx, int hand, int16_t swap_dir);
    void *turn_ctx;

    /* case 43 SET_MINION_DEST */
    int (*set_minion_dest_cb)(void *ctx, uint16_t item,
                              int16_t x, int16_t y, int16_t map);
    void *set_minion_dest_ctx;

    /* case 47 RELEASE_MINION */
    int (*release_minion_cb)(void *ctx, uint16_t minion_record);
    void *release_minion_ctx;
    uint16_t minion_record;   /* word_at(item_record, 2) */

    /* case 46/48/49/50 CREATE_MINION */
    int (*create_minion_cb)(void *ctx, uint8_t type, int16_t power,
                            int16_t dir, int16_t x, int16_t y,
                            int16_t map, uint16_t item, int8_t facing);
    void *create_minion_ctx;

    /* case 53 LOAD_GDAT_INTERFACE */
    int (*load_interface_cb)(void *ctx);
    void *load_interface_ctx;
} DM2_V1_EngageCommandRequest;

/* Engage command receipt — what the dispatcher did. */
typedef struct {
    int valid;
    int fail_closed;
    int hero_dead;
    int action_dispatched;

    DM2_V1_EngageCommandType action_type;
    int success;

    int attack_queued;
    int spell_cast;
    int weapon_wielded;
    int creature_confused;
    int light_toggled;
    int cloud_created;
    int consumed;
    int equipped;
    int missile_shot;
    int healed_self;
    int healed_party;
    int mana_gained;
    int stepped_forward;
    int minion_created;
    int minion_released;
    int minion_dest_set;
    int position_swapped;
    int interface_loaded;
    int enchantment_cast;
    int hero_flag_set;
    int door_attacked;
    int creature_attacked;

    int16_t cooldown_applied;
    int16_t stamina_cost;
    int16_t skill_exp_gained;
} DM2_V1_EngageCommandReceipt;

/* DM2_ENGAGE_COMMAND — dispatch a hand action.
 *
 * Takes the hero index and the resolved command parameters.
 * Returns 1 if the action succeeded, 0 if it failed or was
 * blocked (hero dead, invalid action, etc.).
 *
 * Most sub-functions (WIELD_WEAPON, CAST_CHAMPION_MISSILE,
 * CREATE_MINION, etc.) are fail-closed until their respective
 * modules are bound to live game data.
 *
 * Source: skengage.cpp:23-869 DM2_ENGAGE_COMMAND */
int dm2_v1_engage_command(
    const DM2_V1_EngageCommandRequest *request,
    DM2_V1_EngageCommandReceipt *receipt);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_DM2_V1_ENGAGE_COMMAND_PC34_COMPAT_H */
