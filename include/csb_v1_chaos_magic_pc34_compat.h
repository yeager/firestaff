
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

/* CSBWin Data.h:1686-1708, 1947-1984.  These are source DSA word-code
 * values, not the older Firestaff compatibility-bytecode values below. */
#define CSB_V1_CSBWIN_DSACMD_LOAD 6u
#define CSB_V1_CSBWIN_DSACMD_GOSUB 5u
#define CSB_V1_CSBWIN_DSACMD_AMPERSAND 11u
#define CSB_V1_CSBWIN_DSACMD_JUMP 12u
#define CSB_V1_CSBWIN_DSACMD_AMPERSAND2 21u
#define CSB_V1_CSBWIN_DSACMD_STORE 13u
#define CSB_V1_CSBWIN_DSACMD_VARIABLEFETCH 17u
#define CSB_V1_CSBWIN_DSACMD_VARIABLESTORE 18u
#define CSB_V1_CSBWIN_DSACMD_GLOBALFETCH 19u
#define CSB_V1_CSBWIN_DSACMD_GLOBALSTORE 20u
#define CSB_V1_CSBWIN_DSA_LOAD_INTEGER 26u
#define CSB_V1_CSBWIN_DSA_LOAD_ABS 27u
#define CSB_V1_CSBWIN_DSA_LOAD_DOLLAR 28u
#define CSB_V1_CSBWIN_DSA_LOAD_ABS32 29u
#define CSB_V1_CSBWIN_DSA_LOAD_INTEGER32 30u

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

typedef enum {
    CSB_V1_CSBWIN_DSA_LOAD_OK = 0,
    CSB_V1_CSBWIN_DSA_LOAD_NOT_LOAD = 1,
    CSB_V1_CSBWIN_DSA_LOAD_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_LOAD_SOURCE_ILLEGAL = -2
} CSB_V1_CSBWinDSALoadResult;

typedef enum {
    CSB_V1_CSBWIN_DSA_LOAD_STORE_OK = 0,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_UNSUPPORTED = 2,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_LOAD_STORE_SOURCE_ILLEGAL = -2
} CSB_V1_CSBWinDSALoadStoreResult;

typedef enum {
    CSB_V1_CSBWIN_DSA_STACK_OK = 0,
    CSB_V1_CSBWIN_DSA_STACK_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_STACK_UNSUPPORTED = 2,
    CSB_V1_CSBWIN_DSA_STACK_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_STACK_SOURCE_ILLEGAL = -2
} CSB_V1_CSBWinDSAStackResult;

/* The master location is packed exactly as CSBWin LOCATIONREL::Integer():
 * position bits 16-17, level 10-15, X 5-9, Y 0-4.  `parameters` models the
 * ordered A..Z type-47 actuator chain used by EX_LOAD; unavailable entries
 * produce the source's zero value. */
typedef struct {
    uint32_t master_location;
    const uint32_t *parameters;
    int parameter_count;
} CSB_V1_CSBWinDSALoadContext;

typedef struct {
    uint32_t value;
    int next_state;
    uint16_t words_consumed;
    uint8_t selector;
} CSB_V1_CSBWinDSALoadExecution;

/* Mutable A..Z parameter surface used by DSA.cpp EX_LOAD and EX_STORE. It is
 * accepted only through the runtime-owned authenticated lookup below. */
typedef struct {
    uint32_t master_location;
    uint32_t *parameters;
    int parameter_count;
} CSB_V1_CSBWinDSALoadStoreContext;

typedef struct {
    uint32_t value;
    int load_next_state;
    int next_state;
    uint16_t words_consumed;
    uint8_t load_selector;
    uint8_t store_selector;
} CSB_V1_CSBWinDSALoadStoreExecution;

/* The source STACK has 100 signed 32-bit cells (CSBWin DSA.cpp:98-426).
 * This boundary implements only the pure stack/arithmetic/control STKOP
 * subset of EX_AMPERSAND; world, timer, variable, and filter operations are
 * deliberately not promoted into this authenticated action surface. */
#define CSB_V1_CSBWIN_DSA_STACK_CAPACITY 100
#define CSB_V1_CSBWIN_DSA_GLOBAL_CAPACITY 100

typedef struct {
    uint32_t master_location;
    uint32_t *parameters;
    int parameter_count;
    /* CSBWin DSA.cpp EX_GLOBALFETCH/EX_GLOBALSTORE address the source
     * numGlobalVariables/globalVariables bank. The caller owns this narrow
     * runtime surface; the executor stages it and publishes it only after a
     * fully consumed authenticated action succeeds. */
    uint32_t *global_variables;
    int global_variable_count;
} CSB_V1_CSBWinDSAStackContext;

typedef struct {
    int next_state;
    int forced_state;
    uint16_t words_consumed;
    uint16_t command_count;
    uint16_t stack_depth;
} CSB_V1_CSBWinDSAStackExecution;

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

/* CSBWin DSAState::Program searches its action table in file order and uses
 * the first action whose column equals the requested message column. */
const CSB_V1_DSAImportedAction *csb_v1_chaos_find_imported_action_column(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column);

typedef enum {
    CSB_V1_CSBWIN_DSA_JUMP_OK = 0,
    CSB_V1_CSBWIN_DSA_JUMP_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_JUMP_NOT_FOUND = 2,
    CSB_V1_CSBWIN_DSA_JUMP_NOT_JUMP = 3,
    CSB_V1_CSBWIN_DSA_JUMP_MALFORMED = -1
} CSB_V1_CSBWinDSAJumpResult;

/* Source dispatch-only boundary for DSACMD_JUMP. It owns neither a filter
 * activation nor any dungeon mutation: callers receive a value-copy of the
 * selected state/column transfer and decide when execution is source-complete. */
typedef struct {
    uint32_t source_state;
    uint32_t source_column;
    int continuation_state;
    uint32_t target_state;
    uint32_t target_column;
    uint16_t words_consumed;
} CSB_V1_CSBWinDSAJumpDispatch;

CSB_V1_CSBWinDSAJumpResult
csb_v1_csbwin_dsa_resolve_authenticated_jump_dispatch(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, CSB_V1_CSBWinDSAJumpDispatch *out_dispatch);

typedef enum {
    CSB_V1_CSBWIN_DSA_GOSUB_OK = 0,
    CSB_V1_CSBWIN_DSA_GOSUB_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_GOSUB_NOT_FOUND = 2,
    CSB_V1_CSBWIN_DSA_GOSUB_NOT_GOSUB = 3,
    CSB_V1_CSBWIN_DSA_GOSUB_MALFORMED = -1
} CSB_V1_CSBWinDSAGosubResult;

/* Source dispatch-only boundary for DSACMD_GOSUB. CSBWin records the outer
 * continuation, then calls Execute() at one greater subroutine depth. This
 * resolver reports those selected values but neither enters the nested frame
 * nor activates a filter or mutates the dungeon. */
typedef struct {
    uint32_t source_state;
    uint32_t source_column;
    int continuation_state;
    uint32_t target_state;
    uint32_t target_column;
    uint8_t subroutine_depth_delta;
    uint16_t words_consumed;
} CSB_V1_CSBWinDSAGosubDispatch;

CSB_V1_CSBWinDSAGosubResult
csb_v1_csbwin_dsa_resolve_authenticated_gosub_dispatch(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, CSB_V1_CSBWinDSAGosubDispatch *out_dispatch);

/* Bounded, receipt-only CSBWin Execute() transfer subset. This admits only
 * complete authenticated JUMP and GOSUB action programs. JUMP changes the
 * current state/column in its existing Execute frame; GOSUB pushes the source
 * continuation and enters its selected target one subroutine level deeper.
 * Missing state/column programs terminate a frame just as DSAState::Program
 * returning NULL does in CSBWin. Filter, world, stack, and other opcode paths
 * remain outside this boundary. */
#define CSB_V1_CSBWIN_DSA_EXECUTE_MAX_SUBROUTINE_DEPTH 64
#define CSB_V1_CSBWIN_DSA_EXECUTE_MAX_TRANSFERS 256

typedef enum {
    CSB_V1_CSBWIN_DSA_EXECUTE_OK = 0,
    CSB_V1_CSBWIN_DSA_EXECUTE_NOT_AUTHENTICATED = 1,
    CSB_V1_CSBWIN_DSA_EXECUTE_UNSUPPORTED = 2,
    CSB_V1_CSBWIN_DSA_EXECUTE_MALFORMED = -1,
    CSB_V1_CSBWIN_DSA_EXECUTE_DEPTH_LIMIT = -2,
    CSB_V1_CSBWIN_DSA_EXECUTE_TRANSFER_LIMIT = -3
} CSB_V1_CSBWinDSAExecuteResult;

typedef struct {
    uint32_t source_state;
    uint32_t source_column;
    int initial_subroutine_depth;
    int final_state;
    uint16_t transfer_count;
    uint16_t words_consumed;
    uint8_t maximum_subroutine_depth;
} CSB_V1_CSBWinDSAExecuteReceipt;

CSB_V1_CSBWinDSAExecuteResult
csb_v1_csbwin_dsa_execute_authenticated_transfer_subset(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    uint32_t column, int initial_subroutine_depth,
    CSB_V1_CSBWinDSAExecuteReceipt *out_receipt);

/* Executes precisely one complete CSBWin DSACMD_LOAD action program from an
 * authenticated DSAAction owner. It does not dispatch another opcode, mutate
 * dungeon data, or substitute behavior for source-illegal LOAD_ABS32. */
CSB_V1_CSBWinDSALoadResult csb_v1_csbwin_dsa_execute_load_action(
    const CSB_V1_DSAImportedAction *action,
    const CSB_V1_CSBWinDSALoadContext *context,
    CSB_V1_CSBWinDSALoadExecution *out_execution);

/* Executes only a complete source LOAD -> STORE action. `state` must own the
 * action after checksum-authenticated CSBWin Extended Features import; other
 * DSA command streams remain explicitly unsupported. */
CSB_V1_CSBWinDSALoadStoreResult
csb_v1_csbwin_dsa_execute_authenticated_load_store_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSALoadStoreContext *context,
    CSB_V1_CSBWinDSALoadStoreExecution *out_execution);

/* Executes a complete authenticated action composed solely of CSBWin LOAD,
 * STORE, local VARIABLEFETCH/VARIABLESTORE, global GLOBALFETCH/GLOBALSTORE,
 * and the pure EX_AMPERSAND stack/arithmetic/control STKOP subset. The source
 * creates a fresh 100-cell DSAVARS bank for each ProcessDSATimer6 invocation;
 * this boundary does the same and does not expose it after a successful
 * execution. Global access requires the caller-owned source-sized bank above,
 * and both parameter and global writes commit together only on success. Every
 * source word must be consumed. Unsupported DSA words, AMPERSAND2, filter, or
 * world paths, malformed extensions, stack faults, and
 * source-illegal LOAD_ABS32 reject without changing the caller's parameter
 * surface. */
CSB_V1_CSBWinDSAStackResult
csb_v1_csbwin_dsa_execute_authenticated_stack_action(
    const CSB_V1_ChaosMagicState *state, int dsa_id, uint32_t state_index,
    int action_ordinal, CSB_V1_CSBWinDSAStackContext *context,
    CSB_V1_CSBWinDSAStackExecution *out_execution);
const char *csb_v1_chaos_source_evidence(void);

#endif
