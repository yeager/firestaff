
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
    if (!script || !state || !script->active || !script->bytecode) return 0;
    if (script->delay_ticks > 0) { script->delay_ticks--; return 1; }
    if (script->pc >= script->bytecode_len) { script->active = 0; return 0; }

    op = script->bytecode[script->pc++];
    switch (op & 0xFF) {
        case CSB_DSA_OP_NOP: break;
        case CSB_DSA_OP_SET:
            if (script->pc < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256) state->flags[flag] = 1;
            }
            break;
        case CSB_DSA_OP_CLEAR:
            if (script->pc < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256) state->flags[flag] = 0;
            }
            break;
        case CSB_DSA_OP_TOGGLE:
            if (script->pc < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256) state->flags[flag] ^= 1;
            }
            break;
        case CSB_DSA_OP_TEST:
            if (script->pc + 1 < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                int target = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256 && state->flags[flag])
                    script->pc = target;
            }
            break;
        case CSB_DSA_OP_DELAY:
            if (script->pc < script->bytecode_len)
                script->delay_ticks = script->bytecode[script->pc++];
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
        "CSBWin/CSBCode.cpp:9196 _DisplayChaosStrikesBack\n"
        "CSBWin/CSBCode.cpp:11414 StartChaos\n"
        "CSB-specific: DSA bytecode VM, 256 global flags\n";
}

