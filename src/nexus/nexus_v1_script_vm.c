#include "nexus_v1_script_vm.h"
#include "nexus_v1_world.h"
#include <string.h>
#include <stdio.h>

/* Nexus V1 script VM — SDDRVS.TSK bytecode parser + trigger dispatcher.
 * STUB IMPLEMENTATION — actual SDDRVS.TSK bytecode format is unknown.
 * This module provides the hook scaffolding for when the format is reverse-
 * engineered. Current status: parsing stubs exist but no real bytecode decode.
 * Source: docs/nexus_triggers.md (SDDRVS.TSK analysis),
 * docs/nexus_sensors.md (Nexus trigger model vs DM1/DM2). */

/* Static action handler + user data */
static Nexus_ScriptActionHandler g_handler = NULL;
static void *g_handler_data = NULL;

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
 * Load SDDRVS.TSK for a level — STUB
 * TODO: parse actual SDDRVS.TSK bytecode.
 * Current approach: register a small number of default rules
 * based on level index. Real implementation needs format reverse-
 * engineering of the 5,448-byte SDDRVS.TSK file.
 * Source: docs/nexus_triggers.md — opcode hypothesis.
 * ═══════════════════════════════════════════════════════════════════ */

int nexus_script_vm_load_level(Nexus_ScriptVM *vm, int level_index,
                                const uint8_t *data, int size) {
    int i;

    if (!vm || !vm->initialized) return -1;
    if (level_index < 0 || level_index > 15) return -1;

    /* Unload any previous level scripts */
    nexus_script_vm_unload(vm);

    vm->current_level = level_index;

    /* STUB: no real parsing yet.
     * The SDDRVS.TSK format is unknown.
     * We register minimal placeholder rules that allow
     * the script VM interface to function.
     * TODO: actual bytecode parsing when SDDRVS.TSK format is reverse-engineered.
     *
     * Evidence so far:
     * - File is 5,448 bytes — too small for complex VM but right for simple rule engine
     * - 16 levels × ~340 bytes per level suggests ~10-20 rules per level
     * - 8-byte rule entries plausible (1-byte opcode + 3-byte pos + 4-byte action)
     * - Per-level SLEV*.BIN files (2-12 KB) likely contain additional event/script data */

    /* For now: register some default level-transition rules
     * (these would in reality come from the SDDRVS.TSK parse) */
    if (vm->rule_count < NEXUS_SCRIPT_MAX_RULES) {
        /* Level 0: party starts at entrance */
        /* Level 15: exit reached → end game */
        Nexus_ScriptRule *r = &vm->rules[vm->rule_count++];
        memset(r, 0, sizeof(*r));
        r->rule_id = vm->rule_count;
        r->enabled = 1;
        r->once_only = 0;
        /* Level 15 exit: NEXUS_OP_WHEN_PARTY_ON_XY at (15,15) → NEXUS_OP_END_GAME */
        r->cond.opcode = NEXUS_OP_WHEN_PARTY_ON_XY;
        r->cond.x = 15; r->cond.y = 15;
        r->cond.value = 15; /* level 15 */
        r->action.opcode = NEXUS_OP_END_GAME;
        r->action.x = 15; r->action.y = 15;
    }

    printf("Nexus script VM: loaded level %d (%d bytes, stub parsing)\n",
        level_index, size);
    (void)data;
    return 0;
}

void nexus_script_vm_unload(Nexus_ScriptVM *vm) {
    if (!vm) return;
    vm->rule_count = 0;
    vm->current_level = -1;
}

/* ═══════════════════════════════════════════════════════════════════
 * Condition evaluation helpers
 * ═══════════════════════════════════════════════════════════════════ */

static int nexus_eval_condition(const Nexus_ScriptCondition *cond, void *ctx) {
    (void)ctx;
    if (!cond) return 0;

    switch (cond->opcode) {
    case NEXUS_OP_WHEN_PARTY_ON_XY:
        /* ctx = party position { x, y, level } — checked by caller */
        return 1;
    case NEXUS_OP_WHEN_CHAMPION_HAS:
        return 1;
    case NEXUS_OP_WHEN_LEVEL_LOADED:
        return 1;
    case NEXUS_OP_WHEN_CREATURE_DEAD:
        return 1;
    case NEXUS_OP_WHEN_DOOR_OPEN:
        return 1;
    case NEXUS_OP_WHEN_ITEM_USED:
        return 1;
    default:
        return 0;
    }
}

static void nexus_dispatch_action(const Nexus_ScriptAction *action) {
    if (!action) return;

    /* Call registered handler if any */
    if (g_handler) {
        g_handler(action, g_handler_data);
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

static int nexus_eval_and_fire_rule(Nexus_ScriptVM *vm, int idx,
                                     int party_x, int party_y, int level) {
    Nexus_ScriptRule *r;
    int cond_true;

    if (!vm || idx < 0 || idx >= vm->rule_count) return 0;
    r = &vm->rules[idx];

    if (!r->enabled) return 0;
    if (r->once_only && r->fired_count > 0) return 0;

    /* Evaluate condition */
    cond_true = 0;
    switch (r->cond.opcode) {
    case NEXUS_OP_WHEN_PARTY_ON_XY:
        cond_true = (r->cond.x == party_x && r->cond.y == party_y &&
                     r->cond.value == level);
        break;
    case NEXUS_OP_WHEN_LEVEL_LOADED:
        cond_true = (r->cond.value == level);
        break;
    default:
        /* Generic: if handler registered, delegate */
        cond_true = nexus_eval_condition(&r->cond, NULL);
        break;
    }

    if (!cond_true) return 0;

    /* Fire action */
    nexus_dispatch_action(&r->action);
    r->fired_count++;
    return 1;
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
            fired += nexus_eval_and_fire_rule(vm, i, x, y, level);
    }
    (void)fired;
}

/* Champion picked up item — evaluate HAS_ITEM rules */
void nexus_script_on_champion_item(Nexus_ScriptVM *vm, int champ_idx, int item_id) {
    int i;
    (void)champ_idx; (void)item_id;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_CHAMPION_HAS)
            (void)nexus_eval_and_fire_rule(vm, i, 0, 0, -1);
    }
}

/* Creature died — evaluate CREATURE_DEAD rules */
void nexus_script_on_creature_dead(Nexus_ScriptVM *vm, int creature_type) {
    int i;
    (void)creature_type;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_CREATURE_DEAD)
            (void)nexus_eval_and_fire_rule(vm, i, 0, 0, -1);
    }
}

/* Door state changed */
void nexus_script_on_door_change(Nexus_ScriptVM *vm, int x, int y, int is_open) {
    int i;
    (void)x; (void)y; (void)is_open;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_DOOR_OPEN)
            (void)nexus_eval_and_fire_rule(vm, i, 0, 0, -1);
    }
}

/* Item consumed */
void nexus_script_on_item_used(Nexus_ScriptVM *vm, int item_id) {
    int i;
    (void)item_id;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_ITEM_USED)
            (void)nexus_eval_and_fire_rule(vm, i, 0, 0, -1);
    }
}

/* Level loaded — evaluate LEVEL_LOADED rules */
void nexus_script_on_level_load(Nexus_ScriptVM *vm, int level_index) {
    int i, fired = 0;
    if (!vm || !vm->initialized) return;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].cond.opcode == NEXUS_OP_WHEN_LEVEL_LOADED)
            fired += nexus_eval_and_fire_rule(vm, i, 0, 0, level_index);
    }
    (void)fired;
}

/* ═══════════════════════════════════════════════════════════════════
 * Handler registration
 * ═══════════════════════════════════════════════════════════════════ */

void nexus_script_vm_set_handler(Nexus_ScriptVM *vm,
                                   Nexus_ScriptActionHandler handler,
                                   void *user_data) {
    (void)vm;
    g_handler = handler;
    g_handler_data = user_data;
}

int nexus_script_vm_fire_rule(Nexus_ScriptVM *vm, int rule_id) {
    int i;
    if (!vm || !vm->initialized) return 0;
    for (i = 0; i < vm->rule_count; i++) {
        if (vm->rules[i].rule_id == rule_id)
            return nexus_eval_and_fire_rule(vm, i, 0, 0, -1);
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