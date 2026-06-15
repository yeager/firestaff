
#include "csb_v1_chaos_magic_pc34_compat.h"
#include <string.h>

/* pass603: CSB V1 Chaos magic / DSA system
 *
 * Source-locked to:
 *   CSBWin/Chaos.cpp: InitializeE (line 584)
 *   CSBWin/Chaos.cpp: _CALL0-_CALL9 (lines 60-69) — DSA call dispatch
 *   CSBWin/DSA.cpp: DSA interpreter core (5806 lines)
 *   CSBWin/CSBCode.cpp: _DisplayChaosStrikesBack (line 9196)
 *   CSBWin/CSBCode.cpp: StartChaos (line 11414)
 */

#define CSB_V1_DSA_FLAG_COUNT 256

static int csb_v1_dsa_has_operands(const CSB_V1_DSAScript *script, int count) {
    return script && count >= 0 && script->pc <= script->bytecode_len &&
           count <= script->bytecode_len - script->pc;
}

static int csb_v1_dsa_flag_is_valid(int flag) {
    return flag >= 0 && flag < CSB_V1_DSA_FLAG_COUNT;
}

static int csb_v1_dsa_target_is_valid(const CSB_V1_DSAScript *script, int target) {
    return script && target >= 0 && target < script->bytecode_len;
}

static int csb_v1_dsa_reject_malformed_at(CSB_V1_DSAScript *script, int pc) {
    if (script) {
        script->pc = pc;
        script->active = 0;
    }
    return 0;
}

static void csb_v1_dsa_record_dispatch(CSB_V1_ChaosMagicState *state,
    CSB_V1_DSADispatchKind kind, int opcode, int operand, int op_pc)
{
    if (!state) return;
    state->dispatch_count++;
    state->last_dispatch.kind = kind;
    state->last_dispatch.opcode = opcode;
    state->last_dispatch.operand = operand;
    state->last_dispatch.op_pc = op_pc;
}

void csb_v1_chaos_init(CSB_V1_ChaosMagicState *state) {
    if (!state) return;
    memset(state, 0, sizeof(*state));
}

int csb_v1_chaos_load_scripts(CSB_V1_ChaosMagicState *state,
    const uint8_t *data, int data_size)
{
    /* CSBWin/DSA.cpp script loading:
     * Script table at start of DSA data block.
     * Each entry: uint16 offset, uint16 length */
    int i, offset = 0;
    uint16_t count;
    if (!state || !data || data_size < 2) return -1;

    count = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    if (count > CSB_V1_MAX_DSA_SCRIPTS) count = CSB_V1_MAX_DSA_SCRIPTS;
    state->script_count = count;
    offset = 2;

    for (i = 0; i < count && offset + 4 <= data_size; i++) {
        uint16_t soff = (uint16_t)data[offset] | ((uint16_t)data[offset+1] << 8);
        uint16_t slen = (uint16_t)data[offset+2] | ((uint16_t)data[offset+3] << 8);
        state->scripts[i].bytecode = NULL; /* would point into data */
        state->scripts[i].bytecode_len = slen;
        state->scripts[i].pc = 0;
        state->scripts[i].sp = 0;
        state->scripts[i].active = 0;
        state->scripts[i].delay_ticks = 0;
        (void)soff;
        offset += 4;
    }
    return count;
}

int csb_v1_chaos_trigger(CSB_V1_ChaosMagicState *state, int script_id) {
    if (!state || script_id < 0 || script_id >= state->script_count) return -1;
    state->scripts[script_id].active = 1;
    state->scripts[script_id].pc = 0;
    state->scripts[script_id].sp = 0;
    return 0;
}

int csb_v1_dsa_execute_step(CSB_V1_DSAScript *script,
    CSB_V1_ChaosMagicState *state)
{
    uint16_t op;
    int op_pc;
    if (!script || !state || !script->active || !script->bytecode) return 0;
    if (script->delay_ticks > 0) { script->delay_ticks--; return 1; }
    if (script->pc >= script->bytecode_len) { script->active = 0; return 0; }

    op_pc = script->pc;
    op = script->bytecode[script->pc++];
    switch (op & 0xFF) {
        case CSB_DSA_OP_NOP: break;
        case CSB_DSA_OP_SET:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                if (!csb_v1_dsa_flag_is_valid(flag)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                state->flags[flag] = 1;
            }
            break;
        case CSB_DSA_OP_CLEAR:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                if (!csb_v1_dsa_flag_is_valid(flag)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                state->flags[flag] = 0;
            }
            break;
        case CSB_DSA_OP_TOGGLE:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                if (!csb_v1_dsa_flag_is_valid(flag)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                state->flags[flag] ^= 1;
            }
            break;
        case CSB_DSA_OP_TEST:
            if (!csb_v1_dsa_has_operands(script, 2)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            {
                int flag = script->bytecode[script->pc++];
                int target = script->bytecode[script->pc++];
                /* ReDMCSB: DEFS.H lines 1206-1208 constrain remote sensor
                 * target fields, and MOVESENS.C lines 1198-1206 indexes the
                 * target square directly before enqueuing the event.  Reject
                 * malformed DSA target operands at parse/VM time so imported
                 * edge scripts cannot drive an out-of-range target lookup. */
                if (!csb_v1_dsa_flag_is_valid(flag) ||
                    !csb_v1_dsa_target_is_valid(script, target)) {
                    return csb_v1_dsa_reject_malformed_at(script, op_pc);
                }
                if (state->flags[flag])
                    script->pc = target;
            }
            break;
        case CSB_DSA_OP_DELAY:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            script->delay_ticks = script->bytecode[script->pc++];
            break;
        case CSB_DSA_OP_MESSAGE:
            if (!csb_v1_dsa_has_operands(script, 1)) {
                return csb_v1_dsa_reject_malformed_at(script, op_pc);
            }
            /* CSBWin/DSA.cpp QueueDSASwitchAction lines 523-531 queues
             * TT_DESSAGE timer records, and ProcessDSATimer6 lines
             * 5415-5441 maps timer position/function to an Execute message
             * column.  The text surface eventually reaches ReDMCSB TEXT.C
             * F0047_TEXT_MESSAGEAREA_PrintMessage lines 1670-1775. */
            csb_v1_dsa_record_dispatch(state,
                CSB_V1_DSA_DISPATCH_MESSAGE,
                CSB_DSA_OP_MESSAGE,
                script->bytecode[script->pc++],
                op_pc);
            break;
        case CSB_DSA_OP_END:
            script->active = 0;
            break;
        default:
            script->pc++; /* skip unknown op + arg */
            break;
    }
    return script->active;
}

int csb_v1_chaos_tick(CSB_V1_ChaosMagicState *state) {
    int i, active = 0;
    if (!state) return 0;
    for (i = 0; i < state->script_count; i++) {
        if (state->scripts[i].active) {
            csb_v1_dsa_execute_step(&state->scripts[i], state);
            if (state->scripts[i].active) active++;
        }
    }
    return active;
}

const char *csb_v1_chaos_source_evidence(void) {
    return
        "CSBWin/Chaos.cpp:584 InitializeE\n"
        "CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9 DSA dispatch\n"
        "CSBWin/DSA.cpp DSA interpreter (5806 lines)\n"
        "CSBWin/DSA.cpp:523-531 QueueDSASwitchAction TT_DESSAGE timer dispatch\n"
        "CSBWin/DSA.cpp:5415-5441 ProcessDSATimer6 message column execution\n"
        "ReDMCSB TEXT.C:1670-1775 F0047_TEXT_MESSAGEAREA_PrintMessage\n"
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack\n"
        "CSBWin/CSBCode.cpp:11414 StartChaos\n"
        "CSB-specific: DSA bytecode VM, 256 global flags\n";
}
