
#ifndef FIRESTAFF_CSB_V1_CHAOS_MAGIC_PC34_COMPAT_H
#define FIRESTAFF_CSB_V1_CHAOS_MAGIC_PC34_COMPAT_H

#include <stdint.h>

/* CSB V1 Chaos Magic System
 *
 * CSB extends DM1's spell system with the Chaos magic:
 * - DSA (Dungeon Scripting Architecture) scripts triggered by spells
 * - Programmable spell effects per dungeon level
 * - Custom creature behaviors tied to Chaos scripts
 *
 * Source: CSBWin/Chaos.cpp (5336 lines)
 * Source: CSBWin/DSA.cpp (5806 lines)
 */

#define CSB_V1_MAX_DSA_SCRIPTS 256
#define CSB_V1_DSA_STACK_SIZE 64

typedef enum {
    CSB_DSA_OP_NOP = 0,
    CSB_DSA_OP_SET,
    CSB_DSA_OP_CLEAR,
    CSB_DSA_OP_TOGGLE,
    CSB_DSA_OP_TEST,
    CSB_DSA_OP_JUMP,
    CSB_DSA_OP_CALL,
    CSB_DSA_OP_RETURN,
    CSB_DSA_OP_DELAY,
    CSB_DSA_OP_SOUND,
    CSB_DSA_OP_SPAWN,
    CSB_DSA_OP_MOVE,
    CSB_DSA_OP_DAMAGE,
    CSB_DSA_OP_TELEPORT,
    CSB_DSA_OP_MESSAGE,
    CSB_DSA_OP_END,
    CSB_DSA_OP_COUNT
} CSB_DSA_Opcode;

typedef struct {
    uint16_t *bytecode;
    int bytecode_len;
    int pc;  /* program counter */
    int stack[CSB_V1_DSA_STACK_SIZE];
    int sp;  /* stack pointer */
    int active;
    int delay_ticks;
} CSB_V1_DSAScript;

typedef struct {
    CSB_V1_DSAScript scripts[CSB_V1_MAX_DSA_SCRIPTS];
    int script_count;
    int flags[256]; /* global DSA flags */
} CSB_V1_ChaosMagicState;

void csb_v1_chaos_init(CSB_V1_ChaosMagicState *state);
int csb_v1_chaos_load_scripts(CSB_V1_ChaosMagicState *state,
    const uint8_t *data, int data_size);
int csb_v1_chaos_trigger(CSB_V1_ChaosMagicState *state, int script_id);
int csb_v1_chaos_tick(CSB_V1_ChaosMagicState *state);
int csb_v1_dsa_execute_step(CSB_V1_DSAScript *script,
    CSB_V1_ChaosMagicState *state);
const char *csb_v1_chaos_source_evidence(void);

#endif

