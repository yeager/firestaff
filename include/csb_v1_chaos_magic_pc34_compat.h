
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
#define CSB_V1_MAX_DSA_BYTECODE_BYTES 65536

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

typedef enum {
    CSB_V1_DSA_DISPATCH_NONE = 0,
    CSB_V1_DSA_DISPATCH_MESSAGE
} CSB_V1_DSADispatchKind;

typedef struct {
    CSB_V1_DSADispatchKind kind;
    int opcode;
    int operand;
    int op_pc;
} CSB_V1_DSADispatchRecord;

/* A CSBWin extended-save DSA action after ReadDSAs()/DSA::Read has
 * authenticated its containing section.  `program_words` is an owned copy
 * of the source little-endian DSAAction program; it is intentionally not
 * interpreted as this compatibility VM's private opcode stream. */
typedef struct {
    uint8_t dsa_id;
    uint32_t state_index;
    uint32_t column;
    uint16_t *program_words;
    int program_word_count;
} CSB_V1_DSAImportedAction;

typedef struct {
    CSB_V1_DSAScript scripts[CSB_V1_MAX_DSA_SCRIPTS];
    int script_count;
    int flags[256]; /* global DSA flags */
    int dispatch_count;
    CSB_V1_DSADispatchRecord last_dispatch;
    /* A loaded DSA block owns decoded little-endian words here.  Script
     * pointers remain valid after the caller releases the input buffer. */
    uint16_t *loaded_bytecode;
    int loaded_bytecode_words;
    uint32_t loaded_bytecode_magic;
    CSB_V1_DSAImportedAction *imported_actions;
    int imported_action_count;
} CSB_V1_ChaosMagicState;

void csb_v1_chaos_init(CSB_V1_ChaosMagicState *state);
void csb_v1_chaos_cleanup(CSB_V1_ChaosMagicState *state);
int csb_v1_chaos_load_scripts(CSB_V1_ChaosMagicState *state,
    const uint8_t *data, int data_size);
int csb_v1_chaos_trigger(CSB_V1_ChaosMagicState *state, int script_id);
int csb_v1_chaos_tick(CSB_V1_ChaosMagicState *state);
int csb_v1_dsa_execute_step(CSB_V1_DSAScript *script,
    CSB_V1_ChaosMagicState *state);

/* Imports only checksum-verified CSBWin Extended Features DSA records.
 * The imported program words remain opaque until a source-faithful CSBWin
 * ProcessDSAFilter interpreter consumes them.  The operation is transactional:
 * malformed, encrypted, truncated, or checksum-invalid data leaves state
 * unchanged.  Source: SaveGame.cpp ReadDSAs; DSA.cpp DSA::Read/DSAAction::Read. */
int csb_v1_chaos_import_extended_save_dsas(CSB_V1_ChaosMagicState *state,
    const uint8_t *bytes, int size);

const CSB_V1_DSAImportedAction *csb_v1_chaos_find_imported_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal);
const char *csb_v1_chaos_source_evidence(void);

#endif
