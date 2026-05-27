#ifndef NEXUS_V1_SCRIPT_VM_H
#define NEXUS_V1_SCRIPT_VM_H

#include <stdint.h>
#include "nexus_v1_world.h"  /* for Nexus_WorldOpcode enum */

/* Nexus V1 script VM — SDDRVS.TSK bytecode parser + trigger dispatcher.
 * Source: docs/nexus_triggers.md (SDDRVS.TSK analysis),
 * docs/nexus_sensors.md (Nexus trigger model vs DM1/DM2).
 *
 * SDDRVS.TSK = "Saturn Dungeon Run VS" script file.
 * Format: [WHEN condition] THEN [action] rules.
 * Unlike DM1 (tile-type hardwired) or DM2 (actuator enum dispatch),
 * Nexus uses declarative condition→action rules processed at runtime.
 *
 * Opcode constants are defined in nexus_v1_world.h (Nexus_WorldOpcode).
 * Current status: STUB — structural analysis of 5,448-byte file done,
 * actual opcode/operand format is unknown. */

/* Condition structure — describes a WHEN condition */
typedef struct {
    Nexus_WorldOpcode opcode;
    int x, y;             /* for XY conditions */
    int value;            /* for HAS/LEVEL/DEAD conditions */
    int target_x, target_y, target_level; /* teleport targets */
} Nexus_ScriptCondition;

/* Action structure — describes a THEN action */
typedef struct {
    Nexus_WorldOpcode opcode;
    int x, y;             /* target position */
    int value;            /* generic value param */
    int level;            /* target level for teleports/spawns */
    int item_id;          /* for give item */
    int message_id;       /* for display message */
    int flag_index;       /* for set flag */
} Nexus_ScriptAction;

/* One script rule: WHEN condition THEN action */
typedef struct {
    int rule_id;
    int enabled;
    int once_only;         /* fires once, then disabled */
    int fired_count;      /* times fired */
    Nexus_ScriptCondition cond;
    Nexus_ScriptAction action;
} Nexus_ScriptRule;

/* Script VM state */
#define NEXUS_SCRIPT_MAX_RULES 256
#define NEXUS_SCRIPT_MAX_FLAGS 32

typedef struct {
    Nexus_ScriptRule rules[NEXUS_SCRIPT_MAX_RULES];
    int rule_count;
    uint8_t flags[NEXUS_SCRIPT_MAX_FLAGS];
    int initialized;
    int current_level;
} Nexus_ScriptVM;

/* Init script VM (call at game start) */
void nexus_script_vm_init(Nexus_ScriptVM *vm);

/* Load SDDRVS.TSK script data for a level (0-15).
 * Pass raw file bytes and size.
 * Returns 0 on success, -1 on error.
 * TODO: actual SDDRVS.TSK bytecode format unknown — this is a stub
 * that accepts the data but does not yet parse opcodes.
 * Source: docs/nexus_triggers.md SDDRVS.TSK analysis. */
int nexus_script_vm_load_level(Nexus_ScriptVM *vm, int level_index,
                                const uint8_t *data, int size);

/* Unload scripts for current level */
void nexus_script_vm_unload(Nexus_ScriptVM *vm);

/* Trigger evaluation — call these from game logic */
void nexus_script_on_party_move(Nexus_ScriptVM *vm, int x, int y, int level);
void nexus_script_on_champion_item(Nexus_ScriptVM *vm, int champ_idx, int item_id);
void nexus_script_on_creature_dead(Nexus_ScriptVM *vm, int creature_type);
void nexus_script_on_door_change(Nexus_ScriptVM *vm, int x, int y, int is_open);
void nexus_script_on_item_used(Nexus_ScriptVM *vm, int item_id);
void nexus_script_on_level_load(Nexus_ScriptVM *vm, int level_index);

/* Action handler callback */
typedef void (*Nexus_ScriptActionHandler)(const Nexus_ScriptAction *action, void *user_data);
void nexus_script_vm_set_handler(Nexus_ScriptVM *vm,
                                   Nexus_ScriptActionHandler handler,
                                   void *user_data);

/* Manually fire a rule (for testing) */
int nexus_script_vm_fire_rule(Nexus_ScriptVM *vm, int rule_id);

/* Debug: dump all rules */
void nexus_script_vm_dump(const Nexus_ScriptVM *vm);

#endif /* NEXUS_V1_SCRIPT_VM_H */