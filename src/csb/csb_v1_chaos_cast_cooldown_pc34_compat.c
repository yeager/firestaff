#include "csb_v1_chaos_cast_cooldown_pc34_compat.h"

#include <string.h>

/* CSB V1 Chaos cast/cooldown runtime slice.
 *
 * Source-locked to the CSB lineage path:
 *   CSBWin/Chaos.cpp:60-69 _CALL0-_CALL9 dispatch frame declarations.
 *   CSBWin/Chaos.cpp:584-588 InitializeE clears engine state before CSB.
 *   CSBWin/DSA.cpp:465-531 QueueDSASwitchAction queues DSA timer actions.
 *   CSBWin/DSA.cpp:764-808 EX_GOSUB enters a nested DSA call frame.
 *   CSBWin/DSA.cpp:5053-5120 Execute fetches bytecode and dispatches ops.
 *   CSBWin/DSA.cpp:5288-5294 Execute returns when states terminate.
 *   CSBWin/DSA.cpp:5329-5441 ProcessDSATimer6 wraps active interpreter runs.
 *   CSBWin/CSBCode.cpp:11414 StartChaos enters the CSB utility/game path.
 *   ReDMCSB COMMAND.C:2302-2306 accepts a spell-area cast command only after
 *     the command gate has admitted input for the active magic caster.
 *   ReDMCSB GAMELOOP.C:150-155 decrements disabled-action tick counters.
 *   ReDMCSB MENU.C:1633-1663 clears a completed spell-entry line.
 *   ReDMCSB MENU.C:2036-2039 installs the post-cast disabled-action ticks.
 */

static int csb_v1_chaos_cast_status(
    const CSB_V1_ChaosCastCooldownState *state)
{
    if (!state || !state->chaos) {
        return CSB_V1_CHAOS_CAST_INVALID;
    }
    if (state->active_script_id >= 0) {
        return CSB_V1_CHAOS_CAST_RUNNING;
    }
    if (state->cooldown_ticks > 0) {
        return CSB_V1_CHAOS_CAST_COOLDOWN;
    }
    return CSB_V1_CHAOS_CAST_READY;
}

static int csb_v1_chaos_cast_execute_step(
    CSB_V1_ChaosCastCooldownState *cast_state,
    CSB_V1_DSAScript *script)
{
    uint16_t op;

    if (!cast_state || !cast_state->chaos || !script || !script->active) {
        return 0;
    }
    if (!script->bytecode) {
        script->active = 0;
        return 0;
    }
    if (script->delay_ticks > 0) {
        script->delay_ticks--;
        return script->active;
    }
    if (script->pc < 0 || script->pc >= script->bytecode_len) {
        script->active = 0;
        return 0;
    }

    op = script->bytecode[script->pc++];
    switch (op & 0x00ffu) {
        case CSB_DSA_OP_NOP:
            break;
        case CSB_DSA_OP_SET:
            if (script->pc < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256) {
                    cast_state->chaos->flags[flag] = 1;
                }
            }
            break;
        case CSB_DSA_OP_CLEAR:
            if (script->pc < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256) {
                    cast_state->chaos->flags[flag] = 0;
                }
            }
            break;
        case CSB_DSA_OP_TOGGLE:
            if (script->pc < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256) {
                    cast_state->chaos->flags[flag] ^= 1;
                }
            }
            break;
        case CSB_DSA_OP_TEST:
            if (script->pc + 1 < script->bytecode_len) {
                int flag = script->bytecode[script->pc++];
                int target = script->bytecode[script->pc++];
                if (flag >= 0 && flag < 256 &&
                    cast_state->chaos->flags[flag] &&
                    target >= 0 && target < script->bytecode_len) {
                    script->pc = target;
                }
            } else {
                script->active = 0;
            }
            break;
        case CSB_DSA_OP_CALL:
            if (script->pc < script->bytecode_len &&
                script->sp < CSB_V1_DSA_STACK_SIZE) {
                int target = script->bytecode[script->pc++];
                if (target >= 0 && target < script->bytecode_len) {
                    script->stack[script->sp++] = script->pc;
                    script->pc = target;
                    cast_state->dsa_calls_executed++;
                    cast_state->dsa_call_depth = script->sp;
                } else {
                    script->active = 0;
                }
            } else {
                script->active = 0;
            }
            break;
        case CSB_DSA_OP_RETURN:
            if (script->sp > 0) {
                script->pc = script->stack[--script->sp];
                cast_state->dsa_call_depth = script->sp;
            } else {
                script->active = 0;
            }
            break;
        case CSB_DSA_OP_DELAY:
            if (script->pc < script->bytecode_len) {
                script->delay_ticks = script->bytecode[script->pc++];
            } else {
                script->active = 0;
            }
            break;
        case CSB_DSA_OP_END:
            script->active = 0;
            break;
        default:
            if (script->pc < script->bytecode_len) {
                script->pc++;
            } else {
                script->active = 0;
            }
            break;
    }

    return script->active;
}

void csb_v1_chaos_cast_cooldown_init(CSB_V1_ChaosCastCooldownState *state,
    CSB_V1_ChaosMagicState *chaos, int cooldown_ticks)
{
    if (!state) {
        return;
    }
    memset(state, 0, sizeof(*state));
    state->chaos = chaos;
    state->active_script_id = -1;
    state->default_cooldown_ticks = cooldown_ticks > 0 ? cooldown_ticks : 1;
}

int csb_v1_chaos_cast_cooldown_begin(CSB_V1_ChaosCastCooldownState *state,
    int script_id)
{
    int status;

    status = csb_v1_chaos_cast_status(state);
    if (status == CSB_V1_CHAOS_CAST_INVALID) {
        return status;
    }
    if (status != CSB_V1_CHAOS_CAST_READY) {
        return CSB_V1_CHAOS_CAST_BUSY;
    }
    if (csb_v1_chaos_trigger(state->chaos, script_id) != 0) {
        return CSB_V1_CHAOS_CAST_INVALID;
    }

    state->active_script_id = script_id;
    state->cooldown_ticks = 0;
    state->dsa_call_depth = 0;
    state->casts_started++;
    return CSB_V1_CHAOS_CAST_RUNNING;
}

int csb_v1_chaos_cast_cooldown_tick(CSB_V1_ChaosCastCooldownState *state)
{
    if (!state || !state->chaos) {
        return CSB_V1_CHAOS_CAST_INVALID;
    }

    if (state->active_script_id >= 0) {
        CSB_V1_DSAScript *script;
        if (state->active_script_id >= state->chaos->script_count) {
            state->active_script_id = -1;
            return CSB_V1_CHAOS_CAST_INVALID;
        }
        script = &state->chaos->scripts[state->active_script_id];
        csb_v1_chaos_cast_execute_step(state, script);
        if (!script->active) {
            state->active_script_id = -1;
            state->cooldown_ticks = state->default_cooldown_ticks;
            state->casts_completed++;
            return CSB_V1_CHAOS_CAST_COOLDOWN;
        }
        return CSB_V1_CHAOS_CAST_RUNNING;
    }

    if (state->cooldown_ticks > 0) {
        state->cooldown_ticks--;
    }
    return csb_v1_chaos_cast_status(state);
}

void csb_v1_chaos_cast_cooldown_cancel(CSB_V1_ChaosCastCooldownState *state)
{
    if (!state) {
        return;
    }
    if (state->chaos && state->active_script_id >= 0 &&
        state->active_script_id < state->chaos->script_count) {
        CSB_V1_DSAScript *script = &state->chaos->scripts[state->active_script_id];
        script->active = 0;
        script->pc = 0;
        script->sp = 0;
        script->delay_ticks = 0;
    }
    state->active_script_id = -1;
    state->cooldown_ticks = 0;
    state->dsa_call_depth = 0;
    state->casts_canceled++;
}
