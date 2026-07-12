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
 * owners. Canonical SLEV bytes have one bounded, corpus-evidenced SH-2 task
 * entry parser and otherwise remain blocked from dispatch.
 *
 * Provisional format: [WHEN condition] THEN [action] rules.
 * Unlike DM1 (tile-type hardwired) or DM2 (actuator enum dispatch), this API
 * models a declarative condition -> action dispatcher for future Nexus proof.
 *
 * Opcode constants are defined in nexus_v1_world.h (Nexus_WorldOpcode).
 * Current status: task-header receipts only; no SLEV task opcode or record is
 * promoted to a trigger/action rule. */

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

/* Provenance of operands in the only corpus-verified SLEV task entry. These
 * describe instruction encoding and destination register, not literal
 * ownership or a task-body semantic. */
typedef enum {
    NEXUS_SLEV_SETUP_IMMEDIATE_NONE = 0,
    NEXUS_SLEV_SETUP_IMMEDIATE_SH2_MOV_R2 = 1
} Nexus_SlevSetupImmediateProvenance;

typedef enum {
    NEXUS_SLEV_LITERAL_NONE = 0,
    NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R3 = 1,
    NEXUS_SLEV_LITERAL_SH2_MOVL_PC_RELATIVE_R0 = 2
} Nexus_SlevLiteralProvenance;

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
    int canonical_source_verified;
    int parser_supported;
    int dispatch_enabled;
    int parsed_record_size;
    int parsed_rule_count;
    int real_task_profile_supported;
    int real_task_word_count;
    int real_task_first_opcode;
    int real_task_rts_count;
    int real_task_branch_count;
    int real_task_immediate_count;
    int real_task_jsr_count;
    int real_task_pc_relative_load_count;
    int real_task_literal_pointer_count;
    int real_task_first_literal_offset;
    int real_task_first_literal_address;
    int real_task_last_literal_address;
    int real_task_checksum16;
    int real_task_header_supported;
    int real_task_header_size;
    int real_task_setup_immediate;
    Nexus_SlevSetupImmediateProvenance real_task_setup_immediate_provenance;
    int real_task_primary_literal_offset;
    int real_task_primary_literal_address;
    Nexus_SlevLiteralProvenance real_task_primary_literal_provenance;
    int real_task_aux_literal_offset;
    int real_task_aux_literal_address;
    Nexus_SlevLiteralProvenance real_task_aux_literal_provenance;
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
    int canonical_source_verified;
    int parser_supported;
    int dispatch_enabled;
    int parsed_record_size;
    int parsed_rule_count;
    int real_task_profile_supported;
    int real_task_word_count;
    int real_task_first_opcode;
    int real_task_rts_count;
    int real_task_branch_count;
    int real_task_immediate_count;
    int real_task_jsr_count;
    int real_task_pc_relative_load_count;
    int real_task_literal_pointer_count;
    int real_task_first_literal_offset;
    int real_task_first_literal_address;
    int real_task_last_literal_address;
    int real_task_checksum16;
    int real_task_header_supported;
    int real_task_header_size;
    int real_task_setup_immediate;
    Nexus_SlevSetupImmediateProvenance real_task_setup_immediate_provenance;
    int real_task_primary_literal_offset;
    int real_task_primary_literal_address;
    Nexus_SlevLiteralProvenance real_task_primary_literal_provenance;
    int real_task_aux_literal_offset;
    int real_task_aux_literal_address;
    Nexus_SlevLiteralProvenance real_task_aux_literal_provenance;
    int rules_loaded;
    int blocks_real_script_dispatch;
    int fallback_visuals_permitted;
} Nexus_ScriptRuntimeReceipt;

/* Init script VM (call at game start) */
void nexus_script_vm_init(Nexus_ScriptVM *vm);

/* Load candidate trigger/script data for a level (0-15).
 * Pass raw file bytes and size.
 * Returns 0 on success, -1 on error.
 * The canonical parser recognizes only the observed 36-byte SH-2 task entry
 * grammar and its two in-file PC-relative literals. Task-body bytes are kept
 * profile-only and never create condition/action rules. */
int nexus_script_vm_load_level(Nexus_ScriptVM *vm, int level_index,
                                const uint8_t *data, int size);
/* The canonical runtime must bind SLEV bytes to their Track 1 hash before
 * exposing even bounded parser/profile evidence. Unknown opcodes never run. */
int nexus_script_vm_load_canonical_level(Nexus_ScriptVM *vm, int level_index,
                                          const uint8_t *data, int size,
                                          int canonical_source_verified);
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
