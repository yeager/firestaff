#ifndef NEXUS_V1_SCRIPT_VM_H
#define NEXUS_V1_SCRIPT_VM_H

#include <stdint.h>
#include "nexus_v1_world.h"  /* for Nexus_WorldOpcode enum */

/* Nexus V1 provisional trigger VM + dispatcher.
 * Source: docs/nexus_triggers.md (unresolved trigger owner),
 * docs/nexus_sensors.md (Nexus trigger model vs DM1/DM2).
 *
 * SDDRVS.TSK is now classified as a Saturn sound-driver task, not a proven
 * trigger bytecode file. SLEV*.BIN / DGN metadata remain candidate trigger
 * owners. This module is bounded scaffolding for deterministic condition ->
 * action dispatch while the real Nexus trigger format is still unresolved.
 *
 * Provisional format: [WHEN condition] THEN [action] rules.
 * Unlike DM1 (tile-type hardwired) or DM2 (actuator enum dispatch), this API
 * models a declarative condition -> action dispatcher for future Nexus proof.
 *
 * Opcode constants are defined in nexus_v1_world.h (Nexus_WorldOpcode).
 * Current status: no real Nexus script/trigger bytecode parser. */

struct Nexus_ScriptAction;
typedef void (*Nexus_ScriptActionHandler)(const struct Nexus_ScriptAction *action,
                                          void *user_data);

/* Condition structure — describes a WHEN condition */
typedef struct {
    Nexus_WorldOpcode opcode;
    int x, y;             /* for XY conditions */
    int value;            /* for HAS/LEVEL/DEAD conditions */
    int target_x, target_y, target_level; /* teleport targets */
} Nexus_ScriptCondition;

/* Action structure — describes a THEN action */
typedef struct Nexus_ScriptAction {
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
    int candidate_source_loaded;
    int candidate_source_bytes;
    int parser_supported;
    int dispatch_enabled;
    Nexus_ScriptActionHandler handler;
    void *handler_data;
} Nexus_ScriptVM;

typedef enum {
    NEXUS_SCRIPT_RUNTIME_MISSING = 0,
    NEXUS_SCRIPT_RUNTIME_READY_PARSED = 1,
    NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT = 2,
    NEXUS_SCRIPT_RUNTIME_NO_SOURCE = 3
} Nexus_ScriptRuntimeStatus;

typedef struct {
    Nexus_ScriptRuntimeStatus status;
    int level_index;
    int candidate_source_loaded;
    int candidate_source_bytes;
    int parser_supported;
    int dispatch_enabled;
    int rules_loaded;
    int blocks_real_script_dispatch;
    int fallback_visuals_permitted;
} Nexus_ScriptRuntimeReceipt;

/* Init script VM (call at game start) */
void nexus_script_vm_init(Nexus_ScriptVM *vm);

/* Load candidate trigger/script data for a level (0-15).
 * Pass raw file bytes and size.
 * Returns 0 on success, -1 on error.
 * TODO: actual Nexus trigger bytecode/record format unknown — this is a stub
 * that accepts the data but does not yet parse opcodes.
 * Source: docs/nexus_triggers.md unresolved SLEV*.BIN/DGN trigger owner. */
int nexus_script_vm_load_level(Nexus_ScriptVM *vm, int level_index,
                                const uint8_t *data, int size);
int nexus_script_vm_runtime_receipt(const Nexus_ScriptVM *vm,
                                    Nexus_ScriptRuntimeReceipt *out_receipt);
const char *nexus_script_runtime_status_name(
    Nexus_ScriptRuntimeStatus status);

/* Unload scripts for current level */
void nexus_script_vm_unload(Nexus_ScriptVM *vm);

/* Trigger evaluation — call these from game logic */
void nexus_script_on_party_move(Nexus_ScriptVM *vm, int x, int y, int level);
void nexus_script_on_champion_item(Nexus_ScriptVM *vm, int champ_idx, int item_id);
void nexus_script_on_creature_dead(Nexus_ScriptVM *vm, int creature_type);
void nexus_script_on_door_change(Nexus_ScriptVM *vm, int x, int y, int is_open);
void nexus_script_on_item_used(Nexus_ScriptVM *vm, int item_id);
void nexus_script_on_level_load(Nexus_ScriptVM *vm, int level_index);

void nexus_script_vm_set_handler(Nexus_ScriptVM *vm,
                                   Nexus_ScriptActionHandler handler,
                                   void *user_data);

/* Manually fire a rule (for testing) */
int nexus_script_vm_fire_rule(Nexus_ScriptVM *vm, int rule_id);

/* Debug: dump all rules */
void nexus_script_vm_dump(const Nexus_ScriptVM *vm);

#endif /* NEXUS_V1_SCRIPT_VM_H */
