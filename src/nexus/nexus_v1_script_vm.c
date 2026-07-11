#include "nexus_v1_script_vm.h"
#include "nexus_v1_world.h"
#include <string.h>
#include <stdio.h>

/* Nexus V1 provisional trigger VM + dispatcher.
 * Bounded implementation: the real trigger owner/record format is unresolved.
 * docs/nexus_triggers.md and docs/nexus_sensors.md currently classify
 * SDDRVS.TSK as Saturn sound-driver data; SLEV*.BIN / DGN metadata remain
 * candidate trigger sources. This module provides deterministic runtime
 * condition/action dispatch and accepts only an explicit SLEV rule-table
 * envelope before enabling parsed runtime dispatch. */

#define NEXUS_SLEV_RULE_MAGIC_SIZE 4
#define NEXUS_SLEV_RULE_HEADER_SIZE 8
#define NEXUS_SLEV_RULE_VERSION 1
#define NEXUS_SLEV_RULE_RECORD_SIZE 32

static int nexus_read_u16_le(const uint8_t *p) {
    return p ? (int)((uint16_t)p[0] | ((uint16_t)p[1] << 8)) : 0;
}

static int nexus_read_s16_le(const uint8_t *p) {
    uint16_t v;
    if (!p) return 0;
    v = (uint16_t)nexus_read_u16_le(p);
    return (v & 0x8000u) ? (int)v - 0x10000 : (int)v;
}

static int nexus_read_u16_be(const uint8_t *p) {
    return p ? (int)(((uint16_t)p[0] << 8) | (uint16_t)p[1]) : 0;
}

static int nexus_read_u32_be(const uint8_t *p) {
    return p ? (int)(((uint32_t)p[0] << 24) |
                     ((uint32_t)p[1] << 16) |
                     ((uint32_t)p[2] << 8) |
                     (uint32_t)p[3]) : 0;
}

static int nexus_script_is_condition_opcode(int opcode) {
    return opcode >= NEXUS_OP_WHEN_PARTY_ON_XY &&
           opcode <= NEXUS_OP_WHEN_ITEM_USED;
}

static int nexus_script_is_action_opcode(int opcode) {
    return opcode >= NEXUS_OP_TELEPORT &&
           opcode <= NEXUS_OP_END_GAME;
}

static int nexus_script_parse_slev_rule_table(Nexus_ScriptVM *vm,
                                              const uint8_t *data,
                                              int size) {
    int record_size;
    int count;
    int i;

    if (!vm || !data || size < NEXUS_SLEV_RULE_HEADER_SIZE) return 0;
    if (data[0] != 'S' || data[1] != 'L' ||
        data[2] != 'E' || data[3] != 'V') {
        return 0;
    }
    if (data[4] != NEXUS_SLEV_RULE_VERSION) return 0;
    record_size = data[5];
    if (record_size != NEXUS_SLEV_RULE_RECORD_SIZE) return 0;

    count = nexus_read_u16_le(&data[6]);
    if (count < 0 || count > NEXUS_SCRIPT_MAX_RULES) return 0;
    if (size != NEXUS_SLEV_RULE_HEADER_SIZE + count * record_size) return 0;

    for (i = 0; i < count; i++) {
        const uint8_t *r = data + NEXUS_SLEV_RULE_HEADER_SIZE +
                           i * record_size;
        if (!nexus_script_is_condition_opcode(r[0])) return 0;
        if (!nexus_script_is_action_opcode(r[1])) return 0;
    }

    for (i = 0; i < count; i++) {
        const uint8_t *src = data + NEXUS_SLEV_RULE_HEADER_SIZE +
                             i * record_size;
        Nexus_ScriptRule *dst = &vm->rules[i];

        memset(dst, 0, sizeof(*dst));
        dst->rule_id = nexus_read_u16_le(&src[4]);
        if (dst->rule_id == 0) dst->rule_id = i + 1;
        dst->enabled = (src[2] & 0x02u) ? 0 : 1;
        dst->once_only = (src[2] & 0x01u) ? 1 : 0;
        dst->cond.opcode = (Nexus_WorldOpcode)src[0];
        dst->cond.x = nexus_read_s16_le(&src[6]);
        dst->cond.y = nexus_read_s16_le(&src[8]);
        dst->cond.value = nexus_read_s16_le(&src[10]);
        dst->cond.target_x = nexus_read_s16_le(&src[12]);
        dst->cond.target_y = nexus_read_s16_le(&src[14]);
        dst->cond.target_level = nexus_read_s16_le(&src[16]);
        dst->action.opcode = (Nexus_WorldOpcode)src[1];
        dst->action.x = nexus_read_s16_le(&src[18]);
        dst->action.y = nexus_read_s16_le(&src[20]);
        dst->action.value = nexus_read_s16_le(&src[22]);
        dst->action.level = nexus_read_s16_le(&src[24]);
        dst->action.item_id = nexus_read_s16_le(&src[26]);
        dst->action.message_id = nexus_read_s16_le(&src[28]);
        dst->action.flag_index = nexus_read_s16_le(&src[30]);
    }

    vm->rule_count = count;
    vm->parser_supported = 1;
    vm->dispatch_enabled = count > 0 ? 1 : 0;
    vm->parsed_record_size = record_size;
    vm->parsed_rule_count = count;
    return 1;
}

static void nexus_script_profile_real_slev_task(Nexus_ScriptVM *vm,
                                                const uint8_t *data,
                                                int size) {
    int i;
    int checksum = 0;
    int first_opcode;
    int rts_count = 0;
    int branch_count = 0;
    int immediate_count = 0;
    int jsr_count = 0;
    int pc_relative_load_count = 0;
    int literal_pointer_count = 0;
    int first_literal_offset = -1;
    int first_literal_address = 0;
    int last_literal_address = 0;

    if (!vm) return;
    if (!data || size < 64 || (size & 1) != 0) return;

    first_opcode = nexus_read_u16_be(data);
    for (i = 0; i + 1 < size; i += 2) {
        int word = nexus_read_u16_be(data + i);
        checksum = (checksum + word) & 0xffff;
        if (word == 0x000b) {
            rts_count++;
        }
        if ((word & 0xf000) == 0xa000 || (word & 0xf000) == 0xb000) {
            branch_count++;
        }
        if ((word & 0xf000) == 0xe000) {
            immediate_count++;
        }
        if ((word & 0xf0ff) == 0x400b) {
            jsr_count++;
        }
        if ((word & 0xf000) == 0x9000 || (word & 0xf000) == 0xd000) {
            pc_relative_load_count++;
        }
        if (i + 3 < size) {
            int ptr = nexus_read_u32_be(data + i);
            if ((ptr & 0xffff0000) == 0x00200000) {
                if (literal_pointer_count == 0) {
                    first_literal_offset = i;
                    first_literal_address = ptr;
                }
                last_literal_address = ptr;
                literal_pointer_count++;
            }
        }
    }

    /* Local verified SLEV00-15 assets are SH-2 task-like code streams:
     * first opcode 0x2fe6, even 16-bit words, one or more RTS opcodes.
     * This is a real-format profile only; it intentionally does not enable
     * condition/action dispatch until the call table and operands are decoded. */
    if (first_opcode == 0x2fe6 && rts_count > 0) {
        vm->real_task_profile_supported = 1;
        vm->real_task_word_count = size / 2;
        vm->real_task_first_opcode = first_opcode;
        vm->real_task_rts_count = rts_count;
        vm->real_task_branch_count = branch_count;
        vm->real_task_immediate_count = immediate_count;
        vm->real_task_jsr_count = jsr_count;
        vm->real_task_pc_relative_load_count = pc_relative_load_count;
        vm->real_task_literal_pointer_count = literal_pointer_count;
        vm->real_task_first_literal_offset = first_literal_offset;
        vm->real_task_first_literal_address = first_literal_address;
        vm->real_task_last_literal_address = last_literal_address;
        vm->real_task_checksum16 = checksum;
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Init
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_script_vm_init(Nexus_ScriptVM *vm) {
    if (!vm) return;
    memset(vm, 0, sizeof(*vm));
    vm->initialized = 1;
    vm->current_level = -1;
}

/* ═══════════════════════════════════════════════════════════════════
 * Load candidate trigger data for a level.
 * Real SLEV ownership remains unresolved, so dispatch is enabled only for
 * the explicit SLEV rule-table envelope documented in the public header.
 * Source: docs/nexus_triggers.md — unresolved trigger owner.
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_script_vm_load_level(Nexus_ScriptVM *vm, int level_index,
                                const uint8_t *data, int size) {
    if (!vm || !vm->initialized) return -1;
    if (level_index < 0 || level_index > 15) return -1;

    /* Unload any previous level scripts */
    nexus_script_vm_unload(vm);

    vm->current_level = level_index;
    vm->candidate_source_loaded = (data && size > 0) ? 1 : 0;
    vm->candidate_source_bytes = vm->candidate_source_loaded ? size : 0;
    vm->parser_supported = 0;
    vm->dispatch_enabled = 0;
    vm->parsed_record_size = 0;
    vm->parsed_rule_count = 0;
    vm->real_task_profile_supported = 0;
    vm->real_task_word_count = 0;
    vm->real_task_first_opcode = 0;
    vm->real_task_rts_count = 0;
    vm->real_task_branch_count = 0;
    vm->real_task_immediate_count = 0;
    vm->real_task_jsr_count = 0;
    vm->real_task_pc_relative_load_count = 0;
    vm->real_task_literal_pointer_count = 0;
    vm->real_task_first_literal_offset = -1;
    vm->real_task_first_literal_address = 0;
    vm->real_task_last_literal_address = 0;
    vm->real_task_checksum16 = 0;

    if (vm->candidate_source_loaded) {
        if (!nexus_script_parse_slev_rule_table(vm, data, size)) {
            nexus_script_profile_real_slev_task(vm, data, size);
        }
    }

    /* No synthetic fallback rules here: SLEV*.BIN/DGN trigger bytes that do
     * not match the bounded rule-table envelope are routed into a receipt.
     *
     * Evidence so far:
     * - docs/nexus_triggers.md: SDDRVS.TSK is the 26,610-byte sound driver.
     * - Per-level SLEV*.BIN files (2-12 KB) remain plausible event/script data. */

    printf("Nexus script VM: level %d source=%d bytes=%d parser=%s rules=%d\n",
        level_index, vm->candidate_source_loaded, vm->candidate_source_bytes,
        vm->parser_supported ? "parsed" : "pending", vm->rule_count);
    return 0;
}

void nexus_script_vm_unload(Nexus_ScriptVM *vm) {
    if (!vm) return;
    vm->rule_count = 0;
    vm->current_level = -1;
    vm->candidate_source_loaded = 0;
    vm->candidate_source_bytes = 0;
    vm->parser_supported = 0;
    vm->dispatch_enabled = 0;
    vm->parsed_record_size = 0;
    vm->parsed_rule_count = 0;
    vm->real_task_profile_supported = 0;
    vm->real_task_word_count = 0;
    vm->real_task_first_opcode = 0;
    vm->real_task_rts_count = 0;
    vm->real_task_branch_count = 0;
    vm->real_task_immediate_count = 0;
    vm->real_task_jsr_count = 0;
    vm->real_task_pc_relative_load_count = 0;
    vm->real_task_literal_pointer_count = 0;
    vm->real_task_first_literal_offset = -1;
    vm->real_task_first_literal_address = 0;
    vm->real_task_last_literal_address = 0;
    vm->real_task_checksum16 = 0;
}

int nexus_script_vm_runtime_receipt(const Nexus_ScriptVM *vm,
                                    Nexus_ScriptRuntimeReceipt *out_receipt) {
    if (!out_receipt) return -1;
    memset(out_receipt, 0, sizeof(*out_receipt));
    out_receipt->status = NEXUS_SCRIPT_RUNTIME_MISSING;
    out_receipt->level_index = -1;
    out_receipt->fallback_visuals_permitted = 0;
    if (!vm || !vm->initialized) return 0;

    out_receipt->level_index = vm->current_level;
    out_receipt->candidate_source_loaded = vm->candidate_source_loaded;
    out_receipt->candidate_source_bytes = vm->candidate_source_bytes;
    out_receipt->parser_supported = vm->parser_supported;
    out_receipt->dispatch_enabled = vm->dispatch_enabled;
    out_receipt->parsed_record_size = vm->parsed_record_size;
    out_receipt->parsed_rule_count = vm->parsed_rule_count;
    out_receipt->real_task_profile_supported =
        vm->real_task_profile_supported;
    out_receipt->real_task_word_count = vm->real_task_word_count;
    out_receipt->real_task_first_opcode = vm->real_task_first_opcode;
    out_receipt->real_task_rts_count = vm->real_task_rts_count;
    out_receipt->real_task_branch_count = vm->real_task_branch_count;
    out_receipt->real_task_immediate_count = vm->real_task_immediate_count;
    out_receipt->real_task_jsr_count = vm->real_task_jsr_count;
    out_receipt->real_task_pc_relative_load_count =
        vm->real_task_pc_relative_load_count;
    out_receipt->real_task_literal_pointer_count =
        vm->real_task_literal_pointer_count;
    out_receipt->real_task_first_literal_offset =
        vm->real_task_first_literal_offset;
    out_receipt->real_task_first_literal_address =
        vm->real_task_first_literal_address;
    out_receipt->real_task_last_literal_address =
        vm->real_task_last_literal_address;
    out_receipt->real_task_checksum16 = vm->real_task_checksum16;
    out_receipt->rules_loaded = vm->rule_count;

    if (!vm->candidate_source_loaded) {
        out_receipt->status = NEXUS_SCRIPT_RUNTIME_NO_SOURCE;
    } else if (!vm->parser_supported) {
        out_receipt->status =
            NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT;
        out_receipt->blocks_real_script_dispatch = 1;
    } else {
        out_receipt->status = NEXUS_SCRIPT_RUNTIME_READY_PARSED;
    }
    return 0;
}

const char *nexus_script_runtime_status_name(
    Nexus_ScriptRuntimeStatus status) {
    switch (status) {
    case NEXUS_SCRIPT_RUNTIME_MISSING: return "missing";
    case NEXUS_SCRIPT_RUNTIME_READY_PARSED: return "ready-parsed";
    case NEXUS_SCRIPT_RUNTIME_BLOCKED_UNSUPPORTED_FORMAT:
        return "blocked-unsupported-format";
    case NEXUS_SCRIPT_RUNTIME_NO_SOURCE: return "no-source";
    default: return "unknown";
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Condition evaluation helpers
 * ═══════════════════════════════════════════════════════════════════ */

static void nexus_dispatch_action(Nexus_ScriptVM *vm,
                                  const Nexus_ScriptAction *action) {
    if (!action) return;

    /* Call registered handler if any */
    if (vm && vm->handler) {
        vm->handler(action, vm->handler_data);
        return;
    }

    /* Default logging handlers */
    switch (action->opcode) {
    case NEXUS_OP_TELEPORT:
        printf("  [SCRIPT] TELEPORT to (%d,%d) level %d\n",
            action->x, action->y, action->level);
        break;
    case NEXUS_OP_SPAWN:
        printf("  [SCRIPT] SPAWN creature at (%d,%d) level %d\n",
            action->x, action->y, action->level);
        break;
    case NEXUS_OP_SET_SQUARE:
        printf("  [SCRIPT] SET_SQUARE (%d,%d) = %d\n",
            action->x, action->y, action->value);
        break;
    case NEXUS_OP_SOUND:
        printf("  [SCRIPT] PLAY_SOUND id=%d\n", action->value);
        break;
    case NEXUS_OP_TRIGGER_DOOR:
        printf("  [SCRIPT] TRIGGER_DOOR (%d,%d) value=%d\n",
            action->x, action->y, action->value);
        break;
    case NEXUS_OP_GIVE_ITEM:
        printf("  [SCRIPT] GIVE_ITEM id=%d to party\n", action->item_id);
        break;
    case NEXUS_OP_AWARD_XP:
        printf("  [SCRIPT] AWARD_XP %d to party\n", action->value);
        break;
    case NEXUS_OP_DISPLAY_MESSAGE:
        printf("  [SCRIPT] DISPLAY_MESSAGE id=%d\n", action->message_id);
        break;
    case NEXUS_OP_SET_FLAG:
        printf("  [SCRIPT] SET_FLAG %d = %d\n", action->flag_index, action->value);
        break;
    case NEXUS_OP_END_GAME:
        printf("  [SCRIPT] END_GAME triggered\n");
        break;
    default:
        printf("  [SCRIPT] Unknown action opcode 0x%02x\n", action->opcode);
        break;
    }
}

/* ═══════════════════════════════════════════════════════════════════
 * Rule evaluation and firing
 * ═══════════════════════════════════════════════════════════════════ */

static int nexus_condition_matches_party_xy(const Nexus_ScriptCondition *cond,
                                            int party_x, int party_y,
                                            int level) {
    return cond &&
           cond->opcode == NEXUS_OP_WHEN_PARTY_ON_XY &&
           cond->x == party_x &&
           cond->y == party_y &&
           cond->value == level;
}

static int nexus_condition_matches_value(const Nexus_ScriptCondition *cond,
                                         Nexus_WorldOpcode opcode,
                                         int value) {
    return cond && cond->opcode == opcode && cond->value == value;
}

static int nexus_condition_matches_door_open(const Nexus_ScriptCondition *cond,
                                             int x, int y, int is_open) {
    return cond &&
           cond->opcode == NEXUS_OP_WHEN_DOOR_OPEN &&
           is_open &&
           cond->x == x &&
           cond->y == y;
}

static int nexus_fire_rule_action(Nexus_ScriptVM *vm, Nexus_ScriptRule *r) {
    if (!vm || !r) return 0;
    if (!r->enabled) return 0;
    if (r->once_only && r->fired_count > 0) return 0;

    nexus_dispatch_action(vm, &r->action);
    r->fired_count++;
    return 1;
}

static int nexus_fire_rule_if(Nexus_ScriptVM *vm, int idx, int cond_true) {
    Nexus_ScriptRule *r;

    if (!vm || idx < 0 || idx >= vm->rule_count) return 0;
    r = &vm->rules[idx];

    if (!cond_true) return 0;
    return nexus_fire_rule_action(vm, r);
}

/* ═══════════════════════════════════════════════════════════════════
 * Trigger entry points (called from game logic)
 * ═══════════════════════════════════════════════════════════════════ */

/* Party moved to (x,y) — evaluate ON_XY rules */
void nexus_script_on_party_move(Nexus_ScriptVM *vm, int x, int y, int level) {
    int i, fired = 0;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_PARTY_ON_XY)
            fired += nexus_fire_rule_if(vm, i,
                nexus_condition_matches_party_xy(&vm->rules[i].cond,
                                                 x, y, level));
    }
    (void)fired;
}

/* Champion picked up item — evaluate HAS_ITEM rules */
void nexus_script_on_champion_item(Nexus_ScriptVM *vm, int champ_idx, int item_id) {
    int i;
    (void)champ_idx;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_CHAMPION_HAS)
            (void)nexus_fire_rule_if(vm, i,
                nexus_condition_matches_value(&vm->rules[i].cond,
                    NEXUS_OP_WHEN_CHAMPION_HAS, item_id));
    }
}

/* Creature died — evaluate CREATURE_DEAD rules */
void nexus_script_on_creature_dead(Nexus_ScriptVM *vm, int creature_type) {
    int i;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_CREATURE_DEAD)
            (void)nexus_fire_rule_if(vm, i,
                nexus_condition_matches_value(&vm->rules[i].cond,
                    NEXUS_OP_WHEN_CREATURE_DEAD, creature_type));
    }
}

/* Door state changed */
void nexus_script_on_door_change(Nexus_ScriptVM *vm, int x, int y, int is_open) {
    int i;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_DOOR_OPEN)
            (void)nexus_fire_rule_if(vm, i,
                nexus_condition_matches_door_open(&vm->rules[i].cond,
                                                  x, y, is_open));
    }
}

/* Item consumed */
void nexus_script_on_item_used(Nexus_ScriptVM *vm, int item_id) {
    int i;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_ITEM_USED)
            (void)nexus_fire_rule_if(vm, i,
                nexus_condition_matches_value(&vm->rules[i].cond,
                    NEXUS_OP_WHEN_ITEM_USED, item_id));
    }
}

/* Level loaded — evaluate LEVEL_LOADED rules */
void nexus_script_on_level_load(Nexus_ScriptVM *vm, int level_index) {
    int i, fired = 0;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_LEVEL_LOADED)
            fired += nexus_fire_rule_if(vm, i,
                nexus_condition_matches_value(&vm->rules[i].cond,
                    NEXUS_OP_WHEN_LEVEL_LOADED, level_index));
    }
    (void)fired;
}

/* ═══════════════════════════════════════════════════════════════════
 * Handler registration
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_script_vm_set_handler(Nexus_ScriptVM *vm,
                                   Nexus_ScriptActionHandler handler,
                                   void *user_data) {
    if (!vm) return;
    vm->handler = handler;
    vm->handler_data = user_data;
}

int nexus_script_vm_fire_rule(Nexus_ScriptVM *vm, int rule_id) {
    int i;
    if (!vm || !vm->initialized) return 0;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].rule_id == rule_id)
            return nexus_fire_rule_action(vm, &vm->rules[i]);
    }
    return 0;
}

void nexus_script_vm_dump(const Nexus_ScriptVM *vm) {
    int i;
    if (!vm) return;
    printf("Script VM: %d rules (level %d)\n",
        vm->rule_count, vm->current_level);
    for (i = 0; i < vm->rule_count; i++) {
        const Nexus_ScriptRule *r = &vm->rules[i];
        printf("  Rule %d: cond=0x%02x action=0x%02x enabled=%d fired=%d once=%d\n",
            r->rule_id, r->cond.opcode, r->action.opcode,
            r->enabled, r->fired_count, r->once_only);
    }
}
