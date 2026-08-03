
#ifndef NEXUS_V1_SWITCHES_H
#define NEXUS_V1_SWITCHES_H

/* Nexus V1 lever/switch/button system — wall-mounted or floor switches
 * that trigger doors, traps, or other events when activated.
 * Source: DM1 MOVESENS.C F0267/F0268 sensor processing,
 *         DUNGEON.C F0107 sensor thing types,
 *         DM Nexus Saturn: SDDRVS.TSK switch driver. */

#include <stdint.h>

#define NEXUS_MAX_SWITCHES 128

enum {
    NEXUS_SWITCH_LEVER       = 0,
    NEXUS_SWITCH_BUTTON      = 1,
    NEXUS_SWITCH_PRESSURE    = 2,
    NEXUS_SWITCH_HIDDEN      = 3
};

enum {
    NEXUS_SWITCH_TARGET_DOOR = 0,
    NEXUS_SWITCH_TARGET_TRAP = 1,
    NEXUS_SWITCH_TARGET_TELEPORTER = 2,
    NEXUS_SWITCH_TARGET_SCRIPT = 3
};

typedef struct {
    int active;
    int type;
    int map_x, map_y;
    int state;
    int target_type;
    int target_id;
    int once_only;
    int used;
} Nexus_Switch;

typedef struct {
    Nexus_Switch switches[NEXUS_MAX_SWITCHES];
    int count;
} Nexus_SwitchManager;

void nexus_v1_switch_manager_init(Nexus_SwitchManager *mgr);

int nexus_v1_switch_register(Nexus_SwitchManager *mgr,
    int type, int map_x, int map_y,
    int target_type, int target_id, int once_only);

int nexus_v1_switch_find_at(const Nexus_SwitchManager *mgr,
    int map_x, int map_y);

/* Activate a switch. Returns the target_id to act on, or -1. */
typedef struct {
    int activated;
    int target_type;
    int target_id;
} Nexus_SwitchResult;

Nexus_SwitchResult nexus_v1_switch_activate(Nexus_SwitchManager *mgr,
    int switch_idx);

int nexus_v1_switch_get_state(const Nexus_SwitchManager *mgr, int switch_idx);

#endif
