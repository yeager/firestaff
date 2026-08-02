
#ifndef NEXUS_V1_TRIGGERS_H
#define NEXUS_V1_TRIGGERS_H

/* Nexus V1 trigger system — floor pressure plates and wall switches.
 * Triggers link a source (plate/switch position) to a target action
 * (open/close/toggle door, activate teleporter, spawn creature, etc).
 * Source: DM1 MOVESENS.C F0267/F0268 walk-on/walk-off sensors,
 *         DUNGEON.C sensor records, ReDMCSB COMMAND.C wall click. */

#include <stdint.h>

#define NEXUS_MAX_TRIGGERS 128

#define NEXUS_TRIGGER_FLOOR_PLATE    0
#define NEXUS_TRIGGER_WALL_SWITCH    1
#define NEXUS_TRIGGER_FLOOR_PAD      2

#define NEXUS_TRIGGER_ACT_TOGGLE_DOOR    0
#define NEXUS_TRIGGER_ACT_OPEN_DOOR      1
#define NEXUS_TRIGGER_ACT_CLOSE_DOOR     2
#define NEXUS_TRIGGER_ACT_TELEPORT       3
#define NEXUS_TRIGGER_ACT_ALARM          4
#define NEXUS_TRIGGER_ACT_PIT_OPEN       5
#define NEXUS_TRIGGER_ACT_PIT_CLOSE      6
#define NEXUS_TRIGGER_ACT_MESSAGE        7

typedef struct {
    int source_x, source_y;
    int source_wall;         /* 0-3 for wall switches (N/E/S/W), -1 for floor */
    int target_x, target_y;
    int trigger_type;        /* NEXUS_TRIGGER_* */
    int action;              /* NEXUS_TRIGGER_ACT_* */
    int activated;           /* 1 if currently depressed/activated */
    int once_only;           /* 1 if single-use trigger */
    int used;                /* 1 if single-use trigger has fired */
} Nexus_Trigger;

typedef struct {
    Nexus_Trigger triggers[NEXUS_MAX_TRIGGERS];
    int count;
} Nexus_TriggerManager;

void nexus_v1_triggers_init(Nexus_TriggerManager *mgr);

int nexus_v1_trigger_register(Nexus_TriggerManager *mgr,
                               int src_x, int src_y, int src_wall,
                               int tgt_x, int tgt_y,
                               int type, int action, int once_only);

/* Check floor triggers when party steps on (x,y).
 * Returns number of triggers activated. */
int nexus_v1_triggers_on_step(Nexus_TriggerManager *mgr, int x, int y);

/* Check floor triggers when party steps off (x,y).
 * Deactivates pressure plates. Returns number deactivated. */
int nexus_v1_triggers_off_step(Nexus_TriggerManager *mgr, int x, int y);

/* Activate wall switch at (x,y) wall side.
 * Returns number of triggers activated. */
int nexus_v1_triggers_wall_click(Nexus_TriggerManager *mgr,
                                  int x, int y, int wall);

/* Get the action for a specific activated trigger (for caller dispatch). */
int nexus_v1_trigger_get_action(const Nexus_TriggerManager *mgr, int index);
int nexus_v1_trigger_get_target(const Nexus_TriggerManager *mgr, int index,
                                 int *out_x, int *out_y);

int nexus_v1_triggers_count(const Nexus_TriggerManager *mgr);

#endif
